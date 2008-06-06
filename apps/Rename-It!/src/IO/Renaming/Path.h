#pragma once
#include <Shlwapi.h>	// Used for PathGetArgs(), PathIsDirectory(), and PathFileExists().

namespace Beroux{ namespace IO{ namespace Renaming
{ 
	/**
	 * Path to a file or a folder.
	 * Based on C# System.IO.Path (cf http://msdn2.microsoft.com/en-us/library/system.io.path_members(vs.90).aspx )
	 */
	class CPath
	{
	// Construction
	public:
		CPath(void) :
  			m_nPathRootLength(0),
			m_nFileNameFirst(0),
			m_nExtensionLength(0)
		{
		}

		CPath(const CPath& path)
		{
			m_strPath = path.m_strPath;
			m_nPathRootLength = path.m_nPathRootLength;
			m_nFileNameFirst = path.m_nFileNameFirst;
			m_nExtensionLength = path.m_nExtensionLength;
		}

		CPath(const CString& strFullPath)
		{
			SetPath(strFullPath);
		}

	// Attributes
		/**
		 * Return the path as provided during the creation of this object.
		 * It's the right one to be used when displaying the full path to the user.
		 */
		inline const CString& GetPath() const {
			return m_strPath;
		}

		/**
		 * Gets the root directory information of the specified path.
		 * Ex: "C:\".
		 */
		inline CString GetPathRoot() const {
			return m_strPath.Left(m_nPathRootLength);
		}

		/**
		 * Returns the directory information for the specified path string.
		 * Ex: "C:\foo\"
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
		// Determines whether a path to a file system object such as a file or directory is valid.
		static bool PathFileExists(const CString& strPath)
		{
			ASSERT(strPath.IsEmpty() || strPath.GetAt(strPath.GetLength() - 1) != '.');	// No file or folder should end by a dot (.).
			ASSERT(strPath.IsEmpty() || ::GetFileAttributes(strPath) != S_OK || (::GetFileAttributes(strPath) & FILE_ATTRIBUTE_DIRECTORY) == 0);	// Doesn't fully support directories (yet).

			if (strPath.IsEmpty())
				return false;
			else if (strPath.GetLength() < MAX_PATH)
				return ::PathFileExists(strPath) != 0;
			else
			{
				ASSERT(strPath.Left(4) == _T("\\\\?\\"));	// Only Unicode path case go past MAX_PATH.

				// We must use FindFile for very long path.
				WIN32_FIND_DATA fd;
				HANDLE hFindFile;

				// Directories should not end by '\' for FindFirstFileEx (or it would fail the test).
				if (strPath[strPath.GetLength() - 1] != '\\')
					hFindFile = ::FindFirstFileEx(strPath, FindExInfoStandard, &fd, FindExSearchNameMatch, NULL, 0);
				else
					hFindFile = ::FindFirstFileEx(strPath.Left(strPath.GetLength() - 1), FindExInfoStandard, &fd, FindExSearchNameMatch, NULL, 0);

				if (hFindFile != INVALID_HANDLE_VALUE)
				{
					::FindClose(hFindFile);
					return true;
				}
				else
					return false;
			}
		}

		/**
		 * Returns the absolute path for the specified path string.
		 * The Unicode 32,000 characters path to be used for system functions.
		 */
		static inline CString MakeUnicodePath(const CString& strPath)
		{
			if (strPath.GetLength() >= 2)
			{
				if (strPath.GetLength() >= 4 && strPath.Left(4) == _T("\\\\?\\"))
					return strPath;
				else if (strPath[1] == ':' && isalpha(strPath[0]))
					return _T("\\\\?\\") + strPath;
				else
					return _T("\\\\?\\UNC\\") + strPath.Mid(2);
			}
			else
				return strPath;
		}

		/**
		 * Returns the path without \\?\.
		 */
		static inline CString MakeSimplePath(const CString& strPath)
		{
			if (strPath.Left(8) == _T("\\\\?\\UNC\\"))
				return '\\' + strPath.Mid(7);
			else if (strPath.Left(4) == _T("\\\\?\\"))
				return strPath.Mid(4);
			else
				return strPath;
		}

		// Compare two files names using the file system comparison.
		static inline int FSCompare(const CString& a, const CString& b) {
			static const _locale_t m_liFileSystemLocale = _create_locale(LC_CTYPE, "");
			return _tcsicmp_l(a, b, m_liFileSystemLocale);
		}

		// Compare two files names using the file system comparison.
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
			for (int first = m_nPathRootLength; (end = m_strPath.Find('\\', first)) != -1; first = end + 1)
			{
				vPath.push_back( m_strPath.Mid(first, end - first) );
			}

			return vPath;
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

			// Replace '/' by '\\'.
			int nPos;
			while ((nPos = m_strPath.Find('/')) != -1)
				m_strPath.SetAt(nPos, '\\');

			// Never keep an ending '\'.
			if (!m_strPath.IsEmpty() && m_strPath[m_strPath.GetLength() - 1] == '\\')
				m_strPath = m_strPath.Left(m_strPath.GetLength() - 1);

			// Find the root.
			const int DRIVE_ROOT_LENGTH = 3; // = strlen("C:\\")
			const int UNICODE_ROOT_LENGTH = 4; // = strlen("\\\\?\\")
			const int UNICODE_ROOT_UNC_LENGTH = 8; // = strlen("\\\\?\\UNC\\")
			if (m_strPath.GetLength() >= UNICODE_ROOT_LENGTH && m_strPath.Left(UNICODE_ROOT_LENGTH) == _T("\\\\?\\"))
			{
				if (m_strPath.GetLength() >= UNICODE_ROOT_UNC_LENGTH && m_strPath.Left(UNICODE_ROOT_UNC_LENGTH) == _T("\\\\?\\UNC\\"))
				{
					int nPos = m_strPath.Find('\\', UNICODE_ROOT_UNC_LENGTH);
					if (nPos != -1)
						m_nPathRootLength = nPos + 1;					// `\\?\UNC\network\`
					else
						m_nPathRootLength = UNICODE_ROOT_UNC_LENGTH;	// `\\?\UNC\`
				}
				else
				{
					if (m_strPath.GetLength() >= UNICODE_ROOT_LENGTH+DRIVE_ROOT_LENGTH && m_strPath[UNICODE_ROOT_LENGTH+1] == ':' && m_strPath[UNICODE_ROOT_LENGTH+2] == '\\' && isalpha(m_strPath[UNICODE_ROOT_LENGTH]))
						m_nPathRootLength = UNICODE_ROOT_LENGTH+DRIVE_ROOT_LENGTH;	// `\\?\C:\`
					else
						m_nPathRootLength = DRIVE_ROOT_LENGTH;	// `\\?\`
				}
			}
			else
			{
				if (m_strPath.GetLength() >= DRIVE_ROOT_LENGTH && m_strPath[1] == ':' && m_strPath[2] == '\\' && isalpha(m_strPath[0]))
					m_nPathRootLength = DRIVE_ROOT_LENGTH;		// `C:\`
				else if (m_strPath.GetLength() >= 2 && m_strPath[0] == '\\' && m_strPath[1] == '\\')
				{
					int nPos = m_strPath.Find('\\', 2);
					if (nPos != -1)
						m_nPathRootLength = nPos + 1;				// `\\network\`
					else
						m_nPathRootLength = 0;
				}
				else
					m_nPathRootLength = 0;
			}

			// Split the path into components.
			m_nExtensionLength = 0;
			m_nFileNameFirst = m_strPath.GetLength();
			LPCTSTR pszPath = m_strPath;
			const TCHAR* pchEnd = &pszPath[m_strPath.GetLength()];
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
		int m_nPathRootLength;	// The length of the path root.
		int m_nFileNameFirst;	// First character of the file name (or the length of m_strPath if there is none).
		int m_nExtensionLength;	// The length of the extension including the "." (can be zero).
	};
}}}
