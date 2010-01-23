// ReportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ReportDlg.h"
#include "../RenameIt.h"
#include "RenameDlg.h"
#include "../NewMenu.h"
#include "../Math/OrientedGraph.h"

using namespace Beroux::IO::Renaming;


// CReportDlg dialog

IMPLEMENT_DYNAMIC(CReportDlg, CDialog)
CReportDlg::CReportDlg(CRenamingList& renamingList, CWnd* pParent /*=NULL*/)
	: CDialog(CReportDlg::IDD, pParent)
	, m_renamingList(renamingList)
	, m_bShowAllOperations(false)
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
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_REPORT_LIST, &CReportDlg::OnLvnItemActivateReportList)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(LVN_KEYDOWN, IDC_REPORT_LIST, &CReportDlg::OnLvnKeydownReportList)
	ON_BN_CLICKED(IDC_SHOW_ALL_CHECK, &CReportDlg::OnBnClickedShowAllCheck)
END_MESSAGE_MAP()


// CReportDlg message handlers

BOOL CReportDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Report columns
	CString str;
	str.LoadString(IDS_BEFORE);	m_ctlReportList.InsertColumn(0, str, LVCFMT_LEFT, 420, -1);
	str.LoadString(IDS_AFTER);	m_ctlReportList.InsertColumn(1, str, LVCFMT_LEFT, 420, -1);
	str.LoadString(IDS_ERROR);	m_ctlReportList.InsertColumn(2, str, LVCFMT_LEFT, 500, -1);

	// Create image list
	VERIFY( m_ilImages.Create(16, 16, ILC_COLOR8|ILC_MASK, 0, 4) );
	VERIFY( m_ilImages.Add( ::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_REMOVED)) ) == iconRemoved);
	VERIFY( m_ilImages.Add( ::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_CONFLICT)) ) == iconConflict);
	VERIFY( m_ilImages.Add( ::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_INVALIDNAME)) ) == iconInvalidName);
	VERIFY( m_ilImages.Add( ::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_BADNAME)) ) == iconBadName);
	m_ctlReportList.SetImageList(&m_ilImages, LVSIL_SMALL);
	ASSERT( &m_ilImages == m_ctlReportList.GetImageList(LVSIL_SMALL) );

	// Display the errors.
	ShowErrors();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CReportDlg::OnLvnItemActivateReportList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	// Rename on double click.
	RenameItem(pNMIA->iItem);

	*pResult = 0;
}

bool CReportDlg::RenameItem(int nItem)
{
	// Find the operation corresponding to this item.
	int nOperationIndex = (int) m_ctlReportList.GetItemData(nItem);

	// Show a message when the file doesn't exist.
	if (m_renamingList.GetOperationProblem(nOperationIndex).nErrorCode == CRenamingList::errFileMissing)
	{
		AfxMessageBox(IDS_CANNOT_RENAME_MISSING, MB_ICONINFORMATION);
		return false;
	}

	// Show renaming dialog
	CRenameDlg	ren;

	CPath fnOriginalFileName = m_renamingList[nOperationIndex].GetPathBefore();
	ren.SetOriginalFileName( fnOriginalFileName.GetFileName() );

	CPath pathAfter = m_renamingList[nOperationIndex].GetPathAfter();
	ren.SetNewFileName( pathAfter.GetFileName() );

	ASSERT(fnOriginalFileName.GetDirectoryName() == pathAfter.GetDirectoryName());
	CString strBaseFolder = fnOriginalFileName.GetDirectoryName();

	if (ren.DoModal() == IDOK)
	{
		// Change file name.
		CRenamingList::CRenamingOperation roRenamingOperation = m_renamingList[nOperationIndex];
		roRenamingOperation.SetPathAfter(strBaseFolder + ren.GetNewFileName());
		m_renamingList.SetRenamingOperation(nOperationIndex, roRenamingOperation);
		m_ctlReportList.SetItemText(nItem, 1, roRenamingOperation.GetPathAfter().GetDisplayPath());

		// Clear the error message.
		m_ctlReportList.SetItem(nItem, 0, LVIF_IMAGE, NULL, -1, 0, 0, NULL);
		m_ctlReportList.SetItemText(nItem, 2, _T(""));
		return true;
	}
	else
		return false;
}

BOOL CReportDlg::FindErrors(void)
{
	// Initialize progress display.
	m_dlgProgress.SetTitle(IDS_PGRS_TITLE);
	m_dlgProgress.SetCaption(IDS_PGRS_CHECK_CAPTION);
	m_renamingList.ProgressChanged.connect( bind(&CReportDlg::OnProgress, this, _1, _2, _3, _4) );

	// Find errors in another thread and display the progress.
	AfxBeginThread(CheckingThread, this);
	m_dlgProgress.DoModal();

	// Show the problems.
	ShowErrors();

	return TRUE;
}

void CReportDlg::ShowErrors()
{
	// Clear the displayed list.
	m_ctlReportList.DeleteAllItems();

	// Add operations.
	if (m_bShowAllOperations)
	{
		// Show all operations.
		for (int i=0; i<m_renamingList.GetCount(); ++i)
			InsertOperation(i);
	}
	else
	{
		// Show only errors.
		for (int nOperationIndex=0; nOperationIndex<m_renamingList.GetCount(); ++nOperationIndex)
		{
			const CRenamingList::COperationProblem& problem = m_renamingList.GetOperationProblem(nOperationIndex);

			if (problem.nErrorLevel != CRenamingList::levelNone)
				InsertOperation(nOperationIndex);
		}
	}

	// Update status.
	UpdateStatus();
}

void CReportDlg::InsertOperation(int nRenamingOperationIndex)
{
	// Get the file name before and after to display.
	CPath pathBefore = m_renamingList[nRenamingOperationIndex].GetPathBefore();
	CPath pathAfter = m_renamingList[nRenamingOperationIndex].GetPathAfter();

	// Add item to the report list.
	int nItemIndex = m_ctlReportList.InsertItem(LVIF_TEXT | LVIF_PARAM, m_ctlReportList.GetItemCount(), pathBefore.GetDisplayPath(), 0, 0, -1, nRenamingOperationIndex);
	if (nItemIndex == -1)
	{
		// Show critical error and exit the application.
		ASSERT(false);
		AfxMessageBox(IDS_CRITICAL_ERROR, MB_ICONSTOP);
		AfxPostQuitMessage(1);
		return;
	}
	VERIFY( m_ctlReportList.SetItemText(nItemIndex, 1, pathBefore.GetDisplayPath()) );
	VERIFY( m_ctlReportList.SetItemText(nItemIndex, 2, pathAfter.GetDisplayPath()) );

	// Update to show the problems.
	const CRenamingList::COperationProblem& problem = m_renamingList.GetOperationProblem(nRenamingOperationIndex);

	// Change icon in report list.
	int nIcon = -1;
	BOOST_STATIC_ASSERT(CRenamingList::errCount == 12);
	switch (problem.nErrorCode)
	{
	case CRenamingList::errNoError:
		nIcon = -1;	// Icon to use (no icon by default).
		break;
	
	case CRenamingList::errFileMissing:
	case CRenamingList::errInUse:
		nIcon = iconRemoved;
		break;

	case CRenamingList::errInvalidFileName:
	case CRenamingList::errInvalidDirectoryName:
	case CRenamingList::errBackslashMissing:
	case CRenamingList::errRootChanged:
		nIcon = iconInvalidName;
		break;

	case CRenamingList::errConflict:
		nIcon = iconConflict;
		break;

	case CRenamingList::errDirCaseInconsistent:
	case CRenamingList::errRiskyFileName:
	case CRenamingList::errRiskyDirectoryName:
	case CRenamingList::errLonguerThanMaxPath:
		nIcon = iconBadName;
		break;

	default:
		ASSERT(false);
		MsgBoxUnhandledError(__FILE__, __LINE__);
	}

	m_ctlReportList.SetItem(nItemIndex, 0, LVIF_IMAGE, NULL, nIcon, 0, 0, NULL);
	m_ctlReportList.SetItemText(nItemIndex, 2, problem.strMessage);
}

void CReportDlg::UpdateStatus()
{
	// Update status.
	CString str[3];
	str[0].Format(_T("%d"), m_renamingList.GetCount());
	str[1].Format(_T("%d"), m_renamingList.GetErrorCount());
	str[2].Format(_T("%d"), m_renamingList.GetWarningCount());
	LPCTSTR lpstr[] = {str[0], str[1], str[2]};
	CString	strStatus;
	AfxFormatStrings(strStatus, IDS_REPORT_BEFORE, lpstr, sizeof(lpstr)/sizeof(lpstr[0]));
	GetDlgItem(IDC_STATUS_STATIC)->SetWindowText(strStatus);
}

UINT CReportDlg::CheckingThread(LPVOID lpParam)
{
	CReportDlg* pThis = static_cast<CReportDlg*>(lpParam);

	// Do the checking.
	pThis->m_renamingList.Check();

	// Hide the progress dialog to continue in the main thread.
	pThis->m_dlgProgress.Done();
	return 0;
}

void CReportDlg::OnProgress(const CRenamingList& sender, CRenamingList::EStage nStage, int nDone, int nTotal)
{
	ASSERT(nStage == CRenamingList::stageChecking);
	m_dlgProgress.SetProgress(nStage, nDone, nTotal);
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
		for (int nOperationIndex=0; nOperationIndex<m_renamingList.GetCount(); ++nOperationIndex)
		{
			if (m_renamingList.GetOperationProblem(nOperationIndex).nErrorCode == CRenamingList::errFileMissing)
			{
				// Remove item
				RemoveItem(FindIndexOf(nOperationIndex));
			}
		}
		break;

	case 40:	// Refresh report list
		FindErrors();
		break;
	}

	// Clean memory
	if (bmpDel.m_hObject)
		bmpDel.DeleteObject();
	if (bmpDelAll.m_hObject)
		bmpDelAll.DeleteObject();
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
		FindErrors();
		break;
	}
	*pResult = 0;
}

void CReportDlg::OnOK()
{
	// If there are some errors, re-check.
	if (m_renamingList.GetErrorCount() > 0)
	{
		FindErrors();
		if (m_renamingList.GetErrorCount() > 0)
		{
			// Alert the user.
			AfxMessageBox(IDS_REPORT_ERROR_REMAIN, MB_ICONINFORMATION);

			return;	// No errors must be left.
		}
	}

	// Display a warning if the extension of one or more files is going to change.
	for (int i=0; i<m_renamingList.GetCount(); ++i)
	{
		// Is the extension going to change?
		if (CPath::FSCompare(m_renamingList[i].GetPathBefore().GetExtension(), m_renamingList[i].GetPathAfter().GetExtension()) != 0
			&& m_renamingList[i].GetPathBefore().GetExtension().GetLength() < 5)	// Very long extensions are probably not system extensions
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
	// Check the index.
	if (nIndex < m_ctlReportList.GetItemCount())
	{
		int nRenamingOperation = (int) m_ctlReportList.GetItemData(nIndex);
		m_renamingList.RemoveRenamingOperation(nRenamingOperation);
		m_ctlReportList.DeleteItem(nIndex);

		// After removing an operation of index X,
		// the index any operation Y > X must be indexed as Y-1.
		for (int i=nIndex; i<m_ctlReportList.GetItemCount(); ++i)
		{
			// We assume that items are ordered by increasing index.
			ASSERT((int) m_ctlReportList.GetItemData(i) > nRenamingOperation);

			m_ctlReportList.SetItemData(i, (int) m_ctlReportList.GetItemData(i) - 1);
		}

		// Update the status.
		UpdateStatus();
	}
}

void CReportDlg::OnBnClickedShowAllCheck()
{
	m_bShowAllOperations = (IsDlgButtonChecked(IDC_SHOW_ALL_CHECK) != 0);
	ShowErrors();
}

int CReportDlg::FindIndexOf(int nRenamingOperation) const
{
	// Find the item index corresponding to this operation.
	if (m_bShowAllOperations)
		return nRenamingOperation;
	else
	{
		LVFINDINFO findInfo;
		ZeroMemory(&findInfo, sizeof(LVFINDINFO));
		findInfo.flags = LVFI_PARAM;
		findInfo.lParam = nRenamingOperation;
		int nItemIndex = m_ctlReportList.FindItem(&findInfo);
		ASSERT(nItemIndex != -1);
		return nItemIndex;
	}
}

