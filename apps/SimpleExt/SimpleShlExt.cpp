// SimpleShlExt.cpp : Implementation of CSimpleShlExt
#include "stdafx.h"
#include "SimpleExt.h"
#include "SimpleShlExt.h"
#include <string>
#include <memory>		// Used for auto_ptr

#ifdef _UNICODE
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSimpleShlExt

CSimpleShlExt::CSimpleShlExt() : 
	m_szFiles(NULL)
{
	srand( GetTickCount() );
	m_hMenuBmp = ::LoadBitmap(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDB_MENU_BITMAP));
}


CSimpleShlExt::~CSimpleShlExt()
{
	if (m_szFiles != NULL)
		delete [] m_szFiles;
}


HRESULT CSimpleShlExt::Initialize (LPCITEMIDLIST pidlFolder,
								   LPDATAOBJECT pDataObj,
								   HKEY hProgID )
{
	LPTSTR		lpszBuffer;
	FORMATETC	fmt = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM	stg = { TYMED_HGLOBAL };
	HDROP		hDrop;
	UINT		uNumFiles,
				i;				

	// Look for CF_HDROP data in the data object.
	if (FAILED( pDataObj->GetData(&fmt, &stg) ))
		// Nope! Return an "invalid argument" error back to Explorer.
		return E_INVALIDARG;

	// Get a pointer to the actual data.
	hDrop = (HDROP) GlobalLock ( stg.hGlobal );

	// Make sure it worked.
	if (hDrop == NULL)
		return E_INVALIDARG;

	// Get the number of files selected
	// and make sure there is at least one file
	if ((uNumFiles = DragQueryFile( hDrop, 0xFFFFFFFF, NULL, 0 )) == 0)
	{
		GlobalUnlock( stg.hGlobal );
		ReleaseStgMedium( &stg );
		return E_INVALIDARG;
	}

	// Create buffer
	if (m_szFiles != NULL)
		delete [] m_szFiles;
	m_nFilesLength = 1;
	for (i=0; i<uNumFiles; ++i)
		m_nFilesLength += DragQueryFile(hDrop, i, NULL, 0) + 1;
	lpszBuffer = m_szFiles = new TCHAR[m_nFilesLength];
	if (m_szFiles == NULL)
	{
		GlobalUnlock( stg.hGlobal );
		ReleaseStgMedium( &stg );
		return E_INVALIDARG;
	}

	// Append all files into the buffer seperated by '\0'.
	for (i=0; i<uNumFiles; ++i)
	{
		lpszBuffer += DragQueryFile(hDrop, i, lpszBuffer, MAX_PATH);
		*lpszBuffer++ = _T('\0');
	}
	*lpszBuffer = _T('\0');	// End by a double '\0'.

	GlobalUnlock ( stg.hGlobal );
	ReleaseStgMedium ( &stg );

	return S_OK;
}

HRESULT CSimpleShlExt::QueryContextMenu( HMENU hmenu, UINT  uMenuIndex, 
										 UINT  uidFirstCmd, UINT  uidLastCmd,
										 UINT  uFlags )
{
	// If the flags include CMF_DEFAULTONLY then we shouldn't do anything.
	if ( uFlags & CMF_DEFAULTONLY )
		return MAKE_HRESULT ( SEVERITY_SUCCESS, FACILITY_NULL, 0 );

	InsertMenu(hmenu, uMenuIndex, MF_BYPOSITION, uidFirstCmd, _T("Rename-&It!") );

	// Set the bitmap for the register item.
	if (m_hMenuBmp != NULL)
		SetMenuItemBitmaps(hmenu, uMenuIndex, MF_BYPOSITION, m_hMenuBmp, NULL);

	return MAKE_HRESULT ( SEVERITY_SUCCESS, FACILITY_NULL, 1 );
}

HRESULT CSimpleShlExt::GetCommandString( UINT  idCmd,      UINT uFlags,
										 UINT* pwReserved, LPSTR pszName,
										 UINT  cchMax )
{
	USES_CONVERSION;

	// Check idCmd, it must be 0 since we have only one menu item.
	if (idCmd != 0)
		return E_INVALIDARG;

	// If Explorer is asking for a help string, copy our string into the
	// supplied buffer.
	if (uFlags & GCS_HELPTEXT)
	{
		TCHAR szBuffer[256];
		::LoadString(NULL, IDS_COMMAND_STRING, szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]));

		if (uFlags & GCS_UNICODE)
			// We need to cast pszName to a Unicode string, and then use the
			// Unicode string copy API.
			lstrcpynW( (LPWSTR) pszName, T2CW(szBuffer), cchMax );
		else
			// Use the ANSI string copy API to return the help string.
			lstrcpynA( pszName, T2CA(szBuffer), cchMax );

		return S_OK;
	}

	return E_INVALIDARG;
}

HRESULT CSimpleShlExt::InvokeCommand ( LPCMINVOKECOMMANDINFO pCmdInfo )
{
	STARTUPINFO startupInfo;
	PROCESS_INFORMATION	processInformation;
	TCHAR	szDir[MAX_PATH];
	HKEY	hKey;
	DWORD	dwSize;
	HANDLE	hMapFile,
			hDoneEvent,
			hWait[2];
	LPVOID	lpMapAddress;
	unsigned __int64 nMapSize = m_nFilesLength*sizeof(TCHAR);

	// If lpVerb really points to a string, ignore this function call and bail out.
	if ( 0 != HIWORD( pCmdInfo->lpVerb ))
		return E_INVALIDARG;

	// Get the command index - the only valid one is 0.
	if (LOWORD(pCmdInfo->lpVerb) != 0)
		return E_INVALIDARG;

	// Get RenameIt.exe directory
	if (::RegOpenKey(HKEY_LOCAL_MACHINE, _T("Software\\Rename-It!"), &hKey) != ERROR_SUCCESS)
	{
		::MessageBox(pCmdInfo->hwnd, _T("Unable to open registry key.\nPlease re-install the application."), _T("Rename-It!"), MB_ICONERROR);
		return S_OK;
	}
	dwSize = sizeof(szDir)/sizeof(szDir[0]);
	if (::RegQueryValueEx(hKey, _T("ExeDir"), NULL, NULL, (LPBYTE)szDir, &dwSize) != ERROR_SUCCESS)
	{
		::MessageBox(pCmdInfo->hwnd, _T("Unable to read registry key.\nPlease re-install the application."), _T("Rename-It!"), MB_ICONERROR);
		return S_OK;
	}
	::RegCloseKey(hKey);

	// Create file mapping
	tstring strMapFileName;
	strMapFileName.reserve(32);
	while (true)
	{
		// Random name
		strMapFileName = _T("RenIt-MapFileEvent-");
		for (int i=0; i<10; ++i)
			strMapFileName += _T("0123456789ABCDEF")[rand() % 16];

		// Create file mapping
		hMapFile = ::CreateFileMapping(
			INVALID_HANDLE_VALUE,			// Current file handle
			NULL,							// Default security
			PAGE_READWRITE,					// Read/write permission
			DWORD(nMapSize >> 32),			// Max. object size (high-order)
			DWORD(nMapSize),				// Max. object size (low-order)
			strMapFileName.c_str());		// Name of mapping object
		if (hMapFile == NULL)
		{
			::MessageBox(pCmdInfo->hwnd, _T("Unable to create file mapping."), _T("Rename-It!"), MB_ICONERROR);
			return S_OK;
		}
		if (::GetLastError() != ERROR_ALREADY_EXISTS)
			break;
		::CloseHandle(hMapFile);
	}
	// Copy data
	lpMapAddress = ::MapViewOfFile(
		hMapFile,							// Handle to mapping object
		FILE_MAP_ALL_ACCESS,				// Read/write permission
		0,									// Max. object size (high-order)
		0,									// Max. object size (low-order)
		nMapSize);							// Map entire file
	if (lpMapAddress == NULL)
	{
		::MessageBox(pCmdInfo->hwnd, _T("Could not map view of file."), _T("Rename-It!"), MB_ICONERROR);
		return S_OK;
	}
	memcpy(lpMapAddress, m_szFiles, nMapSize);

	// Create event
	tstring strEventName;
	strEventName.reserve(32);
	while (true)
	{
		// Random name
		strEventName = _T("RenIt-DoneEvent-");
		for (int i=0; i<10; ++i)
			strEventName += _T("0123456789ABCDEF")[rand() % 16];

		// Create event
		if ((hDoneEvent=::CreateEvent(NULL, TRUE, FALSE, strEventName.c_str())) == NULL)
		{
			::MessageBox(pCmdInfo->hwnd, _T("Unable to create event."), _T("Rename-It!"), MB_ICONERROR);
			return S_OK;
		}
		if (::GetLastError() != ERROR_ALREADY_EXISTS)
			break;
		::CloseHandle(hDoneEvent);
	}

	// Parameters
	tstring strApplication;
	strApplication.reserve(MAX_PATH);
	strApplication = _T("\"");
	strApplication += szDir;
	strApplication += _T("\\RenameIt.exe\"");

	TCHAR szMaxSize[32];
	_stprintf_s(szMaxSize, _T("%I64u"), nMapSize);

	tstring strCommandLine;
	strCommandLine.reserve(MAX_PATH);
	strCommandLine += strApplication;
	strCommandLine += _T(" /$shell$ext$ ");
	strCommandLine += strMapFileName;
	strCommandLine += _T(":");
	strCommandLine += szMaxSize;
	strCommandLine += _T(":");
	strCommandLine += strEventName;

	// Lauch the program
	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);

	std::auto_ptr<TCHAR> szCommandLine(new TCHAR[strCommandLine.length() + 1]);
	_tcscpy(szCommandLine.get(), strCommandLine.c_str());

	if (!::CreateProcess(NULL, szCommandLine.get(), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInformation))
	{
		LPTSTR lpMsgBuf = NULL;
		DWORD dwError = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dwError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL );

		if (lpMsgBuf)
		{
			// Display the error.
			MyFormatMessage(IDS_CREATE_RENAMEIT_PROCESS_ERROR, strCommandLine.c_str(), lpMsgBuf);

			// Free the buffer.
			LocalFree(lpMsgBuf);
		}
		else
			::MessageBox(NULL, _T("Critical error."), _T("Rename-It!"), MB_ICONSTOP);

		return S_OK;
	}

	// Wait
	hWait[0] = processInformation.hProcess;
	hWait[1] = hDoneEvent;
	::WaitForMultipleObjects(sizeof(hWait)/sizeof(hWait[0]), hWait, FALSE, INFINITE);
	::CloseHandle(processInformation.hThread);
	::CloseHandle(processInformation.hProcess);
	UnmapViewOfFile(lpMapAddress);
	return S_OK;
}

void CSimpleShlExt::MyFormatMessage(UINT nResourceID, ...)
{
	va_list ap;
	va_start(ap, nResourceID);

	// Load the format.
	TCHAR lpStringBuf[1024];
	int nLength = ::LoadString((HINSTANCE)GetModuleHandle(NULL), nResourceID, lpStringBuf, sizeof(lpStringBuf)/sizeof(lpStringBuf[0]));

	// Format the message.
	LPTSTR lpMsgBuf = NULL;
	if (nLength > 0 &&
		::FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
		lpStringBuf,
		0,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		&ap))
	{
		// Display the formated message.
		::MessageBox(NULL, lpMsgBuf, _T("Rename-It!"), MB_ICONERROR);

		LocalFree(lpMsgBuf);
	}
	else
		::MessageBox(NULL, _T("Critical error!"), _T("Rename-It!"), MB_ICONSTOP);

	va_end(ap);
}
