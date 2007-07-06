#pragma once
#include "afxcmn.h"
#include "../resource.h"
#include "RenamingList.h"
#include "RenamingProgressDlg.h"

/**
 * CReportDlg dialog.
 * Rename and show the problems that appeared.
 */
class CReportDlg : public CDialog
{
	DECLARE_DYNAMIC(CReportDlg)

public:
	CReportDlg(CRenamingList& renamingList, CWnd* pParent = NULL);   // standard constructor
	virtual ~CReportDlg();

// Dialog Data
	enum { IDD = IDD_REPORT };

private:
	enum EIcon
	{
		iconRemoved = 0,
		iconConflict,
		iconInvalidName,
		iconBadName,
	};

	CRenamingList&	m_renamingList;
	CProgressDlg	m_dlgProgress;
	CListCtrl		m_ctlReportList;
	CImageList		m_ilImages;
	bool			m_bShowAllOperations;

	virtual BOOL OnInitDialog();
	afx_msg void OnLvnItemActivateReportList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnLvnKeydownReportList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedShowAllCheck();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	bool RenameItem(int nItem);
	BOOL FindErrors(void);
	void ShowErrors();
	void InsertOperation(int nRenamingOperationIndex);
	void UpdateStatus();
	static UINT CheckingThread(LPVOID lpParam);
	void OnProgress(CRenamingList::EStage nStage, int nDone, int nTotal);
	virtual void OnOK();
	void RemoveItem(int nIndex);
	bool RenameFiles();
	int FindIndexOf(int nRenamingOperation) const;
};
