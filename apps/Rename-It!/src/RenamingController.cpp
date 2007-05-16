#include "StdAfx.h"
#include "RenamingController.h"
#include "RenamingManager.h"
#include "Report.h"

CRenamingController::CRenamingController(void) :
	m_nErrorLevel(elALL)
{
}

CRenamingController::~CRenamingController(void)
{
}

bool CRenamingController::RenameFiles(const CFileList& flBefore, const CFileList& flAfter)
{
	// Create a renaming list.
	m_rmRenamingManager.Create(flBefore, flAfter);

	// Remove files that have the same name before and after.
	for (int i=0; i<m_rmRenamingManager.GetCount(); ++i)
		if (m_rmRenamingManager[i].fnBefore == m_rmRenamingManager[i].fnAfter)
		{
			m_rmRenamingManager.RemoveRenamingOperation(i);
			--i;
		}

	// When there is no file to be renamed...
	if (m_rmRenamingManager.GetCount() == 0)
	{
		DisplayError(IDS_NO_FILENAME_CHANGES, elNotice);
		return true; // No file to rename.
	}

	// Check for errors and report them (if any); ask the user to fix them.
	{
		CReport report;
		if (!report.SearchAndFixErrors(m_rmRenamingManager))
			return false;	// Some errors are left.
	}

	// Keep the number of files the user would like to rename.
	int nFilesToRename = m_rmRenamingManager.GetCount();	// This value may change later, so we keep it.

	{
		// Define the callbacks for the renaming manager.
		m_rmRenamingManager.SetRenamedCallback(boost::bind(&CRenamingController::OnRenamed, this, _1, _2));
		m_rmRenamingManager.SetProgressCallback(boost::bind(&CRenamingController::OnProgress, this, _1, _2, _3));

		// Initialize progress display.
		m_dlgProgress.SetTitle(IDS_PGRS_TITLE);
		m_dlgProgress.SetCaption(IDS_PGRS_RENAMING_CAPTION);

		// Create a new thread to do the renaming while the current thread displays the GUI.
		AfxBeginThread(RenamingThread, this);
		if (m_dlgProgress.DoModal() == IDCANCEL)
			return false;
	}

	// No error?
	if (m_dlgRenameError.GetErrorCount() == 0)
	{// All OK
		// Show message box.
		CString str;
		str.Format(IDS_RENAMED_ALL, nFilesToRename);
		DisplayError(str, elNotice);
		return true;
	}

	// Ask the user what he wants to do.
	m_dlgRenameError.DoModal();
	switch (m_dlgRenameError.GetUserAction())
	{
	case CRenameErrorDlg::uaKeepCurrentState:
		return true;	// Just say it's been done as the user wanted.

	case CRenameErrorDlg::uaReverseToPreviousState:
		// FIXME
		return false;

	default:
		ASSERT(false);
		return false;
	}
}

void CRenamingController::OnRenamed(int nIndex, DWORD dwErrorCode)
{
	if (dwErrorCode == 0)
	{// Renaming succeed
		m_dlgRenameError.Add(
			m_rmRenamingManager.GetRenamingOperation(nIndex).fnBefore,
			m_rmRenamingManager.GetRenamingOperation(nIndex).fnAfter,
			NULL);
	}	
	else
	{// Renaming error
		// Get error message
		LPTSTR		lpMsgBuf = NULL;
		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dwErrorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL );

		// Add that error.
		m_dlgRenameError.Add(
			m_rmRenamingManager.GetRenamingOperation(nIndex).fnBefore,
			m_rmRenamingManager.GetRenamingOperation(nIndex).fnAfter,
			lpMsgBuf);

		// Free the buffer.
		LocalFree( lpMsgBuf );
	}
}

void CRenamingController::OnProgress(CRenamingManager::EStage nStage, int nDone, int nTotal)
{
	switch (nStage)
	{
	case CRenamingManager::stagePreRenaming:
		m_dlgProgress.SetCaption(IDS_PGRS_PREREN_CAPTION);
		break;

	case CRenamingManager::stageRenaming:
		m_dlgProgress.SetCaption(IDS_PGRS_RENAMING_CAPTION);
		break;

	default:
		ASSERT(FALSE);
	}
	m_dlgProgress.SetProgress(nDone, nTotal);
}

UINT CRenamingController::RenamingThread(LPVOID lpParam)
{
	CRenamingController* pThis = static_cast<CRenamingController*>(lpParam);
	pThis->m_rmRenamingManager.PerformRenaming();
	pThis->m_dlgProgress.Done();
	return 0;
}

void CRenamingController::DisplayError(UINT nMsgID, EErrorLevel nErrorLevel) const
{
	CString str;
	str.LoadString(nMsgID);
	DisplayError(str, nErrorLevel);
}

void CRenamingController::DisplayError(const CString& strErrorMsg, EErrorLevel nErrorLevel) const
{
	if (m_nErrorLevel & nErrorLevel)
	{
		switch (nErrorLevel)
		{
		case elWarning:
			AfxMessageBox(strErrorMsg, MB_ICONWARNING);
			break;

		case elNotice:
			AfxMessageBox(strErrorMsg, MB_ICONINFORMATION);
			break;

		default:
			ASSERT(false);
		case elError:
			AfxMessageBox(strErrorMsg, MB_ICONERROR);
			break;
		}
	}
}
