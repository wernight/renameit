// RenameErrorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RenameErrorDlg.h"
#include "RenameIt.h"

// CRenameErrorDlg dialog

IMPLEMENT_DYNAMIC(CRenameErrorDlg, CSizingDialog)

CRenameErrorDlg::CRenameErrorDlg(CWnd* pParent /*=NULL*/)
	: CSizingDialog(CRenameErrorDlg::IDD, pParent)
	, m_nErrorCount(0)
	, m_bDialogInitialized(false)
	, m_bUsingTransaction(false)
{
}

CRenameErrorDlg::~CRenameErrorDlg()
{
}

CRenameErrorDlg::EUserAction CRenameErrorDlg::ShowDialog()
{
	// Show the dialog window.
	VERIFY(DoModal() == IDOK);
	return m_nSelectedAction;
}

void CRenameErrorDlg::DoDataExchange(CDataExchange* pDX)
{
	CSizingDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_REPORT_LIST, m_ctlReport);
	DDX_Control(pDX, IDC_DESCR_STATIC, m_ctlDescriptionStatic);
	DDX_Control(pDX, IDC_ACTION_RADIO, m_ctlActionCommit);
	DDX_Control(pDX, IDC_ACTION2_RADIO, m_ctlActionRollBack);
	DDX_Control(pDX, IDC_ACTION3_RADIO, m_ctlActionKeep);
}


BEGIN_MESSAGE_MAP(CRenameErrorDlg, CSizingDialog)
	ON_BN_CLICKED(IDC_DETAILS_BUTTON, &CRenameErrorDlg::OnBnClickedButtonShowDetails)
	ON_BN_CLICKED(IDC_HIDE_DETAILS_BUTTON, &CRenameErrorDlg::OnBnClickedButtonHideDetails)
	ON_BN_CLICKED(IDC_SHOW_ONLY_PROBLEMS_CHECK, &CRenameErrorDlg::OnBnClickedShowOnlyProblemsCheck)
	ON_BN_CLICKED(IDC_ACTION_RADIO, &CRenameErrorDlg::OnBnClickedActionRadio)
	ON_BN_CLICKED(IDC_ACTION2_RADIO, &CRenameErrorDlg::OnBnClickedActionRadio)
	ON_BN_CLICKED(IDC_ACTION3_RADIO, &CRenameErrorDlg::OnBnClickedActionRadio)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CRenameErrorDlg message handlers

BOOL CRenameErrorDlg::OnInitDialog()
{
	CSizingDialog::OnInitDialog();

	// Define how to resize.
	AddResizableCtrl(IDC_DESCR_STATIC, _T("CX"));
	AddResizableCtrl(IDC_SHOW_ONLY_PROBLEMS_CHECK, _T("Y"));
	AddResizableCtrl(IDC_HIDE_DETAILS_BUTTON, _T("XY"));
	AddResizableCtrl(IDC_REPORT_LIST, _T("C"));
	AddResizableCtrl(IDOK, _T("RX+Y"));

	// Show correct full error message.
	{
		CString strDescription;
		m_ctlDescriptionStatic.GetWindowText(strDescription);
		if (m_nErrorCount == m_vErrors.size())
			strDescription = strDescription.Mid(strDescription.Find(_T("||")) + 2);
		else
			strDescription = strDescription.Left(strDescription.Find(_T("||")));

		CString strCount;
		strCount.Format(_T("%d"), m_nErrorCount); strDescription.Replace(_T("$(Errors)"), strCount);
		strCount.Format(_T("%d"), m_vErrors.size()); strDescription.Replace(_T("$(Total)"), strCount);

		m_ctlDescriptionStatic.SetWindowText(strDescription);
	}

	// Create image list.
	VERIFY( m_ilImages.Create(16, 16, ILC_COLOR8|ILC_MASK, 0, 4) );
	VERIFY( m_ilImages.Add( ::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_REN_OK)) ) == iconOk);
	VERIFY( m_ilImages.Add( ::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_REN_ERROR)) ) == iconError);
	m_ctlReport.SetImageList(&m_ilImages, LVSIL_SMALL);
	ASSERT( &m_ilImages == m_ctlReport.GetImageList(LVSIL_SMALL) );

	// Define the detailed report list.
	CString str;
	str.LoadString(IDS_BEFORE); m_ctlReport.InsertColumn(0, str);
	str.LoadString(IDS_AFTER); m_ctlReport.InsertColumn(1, str);
	str.LoadString(IDS_ERROR); m_ctlReport.InsertColumn(2, str);
	UpdateErrorList();

	// Show the control depending on m_bUsingTransaction.
	{
		// Show the available options.
		m_ctlActionCommit.ShowWindow(m_bUsingTransaction ? SW_SHOW : SW_HIDE);
		m_ctlActionRollBack.ShowWindow(m_bUsingTransaction ? SW_SHOW : SW_HIDE);
		m_ctlActionKeep.ShowWindow(m_bUsingTransaction ? SW_HIDE : SW_SHOW);

		// Select the default action.
		if (!m_bUsingTransaction)
			m_ctlActionKeep.SetCheck(BST_CHECKED);

		// Enable the OK button only when some option is selected.
		GetDlgItem(IDOK)->EnableWindow(!m_bUsingTransaction);
	}

	m_bDialogInitialized = true;
	SetWindowPos(NULL, 0, 0, 300, 200, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CRenameErrorDlg::OnSize(UINT nType, int cx, int cy)
{
	CSizingDialog::OnSize(nType, cx, cy);

	// Resize columns...
	if (m_bDialogInitialized)
	{
		const int nScrollBarWidth = 30;

		// Error list control columns
		RECT rect;
		m_ctlReport.GetWindowRect(&rect);
		int nCtrlWidth = rect.right - rect.left - nScrollBarWidth;
		m_ctlReport.SetColumnWidth(0, nCtrlWidth/3);
		m_ctlReport.SetColumnWidth(1, nCtrlWidth/3);
		m_ctlReport.SetColumnWidth(2, nCtrlWidth/3);
	}
}

void CRenameErrorDlg::UpdateErrorList()
{
	bool bShowOnlyProblems = (IsDlgButtonChecked(IDC_SHOW_ONLY_PROBLEMS_CHECK) != 0);

	m_ctlReport.DeleteAllItems();
	for (int i=0; i<(int)m_vErrors.size(); ++i)
	{
		if (bShowOnlyProblems && m_vErrors[i].strError.IsEmpty())
			continue;

		int nInsertedIndex = m_ctlReport.InsertItem(i, m_vErrors[i].fnBefore.GetDisplayPath());
		m_ctlReport.SetItem(nInsertedIndex, 1, LVIF_TEXT, m_vErrors[i].fnAfter.GetDisplayPath(), NULL, NULL, NULL, NULL);
		m_ctlReport.SetItem(nInsertedIndex, 2, LVIF_TEXT, m_vErrors[i].strError, NULL, NULL, NULL, NULL);

		// Change icon
		if (m_vErrors[i].strError.IsEmpty())
			m_ctlReport.SetItem(nInsertedIndex, 0, LVIF_IMAGE, NULL, iconOk, 0, 0, NULL);
		else
			m_ctlReport.SetItem(nInsertedIndex, 0, LVIF_IMAGE, NULL, iconError, 0, 0, NULL);
	}
}

void CRenameErrorDlg::OnBnClickedButtonShowDetails()
{
	SetWindowPos(NULL, 0, 0, 2*256, 2*214, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
	m_ctlReport.ShowWindow(SW_SHOW);
	GetDlgItem(IDC_DETAILS_BUTTON)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_HIDE_DETAILS_BUTTON)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_SHOW_ONLY_PROBLEMS_CHECK)->ShowWindow(SW_SHOW);
}

void CRenameErrorDlg::OnBnClickedButtonHideDetails()
{
	m_ctlReport.ShowWindow(SW_HIDE);
	GetDlgItem(IDC_DETAILS_BUTTON)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_HIDE_DETAILS_BUTTON)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_ONLY_PROBLEMS_CHECK)->ShowWindow(SW_HIDE);

	// TODO: Restore the original window size.
}

void CRenameErrorDlg::OnBnClickedShowOnlyProblemsCheck()
{
	UpdateErrorList();
}

void CRenameErrorDlg::OnBnClickedActionRadio()
{
	GetDlgItem(IDOK)->EnableWindow(TRUE);
}

void CRenameErrorDlg::OnCancel()
{
	// We don't allow to cancel.
	//CSizingDialog::OnCancel();
}

void CRenameErrorDlg::OnOK()
{
	// Get the selected answer.
	if (m_bUsingTransaction)
	{
		if (m_ctlActionCommit.GetState() & 0x0003)
			m_nSelectedAction = uaKeepCurrentState;
		else if (m_ctlActionRollBack.GetState() & 0x0003)
			m_nSelectedAction = uaReverseToPreviousState;
		else
		{
			ASSERT(false);
			m_nSelectedAction = uaReverseToPreviousState;
		}
	}
	else
	{
		if (m_ctlActionKeep.GetState() & 0x0003)
			m_nSelectedAction = uaKeepCurrentState;
		else
		{
			ASSERT(false);
			m_nSelectedAction = uaReverseToPreviousState;
		}
	}

	// OK
	m_bDialogInitialized = false;
	CSizingDialog::OnOK();
}
