#pragma once
#include "afxcmn.h"
#include "../resource.h"
#include "RenamingManager.h"
#include "ProgressDlg.h"

/**
 * CReportDlg dialog.
 * Rename and show the problems that appeared.
 */
class CReportDlg : public CDialog
{
	DECLARE_DYNAMIC(CReportDlg)

public:
	CReportDlg(CRenamingManager& renamingList, CWnd* pParent = NULL);   // standard constructor
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

	CRenamingManager&	m_renamingList;
	CListCtrl		m_ctlReportList;
	int				m_nErrors;
	int				m_nWarnings;
	vector<unsigned>	m_uvErrorFlag;	// TODO: Should be removed.
	CImageList		m_ilImages;
	CProgressDlg	m_dlgProgress;

	virtual BOOL OnInitDialog();
	afx_msg void OnLvnItemActivateReportList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnBnClickedRefreshButton();
	afx_msg void OnLvnKeydownReportList(NMHDR *pNMHDR, LRESULT *pResult);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	BOOL CreateReport();
	BOOL RenameItem(int nItem);
	BOOL FindErrors(void);
	static UINT CheckingThread(LPVOID lpParam);
	void UpdateStatus(void);
	virtual void OnOK();
	void RemoveItem(int nIndex);
	bool RenameFiles();
	void OnRenameError(int nIndex, DWORD dwErrorCode);
	void OnProgress(CRenamingManager::EStage nStage, int nDone, int nTotal);
};
