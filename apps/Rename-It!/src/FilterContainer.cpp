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


namespace Beroux{ namespace IO{ namespace Renaming{ namespace Filter
{

CFilterContainer::CFilterContainer(void)
{
}

CFilterContainer::CFilterContainer(const CFilterContainer& copy)
{
	m_nPathRenamePart = copy.m_nPathRenamePart;

	// Copy the filters.
	for (const_iterator iter=copy.m_vFilters.begin(); iter!=copy.m_vFilters.end(); ++iter)
		AddFilter(*iter);
}

CFilterContainer::~CFilterContainer(void)
{
	RemoveAllFilters();
}

void CFilterContainer::AddFilter(const IFilter* filter)
{
	// Add to the list.
	m_vFilters.push_back(CloneFilter(filter));
}

void CFilterContainer::RemoveFilter(int nFilterIndex)
{
	iterator at = m_vFilters.begin() + nFilterIndex;
	delete *at;
	m_vFilters.erase(at);
}

void CFilterContainer::RemoveAllFilters()
{
	BOOST_FOREACH(IFilter* pFilter, m_vFilters)
	{
		delete pFilter;
	}

	m_vFilters.clear();
}

shared_ptr<IFilter> CFilterContainer::GetFilterAt(int nFilterIndex) const
{
	return shared_ptr<IFilter>( CloneFilter(m_vFilters[nFilterIndex]) );
}

void CFilterContainer::UpdateFilter(int nFilterIndex, const IFilter* filter)
{
	delete m_vFilters[nFilterIndex];
	m_vFilters[nFilterIndex] = CloneFilter(filter);
}

// Swap filter A with filter B.
void CFilterContainer::SwapFilters(UINT nItemA, UINT nItemB)
{
	IFilter *tmp = m_vFilters[nItemA];
	m_vFilters[nItemA] = m_vFilters[nItemB];
	m_vFilters[nItemB] = tmp;
}

BOOL CFilterContainer::SaveFilters(const CString& filename)
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
		BOOST_STATIC_ASSERT(CFilteredPath::renameVersion == 100);
		if (m_nPathRenamePart & CFilteredPath::renameLastFolder)
			strRenameWhat += "LastDir";
		else
		{
			if (m_nPathRenamePart & CFilteredPath::renameRoot)
				strRenameWhat += "Root+";
			if (m_nPathRenamePart & CFilteredPath::renameFoldersPath)
				strRenameWhat += "Dirs+";
			if (m_nPathRenamePart & CFilteredPath::renameFilename)
				strRenameWhat += "File+";
			if (m_nPathRenamePart & CFilteredPath::renameExtension)
				strRenameWhat += "Ext+";
			
			if (!strRenameWhat.IsEmpty())
				strRenameWhat = strRenameWhat.Left(strRenameWhat.GetLength() - 1);
		}
		_ftprintf(file, _T("RenameWhat=\"%s\"\n"), (LPCTSTR)strRenameWhat);
		_ftprintf(file, _T("\n"));
	}

	// Write data
	BOOST_FOREACH(IFilter* pFilter, m_vFilters)
	{
		_ftprintf(file, _T("[%s]\n"), (LPCTSTR)pFilter->GetFilterCodeName());

		CMapStringToString mapArgs;
		pFilter->GetArgs(mapArgs);
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
	scoped_ptr<IFilter> filter;

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
			if (filter.get() != NULL)
			{
				// Add the previously loaded filter to the list
				filter->SetArgs(mapArgs);
				if (filter->GetFilterCodeName() == "RenameWhat")
				{
					// Get the rename what part.
					m_nPathRenamePart = static_cast<CRenameWhat*>(filter.get())->GetRenameWhat();
				}
				else
				{
					AddFilter(filter.get());
					++nFiltersAdded;
				}
				filter.reset();
				mapArgs.RemoveAll();
			}

			// Check if filter exist...
			if (strType == _T("General"))
				filter.reset(new CRenameWhat());
			else if (strType == _T("Search and replace"))
				filter.reset(new CSearchReplaceFilter());
			else
			{
				// Unknown filter
				CString strMsg;
				AfxFormatString1(strMsg, IDS_UNKNOWN_FILTER, strType);
				AfxMessageBox(strMsg, MB_ICONWARNING);
			}
		}
		// Else if we are loading a filter's arguments
		else if (filter.get() != NULL)	// key="value"
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

	if (filter.get() != NULL)
	{
		filter->SetArgs(mapArgs);
		if (filter->GetFilterCodeName() == "RenameWhat")
		{
			// Get the rename what part.
			m_nPathRenamePart = static_cast<CRenameWhat*>(filter.get())->GetRenameWhat();
		}
		else
		{
			AddFilter(filter.get());
			++nFiltersAdded;
		}
		filter.reset();
		mapArgs.RemoveAll();
	}

	fclose(file);

	return nFiltersAdded;
}

IFilter* CFilterContainer::CloneFilter(const IFilter* filter)
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
	}
	return pClone;
}

}}}}
