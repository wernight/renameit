#pragma once

#include "IPreviewFileList.h"
#include "FilterContainer.h"

/**
 * A file list that allows to preview the effect of a renaming filter.
 */
template <typename InputIterator>
class CPreviewFileList : public IPreviewFileList
{
public:
	/**
	 * Create the previewable file list.
	 * @note The filters will be modified during the previewing but the state will be the same after.
	 * @param beginSampleFile First file name in the files' list.
	 * @param firstSampleFile First file name that can be previewed.
	 * @param lastSampleFile Last file name that can be previewed.
	 * @param defaultSample The default previewed file.
	 * @param filters A filter container containing the filters BEFORE the previewed filter.
	 */
	CPreviewFileList(InputIterator beginSampleFile, InputIterator firstSampleFile, InputIterator lastSampleFile, InputIterator defaultSample, const CFilterContainer& filters) :
		m_beginSampleFile(beginSampleFile),
		m_firstSampleFile(firstSampleFile),
		m_lastSampleFile(lastSampleFile),
		m_defaultSample(defaultSample),
		m_fcFilters(filters)
	{
	}

// Attributes
	virtual Iterator* GetSampleFilesIterator() const
	{
		return new CIterator<InputIterator>(m_firstSampleFile, m_lastSampleFile);
	}

	virtual Iterator* GetDefaultSampleFile() const
	{
		return new CIterator<InputIterator>(m_defaultSample, m_lastSampleFile);
	}

	virtual void SetDefaultSampleFile(const Iterator& value)
	{
		m_defaultSample = static_cast<const CIterator<InputIterator>& >(value).GetInputIterator();
	}

	virtual CFilteredPath GetOriginalFileName() const
	{
		return CFilteredPath(*m_defaultSample, m_fcFilters.GetPathRenamePart());
	}

	virtual CFilteredPath GetBeforePreviewRenaming() const
	{
		// Create a copy.
		CFilterContainer fc = m_fcFilters;

		// Filter the current default sample file.
		CPath fnFileName;
		InputIterator last = m_defaultSample; ++last;
		fc.FilterFileNames(
			m_beginSampleFile,
			m_defaultSample,
			last,
			&fnFileName);

		return CFilteredPath(fnFileName, m_fcFilters.GetPathRenamePart());
	}

// Operations
	virtual CFilteredPath PreviewRenaming(const IFilter* pFilterToPreview) const
	{
		// Add the filter to preview at the end of the filters' list.
		CFilterContainer fc = m_fcFilters;
		fc.AddFilter(pFilterToPreview);

		// Filter the current default sample file.
		CPath fnFileName;
		InputIterator last = m_defaultSample; ++last;
		fc.FilterFileNames(
			m_beginSampleFile,
			m_defaultSample,
			last,
			&fnFileName);

		return CFilteredPath(fnFileName, m_fcFilters.GetPathRenamePart());
	}

// Implementation
private:
	template <class InputIterator>
	class CIterator : public Iterator
	{
	public:
		CIterator(const InputIterator& first, const InputIterator& last)
		{
			m_first = first;
			m_last = last;
		}

		// Return true when there are more elements in the list.
		virtual bool HasNext() const
		{
			return m_first != m_last;
		}

		// Return the current element and move to the next one.
		virtual CPath GetCurrent() const
		{
			return *m_first;
		}

		virtual void MoveNext()
		{
			++m_first;
		}

		// Return the current InputIterator.
		InputIterator GetInputIterator() const
		{
			return m_first;
		}

	private:
		InputIterator m_first;
		InputIterator m_last;
	};

	InputIterator m_beginSampleFile;
	InputIterator m_firstSampleFile;
	InputIterator m_lastSampleFile;
	InputIterator m_defaultSample;
	CFilterContainer m_fcFilters;
};
