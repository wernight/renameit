/* 
* Copyright (C) 2002 Markus Eriksson, marre@renameit.hypermart.net
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#pragma once

#include "FileName.h"

class IFilter
{
	friend class CFilterContainer;

// Definitions
public:
	enum ERenamePart
	{
		rpFileNameWithoutExtension = 0,		// Rename only the file name without it's extension. If it's a folder, rename the last sub-folder only.
		rpFileNameWithExtension,			// Rename the file name, taking in count it's extension. If it's a folder, rename the last sub-folder only (same as rpFileNameWithoutExtension).
		rpFullPath							// Rename the full path.
	};

// Constructors
	virtual ~IFilter() {};

// Operations
protected:// Shouldn't access to this because the OnStart/End have to be managed by FilterContainer.
	/** Filter the file name.
	 * @param originalFilename	Path to the original file name.
	 * @param filename			Current new file name, without extension.
	 * @param ext				Current new extension.
	 * @return True if file should be renamed.
	 */
	virtual void FilterPath(const CFileName& fnOriginalFilename, CString &strFilename) = 0;
public:

	/** Display a window to configure the filter.
	 * @param originalFilename	Original sample file name.
	 * @param filename			Current sample file name.
	 * @param ext				Current sample file extension.
	 * @return The value returned by CDialog::DoModal().
	 */
	virtual int ShowDialog(const CFileName& fnOriginalFilename, const CString& strFilename) = 0;

// Attributes
	// Return the unique name of the filter.
	virtual CString GetFilterCodeName() const = 0;

	// Return the human name of the filter in user language.
	virtual CString GetFilterName() const = 0;

	// User friendly description of the filter's actions over file's name.
	virtual CString GetFilterDescription() const = 0;

	// Return filter's parameters.
	virtual void GetArgs(CMapStringToString& mapArgs) const = 0;

	// Initialize filter's parameters.
	virtual void LoadDefaultArgs() = 0;

	// Define filter's parameters.
	virtual void SetArgs(const CMapStringToString& mapArgs) = 0;

// Events
protected:
	virtual void OnStartRenamingList(ERenamePart nPathRenamePart) = 0;
	virtual void OnStartRenamingFile(const CFileName& fnPath, const CString& strName) = 0;
	virtual void OnEndRenamingFile() = 0;
	virtual void OnEndRenamingList() = 0;
};
