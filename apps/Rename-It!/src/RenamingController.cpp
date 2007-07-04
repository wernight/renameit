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
		if (m_renamingList[i].fnBefore == m_renamingList[i].fnAfter)
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

	do
	{
		// Define the callbacks for the renaming manager.
		m_renamingList.SetRenamedCallback(boost::bind(&CRenamingController::OnRenamed, this, _1, _2));
		m_renamingList.SetProgressCallback(boost::bind(&CRenamingController::OnProgress, this, _1, _2, _3));

		// Do/Continue the processing while showing the progress.
		AfxBeginThread(RenamingThread, this);
//FIXME: On cancelling the thread operation must be stopped.		m_dlgProgress.EnableCancel(m_nCurrentStage != CRenamingList::stageRenaming);
		if (m_dlgProgress.DoModal() == IDCANCEL)
			return false;

		switch (m_nCurrentStage)
		{
		case CRenamingList::stageChecking:	// The checking failed
			{
			// Show the report dialog.
			CReport report;
			ASSERT(m_renamingList.GetCount() == m_uvErrorFlag.size());
			if (!report.ShowReportFixErrors(m_renamingList, m_uvErrorFlag))
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
	if (m_dlgRenameError.GetErrorCount() == 0)
	{// All OK
		// Show message box.
		CString str;
		str.Format(IDS_RENAMED_ALL, m_nFilesToRename);
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
			m_renamingList.GetRenamingOperation(nIndex).fnBefore,
			m_renamingList.GetRenamingOperation(nIndex).fnAfter,
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
			m_renamingList.GetRenamingOperation(nIndex).fnBefore,
			m_renamingList.GetRenamingOperation(nIndex).fnAfter,
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
		pThis->m_uvErrorFlag = pThis->m_renamingList.FindErrors();
		ASSERT(pThis->m_renamingList.GetCount() == pThis->m_uvErrorFlag.size());

		// Look if there are problems.
		for (vector<unsigned>::iterator iter=pThis->m_uvErrorFlag.begin(); iter!=pThis->m_uvErrorFlag.end(); ++iter)
		{
			if (*iter != 0)
			{// A problem has been found,
				// Hide the progress window so the main thread can continue.
				ASSERT(pThis->m_renamingList.GetCount() == pThis->m_uvErrorFlag.size());
				pThis->m_dlgProgress.Done();
				return 0;
			}
		}
		pThis->m_nCurrentStage = CRenamingList::stagePreRenaming;

	case CRenamingList::stagePreRenaming:
		// Keep the number of files the user would like to rename.
		pThis->m_nFilesToRename = pThis->m_renamingList.GetCount();	// This value may change later, so we keep it.

		// Do the renaming.
		pThis->m_renamingList.PerformRenaming();
		pThis->m_nCurrentStage = CRenamingList::stageRenaming;

		// Now we are done, we can hide the progress dialog.
		pThis->m_dlgProgress.Done();
		return 0;

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
