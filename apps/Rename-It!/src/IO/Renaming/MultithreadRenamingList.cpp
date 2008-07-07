#include "StdAfx.h"
#include "MultithreadRenamingList.h"

namespace Beroux{ namespace IO{ namespace Renaming
{
	CMultithreadRenamingList::CMultithreadRenamingList()
		: m_bAllowWarnings(false)
	{
	}

	CMultithreadRenamingList::~CMultithreadRenamingList()
	{
		WaitForTerminaison();
	}

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
		// Reset the cancel message.
		m_cancelMessage.m_bMessageSet = false;

		// Start the thread.
		m_pWinThread.reset( AfxBeginThread(RenamingThread,
			new CThreadArgs(
				renamingList,
				ktm,
				Done,
				m_bAllowWarnings,
				m_cancelMessage)
			) );
		m_pWinThread->m_bAutoDelete = FALSE;
	}

	void CMultithreadRenamingList::Cancel(bool bCancelOnlyRollBackable)
	{
		// Set the message
		m_cancelMessage.m_bCancelOnlyRollBackable = bCancelOnlyRollBackable;
		m_cancelMessage.m_bMessageSet = true;
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

		// Wait a bit to let the main thread initialize.
		::Sleep(100);

		// Do the renaming.
		ERenamingResult result = CheckAndRename(threadArgs->m_renamingList, threadArgs->m_ktm, threadArgs->m_bAllowWarnings, threadArgs->m_cancelMessage);

		threadArgs->m_fnOnDone(result);
		return result;
	}

	CMultithreadRenamingList::ERenamingResult CMultithreadRenamingList::CheckAndRename(CRenamingList& renamingList, CKtmTransaction& ktm, bool bAllowWarnings, CCancelMessage& cancelMessage)
	{
		// Check if there are some errors.
		renamingList.Check();
		if (renamingList.GetErrorCount() > 0 ||
			(!bAllowWarnings && renamingList.GetWarningCount() > 0))
			return resultCheckingFailed;

		// Cancel?
		if (cancelMessage.m_bMessageSet)
			return resultCancelled;

		// Do the renaming.
		if (!renamingList.PerformRenaming(ktm))
			return resultRenamingFailed;

		// Cancel?
		if (cancelMessage.m_bMessageSet)
		{
			if (!cancelMessage.m_bCancelOnlyRollBackable || ktm.IsUsingKtm())
				return resultCancelled;
		}

		return resultSuccess;
	}
}}}
