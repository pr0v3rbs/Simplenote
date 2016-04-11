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
		// sURL���� ���� �ּҿ� ������ �������� ��´�. http�� https�� �ƴ� ��쿡�� ����
		if (!AfxParseURL(sURL, dwServiceType, strServerName, strObject, nPort) ||
			(dwServiceType != AFX_INET_SERVICE_HTTP && dwServiceType != AFX_INET_SERVICE_HTTPS))
		{
			return GetLastError();
		}

		//�߰���� ����
		int nPos = sURL.Find(strServerName, 0);
		sSubPath = sURL.Mid(nPos + strServerName.GetLength());

		//���� ����
		hSession = InternetOpen(_T("Request"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if (hSession == NULL)
			return GetLastError();

		//���� �� ��Ʈ ��� ����
		hConnection = InternetConnect(hSession, strServerName, nPort, NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);
		if (hConnection == NULL)
		{
			InternetCloseHandle(hSession);
			return GetLastError();
		}

		//���� https ssl ����� �ʿ��ϴٸ� INTERNET_FLAG_SECURE �÷��� �߰�
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
		Method ��� ���� �� �÷��� ����
		GET�� ��� sSubPath�� ���ϴ� ���ڸ� �־��ָ� �ȴ�.
		POST�� ��쿡 ���� ��θ� �־��ָ� �ȴ�. ���� �����ʹ� HttpSendRequest���� ����
		*/
		hRequest = HttpOpenRequest(hConnection, _T("POST"), sSubPath, _T("HTTP/1.1"), NULL, NULL, dwFlags, 0);
		if (hRequest == NULL)
		{
			InternetCloseHandle(hConnection);
			InternetCloseHandle(hSession);

			return GetLastError();
		}

		/*
		https�� ����� ��� SSL ����� �� �� ������ ���� �ɼ�
		�׽�Ʈ ȯ�濡�� ���������� ���� ������ �ϰ�� 12045 ������ �߻��Ѵ�.
		12045 �˼� ���� �߱ޱ�� ������ ���쿡 �� �ɼ��� ������ �ָ� �����ϰ� ����Ѵ�.
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

		//����� ��� ����
		dwFlags = HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD;
		if (!HttpAddRequestHeaders(hRequest, aHeader, aHeader.GetLength(), dwFlags))
		{
			InternetCloseHandle(hRequest);
			InternetCloseHandle(hConnection);
			InternetCloseHandle(hSession);

			return GetLastError();
		}

		//HttpSendRequest ������ ������ ������ �������� �ʰ� ���� �غ� �ϴ� �����̴�.
		//���� ������ �� �� �Ǿ��ų� ip, port �� ���� ���� ��� ��κ� ���⼭ ������ �߻��Ѵ�.
		if (!HttpSendRequest(hRequest, NULL, NULL, sPostMsg.GetBuffer(0), sPostMsg.GetLength()))
		{
			InternetCloseHandle(hRequest);
			InternetCloseHandle(hConnection);
			InternetCloseHandle(hSession);

			return GetLastError();
		}

		//request ���� response�� �޴� �κ�
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