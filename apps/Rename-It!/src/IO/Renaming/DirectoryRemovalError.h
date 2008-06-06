#pragma once
#include "IRenameError.h"

namespace Beroux{ namespace IO{ namespace Renaming
{
	/**
	 * Failed to remove a folder to clean-up after renaming.
	 */
	class CDirectoryRemovalError : public IRenameError
	{
	public:
		CDirectoryRemovalError(const CString& strDirectoryPath, DWORD dwErrorCode) :
		  IRenameError(levelError),
		  m_strDirectoryPath(strDirectoryPath),
		  m_dwErrorCode(dwErrorCode)
		{}

	// Attributes
		const CString& GetDirectoryPath() const { return m_strDirectoryPath; }

		DWORD GetErrorCode() const { return m_dwErrorCode; }

		CString GetErrorMessage() const { return IRenameError::GetErrorMessage(m_dwErrorCode); }

	// Implementation
	private:
		CString m_strDirectoryPath;
		DWORD m_dwErrorCode;
	};
}}}
