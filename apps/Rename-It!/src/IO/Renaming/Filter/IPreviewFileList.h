#pragma once

namespace Beroux{ namespace IO{ namespace Renaming{ namespace Filter
{
	// Forward declaration
	class IPreviewFileList;
}}}}

#include "IFilter.h"
#include "FilteredPath.h"

namespace Beroux{ namespace IO{ namespace Renaming{ namespace Filter
{
	/**
	 * Interface of CPreviewFileList to avoid templates
	 * (because templates don't work with CObject xxx_DYNAMIC).
	 */
	class IPreviewFileList
	{
	public:
	// Declaration
		class Iterator
		{
		public:
			/// Returns true when there are more elements in the list.
			virtual bool HasNext() const = 0;

			/// Current element.
			virtual CPath GetCurrent() const = 0;

			/// Move to the next element.
			virtual void MoveNext() = 0;
		};

	// Construction
		IPreviewFileList() {}
		virtual ~IPreviewFileList() {}

	// Attributes
		/**
		 * Iterator positioned at the beginning to iterate all samples.
		 * @return An Iterator that will give all samples CPath.
		 */
		virtual Iterator* GetSampleFilesIterator() const = 0;

		/**
		 * Iterator positioned at the current default sample CPath.
		 * The default one is the one to be shown to the user unless
		 * the user wants another one.
		 * @return An Iterator positioned at the default CPath.
		 */
//		virtual Iterator* GetDefaultSampleFile() const = 0;

		/**
		 * Move the current default sample CPath to another sample in the list.
		 */
//		virtual void SetDefaultSampleFile(const Iterator& value) = 0;

		/**
		 * Return the current default sample file path before any filtering is done.
		 */
		virtual CFilteredPath GetOriginalFileName() const = 0;

		/**
		 * Return the current default sample file path before the preview filter is applied.
		 */
		virtual CFilteredPath GetBeforePreviewRenaming() const = 0;

	// Operations
		/**
		 * Preview the effect of a filter on the sample file name.
		 * The preview filter is added to the container and used to filter the default sample file name.
		 * @param pFilterToPreview The filter that will be used to preview the filtered file name.
		 * @return The name after filtering (including a filter to preview the effect of) of the current default sample file.
		 */
		virtual CFilteredPath PreviewRenaming(const IFilter* pFilterToPreview) const = 0;

		/**
		 * Preview the effect of a filter on a given string which will replace the sample file name.
		 * This is the same as PreviewRenaming(const IFilter*) except the filtered part is replaced by the given string.
		 *
		 * @param pFilterToPreview The filter that will be used to preview the filtered file name.
		 * @param strSampleRenamedPath New current default path filtered part.
		 * @return The name after filtering (including a filter to preview the effect of) of the current default sample file.
		 */
		virtual CFilteredPath PreviewRenaming(const IFilter* pFilterToPreview, const CString& strSampleRenamedPath) const = 0;
	};
}}}}
