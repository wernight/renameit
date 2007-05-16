#pragma once

#include "RenamingManager.h"
#include "ProgressDlg.h"
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
	void OnProgress(CRenamingManager::EStage nStage, int nDone, int nTotal);
	void DisplayError(UINT nMsgID, EErrorLevel nErrorLevel) const;
	void DisplayError(const CString& strErrorMsg, EErrorLevel nErrorLevel) const;

	CRenamingManager m_rmRenamingManager;
	CProgressDlg m_dlgProgress;
	CRenameErrorDlg m_dlgRenameError;
	unsigned m_nErrorLevel;
};
