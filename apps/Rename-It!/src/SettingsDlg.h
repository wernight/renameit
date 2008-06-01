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
#pragma once
#include "afxwin.h"
#include "../resource.h"

namespace Gui
{
	/////////////////////////////////////////////////////////////////////////////
	// CConfigure dialog

	class CSettingsDlg : public CDialog
	{
	// Construction
	public:
		CSettingsDlg(CWnd* pParent = NULL);   // standard constructor

	// Attributes
		void SetPath(CString &path) { m_strFiltersDirectory = path; }
		CString GetPath() const { return m_strFiltersDirectory; }

		void SetType(int type) { m_type = type; }
		int GetType() const { return m_type; }

		BOOL AutoAddRenamer() const { return m_bAutoAddRenamer; }

	// Operations
		// Save configuration
		bool SaveConfig(void);

		// Return path to the nIndex's recent file opened.
		int GetRecentFilter(int nIndex, CString &strPath);

		// Place the given path on top of the recent filter files
		bool AddRecentFilter(const CString& strPath);

		// Return number of recent filter you may get using GetRecentFilter(...)
		int GetRecentFilterCount(void);

	// Dialog Data
	protected:
		enum { IDD = IDD_CONFIGURE };

		CString	m_strFiltersDirectory;
		int		m_type;
		BOOL	m_bAutoAddRenamer;

	// Overrides
	protected:
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CConfigure)
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	// Implementation
	protected:
		// Generated message map functions
		//{{AFX_MSG(CConfigure)
		afx_msg void OnBrowse();
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
		virtual void OnOK();
	};
}
