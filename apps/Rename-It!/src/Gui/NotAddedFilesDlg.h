#pragma once
#include "afxcmn.h"
#include "../../resource.h"

// Boîte de dialogue CNotAddedFilesDlg

class CNotAddedFilesDlg : public CDialog
{
	DECLARE_DYNAMIC(CNotAddedFilesDlg)

public:
	CNotAddedFilesDlg(CWnd* pParent = NULL);   // constructeur standard
	virtual ~CNotAddedFilesDlg();
	virtual BOOL OnInitDialog();

	// Add a file's name and error message to the list.
	bool AddFile(LPCTSTR szFileName, LPCTSTR szErrorMessage);
	bool AddFile(LPCTSTR szFileName, UINT nResourceString);

	// Return true if the dialog has file names and error messages.
	bool HasErrors();

	// Clear the file names and error messages list.
	void ClearList();

// Données de boîte de dialogue
	enum { IDD = IDD_NOT_ADDED_FILES };
	CListCtrl m_ctlFilesList;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Prise en charge DDX/DDV

	DECLARE_MESSAGE_MAP()

	CStringList m_slFileNames,
				m_slErrorMessages;
};
