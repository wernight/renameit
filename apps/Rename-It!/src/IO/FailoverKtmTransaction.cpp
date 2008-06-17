#include "stdafx.h"
#include "FailoverKtmTransaction.h"

#define KTM_FAILOVER_WRAP(ReturnType, ReturnErrorValue, FunctionArgs) \
	ReturnType ret = CKtmTransaction::FunctionArgs; \
	if (ret == ReturnErrorValue && ::GetLastError() == ERROR_RM_NOT_ACTIVE) \
	{ \
		m_bIsLastOperationUsingKtm = false; \
		return ::FunctionArgs; \
	} \
	else \
	{ \
		m_bIsLastOperationUsingKtm = UseTransactedFunctions(); \
		return ret; \
	}


namespace Beroux{ namespace IO
{

CFailoverKtmTransaction::CFailoverKtmTransaction(
	 LPSECURITY_ATTRIBUTES lpTransactionAttributes // = NULL
	,DWORD CreateOptions                           // = 0
	,DWORD Timeout                                 // = NULL
	,LPWSTR Description                            // = NULL
	,bool useTransactionsIfAvailable               // = true
)
: CKtmTransaction(lpTransactionAttributes, CreateOptions, Timeout, Description, useTransactionsIfAvailable)
, m_bIsLastOperationUsingKtm(useTransactionsIfAvailable)
{
}

BOOL CFailoverKtmTransaction::CreateDirectoryEx(LPCTSTR lpTemplateDirectory, LPCTSTR lpNewDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	KTM_FAILOVER_WRAP(BOOL, FALSE, CreateDirectoryEx(lpTemplateDirectory, lpNewDirectory, lpSecurityAttributes));
}

BOOL CFailoverKtmTransaction::RemoveDirectory(LPCTSTR lpPathName)
{
	KTM_FAILOVER_WRAP(BOOL, FALSE, RemoveDirectory(lpPathName));
}

BOOL CFailoverKtmTransaction::CopyFileEx(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, 
	LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags)
{
	KTM_FAILOVER_WRAP(BOOL, FALSE, CopyFileEx(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags));
}

BOOL CFailoverKtmTransaction::MoveFileEx(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, DWORD dwFlags){
	KTM_FAILOVER_WRAP(BOOL, FALSE, MoveFileEx(lpExistingFileName, lpNewFileName, dwFlags));
}
#if (_WIN32_WINNT >= 0x0500)
BOOL CFailoverKtmTransaction::MoveFileWithProgress(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags)
{
	KTM_FAILOVER_WRAP(BOOL, FALSE, MoveFileWithProgress(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags));
}
#endif // (_WIN32_WINNT >= 0x0500)

HANDLE CFailoverKtmTransaction::CreateFile(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, 
										   DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	KTM_FAILOVER_WRAP(HANDLE, INVALID_HANDLE_VALUE, CreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile));
}

BOOL CFailoverKtmTransaction::DeleteFile(LPCTSTR lpFileName)
{
	KTM_FAILOVER_WRAP(DWORD, 0, DeleteFile(lpFileName));
}

HANDLE CFailoverKtmTransaction::FindFirstFileEx(LPCTSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPVOID lpFindFileData, FINDEX_SEARCH_OPS fSearchOp, LPVOID lpSearchFilter, DWORD dwAdditionalFlags)
{
	KTM_FAILOVER_WRAP(HANDLE, INVALID_HANDLE_VALUE, FindFirstFileEx(lpFileName, fInfoLevelId, lpFindFileData, fSearchOp, lpSearchFilter, dwAdditionalFlags));
}

DWORD CFailoverKtmTransaction::GetCompressedFileSize(LPCTSTR lpFileName, LPDWORD lpFileSizeHigh)
{
	KTM_FAILOVER_WRAP(DWORD, INVALID_FILE_SIZE, GetCompressedFileSize(lpFileName, lpFileSizeHigh));
}

BOOL CFailoverKtmTransaction::GetFileAttributesEx(LPCTSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpFileInformation)
{
	KTM_FAILOVER_WRAP(BOOL, FALSE, GetFileAttributesEx(lpFileName, fInfoLevelId, lpFileInformation));
}

BOOL CFailoverKtmTransaction::SetFileAttributes(LPCTSTR lpFileName, DWORD dwFileAttributes)
{
	KTM_FAILOVER_WRAP(BOOL, FALSE, SetFileAttributes(lpFileName, dwFileAttributes));
}

DWORD CFailoverKtmTransaction::GetFullPathName(LPCTSTR lpFileName, DWORD nBufferLength, LPTSTR lpBuffer, LPTSTR* lpFilePart)
{
	KTM_FAILOVER_WRAP(DWORD, 0, GetFullPathName(lpFileName, nBufferLength, lpBuffer, lpFilePart));
}

DWORD CFailoverKtmTransaction::GetLongPathName(LPCTSTR lpszShortPath, LPTSTR lpszLongPath, DWORD cchBuffer)
{
	KTM_FAILOVER_WRAP(DWORD, 0, GetLongPathName(lpszShortPath, lpszLongPath, cchBuffer));
}

#if (_WIN32_WINNT >= 0x0500)
BOOL CFailoverKtmTransaction::CreateHardLink(LPCTSTR lpFileName, LPCTSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	KTM_FAILOVER_WRAP(BOOL, FALSE, CreateHardLink(lpFileName, lpExistingFileName, lpSecurityAttributes));
}
#endif // (_WIN32_WINNT >= 0x0500)


//////////////////////////////////////////////////////////////////////////
// Registry Functions
//////////////////////////////////////////////////////////////////////////

LONG CFailoverKtmTransaction::RegCreateKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD Reserved, LPTSTR lpClass, DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition)
{
	// No failover for that one as it shouldn't fail when transactions are supported.
	m_bIsLastOperationUsingKtm = UseTransactedFunctions();
	return CKtmTransaction::RegCreateKeyEx(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
}

LONG CFailoverKtmTransaction::RegDeleteKey(HKEY hKey, LPCTSTR lpSubKey)
{
	// No failover for that one as it shouldn't fail when transactions are supported.
	m_bIsLastOperationUsingKtm = UseTransactedFunctions();
	return CKtmTransaction::RegDeleteKey(hKey, lpSubKey);
}

LONG CFailoverKtmTransaction::RegOpenKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult)
{
	// No failover for that one as it shouldn't fail when transactions are supported.
	m_bIsLastOperationUsingKtm = UseTransactedFunctions();
	return CKtmTransaction::RegOpenKeyEx(hKey, lpSubKey, ulOptions, samDesired, phkResult);
}

}}
