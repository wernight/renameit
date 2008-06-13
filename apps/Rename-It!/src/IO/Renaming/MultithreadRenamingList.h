#pragma once
#include "RenamingList.h"

namespace Beroux{ namespace IO{ namespace Renaming
{
	/**
	 * Do the renaming of a CRenamingList in a worker thread.
	 *
	 * Example:
	 * \code
	 * KTMTransaction ktm;
	 * CMultithreadRenamingList mrl;
	 * mrl.Start(my_renaming_list, ktm);
	 *
	 * while (!mrl.IsDone())
	 *   ; // Wait
	 *
	 * bool success = (mrl.GetRenamingResult() == CMultithreadRenamingList::resultSuccess);
	 * success &= ktm.Commit();
	 * \endcode
	 *
	 * When the renaming failed during renaming ("resultRenamingFailed"),
	 * you have the choice to ktm.Commit() or ktm.RollBack(); where
	 * committing whould renaming (most probably successfully) all files,
	 * while the rolling back will avoid any change on the disk.
	 */
	class CMultithreadRenamingList
	{
	public:
	// Definitions
		enum ERenamingResult
		{
			resultNotStarted,
			resultInProgress,

			resultCheckingFailed,
			resultRenamingFailed,

			resultSuccess,
		};

	// Construction

	// Attributes
		/**
		 * Tells if the worker thread has finished.
		 * Reports also true on failure and when nothing is started.
		 */
		bool IsDone() const;

		/**
		 * Result of the renaming, or current status.
		 */
		ERenamingResult GetRenamingResult() const;

	// Operations
		/**
		 * Perform the checking and renaming in a worker thread.
		 * If the checking failed, it stops there and GetRenamingResult()
		 * returns resultCheckingFailed.
		 * @param renamingList List of operations to perform.
		 * @param ktm The KTMTransaction to be used by CRenamingList.
		 * @note Don't access the renamingList until the renaming is over.
		 */
		void Start(CRenamingList& renamingList, KTMTransaction& ktm);

		/**
		 * Returns only after the worker thread has finished.
		 */
		void WaitForTerminaison();

	// Implementation
	private:
		struct CThreadArgs
		{
			CThreadArgs(CRenamingList& renamingList, KTMTransaction& ktm)
				: m_renamingList(renamingList)
				, m_ktm(ktm)
			{}

			CRenamingList& m_renamingList;
			KTMTransaction& m_ktm;
		};

		static UINT RenamingThread(LPVOID lpParam);

		boost::shared_ptr<CWinThread> m_pWinThread;
	};
}}}
