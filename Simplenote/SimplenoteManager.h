#pragma once
#include "NoteManager.h"

class SimplenoteManager
{
public:
    bool Initialize(CEdit* editBox, CListBox* listBox);
    void Finalize();
    bool Login(CString& userID, CString& userPW);
    void ModifyOn(__time64_t modifyTime);
    void SelectChanged();
    void MakeNewNote();

private:
    static UINT CommunicatorThread_(LPVOID param);
    UINT CommunicateLoop_();
    void CheckChangedNote_(_In_ std::vector<Note*>& notePtrList);

    CEdit* editBox_;
    CListBox* listBox_;
    bool isTerminate_;
    CWinThread* thread_;
    NoteManager noteManager_;
    __time64_t lastModifyTime_;
    std::mutex mutex_;

};