#pragma once

/**
 * Path to a file or a folder.
 * Based on C# System.IO.Path (cf http://msdn2.microsoft.com/en-us/library/system.io.path_members(vs.90).aspx )
 */
class CPath
{
// Construction
public:
	CPath(void) :
		m_nDisplayStart(0)
	{
	}

	CPath(const CString& strFullPath)
	{
		SetPath(strFullPath);
	}

// Attributes
	/**
	 * Returns the absolute path for the specified path string.
	 * The unicode 32,000 characters path to be used for system functions.
	 */
	inline const CString& GetFullPath() const {
		return m_strPath;
	}

	/**
	 * Return the path without "\\?\" to make it more user friendly.
	 */
	inline CString GetDisplayPath() const {
		return m_strPath.Mid(m_nDisplayStart);
	}

	/**
	 * Gets the direct letter of the path.
	 * Ex: "C:" or "" if there is none.
	 */
	inline CString GetDrive() const {
		if (m_nDriveLength == 0)
			return _T("");
		else
			return m_strPath.Mid(m_nDisplayStart, m_nDriveLength);
	}

	/**
	 * Gets the root directory information of the specified path.
	 * Ex: "\\?\C:\".
	 */
	inline CString GetPathRoot() const {
		return m_strPath.Left(m_nDisplayStart + m_nDriveLength + 1);
	}

	/**
	 * Returns the directory information for the specified path string.
	 * Ex: "\\?\C:\foo\"
	 */
	inline CString GetDirectoryName() const {
		return m_strPath.Left(m_nFileNameFirst);
	}

	/**
	 * Returns the file name and extension of the specified path string.
	 * Ex: "bar.txt"
	 */
	inline CString GetFileName() const {
		return m_strPath.Mid(m_nFileNameFirst);
	}

	/**
	 * Returns the file name of the specified path string without the extension.
	 * Ex: "bar"
	 */
	inline CString GetFileNameWithoutExtension() const {
		return m_strPath.Mid(m_nFileNameFirst, m_strPath.GetLength() - m_nFileNameFirst - m_nExtensionLength);
	}

	/**
	 * Returns the extension of the specified path string.
	 * Ex: ".txt"
	 */
	inline CString GetExtension() const {
		return m_strPath.Right(m_nExtensionLength);
	}

// Operations
	// Compare two files names using the file system comparaison.
	static inline int FSCompare(const CString& a, const CString& b) {
		static const _locale_t m_liFileSystemLocale = _create_locale(LC_CTYPE, "");
		return _tcsicmp_l(a, b, m_liFileSystemLocale);
	}

	// Compare two files names using the file system comparaison.
	inline int FSCompare(const CPath& other) const {
		static const _locale_t m_liFileSystemLocale = _create_locale(LC_CTYPE, "");
		return _tcsicmp_l(m_strPath, other.m_strPath, m_liFileSystemLocale);
	}

	inline int FSCompare(const CPath& other, int nLength) const {
		static const _locale_t m_liFileSystemLocale = _create_locale(LC_CTYPE, "");
		return _tcsnicmp_l(m_strPath, other.m_strPath, nLength, m_liFileSystemLocale);
	}

	/**
	 * Return an array of parent directories starting from the root.
	 */
	inline vector<CString> GetDirectories() const {
		vector<CString> vPath;

		int end;
		for (int first = m_nDisplayStart + m_nDriveLength; (end = m_strPath.Find('\\', first)) != -1; first = end + 1)
		{
			if (first != end)
				vPath.push_back( m_strPath.Mid(first, end - first) );
		}

		return vPath;
	}

	inline operator const CString&() const {
		return m_strPath;
	}

	inline bool operator==(const CPath& other) const {
		return m_strPath == other.m_strPath;
	}

	inline bool operator!=(const CPath& other) const {
		return m_strPath != other.m_strPath;
	}

// Private
protected:
	inline void SetPath(const CString& strFullPath)
	{
		// Save the full path.
		m_strPath = strFullPath;
		if (strFullPath.GetLength() > 4 && strFullPath.Left(4) == _T("\\\\?\\"))
		{
			if (strFullPath.GetLength() > 8 && strFullPath.Left(8) == _T("\\\\?\\UNC\\"))
				m_nDisplayStart = 8;
			else
				m_nDisplayStart = 4;
		}
		else
			m_nDisplayStart = 0;

		// Replace '/' by '\\'.
		int nPos;
		while ((nPos = m_strPath.Find('/')) != -1)
			m_strPath.SetAt(nPos, '\\');

		// Find the drive.
		if (strFullPath.GetLength() > m_nDisplayStart + 2
				&& isalpha(strFullPath[m_nDisplayStart])
				&& strFullPath[m_nDisplayStart + 1] == ':')
			m_nDriveLength = 2;
		else
			m_nDriveLength = 0;

		// Split the path into components.
		m_nExtensionLength = 0;
		m_nFileNameFirst = strFullPath.GetLength();
		LPCTSTR pszPath = strFullPath;
		const TCHAR* pchEnd = &pszPath[strFullPath.GetLength()];
		for (const TCHAR* pch = pchEnd-1; pch >= pszPath; --pch)
		{
			switch (*pch)
			{
			case '.':
				if (m_nExtensionLength == 0)
					m_nExtensionLength = (int)(pchEnd - pch);
				break;

			case '\\':
				m_nFileNameFirst = (int)(pch - pszPath) + 1;
				goto loop_exit;
			}
		}
loop_exit:;
	}

	CString	m_strPath;		// The full path to the file or folder (ex: "\\?\C:\foo\bar").
	int m_nDisplayStart;	// First character to display when showing the path (used to hide \\?\ on display).
	int m_nDriveLength;		// The length of the drive (either 2 or 0).
	int m_nFileNameFirst;	// First character of the file name (or the length of m_strPath if there is none).
	int m_nExtensionLength;	// The length of the extension including the "." (can be zero).
};
