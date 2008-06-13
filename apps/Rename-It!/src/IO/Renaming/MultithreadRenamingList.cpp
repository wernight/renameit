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
			if (!::GetExitCodeThread(m_pWinThread->m_hThread, &dwExitCode))
				return resultInProgress;
			if (dwExitCode == STILL_ACTIVE)
				return resultInProgress;
			return (ERenamingResult) dwExitCode;
		}
	}

	void CMultithreadRenamingList::Start(CRenamingList& renamingList, CKtmTransaction& ktm)
	{
		// Start the thread.
		m_pWinThread.reset( AfxBeginThread(RenamingThread, new CThreadArgs(renamingList, ktm, m_fOnDone)) );
		m_pWinThread->m_bAutoDelete = FALSE;
	}

	void CMultithreadRenamingList::WaitForTerminaison()
	{
		if (m_pWinThread.get() != NULL)
			::WaitForSingleObject(m_pWinThread->m_hThread, INFINITE);
	}

	UINT CMultithreadRenamingList::RenamingThread(LPVOID lpParam)
	{
		// Retrieve the thread arguments.
		scoped_ptr<CThreadArgs> threadArgs( static_cast<CThreadArgs*>(lpParam) );

		// Do the renaming.
		ERenamingResult result = CheckAndRename(threadArgs->m_renamingList, threadArgs->m_ktm);

		if (threadArgs->m_fOnDone)
			threadArgs->m_fOnDone(result);
		return result;
	}

	CMultithreadRenamingList::ERenamingResult CMultithreadRenamingList::CheckAndRename(CRenamingList& renamingList, CKtmTransaction& ktm)
	{
		// Check if there are some errors.
		if (!renamingList.Check())
			return resultCheckingFailed;

		// Do the renaming.
		if (!renamingList.PerformRenaming(ktm))
			return resultRenamingFailed;

// OLD:
//		if (!bSuccess)
// 		{
// 			// TODO: Possibly commit or roll-back depending on the user's choice.
// 			if (MessageBox(NULL, _T("Errors.\nPress YES to commit or NO roll-back."), _T("Rename-It! Debug"), MB_YESNO) == IDYES)
// 				VERIFY(ktm.Commit());
// 			else
// 				VERIFY(ktm.RollBack());
// 			return resultRenamingFailed;
// 		}
// 		else
// 			if (!ktm.Commit())
//				return resultRenamingFailed;

		return resultSuccess;
	}

}}}
