#pragma once
#include "IOOperation.h"

namespace Beroux{ namespace IO{ namespace Renaming{ namespace IOOperation
{
	// Perform a CreateDirectoryEx() to create all folders missing up to the
	// provided folder.
	class CCreateDirectoryOperation : public CIOOperation
	{
	// Construction
	public:
		CCreateDirectoryOperation(const CPath& pathFolderName) :
		  m_pathFolderName(pathFolderName)
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
			CString strParentPath = m_pathFolderName.GetPathRoot();
			BOOST_FOREACH(CString strDirectoryName, m_pathFolderName.GetDirectories())
			{
				// Keep the parent directory of this parent directory.
				CString strParentParentPath = strParentPath;

				// Get the full parent directory's path.
				strParentPath += strDirectoryName;

				// Check if the parent path already exists.
				WIN32_FIND_DATA fd;
				HANDLE hFind = ktm.FindFirstFileEx(strParentPath, FindExInfoStandard, &fd, FindExSearchNameMatch/*FindExSearchLimitToDirectories*/, NULL, 0);
				if (hFind == INVALID_HANDLE_VALUE)
				{
					if (::GetLastError() == ERROR_FILE_NOT_FOUND)
					{
						// Create the parent directory.
						if (!ktm.CreateDirectoryEx(NULL, strParentPath, NULL))
							return elError;
					}
					else
						return elWarning;
				}
			}

			return elSuccess;
		}

	// Implementation
	private:
		CPath m_pathFolderName;
	};
}}}}
