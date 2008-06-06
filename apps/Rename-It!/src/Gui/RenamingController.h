#pragma once
#include "RenamingProgressDlg.h"
#include "IO/Renaming/RenameErrorDlg.h"

namespace Beroux{ namespace IO{ namespace Renaming{
	class CRenamingList;
}}}

namespace Gui
{
	/**
	 * Controls the high-level renaming process.
	 */
	class CRenamingController
	{
	// Definitions
	public:
		enum EErrorLevels
		{
			elError		= 0x0001,
			elWarning	= 0x0002,
			elNotice	= 0x0004,
			elALL		= 0xFFFF,
		};

	// Construction
		CRenamingController();
		~CRenamingController();

	// Attributes
		void SetErrorLevel(unsigned nErrorLevelFlags) {
			m_nErrorLevel = nErrorLevelFlags;
		}

	// Operations
		/**
		 * Do the renaming and inform the user of the progress
		 * and of the problems.
		 * @return True if the renaming is successfully done.
		 */
		bool RenameFiles(const Beroux::IO::Renaming::CFileList& flBefore, const Beroux::IO::Renaming::CFileList& flAfter);

	// Implementation
	private:
		static UINT RenamingThread(LPVOID lpParam);

		void OnRenamed(const Beroux::IO::Renaming::CPath& pathNameBefore, const Beroux::IO::Renaming::CPath& pathNameAfter);
		void OnRenameError(const Beroux::IO::Renaming::IRenameError& renameError);
		void OnProgress(Beroux::IO::Renaming::CRenamingList::EStage nStage, int nDone, int nTotal);
		void DisplayError(UINT nMsgID, EErrorLevels nErrorLevel) const;
		void DisplayError(const CString& strErrorMsg, EErrorLevels nErrorLevel) const;

		shared_ptr<Beroux::IO::Renaming::CRenamingList> m_renamingList;
		Gui::CProgressDlg m_dlgProgress;
		Beroux::IO::Renaming::CRenameErrorDlg m_dlgRenameError;
		unsigned m_nErrorLevel;
		unsigned m_nFilesToRename;
		int m_nCurrentStage; // The current stage of the renaming process.
		int m_nPreviousStage; // The last stage for which the progress has been displayed.
	};
}
