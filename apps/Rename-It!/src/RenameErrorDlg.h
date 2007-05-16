#pragma once
#include "afxcmn.h"
#include "../resource.h"
#include "FileName.h"
#include "afxwin.h"

/** CRenameErrorDlg dialog
 * Add the errors using Add() before calling DoModal().
 * Once the method returns, use GetUserAction to retrieve the user's action.
 */
class CRenameErrorDlg : public CDialog
{
	DECLARE_DYNAMIC(CRenameErrorDlg)

public:
	CRenameErrorDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRenameErrorDlg();

	/**
	 * Add an operation done.
	 * @param fnBefore	File name before renaming.
	 * @param fnAfter	File name after renaming (or supposed to be).
	 * @param strError	When it's empty, it means the operation was successful.
	 */
	void Add(const CFileName& fnBefore, const CFileName& fnAfter, const CString& strError=_T("")) {
		if (!strError.IsEmpty())
			++m_nErrorCount;
		m_vErrors.push_back( CError(fnBefore, fnAfter, strError) );
	}

	unsigned GetErrorCount() const {
		return m_nErrorCount;
	}

	enum EUserAction
	{
		uaKeepCurrentState,
		uaReverseToPreviousState
	};
	EUserAction GetUserAction() const {
		return m_nAction;
	}

// Dialog Data
private:
	enum { IDD = IDD_RENAME_ERROR };

	enum EIcon
	{
		iconOk = 0,
		iconError,
	};

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()

	struct CError
	{
		CError(const CFileName& fnBefore, const CFileName& fnAfter, const CString& strError) :
			fnBefore(fnBefore),
			fnAfter(fnAfter),
			strError(strError)
		{}

		CFileName	fnBefore;
		CFileName	fnAfter;
		CString		strError;
	};
	vector<CError>	m_vErrors;
	unsigned m_nErrorCount;

	CImageList m_ilImages;
	CListCtrl m_ctlReport;
	CStatic m_ctlDescriptionStatic;
	CButton m_ctlAction;
	EUserAction m_nAction;
};
