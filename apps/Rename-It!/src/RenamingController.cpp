#include "StdAfx.h"
#include "RenamingController.h"
#include "RenamingList.h"
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
	// Initialize progress display.
	m_dlgProgress.SetTitle(IDS_PGRS_TITLE);

	// Create a renaming list.
	m_renamingList.Create(flBefore, flAfter);

	// Remove files that have the same name before and after.
	for (int i=0; i<m_renamingList.GetCount(); ++i)
		if (m_renamingList[i].pathBefore == m_renamingList[i].pathAfter)
		{
			m_renamingList.RemoveRenamingOperation(i);
			--i;
		}

	// When there is no file to be renamed...
	if (m_renamingList.GetCount() == 0)
	{
		DisplayError(IDS_NO_FILENAME_CHANGES, elNotice);
		return true; // No file to rename.
	}

	// Initialize
	m_nCurrentStage = CRenamingList::stageChecking;
	m_nPreviousStage = -1;	// No progress displayed yet.

	// Keep the number of files the user would like to rename.
	m_nFilesToRename = m_renamingList.GetCount();	// This value may change later, so we keep it.

	bool bSuccess = false;	// Some errors are supposed.
	do
	{
		// Define the callbacks for the renaming manager.
		m_renamingList.SetRenamedCallback(boost::bind(&CRenamingController::OnRenamed, this, _1, _2));
		m_renamingList.SetProgressCallback(boost::bind(&CRenamingController::OnProgress, this, _1, _2, _3));

		// Do/Continue the processing while showing the progress.
		{
			CWinThread thread(RenamingThread, this);
			thread.m_bAutoDelete = FALSE;
			thread.CreateThread();
			/* FIXME: On cancelling the thread operation must be stopped.
			 * m_dlgProgress.EnableCancel(m_nCurrentStage != CRenamingList::stageRenaming);
			 */
			if (m_dlgProgress.DoModal() == IDCANCEL)
				return false;

			// Wait for the working thread to terminate.
			WaitForSingleObject(thread.m_hThread, INFINITE);
			DWORD dwExitCode = 0;
			::GetExitCodeThread(thread.m_hThread, &dwExitCode);
			bSuccess = (dwExitCode == 0);
		}

		// Where are we?
		switch (m_nCurrentStage)
		{
		case CRenamingList::stageChecking:	// The checking failed
			{
			// Alert the user.
			AfxMessageBox(IDS_REPORT_ERROR_DETECTED, MB_ICONINFORMATION);

			// Show the report dialog.
			CReport report;
			if (!report.ShowReportFixErrors(m_renamingList))
				return false;	// Some errors are left or the user cancelled.
			else
				// We continue at the next stage.
				++m_nCurrentStage;
			break;
			}

		case CRenamingList::stageRenaming:	// The renaming is done
			break;

		default:
			ASSERT(false);	// We shouldn't break here.
		}
	} while (m_nCurrentStage != CRenamingList::stageRenaming);

	// No error?
	if (bSuccess)
	{// All OK
		// Show message box.
		CString str;
		str.Format(IDS_RENAMED_ALL, m_nFilesToRename);
		DisplayError(str, elNotice);
		return true;
	}
	else
	{// Some errors
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
}

void CRenamingController::OnRenamed(int nIndex, DWORD dwErrorCode)
{
	if (dwErrorCode == 0)
	{// Renaming succeed
		m_dlgRenameError.Add(
			m_renamingList.GetRenamingOperation(nIndex).pathBefore,
			m_renamingList.GetRenamingOperation(nIndex).pathAfter,
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
			m_renamingList.GetRenamingOperation(nIndex).pathBefore,
			m_renamingList.GetRenamingOperation(nIndex).pathAfter,
			lpMsgBuf);

		// Free the buffer.
		LocalFree( lpMsgBuf );
	}
}

UINT CRenamingController::RenamingThread(LPVOID lpParam)
{
	CRenamingController* pThis = static_cast<CRenamingController*>(lpParam);

	switch (pThis->m_nCurrentStage)
	{
	case CRenamingList::stageChecking:
		// Check if there are some errors.
		if (!pThis->m_renamingList.Check())
		{// A problem has been found.
			// Hide the progress window so the main thread can continue.
			pThis->m_dlgProgress.Done();
			return 0;
		}
		pThis->m_nCurrentStage = CRenamingList::stagePreRenaming;

	case CRenamingList::stagePreRenaming:
		// Keep the number of files the user would like to rename.
		pThis->m_nFilesToRename = pThis->m_renamingList.GetCount();	// This value may change later, so we keep it.

		// Do the renaming.
		{
			bool bSuccess = pThis->m_renamingList.PerformRenaming();
			pThis->m_nCurrentStage = CRenamingList::stageRenaming;

			// Now we are done, we can hide the progress dialog.
			pThis->m_dlgProgress.Done();
			if (bSuccess)
				return 0;
			else
				return 1;
		}

	default:
		ASSERT(false);
		return 0;
	}
}

void CRenamingController::OnProgress(CRenamingList::EStage nStage, int nDone, int nTotal)
{
	if (nStage != m_nPreviousStage)
	{
		switch (nStage)
		{
		case CRenamingList::stageChecking:
			m_dlgProgress.SetCaption(IDS_PGRS_CHECK_CAPTION);
			break;

		case CRenamingList::stagePreRenaming:
			m_dlgProgress.SetCaption(IDS_PGRS_PREREN_CAPTION);
			break;

		case CRenamingList::stageRenaming:
			m_dlgProgress.EnableCancel(false);
			m_dlgProgress.SetCaption(IDS_PGRS_RENAMING_CAPTION);
			break;

		default:
			ASSERT(FALSE);
		}

		m_nPreviousStage = nStage;
	}
	m_dlgProgress.SetProgress(nStage, nDone, nTotal);
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
