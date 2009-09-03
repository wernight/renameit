/* 
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
#pragma once

// ReplaceFilterDlg.h : header file
//

#include "../../../MenuButton.h"
#include "../../../resource.h"
#include "Gui/Control/InviteRichEdit.h"
#include "IO/Renaming/Filter/SearchReplaceFilter.h"
#include "SyntaxColor/SyntaxColor.h"
#include <afxcmn.h>
#include <afxwin.h>

namespace Beroux{ namespace IO{ namespace Renaming{ namespace Filter
{
	/////////////////////////////////////////////////////////////////////////////
	// CConfigDlg dialog
	class CSearchReplaceDlg : public CDialog
	{
	// Construction
	public:
		CSearchReplaceDlg(CSearchReplaceFilter& filter, IPreviewFileList& previewSamples, CWnd* pParent = NULL);   // standard constructor
		~CSearchReplaceDlg();

	// Dialog Data
		//{{AFX_DATA(CConfigDlg)
		enum { IDD = IDD_SEARCH_REPLACE_DIALOG };
		bool m_bInitialized;
		HICON m_hIcon;
		CSearchReplaceFilter& m_filter;
		Gui::Control::CInviteRichEdit m_ctlSearchRichEdit;
		Gui::Control::CInviteRichEdit m_ctlReplaceRichEdit;
		CMenuButton	m_ctlSearchButton;
		CMenuButton	m_ctlReplaceButton;
		CString	m_strBefore;
		CString	m_strReplace;
		CString	m_strSearch;
		BOOL	m_bCaseSensitive;
		CComboBox m_ctrlLocaleCombo;
		BOOL	m_bReplaceOnce;
		BOOL	m_bMatchWholeText;
		BOOL	m_bUse;
		int		m_nUse;
		BOOL	m_bChangeCase;
		int		m_nChangeCase;
		BOOL	m_bID3Tag;
		BOOL	m_bSeries;
		int		m_nSerieStart;
		int		m_nSerieStep;
		CSpinButtonCtrl m_ctrlSeriesStartSpin;
		CSpinButtonCtrl m_ctrlSeriesStepSpin;
		//}}AFX_DATA

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CConfigDlg)
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	protected:
		enum ESyntaxColor
		{
			scRegExpSearch,
			scRegExpReplace,
			scWildcardsSearch,
			scWildcardsReplace,
			scID3Tag,
			scGeneralMacros,
			scEnumeration,
			scCount
		};

		/**
		 * Discard any call to UpdateFilelist() until the last PopUpdatesFreeze().
		 */
		void PushUpdatesFreeze();

		/**
		 * Refresh the display (files list and filters list preview)
		 * when the last push is poped back.
		 * Should be called each time m_vFilesNames or m_fcFilters are modified.
		 */
		void PopUpdatesFreeze();

		void ColorSearchText();

		void ColorReplacementText();

		void UpdateSample();

	//	CPath m_fnOriginalFileName;
	//	CString m_strOriginalRenamedPart;
		IPreviewFileList& m_previewSamples;
		CToolTipCtrl* m_pToolTip;
		vector<SyntaxColor::CSyntaxColor*> m_vSyntaxColor;
		int m_nUpdatesFreeze;

		// Generated message map functions
		//{{AFX_MSG(CConfigDlg)
		afx_msg void OnEnChangeSearch();
		afx_msg void OnEnChangeReplace();
		afx_msg void OnChangeBefore();
		afx_msg void OnSearchButton();
		afx_msg void OnReplaceButton();
		afx_msg void OnMatchcase();
		afx_msg void OnBnClickedReplaceOnceCheck();
		virtual BOOL OnInitDialog();
		afx_msg void OnBnClickedUseCheck();
		afx_msg void OnCbnSelchangeUseCombo();
		afx_msg void OnBnClickedCaseCheck();
		afx_msg void OnCbnSelchangeCaseCombo();
		afx_msg void OnCbnEditchangeLocaleCombo();
		afx_msg void OnCbnSelchangeLocaleCombo();
		afx_msg void OnBnClickedSeriesButton();
		afx_msg void OnChangeSeries();
		afx_msg void OnBnClickedID3TagCheck();
		afx_msg void OnMatchWholeText();
		virtual BOOL PreTranslateMessage(MSG* pMsg);
		afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
		virtual void HtmlHelp(DWORD_PTR dwData, UINT nCmd = 0x000F);
		virtual HRESULT get_accHelpTopic(BSTR *pszHelpFile, VARIANT varChild, long *pidTopic);
		afx_msg void OnDestroy();
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
	};
}}}}
