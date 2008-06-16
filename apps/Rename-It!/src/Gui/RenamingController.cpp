#include "StdAfx.h"
#include "RenamingController.h"
#include "IO/Renaming/DirectoryRemovalError.h"
#include "IO/Renaming/RenamingList.h"
#include "IO/Renaming/RenamingError.h"
#include "IO/Renaming/RenamingList.h"
#include "IO/KtmTransaction.h"
#include "Gui/ReportDlg.h"

using namespace Beroux::IO::Renaming;

CRenamingController::CRenamingController() :
	m_nErrorLevel(elALL)
{
}

CRenamingController::~CRenamingController()
{
}

bool CRenamingController::RenameFiles(const CFileList& flBefore, const CFileList& flAfter)
{
	// Initialize progress display.
	m_dlgProgress.SetTitle(IDS_PGRS_TITLE);

	// Create a renaming list.
	m_renamingList.reset(new CRenamingList(flBefore, flAfter));

	// Remove files that have the same name before and after.
	for (int i=0; i<m_renamingList->GetCount(); ++i)
		if ((*m_renamingList)[i].GetPathBefore() == (*m_renamingList)[i].GetPathAfter())
		{
			m_renamingList->RemoveRenamingOperation(i);
			--i;
		}

	// When there is no file to be renamed...
	if (m_renamingList->IsEmpty())
	{
		DisplayError(IDS_NO_FILENAME_CHANGES, elNotice);
		return true; // No file to rename.
	}

	// Initialize
	m_nPreviousStage = -1;	// No progress displayed yet.

	// Keep the number of files the user would like to rename.
	unsigned nFilesToRename = m_renamingList->GetCount();	// This value may change later, so we keep it.

	// Initialize the renaming.
	CMultithreadRenamingList multithreadRenamingList;
	Beroux::IO::CKtmTransaction ktm;

	// Define the callbacks for the renaming manager.
	m_renamingList->SetRenamedCallback(bind(&CRenamingController::OnRenamed, this, _1, _2));
	m_renamingList->SetRenameErrorCallback(bind(&CRenamingController::OnRenameError, this, _1));
	m_renamingList->SetProgressCallback(bind(&CRenamingController::OnProgress, this, _1, _2, _3));
	multithreadRenamingList.SetDoneCallback(bind(&CRenamingController::OnDone, this, _1));

	// Start renaming.
RestartRenaming:
	multithreadRenamingList.Start(*m_renamingList, ktm);

	/* FIXME: On canceling the thread operation must be stopped.
	 * m_dlgProgress.EnableCancel(m_nCurrentStage != CRenamingList::stageRenaming);
	 * or... if KTM supports transactions.
	 */
	if (m_dlgProgress.DoModal() == IDCANCEL)
	{
		ktm.RollBack();
		return false;
	}

	// Wait for the working thread to terminate.
	multithreadRenamingList.WaitForTerminaison();

	// Where are we?
	switch (multithreadRenamingList.GetRenamingResult())
	{
	case CMultithreadRenamingList::resultCheckingFailed:
		// Alert the user.
		AfxMessageBox(IDS_REPORT_ERROR_DETECTED, MB_ICONINFORMATION);

		// Show the report dialog.
		if (CReportDlg(*m_renamingList).DoModal() == IDOK)
		{
			multithreadRenamingList.SetAllowWarnings(true);
			goto RestartRenaming;
		}
		else
			return false;	// Some errors are left and the user canceled.
		break;

	case CMultithreadRenamingList::resultRenamingFailed:
		// Ask the user what he/she wants to do.
		m_dlgRenameError.SetTransactionSupported(ktm.IsUsingKtm());
		switch (m_dlgRenameError.ShowDialog())
		{
		case CRenameErrorDlg::uaKeepCurrentState:
			if (ktm.Commit())
				return true;	// Just say it's been done as the user wanted.
			return false;

		case CRenameErrorDlg::uaReverseToPreviousState:
			ktm.RollBack();
			return false;

		default:
			ASSERT(false);
			return false;
		}
		break;

	case CMultithreadRenamingList::resultSuccess:
		if (ktm.Commit())
		{
			// Show message box.
			CString str;
			str.Format(IDS_RENAMED_ALL, nFilesToRename);
			DisplayError(str, elNotice);
			return true;
		}
		return false;

	default:
		ASSERT(false);
		return false;
	}
}

void CRenamingController::OnRenamed(const CPath& pathNameBefore, const CPath& pathNameAfter)
{
	// Renaming succeed
	m_dlgRenameError.Add(pathNameBefore, pathNameAfter, NULL);
}

void CRenamingController::OnRenameError(const IRenameError& renameError)
{
	if (typeid(renameError) == typeid(CRenamingError))
	{// Renaming error
		const CRenamingError& renamingError = static_cast<const CRenamingError&>(renameError);

		// Add that error.
		CString strErrorMessage;
		strErrorMessage.Format(_T("%d %s"), renamingError.GetErrorCode(), renamingError.GetErrorMessage());
		m_dlgRenameError.Add(
			renamingError.GetPathNameBefore(),
			renamingError.GetPathNameAfter(),
			strErrorMessage);
	}
	else if (typeid(renameError) == typeid(CDirectoryRemovalError))
	{// Directory removal error
		const CDirectoryRemovalError& dirRemovalError = static_cast<const CDirectoryRemovalError&>(renameError);

		// Add that error.
		// TODO: Test it.
		CString strErrorMessage;
		strErrorMessage.Format(_T("%d %s"), dirRemovalError.GetErrorCode(), dirRemovalError.GetErrorMessage());
		m_dlgRenameError.Add(
			dirRemovalError.GetDirectoryPath(),
			CPath(),
			strErrorMessage);
	}
	else
	{// Unknown error.
		ASSERT(false);
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

void CRenamingController::OnDone(CMultithreadRenamingList::ERenamingResult nRenamingResult)
{
	m_dlgProgress.Done();
}

void CRenamingController::DisplayError(UINT nMsgID, EErrorLevels nErrorLevel) const
{
	CString str;
	str.LoadString(nMsgID);
	DisplayError(str, nErrorLevel);
}

void CRenamingController::DisplayError(const CString& strErrorMsg, EErrorLevels nErrorLevel) const
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

