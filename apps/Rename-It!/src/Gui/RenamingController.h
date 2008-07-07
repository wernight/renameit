#pragma once
#include "RenamingProgressDlg.h"
#include "IO/Renaming/MultithreadRenamingList.h"
#include "IO/Renaming/RenameErrorDlg.h"

namespace Beroux{ namespace IO{ namespace Renaming{
	class CRenamingList;
}}}

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

	void OnRenamingIOOperationPerformed(const Beroux::IO::Renaming::CRenamingList& sender, const Beroux::IO::Renaming::IOOperation::CIOOperation& ioOperation, Beroux::IO::Renaming::IOOperation::CIOOperation::EErrorLevel nErrorLevel);
	void OnRenamingProgressChanged(const Beroux::IO::Renaming::CRenamingList& sender, Beroux::IO::Renaming::CRenamingList::EStage nStage, int nDone, int nTotal);
	void OnRenamingDone(Beroux::IO::Renaming::CMultithreadRenamingList::ERenamingResult nRenamingResult);
	void DisplayError(UINT nMsgID, EErrorLevels nErrorLevel) const;
	void DisplayError(const CString& strErrorMsg, EErrorLevels nErrorLevel) const;

	CProgressDlg m_dlgProgress;
	CRenameErrorDlg m_dlgRenameError;
	unsigned m_nErrorLevel;
	int m_nPreviousStage; // The last stage for which the progress has been displayed.
};
