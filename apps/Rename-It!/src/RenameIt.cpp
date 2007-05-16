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
// RenameIt.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "RenameIt.h"
#include "RenameItDlg.h"

#include <Shlwapi.h>	// Used for PathGetArgs() and PathIsDirectory()

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRenameItApp

BEGIN_MESSAGE_MAP(CRenameItApp, CWinApp)
	//{{AFX_MSG_MAP(CRenameItApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
	ON_COMMAND(ID_HELP_INDEX, &CWinApp::OnHelpIndex)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRenameItApp construction

CRenameItApp::CRenameItApp()
{
	EnableHtmlHelp();

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CRenameItApp object

CRenameItApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CRenameItApp initialization

BOOL CRenameItApp::InitInstance()
{
	// Call AfxInitRichEdit2() to initialize richedit2 library.
	AfxInitRichEdit2();

	// Show main dialog
    CRenameItDlg	dlg;
    m_pMainWnd = &dlg;
    dlg.DoModal();

    // Since the dialog has been closed, return FALSE so that we exit the
    //  application, rather than start the application's message pump.
    return FALSE;
}

// Show a generic error dialog box for some weird errors.
BOOL MsgBoxUnhandledError(LPCSTR szFile, UINT nLine)
{
	CString	strLine,
			strMsg;

	// Show error message
	strLine.Format(_T("%d"), nLine);
	AfxFormatString2(strMsg, IDS_GENERICERROR, _T(__FILE__), strLine);
	AfxMessageBox(strMsg, MB_ICONERROR);
	return TRUE;
}
