// ReportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ReportDlg.h"
#include "RenameIt.h"
#include "RenameDlg.h"
#include "NewMenu.h"
#include "OrientedGraph.h"

extern CRenameItApp	theApp;

// CReportDlg dialog

IMPLEMENT_DYNAMIC(CReportDlg, CDialog)
CReportDlg::CReportDlg(CRenamingManager& renamingList, CWnd* pParent /*=NULL*/)
	: CDialog(CReportDlg::IDD, pParent)
	, m_renamingList(renamingList)
{
}

CReportDlg::~CReportDlg()
{
}

void CReportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_REPORT_LIST, m_ctlReportList);
}


BEGIN_MESSAGE_MAP(CReportDlg, CDialog)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_REPORT_LIST, OnLvnItemActivateReportList)
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDC_REFRESH_BUTTON, OnBnClickedRefreshButton)
	ON_NOTIFY(LVN_KEYDOWN, IDC_REPORT_LIST, OnLvnKeydownReportList)
END_MESSAGE_MAP()


// CReportDlg message handlers

BOOL CReportDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CreateReport();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CReportDlg::CreateReport()
{
	// Report columns
	CString str;
	str.LoadString(IDS_BEFORE);	m_ctlReportList.InsertColumn(0, str, LVCFMT_LEFT, 220, -1);
	str.LoadString(IDS_AFTER);	m_ctlReportList.InsertColumn(1, str, LVCFMT_LEFT, 220, -1);
	str.LoadString(IDS_DIR);	m_ctlReportList.InsertColumn(2, str, LVCFMT_LEFT, 400, -1);
	str.LoadString(IDS_ERROR);	m_ctlReportList.InsertColumn(3, str, LVCFMT_LEFT, 500, -1);

	// Create image list
	VERIFY( m_ilImages.Create(16, 16, ILC_COLOR8|ILC_MASK, 0, 4) );
	VERIFY( m_ilImages.Add( LoadIcon(theApp.m_hInstance,MAKEINTRESOURCE(IDI_REMOVED)) ) == iconRemoved);
	VERIFY( m_ilImages.Add( LoadIcon(theApp.m_hInstance,MAKEINTRESOURCE(IDI_CONFLICT)) ) == iconConflict);
	VERIFY( m_ilImages.Add( LoadIcon(theApp.m_hInstance,MAKEINTRESOURCE(IDI_INVALIDNAME)) ) == iconInvalidName);
	VERIFY( m_ilImages.Add( LoadIcon(theApp.m_hInstance,MAKEINTRESOURCE(IDI_BADNAME)) ) == iconBadName);
	m_ctlReportList.SetImageList(&m_ilImages, LVSIL_SMALL);
	ASSERT( &m_ilImages == m_ctlReportList.GetImageList(LVSIL_SMALL) );

	// Create report list
	for (int i=0; i<m_renamingList.GetCount(); ++i)
	{
		CFileName fnOriginalFileName = m_renamingList[i].fnBefore;
		CString strOriFileName = fnOriginalFileName.GetFileName() + fnOriginalFileName.GetExtension();
		CString strPath = fnOriginalFileName.GetDrive() + fnOriginalFileName.GetDirectory();

		CFileName fnNewFileName = m_renamingList[i].fnAfter;
		CString strNewFileName = fnNewFileName.GetFileName() + fnNewFileName.GetExtension();

		ASSERT(	CFileName(fnOriginalFileName.GetDrive() + fnOriginalFileName.GetDirectory()).FSCompare(
				CFileName(fnNewFileName.GetDrive() + fnNewFileName.GetDirectory())) == 0);	// Same path before/after renaming

		// Add item to the report list.
		int nItemIndex = m_ctlReportList.InsertItem(i, strOriFileName);
		if (nItemIndex == -1)
			return FALSE;
		VERIFY( m_ctlReportList.SetItemText(nItemIndex, 1, strNewFileName) );
		VERIFY( m_ctlReportList.SetItemText(nItemIndex, 2, strPath) );
	}

	// Find errors.
	VERIFY( FindErrors() );

	// Update status.
	UpdateStatus();

	// Rename file if no errors found.
	if (m_nErrors == 0 && m_nWarnings == 0)
		OnOK();

	return TRUE;
}

void CReportDlg::OnLvnItemActivateReportList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	POSITION	pos;
	int			nItem;

	if ((pos=m_ctlReportList.GetFirstSelectedItemPosition()) != NULL &&
		!(m_uvErrorFlag[nItem=m_ctlReportList.GetNextSelectedItem(pos)] & CRenamingManager::errMissingFlag))
		RenameItem(nItem);
	*pResult = 0;
}

BOOL CReportDlg::RenameItem(int nItem)
{
	// Pre-condition.
	ASSERT( !(m_uvErrorFlag[nItem] & CRenamingManager::errMissingFlag) );

	// Show renaming dialog
	CRenameDlg	ren;

	CFileName fnOriginalFileName = m_renamingList[nItem].fnBefore;
	ren.SetOriginalFileName( fnOriginalFileName.GetFileName() + fnOriginalFileName.GetExtension() );

	CFileName fnNewFileName = m_renamingList[nItem].fnAfter;
	ren.SetNewFileName( fnNewFileName.GetFileName() + fnNewFileName.GetExtension() );

	ASSERT(fnOriginalFileName.GetDrive() + fnOriginalFileName.GetDirectory() == fnNewFileName.GetDrive() + fnNewFileName.GetDirectory());
	CString strBaseFolder = fnOriginalFileName.GetDrive() + fnOriginalFileName.GetDirectory();

	if (ren.DoModal() == IDOK)
	{
		// Change file name
		CRenamingManager::CRenamingOperation roRenamingOperation = m_renamingList[nItem];
		roRenamingOperation.fnAfter = strBaseFolder + ren.GetNewFileName();
		m_renamingList.SetRenamingOperation(nItem, roRenamingOperation);
		m_ctlReportList.SetItem(nItem, 1, LVIF_TEXT, ren.GetNewFileName(), NULL, NULL, NULL, NULL);

		// Check for errors and set icons
		FindErrors();
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CReportDlg::FindErrors(void)
{
	// Clear error list.
	m_nErrors = 0;				// 0 errors found (until now)
	m_nWarnings = 0;			// 0 warnings found (unit now)

	// Initialize progress display.
	m_dlgProgress.SetTitle(IDS_PGRS_TITLE);
	m_dlgProgress.SetCaption(IDS_PGRS_CHECK_CAPTION);
	m_renamingList.SetProgressCallback(boost::bind(&CReportDlg::OnProgress, this, _1, _2, _3));

	// Find errors in another thread and display the progress.
	AfxBeginThread(CheckingThread, this);
	m_dlgProgress.DoModal();
	ASSERT(m_renamingList.GetCount() == m_uvErrorFlag.size());

	// Look for detected problems.
	for (int i=0; i<m_renamingList.GetCount(); ++i)
	{
		// Change icon in report list.
		int nErrorFlag = -1;	// Icon to use (no icon by default).
		CString strErrorMsg;
		if (m_uvErrorFlag[i] == 0)
		{
			// Don't do anything
		}
		else if (m_uvErrorFlag[i] & CRenamingManager::errMissingFlag)
		{
			++m_nErrors;
			nErrorFlag = iconRemoved;
			strErrorMsg.LoadString(IDS_REMOVED_FROM_DISK);
		}
		else if (m_uvErrorFlag[i] & CRenamingManager::errInvalidNameFlag)
		{
			++m_nErrors;
			nErrorFlag = iconInvalidName;
			strErrorMsg.LoadString(IDS_INVALID_FILE_NAME);
		}
		else if (m_uvErrorFlag[i] & CRenamingManager::errConflictFlag)
		{
			++m_nErrors;
			nErrorFlag = iconConflict;
			strErrorMsg.LoadString(IDS_CONFLICT);
		}
		else if (m_uvErrorFlag[i] & CRenamingManager::errBadNameFlag)
		{
			++m_nWarnings;
			nErrorFlag = iconBadName;
			strErrorMsg.LoadString(IDS_BAD_FILE_NAME);
		}
		else
		{
			ASSERT(false);
			++m_nErrors;
			MsgBoxUnhandledError(__FILE__, __LINE__);
			strErrorMsg = _T(" ? ? ? ");
		}
		m_ctlReportList.SetItem(i, 0, LVIF_IMAGE, NULL, nErrorFlag, 0, 0, NULL);
		m_ctlReportList.SetItemText(i, 3, strErrorMsg);
	}

	// Enable user to rename files if there are no more errors
	GetDlgItem(IDOK)->EnableWindow(m_nErrors == 0);

	return TRUE;
}

UINT CReportDlg::CheckingThread(LPVOID lpParam)
{
	CReportDlg* pThis = static_cast<CReportDlg*>(lpParam);
	pThis->m_uvErrorFlag = pThis->m_renamingList.FindErrors();
	pThis->m_dlgProgress.Done();
	return 0;
}

void CReportDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CNewMenu	menu;
	CString		str;
	DWORD		dwSelectionMade;
	POSITION	pos;
	CBitmap		bmpDel,
				bmpDelAll;
	UINT		uSelItem,
				uFlagRen = 0,
				uFlagDel = 0;
	CRect		rect;
	int			i;

	// Report list context menu
	if (pWnd->GetDlgCtrlID() != IDC_REPORT_LIST)
		return;

	// Get focused item of the list
	switch (m_ctlReportList.GetSelectedCount())
	{
	case 0:		// 0 item selected
		uFlagRen = uFlagDel = MF_GRAYED;
		break;
	case 1:		// 1 item selected
		break;
	default:	// +1 items selected
		uFlagRen = MF_GRAYED;
	}

	// Get selected item index
	pos = m_ctlReportList.GetFirstSelectedItemPosition();
	if (pos == NULL)
		uSelItem = -1;
	else
		uSelItem = m_ctlReportList.GetNextSelectedItem( pos );

	// If used keyboard to open context menu, find point
	if (point.x < 0 && point.y < 0)
	{
		if (uSelItem == -1)	// Get control position
			m_ctlReportList.GetWindowRect(rect);
		else				// Get selected item position
		{
			VERIFY( m_ctlReportList.GetItemRect(uSelItem, rect, LVIR_BOUNDS) );
			m_ctlReportList.ClientToScreen(rect);
		}
		point = rect.CenterPoint();
	}

	// Create menu bitmaps
	VERIFY( bmpDel.LoadBitmap(IDB_DEL) );
	VERIFY( bmpDelAll.LoadBitmap(IDB_DELALL) );

	// Create a context menu
	VERIFY( menu.CreatePopupMenu() );
	str.LoadString(IDS_MENU_RENAME);
	VERIFY( menu.AppendMenu(MF_STRING | uFlagRen, 10, str, NULL) );
	VERIFY( menu.AppendMenu(MF_SEPARATOR) );
	str.LoadString(IDS_MENU_REMOVE);
	VERIFY( menu.AppendMenu(MF_STRING | uFlagDel, 20, str, &bmpDel) );
	str.LoadString(IDS_MENU_REMOVE_MISSING);
	VERIFY( menu.AppendMenu(MF_STRING, 30, str, &bmpDelAll) );
	VERIFY( menu.AppendMenu(MF_SEPARATOR) );
	str.LoadString(IDS_MENU_REFRESH);
	VERIFY( menu.AppendMenu(MF_STRING, 40, str, NULL) );
	VERIFY( menu.SetDefaultItem(10) );
	str.LoadString(IDS_RENAME_LIST);
	VERIFY( menu.SetMenuTitle(str) );

	// Show and track the menu
	dwSelectionMade = menu.TrackPopupMenu(
		TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,
		point.x,
		point.y,
		this);

	// Execute command
	switch (dwSelectionMade)
	{
	case 10:	// Rename
		RenameItem(uSelItem);
		break;
	case 20:	// Remove
		while (true)
		{
			// Remove
			RemoveItem(uSelItem);

			// Get next selected item
			if ((pos = m_ctlReportList.GetFirstSelectedItemPosition()) == NULL)
				break;
			uSelItem = m_ctlReportList.GetNextSelectedItem( pos );
		}
		FindErrors();	// Find if errors left
		break;
	case 30:	// Remove missing
		for (i=0; i<(int)m_uvErrorFlag.size(); ++i)
			if (m_uvErrorFlag[i] & CRenamingManager::errMissingFlag)
			{
				// Remove item
				RemoveItem(i);
				i--;
			}
		FindErrors();	// Find if errors left
		break;
	case 40:	// Refresh report list
		OnBnClickedRefreshButton();
		break;
	}

	// Clean memory
	if (bmpDel.m_hObject)
		bmpDel.DeleteObject();
	if (bmpDelAll.m_hObject)
		bmpDelAll.DeleteObject();
}

void CReportDlg::OnBnClickedRefreshButton()
{
	FindErrors();
}

void CReportDlg::OnLvnKeydownReportList(NMHDR *pNMHDR, LRESULT *pResult)
{
	POSITION	pos;
	int			nSelItem;
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);

	// Intercept DEL-KEY
	switch (pLVKeyDow->wVKey)
	{
	case VK_DELETE:	// Remove select items
		while (true)
		{
			// Get selected item
			if ((pos = m_ctlReportList.GetFirstSelectedItemPosition()) == NULL)
				break;
			nSelItem = m_ctlReportList.GetNextSelectedItem(pos);

			// Remove
			RemoveItem(nSelItem);
		}
		break;
	case VK_F2:	// Rename
		// Get selected item
		if ((pos = m_ctlReportList.GetFirstSelectedItemPosition()) == NULL)
			break;
		nSelItem = m_ctlReportList.GetNextSelectedItem(pos);

		// Show renaming dialog
		RenameItem(nSelItem);
		break;
	case VK_F5:	// Refresh
		OnBnClickedRefreshButton();
		break;
	}
	*pResult = 0;
}

void CReportDlg::UpdateStatus(void)
{
	CString	strStatus;

	strStatus.Format(IDS_REPORT_BEFORE, m_renamingList.GetCount(), m_nErrors);
	GetDlgItem(IDC_STATUS_STATIC)->SetWindowText(strStatus);
}

void CReportDlg::OnOK()
{
	// No erors must be found.
	if (m_nErrors)
		return;

	// Display a warning if the extension of one or more files is going to change.
	for (int i=0; i<m_renamingList.GetCount(); ++i)
	{
		// Is the extension going to change?
		if (CFileName::FSCompare(m_renamingList[i].fnBefore.GetExtension(), m_renamingList[i].fnAfter.GetExtension()) != 0
			&& m_renamingList[i].fnBefore.GetExtension().GetLength() < 5)	// Very long extensions are probably not system extensions
		{
			if (AfxMessageBox(IDS_MODIFY_EXTENSION_WARNING, MB_ICONWARNING | MB_YESNO) == IDNO)
				return;
			else
				break;
		}
	}

	CDialog::OnOK();
}

void CReportDlg::RemoveItem(int nIndex)
{
	// Pre-conditions.
	ASSERT(m_ctlReportList.GetItemCount() == m_renamingList.GetCount());
	ASSERT(m_uvErrorFlag.size() == m_renamingList.GetCount());

	if (nIndex < m_renamingList.GetCount())
		m_renamingList.RemoveRenamingOperation(nIndex);
	if (nIndex < (int)m_uvErrorFlag.size())
		m_uvErrorFlag.erase(m_uvErrorFlag.begin() + nIndex);
	if (nIndex < m_ctlReportList.GetItemCount())
		m_ctlReportList.DeleteItem(nIndex);
}

void CReportDlg::OnProgress(CRenamingManager::EStage nStage, int nDone, int nTotal)
{
	ASSERT(nStage == CRenamingManager::stageChecking);
	m_dlgProgress.SetProgress(nDone, nTotal);
}
