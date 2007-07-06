#pragma once

#include "RenamingList.h"
#include "RenamingProgressDlg.h"
#include "RenameErrorDlg.h"

/**
 * Controls the high-level renaming process.
 */
class CRenamingController
{
// Definitions
public:
	enum EErrorLevel
	{
		elError		= 0x0001,
		elWarning	= 0x0002,
		elNotice	= 0x0004,
		elALL		= 0xFFFF,
	};

// Construction
	CRenamingController(void);
	~CRenamingController(void);

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
	bool RenameFiles(const CFileList& flBefore, const CFileList& flAfter);

// Implementation
private:
	static UINT RenamingThread(LPVOID lpParam);

	void OnRenamed(int nIndex, DWORD dwErrorCode);
	void OnProgress(CRenamingList::EStage nStage, int nDone, int nTotal);
	void DisplayError(UINT nMsgID, EErrorLevel nErrorLevel) const;
	void DisplayError(const CString& strErrorMsg, EErrorLevel nErrorLevel) const;

	CRenamingList m_renamingList;
	CProgressDlg m_dlgProgress;
	CRenameErrorDlg m_dlgRenameError;
	unsigned m_nErrorLevel;
	unsigned m_nFilesToRename;
	int m_nCurrentStage; // The current stage of the renaming process.
	int m_nPreviousStage; // The last stage for which the progress has been displayed.
};
