
// SimpleNoteDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "SimplenoteManager.h"
#include "NoteManager.h"


// CSimpleNoteDlg dialog
class CSimpleNoteDlg : public CDialogEx
{
    // Construction & Destruction
public:
    CSimpleNoteDlg(CWnd* pParent = NULL);	// standard constructor
    ~CSimpleNoteDlg();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_SIMPLENOTE_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnEnChangeEdit();
    CEdit editBox;
    CEdit editUserID;
    CEdit editUserPW;
    CListBox listBox;

private:
    SimplenoteManager simplenoteManager_;
public:
    afx_msg void OnLbnSelchangeNoteList();
    afx_msg void OnBnClickedNewNote();
    afx_msg void OnBnClickedLogin();
};
