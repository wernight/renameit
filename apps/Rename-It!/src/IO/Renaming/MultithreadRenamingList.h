#pragma once
#include "RenamingList.h"

namespace Beroux{ namespace IO{ namespace Renaming
{
	/**
	 * Do the renaming of a CRenamingList in a worker thread.
	 *
	 * Example:
	 * \code
	 * CMultithreadRenamingList mrl;
	 * mrl.Start(my_renaming_list);
	 *
	 * while (!mrl.IsDone())
	 *   ; // Wait
	 *
	 * bool success = (mrl.GetRenamingResult() == CMultithreadRenamingList::resultSuccess);
	 * \endcode
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
		 * @note Don't access the renamingList until the renaming is over.
		 */
		void Start(CRenamingList& renamingList);

		/**
		 * Returns only after the worker thread has finished.
		 */
		void WaitForTerminaison();

	// Implementation
	private:
		struct CThreadArgs
		{
			CThreadArgs(CRenamingList& renamingList)
				: m_renamingList(renamingList)
			{}

			CRenamingList& m_renamingList;
		};

		static UINT RenamingThread(LPVOID lpParam);

		boost::shared_ptr<CWinThread> m_pWinThread;
	};
}}}
