#pragma once
class IPreviewFileList;

#include "IFilter.h"
#include "FilteredPath.h"

/**
 * Interface of CPreviewFileList to avoid templates (because templates don't work with CObject xxx_DYNAMIC).
 */
class IPreviewFileList
{
public:
// Declaration
	class Iterator
	{
	public:
		// Return true when there are more elements in the list.
		virtual bool HasNext() const = 0;

		// Return the current element.
		virtual CPath GetCurrent() const = 0;

		// Move to the next element.
		virtual void MoveNext() = 0;
	};

// Construction
	IPreviewFileList() {}
	virtual ~IPreviewFileList() {}

// Attributes
	virtual Iterator* GetSampleFilesIterator() const = 0;

	virtual Iterator* GetDefaultSampleFile() const = 0;

	virtual void SetDefaultSampleFile(const Iterator& value) = 0;

// Operations
	/**
	 * Return the original file name of the current default sample file.
	 */
	virtual CFilteredPath GetOriginalFileName() const = 0;

	/**
	 * Preview the effect of a filter on the sample file name.
	 * The preview filter is added to the container and used to filter the default sample file name.
	 * @param pFilterToPreview The filter that will be used to preview the filtered file name.
	 * @return The new file name after filtering of the current default sample file.
	 */
	virtual CFilteredPath PreviewRenaming(const IFilter* pFilterToPreview) const = 0;
};
