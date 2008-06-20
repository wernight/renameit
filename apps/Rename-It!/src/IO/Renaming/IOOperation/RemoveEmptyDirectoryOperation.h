#pragma once
#include "IOOperation.h"

namespace Beroux{ namespace IO{ namespace Renaming{ namespace IOOperation
{
	// Perform a RemoveDirectory() if the folder is empty.
	class CRemoveEmptyDirectoryOperation : public CIOOperation
	{
	// Construction
	public:
		CRemoveEmptyDirectoryOperation(const CString& strFolderName) :
		  m_strFolderName(strFolderName)
		{
		}

	// Attributes
		const CString& GetDirectoryPath() const
		{
			return m_strFolderName;
		}

	// Overrides
		virtual EErrorLevel Perform(CKtmTransaction& ktm)
		{
			// Note: ERROR_ACCESS_DENIED may be reported for non-empty dir that are protected,
			//       so we need to check if the directory is empty prior to calling RemoveDirectory().
			if (DirectoryIsEmpty(ktm) &&
				!ktm.RemoveDirectory(m_strFolderName))
			{
				/** Some possible error codes include:
				 * ERROR_ACCESS_DENIED		For folders marked as read-only, and probably for folders with other security settings.
				 * ERROR_FILE_CORRUPT		When the directory cannot be accessed because of some problem.
				 * ERROR_DIR_NOT_EMPTY		Directory not empty.
				 * ERROR_FILE_NOT_FOUND		Directory doesn't exist.
				 */
				ASSERT(::GetLastError() != ERROR_DIR_NOT_EMPTY);
				return elWarning;
			}
			else
				return elSuccess;
		}

	// Implementation
	private:
		bool DirectoryIsEmpty(CKtmTransaction& ktm)
		{
			ASSERT(m_strFolderName.Right(1) == '\\');

			WIN32_FIND_DATA fd;
			HANDLE hFindFile;

			hFindFile = ktm.FindFirstFileEx(m_strFolderName + _T("*.*"), FindExInfoStandard, &fd, FindExSearchNameMatch, NULL, 0);

			if (hFindFile == INVALID_HANDLE_VALUE)
			{
				DWORD dwErrorCode = ::GetLastError();
#ifdef _DEBUG
				// Get error message
				LPTSTR lpMsgBuf = NULL;
				FormatMessage( 
					FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					NULL,
					dwErrorCode,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,
					0,
					NULL );
				CString strErrorMessage = lpMsgBuf;
				LocalFree( lpMsgBuf );
#endif
				ASSERT(dwErrorCode == ERROR_PATH_NOT_FOUND);
				return false;
			}
			else
			{
				do
				{
					if (_tcscmp(fd.cFileName, _T(".")) != 0 &&
						_tcscmp(fd.cFileName, _T("..")) != 0)
					{
						::FindClose(hFindFile);
						return false;
					}
				} while (::FindNextFile(hFindFile, &fd));

				::FindClose(hFindFile);
				return true;
			}
		}

		CString m_strFolderName;
	};
}}}}
