#pragma once
#include "IFilter.h"
#include "id3/tag.h"
#include "pcre.h"

class CSearchReplaceFilter :
	public IFilter
{
	friend class CSearchReplaceDlg;

// Construction
public:
	CSearchReplaceFilter(void);
	~CSearchReplaceFilter(void);

// IFilter implementation
// IFilter Operations
protected:// Shouldn't access to this because the OnStart/End have to be managed by FilterContainer.
	/** Filter the file name.
	 * @param originalFilename	Path to the original file name.
	 * @param filename			Current new file name, without extension.
	 * @param ext				Current new extension.
	 * @return True if file should be renamed.
	 */
	void FilterPath(const CFileName& fnOriginalFilename, CString& strFilename);
public:

	/** Display a window to configure the filter.
	 * @param originalFilename	Original sample file name.
	 * @param filename			Current sample file name.
	 * @param ext				Current sample file extension.
	 * @return The value returned by CDialog::DoModal().
	 */
	int ShowDialog(const CFileName& fnOriginalFilename, const CString& strFilename);

// IFilter Attributes
	// Return the unique name of the filter.
	CString GetFilterCodeName() const;

	// Return the human name of the filter in user language.
	CString GetFilterName() const;

	// User friendly description of the filter's actions over file's name.
	CString GetFilterDescription() const;

	// Return filter's parameters to be exported/saved.
	void GetArgs(CMapStringToString& mapArgs) const;

	// Load default filter's parameters.
	void LoadDefaultArgs();

	// Define some or all of filter's parameters (for import/load).
	void SetArgs(const CMapStringToString& mapArgs);

// Events
protected:
	virtual void OnStartRenamingList(ERenamePart nPathRenamePart);
	virtual void OnStartRenamingFile(const CFileName& fnPath, const CString& strName);
	virtual void OnEndRenamingFile();
	virtual void OnEndRenamingList();

// Attributes
public:
	enum EUse {
		useNone = -1,
		useRegExp,
		useWildcards,
	};

	enum EChangeCase {
		caseNone = -1,	// Don't change the case
		caseLower,
		caseUpper,
		caseSentense,
		caseWord,
		caseInvert,
	};

	CString GetSearch() const {
		return m_strSearch;
	}

	CString GetReplace() const {
		return m_strReplace;
	}

	bool IsCaseSensitive() const {
		return m_bCaseSensitive;
	}

	bool IsReplaceOnce() const {
		return m_bOnce;
	}

	bool IsMatchWholeText() const {
		return m_bMatchWholeText;
	}

	EUse GetUse() const {
		return m_nUse;
	}

	EChangeCase GetChangeCase() const {
		return m_nChangeCase;
	}

	CString GetLocale() const {
		return m_strLocale;
	}

	bool IsSeriesEnabled() const {
		return m_bSeries;
	}

	int GetSeriesStart() const {
		return m_nSeriesStart;
	}

	int GetSeriesStep() const {
		return m_nSeriesStep;
	}

	bool IsID3TagEnabled() const {
		return m_bID3Tag;
	}


	void SetSearch(const CString& strValue) {
		if (m_strSearch != strValue)
		{
			m_strSearch = strValue;
			CompileRegExp();
		}
	}

	void SetReplace(const CString& strValue) {
		m_strReplace = strValue;
	}

	void SetCaseSensitive(bool bValue) {
		if (m_bCaseSensitive != bValue)
		{
			m_bCaseSensitive = bValue;
			CompileRegExp();
		}
	}

	void SetReplaceOnce(bool bValue) {
		m_bOnce = bValue;
	}

	void SetMatchWholeText(bool bValue) {
		if (m_bMatchWholeText != bValue)
		{
			m_bMatchWholeText = bValue;
			CompileRegExp();
		}
	}

	void SetUse(EUse nValue) {
		if (m_nUse != nValue)
		{
			m_nUse = nValue;
			CompileRegExp();
		}
	}

	void SetChangeCase(EChangeCase nValue) {
		m_nChangeCase = nValue;
	}

	void SetLocale(const CString& strValue) {
		m_strLocale = strValue;
	}

	void SetSeriesEnabled(bool bValue) {
		m_bSeries = bValue;
	}

	void SetSeriesStart(int nValue) {
		m_nSeriesStart = nValue;
	}

	void SetSeriesStep(int nValue) {
		m_nSeriesStep = nValue;
	}

	void SetID3TagEnabled(bool bValue) {
		m_bID3Tag = bValue;
	}

// Implementation
private:
	// Filter's arguments (elments to export/import to save/load the filter)
	CString		m_strSearch;
	CString		m_strReplace;
	bool		m_bCaseSensitive;
	bool		m_bOnce;
	bool		m_bMatchWholeText;
	EUse		m_nUse;
	EChangeCase	m_nChangeCase;
	CString		m_strLocale;
	bool		m_bSeries;
	int			m_nSeriesStart;
	int			m_nSeriesStep;
	bool		m_bID3Tag;

	static bool AddID3TagMacros(const CFileName& fnOriginalFilename, CStringList& slMacros, CStringList& slValues);

	static void ReplaceInvalidChars(CString& strSubject);

	static void ReplaceSeries(CString& strReplace, int nCounterValue);

	unsigned FilterRegExp(const CString& strSubjectIn, CString& strSubjectOut, const CString& strReplace) const;

	static void EscapeRegExp(CString& str);

	/**
	 * Search and replace in the subject.
	 * @return True if strSubject has been modified.
	 */
	bool FilterString(CString& strSubject, const CString& strSearch, const CString& strReplace) const;

	static CString myMid(const CString &str, int nFirst);

	static CString myMid(const CString &str, int nFirst, int nCount);

	/** Replace all macros in string by their relative values.
	 */
	void ReplaceMacrosIn(CString &strInOut, const CStringList &slMacros, const CStringList &slValues) const;

	static CString GetID3TagValue(ID3_Tag &id3Tag, ID3_FrameID nid3ID);

	// Free the compiled RegExp.
	void RegExpFree();

	/**
	 * Compile the regular expression and store it in m_regexpCompiled and m_regexpExtra.
	 * It should be re-compiled each time on of thos is modified:
	 * - m_strSearch
	 * - m_nUse
	 * - m_bCaseSensitive
	 * - m_bMatchWholeText
	 * @return True on success. m_strRegExpCompileError contain an explanatory error in case of error.
	 */
	bool CompileRegExp();

	/**
	 * Change the case in strSubject between the two markers "\x06\x03\x05".
	 * The markers are removed after. If the markers are not found, nothing is changed.
	 * @param[in,out] strSubject	String in which to change the case (that contains the markers).
	 * @param[in] nCaseChangeOption	How to change the case.
	 * @return True if the markers have been found and the case change has been applied.
	 */
	static bool ChangeCase(CString& strSubject, EChangeCase nCaseChangeOption);

	static const int CASE_MARKER_LENGTH;
	static const LPCTSTR CASE_MARKER;

	// Variables used to improve speed by keeping the compiled RegExp.
	pcre*		m_regexpCompiled;
	pcre_extra*	m_regexpExtra;
	CString		m_strRegExpCompileError;

	// Variables used during the process of renaming a list of files.
	CFileName	m_fnOriginalFile;
	CString		m_strUnfilteredName;
	int			m_nSeriesCounter;
};
