#include "stdafx.h"
#include "NoteManager.h"
#include <fstream>
#include <string>
#include "json/json.h"
#include "HttpSend.h"
#include "base64.h"

void NoteManager::Initialize()
{
    newNoteIdx_ = 0;
    isLogin_ = false;
}

bool NoteManager::Login(CString& userID, CString& userPW)
{
    bool result = false;

    email_ = userID;
    std::string id = CW2A(userID, CP_UTF8);
    std::string pw = CW2A(userPW, CP_UTF8);
    std::string data = "email=" + id + "&password=" + pw;
    std::string base64Data = Base64Encode(reinterpret_cast<const unsigned char*>(data.c_str()), data.length());
    CString response;

    if (HttpSend(_T("https://app.simplenote.com/api/login"), base64Data.c_str(), response) == 0)
    {
        token_ = response;
        if (token_.IsEmpty() == false)
        {
            email_ = userID;
            result = true;
            isLogin_ = true;
        }
    }

    return result;
}

void NoteManager::CreateNote(struct Note** newNotePtr)
{
    /*struct Note newNote;
    newNote.content = _T("");
    newNote.createTime = 0;
    newNote.deleted = 0;
    newNote.isLocal = true;
    newNote.key = _T("");
    newNote.modifyTime = 0;
    newNote.title = _T("");
    newNote.updated = false;

    noteList_.push_back(newNote);

    *newNotePtr = &noteList_[noteList_.size() - 1];

    ++newNoteIdx_;*/
}

void NoteManager::UploadNote()
{
    for (auto iter = noteMap_.begin(); iter != noteMap_.end(); ++iter)
    {
        if (iter->second.isModifying)
        {
            if (UploadNote_(iter->second) == S_OK)
            {
                iter->second.isModifying = false;
            }
            SaveNote_(iter->second);
        }
    }
}

void NoteManager::DownloadNoteList()
{
    if (isLogin_)
    {
        CString url = _T("https://app.simplenote.com/api2/index?auth=") + token_ + _T("&email=") + email_;

        CreateDirectory(_T(".\\notes\\") + email_, NULL);

        URLDownloadToFile(NULL, url, _T(".\\notes\\") + email_ + _T("\\notes_list"), 0, NULL);
    }
}

bool NoteManager::ParseNoteList(_In_ std::vector<Note*>& notePtrList)
{
    bool result = false;
    std::ifstream noteListFile(_T(".\\notes\\") + email_ + _T("\\notes_list"));
    std::string noteListInfo;

    if (noteListFile.is_open())
    {
        Json::Reader reader;
        Json::Value root;
        Json::Value notes;
        std::string temString;
        CString noteKey;
        Note note;

        getline(noteListFile, noteListInfo);
        noteListFile.close();

        if (reader.parse(noteListInfo, root))
        {
            notes = root["data"];
            for (auto iter = notes.begin(); iter != notes.end(); ++iter)
            {
                if (iter->isObject())
                {
                    noteKey = (*iter)["key"].asCString();
                    if (noteMap_.find(noteKey) == noteMap_.end())
                    {
                        temString = (*iter)["modifydate"].asString();
                        note.modifyTime = atof(temString.c_str());
                        note.deleted = (*iter)["deleted"].asInt();
                        temString = (*iter)["createdate"].asString();
                        note.createTime = atof(temString.c_str());
                        note.isModifying = false;
                        note.isLocal = false;
                        note.key = noteKey;
                        noteMap_[noteKey] = note;
                        if (noteMap_[noteKey].deleted == 0)
                        {
                            noteMap_[noteKey].status = 1;   // editable new note; need to get content;
                            notePtrList.push_back(&noteMap_[noteKey]);
                        }
                    }
                    else
                    {
                        int prevDeleted = noteMap_[noteKey].deleted;

                        temString = (*iter)["modifydate"].asString();
                        noteMap_[noteKey].modifyTime = atof(temString.c_str());
                        noteMap_[noteKey].deleted = (*iter)["deleted"].asInt();
                        temString = (*iter)["createdate"].asString();
                        noteMap_[noteKey].createTime = atof(temString.c_str());

                        if (prevDeleted == 1 && noteMap_[noteKey].deleted == 0)
                        {
                            notePtrList.push_back(&noteMap_[noteKey]);
                        }
                    }
                }
            }

            result = true;
            //DeleteFile(_T(".\\notes\\") + email_ + _T("\\notes_list"));
        }
    }

    return result;
}

void NoteManager::UpdateNote()
{
    for (auto iter = noteMap_.begin(); iter != noteMap_.end(); ++iter)
    {
        if (NeedToUpdate_(iter->second))
        {
            if (DownloadNote_(iter->second.key) == S_OK)
            {
                if (UpdateNote_(iter->second))
                {
                    GetNoteFromFile_(iter->second);
                    iter->second.status = 2;
                }
            }
        }
        // empty content note
        else if (iter->second.status == 1)
        {
            GetNoteFromFile_(iter->second);
            iter->second.status = 0;
        }
    }
}

HRESULT NoteManager::UploadNote_(Note& note)
{
    HRESULT result = E_FAIL;
    CString url;

    // upload previous note
    if (isLogin_)
    {
        if (note.key.GetLength() > 0)
        {
            url = _T("https://app.simplenote.com/api2/data/") + note.key + _T("?auth=") + token_ + _T("&email=") + email_;
            Json::StyledWriter writer;
            CStringA postData;
            CStringA pythonStyleString;
            CString response;
            Json::Value root;
            std::vector<int> unicodeChecker;

            ConvertPythonData_(note.content, pythonStyleString, unicodeChecker);
            root["content"] = pythonStyleString.GetString();

            // send post data
            pythonStyleString = writer.write(root).c_str();
            UrlEncode_(pythonStyleString, postData, unicodeChecker);
            if (HttpSend(url, postData, response) == 0)
            {
                Json::Reader reader;
                CT2CA pszConvertedAnsiString(response);
                std::string temString(pszConvertedAnsiString);
                if (reader.parse(temString, root))
                {
                    note.modifyTime = atof(root["modifydate"].asString().c_str());
                    result = S_OK;
                }
            }

            note.isLocal = result != S_OK;
        }
        // upload new note
        else
        {
            url = _T("https://app.simplenote.com/api2/data?auth=") + token_ + _T("&email=") + email_;
            // if upload success then parse,
            // else save local and insert noteList;
        }
    }

    return result;
}

void NoteManager::SaveNote_(Note& note)
{
    // TODO : save local note when not logined.
    std::string utf8String;

    ConvertCStringToString_(note.content, utf8String);
    std::ofstream noteFile(_T(".\\notes\\") + email_ + _T("\\") + note.key + _T("\\content"));
    noteFile << utf8String;
    noteFile.close();

    if (note.isLocal)
    {
        noteFile.open(_T(".\\notes\\") + email_ + _T("\\") + note.key + _T("\\local"));
        noteFile.close();
    }
    else
    {
        noteFile.open(_T(".\\notes\\") + email_ + _T("\\") + note.key + _T("\\modifydate"));
        noteFile << std::to_string(note.modifyTime);
        noteFile.close();
        DeleteFile(_T(".\\notes\\") + email_ + _T("\\") + note.key + _T("\\local"));
    }
}

bool NoteManager::NeedToUpdate_(Note& note)
{
    bool result = false;
    std::ifstream localFile(_T(".\\notes\\") + email_ + _T("\\") + note.key + _T("\\local"));

    // local file, must not update, but need to upload
    if (localFile.is_open())
    {
        note.isLocal = true;
        note.isModifying = true;
        localFile.close();
    }
    else
    {
        std::ifstream modifydateFile(_T(".\\notes\\") + email_ + _T("\\") + note.key + _T("\\modifydate"));
        std::string localModifyTimeString;
        double localModifyTime;

        if (modifydateFile.is_open())
        {
            getline(modifydateFile, localModifyTimeString);
            modifydateFile.close();
            localModifyTime = atof(localModifyTimeString.c_str());

            if (localModifyTime < note.modifyTime)
            {
                result = true;
            }
        }
        else // file doesn't exist
        {
            result = true;
        }
    }

    return result;
}

HRESULT NoteManager::DownloadNote_(CString& key, CString version)
{
    HRESULT result = S_FALSE;

    if (isLogin_)
    {
        CString url = L"";
        CString paramVersion = L"";

        if (version != L"")
        {
            paramVersion = L"/" + version;
        }

        url = L"https://app.simplenote.com/api2/data/" + key + paramVersion;
        url += L"?auth=" + token_ + L"&email=" + email_;

        result = URLDownloadToFile(NULL, url, _T(".\\notes\\") + email_ + _T("\\") + key + _T("_info"), 0, NULL);
    }

    return result;
}

bool NoteManager::UpdateNote_(Note& note)
{
    bool result = false;
    std::ifstream noteInfoFile(_T(".\\notes\\") + email_ + _T("\\") + note.key + _T("_info"));
    std::string noteInfo;

    if (noteInfoFile.is_open())
    {
        getline(noteInfoFile, noteInfo);

        Json::Reader reader;
        Json::Value noteJson;

        if (reader.parse(noteInfo, noteJson))
        {
            CreateDirectory(_T(".\\notes\\") + email_ + _T("\\") + note.key, NULL);
            std::ofstream noteFile(_T(".\\notes\\") + email_ + _T("\\") + note.key + _T("\\modifydate"));
            noteFile << noteJson["modifydate"].asString();
            noteFile.close();

            noteFile.open(_T(".\\notes\\") + email_ + _T("\\") + note.key + _T("\\content"));
            noteFile << noteJson["content"].asString();
            noteFile.close();

            result = true;
            noteInfoFile.close();
            DeleteFile(_T(".\\notes\\") + email_ + _T("\\") + note.key + _T("_info"));
        }
    }

    return result;
}

void NoteManager::GetNoteFromFile_(Note& note)
{
    std::wifstream noteInfoFile(_T(".\\notes\\") + email_ + _T("\\") + note.key + _T("\\content"));
    std::wstring str;

    if (noteInfoFile.is_open())
    {
        note.content.Empty();
        noteInfoFile.imbue(std::locale(noteInfoFile.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>()));
        while (!noteInfoFile.eof())
        {
            getline(noteInfoFile, str);
            note.content += str.c_str();
            note.content += L"\x0d\x0a";
        }

        note.content.Delete(note.content.GetLength() - 2, 2);
        noteInfoFile.close();
    }
}

void NoteManager::ConvertPythonData_(_In_ CString& inData, _Outref_ CStringA& outData, std::vector<int>& unicodeChecker)
{
    outData = "";

    for (int i = 0; i < inData.GetLength(); i++)
    {
        if (inData[i] == L'\x0d' && inData[i + 1] == L'\x0a')
        {
            continue;
        }
        else if (inData[i] < 128)
        {
            outData.AppendChar(static_cast<char>(inData[i]));
        }
        else
        {
            outData.AppendFormat("\\u%04x", inData[i]);
            unicodeChecker.push_back(outData.GetLength() - 6);
        }
    }
}

void NoteManager::UrlEncode_(_In_ CStringA& inData, _Outref_ CStringA& outData, _In_ std::vector<int>& unicodeChecker)
{
    outData = "";

    int unicodeCheckerIdx = 0;
    int baseIdx = 18;
    for (int i = 0; i < inData.GetLength(); i++)
    {
        if (('a' <= inData[i] && inData[i] <= 'z') ||
            ('A' <= inData[i] && inData[i] <= 'Z') ||
            ('0' <= inData[i] && inData[i] <= '9'))
        {
            outData.AppendChar(inData[i]);
        }
        else if (unicodeChecker.size() > 0 && i == baseIdx + unicodeChecker[unicodeCheckerIdx])
        {
            // skip this index to change "\\u3141" -> "\u3141"
            unicodeCheckerIdx++;
        }
        else
        {
            if (inData[i] == 0x5c)
                baseIdx++;
            outData.AppendFormat("%%%02X", inData[i]);
        }
    }
}

void NoteManager::ConvertCStringToString_(_In_ CString& inData, _Out_ std::string& outData)
{
    outData = CW2A(inData, CP_UTF8);

    std::string::size_type pos = 0;
    std::string::size_type offset = 0;

    while ((pos = outData.find("\x0d\x0a", offset)) != std::string::npos)
    {
        outData.replace(outData.begin() + pos, outData.begin() + pos + 2, "\x0a");
        offset = pos + 1;
    }
}