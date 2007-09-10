#include "StdAfx.h"
#include "SearchReplaceFilter.h"
#include "SearchReplaceDlg.h"
#include "id3/readers.h"

// ID3Lib fix: END_OF_READER unreferenced
const ID3_Reader::int_type ID3_Reader::END_OF_READER = std::char_traits<ID3_Reader::int_type>::eof();

const LPCTSTR CSearchReplaceFilter::CASE_MARKER = _T("\x06\x03\x05");
const int CSearchReplaceFilter::CASE_MARKER_LENGTH = (int) _tcslen(CASE_MARKER);

const LPCTSTR CSearchReplaceFilter::MACRO_MARK = _T("\x07\x03\x05");
const int CSearchReplaceFilter::MACRO_MARK_LENGTH = (int) _tcslen(MACRO_MARK);
const int CSearchReplaceFilter::MACRO_CODE_LENGTH = 2;

CSearchReplaceFilter::CSearchReplaceFilter(void) :
	m_regexpCompiled(NULL),
	m_regexpExtra(NULL),
	m_nSeriesCounter(0)
{
	LoadDefaultArgs();
}

CSearchReplaceFilter::~CSearchReplaceFilter(void)
{
	RegExpFree();
}

void CSearchReplaceFilter::FilterPath(CString& strFilename, const CPath& fnOriginalFilename, const CString& strUnfilteredName)
{
	bool bMatchesSearch = false;	// True when the string to filter matches the search pattern.
	CString strvMacroValues[macrosCount];

	// ID3 Tag mode?
	if (m_bID3Tag)
	{
		if (!AddID3TagMacros(fnOriginalFilename, strvMacroValues))
			return;
	}

	// When search at replace are empty, nothing is replaced.
	if (m_strSearch.IsEmpty() && m_strReplaceCompiled.IsEmpty())
	{
		bMatchesSearch = true;

		// If search and replace are empty but change case is activated
		if (m_nChangeCase != caseNone)
		{
			// Add markers in the replacement string.
			CString strResult = CASE_MARKER + strFilename + CASE_MARKER;
			if (ChangeCase(strResult, m_nChangeCase))
				strFilename = strResult;
		}
	}
	else
	{
		// Prepare the replacement string.
		CString	strReplace = m_strReplaceCompiled;
		{
			// Number series.
			if (m_bSeries)
				// Replace %d by 123 in strReplace.
				ReplaceSeries(strReplace, m_nSeriesCounter);

			// Replace special macros.
			if (m_nReplaceMacrosFlags != 0)
			{
				if (m_nReplaceMacrosFlags & 1<<macroDirectoryName)
					strvMacroValues[macroDirectoryName] = fnOriginalFilename.GetDirectoryName();
				if (m_nReplaceMacrosFlags & 1<<macroFileName)
					strvMacroValues[macroFileName] = fnOriginalFilename.GetFileNameWithoutExtension();
				if (m_nReplaceMacrosFlags & 1<<macroExtension)
					strvMacroValues[macroExtension] = fnOriginalFilename.GetExtension();
				if (m_nReplaceMacrosFlags & 1<<macroUnfilteredName)
					strvMacroValues[macroUnfilteredName] = strUnfilteredName;
				if (m_nReplaceMacrosFlags & 1<<macroFilteredName)
					strvMacroValues[macroFilteredName] = strFilename;

				// When using regexp...
				switch (m_nUse)
				{
				case useWildcards:	// Wildcard
				case useRegExp:	// RegExp
					// ... escape all the macro (as the rest has been escaped in PrecompileReplace().
					for (int i=0; i<macrosCount; ++i)
						if (m_nReplaceMacrosFlags & 1<<i)
							EscapeRegExp(strvMacroValues[i]);
					break;
				}

				ReplaceMacrosIn(strReplace, strvMacroValues);
			}

			// Add markers in the replacement string.
			if (m_nChangeCase != caseNone)
				strReplace = CASE_MARKER + strReplace + CASE_MARKER;
		}

		// Replace search by replace in file name.
		switch (m_nUse)
		{
		case useWildcards:	// Wildcard
		case useRegExp:	// RegExp
			{
				CString strOutput = strFilename;
				if (FilterRegExp(strFilename, strOutput, strReplace) > 0)
				{// when the search expression is matches.
					strFilename = strOutput;
					bMatchesSearch = true;
				}
			}
			break;

		default:	// General search & replace
			{
				// Prepare the search string.
				CString strSearch = m_strSearch;
				// If search string is empty.
				if (strSearch.IsEmpty())
				{
					// Replace the whole file name.
					ASSERT(m_nUse == useNone);
					strSearch = strFilename;
				}

				if (FilterString(strFilename, strSearch, strReplace))
				{// when the search expression is matches.
					bMatchesSearch = true;
				}
			}
		}

		// Change the case in the replaced string.
		if (m_nChangeCase != caseNone)
		{
			if (ChangeCase(strFilename, m_nChangeCase))
				bMatchesSearch = true;
		}
	}

	// Number series
	if (m_bSeries && bMatchesSearch)
	{
		// Increment counter's value
		m_nSeriesCounter += m_nSeriesStep;
	}
}

void CSearchReplaceFilter::OnStartRenamingList()
{
	m_nSeriesCounter = m_nSeriesStart;
}

void CSearchReplaceFilter::OnEndRenamingList()
{
}

bool CSearchReplaceFilter::AddID3TagMacros(const CPath& fnOriginalFilename, CString strvMacroValues[]) const
{
	// Load ID3 tag info.
#if (defined _UNICODE || defined UNICODE)
	// A workaround in VC7
	FILE* f = NULL;
	if (_wfopen_s(&f, fnOriginalFilename.GetFullPath(), _T("rb")) != 0)
		return false;	// File not found.
	ifstream stm(f);
#else
	ifstream stm(fnOriginalFilename.GetFullPath(), ios_base::in | ios_base::binary);
#endif
	ID3_IFStreamReader id3Reader(stm);

	ID3_Tag	id3Tag;
	id3Tag.Link(id3Reader);

	// No ID3 tag?
	if (!id3Tag.HasV1Tag() && !id3Tag.HasV2Tag())
	{
#ifdef _UNICODE
		fclose(f);
#endif
		return false;		// then don't change the file name
	}

	// Add to the list of Macros -> Values.
	if (m_nReplaceMacrosFlags & 1<<macroID3Track)
	{
		int nTrackNum = _ttoi(GetID3TagValue(id3Tag, ID3FID_TRACKNUM));
		CString strTrackNum;
		strTrackNum.Format(_T("%d"), nTrackNum);
		strvMacroValues[macroID3Track] = strTrackNum;
	}

	if (m_nReplaceMacrosFlags & 1<<macroID3Artist)
		strvMacroValues[macroID3Artist] = GetID3TagValue(id3Tag, ID3FID_LEADARTIST);

	if (m_nReplaceMacrosFlags & 1<<macroID3Title)
		strvMacroValues[macroID3Title] = GetID3TagValue(id3Tag, ID3FID_TITLE);

	if (m_nReplaceMacrosFlags & 1<<macroID3Album)
		strvMacroValues[macroID3Album] = GetID3TagValue(id3Tag, ID3FID_ALBUM);

	if (m_nReplaceMacrosFlags & 1<<macroID3Year)
		strvMacroValues[macroID3Year] = GetID3TagValue(id3Tag, ID3FID_YEAR);

	if (m_nReplaceMacrosFlags & 1<<macroID3Comment)
		strvMacroValues[macroID3Comment] = GetID3TagValue(id3Tag, ID3FID_COMMENT);

	if (m_nReplaceMacrosFlags & 1<<macroID3Genre)
		strvMacroValues[macroID3Genre] = GetID3TagValue(id3Tag, ID3FID_CONTENTTYPE);

#ifdef _UNICODE
	fclose(f);
#endif
	return true;
}

void CSearchReplaceFilter::ReplaceInvalidChars(CString& strSubject)
{
	for (int i=0; i<strSubject.GetLength(); ++i)
		switch (strSubject[i])
		{
		case '/':
		case '\\':
		case '|':
			strSubject.SetAt(i, '-');
			break;

		case '*':
		case '?':
			strSubject.SetAt(i, '_');
			break;

		case '<':
			strSubject.SetAt(i, '(');
			break;

		case '>':
			strSubject.SetAt(i, ')');
			break;

		case ':':
			// Replace "xxx: xxx" by "xxx - xxx", but other cases.
			if (i > 0 && strSubject[i-1] != ' ' &&
				i+1 < strSubject.GetLength() && strSubject[i+1] == ' ')
			{
				strSubject.Insert(i++, ' ');
			}
			strSubject.SetAt(i, '-');
			break;

		case '"':
			strSubject.SetAt(i, '\'');
			strSubject.Insert(i++, '\'');
			break;
		}
}

void CSearchReplaceFilter::ReplaceSeries(CString& strReplace, int nCounterValue)
{
	// Check the format string
	for (int p=0; (p = strReplace.Find('%', p)) != -1; p+=2)
	{
		int n = p+1;

		// Escaped %?
		if (strReplace[n] == '%')
			continue;

		// Flags
		if (strReplace[n] == '-' || strReplace[n] == '+' || strReplace[n] == '0' || strReplace[n] == ' ')
			++n;

		// Width
		while (strReplace[n] >= '0' && strReplace[n] <= '9')
			++n;
		
		// Type
		if (strReplace[n] != 'd')
		{
			// Invalide %, so escape it
			strReplace.Insert(p, '%');
			continue;
		}

		// The format of this %d is correct

		// Escape any further un-escaped % (% -> %%)
		while ((n = strReplace.Find('%', n)) != -1)
		{
			if (strReplace[++n] != '%')
				strReplace.Insert(n, '%');
			++n;
		}
		break;
	}

	// Replace %d by the counter's value
	strReplace.Format(strReplace, nCounterValue);
}

int CSearchReplaceFilter::ShowDialog(IPreviewFileList& previewSamples)
{
	CSearchReplaceDlg dlg(*this, previewSamples);
	return (int) dlg.DoModal();
}

// Return the unique name of the filter.
CString CSearchReplaceFilter::GetFilterCodeName() const
{
	return _T("Search and replace");
}

// Return the human name of the filter in user language.
CString CSearchReplaceFilter::GetFilterName() const
{
	CString strName;
	strName.LoadString(IDS_SEARCH_AND_REPLACE);
	return strName;
}

// User friendly description of the filter's actions over file's name.
CString CSearchReplaceFilter::GetFilterDescription() const
{
	CString ruleDescr,
			strBuffer;

	if (m_bID3Tag)
		ruleDescr.LoadString(IDS_DESCR_ID3);
	else
		ruleDescr.Empty();

	// + "Replace"
	switch (m_nUse)
	{
	case useRegExp:	// RegExp
		strBuffer.LoadString(IDS_DESCR_REPLACE_REGEXP);
		break;

	case useWildcards:	// Wildcard
		strBuffer.LoadString(IDS_DESCR_REPLACE_WILDCARD);
		break;

	default:
		strBuffer.LoadString(IDS_DESCR_REPLACE);
	}
	ruleDescr += strBuffer;

	// + SEARCH
	if (!m_strSearch.IsEmpty())
		ruleDescr += _T(" \"") + m_strSearch + _T("\"");

	// + "width" + REPLACE
	strBuffer.LoadString(IDS_DESCR_WITH);
	ruleDescr += _T(" ") + strBuffer + _T(" \"") +  m_strReplace + _T("\"");

	ruleDescr += _T(" [");
	if (m_bCaseSensitive)
		strBuffer.LoadString(IDS_DESCR_MATCH_CASE);
	else
		strBuffer.LoadString(IDS_DESCR_SKIP_CASE);
	ruleDescr += strBuffer;
	if (m_bOnce)
	{
		strBuffer.LoadString(IDS_DESCR_ONCE);
		ruleDescr += _T(", ") + strBuffer;
	}
	if (m_bMatchWholeText)
	{
		strBuffer.LoadString(IDS_DESCR_WHOLE_TEXT);
		ruleDescr += _T(", ") + strBuffer;
	}
	if (m_bSeries)
	{
		strBuffer.LoadString(IDS_DESCR_SERIES);
		ruleDescr += _T(", ") + strBuffer;
	}
	ruleDescr += _T("]");

	// TODO: nChangeCase

	return ruleDescr;
}

// Return filter's parameters to be exported/saved.
void CSearchReplaceFilter::GetArgs(CMapStringToString& mapArgs) const
{
	mapArgs[_T("search")] = m_strSearch;
	mapArgs[_T("replace")] = m_strReplace;
	mapArgs[_T("casesensitive")].Format(_T("%d"), m_bCaseSensitive);
	mapArgs[_T("once")].Format(_T("%d"), m_bOnce);
	mapArgs[_T("matchWholeText")].Format(_T("%d"), m_bMatchWholeText);
	mapArgs[_T("use")].Format(_T("%d"),  m_nUse);
	mapArgs[_T("changeCase")].Format(_T("%d"),  m_nChangeCase);
	mapArgs[_T("series")].Format(_T("%d"),  m_bSeries);
	mapArgs[_T("seriesStart")].Format(_T("%d"),  m_nSeriesStart);
	mapArgs[_T("seriesStep")].Format(_T("%d"),  m_nSeriesStep);
	mapArgs[_T("id3tag")].Format(_T("%d"),  m_bID3Tag);
}

// Load default filter's parameters.
void CSearchReplaceFilter::LoadDefaultArgs()
{
	m_strSearch.Empty();
	m_strReplace.Empty();
	m_bOnce = false;
	m_bCaseSensitive = true;
	m_bMatchWholeText = false;
	m_nUse = useWildcards;
	m_nChangeCase = caseNone;
	m_bSeries = false;
	m_nSeriesStart = 1;
	m_nSeriesStep = 1;
	m_bID3Tag = false;

	// Re-compile the regexp.
	CompileRegExp();
	PrecompileReplace();
}

// Define some or all of filter's parameters (for import/load).
void CSearchReplaceFilter::SetArgs(const CMapStringToString& mapArgs)
{
	CString strValue;

	if (mapArgs.Lookup(_T("search"), strValue))
		m_strSearch = strValue;

	if (mapArgs.Lookup(_T("replace"), strValue))
		m_strReplace = strValue;

	if (mapArgs.Lookup(_T("casesensitive"), strValue))
		m_bCaseSensitive = _ttoi( (LPCTSTR)strValue ) != 0;

	if (mapArgs.Lookup(_T("once"), strValue))
		m_bOnce = _ttoi( (LPCTSTR)strValue ) != 0;

	if (mapArgs.Lookup(_T("matchWholeText"), strValue))
		m_bMatchWholeText = _ttoi( (LPCTSTR)strValue ) != 0;

	if (mapArgs.Lookup(_T("use"), strValue))
	{
		int nUse = _ttoi( (LPCTSTR)strValue );
		if (nUse >= useNone && nUse <= useWildcards)
			m_nUse = (EUse) nUse;
	}

	if (mapArgs.Lookup(_T("changeCase"), strValue))
	{
		int nChangeCase = _ttoi( (LPCTSTR)strValue );
		if (nChangeCase >= caseNone && caseInvert <= caseInvert)
			m_nChangeCase = (EChangeCase) nChangeCase;
	}

	if (mapArgs.Lookup(_T("series"), strValue))
		m_bSeries = _ttoi( (LPCTSTR) strValue ) != 0;

	if (mapArgs.Lookup(_T("seriesStart"), strValue))
		m_nSeriesStart = _ttoi( (LPCTSTR)strValue );

	if (mapArgs.Lookup(_T("seriesStep"), strValue))
		m_nSeriesStep = _ttoi( (LPCTSTR)strValue );

	if (mapArgs.Lookup(_T("id3tag"), strValue))
		m_bID3Tag = _ttoi( (LPCTSTR)strValue ) != 0;

	// Re-compile the regexp.
	CompileRegExp();

	// Precompile the macros.
	PrecompileReplace();
}

void CSearchReplaceFilter::RegExpFree()
{
	// Free the PCRE compiled RegExp.
	if (m_regexpCompiled != NULL)
	{
		pcre_free(m_regexpCompiled);
		m_regexpCompiled = NULL;
	}
	if (m_regexpExtra != NULL)
	{
		pcre_free(m_regexpExtra);
		m_regexpExtra = NULL;
	}

	// Empty the error string.
	m_strRegExpCompileError.Empty();
}

bool CSearchReplaceFilter::CompileRegExp()
{
	// Free the previous one.
	RegExpFree();

	// Are we going to use PCRE?
	if (m_nUse == useNone)
		return true;

	CString strSearch = m_strSearch;

	// If search string is empty.
	if (strSearch.IsEmpty())
	{
		// Replace the whole file name.
		strSearch = _T(".*");
	}
	else
	{
		// If we use Wildcards, we convert them to RegExp.
		if (m_nUse == useWildcards)
		{
			// Change the Wildcard into a RegExp.
			EscapeRegExp(strSearch);
			strSearch.Replace(_T("\\?"), _T("."));
			strSearch.Replace(_T("\\*"), _T("(.*)"));

			// Match whole text?
			if (m_bMatchWholeText)
				strSearch = '^' + strSearch + '$';
		}
	}

	// Compile regexp...
	int	 nOptions = 0;
	if (!m_bCaseSensitive)
		nOptions |= PCRE_CASELESS;

	const char *pchError;
	int nErrorOffset;

	CHAR	szPattherA[1024];
#ifdef _UNICODE
	szPattherA[WideCharToMultiByte(CP_ACP, 0, strSearch, -1, szPattherA, sizeof(szPattherA)/sizeof(szPattherA[0]), NULL, NULL)] = '\0';
#else
	strcpy_s(szPattherA, strSearch);
#endif
	m_regexpCompiled = pcre_compile(szPattherA, nOptions, &pchError, &nErrorOffset, NULL);
	if (m_regexpCompiled == NULL)
	{
		m_strRegExpCompileError = _T("*** RegExp Error: ");
		m_strRegExpCompileError += pchError;
	}
	else
		m_regexpExtra = pcre_study(m_regexpCompiled, 0, &pchError);

	return m_regexpCompiled != NULL && m_regexpExtra != NULL;
}

void CSearchReplaceFilter::PrecompileReplace()
{
	// Create a copy of the replacement string.
	m_strReplaceCompiled = m_strReplace;

	// Pre-compile macros.
	m_nReplaceMacrosFlags = 0;
	for (int nMacroCode=0; nMacroCode<macrosCount; ++nMacroCode)
	{
		CString strMacroName;
		switch (nMacroCode)
		{
		// General
		case macroDirectoryName:	strMacroName = _T("$(FileDir)"); break;
		case macroFileName:			strMacroName = _T("$(FileName)"); break;
		case macroExtension:		strMacroName = _T("$(FileExt)"); break;
		case macroUnfilteredName:	strMacroName = _T("$(UnfilteredName)"); break;
		case macroFilteredName:		strMacroName = _T("$(FilteredName)"); break;
		// ID3
		case macroID3Track:		strMacroName = _T("$(ID3Track)"); break;
		case macroID3Artist:	strMacroName = _T("$(ID3Artist)"); break;
		case macroID3Title:		strMacroName = _T("$(ID3Title)"); break;
		case macroID3Album:		strMacroName = _T("$(ID3Album)"); break;
		case macroID3Year:		strMacroName = _T("$(ID3Year)"); break;
		case macroID3Comment:	strMacroName = _T("$(ID3Comment)"); break;
		case macroID3Genre:		strMacroName = _T("$(ID3Genre)"); break;

		default:
			ASSERT(false);
			continue;
		}

		// Search & replace macros by MACRO_MARK+code.
		CString strCompiledMacro= MACRO_MARK;
		strCompiledMacro.AppendFormat(_T("%02d"), nMacroCode);
		if (m_strReplaceCompiled.Replace(strMacroName, strCompiledMacro) > 0)
		{
			// Flag that macro.
			m_nReplaceMacrosFlags |= 1<<nMacroCode;
		}
	}

	// Pre-compile wildcards.
	if (m_nUse == useWildcards)
	{
		// If we use Wildcards, we convert them to RegExp.
		// Change the Wildcard into a RegExp.
		EscapeRegExp(m_strReplaceCompiled);
		int nCapture = 1;
		for (int i=0; i<m_strReplaceCompiled.GetLength(); ++i)
		{
			if (m_strReplaceCompiled[i] == '*')
			{
				m_strReplaceCompiled.SetAt(i, '0' + nCapture);
				if (++nCapture > 9)
					break;
			}
		}
	}
}

unsigned CSearchReplaceFilter::FilterRegExp(const CString &strIn, CString& strOut, const CString &strReplace) const
{
	if (m_regexpCompiled == NULL)
	{
		strOut = m_strRegExpCompileError;
		return true;
	}

	// UGLY CODE MODE ON
	unsigned	nReplacements = 0;
	const int	OFFSETS_SIZE = 3*10;
	int			nvOffsets[OFFSETS_SIZE];

	if (m_bOnce)
	{// Replace once.
		CHAR	szInA[1024];
#ifdef _UNICODE
		szInA[WideCharToMultiByte(CP_ACP, 0, strIn, -1, szInA, sizeof(szInA)/sizeof(szInA[0]), NULL, NULL)] = '\0';
#else
		strcpy_s(szInA, strIn);
#endif
		int nCount = pcre_exec(m_regexpCompiled, m_regexpExtra, szInA, (int)strlen(szInA)/sizeof(szInA[0]), 0, 0, nvOffsets, OFFSETS_SIZE);
		if (nCount > 0)
		{
			// We found something
			++nReplacements;
			strOut = strIn.Left(nvOffsets[0]);

			// Replace all \0 \1 \2... in strReplace and replace them by the captured value,
			// \\ by \, and \x by x.
			int nStart = 0;
			for (int nFindPos, nPrevFindPos = 0; (nFindPos = strReplace.Find(_T('\\'), nPrevFindPos)) != -1; )
			{
				if (nFindPos+1 < strReplace.GetLength())
				{
					// Copy what's before the \.
					strOut += strReplace.Mid(nStart, nFindPos - nStart);

					TCHAR chr = strReplace[nFindPos+1];
					if (isdigit(chr))
					{// Replace \1 by the captured expression N°1.
						int index = chr - _T('0');
						if (index < nCount &&
							nvOffsets[index*2] != -1)
						{
							strOut += strIn.Mid(nvOffsets[index*2], nvOffsets[index*2+1] - nvOffsets[index*2]);
						}
						else
							strOut += _T("?");

						nStart = nFindPos + 2;
					}
					else
					{// Replace \X by X.
						nStart = nFindPos + 1;
					}

					nPrevFindPos = nFindPos + 2;
				}
				else
					nPrevFindPos = nFindPos + 1;
			}
			strOut += strReplace.Mid(nStart);

			// Add what's after the end of this substitution.
			strOut += strIn.Mid(nvOffsets[1]);
		}
	}
	else
	{// Replace as many as possible.
		CString strTmpIn = strIn;
		strOut.Empty();

		while (true)
		{
			CHAR	szInA[1024];
#ifdef _UNICODE
			szInA[WideCharToMultiByte(CP_ACP, 0, strTmpIn, -1, szInA, sizeof(szInA)/sizeof(szInA[0]), NULL, NULL)] = '\0';
#else
			strcpy_s(szInA, strTmpIn);
#endif
			int nCount = pcre_exec(m_regexpCompiled, m_regexpExtra, szInA, strTmpIn.GetLength(), 0, 0, nvOffsets, OFFSETS_SIZE);
			if (nCount <= 0 || nvOffsets[0] == nvOffsets[1])
				break;

			// We found something
			++nReplacements;
			strOut += strTmpIn.Left(nvOffsets[0]);

			// Replace all \0 \1 \2... in strReplace and replace them by the captured value.
			// \\ by \, and \x by x.
			int nStart = 0;
			for (int nFindPos, nPrevFindPos = 0; (nFindPos = strReplace.Find(_T('\\'), nPrevFindPos)) != -1; )
			{
				if (nFindPos+1 < strReplace.GetLength())
				{
					// Copy what's before the \.
					strOut += strReplace.Mid(nStart, nFindPos - nStart);

					TCHAR chr = strReplace[nFindPos+1];
					if (isdigit(chr))
					{// Replace \1 by the captured expression N°1.
						int index = chr - _T('0');
						if (index < nCount &&
							nvOffsets[index*2] != -1)
						{
							strOut += strTmpIn.Mid(nvOffsets[index*2], nvOffsets[index*2+1] - nvOffsets[index*2]);
						}
						else
							strOut += _T("?");

						nStart = nFindPos + 2;
					}
					else
					{// Replace \X by X.
						nStart = nFindPos + 1;
					}

					nPrevFindPos = nFindPos + 2;
				}
				else
					nPrevFindPos = nFindPos + 1;
			}
			strOut += strReplace.Mid(nStart);

			// Continue after the end of this substitution.
			strTmpIn = strTmpIn.Mid(nvOffsets[1]);
		}
		strOut += strTmpIn;
	}

	return nReplacements;
}

void CSearchReplaceFilter::EscapeRegExp(CString& str)
{
	// The special regular expression characters are:
	// . \ + * ? [ ^ ] $ ( ) { } = ! < > | :
	for (int i=0; i<str.GetLength(); ++i)
		switch (str[i])
		{
		case '.': case '\\': case '+': case '*':
		case '?': case '[':  case '^': case ']': 
		case '$': case '(':  case ')': case '{':
		case '}': case '=': case '!':  case '<':
		case '>': case '|':
			str.Insert(i++, '\\');
		}
}

bool CSearchReplaceFilter::FilterString(CString& strSubject, const CString& strSearch, const CString& strReplace) const
{
	if (m_bMatchWholeText)
	{
		if (strSubject.CompareNoCase(strSearch) == 0)
		{
			strSubject = strReplace;
			return true;
		}
		else
			return false;
	}
	else
	{
		CString strCaseSubject = strSubject;
		CString strCaseSearch = strSearch;

		// Case insensitive search?
		if (!m_bCaseSensitive)
		{
			strCaseSubject.MakeLower();
			strCaseSearch.MakeLower();
		}

		bool bReplacedString = false;
		CString strOutput;
		int nStart = 0;
		while (true)
		{
			// Search
			int nFoundPos = strCaseSubject.Find(strCaseSearch, nStart);
			if (nFoundPos == -1)
				break;
			// Replace
			bReplacedString = true;
			strOutput += strSubject.Mid(nStart, nFoundPos-nStart);
			strOutput += strReplace;
			nStart = nFoundPos + strCaseSearch.GetLength();
			// Only replace once
			if (m_bOnce)
				break;
		}
		strOutput += strSubject.Mid(nStart);

		strSubject = strOutput;
		return bReplacedString;
	}
}

void CSearchReplaceFilter::ReplaceMacrosIn(CString& strContent, const CString strvMacroValues[]) const
{
	int nStart = 0;
	while (true)
	{
		int nMacroPos = strContent.Find(MACRO_MARK, nStart);
		if (nMacroPos == -1 || nMacroPos + 2 >= strContent.GetLength())
			break;
		
		// Get the macro code.
		int nMacroCode = (strContent[nMacroPos+MACRO_MARK_LENGTH] - '0')*10 + (strContent[nMacroPos+MACRO_MARK_LENGTH+1] - '0');
		if (nMacroCode < 0 || nMacroCode >= macrosCount)
			continue;	// Invalid macro code (hack?)
		if (((m_nReplaceMacrosFlags>>nMacroCode) & 1) != 1)
			continue;	// Invalid macro mark (hack?)

		// Replace by the macro's value.
		strContent = strContent.Left(nMacroPos) + strvMacroValues[nMacroCode] + strContent.Mid(nMacroPos+MACRO_MARK_LENGTH+MACRO_CODE_LENGTH);

		// Go to the next possible macro.
		nStart = nMacroPos + strvMacroValues[nMacroCode].GetLength();
	}
}

CString CSearchReplaceFilter::GetID3TagValue(ID3_Tag &id3Tag, ID3_FrameID nid3ID)
{
	// Don't free frame or field with delete. 
	// id3lib will do that for us...
	ID3_Frame* pid3Frame = id3Tag.Find(nid3ID);
	if (pid3Frame == NULL)
		return _T(""); // That's bad!

	ID3_Field* pid3Field = pid3Frame->GetField(ID3FN_TEXT);
	if (pid3Field == NULL)
		return _T(""); // That's bad!

	// TODO: Unicode strings are not correctly translated. Use pid3Field->GetRawUnicodeText() with encoding, or another lib.
	CString strTagValue;
	strTagValue = pid3Field->GetRawText();
	ReplaceInvalidChars(strTagValue);
	return strTagValue;
}

bool CSearchReplaceFilter::ChangeCase(CString& strSubject, EChangeCase nCaseChangeOption)
{
	// Search for the markers.
	int nStartMarker = strSubject.Find(CASE_MARKER);
	int nEndMarker = nStartMarker;
	do
	{
		int nFound = strSubject.Find(CASE_MARKER, nEndMarker+1);
		if (nFound == -1)
			break;
		nEndMarker = nFound;
	} while (true);
	if (nStartMarker == -1 || nStartMarker == nEndMarker)
		return false;

	// Change the case between the markers.
	CString strChangeCaseIn = strSubject.Mid(nStartMarker+CASE_MARKER_LENGTH, nEndMarker-nStartMarker-CASE_MARKER_LENGTH);
	LPTSTR szString = strChangeCaseIn.GetBuffer();
	switch (nCaseChangeOption)
	{
	case caseLower: // LOWER
		_tcslwr(szString);
		break;

	case caseUpper: // UPPER
		_tcsupr(szString);
		break;

	case caseSentense: // SENTENSE
		_tcslwr(szString);
		szString[0] = _totupper(szString[0]);
		break;

	case caseWord: // WORD
		{
			_tcslwr(szString);
			bool inWord = false;

			for (int i=0; i<(int)_tcsclen(szString); i++)
			{
				switch (szString[i])
				{
				case _T(' '): case _T('.'): case _T(','): case _T('_'): case _T('-'):
				case _T('+'): case _T('~'): case _T('#'): case _T('='): case _T('&'):
				case _T('('): case _T(')'):
				case _T('['): case _T(']'):
				case _T('{'): case _T('}'):
					inWord = false;
					break;
				default:
					if (!inWord)
					{
						szString[i] = _totupper(szString[i]);
						inWord = true;
					}
				}
			}
		}
		break;

	case caseInvert:	// INVERT
		for (int i=0; i<(int)_tcsclen(szString); i++)
		{
			// If it's not an ASCII
//			if (_istascii(szString[i]))	// tolower/upper won't return right value for non-ascii
				if (_istupper(szString[i]))
					szString[i] = _totlower(szString[i]);
				else
					szString[i] = _totupper(szString[i]);
		}
		break;

	default:
		ASSERT(false);
	}

	strChangeCaseIn.ReleaseBuffer();

	// Remove the markers and replace them by the new string.
	strSubject = strSubject.Left(nStartMarker) + strChangeCaseIn + strSubject.Mid(nEndMarker+CASE_MARKER_LENGTH);
	return true;
}
