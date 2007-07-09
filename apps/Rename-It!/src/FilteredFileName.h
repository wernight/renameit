#pragma once

#include "FileName.h"

/**
 * A file name that is renamed by a rename-it! filter. So the part being renamed can be accessed.
 */
class CFilteredFileName : public CFileName
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
	CFilteredFileName(const CFileName& fnFileName, unsigned nRenamedPart) : CFileName(fnFileName),
		m_nRenamePart(nRenamedPart)
	{
	}

// Attributes
	/**
	 * The part being renamed by filters.
	 * @return A string corresponding to the renamed part of the full path.
	 */
	CString GetFilteredSubstring() const 
	{
		// For the special case when there is only the extension to rename.
		if (m_nRenamePart == renameExtension)
			return m_strExtension.Mid(1);

		// General case.
		CString strRenamedPart;
		if (m_nRenamePart & renameFolders)
			strRenamedPart += m_strDirectory;

		if (m_nRenamePart & renameFilename)
			strRenamedPart += m_strFileName;

		if (m_nRenamePart & renameExtension)
			strRenamedPart += m_strExtension;
		return strRenamedPart;
	}

	/**
	 * Modify the renamed part of the file name.
	 * @strValue A string corresponding to the new renamed part of the full path.
	 */
	void SetFilteredSubstring(const CString& strValue) 
	{
		CString strNewFileName;

		// For the special case, if there is only the extension
		if (m_nRenamePart == renameExtension)
		{
			// Change the renamed part.
			SetTo(GetDrive() + GetDirectory() + GetFileName() + _T(".") + strValue);
		}
		else
		{
			// Index of the first character to rename.
			int nStart = m_strDrive.GetLength();
			if ((m_nRenamePart & renameFolders) == 0)
			{
				nStart += m_strDirectory.GetLength();
				ASSERT(m_nRenamePart & renameFilename);	// We assume it's not only the extension that is renamed.
			}

			// Index of the last character to rename.
			int nEnd = m_strFullPath.GetLength();
			if ((m_nRenamePart & renameExtension) == 0)
			{
				nEnd -= m_strExtension.GetLength();
				if ((m_nRenamePart & renameFilename) == 0)
				{
					nEnd -= m_strFileName.GetLength();
					ASSERT(m_nRenamePart & renameFolders);
				}
			}

			// Change the renamed part.
			SetTo(m_strFullPath.Mid(0, nStart) + strValue + m_strFullPath.Mid(nEnd));
		}
	}

// Implementation
private:
	unsigned m_nRenamePart;	// A set of ERenamePartFlags bit flags.
};
