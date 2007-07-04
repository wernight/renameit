#pragma once

#include "IFilter.h"
#include "IPreviewFileList.h"
#include "FilteredFileName.h"

/** 
 * Contain a set of filters.
 */
class CFilterContainer
{
// Definitions
public:
// Constructors
	CFilterContainer();

	CFilterContainer(const CFilterContainer& copy);

	~CFilterContainer();

// Attributes
	// A set of flags from CRenamePartSelectionCtrl::ERenamePartFlags
	void SetPathRenamePart(unsigned nPathRenamePart){ m_nPathRenamePart = nPathRenamePart; }
	unsigned GetPathRenamePart() const { return m_nPathRenamePart; }

	UINT GetFilterCount() const{ return (UINT) m_clFilters.GetCount(); }

// Operations
	// Add a filter at the end of the list.
	void AddFilter(const IFilter* filter);

	/**
	 * Remove one filter.
	 * @param nFilterIndex Index of the filter to remove.
	 */
	void RemoveFilter(int nFilterIndex);

	/**
	 * Remove all the filters.
	 */
	void RemoveAllFilters();

	// Load/Save all the filters to a file.
	BOOL SaveFilters(const CString &filename);
	int LoadFilters(const CString &filename);

	// Return the filter at the given index.
	IFilter* GetFilter(int nFilterIndex) const;

	// Set the filter nFilterIndex.
	void UpdateFilter(int nFilterIndex, IFilter* filter);

	// Swap filter A with filter B.
	BOOL SwapFilters(UINT nItemA, UINT nItemB);

	/**
	 * Filter a list of file names from [first, last) to [result, result + (last - first)).
	 * InputIterator must follow concept of Input Iterator over an array of CFileName (ex: CFileName x[10]).
	 * OutputIterator must follow concept of Output Iterator over an array of CFileName (ex: CFileName y[10]).
	 * @param[in] begin	The beginning of the file list.
	 * @param[in] first	First file to filter.
	 * @param[in] last	Last file to filter.
	 * @param[out] result Destination container where the filtered file names will be added.
	 * @return End of the output range (ie. result + (last - first)).
	 */
	template<class InputIterator, class OutputIterator>
	OutputIterator FilterFileNames(InputIterator begin, InputIterator first, InputIterator last, OutputIterator result) const
	{
		// Tell all filters we're going to start renaming a list of files.
		for (POSITION pos=m_clFilters.GetHeadPosition(); pos!=NULL; ) 
			m_clFilters.GetNext(pos)->OnStartRenamingList();

		// We filter without saving the result the files between "begin" and "first".
		for (; begin!=first; ++begin)
		{
			// Get the filtered part of the path.
			CString strFilteredPart = CFilteredFileName(*begin, m_nPathRenamePart).GetFilteredSubstring();

			// Filter the name through all filters.
			CString strUnfilteredName = strFilteredPart;
			for (POSITION pos=m_clFilters.GetHeadPosition(); pos!=NULL; ) 
				m_clFilters.GetNext(pos)->FilterPath(strFilteredPart, *begin, strUnfilteredName);
		}

		// We filter and insert in the ouput container the result the files between "first" and "last".
		for (begin; begin!=last; ++begin)
		{
			// Get the filtered part of the path.
			CFilteredFileName ffnFileName(*begin, m_nPathRenamePart);
			CString strFilteredPart = ffnFileName.GetFilteredSubstring();

			// Filter the name through all filters.
			CString strUnfilteredName = strFilteredPart;
			for (POSITION pos=m_clFilters.GetHeadPosition(); pos!=NULL; ) 
				m_clFilters.GetNext(pos)->FilterPath(strFilteredPart, *begin, strUnfilteredName);

			// Insert in the output container.
			ffnFileName.SetFilteredSubstring(strFilteredPart);
			*result = ffnFileName;
			++result;
		}

		// Tell all filters we're done.
		for (POSITION pos=m_clFilters.GetHeadPosition(); pos!=NULL; ) 
			m_clFilters.GetNext(pos)->OnEndRenamingList();

		return result;
	}

// Implementation
private:
	/**
	 * A fake filter that doesn't filter files but which only purpose if to be keep the part to rename.
	 */
	class CRenameWhat : public IFilter
	{
	public:
		CRenameWhat(void) : m_nRenameWhat(-1) {}
		~CRenameWhat(void) {}

	// Attributes
		int GetRenameWhat() const {
			return m_nRenameWhat;
		}

	// Implementation of IFilter

		virtual void SetArgs(const CMapStringToString& mapArgs)
		{
			CString strRenameWhat;
			if (!mapArgs.Lookup(_T("RenameWhat"), strRenameWhat))
				return;

			m_nRenameWhat = 0;
			if (strRenameWhat.Find(_T("Dir")) >= 0)
				m_nRenameWhat |= CFilteredFileName::renameFolders;
			if (strRenameWhat.Find(_T("File")) >= 0)
				m_nRenameWhat |= CFilteredFileName::renameFilename;
			if (strRenameWhat.Find(_T("Ext")) >= 0)
				m_nRenameWhat |= CFilteredFileName::renameExtension;
		}

		// Functions that are not used.
	protected:
		virtual void FilterPath(CString& strFileName, const CFileName& fnOriginalFilename, const CString& strUnfilteredName) { ASSERT(FALSE); }
	public:
		virtual int ShowDialog(IPreviewFileList& previewSamples) { ASSERT(FALSE); return IDCANCEL; }
		virtual CString GetFilterCodeName() const { return _T("RenameWhat"); }
		virtual CString GetFilterName() const { return _T("RenameWhat"); }
		virtual CString GetFilterDescription() const { return _T("RenameWhat"); }
		virtual void GetArgs(CMapStringToString& mapArgs) const { ASSERT(FALSE); }
		virtual void LoadDefaultArgs() { ASSERT(FALSE); }
	protected:
		virtual void OnStartRenamingList() { ASSERT(FALSE); }
		virtual void OnEndRenamingList() { ASSERT(FALSE); }

	// Implementation
	private:
		int m_nRenameWhat;
	};

	unsigned m_nPathRenamePart;	// Part to be renamed from the path; A set of flags from ERenamePartFlags

	CTypedPtrList<CPtrList, IFilter*> m_clFilters;
};
