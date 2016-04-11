#include "stdafx.h"
#include <WinInet.h>
#include <afxinet.h>
#include "HttpSend.h"

int HttpSend(CString sURL, CStringA sPostMsg, CString& sRecv)
{
	try
	{
		HINTERNET hSession, hConnection, hRequest;

		CString strServerName;
		CString strObject, strHeader, strData;
		INTERNET_PORT nPort;
		DWORD dwServiceType;
		CString sSubPath;
		DWORD dwFlags;
		BOOL bRead;

		char *pszBuf = NULL;
		DWORD dwBytesRead = 0;
		DWORD bufSize = 4096;

		CString aHeader;
		aHeader += _T("Accept: text*/*\r\n");
		aHeader += _T("User-Agent: Mozilla/4.0 (compatible; MSIE 5.0;* Windows NT)\r\n");
		aHeader += _T("Content-type: application/x-www-form-urlencoded\r\n");
		aHeader += _T("\r\n\r\n");


		/*
		ex)
		sURL = "http://127.0.0.1/test.asp";
		strServerName = "127.0.0.1";
		nPort = 80;
		sSubPath = "/test.asp";

		*/
		// sURL에서 서버 주소와 나머지 정보들을 얻는다. http와 https가 아닌 경우에는 리턴
		if (!AfxParseURL(sURL, dwServiceType, strServerName, strObject, nPort) ||
			(dwServiceType != AFX_INET_SERVICE_HTTP && dwServiceType != AFX_INET_SERVICE_HTTPS))
		{
			return GetLastError();
		}

		//중간경로 추출
		int nPos = sURL.Find(strServerName, 0);
		sSubPath = sURL.Mid(nPos + strServerName.GetLength());

		//세션 오픈
		hSession = InternetOpen(_T("Request"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if (hSession == NULL)
			return GetLastError();

		//서버 및 포트 방식 설정
		hConnection = InternetConnect(hSession, strServerName, nPort, NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);
		if (hConnection == NULL)
		{
			InternetCloseHandle(hSession);
			return GetLastError();
		}

		//만약 https ssl 통신이 필요하다면 INTERNET_FLAG_SECURE 플래그 추가
		if (dwServiceType == AFX_INET_SERVICE_HTTPS)
		{
			dwFlags = INTERNET_FLAG_SECURE |
				INTERNET_FLAG_RELOAD |
				INTERNET_FLAG_DONT_CACHE |
				INTERNET_FLAG_NO_COOKIES;
		}
		else
		{
			dwFlags = INTERNET_FLAG_RELOAD |
				INTERNET_FLAG_DONT_CACHE |
				INTERNET_FLAG_NO_COOKIES;
		}

		/*
		Method 방식 설정 및 플래그 설정
		GET일 경우 sSubPath에 원하는 인자를 넣어주면 된다.
		POST일 경우에 하위 경로만 넣어주면 된다. 실제 데이터는 HttpSendRequest에서 전송
		*/
		hRequest = HttpOpenRequest(hConnection, _T("POST"), sSubPath, _T("HTTP/1.1"), NULL, NULL, dwFlags, 0);
		if (hRequest == NULL)
		{
			InternetCloseHandle(hConnection);
			InternetCloseHandle(hSession);

			return GetLastError();
		}

		/*
		https를 사용할 경우 SSL 통신을 할 때 인증서 관련 옵션
		테스트 환경에서 개인적으로 만든 인증서 일경우 12045 에러가 발생한다.
		12045 알수 없는 발급기관 에러가 뜰경우에 이 옵션을 설정해 주면 무시하고 통신한다.
		*/
		if (dwServiceType == AFX_INET_SERVICE_HTTPS)
		{
			dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
				SECURITY_FLAG_IGNORE_REVOCATION |
				SECURITY_FLAG_IGNORE_REDIRECT_TO_HTTP |
				SECURITY_FLAG_IGNORE_REDIRECT_TO_HTTPS |
				SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
				SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
			if (!InternetSetOption(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags)))
			{
				InternetCloseHandle(hRequest);
				InternetCloseHandle(hConnection);
				InternetCloseHandle(hSession);

				return GetLastError();
			}
		}

		//통신할 헤더 설정
		dwFlags = HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD;
		if (!HttpAddRequestHeaders(hRequest, aHeader, aHeader.GetLength(), dwFlags))
		{
			InternetCloseHandle(hRequest);
			InternetCloseHandle(hConnection);
			InternetCloseHandle(hSession);

			return GetLastError();
		}

		//HttpSendRequest 전까진 실제로 서버에 접속하지 않고 접속 준비만 하는 상태이다.
		//서버 설정이 잘 못 되었거나 ip, port 가 맞지 않을 경우 대부분 여기서 에러가 발생한다.
		if (!HttpSendRequest(hRequest, NULL, NULL, sPostMsg.GetBuffer(0), sPostMsg.GetLength()))
		{
			InternetCloseHandle(hRequest);
			InternetCloseHandle(hConnection);
			InternetCloseHandle(hSession);

			return GetLastError();
		}

		//request 대한 response를 받는 부분
		pszBuf = (char *)malloc(sizeof(char) * bufSize + 1);
		memset(pszBuf, '\0', bufSize + 1);

		do
		{
			bRead = InternetReadFile(hRequest, pszBuf, bufSize, &dwBytesRead);

			if ((bRead) && (dwBytesRead>0)) {
				sRecv += pszBuf;
				memset(pszBuf, '\0', bufSize + 1);
			}
		} while ((bRead == TRUE) && (dwBytesRead > 0));

		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnection);
		InternetCloseHandle(hSession);

		free(pszBuf);
	}
	catch (CException& e)
	{
		TCHAR szCause[255];
		CString strFormatted;

		e.GetErrorMessage(szCause, 255);

		sRecv = szCause;

		return 1;
	}
	return NULL;
}