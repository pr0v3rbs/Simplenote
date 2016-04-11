
// SimpleNoteDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SimpleNote.h"
#include "SimpleNoteDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSimpleNoteDlg dialog



CSimpleNoteDlg::CSimpleNoteDlg(CWnd* pParent /*=NULL*/)
    : CDialogEx(IDD_SIMPLENOTE_DIALOG, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CSimpleNoteDlg::~CSimpleNoteDlg()
{
    simplenoteManager_.Finalize();
}

void CSimpleNoteDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT, editBox);
    DDX_Control(pDX, IDC_NOTE_LIST, listBox);
    DDX_Control(pDX, IDC_USER_ID, editUserID);
    DDX_Control(pDX, IDC_USER_PW, editUserPW);
}

BEGIN_MESSAGE_MAP(CSimpleNoteDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_EN_CHANGE(IDC_EDIT, &CSimpleNoteDlg::OnEnChangeEdit)
    ON_LBN_SELCHANGE(IDC_NOTE_LIST, &CSimpleNoteDlg::OnLbnSelchangeNoteList)
    ON_BN_CLICKED(IDC_NEW_NOTE, &CSimpleNoteDlg::OnBnClickedNewNote)
    ON_BN_CLICKED(IDC_LOGIN, &CSimpleNoteDlg::OnBnClickedLogin)
END_MESSAGE_MAP()


// CSimpleNoteDlg message handlers

BOOL CSimpleNoteDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    // TODO: Add extra initialization here
    simplenoteManager_.Initialize(&editBox, &listBox);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

// prevent program exit when input enter on edit-box
BOOL CSimpleNoteDlg::PreTranslateMessage(MSG * pMsg)
{
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
    {
        if (GetDlgItem(IDC_EDIT) == GetFocus())
        {
        }
    }

    return 0;
}

void CSimpleNoteDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSimpleNoteDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSimpleNoteDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CSimpleNoteDlg::OnEnChangeEdit()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialogEx::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.
    // TODO:  Add your control notification handler code here

    CTime time = GetCurrentTime();
    simplenoteManager_.ModifyOn(time.GetTime());
}

void CSimpleNoteDlg::OnLbnSelchangeNoteList()
{
    // TODO: Add your control notification handler code here
    simplenoteManager_.SelectChanged();
}

// make new note
void CSimpleNoteDlg::OnBnClickedNewNote()
{
    //simplenoteManager_.MakeNewNote();
}

// click login
void CSimpleNoteDlg::OnBnClickedLogin()
{
    CString userID;
    CString userPW;
    editUserID.GetWindowTextW(userID);
    editUserPW.GetWindowTextW(userPW);

    if (simplenoteManager_.Login(userID, userPW))
    {
        userPW.Empty();
        editUserPW.Clear();
        MessageBox(_T("Login Success!"), _T("Success"), MB_OK);
    }
    else
    {
        MessageBox(_T("Login Fail!"), _T("Error"), MB_ICONERROR);
    }
}