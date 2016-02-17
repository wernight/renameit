#include "StdAfx.h"
#include "AboutDlg.h"

//#define IDC_WEB_SO_ON_DEM 1133
//#define IDC_DEV_SO_ON_DEM 1134

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

HBRUSH CAboutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// Change the color
	switch (pWnd->GetDlgCtrlID())
	{
	case IDC_WEB_EDIT:
	case IDC_DEV_EDIT:
	//case IDC_WEB_SO_ON_DEM:
   // case IDC_DEV_SO_ON_DEM:
		pDC->SetTextColor(RGB(0x00, 0x00, 0xFF)); 
	}

	return hbr;
}

BOOL CAboutDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// Change the cursor
	switch (pWnd->GetDlgCtrlID())
	{
	case IDC_WEB_EDIT:
	case IDC_DEV_EDIT:
	//case IDC_WEB_SO_ON_DEM:
	//case IDC_DEV_SO_ON_DEM:
		
		::SetCursor(AfxGetApp()->LoadCursor(IDC_LINK_CURSOR));
		return TRUE;
	}

	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

BOOL CAboutDlg::PreTranslateMessage(MSG* pMsg)
{
	// Site Web
	if (pMsg->message == WM_LBUTTONDOWN)
		switch (::GetDlgCtrlID(pMsg->hwnd))
	{
		case IDC_WEB_EDIT:
			ShellExecute(GetSafeHwnd(), _T("open"), _T("http://www.beroux.com/?id=22"), NULL, NULL, SW_SHOWNORMAL);
			return TRUE;
		case IDC_DEV_EDIT:
			ShellExecute(GetSafeHwnd(), _T("open"), _T("mailto:werner@beroux.com"), NULL, NULL, SW_SHOWNORMAL);
			return TRUE;

		//case IDC_WEB_SO_ON_DEM:
		//	ShellExecute(GetSafeHwnd(), _T("open"), _T("http://www.software-on-demand-ita.com"), NULL, NULL, SW_SHOWNORMAL);
			//return TRUE;

//		case IDC_DEV_SO_ON_DEM:
			//ShellExecute(GetSafeHwnd(), _T("open"), _T("mailto:gppischedda@gmail.com"), NULL, NULL, SW_SHOWNORMAL);
			//return TRUE;

	}

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CAboutDlg::OnInitDialog()
{
	CString	str;

	CDialog::OnInitDialog();

	str.LoadString(IDS_HOME_PAGE);
	GetDlgItem(IDC_WEB_EDIT)->SetWindowText(str);
	GetDlgItem(IDC_DEV_EDIT)->SetWindowText(_T("Werner BEROUX"));

	//GetDlgItem(IDC_WEB_SO_ON_DEM)->SetWindowText(L"http://www.software-on-demand-ita.com");
//	GetDlgItem(IDC_DEV_SO_ON_DEM)->SetWindowText(_T("Giuseppe Pischedda"));

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

