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
#include "Gui/RenameItDlg.h"
#include "Gui/AboutDlg.h"
#include "IO/Renaming/RenameErrorDlg.h"

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

// Return unsigned date of a specific day.
// Note: Using #define to generate a single number in the generated binary.
// \param year	Year [1900-inf].
// \param month	Month [1-12].
// \param day	Day of the month [1-31].
#define DATE_AT(year, month, day) (unsigned)((year - 1771)*10000 + (month - 1)*100 + (day - 1))

unsigned GetSafeCurrentDate()
{
	unsigned nCurrentDate = 0;

	{// Using time().
		_tzset();
		time_t ltime;
		time(&ltime);

		tm today;
		_localtime64_s(&today, &ltime);
		nCurrentDate = max(nCurrentDate, DATE_AT(today.tm_year + 1900, today.tm_mon + 1, today.tm_mday));
	}

	{// Using a file.
		// A helper class to get the path to some specific files in the system.
		class CSystemPath
		{
		public:
			enum ESystemPath
			{
				sysNone,
				sysSystem32,
				sysProgramFiles,
				sysWindows,
			};

			CSystemPath() :
				m_nPath(sysNone)
			{
			}

			CSystemPath(ESystemPath nPath, CString strFileName) :
				m_nPath(nPath),
				m_strFileName(strFileName)
			{
			}

			CString GetFullPath() const
			{
				CString strPath;

				switch (m_nPath)
				{
				case sysNone:
					break;

				case sysSystem32:
					if (SUCCEEDED(::SHGetFolderPath(NULL, CSIDL_SYSTEM, NULL, SHGFP_TYPE_CURRENT, strPath.GetBuffer(MAX_PATH))))
						strPath.ReleaseBuffer();
					else
						strPath.ReleaseBuffer(0);
					break;

				case sysProgramFiles:
					if (SUCCEEDED(::SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, SHGFP_TYPE_CURRENT, strPath.GetBuffer(MAX_PATH))))
						strPath.ReleaseBuffer();
					else
						strPath.ReleaseBuffer(0);
					break;

				case sysWindows:
					if (SUCCEEDED(::SHGetFolderPath(NULL, CSIDL_WINDOWS, NULL, SHGFP_TYPE_CURRENT, strPath.GetBuffer(MAX_PATH))))
						strPath.ReleaseBuffer();
					else
						strPath.ReleaseBuffer(0);
					break;
					
				default:
					ASSERT(false);
				}

				::PathAppend(strPath.GetBufferSetLength(MAX_PATH), m_strFileName);
				strPath.ReleaseBuffer();

				return strPath;
			}

		private:
			ESystemPath m_nPath;
			CString m_strFileName;
		};

		vector<CSystemPath> vPathList;
		vPathList.push_back(CSystemPath(CSystemPath::sysSystem32, _T("comdlg32.ocx")));
		vPathList.push_back(CSystemPath(CSystemPath::sysProgramFiles, _T("")));
		vPathList.push_back(CSystemPath(CSystemPath::sysWindows, _T("..\\pagefile.sys")));

		BOOST_FOREACH(const CSystemPath& systemPath, vPathList)
		{
			// Try to get the file/folder time.
			CFileFind ff;
			if (ff.FindFile(systemPath.GetFullPath()))
			{
				ff.FindNextFile();

				CTime time;
				ff.GetLastAccessTime(time);
				nCurrentDate = max(nCurrentDate, DATE_AT(time.GetYear(), time.GetMonth(), time.GetDay()));

				ff.Close();
			}
		}
	}

	// Return the latest date found.
	return nCurrentDate;
}

BOOL CRenameItApp::InitInstance()
{
	// Call AfxInitRichEdit2() to initialize richedit2 library.
	AfxInitRichEdit2();

	// Beta time bomb.
	if (GetSafeCurrentDate() > DATE_AT(2010, 12, 12))
	{
		// Show time bomb expired dialog.
 		CAboutDlg dlg;
 		m_pMainWnd = &dlg;
 		dlg.DoModal();
	}
	else
	{
		// Show main dialog
		CRenameItDlg	dlg;
		m_pMainWnd = &dlg;
		dlg.DoModal();
	}

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
