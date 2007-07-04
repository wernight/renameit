/**
 * Copyright (c) 2004 Werner BEROUX
 * Mail: werner@beroux.com
 * Web : www.beroux.com
 */
#include "StdAfx.h"
#include "FilterContainer.h"
#include "../resource.h"

// Include the different types of filters
#include "SearchReplaceFilter.h"

CFilterContainer::CFilterContainer(void)
{
}

CFilterContainer::CFilterContainer(const CFilterContainer& copy)
{
	m_nPathRenamePart = copy.m_nPathRenamePart;

	// Copy the filters.
	for (POSITION pos=copy.m_clFilters.GetHeadPosition(); pos!=NULL; )
		AddFilter(copy.m_clFilters.GetNext(pos));
}

CFilterContainer::~CFilterContainer(void)
{
	RemoveAllFilters();
}

void CFilterContainer::AddFilter(const IFilter* filter)
{
	// Create a copy of the filter.
	IFilter* pClone = NULL;
	if (filter->GetFilterCodeName() == _T("Search and replace"))
		pClone = new CSearchReplaceFilter();
	else
		ASSERT(FALSE);

	if (pClone != NULL)
	{
		CMapStringToString mapArgs;
		filter->GetArgs(mapArgs);
		pClone->SetArgs(mapArgs);

		// Add to the list.
		m_clFilters.AddTail(pClone);
	}
}

void CFilterContainer::RemoveFilter(int nFilterIndex)
{
	POSITION pos = m_clFilters.FindIndex(nFilterIndex);
	if (pos != NULL)
	{
		delete m_clFilters.GetAt(pos);
		m_clFilters.RemoveAt(pos);
	}
}

void CFilterContainer::RemoveAllFilters()
{
	for (POSITION pos=m_clFilters.GetHeadPosition(); pos!=NULL; )
		delete m_clFilters.GetNext(pos);
	m_clFilters.RemoveAll();
}

IFilter* CFilterContainer::GetFilter(int nFilterIndex) const
{
	// Search the filter
	POSITION pos = m_clFilters.FindIndex(nFilterIndex);
	if (pos == NULL)
		return NULL;	// Filter not found

	// Return filter
	return m_clFilters.GetAt(pos);
}

void CFilterContainer::UpdateFilter(int nFilterIndex, IFilter* filter)
{
	POSITION pos;
	pos = m_clFilters.FindIndex(nFilterIndex);
	if (pos != NULL)
		m_clFilters.SetAt(pos, filter);
	else
		ASSERT(false);
}

// Swap filter A with filter B.
BOOL CFilterContainer::SwapFilters(UINT nItemA, UINT nItemB)
{
	POSITION	posA = m_clFilters.FindIndex(nItemA),
				posB = m_clFilters.FindIndex(nItemB);

	if (posA == NULL || posB == NULL)
	{
		ASSERT(false);
		return FALSE;
	}

	if (nItemA == nItemB)
		return TRUE;

	// Swap both items
	IFilter *filter = m_clFilters.GetAt(posA);
	m_clFilters.RemoveAt(posA);
	if (nItemA < nItemB)
		m_clFilters.InsertAfter(posB, filter);
	else
		m_clFilters.InsertBefore(posB, filter);

	return TRUE;
}

BOOL CFilterContainer::SaveFilters(const CString &filename)
{
	// Check attributes is file already exist
	DWORD dwAttrib = GetFileAttributes(filename);
	if (dwAttrib != INVALID_FILE_ATTRIBUTES)		// File exist
	{
		if (dwAttrib & FILE_ATTRIBUTE_SYSTEM)		// System
		{
			AfxMessageBox(ID_OVERWRITE_SYSTEM_FILE, MB_ICONWARNING);
			return FALSE;
		}
		else if (dwAttrib & FILE_ATTRIBUTE_READONLY)// Read only
		{
			if (AfxMessageBox(ID_OVERWRITE_READONLY_FILE, MB_YESNO | MB_ICONQUESTION) == IDNO)
				return FALSE;
			// Remove read only attribute
			dwAttrib &= ~FILE_ATTRIBUTE_READONLY;
			VERIFY( SetFileAttributes(filename, dwAttrib & ~FILE_ATTRIBUTE_READONLY) );
		}
	}

	// Open file
	FILE	*file;
	if (_tfopen_s(&file, filename, _T("w")) != 0)
	{
		AfxMessageBox(ID_CANT_SAVE_FILE, MB_ICONERROR);
		return FALSE;
	}

	// Save the rename-what
	_ftprintf(file, _T("[General]\n"));
	{
		CString strRenameWhat;
		if (m_nPathRenamePart & CFilteredFileName::renameFolders)
			strRenameWhat += "Dir";
		if (m_nPathRenamePart & CFilteredFileName::renameFilename)
			strRenameWhat += "File";
		if (m_nPathRenamePart & CFilteredFileName::renameExtension)
			strRenameWhat += "Ext";
		_ftprintf(file, _T("RenameWhat=\"%s\"\n"), (LPCTSTR)strRenameWhat);
		_ftprintf(file, _T("\n"));
	}

	// Write data
	for (POSITION pos = m_clFilters.GetHeadPosition(); pos != NULL; )
	{
		IFilter *filter = m_clFilters.GetNext(pos);

		_ftprintf(file, _T("[%s]\n"), (LPCTSTR)filter->GetFilterCodeName());

		CMapStringToString mapArgs;
		filter->GetArgs(mapArgs);
		for (POSITION pos2 = mapArgs.GetStartPosition(); pos2 != NULL; )
		{
			CString strKey, strValue;
			mapArgs.GetNextAssoc(pos2, strKey, strValue);
			_ftprintf(file, _T("%s=\"%s\"\n"), (LPCTSTR) strKey, (LPCTSTR) strValue);
		}

		_ftprintf(file, _T("\n"));
	}

	fclose(file);
	return TRUE;
}

int CFilterContainer::LoadFilters(const CString &filename)
{
	TCHAR	line[4*1024];
	int		nFiltersAdded = 0;

	CMapStringToString mapArgs;
	IFilter *filter = NULL;
	CRenameWhat filterRenameWhat;

	// open file
	FILE	*file;
	if (_tfopen_s(&file, filename, _T("r")) != 0)
		return -1;

	// Read in one line...
	while (_fgetts(line, sizeof(line), file) != NULL)
	{
		if (lstrlen(line) == 0)
			continue;

		if (line[0] == _T('['))	// [type]
		{
			// Get the "type" in strType
			LPTSTR pos = _tcsrchr(line, _T(']'));
			if (pos == NULL)
			{
				// Invalid filter file
				// TODO: Display a warning message.
				continue;
			}
			*pos = _T('\0');
			CString strType = &line[1];

			// If previously we have loaded a filter,
			if (filter != NULL)
			{
				// Add the previously loaded filter to the list
				filter->SetArgs(mapArgs);
				if (filter != &filterRenameWhat)
				{
					m_clFilters.AddTail(filter);
					++nFiltersAdded;
				}
				filter = NULL;
				mapArgs.RemoveAll();
			}

			// Check if filter exist...
			if (strType == _T("General"))
				filter = &filterRenameWhat;
			else if (strType == _T("Search and replace"))
				filter = new CSearchReplaceFilter();
			else
			{
				// Unknown filter
				CString strMsg;
				AfxFormatString1(strMsg, IDS_UNKNOWN_FILTER, strType);
				AfxMessageBox(strMsg, MB_ICONWARNING);
			}
		}
		// Else if we are loading a filter's arguments
		else if (filter != NULL)	// key="value"
		{
			// Key
			LPTSTR pos = _tcschr(line, _T('='));
			if (pos == NULL)
			{
				// Invalid filter file
				// TODO: Display a warning message.
				continue;
			}
			*pos = _T('\0');
			CString strKey = line;

			// Value
			LPTSTR pszValueStart = _tcschr(&pos[1], _T('"'));
			if (pszValueStart == NULL)	// Start
			{
				// Invalid filter file
				// TODO: Display a warning message.
				continue;
			}
			LPTSTR pszValueEnd = _tcsrchr(&pszValueStart[1], _T('"'));
			if (pszValueEnd == NULL)	// End
			{
				// Invalid filter file
				// TODO: Display a warning message.
				continue;
			}
			*pszValueEnd = _T('\0');
			CString strValue = &pszValueStart[1];

			// Add argument
			mapArgs[strKey] = strValue;
		}
	}

	if (filter != NULL)
	{
		filter->SetArgs(mapArgs);
		if (filter != &filterRenameWhat)
		{
			m_clFilters.AddTail(filter);
			++nFiltersAdded;
		}
		filter = NULL;
		mapArgs.RemoveAll();
	}

	fclose(file);

	// Get the rename what part.
	if (filterRenameWhat.GetRenameWhat() > 0)
		m_nPathRenamePart = filterRenameWhat.GetRenameWhat();

	return nFiltersAdded;
}
