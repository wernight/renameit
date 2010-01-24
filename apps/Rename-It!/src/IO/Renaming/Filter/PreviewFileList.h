#pragma once

#include "IPreviewFileList.h"
#include "FilterContainer.h"

namespace Beroux{ namespace IO{ namespace Renaming{ namespace Filter
{
	/**
	 * A file list that allows to preview the effect of a renaming filter.
	 */
	template <typename InputIterator>
	class CPreviewFileList : public IPreviewFileList
	{
	public:
		typedef InputIterator InputIterator;

		/**
		 * Create the preview-able file list.
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

		const InputIterator GetBeginSample() const {
			return m_beginSampleFile;
		}

		const InputIterator GetDefaultSample() const {
			return m_defaultSample;
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

			return FilterSample(fc);
		}

	// Operations
		virtual CFilteredPath PreviewRenaming(const IFilter* pFilterToPreview) const
		{
			// Add the filter to preview at the end of the filters' list.
			CFilterContainer fc = m_fcFilters;
			fc.AddFilter(pFilterToPreview);

			return FilterSample(fc);
		}

		virtual CFilteredPath PreviewRenaming(const IFilter* pFilterToPreview, const CString& strSampleRenamedPath) const
		{
			// Add the filter to preview at the end of the filters' list.
			CFilterContainer fc = m_fcFilters;
			fc.AddFilter(pFilterToPreview);

			// Change the string renamed
			CFilteredPath newSampleFile(*m_defaultSample, m_fcFilters.GetPathRenamePart());
			newSampleFile.SetFilteredSubstring(strSampleRenamedPath);
			
			// Wrap the iterator so that the default sample returns another CPath
// 			list<InputIterator> newSamples;
// 			for (InputIterator ii=m_beginSampleFile; ii!=m_defaultSample; ++ii)
// 				newSamples.push(ii);
// 			newSamples.push(newSampleFile);

			// Filter the current default sample file.
			CPath fnFileName;
			InputIterator last = m_defaultSample; ++last;
			fc.FilterFileNames(
				CReplaceDefaultWrapper<InputIterator>(m_beginSampleFile, m_defaultSample, newSampleFile),
				CReplaceDefaultWrapper<InputIterator>(m_defaultSample, m_defaultSample, newSampleFile),
				CReplaceDefaultWrapper<InputIterator>(last, m_defaultSample, newSampleFile),
				&fnFileName);

			return CFilteredPath(fnFileName, m_fcFilters.GetPathRenamePart());
		}

	// Implementation
	private:
		template <class _InputIterator>
		class CReplaceDefaultWrapper
		{
		public:
			CReplaceDefaultWrapper(_InputIterator current, _InputIterator defaultSample, const CPath& newSample)
			: m_current(current)
			, m_defaultSample(defaultSample)
			, m_newSample(newSample)
			{
			}

			// Equality Comparable (not so easy what that should mean here)
			bool operator==(const CReplaceDefaultWrapper& rhs) const
			{
				return m_current == rhs.m_current;
			}

			bool operator!=(const CReplaceDefaultWrapper& rhs) const
			{
				return m_current != rhs.m_current;
			}

			// Trivial Iterator:
			const Beroux::IO::Renaming::CPath& operator* () const
			{
				if (m_current != m_defaultSample)
					return *m_current;
				else
					return m_newSample;
			}

			const Beroux::IO::Renaming::CPath* operator->() const {
				return & operator*();
			}

			// Input Iterator
			CReplaceDefaultWrapper& operator++()
			{
				++m_current;
				return *this;
			}

		private:
			_InputIterator m_current;
			_InputIterator m_defaultSample;
			const CPath& m_newSample;
		};

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

		virtual CFilteredPath FilterSample(CFilterContainer& fc) const
		{
			// Filter the current default sample file.
			CPath fnFileName;
			InputIterator last = m_defaultSample; ++last;
			fc.FilterFileNames(
				m_beginSampleFile,
				m_defaultSample,
				last,
				&fnFileName);

			// Return a filtered Path with the correct "filtered part" set.
			return CFilteredPath(fnFileName, m_fcFilters.GetPathRenamePart());
		}

		InputIterator m_beginSampleFile;
		InputIterator m_firstSampleFile;
		InputIterator m_lastSampleFile;
		InputIterator m_defaultSample;
		CFilterContainer m_fcFilters;
	};
}}}}
