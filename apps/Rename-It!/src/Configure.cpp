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
// Configure.cpp : implementation file
//

#include "stdafx.h"
#include "RenameIt.h"
#include "Configure.h"

#include <shlobj.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigure dialog


CConfigure::CConfigure(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigure::IDD, pParent)
    , m_type(0)
	, m_strFiltersDirectory(_T(""))
	, m_bAutoAddRenamer(FALSE)
{
	HKEY	hKey;
	DWORD	dwSize,
			dwValue;

	// Read saved configuration
	if (RegOpenKey(HKEY_CURRENT_USER, _T("Software\\Rename-It!"), &hKey) == ERROR_SUCCESS)
	{
		// Load filters' directory
        if (RegQueryValueEx(hKey, _T("Filters"), 0, NULL, (BYTE*)m_strFiltersDirectory.GetBuffer(MAX_PATH), &(dwSize=MAX_PATH*sizeof(TCHAR))) == ERROR_SUCCESS)
			m_strFiltersDirectory.ReleaseBuffer(dwSize/sizeof(TCHAR));
		else
			m_strFiltersDirectory.ReleaseBuffer(0);

		// Load "Auto add renamer"
        if (RegQueryValueEx(hKey, _T("AutoAddRenamer"), 0, NULL, (BYTE*)&dwValue, &(dwSize=sizeof(DWORD))) == ERROR_SUCCESS)
 			m_bAutoAddRenamer = dwValue != 0;

		// Load default renamed part
        if (RegQueryValueEx(hKey, _T("DefaultRenamePart"), 0, NULL, (BYTE*)&dwValue, &(dwSize=sizeof(DWORD))) == ERROR_SUCCESS)
			m_type = dwValue;

        RegCloseKey(hKey);
	}
}


void CConfigure::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIRECTORY, m_strFiltersDirectory);
	DDX_Check(pDX, IDC_AUTO_ADD_RENAMER_CHECK, m_bAutoAddRenamer);
}

BEGIN_MESSAGE_MAP(CConfigure, CDialog)
	//{{AFX_MSG_MAP(CConfigure)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigure message handlers

void CConfigure::OnBrowse() 
{
    TCHAR path[MAX_PATH];
    LPMALLOC pMalloc;    // Gets the Shell's default allocator

    if (::SHGetMalloc(&pMalloc) == NOERROR)
	{
        BROWSEINFO browse;
        browse.hwndOwner = GetSafeHwnd();
        browse.pidlRoot = NULL;
        browse.pszDisplayName = path;
		CString strTitle;
		strTitle.LoadString(IDS_SCAN_FOR_PLUGINS);
        browse.lpszTitle = (LPCTSTR) strTitle;
        browse.ulFlags = BIF_RETURNONLYFSDIRS;
        browse.lpfn = NULL;

        LPITEMIDLIST pidl = SHBrowseForFolder(&browse);

        if (pidl != NULL)
		{
            if(SHGetPathFromIDList( pidl, path )) {
                m_strFiltersDirectory = path;
                UpdateData(FALSE);
            }
            pMalloc->Free(pidl);
        }
    }
    pMalloc->Release();
}

void CConfigure::OnOK()
{
	if (!UpdateData(TRUE))
		return;

	// Save configuration
	if (SaveConfig())
		CDialog::OnOK();
	else
		AfxMessageBox(IDS_CANTSAVE, MB_OK, NULL);
}

// Save configuration
bool CConfigure::SaveConfig(void)
{
	HKEY	hKey;

	// Open key
	if (RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Rename-It!"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) != ERROR_SUCCESS)
		return false;

	// Save m_strFiltersDirectory
	RegSetValueEx(hKey, _T("Filters"), 0, REG_SZ, (BYTE*)(LPCTSTR)m_strFiltersDirectory, (m_strFiltersDirectory.GetLength()+1)*sizeof(TCHAR));

	// Save "Auto add renamer"
 	DWORD	dwBool = m_bAutoAddRenamer;
	RegSetValueEx(hKey, _T("AutoAddRenamer"), 0, REG_DWORD, (BYTE*)&dwBool, sizeof(DWORD));

	// Save m_type
	DWORD	dwType = m_type;
	RegSetValueEx(hKey, _T("DefaultRenamePart"), 0, REG_DWORD, (BYTE*)&dwType, sizeof(DWORD));

	// Close key
	RegCloseKey(hKey);
	return true;
}

// Return path to the nIndex's recent file opened.
int CConfigure::GetRecentFilter(int nIndex, CString &strPath)
{
	CString	strKeyName;
	DWORD	dwSize;
	HKEY	hKey;

	// Open key
	if (RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Rename-It!\\Recent File List"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) != ERROR_SUCCESS)
		return 0;
	strKeyName.Format(_T("File%d"), nIndex);
    RegQueryValueEx(hKey, strKeyName, 0, NULL, (BYTE*)strPath.GetBuffer(MAX_PATH), &(dwSize=MAX_PATH*sizeof(TCHAR)));
	strPath.ReleaseBuffer(dwSize/sizeof(TCHAR));
    RegCloseKey(hKey);

	return strPath.GetLength();
}

// Place the given path on top of the recent filter files
bool CConfigure::AddRecentFilter(const CString& strPath)
{
	CString	strKeyName,
			strRecentPath;
	DWORD	dwSize;
	HKEY	hKey;
	int		i;

	// Check if file is already in the recent list
	if (RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Rename-It!\\Recent File List"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) != ERROR_SUCCESS)
		return false;
	for (i=1; i<5; ++i)		// Only 5 recent files
	{
		// Get recent path
		strKeyName.Format(_T("File%d"), i);
		if (RegQueryValueEx(hKey, strKeyName, 0, NULL, (BYTE*)strRecentPath.GetBuffer(MAX_PATH), &(dwSize=MAX_PATH*sizeof(TCHAR))) != ERROR_SUCCESS
			|| dwSize < sizeof(TCHAR))
			break;
		strRecentPath.ReleaseBuffer(dwSize/sizeof(TCHAR));

		// Compare
		if (strPath.CompareNoCase(strRecentPath) == 0)
			break;
	}

	// Move down every entry up to current item
	for (; i>1; --i)
	{
		// Get recent path
		strKeyName.Format(_T("File%d"), i-1);
		RegQueryValueEx(hKey, strKeyName, 0, NULL, (BYTE*)strRecentPath.GetBuffer(MAX_PATH), &(dwSize=MAX_PATH*sizeof(TCHAR)));
		strRecentPath.ReleaseBuffer(dwSize/sizeof(TCHAR));

		// Set recent path
		strKeyName.Format(_T("File%d"), i);
		RegSetValueEx(hKey, strKeyName, 0, REG_SZ, (BYTE*)(LPCTSTR)strRecentPath, (strRecentPath.GetLength()+1)*sizeof(TCHAR));
	}

	// Set recent path
	strKeyName.Format(_T("File%d"), i);
	if (RegSetValueEx(hKey, strKeyName, 0, REG_SZ, (BYTE*)(LPCTSTR)strPath, (strPath.GetLength()+1)*sizeof(TCHAR)) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return false;
	}

	// Close
	RegCloseKey(hKey);
	return true;
}

// Return number of recent filter you may get using GetRecentFilter(...)
int CConfigure::GetRecentFilterCount(void)
{
	CString	strKeyName;
	DWORD	dwSize;
	HKEY	hKey;
	int		i;

	// Check if file is already in the recent list
	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Rename-It!\\Recent File List"), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
		return 0;
	for (i=1; true; ++i)
	{
		// Get recent path
		strKeyName.Format(_T("File%d"), i);
		if (RegQueryValueEx(hKey, strKeyName, 0, NULL, NULL, &dwSize) != ERROR_SUCCESS
			|| dwSize < 1)
			return i-1;
	}
}
