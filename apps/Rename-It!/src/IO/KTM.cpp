//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "KTM.h"
//////////////////////////////////////////////////////////////////////////


namespace Beroux{ namespace IO
{

KTMTransaction::KTMTransaction(
	 LPSECURITY_ATTRIBUTES lpTransactionAttributes // = NULL
	,DWORD CreateOptions                           // = 0
	,DWORD Timeout                                 // = NULL
	,LPWSTR Description                            // = NULL
	,bool useTransactionsIfAvailable               // = true
)
: m_transaction(NULL)
, m_ktmw32_dll(NULL)
, m_advapi32_dll(NULL)
, m_kernel32_dll(NULL)
, m_ProcAddress_CreateTransaction(NULL)
, m_ProcAddress_RollbackTransaction(NULL)
, m_ProcAddress_CommitTransaction(NULL)
, m_ProcAddress_RegCreateKeyTransacted(NULL)
, m_ProcAddress_RegDeleteKeyTransacted(NULL)
, m_ProcAddress_RegOpenKeyTransacted(NULL)
, m_ProcAddress_DeleteFileTransacted(NULL)
, m_ProcAddress_CopyFileTransacted(NULL)
, m_ProcAddress_CreateDirectoryTransacted(NULL)
, m_ProcAddress_MoveFileTransacted(NULL)
, m_ProcAddress_CreateFileTransacted(NULL)
, m_ProcAddress_RemoveDirectoryTransacted(NULL)
, m_ProcAddress_GetFullPathNameTransacted(NULL)
, m_ProcAddress_GetLongPathNameTransacted(NULL)
, m_ProcAddress_CreateHardLinkTransacted(NULL)
, m_ProcAddress_GetCompressedFileSizeTransacted(NULL)
, m_ProcAddress_GetFileAttributesTransacted(NULL)
, m_ProcAddress_SetFileAttributesTransacted(NULL)
, m_ProcAddress_FindFirstFileTransacted(NULL)
{
	if(useTransactionsIfAvailable)
	{
		m_ktmw32_dll = ::LoadLibrary(_T("Ktmw32.dll")); // just try this one first (if this is not there - then we are not going transactions)
		if(NULL != m_ktmw32_dll)
		{
			m_advapi32_dll = ::LoadLibrary(_T("Advapi32.dll"));
			m_kernel32_dll = ::LoadLibrary(_T("Kernel32.dll"));

			if((NULL != m_advapi32_dll) && (NULL != m_kernel32_dll))
			{
				if(InitFunctions()) // make sure all functions load properly
				{
					m_transaction = m_ProcAddress_CreateTransaction(lpTransactionAttributes, 0, CreateOptions, 0, 0, Timeout, Description);
				}
			}
		}
	}
}

bool KTMTransaction::UseTransactedFunctions()
{
	// all libraries must load AND CreateTransaction in constructor must succeed
	return (NULL != m_transaction);
}

KTMTransaction::~KTMTransaction()
{
	// hitting destructor before a commit took place (e.g. exception ?) so we rollback
	if(NULL != m_transaction){
		RollBack();	
	}

	// free libraries (if need be)
	FreeLib(m_ktmw32_dll  );
	FreeLib(m_advapi32_dll);
	FreeLib(m_kernel32_dll);
}

void KTMTransaction::FreeLib(HMODULE& libToFree)
{
	if(NULL != libToFree) {
		::FreeLibrary(libToFree);
		libToFree = NULL;
	}
}

template< class T> // template used so we can avoid having to write out the cast type
bool GetCastProcAddress(HMODULE hModule, LPCSTR lpProcName, T& funcPtrToAssign) {	
	funcPtrToAssign = ( T ) ::GetProcAddress(hModule, lpProcName);
	return (NULL != funcPtrToAssign); // loaded okay
}

bool KTMTransaction::InitFunctions()
{
	//////////////////////////////////////////////////////////////////////////
	// UNICODE note:
	// function names to GetProcAddress are always ANSI strings 
	// (even for UNICODE builds) so we do not use the _T() wrapper
	//
	// We use the [FuncSuffix] macro to paste the proper suffix
	// For example, ["RegCreateKeyTransacted" FuncSuffix ]
	// preprocesses to ["RegCreateKeyTransactedA"] or ["RegCreateKeyTransactedW"]
	// because two quoted strings beside each other get glued together during compilation
	// (the downside is going to programmer-hell for using macros)
	//////////////////////////////////////////////////////////////////////////

#ifdef UNICODE
	#define FuncSuffix  "W"
#else
	#define FuncSuffix  "A"
#endif // UNICODE

	// Transaction Control Functions (from Ktmw32.dll) ***NOTE*** these functions do not have the W and A versions in the DLL
	if( ! GetCastProcAddress(m_ktmw32_dll  , "CreateTransaction"                         , m_ProcAddress_CreateTransaction              )) { return false; }
	if( ! GetCastProcAddress(m_ktmw32_dll  , "RollbackTransaction"                       , m_ProcAddress_RollbackTransaction            )) { return false; }
	if( ! GetCastProcAddress(m_ktmw32_dll  , "CommitTransaction"                         , m_ProcAddress_CommitTransaction              )) { return false; }

	// Registry functions (from Advapi32.dll)
	if( ! GetCastProcAddress(m_advapi32_dll, "RegCreateKeyTransacted"          FuncSuffix, m_ProcAddress_RegCreateKeyTransacted         )) { return false; }
	if( ! GetCastProcAddress(m_advapi32_dll, "RegDeleteKeyTransacted"          FuncSuffix, m_ProcAddress_RegDeleteKeyTransacted         )) { return false; }
	if( ! GetCastProcAddress(m_advapi32_dll, "RegOpenKeyTransacted"            FuncSuffix, m_ProcAddress_RegOpenKeyTransacted           )) { return false; }

	// File functions (from Kernel32.dll)
	if( ! GetCastProcAddress(m_kernel32_dll, "DeleteFileTransacted"            FuncSuffix, m_ProcAddress_DeleteFileTransacted           )) { return false; }
	if( ! GetCastProcAddress(m_kernel32_dll, "CopyFileTransacted"              FuncSuffix, m_ProcAddress_CopyFileTransacted             )) { return false; }
	if( ! GetCastProcAddress(m_kernel32_dll, "CreateDirectoryTransacted"       FuncSuffix, m_ProcAddress_CreateDirectoryTransacted      )) { return false; }
	if( ! GetCastProcAddress(m_kernel32_dll, "MoveFileTransacted"              FuncSuffix, m_ProcAddress_MoveFileTransacted             )) { return false; }
	if( ! GetCastProcAddress(m_kernel32_dll, "CreateFileTransacted"            FuncSuffix, m_ProcAddress_CreateFileTransacted           )) { return false; }
	if( ! GetCastProcAddress(m_kernel32_dll, "RemoveDirectoryTransacted"       FuncSuffix, m_ProcAddress_RemoveDirectoryTransacted      )) { return false; }
	if( ! GetCastProcAddress(m_kernel32_dll, "GetFullPathNameTransacted"       FuncSuffix, m_ProcAddress_GetFullPathNameTransacted      )) { return false; }
	if( ! GetCastProcAddress(m_kernel32_dll, "GetLongPathNameTransacted"       FuncSuffix, m_ProcAddress_GetLongPathNameTransacted      )) { return false; }
	if( ! GetCastProcAddress(m_kernel32_dll, "CreateHardLinkTransacted"        FuncSuffix, m_ProcAddress_CreateHardLinkTransacted       )) { return false; }
	if( ! GetCastProcAddress(m_kernel32_dll, "GetCompressedFileSizeTransacted" FuncSuffix, m_ProcAddress_GetCompressedFileSizeTransacted)) { return false; }
	if( ! GetCastProcAddress(m_kernel32_dll, "GetFileAttributesTransacted"     FuncSuffix, m_ProcAddress_GetFileAttributesTransacted    )) { return false; }
	if( ! GetCastProcAddress(m_kernel32_dll, "SetFileAttributesTransacted"     FuncSuffix, m_ProcAddress_SetFileAttributesTransacted    )) { return false; }
	if( ! GetCastProcAddress(m_kernel32_dll, "FindFirstFileTransacted"         FuncSuffix, m_ProcAddress_FindFirstFileTransacted        )) { return false; }

	return true; // okay
}

bool KTMTransaction::RollBack()
{
	if(UseTransactedFunctions()){
		const bool wasSuccess(0 != m_ProcAddress_RollbackTransaction(m_transaction) );
		m_transaction = NULL;
		return wasSuccess; // returns true for success (call GetLastError on failure to get the reason)
	}else{
		return true; // no transaction to fail
	}
}

bool KTMTransaction::Commit()
{
	if(UseTransactedFunctions()){
		const bool wasSuccess(0 != m_ProcAddress_CommitTransaction(m_transaction) );
		m_transaction = NULL;
		return wasSuccess; // returns true for success (call GetLastError on failure to get the reason)
	}else{
		return true; // no transaction to fail
	}
}

HANDLE KTMTransaction::GetTransaction()
{
	return m_transaction; // handle to the current transaction, usually not needed (may be NULL, e.g. on Win XP)
}

BOOL KTMTransaction::DeleteFile(LPCTSTR lpFileName)
{
	if(UseTransactedFunctions()){
		return m_ProcAddress_DeleteFileTransacted(lpFileName, m_transaction);
	}else{
		return ::DeleteFile(lpFileName);
	}
}

BOOL KTMTransaction::CopyFile(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, BOOL bFailIfExists){
	// Overload - just calls other function
	BOOL cancel(FALSE);
	return CopyFileEx(lpExistingFileName, lpNewFileName, NULL, NULL, &cancel, 
		bFailIfExists ? (COPY_FILE_FAIL_IF_EXISTS) : (0));
}
BOOL KTMTransaction::CopyFileEx(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, 
	LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags)
{
	if(UseTransactedFunctions()){
		return m_ProcAddress_CopyFileTransacted(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags, m_transaction);
	}else{
		return ::CopyFileEx(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
	}
}

BOOL KTMTransaction::CreateDirectoryEx(LPCTSTR lpTemplateDirectory, LPCTSTR lpNewDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	if(UseTransactedFunctions()){
		return m_ProcAddress_CreateDirectoryTransacted(lpTemplateDirectory, lpNewDirectory, lpSecurityAttributes, m_transaction);
	}else{
		return ::CreateDirectoryEx(lpTemplateDirectory, lpNewDirectory, lpSecurityAttributes);
	}
}

BOOL KTMTransaction::MoveFileEx(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, DWORD dwFlags){
	// Overload - just calls other function
	if (UseTransactedFunctions()){
		return m_ProcAddress_MoveFileTransacted(lpExistingFileName, lpNewFileName, NULL, NULL, dwFlags, m_transaction);
	}else{
		return ::MoveFileEx(lpExistingFileName, lpNewFileName, dwFlags);
	}
}
#if (_WIN32_WINNT >= 0x0500)
BOOL KTMTransaction::MoveFileWithProgress(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags)
{
	if(UseTransactedFunctions()){
		return m_ProcAddress_MoveFileTransacted(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags, m_transaction);
	}else{
		return ::MoveFileWithProgress(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags);
	}
}
#endif // (_WIN32_WINNT >= 0x0500)

HANDLE KTMTransaction::CreateFile(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, 
	DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	if(UseTransactedFunctions()){
		return m_ProcAddress_CreateFileTransacted(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile, m_transaction, 0, 0);
	}else{
		return ::CreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
}

BOOL KTMTransaction::RemoveDirectory(LPCTSTR lpPathName)
{
	if(UseTransactedFunctions()){
		return m_ProcAddress_RemoveDirectoryTransacted(lpPathName, m_transaction);
	}else{
		return ::RemoveDirectory(lpPathName);
	}
}

DWORD KTMTransaction::GetFullPathName(LPCTSTR lpFileName, DWORD nBufferLength, LPTSTR lpBuffer, LPTSTR* lpFilePart)
{
	if(UseTransactedFunctions()){
		return m_ProcAddress_GetFullPathNameTransacted(lpFileName, nBufferLength, lpBuffer, lpFilePart, m_transaction);
	}else{
		return ::GetFullPathName(lpFileName, nBufferLength, lpBuffer, lpFilePart);
	}
}

DWORD KTMTransaction::GetLongPathName(LPCTSTR lpszShortPath, LPTSTR lpszLongPath, DWORD cchBuffer)
{
	if(UseTransactedFunctions()){
		return m_ProcAddress_GetLongPathNameTransacted(lpszShortPath, lpszLongPath, cchBuffer, m_transaction);
	}else{
		return ::GetLongPathName(lpszShortPath, lpszLongPath, cchBuffer);
	}
}

#if (_WIN32_WINNT >= 0x0500)
BOOL KTMTransaction::CreateHardLink(LPCTSTR lpFileName, LPCTSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	if(UseTransactedFunctions()){
		return m_ProcAddress_CreateHardLinkTransacted(lpFileName, lpExistingFileName, lpSecurityAttributes, m_transaction);
	}else{
		return ::CreateHardLink(lpFileName, lpExistingFileName, lpSecurityAttributes);
	}
}
#endif // (_WIN32_WINNT >= 0x0500)

DWORD KTMTransaction::GetCompressedFileSize(LPCTSTR lpFileName, LPDWORD lpFileSizeHigh)
{
	if(UseTransactedFunctions()){
		return m_ProcAddress_GetCompressedFileSizeTransacted(lpFileName, lpFileSizeHigh, m_transaction);
	}else{
		return ::GetCompressedFileSize(lpFileName, lpFileSizeHigh);
	}
}

BOOL KTMTransaction::GetFileAttributesEx(LPCTSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpFileInformation)
{
	if(UseTransactedFunctions()){
		return m_ProcAddress_GetFileAttributesTransacted(lpFileName, fInfoLevelId, lpFileInformation, m_transaction);
	}else{
		return ::GetFileAttributesEx(lpFileName, fInfoLevelId, lpFileInformation);
	}
}

BOOL KTMTransaction::SetFileAttributes(LPCTSTR lpFileName, DWORD dwFileAttributes)
{
	if(UseTransactedFunctions()){
		return m_ProcAddress_SetFileAttributesTransacted(lpFileName, dwFileAttributes, m_transaction);
	}else{
		return ::SetFileAttributes(lpFileName, dwFileAttributes);
	}
}

HANDLE KTMTransaction::FindFirstFileEx(LPCTSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPVOID lpFindFileData, FINDEX_SEARCH_OPS fSearchOp, LPVOID lpSearchFilter, DWORD dwAdditionalFlags)
{
	if(UseTransactedFunctions()){
		return m_ProcAddress_FindFirstFileTransacted(lpFileName, fInfoLevelId, lpFindFileData, fSearchOp, lpSearchFilter, dwAdditionalFlags, m_transaction);
	}else{
		return ::FindFirstFileEx(lpFileName, fInfoLevelId, lpFindFileData, fSearchOp, lpSearchFilter, dwAdditionalFlags);
	}
}


//////////////////////////////////////////////////////////////////////////
// Registry Functions
//////////////////////////////////////////////////////////////////////////

LONG KTMTransaction::RegCreateKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD Reserved, LPTSTR lpClass, DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition)
{
	if(UseTransactedFunctions()){
		return m_ProcAddress_RegCreateKeyTransacted(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition, m_transaction, NULL);
	}else{
		return ::RegCreateKeyEx(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
	}
}

LONG KTMTransaction::RegDeleteKey(HKEY hKey, LPCTSTR lpSubKey)
{
	if(UseTransactedFunctions()){
#ifdef _WIN64
		REGSAM samDesired = KEY_WOW64_64KEY; // Delete the key from the 64-bit registry view
#else
		REGSAM samDesired = KEY_WOW64_32KEY; // Delete the key from the 32-bit registry view
#endif
		return m_ProcAddress_RegDeleteKeyTransacted(hKey, lpSubKey, samDesired, 0, m_transaction, NULL);
	}else{
		return ::RegDeleteKey(hKey, lpSubKey);
	}
}

LONG KTMTransaction::RegOpenKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult)
{
	if(UseTransactedFunctions()){
		return m_ProcAddress_RegOpenKeyTransacted(hKey, lpSubKey, ulOptions, samDesired, phkResult, m_transaction, NULL);
	}else{
		return ::RegOpenKeyEx(hKey, lpSubKey, ulOptions, samDesired, phkResult);
	}
}

}}
