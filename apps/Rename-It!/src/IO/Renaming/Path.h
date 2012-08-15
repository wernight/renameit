#pragma once
#include <Shlwapi.h>	// Used for PathGetArgs(), PathIsDirectory(), and PathFileExists().

namespace Beroux{ namespace IO{ namespace Renaming
{ 
	/**
	 * Path to a file or a folder.
	 * 
	 * For a folder the ending backslash (\\) is removed, and
	 * the path's file name becomes the directory name, and the extension
	 * becomes the directory's "extension".
	 *
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
		inline bool IsUnicode() const {
			return IsUnicodePath(m_strPath);
		}

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
		 * The root is defined as the most parent part of the path in which
		 * no files or folders can be created/removed/renamed/...
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
		// Determines whether a path to a file system object such as a file or directory is valid and is already existing.
		static bool PathFileExists(const CString& strPath)
		{
			ASSERT(strPath.IsEmpty() || ::GetFileAttributes(strPath) != S_OK || ((::GetFileAttributes(strPath) & FILE_ATTRIBUTE_DIRECTORY) == 0));	// Doesn't fully support directories (yet).

			if (!strPath.IsEmpty() && strPath.GetAt(strPath.GetLength() - 1) == '.')	// No file or folder should end by a dot (.).
				return false;

			ASSERT(strPath.IsEmpty() || strPath.GetAt(strPath.GetLength() - 1) != '.');	// No file or folder should end by a dot (.).

			if (strPath.IsEmpty())
				return false;
			else
			{
				HANDLE hOpenFile = ::CreateFile(strPath,
					0,	// We only want to query without actually opening the file/directory.
					FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL, OPEN_EXISTING, 
					FILE_ATTRIBUTE_NORMAL,
					NULL);
				if (hOpenFile != INVALID_HANDLE_VALUE)
				{
					::CloseHandle(hOpenFile);
					return true;
				}
				else
				{
					switch (::GetLastError())
					{
					case ERROR_FILE_NOT_FOUND:	// i.e., the file does NOT exist.
					case ERROR_PATH_NOT_FOUND:
					case ERROR_INVALID_NAME:	// e.g., "C:\foo<*>bar" is not a valid file name (and therefor cannot exist).
					case ERROR_BAD_PATHNAME:
						return false;

					case ERROR_ACCESS_DENIED:
						// Fall-back on another method.
						if (strPath.GetLength() < MAX_PATH)
							return ::PathFileExists(strPath) != 0;
						else
						{// Here we assume the path doesn't have wildcards because it's valid.
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

					default:
#if _DEBUG
						DWORD dwErrorCode = ::GetLastError();
						ASSERT(false);	// Assert to try to find out when does this possibly happen.
#endif
						return true;	// It failed but not because the file wasn't found.
					}
				}
			}
		}

		/**
		 * Return the same path as provided with the case of the currently existing full file/folder path.
		 */
		static CString FindPathCase(const CString& strPathName)
		{
			CFileFind ff;

			// Directories should not end by '\' for FindFirstFileEx (or it would fail the test).
			BOOL bPathFound;
			if (strPathName[strPathName.GetLength() - 1] != '\\')
				bPathFound = ff.FindFile(strPathName);
			else
				bPathFound = ff.FindFile(strPathName.Left(strPathName.GetLength() - 1));

			if (!bPathFound)
			{
				ASSERT(false);
				return _T("");
			}

			ff.FindNextFile();
			return ff.GetFilePath();
		}

		/**
		 * Tell if an existing path is a directory.
		 */
		static bool IsDirectory(const CString& strPath)
		{
			CFileFind ff;

			// Directories should not end by '\' for FindFirstFileEx (or it would fail the test).
			BOOL bPathFound;
			if (strPath[strPath.GetLength() - 1] != '\\')
				bPathFound = ff.FindFile(strPath);
			else
				bPathFound = ff.FindFile(strPath.Left(strPath.GetLength() - 1));

			if (bPathFound)
			{
				ff.FindNextFile();
				return ff.IsDirectory() != 0;
			}
			else
				return false;
		}

		static inline bool IsUnicodePath(const CString& strPath)
		{
			return strPath.Left(4) == _T("\\\\?\\");
		}

		/**
		 * Returns the absolute path for the specified path string.
		 * The Unicode 32,000 characters path to be used for system functions.
		 */
		static inline CString MakeUnicodePath(const CString& strPath)
		{
			if (IsUnicodePath(strPath))
				return strPath;
			else
			{
				if (strPath.Left(2) == _T("\\\\"))
					return _T("\\\\?\\UNC\\") + strPath.Mid(2);
				else
					return _T("\\\\?\\") + strPath;
			}
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

		/**
		 * Return the path use to be displayed to the user.
		 */
		CString GetDisplayPath() const
		{
			return MakeSimplePath(GetPath());
		}

		friend ostream& operator <<(ostream& os, const CPath& value);

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
					{
						m_nPathRootLength = nPos + 1;					// `\\?\UNC\network\`
						int nPos = m_strPath.Find('\\', m_nPathRootLength);
						if (nPos != -1)
							m_nPathRootLength = nPos + 1;					// `\\?\UNC\network\share_name\`
					}
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
					{
						m_nPathRootLength = nPos + 1;				// `\\network\`
						int nPos = m_strPath.Find('\\', m_nPathRootLength);
						if (nPos != -1)
							m_nPathRootLength = nPos + 1;				// `\\network\share_name\`
					}
					else
						m_nPathRootLength = 0;
				}
				else
					m_nPathRootLength = 0;
			}

			// Never keep an ending '\' after directory names.
			if (!m_strPath.IsEmpty()
				&& m_strPath[m_strPath.GetLength() - 1] == '\\'
				&& m_strPath.GetLength() > m_nPathRootLength)
				m_strPath = m_strPath.Left(m_strPath.GetLength() - 1);

			// Split the path into components: base + filename + ext.
			m_nExtensionLength = 0;
			m_nFileNameFirst = m_strPath.GetLength();
			LPCTSTR pszPath = m_strPath;
			const TCHAR* pchEnd = &pszPath[m_strPath.GetLength()];
			for (const TCHAR* pch = pchEnd-1; pch >= pszPath; --pch)
			{
				switch (*pch)
				{
				case '.':
					// If the extension hasn't been set, and it's not the a filename starting by a dot (.).
					if (m_nExtensionLength == 0 && (pch == pszPath || *(pch - 1) != '\\'))
						m_nExtensionLength = (int)(pchEnd - pch);
					break;

				case '\\':
					m_nFileNameFirst = (int)(pch - pszPath) + 1;
					// Exit the for loop.
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

	ostream& operator <<(ostream& os, const CPath& value);
}}}
