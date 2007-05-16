#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "../resource.h"

/** CProgressDlg dialog: Display a progression.
 * Use EnableCancel(true) before DoModal() to allow operation cancelling.
 * Other methods should be thread-safe: You can call them before, during or even after DoModal().
 *
 * Use SetTitle("My Progress") and SetCaption("Operation $(Done) of $(Total)...") to change
 * the title and the displayed text.
 *
 * During DoModal(), call the SetProgress() from another thread to update the current progress.
 *
 * Call Done() when the operation is complete, and the dialog will return IDOK.
 * If during the operation, if the user pressed the Cancel button (when enabled), it returns IDCANCEL.
 */
class CProgressDlg : public CDialog
{
	DECLARE_DYNAMIC(CProgressDlg)

public:
	CProgressDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CProgressDlg();

// Pre-display attributes
	// Change the dialog title.
	void SetTitle(const CString& strValue) {
		m_strDialogTitle = strValue;
	}
	void SetTitle(UINT nStringID) {
		m_strDialogTitle.LoadString(nStringID);
	}

	/** Change the progress caption.
	 * "$(Done)", "$(Total)", "$(Percents)" are replaced by their values.
	 */
	void SetCaption(const CString& strValue) {
		m_strProgressCaption = strValue;
	}
	void SetCaption(UINT nStringID) {
		m_strProgressCaption.LoadString(nStringID);
	}

	// Enable or disable the user cancelling possibility.
	void EnableCancel(bool bEnable=true) {
		m_bEnableCancel = bEnable;
	}

	void Done() {
		m_bDone = true;
	}

// Operations (can always be used)
	// Set the current progress.
	void SetProgress(unsigned nDone, unsigned nTotal) {
		m_nDone = nDone;

		ASSERT(nTotal > 0);
		m_nTotal = nTotal;
	}

// Dialog Data
private:
	enum { IDD = IDD_PROGRESS };

	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	// Update the current progress displayed from m_nDone and m_nTotal.
	void UpdateProgress();

	unsigned m_nTotal;
	unsigned m_nDone;
	bool m_bDone;

	CString m_strDialogTitle;
	CString m_strProgressCaption;
	bool m_bEnableCancel;
	CStatic m_ctlProgressStatic;
	CProgressCtrl m_ctlProgress;
public:
	virtual INT_PTR DoModal();
};
