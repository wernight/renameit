/*
* Copyright (C) 2002 Markus Eriksson, marre@renameit.hypermart.net
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/
// RenameItDlg.h : header file
//

#pragma once
#include "../SizingDialog.h"
#include "../IO/Renaming/Filter/FilterContainer.h"
#include "NotAddedFilesDlg.h"
#include "Control/ExplorerListCtrl.h"
#include "Control/RenamePartSelectionCtrl.h"
#include "MemoryFileList.h"
#include "../IO/Renaming/Filter/PreviewFileList.h"

/////////////////////////////////////////////////////////////////////////////
// CRenameItDlg dialog

class CRenameItDlg : public CSizingDialog
{
	DECLARE_DYNAMIC(CRenameItDlg)

// Construction
public:
	CRenameItDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CRenameItDlg();

private:
// Dialog Data
	//{{AFX_DATA(CRenameItDlg)
	enum { IDD = IDD_RENAMEIT_DIALOG };

	Gui::CMemoryFileList m_flFiles;			// A list of files to rename.
	Gui::CMemoryFileList m_flDirectories;	// A list of folders to rename.
	bool m_bDialogInit;
	HICON m_hIcon;
	CButton	m_ctlButtonMoveUp;
	CButton	m_ctlButtonMoveDown;
	CListCtrl m_ctlListFilters;
	Beroux::IO::Renaming::Filter::CFilterContainer m_fcFilters;
	Gui::Control::CExplorerListCtrl m_ctlListFilenames;
	int m_nUpdatesFreeze;	// When >0 the UpdateFilelist() does nothing. See Push/PopFileListUpdateFreeze.
	CNotAddedFilesDlg m_dlgNotAddedFiles;
	CStatusBarCtrl m_statusBar;
	Gui::Control::CRenamePartSelectionCtrl m_ctrlRenamePart;
	CToolTipCtrl* m_pToolTip;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRenameItDlg)
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CRenameItDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonAddRenamer();
	afx_msg void OnBnClickedButtonAddfilter2();
	afx_msg void OnButtonFilterlist();
	afx_msg void OnHelpAbout();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSaveAs();
	afx_msg void OnFileExit();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnButtonClearfiles();
	afx_msg void OnButtonClearrule();
	afx_msg void OnButtonRemovefile();
	afx_msg void OnButtonRemoverule();
	afx_msg void OnButtonAddfile();
	afx_msg void OnKeydownRulesList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkRulesList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFileConfigure();
	afx_msg void OnButtonAddfolder();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnLvnInsertitemRulesList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnDeleteitemRulesList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnButtonMovedown();
	afx_msg void OnButtonMoveup();
	afx_msg void OnItemchangedRulesList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnViewAlwaysOnTop();
	afx_msg void OnLvnEndlabeleditFilenamesIn(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnOrderList(WPARAM wParam, LPARAM lParam);
	afx_msg void OnMovedItemFileNamesIn(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnFeedbackBugreport();
	afx_msg void OnFeedbackInformationrequest();
	afx_msg void OnFeedbackSuggestions();
	afx_msg void OnRenamePartSelectionChange(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLvnGetdispinfoFilenamesIn(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnOdfinditemFilenamesIn(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnKeydownFilenamesIn(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickFilenamesIn(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	BOOL OnContextMenuRules(CPoint point);
	BOOL OnContextMenuFilenames(CPoint point);

	/** Add a file to the renaming elements.
	 * You have to call Push/PopUpdatesFreeze before and after.
	 * @param filename			File name to add to the list.
	 * @param pstrErrorBuffer	String that will contain the error message returned (may be NULL).
	 * @return True is successful else copy in pstrErrorBuffer the error message.
	 */
	bool AddFile(const CString &filename);

	bool AddFolder(const CString& strPath);

	// Add files in the folder and subfolders if the flag is set
	void AddFilesInFolder(const CString &dirname, bool bSubfolders=true);

	void UpdateFilelist();
	
	void UpdateFilterlist();
	
	void SelectAllFiles();
	
	void UnselectAllFiles();
	
	void InverseFileSelection();
	
	bool AddFilter(Beroux::IO::Renaming::Filter::IFilter *pFilter);

	// Discard any call to UpdateFilelist() until the last PopUpdatesFreeze().
	void PushUpdatesFreeze() {
		++m_nUpdatesFreeze;
	}

	// Refresh the display (files list and filters list preview) when the last push is popped back.
	// Should be called each time m_vFilesNames or m_fcFilters are modified.
	void PopUpdatesFreeze() {
		--m_nUpdatesFreeze;
		ASSERT(m_nUpdatesFreeze >= 0);
		if (m_nUpdatesFreeze == 0)
		{
			UpdateFilterlist();
			UpdateFilelist();
		}
	}

	// Process the provided command line arguments
	bool ProcessCommandLine(LPCTSTR szArgs);
	void ProcessShellCommandLine(LPCTSTR szArgs);
	void ProcessUserCommandLine();

	BOOL EditRule(int nRuleItem);
	bool LoadFilterList(LPCTSTR szFileName);
	bool SaveFilterList();
	void OnUpdateStatusBar();
	bool IsFilterSelected(UINT idxItem) {
		return (m_ctlListFilters.GetItemState( idxItem, LVIS_SELECTED) == LVIS_SELECTED);
	}
	void SwapFilterItems( UINT itemIdx1, UINT itemIdx2);
	void UpdateMoveButtonState();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnContextHelp();
	bool RenameAllFiles(unsigned nRenamingControllerErrorLevel=0xFFFF);
	void ToggleCheckBox(int nItem);

	/**
	 * Create an object that can preview the effect of a new filter
	 * on any one of the checked files.
	 * @param nFilterIndex	Index of the filter in the filter container of the new filter to preview between [0, count].
	 */
	Beroux::IO::Renaming::Filter::IPreviewFileList* GetPreviewSamples(int nFilterIndex);

	// Which list to use: Files or Directories?
	Gui::CMemoryFileList& GetDisplayedList();
};
