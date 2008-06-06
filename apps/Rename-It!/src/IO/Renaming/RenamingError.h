#pragma once
#include "IRenameError.h"

namespace Beroux{ namespace IO{ namespace Renaming
{
	/**
	 * Failed to rename a file or a directory.
	 */
	class CRenamingError : public IRenameError
	{
	public:
		CRenamingError(const CPath& pathNameBefore, const CPath& pathNameAfter, DWORD dwErrorCode) :
		  IRenameError(levelWarning),
		  m_pathNameBefore(pathNameBefore),
		  m_pathNameAfter(pathNameAfter),
		  m_dwErrorCode(dwErrorCode)
		{}

	// Attributes
		const CPath& GetPathNameBefore() const { return m_pathNameBefore; }

		const CPath& GetPathNameAfter() const { return m_pathNameAfter; }

		DWORD GetErrorCode() const { return m_dwErrorCode; }

		CString GetErrorMessage() const { return IRenameError::GetErrorMessage(m_dwErrorCode); }

	// Implementation
	private:
		CPath m_pathNameBefore;
		CPath m_pathNameAfter;
		DWORD m_dwErrorCode;
	};
}}}
