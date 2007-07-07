#pragma once
//////////////////////////////////////////////////////////////////////////
// #include "KTM.h"
//
// Copyright (C) 2007 Warren Stevens. All rights reserved.
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any
// damages arising from the use of this software.
// 
// Permission is granted to anyone to use this software for any
// purpose, including commercial applications, and to alter it and
// redistribute it, subject to the following restrictions:
// 
// 1. If you use this software in a product, an acknowledgment in the 
// product documentation, or about box, and an email letting me know 
// it is being used, would be appreciated, but is not required.
//
// 2. The origin of this software must not be misrepresented; you must
// not claim that you wrote the original software. Altered source versions 
// must be plainly marked as such, and must not be misrepresented as being 
// the original software.
//
// 3. Original, or altered, source versions may not be sold for 
// profit without the authors written consent.
// 
// 4. This notice may not be removed or altered from any source
// distribution.
//////////////////////////////////////////////////////////////////////////
// History
// (Y-M-D), version, comment
// 2007-03-07, Version 1.00, Initial release.
//////////////////////////////////////////////////////////////////////////



class KTMTransaction
{
public:
	// constructor typically called using default parameters - see MSDN help for details
	// set last parameter to false to disable the transaction functionality
	KTMTransaction(
		 LPSECURITY_ATTRIBUTES lpTransactionAttributes = NULL // see MSDN help
		,DWORD CreateOptions = 0                              // see MSDN help
		,DWORD Timeout       = NULL                           // NULL == infinite timeout
		,LPWSTR Description  = NULL                           // user-readable string
		,bool useTransactionsIfAvailable = true               // set to false to disable the transaction functionality
		);

	~KTMTransaction(); // causes rollback if you do not call Commit

	bool   RollBack();  // returns true for success, false for failure (call GetLastError on failure to get the reason)
	bool   Commit();    // returns true for success, false for failure (call GetLastError on failure to get the reason)
	HANDLE GetTransaction(); // handle to the current transaction, usually not needed (may be NULL, e.g. on Win XP)


	///////////////////////////////////////////////////////////////////////////////////////
	// NOTE: The transacted functions take the exact same parameters (in the same order) 
	//       as the regular Win32 functions of the same name.
	///////////////////////////////////////////////////////////////////////////////////////

	// File Functions
	BOOL   CopyFile(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, BOOL bFailIfExists);
	BOOL   CopyFileEx(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags);
	BOOL   CreateDirectoryEx(LPCTSTR lpTemplateDirectory, LPCTSTR lpNewDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
#if (_WIN32_WINNT >= 0x0500)
	BOOL   CreateHardLink(LPCTSTR lpFileName, LPCTSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
#endif // (_WIN32_WINNT >= 0x0500)
	HANDLE CreateFile(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
	BOOL   DeleteFile(LPCTSTR lpFileName);
	HANDLE FindFirstFileEx(LPCTSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPVOID lpFindFileData, FINDEX_SEARCH_OPS fSearchOp, LPVOID lpSearchFilter, DWORD dwAdditionalFlags);
	DWORD  GetCompressedFileSize(LPCTSTR lpFileName, LPDWORD lpFileSizeHigh);
	BOOL   GetFileAttributesEx(LPCTSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpFileInformation);
	DWORD  GetFullPathName(LPCTSTR lpFileName, DWORD nBufferLength, LPTSTR lpBuffer, LPTSTR* lpFilePart);
	DWORD  GetLongPathName(LPCTSTR lpszShortPath, LPTSTR lpszLongPath, DWORD cchBuffer);	
	BOOL   MoveFileEx(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, DWORD dwFlags);
#if (_WIN32_WINNT >= 0x0500)
	BOOL   MoveFileWithProgress(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags);
#endif // (_WIN32_WINNT >= 0x0500)
	BOOL   RemoveDirectory(LPCTSTR lpPathName);	
	BOOL   SetFileAttributes(LPCTSTR lpFileName, DWORD dwFileAttributes);

	// Registry Functions
	LONG RegCreateKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD Reserved, LPTSTR lpClass, DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition);
	LONG RegDeleteKey(HKEY hKey, LPCTSTR lpSubKey);
	LONG RegOpenKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);

protected:
	bool InitFunctions();
	bool UseTransactedFunctions();
	void FreeLib(HMODULE& libToFree);

	HANDLE  m_transaction;

	// DLLs we need to load dynamically
	HMODULE m_ktmw32_dll;
	HMODULE m_advapi32_dll;
	HMODULE m_kernel32_dll;
	
	//////////////////////////////////////////////////////////////////////////
	// typedefs to clean up the dynamic function calling code
	//////////////////////////////////////////////////////////////////////////

	// Transaction Control Functions (from Ktmw32.dll)
	typedef HANDLE (CALLBACK* FA_CreateTransaction)(LPSECURITY_ATTRIBUTES, LPGUID, DWORD, DWORD, DWORD, DWORD, LPWSTR);
	typedef BOOL   (CALLBACK* FA_RollbackTransaction)(HANDLE);
	typedef BOOL   (CALLBACK* FA_CommitTransaction)(HANDLE);
	
	// Registry functions (from Advapi32.dll)
	typedef LONG   (CALLBACK* FA_RegCreateKeyTransacted)(HKEY, LPCTSTR, DWORD, LPTSTR, DWORD, REGSAM, const LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD, HANDLE, PVOID);
	typedef LONG   (CALLBACK* FA_RegDeleteKeyTransacted)(HKEY, LPCTSTR, REGSAM, DWORD, HANDLE, PVOID);
	typedef LONG   (CALLBACK* FA_RegOpenKeyTransacted)(HKEY, LPCTSTR, DWORD, REGSAM, PHKEY, HANDLE, PVOID);
	
	// File functions (from Kernel32.dll)
	typedef BOOL   (CALLBACK* FA_DeleteFileTransacted)(LPCTSTR, HANDLE);
	typedef BOOL   (CALLBACK* FA_CopyFileTransacted)(LPCTSTR, LPCTSTR, LPPROGRESS_ROUTINE, LPVOID, LPBOOL, DWORD, HANDLE);
	typedef BOOL   (CALLBACK* FA_CreateDirectoryTransacted)(LPCTSTR, LPCTSTR, LPSECURITY_ATTRIBUTES, HANDLE);
	typedef BOOL   (CALLBACK* FA_MoveFileTransacted)(LPCTSTR, LPCTSTR, LPPROGRESS_ROUTINE, LPVOID, DWORD, HANDLE);
	typedef HANDLE (CALLBACK* FA_CreateFileTransacted)(LPCTSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE, HANDLE, PUSHORT, PVOID);
	typedef BOOL   (CALLBACK* FA_RemoveDirectoryTransacted)(LPCTSTR, HANDLE);	
	typedef DWORD  (CALLBACK* FA_GetFullPathNameTransacted)(LPCTSTR, DWORD, LPTSTR, LPTSTR* , HANDLE);
	typedef DWORD  (CALLBACK* FA_GetLongPathNameTransacted)(LPCTSTR, LPTSTR, DWORD, HANDLE);
	typedef BOOL   (CALLBACK* FA_CreateHardLinkTransacted)(LPCTSTR, LPCTSTR, LPSECURITY_ATTRIBUTES, HANDLE);
	typedef DWORD  (CALLBACK* FA_GetCompressedFileSizeTransacted)(LPCTSTR, LPDWORD, HANDLE);
	typedef DWORD  (CALLBACK* FA_GetFileAttributesTransacted)(LPCTSTR, GET_FILEEX_INFO_LEVELS, LPVOID, HANDLE);
	typedef BOOL   (CALLBACK* FA_SetFileAttributesTransacted)(LPCTSTR, DWORD, HANDLE);
	typedef HANDLE (CALLBACK* FA_FindFirstFileTransacted)(LPCTSTR, FINDEX_INFO_LEVELS, LPVOID, FINDEX_SEARCH_OPS, LPVOID, DWORD, HANDLE);

	//////////////////////////////////////////////////////////////////////////
	// Function Addresses (see InitFunctions() for initialization)
	//////////////////////////////////////////////////////////////////////////

	// Transaction Control Functions (from Ktmw32.dll)
	FA_CreateTransaction   m_ProcAddress_CreateTransaction;
	FA_RollbackTransaction m_ProcAddress_RollbackTransaction;
	FA_CommitTransaction   m_ProcAddress_CommitTransaction;
	
	// Registry functions (from Advapi32.dll)
	FA_RegCreateKeyTransacted m_ProcAddress_RegCreateKeyTransacted;
	FA_RegDeleteKeyTransacted m_ProcAddress_RegDeleteKeyTransacted;
	FA_RegOpenKeyTransacted   m_ProcAddress_RegOpenKeyTransacted;	
	
	// File functions (from Kernel32.dll)
	FA_DeleteFileTransacted            m_ProcAddress_DeleteFileTransacted;
	FA_CopyFileTransacted              m_ProcAddress_CopyFileTransacted;
	FA_CreateDirectoryTransacted       m_ProcAddress_CreateDirectoryTransacted;
	FA_MoveFileTransacted              m_ProcAddress_MoveFileTransacted;
	FA_CreateFileTransacted            m_ProcAddress_CreateFileTransacted;
	FA_RemoveDirectoryTransacted       m_ProcAddress_RemoveDirectoryTransacted;
	FA_GetFullPathNameTransacted       m_ProcAddress_GetFullPathNameTransacted;
	FA_GetLongPathNameTransacted       m_ProcAddress_GetLongPathNameTransacted;
	FA_CreateHardLinkTransacted        m_ProcAddress_CreateHardLinkTransacted;	
	FA_GetCompressedFileSizeTransacted m_ProcAddress_GetCompressedFileSizeTransacted;
	FA_GetFileAttributesTransacted     m_ProcAddress_GetFileAttributesTransacted;
	FA_SetFileAttributesTransacted     m_ProcAddress_SetFileAttributesTransacted;
	FA_FindFirstFileTransacted         m_ProcAddress_FindFirstFileTransacted;
};