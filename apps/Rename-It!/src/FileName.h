#pragma once

/**
 * A file name.
 */
class CFileName
{
// Construction
public:
	CFileName(void)
	{
	}

	CFileName(const CString& strFullPath)
	{
		SetTo(strFullPath);
	}

// Attributes
	inline const CString& GetFullPath() const {
		return m_strFullPath;
	}

	inline const CString& GetDrive() const {
		return m_strDrive;
	}

	inline const CString& GetDirectory() const {
		return m_strDirectory;
	}

	inline const CString& GetFileName() const {
		return m_strFileName;
	}

	inline const CString& GetExtension() const {
		return m_strExtension;
	}

// Operations
	// Compare two files names using the file system comparaison.
	static inline int FSCompare(const CString& a, const CString& b) {
		static const _locale_t m_liFileSystemLocale = _create_locale(LC_CTYPE, "");
		return _tcsicmp_l(a, b, m_liFileSystemLocale);
	}

	// Compare two files names using the file system comparaison.
	inline int FSCompare(const CFileName& other) const {
		static const _locale_t m_liFileSystemLocale = _create_locale(LC_CTYPE, "");
		return _tcsicmp_l(m_strFullPath, other.m_strFullPath, m_liFileSystemLocale);
	}

	inline int FSCompare(const CFileName& other, int nLength) const {
		static const _locale_t m_liFileSystemLocale = _create_locale(LC_CTYPE, "");
		return _tcsnicmp_l(m_strFullPath, other.m_strFullPath, nLength, m_liFileSystemLocale);
	}

	inline void SetTo(const CString& strFullPath)
	{
		// Save the full path.
		m_strFullPath = strFullPath;

		// Split the path into components.
		_tsplitpath_s((LPCTSTR)m_strFullPath,
			m_strDrive.GetBuffer(_MAX_DRIVE),
			_MAX_DRIVE,
			m_strDirectory.GetBuffer(_MAX_DIR),
			_MAX_DIR,
			m_strFileName.GetBuffer(_MAX_FNAME),
			_MAX_FNAME,
			m_strExtension.GetBuffer(_MAX_EXT),
			_MAX_EXT);
		m_strDrive.ReleaseBuffer();
		m_strDirectory.ReleaseBuffer();
		m_strFileName.ReleaseBuffer();
		m_strExtension.ReleaseBuffer();
	}

	inline operator const CString&() const {
		return m_strFullPath;
	}

	inline bool operator==(const CFileName& other) const {
		return m_strFullPath == other.m_strFullPath;
	}

	inline bool operator!=(const CFileName& other) const {
		return m_strFullPath != other.m_strFullPath;
	}

// Private
protected:
	CString	m_strFullPath;

	CString m_strDrive;
	CString m_strDirectory;
	CString	m_strFileName;
	CString m_strExtension;
};
