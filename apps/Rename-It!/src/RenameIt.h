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
// RenameIt.h : main header file for the RENAMEIT application
//

#if !defined(AFX_RENAMEIT_H__ECCA64B4_40F9_11D2_AD2F_00A0C9289E24__INCLUDED_)
#define AFX_RENAMEIT_H__ECCA64B4_40F9_11D2_AD2F_00A0C9289E24__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "../resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CRenameItApp:
// See RenameIt.cpp for the implementation of this class
//

class CRenameItApp : public CWinApp
{
public:
	CRenameItApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRenameItApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CRenameItApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	// Check if another instance is running and a command line is provided
	bool CheckMultipleInstances();
};


/////////////////////////////////////////////////////////////////////////////

// Show a generic error dialog box for some weird errors.
BOOL MsgBoxUnhandledError(LPCSTR szFile, UINT nLine);

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RENAMEIT_H__ECCA64B4_40F9_11D2_AD2F_00A0C9289E24__INCLUDED_)
