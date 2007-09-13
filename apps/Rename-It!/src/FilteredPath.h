#pragma once

#include "Path.h"

/**
 * A file name that is renamed by a rename-it! filter. So the part being renamed can be accessed.
 */
class CFilteredPath : public CPath
{
public:
// Definitions
	/**
	 * Renamed part (must be continuous: (0x01 | 0x04) is not allowed as there is 0x02 in the middle missing).
	 * WARNING: renameLastFolder can only be used alone.
	 * The renameRoot shouldn't be used with folders without the file name, because folders's root cannot be changed.
	 */
	enum ERenamePartFlags {
		renameRoot =		0x01,	// 'C:\'
		renameFoldersPath =	0x02,	// 'ParentFolder\Subfolder\'
		renameLastFolder =	0x04,	// 'Subfolder'
		renameFilename =	0x08,	// 'Filename'
		renameExtension =	0x10,	// 'Ext'
		renameVersion = 100			// Some value that should be incremented every time there is an important change in the flags.
	};

// Construction
	CFilteredPath(const CPath& fnFileName, unsigned nRenamedPart) : CPath(fnFileName),
		m_nRenamePart(nRenamedPart)
	{
		FindFilteredSubstring();
		ASSERT(0 <= m_nFirst && m_nFirst <= m_strPath.GetLength());
		ASSERT(0 <= m_nEnd && m_nEnd <= m_strPath.GetLength());
	}

// Attributes
	/**
	 * The part being renamed by filters.
	 * @return A string corresponding to the renamed part of the full path.
	 */
	CString GetFilteredSubstring() const 
	{
		ASSERT(0 <= m_nFirst && m_nFirst <= m_strPath.GetLength());
		ASSERT(0 <= m_nEnd && m_nEnd <= m_strPath.GetLength());

		return m_strPath.Mid(m_nFirst, m_nEnd - m_nFirst);
	}

	/**
	 * Modify the renamed part of the file name.
	 * @strValue A string corresponding to the new renamed part of the full path.
	 */
	void SetFilteredSubstring(const CString& strValue) 
	{
		ASSERT(0 <= m_nFirst && m_nFirst <= m_strPath.GetLength());
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
		BOOST_STATIC_ASSERT(renameVersion == 100);

		// Note: It may be awkward to select the file name when renaming the folders,
		//       but that's because when renaming folders, the file name IS the last
		//       folder's name.

		// Find the first character of the filtered substring.
		if (m_nRenamePart & renameRoot)
			m_nFirst = 0;
		else if (m_nRenamePart & renameFoldersPath)
			m_nFirst = m_nPathRootLength;
		else if (m_nRenamePart & (renameLastFolder | renameFilename))
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
		else if (m_nRenamePart & (renameFoldersPath | renameLastFolder | renameFilename))
			m_nEnd = m_strPath.GetLength() - m_nExtensionLength;
		else
		{
			ASSERT(m_nRenamePart == renameRoot);
			m_nEnd = m_nPathRootLength;
		}

		ASSERT(0 <= m_nFirst && m_nFirst <= m_strPath.GetLength());
		ASSERT(0 <= m_nEnd && m_nEnd <= m_strPath.GetLength());
	}

	unsigned m_nRenamePart;	// A set of ERenamePartFlags bit flags.
	int m_nFirst;	// First character of the filtered substring.
	int m_nEnd;		// The character right after last character of the filtered substring.
};
