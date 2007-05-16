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

CFilterContainer::~CFilterContainer(void)
{
	RemoveAllFilters();
}

void CFilterContainer::AddFilter(IFilter* filter)
{
	m_clFilters.AddTail(filter);
}

CString CFilterContainer::FilterString(const CFileName& fnPath) const
{
	// What is the filtered part of the path
	CString strBeforeFilteredPart;
	CString strFilteredPart;
	CString strAfterFilteredPart;
	GetFilteredPart(fnPath, strBeforeFilteredPart, strFilteredPart, strAfterFilteredPart);

	// OnStartRenamingFile
	for (POSITION pos=m_clFilters.GetHeadPosition(); pos!=NULL; ) 
		m_clFilters.GetNext(pos)->OnStartRenamingFile(fnPath, strFilteredPart);

	// Filter the name through all filters.
	for (POSITION pos=m_clFilters.GetHeadPosition(); pos!=NULL; ) 
		m_clFilters.GetNext(pos)->FilterPath(fnPath, strFilteredPart);

	// OnEndRenamingFile
	for (POSITION pos=m_clFilters.GetHeadPosition(); pos!=NULL; ) 
		m_clFilters.GetNext(pos)->OnEndRenamingFile();

	return strBeforeFilteredPart + strFilteredPart + strAfterFilteredPart;
}

int CFilterContainer::ShowDialog(int nFilterIndex, const CFileName& fnSamplePath)
{
	// Check that the filter exist
	POSITION posFilter = m_clFilters.FindIndex(nFilterIndex);
	if (posFilter == NULL)
	{
		ASSERT(false);
		return 0;
	}

	// What is the filtered part of the path
	CString strBeforeFilteredPart;
	CString strFilteredPart;
	CString strAfterFilteredPart;
	GetFilteredPart(fnSamplePath, strBeforeFilteredPart, strFilteredPart, strAfterFilteredPart);

	// OnStartRenamingFile
	for (POSITION pos=m_clFilters.GetHeadPosition(); pos!=NULL; ) 
		m_clFilters.GetNext(pos)->OnStartRenamingFile(fnSamplePath, strFilteredPart);

	// Filter the name through all filters.
	for (POSITION pos=m_clFilters.GetHeadPosition(); pos!=posFilter; )
		m_clFilters.GetNext(pos)->FilterPath(fnSamplePath, strFilteredPart);

	// OnEndRenamingFile
	for (POSITION pos=m_clFilters.GetHeadPosition(); pos!=NULL; ) 
		m_clFilters.GetNext(pos)->OnEndRenamingFile();

	// Show Dialog
	return m_clFilters.GetAt(posFilter)->ShowDialog(fnSamplePath, strFilteredPart);
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
		if (m_nPathRenamePart & CRenamePartSelectionCtrl::renameFolders)
			strRenameWhat += "Dir";
		if (m_nPathRenamePart & CRenamePartSelectionCtrl::renameFilename)
			strRenameWhat += "File";
		if (m_nPathRenamePart & CRenamePartSelectionCtrl::renameExtension)
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

void CFilterContainer::OnStartRenamingList() const
{
	// OnStartRenamingList
	for (POSITION pos=m_clFilters.GetHeadPosition(); pos!=NULL; ) 
		m_clFilters.GetNext(pos)->OnStartRenamingList((IFilter::ERenamePart) m_nPathRenamePart);
}

void CFilterContainer::OnEndRenamingList() const
{
	// OnEndRenamingList
	for (POSITION pos=m_clFilters.GetHeadPosition(); pos!=NULL; ) 
		m_clFilters.GetNext(pos)->OnEndRenamingList();
}

// What is the filtered part of the path
void CFilterContainer::GetFilteredPart(const CFileName& fnFullPath, CString& strBeforePartToRename, CString& strPartToRename, CString& strAfterPartToRename) const
{
	// Clear
	strBeforePartToRename = fnFullPath.GetDrive();
	strPartToRename.Empty();
	strAfterPartToRename.Empty();

	// For the special case, if there is only the extension
	if (m_nPathRenamePart == CRenamePartSelectionCtrl::renameExtension)
	{
		strBeforePartToRename = fnFullPath.GetDrive();
		strBeforePartToRename += fnFullPath.GetDirectory();
		strBeforePartToRename += fnFullPath.GetFileName();
		strBeforePartToRename += _T(".");
		strPartToRename = fnFullPath.GetExtension().Mid(1);
		strAfterPartToRename.Empty();
		return;
	}

	if (m_nPathRenamePart & CRenamePartSelectionCtrl::renameFolders)
		strPartToRename += fnFullPath.GetDirectory();
	else
		if (strPartToRename.IsEmpty())
			strBeforePartToRename += fnFullPath.GetDirectory();
		else
			strAfterPartToRename += fnFullPath.GetDirectory();

	if (m_nPathRenamePart & CRenamePartSelectionCtrl::renameFilename)
		strPartToRename += fnFullPath.GetFileName();
	else
		if (strPartToRename.IsEmpty())
			strBeforePartToRename += fnFullPath.GetFileName();
		else
			strAfterPartToRename += fnFullPath.GetFileName();

	if (m_nPathRenamePart & CRenamePartSelectionCtrl::renameExtension)
		strPartToRename += fnFullPath.GetExtension();
	else
		if (strPartToRename.IsEmpty())
			strBeforePartToRename += fnFullPath.GetExtension();
		else
			strAfterPartToRename += fnFullPath.GetExtension();
}

CFileList CFilterContainer::FilterFileNames(const CFileList& fileList) const
{
	CFileList filteredFileNames;

	OnStartRenamingList();
	for (int i=0; i<fileList.GetFileCount(); ++i)
		filteredFileNames.AddFile( FilterString(fileList[i]) );
	OnEndRenamingList();

	return filteredFileNames;
}
