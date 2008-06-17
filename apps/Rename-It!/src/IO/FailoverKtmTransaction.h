#pragma once
#include "KtmTransaction.h"

namespace Beroux{ namespace IO
{
	/**
	 * A KTM transaction that uses non-KTM versions when the KTM version fails.
	 * It happens that a system supports KTM but not all of its file systems.
	 * In such cases CKtmTransaction issue a ERROR_RM_NOT_ACTIVE error code.
	 *
	 * This class handles that error an failover the non-KTM version of the same
	 * function in such cases. When the function fails, it returns an error.
	 *
	 * When an operation failed, it's possible to know if the KTM version of that
	 * operation failed or if the non-KTM version failed by calling:
	 * IsLastOperationUsingKtm() before doing other operations.
	 */
	class CFailoverKtmTransaction : public CKtmTransaction
	{
	public:
		/**
		 * Constructor typically called using default parameters - see MSDN help for details
		 * set last parameter to false to disable the transaction functionality.
		 * \param lpTransactionAttributes See MSDN help.
		 * \param createOptions See MSDN help.
		 * \param timeout NULL == infinite timeout.
		 * \param description User-readable string.
		 * \param useTransactionsIfAvailable Set to false to disable the transaction functionality.
		 */
		CFailoverKtmTransaction(
			 LPSECURITY_ATTRIBUTES lpTransactionAttributes = NULL
			,DWORD createOptions = 0
			,DWORD timeout       = NULL
			,LPWSTR description  = NULL
			,bool useTransactionsIfAvailable = true
			);

		bool IsLastOperationUsingKtm() {
			return m_bIsLastOperationUsingKtm;
		}

		///////////////////////////////////////////////////////////////////////////////////////
		// NOTE: The transacted functions take the exact same parameters (in the same order) 
		//       as the regular Win32 functions of the same name.
		///////////////////////////////////////////////////////////////////////////////////////

		// File Functions
		virtual BOOL   CreateDirectoryEx(LPCTSTR lpTemplateDirectory, LPCTSTR lpNewDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
		virtual BOOL   RemoveDirectory(LPCTSTR lpPathName);	
		virtual BOOL   CopyFileEx(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags);
		virtual BOOL   MoveFileEx(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, DWORD dwFlags);
#if (_WIN32_WINNT >= 0x0500)
		virtual BOOL   MoveFileWithProgress(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags);
#endif // (_WIN32_WINNT >= 0x0500)
		virtual HANDLE CreateFile(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
		virtual BOOL   DeleteFile(LPCTSTR lpFileName);
		virtual HANDLE FindFirstFileEx(LPCTSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPVOID lpFindFileData, FINDEX_SEARCH_OPS fSearchOp, LPVOID lpSearchFilter, DWORD dwAdditionalFlags);
		virtual DWORD  GetCompressedFileSize(LPCTSTR lpFileName, LPDWORD lpFileSizeHigh);
		virtual BOOL   GetFileAttributesEx(LPCTSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpFileInformation);
		virtual BOOL   SetFileAttributes(LPCTSTR lpFileName, DWORD dwFileAttributes);
		virtual DWORD  GetFullPathName(LPCTSTR lpFileName, DWORD nBufferLength, LPTSTR lpBuffer, LPTSTR* lpFilePart);
		virtual DWORD  GetLongPathName(LPCTSTR lpszShortPath, LPTSTR lpszLongPath, DWORD cchBuffer);	
#if (_WIN32_WINNT >= 0x0500)
		virtual BOOL   CreateHardLink(LPCTSTR lpFileName, LPCTSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
#endif // (_WIN32_WINNT >= 0x0500)

		// Registry Functions
		virtual LONG RegCreateKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD Reserved, LPTSTR lpClass, DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition);
		virtual LONG RegDeleteKey(HKEY hKey, LPCTSTR lpSubKey);
		virtual LONG RegOpenKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);

	// Implementation
	private:
		bool m_bIsLastOperationUsingKtm;
	};
}}
