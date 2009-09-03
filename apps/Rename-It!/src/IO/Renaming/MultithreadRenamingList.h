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
			resultCancelled,
		};


		/**
		 * A callback function called after the renaming is done,
		 * meaning when the worker thread finished working.
		 * @param[in] nRenamingResult	The result of the renaming.
		 */
		typedef signal<void (ERenamingResult)> CDoneSignal;
		
		CDoneSignal Done;

	// Construction
		CMultithreadRenamingList();

		~CMultithreadRenamingList();

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
		 * Stop the renaming or pre-renaming operation started.
		 * \param bCancelOnlyRollBackable	Return false if the renaming at the current stage cannot be rolled back.
		 * \return True if the operation was canceled, false if it cannot be canceled.
		 */
		void Cancel(bool bCancelOnlyRollBackable=false);

		/**
		 * Returns only after the worker thread has finished.
		 */
		void WaitForTerminaison();

	// Implementation
	private:
		struct CCancelMessage
		{
			// Set to true when the cancel message is set and ready to be processed
			// by the worker thread.
			bool m_bMessageSet;

			// Input argument, set to true to avoid canceling when cannot roll back.
			bool m_bCancelOnlyRollBackable;
		};

		struct CThreadArgs
		{
			CThreadArgs(CRenamingList& renamingList, CKtmTransaction& ktm, CDoneSignal& fnOnDone, bool bAllowWarnings, CCancelMessage& cancelMessage)
				: m_renamingList(renamingList)
				, m_ktm(ktm)
				, m_fnOnDone(fnOnDone)
				, m_bAllowWarnings(bAllowWarnings)
				, m_cancelMessage(cancelMessage)
			{}

			CRenamingList& m_renamingList;
			CKtmTransaction& m_ktm;
			CDoneSignal& m_fnOnDone;
			bool m_bAllowWarnings;
			CCancelMessage& m_cancelMessage;
		};

		static UINT RenamingThread(LPVOID lpParam);

		static ERenamingResult CheckAndRename(CRenamingList& renamingList, CKtmTransaction& ktm, bool bAllowWarnings, CCancelMessage& cancelMessage);

		shared_ptr<CWinThread> m_pWinThread;
		bool m_bAllowWarnings;
		CCancelMessage m_cancelMessage;
	};
}}}
