#pragma once
#include "RenamingList.h"

namespace Beroux{ namespace IO{ namespace Renaming
{
	/**
	 * Do the renaming of a CRenamingList in a worker thread.
	 *
	 * Example:
	 * \code
	 * CKtmTransaction ktm;
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
	 *
	 * \warning All callbacks from this class or from CRenamingList are from
	 *          the worker thread, NOT the calling thread.
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


		/**
		 * A callback function called after the renaming is done,
		 * meaning when the worker thread finished working.
		 * @param[in] nRenamingResult	The result of the renaming.
		 */
		typedef boost::function<void (ERenamingResult)> CDoneEventHandler;

	// Construction
		CMultithreadRenamingList();

	// Attributes
		/**
		 * Set if warnings errors should stop the renaming at checking stage.
		 * By default warning do stop the renaming.
		 */
		void SetAllowWarnings(bool value) {
			m_bAllowWarnings = value;
		}

		/**
		 * Tells if the worker thread has finished.
		 * Reports also true on failure and when nothing is started.
		 */
		bool IsDone() const;

		/**
		 * Result of the renaming, or current status.
		 */
		ERenamingResult GetRenamingResult() const;

		/**
		 * Callback when the renaming in done.
		 * \note Use boost::bind(&MyClass::MyCallBack, &myClassInstance, _1);
		 */
		void SetDoneCallback(const CDoneEventHandler& listener) {
			m_fOnDone = listener;
		}

	// Operations
		/**
		 * Perform the checking and renaming in a worker thread.
		 * If the checking failed, it stops there and GetRenamingResult()
		 * returns resultCheckingFailed.
		 * @param renamingList List of operations to perform.
		 * @param ktm The CKtmTransaction to be used by CRenamingList.
		 * @note Don't access the renamingList until the renaming is over.
		 */
		void Start(CRenamingList& renamingList, CKtmTransaction& ktm);

		/**
		 * Returns only after the worker thread has finished.
		 */
		void WaitForTerminaison();

	// Implementation
	private:
		struct CThreadArgs
		{
			CThreadArgs(CRenamingList& renamingList, CKtmTransaction& ktm, CDoneEventHandler& fOnDone, bool bAllowWarnings)
				: m_renamingList(renamingList)
				, m_ktm(ktm)
				, m_fOnDone(fOnDone)
				, m_bAllowWarnings(bAllowWarnings)
			{}

			CRenamingList& m_renamingList;
			CKtmTransaction& m_ktm;
			CDoneEventHandler& m_fOnDone;
			bool m_bAllowWarnings;
		};

		static UINT RenamingThread(LPVOID lpParam);

		static ERenamingResult CheckAndRename(CRenamingList& renamingList, CKtmTransaction& ktm, bool bAllowWarnings);

		boost::shared_ptr<CWinThread> m_pWinThread;
		CDoneEventHandler m_fOnDone;
		bool m_bAllowWarnings;
	};
}}}
