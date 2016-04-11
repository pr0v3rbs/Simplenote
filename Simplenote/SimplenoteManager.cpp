#include "stdafx.h"
#include "SimplenoteManager.h"

#define CURRENT_NOTE(listBox) reinterpret_cast<Note*>(listBox->GetItemDataPtr(listBox->GetCurSel()))
#define IDX_NOTE(listBox, idx) reinterpret_cast<Note*>(listBox->GetItemDataPtr(idx))

bool SimplenoteManager::Initialize(CEdit* editBox, CListBox* listBox)
{
    bool result = false;

    editBox_ = editBox;
    listBox_ = listBox;
    lastModifyTime_ = 0;
    isTerminate_ = false;

    thread_ = AfxBeginThread(CommunicatorThread_, this, THREAD_PRIORITY_NORMAL, 0, 0);
    if (thread_ != NULL)
    {
        result = true;
    }

    return result;
}

bool SimplenoteManager::Login(CString& userID, CString& userPW)
{
    bool result = false;

    if (noteManager_.Login(userID, userPW))
    {
        result = true;
    }

    return result;
}

void SimplenoteManager::Finalize()
{
    isTerminate_ = true;
    WaitForSingleObject(thread_->m_hThread, INFINITE);
}

void SimplenoteManager::ModifyOn(__time64_t modifyTime)
{
    CString content;
    editBox_->GetWindowTextW(content);
    CURRENT_NOTE(listBox_)->content = content;
    CURRENT_NOTE(listBox_)->isModifying = true;
    lastModifyTime_ = modifyTime;
}

void SimplenoteManager::SelectChanged()
{
    if (mutex_.try_lock())
    {
        editBox_->SetWindowTextW(CURRENT_NOTE(listBox_)->content);
        mutex_.unlock();
    }
}

void SimplenoteManager::MakeNewNote()
{
    struct Note* newNote;
    noteManager_.CreateNote(&newNote);
    listBox_->InsertString(0, newNote->content);
    listBox_->SetItemDataPtr(0, newNote);
    listBox_->SetCurSel(0);
    editBox_->SetWindowTextW(_T(""));
}

UINT SimplenoteManager::CommunicatorThread_(LPVOID param)
{
    SimplenoteManager* instance = (SimplenoteManager*)param;
    return instance->CommunicateLoop_();
}

UINT SimplenoteManager::CommunicateLoop_()
{
    CTime time;
    std::vector<Note*> notePtrList;

    while (isTerminate_ == false)
    {
        time = GetCurrentTime();
        mutex_.lock();
        if (listBox_->GetCurSel() == LB_ERR || CURRENT_NOTE(listBox_)->isModifying == false || time.GetTime() - lastModifyTime_ > 5000)
        {
            // upload note
            noteManager_.UploadNote();

            // download notelist
            noteManager_.DownloadNoteList();

            if (noteManager_.ParseNoteList(notePtrList))
            {
                noteManager_.UpdateNote();

                CheckChangedNote_(notePtrList);

                // change note subject
                for (int idx = 0; idx < listBox_->GetCount(); ++idx)
                {
                    if (IDX_NOTE(listBox_, idx)->status == 2)
                    {
                        listBox_->SetDlgItemTextW(idx, IDX_NOTE(listBox_, idx)->content);

                        if (idx == listBox_->GetCurSel())
                        {
                            editBox_->SetWindowTextW(IDX_NOTE(listBox_, idx)->content);
                        }

                        IDX_NOTE(listBox_, idx)->status = 0;
                    }
                }
            }
        }

        // setting edit box
        if (listBox_->GetCurSel() == LB_ERR || CURRENT_NOTE(listBox_)->deleted == 1)
        {
            // select latest note
            if (listBox_->GetCount() > 0)
            {
                listBox_->SetCurSel(0);
                editBox_->SetWindowTextW(CURRENT_NOTE(listBox_)->content);
            }
            else
            {
                editBox_->Clear();
            }
        }
        mutex_.unlock();

        for (int i = 0; i < 50 && isTerminate_ == false; ++i)
            Sleep(100);
    }

    // update latest note
    noteManager_.UploadNote();

    return 1;
}

void SimplenoteManager::CheckChangedNote_(_In_ std::vector<Note*>& notePtrList)
{
    int idx = 0;

    // add new note
    for (size_t i = 0; i < notePtrList.size(); ++i)
    {
        idx = 0;
        for (; idx < listBox_->GetCount(); ++idx)
        {
            if (IDX_NOTE(listBox_, idx)->modifyTime < notePtrList[i]->modifyTime)
            {
                break;
            }
        }

        listBox_->InsertString(idx, notePtrList[i]->content);
        listBox_->SetItemDataPtr(idx, notePtrList[i]);
    }

    // delete removed note
    for (idx = 0; idx < listBox_->GetCount(); ++idx)
    {
        if (IDX_NOTE(listBox_, idx)->deleted == 1)
        {
            listBox_->SetItemDataPtr(idx, NULL);
            listBox_->DeleteString(idx);
            idx--;
        }
    }

    notePtrList.clear();
}