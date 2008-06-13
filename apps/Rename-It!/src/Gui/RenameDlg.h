#pragma once

#include "../IO/Renaming/Path.h"

/** CRenameDlg dialog
 * Ask to manually rename a file.
 */
class CRenameDlg : public CDialog
{
	DECLARE_DYNAMIC(CRenameDlg)

public:
	CRenameDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRenameDlg();

	void SetOriginalFileName(const CString& strFileName) {
		m_strOriginalFileName = strFileName;
	}

	void SetNewFileName(const CString& strFileName) {
		m_strNewFileName = strFileName;
	}

	CString GetNewFileName() const {
		return m_strNewFileName;
	}

// Dialog Data
protected:
	enum { IDD = IDD_RENAME };

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();

	CString m_strOriginalFileName;
	CString m_strNewFileName;
};
