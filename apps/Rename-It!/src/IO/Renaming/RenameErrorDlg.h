#pragma once
#include "../../ResizingDialog.h"
#include "../../../resource.h"
#include "Path.h"
#include <afxcmn.h>
#include <afxwin.h>

namespace Beroux{ namespace IO{ namespace Renaming
{
	/** CRenameErrorDlg dialog
	 * Add the errors using Add() before calling DoModal().
	 * Once the method returns, use GetUserAction to retrieve the user's action.
	 */
	class CRenameErrorDlg : public CResizingDialog
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
		void Add(const CPath& fnBefore, const CPath& fnAfter, const CString& strError=_T("")) {
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

		void UpdateErrorList();
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		virtual BOOL OnInitDialog();
		virtual void OnCancel();
		virtual void OnOK();
		afx_msg void OnBnClickedButtonShowDetails();
		afx_msg void OnBnClickedButtonHideDetails();
		afx_msg void OnBnClickedShowOnlyProblemsCheck();
		afx_msg void OnSize(UINT nType, int cx, int cy);

		DECLARE_MESSAGE_MAP()

		struct CError
		{
			CError(const CPath& fnBefore, const CPath& fnAfter, const CString& strError) :
				fnBefore(fnBefore),
				fnAfter(fnAfter),
				strError(strError)
			{}

			CPath	fnBefore;
			CPath	fnAfter;
			CString		strError;
		};
		vector<CError>	m_vErrors;
		unsigned m_nErrorCount;

		CImageList m_ilImages;
		CListCtrl m_ctlReport;
		CStatic m_ctlDescriptionStatic;
		CButton m_ctlAction;
		EUserAction m_nAction;
		bool m_bDialogInitialized;
	};
}}}
