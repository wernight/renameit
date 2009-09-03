#pragma once
#include "IOOperation.h"

namespace Beroux{ namespace IO{ namespace Renaming{ namespace IOOperation
{
	/**
	 * Perform a CreateDirectoryEx() to create all folders missing up to the
	 * provided folder included.
	 */
	class CCreateDirectoryOperation : public CIOOperation
	{
	// Construction
	public:
		CCreateDirectoryOperation(const CPath& pathFolderName) :
		  m_pathFolderName(pathFolderName),
		  m_nRootLength(pathFolderName.GetPathRoot().GetLength())
		{
		}

	// Attributes
		const CPath& GetDirectoryPath() const
		{
			return m_pathFolderName;
		}

	// Overrides
		virtual EErrorLevel Perform(CKtmTransaction& ktm)
		{
			// Check that every parent directory exists, or create it.
			return CreateDirectory(m_pathFolderName.GetPath(), ktm);
		}

	// Implementation
	private:
		EErrorLevel CreateDirectory(const CString& strDirectoryPath, CKtmTransaction& ktm) const
		{
			// If it's the path root, can't create it.
			if (strDirectoryPath.GetLength() > m_nRootLength)
			{
				// Check if the path already exists.
				WIN32_FIND_DATA fd;
				HANDLE hFind = ktm.FindFirstFileEx(strDirectoryPath, FindExInfoStandard, &fd, FindExSearchNameMatch/*FindExSearchLimitToDirectories*/, NULL, 0);
				if (hFind == INVALID_HANDLE_VALUE)
				{
					DWORD dwLastError = ::GetLastError();
					if (dwLastError == ERROR_PATH_NOT_FOUND || dwLastError == ERROR_FILE_NOT_FOUND)
					{
						// Create the parent directory.
						ASSERT(strDirectoryPath.Find('/') == -1);
						int nPos = strDirectoryPath.ReverseFind('\\');
						if (nPos == -1)
							return elWarning;	// That shouldn't happen.
						EErrorLevel nParentRet = CreateDirectory(strDirectoryPath.Left(nPos), ktm);
						if (nParentRet != elSuccess)
							return nParentRet;

						// Create the directory.
						if (!ktm.CreateDirectoryEx(CPath(strDirectoryPath).GetDirectoryName(), strDirectoryPath, NULL))
							return elError;
					}
					else
						return elWarning;
				}
				else
					::FindClose(hFind);
			}

			return elSuccess;
		}

		CPath m_pathFolderName;
		int m_nRootLength;
	};
}}}}
