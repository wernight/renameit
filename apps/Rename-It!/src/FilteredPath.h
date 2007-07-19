#pragma once

#include "Path.h"

/**
 * A file name that is renamed by a rename-it! filter. So the part being renamed can be accessed.
 */
class CFilteredPath : public CPath
{
public:
// Definitions
	// Renamed part (must be continuous: (0x01 | 0x04) is not allowed as there is 0x02 in the middle missing).
	enum ERenamePartFlags {
		renameFolders = 0x01,
		renameFilename = 0x02,
		renameExtension = 0x04
	};

// Construction
	CFilteredPath(const CPath& fnFileName, unsigned nRenamedPart) : CPath(fnFileName),
		m_nRenamePart(nRenamedPart)
	{
		FindFilteredSubstring();
		ASSERT(0 <= m_nFirst && m_nFirst < m_strPath.GetLength());
		ASSERT(0 <= m_nEnd && m_nEnd <= m_strPath.GetLength());
	}

// Attributes
	/**
	 * The part being renamed by filters.
	 * @return A string corresponding to the renamed part of the full path.
	 */
	CString GetFilteredSubstring() const 
	{
		ASSERT(0 <= m_nFirst && m_nFirst < m_strPath.GetLength());
		ASSERT(0 <= m_nEnd && m_nEnd <= m_strPath.GetLength());

		return m_strPath.Mid(m_nFirst, m_nEnd - m_nFirst);
	}

	/**
	 * Modify the renamed part of the file name.
	 * @strValue A string corresponding to the new renamed part of the full path.
	 */
	void SetFilteredSubstring(const CString& strValue) 
	{
		ASSERT(0 <= m_nFirst && m_nFirst < m_strPath.GetLength());
		ASSERT(0 <= m_nEnd && m_nEnd <= m_strPath.GetLength());

		// Change the renamed part.
		SetPath(m_strPath.Left(m_nFirst) + strValue + m_strPath.Mid(m_nEnd));

		FindFilteredSubstring();
	}

// Implementation
private:
	// Update the m_nFirst and m_nLast.
	inline void FindFilteredSubstring()
	{
		// Find the first character of the filtered substring.
		if (m_nRenamePart & renameFolders)
			m_nFirst = m_nDisplayStart;
		else if (m_nRenamePart & renameFilename)
			m_nFirst = m_nFileNameFirst;
		else
		{
			// Only the extension is being renamed.
			ASSERT(m_nRenamePart == renameExtension);
			m_nFirst = m_strPath.GetLength() - std::max<int>(m_nExtensionLength - 1, 0);
		}

		// Find the last character of the filtered substring.
		if (m_nRenamePart & renameExtension)
			m_nEnd = m_strPath.GetLength();
		else if (m_nRenamePart & renameFilename)
			m_nEnd = m_strPath.GetLength() - m_nExtensionLength;
		else
		{
			ASSERT(m_nRenamePart == renameFolders);
			m_nEnd = m_nFileNameFirst;
		}

		ASSERT(0 <= m_nFirst && m_nFirst < m_strPath.GetLength());
		ASSERT(0 <= m_nEnd && m_nEnd <= m_strPath.GetLength());
	}

	unsigned m_nRenamePart;	// A set of ERenamePartFlags bit flags.
	int m_nFirst;	// First character of the filtered substring.
	int m_nEnd;		// The character right after last character of the filtered substring.
};
