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

namespace Beroux{ namespace IO{ namespace Renaming{ namespace Filter
{
	// Forward declaration.
	class IFilter;
}}}}

#include "../Path.h"
#include "IPreviewFileList.h"

namespace Beroux{ namespace IO{ namespace Renaming{ namespace Filter
{
	/**
	 * Interface of filters.
	 */
	class IFilter
	{
		friend class CFilterContainer;

	// Constructors
	public:
		virtual ~IFilter() {};

	// Operations
	protected:	// Protected to make sure the caller uses OnStartRenamingList and OnEndRenamingList.
		// Should be called before filtering a list of files with FilterPath.
		virtual void OnStartRenamingList() = 0;

		/** Filter the file name.
		 * OnStartRenamingList and OnEndRenamingList should be called.
		 * @param[in,out] strFileName		Current new part being renamed of the file name. Contains the new name after.
		 * @param[in] fnOriginalFilename	Path to the original file name (the file should exist).
		 * @param[in] strUnfilteredName		The part being renaming, before any filter is applied.
		 * @return True if file should be renamed.
		 */
		virtual void FilterPath(CString& strFileName, const CPath& fnOriginalFilename, const CString& strUnfilteredName) = 0;

		// Should be called after filtering a list of files.
		virtual void OnEndRenamingList() = 0;

	public:
		/** Show the filter edition dialog.
		 * @param previewSamples	An object that can preview the effect of the filter.
		 * @return The value returned by CDialog::DoModal().
		 */
		virtual int ShowDialog(IPreviewFileList& previewSamples) = 0;

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
		
		// Return true when the filter effect depends on previously filtered names in the list.
		virtual bool IsPastFilteredDependant() = 0;
	};
}}}}
