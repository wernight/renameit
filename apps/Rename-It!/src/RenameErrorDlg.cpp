// RenameErrorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RenameErrorDlg.h"
#include "RenameIt.h"

extern CRenameItApp	theApp;

namespace Beroux{ namespace IO{ namespace Renaming
{

// CRenameErrorDlg dialog

IMPLEMENT_DYNAMIC(CRenameErrorDlg, CDialog)

CRenameErrorDlg::CRenameErrorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRenameErrorDlg::IDD, pParent)
	, m_nErrorCount(0)
{

}

CRenameErrorDlg::~CRenameErrorDlg()
{
}

void CRenameErrorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_REPORT_LIST, m_ctlReport);
	DDX_Control(pDX, IDC_DESCR_STATIC, m_ctlDescriptionStatic);
	DDX_Control(pDX, IDC_ACTION_RADIO, m_ctlAction);
}


BEGIN_MESSAGE_MAP(CRenameErrorDlg, CDialog)
END_MESSAGE_MAP()


// CRenameErrorDlg message handlers

BOOL CRenameErrorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Replace strings in caption text.
	CString strCount;
	CString strDescription;
	m_ctlDescriptionStatic.GetWindowText(strDescription);
	strCount.Format(_T("%d"), m_nErrorCount); strDescription.Replace(_T("$(Files)"), strCount);
	strCount.Format(_T("%d"), m_vErrors.size()); strDescription.Replace(_T("$(Total)"), strCount);
	m_ctlDescriptionStatic.SetWindowText(strDescription);

	// Create image list.
	VERIFY( m_ilImages.Create(16, 16, ILC_COLOR8|ILC_MASK, 0, 4) );
	VERIFY( m_ilImages.Add( LoadIcon(theApp.m_hInstance, MAKEINTRESOURCE(IDI_REN_OK)) ) == iconOk);
	VERIFY( m_ilImages.Add( LoadIcon(theApp.m_hInstance, MAKEINTRESOURCE(IDI_REN_ERROR)) ) == iconError);
	m_ctlReport.SetImageList(&m_ilImages, LVSIL_SMALL);
	ASSERT( &m_ilImages == m_ctlReport.GetImageList(LVSIL_SMALL) );

	// Define the detailed report list.
	CString str;
	str.LoadString(IDS_BEFORE); m_ctlReport.InsertColumn(0, str);
	str.LoadString(IDS_AFTER); m_ctlReport.InsertColumn(1, str);
	for (int i=0; i<(int)m_vErrors.size(); ++i)
	{
		int nInsertedIndex = m_ctlReport.InsertItem(i, m_vErrors[i].fnBefore.GetPath());
		m_ctlReport.SetItem(nInsertedIndex, 1, LVIF_TEXT, m_vErrors[i].fnAfter.GetPath(), NULL, NULL, NULL, NULL);
		m_ctlReport.SetItem(nInsertedIndex, 2, LVIF_TEXT, m_vErrors[i].strError, NULL, NULL, NULL, NULL);

		// Change icon
		if (m_vErrors[i].strError.IsEmpty())
			m_ctlReport.SetItem(i, 0, LVIF_IMAGE, NULL, iconOk, 0, 0, NULL);
		else
			m_ctlReport.SetItem(i, 0, LVIF_IMAGE, NULL, iconError, 0, 0, NULL);
	}

	// Select the default action.
	m_ctlAction.SetCheck(BST_CHECKED);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CRenameErrorDlg::OnCancel()
{
	// We don't allow to cancel.
	//CDialog::OnCancel();
}

void CRenameErrorDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	if (m_ctlAction.GetState() & 0x0003)
		m_nAction = uaKeepCurrentState;
	else
		m_nAction = uaReverseToPreviousState;
	CDialog::OnOK();
}

}}}
