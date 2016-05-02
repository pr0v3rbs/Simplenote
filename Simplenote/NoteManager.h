#pragma once

#include <vector>
#include <map>

enum NoteStatus
{
    kNormal = 0,
    kNew,
    kUpdate,
};

struct Note
{
    double modifyTime;
    double createTime;
    CString key;
    CString content;
    int deleted;
    int status;
    CString title;
    bool isLocal;
    bool isModifying;
};

class NoteManager
{
public:
    void Initialize();
    bool Login(CString& userID, CString& userPW);
    void CreateNote(struct Note** newNotePtr);
    void UploadNote();
    void DownloadNoteList();
    bool ParseNoteList(_In_ std::vector<Note*>& notePtrList);
    void UpdateNote();

private:
    HRESULT UploadNote_(Note* note);
    void SaveNote_(Note* note);
    bool NeedToUpdate_(Note* note);
    HRESULT DownloadNote_(CString& key, CString version = _T(""));
    bool UpdateNote_(Note* note);
    void GetNoteFromFile_(Note* note);
    void ConvertPythonData_(_In_ CString& inData, _Outref_ CStringA& outData, _Outref_ std::vector<int>& unicodeChecker);
    void UrlEncode_(_In_ CStringA& inData, _Outref_ CStringA& outData, _In_ std::vector<int>& unicodeChecker);
    void ConvertCStringToString_(_In_ CString& inData, _Out_ std::string& outData);

    CString email_;
    CString token_;
    std::map<CString, Note*> noteMap_;
    bool isLogin_;
};