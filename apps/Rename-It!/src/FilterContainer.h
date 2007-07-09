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

	int GetFilterCount() const { return (int) m_clFilters.GetCount(); }

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
	 * @param[in] a_begin	The beginning of the file list.
	 * @param[in] a_first	First file to filter.
	 * @param[in] a_last	Last file to filter.
	 * @param[out] b_result Destination container where the filtered file names will be added.
	 * @return End of the output range (ie. result + (last - first)).
	 */
	template<class InputIterator, class OutputIterator>
	OutputIterator FilterFileNames(InputIterator a_begin, InputIterator a_first, InputIterator a_last, OutputIterator b_result) const
	{
		// Change the locale to fit the current user settings.
		CString strLocaleBak = _tsetlocale(LC_ALL, NULL);
		_tsetlocale(LC_ALL, _T(""));

		// Tell all filters we're going to start renaming a list of files.
		for (POSITION pos=m_clFilters.GetHeadPosition(); pos!=NULL; ) 
			m_clFilters.GetNext(pos)->OnStartRenamingList();

		// When one or more filter depend on previously filtered items...
		POSITION posLastPastFilteredDependant = NULL;
		for (POSITION pos=m_clFilters.GetTailPosition(); pos!=NULL; )
		{
			POSITION cur = pos;
			if (m_clFilters.GetPrev(pos)->IsPastFilteredDependant())
			{
				POSITION posLastPastFilteredDependant = cur;
				break;
			}
		}
		if (posLastPastFilteredDependant != NULL)
		{	
			// ... then, we filter without saving the result the files between "begin" and "first".
			for (; a_begin!=a_first; ++a_begin)
			{
				// Get the filtered part of the path.
				CString strFilteredPart = CFilteredFileName(*a_begin, m_nPathRenamePart).GetFilteredSubstring();
				CString strUnfilteredName = strFilteredPart;
	
				// Filter the name through all filters.
				POSITION pos = m_clFilters.GetHeadPosition();
				do
					m_clFilters.GetNext(pos)->FilterPath(strFilteredPart, *a_begin, strUnfilteredName);
				while (pos != posLastPastFilteredDependant);
			}
		}

		// We filter and insert in the ouput container the result the files between "first" and "last".
		for (; a_first!=a_last; ++a_first)
		{
			// Get the filtered part of the path.
			CFilteredFileName ffnFileName(*a_first, m_nPathRenamePart);
			CString strFilteredPart = ffnFileName.GetFilteredSubstring();

			// Filter the name through all filters.
			CString strUnfilteredName = strFilteredPart;
			for (POSITION pos=m_clFilters.GetHeadPosition(); pos!=NULL; ) 
				m_clFilters.GetNext(pos)->FilterPath(strFilteredPart, *a_first, strUnfilteredName);

			// Insert in the output container.
			ffnFileName.SetFilteredSubstring(strFilteredPart);
			*b_result = ffnFileName;
			++b_result;
		}

		// Tell all filters we're done.
		for (POSITION pos=m_clFilters.GetHeadPosition(); pos!=NULL; ) 
			m_clFilters.GetNext(pos)->OnEndRenamingList();

		// Restaure the locale.
		_tsetlocale(LC_ALL, strLocaleBak);

		return b_result;
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
		virtual bool IsPastFilteredDependant() { return false; }
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
