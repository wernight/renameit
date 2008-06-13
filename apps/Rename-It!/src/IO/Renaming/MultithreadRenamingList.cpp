#include "StdAfx.h"
#include "MultithreadRenamingList.h"

namespace Beroux{ namespace IO{ namespace Renaming
{

	bool CMultithreadRenamingList::IsDone() const
	{
		return GetRenamingResult() != resultInProgress;
	}

	CMultithreadRenamingList::ERenamingResult CMultithreadRenamingList::GetRenamingResult() const
	{
		if (m_pWinThread.get() == NULL)
			return resultNotStarted;
		else
		{
			// Wait for the working thread to terminate.
			DWORD dwExitCode = 0;
			if (::GetExitCodeThread(m_pWinThread->m_hThread, &dwExitCode) != 0)
				return resultInProgress;
			if (dwExitCode == STILL_ACTIVE)
				return resultInProgress;
			return (ERenamingResult) dwExitCode;
		}
	}

	void CMultithreadRenamingList::Start(CRenamingList& renamingList)
	{
		// Start the thread.
		m_pWinThread.reset( AfxBeginThread(RenamingThread, &renamingList) );
		m_pWinThread->m_bAutoDelete = FALSE;
	}

	void CMultithreadRenamingList::WaitForTerminaison()
	{
		if (m_pWinThread.get() != NULL)
			::WaitForSingleObject(m_pWinThread->m_hThread, INFINITE);
	}

	UINT CMultithreadRenamingList::RenamingThread(LPVOID lpParam)
	{
		CRenamingList* renamingList = static_cast<CRenamingList*>(lpParam);
		bool bSuccess = true;	// Success is assumed at first.

		//////////////////////////////////////////////////////////////////////////////
		// Stage: Checking

		// Check if there are some errors.
		if (!renamingList->Check())
			return resultCheckingFailed;

		//////////////////////////////////////////////////////////////////////////////
		// Stage: Renaming

		// Do the renaming.
		Beroux::IO::KTMTransaction ktm;
		bSuccess &= renamingList->PerformRenaming(ktm);
		if (!bSuccess)
		{
			// TODO: Possibly commit or roll-back depending on the user's choice.
			if (MessageBox(NULL, _T("Errors.\nPress YES to commit or NO roll-back."), _T("Rename-It! Debug"), MB_YESNO) == IDYES)
				VERIFY(ktm.Commit());
			else
				VERIFY(ktm.RollBack());
			return resultRenamingFailed;
		}
		else
			if (!ktm.Commit())
				return resultRenamingFailed;

		return resultSuccess;
	}

}}}
