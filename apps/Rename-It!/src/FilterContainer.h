#pragma once

#include "IFilter.h"
#include "IPreviewFileList.h"
#include "FilteredPath.h"

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

	int GetFilterCount() const { return (int) m_vFilters.size(); }

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
	shared_ptr<IFilter> GetFilterAt(int nFilterIndex) const;

	// Set the filter nFilterIndex.
	void UpdateFilter(int nFilterIndex, const IFilter* filter);

	// Swap filter A with filter B.
	void SwapFilters(UINT nItemA, UINT nItemB);

	/**
	 * Filter a list of file names from [first, last) to [result, result + (last - first)).
	 * InputIterator must follow concept of Input Iterator over an array of CPath (ex: CPath x[10]).
	 * OutputIterator must follow concept of Output Iterator over an array of CPath (ex: CPath y[10]).
	 * @param[in] a_begin	The beginning of the file list.
	 * @param[in] a_first	First file to filter.
	 * @param[in] a_last	Last file to filter.
	 * @param[out] b_result Destination container where the filtered file names will be added.
	 * @return End of the output range (ie. result + (last - first)).
	 */
	template<class InputIterator, class OutputIterator>
	OutputIterator FilterFileNames(InputIterator a_begin, InputIterator a_first, InputIterator a_last, OutputIterator b_result)
	{
		// Change the locale to fit the current user settings.
		CString strLocaleBak = _tsetlocale(LC_ALL, NULL);
		_tsetlocale(LC_ALL, _T(""));

		// Tell all filters we're going to start renaming a list of files.
		for (iterator iter=m_vFilters.begin(); iter!=m_vFilters.end(); ++iter)
			(*iter)->OnStartRenamingList();

		// When one or more filter depend on previously filtered items...
		for (int i=(int)m_vFilters.size()-1; i>=0; --i)
		{
			if (m_vFilters[i]->IsPastFilteredDependant())
			{
				iterator iterLastPastFilteredDependant = m_vFilters.begin() + i;

				// ... then, we filter without saving the result the files between "begin" and "first".
				for (; a_begin!=a_first; ++a_begin)
				{
					// Get the filtered part of the path.
					CString strFilteredPart = CFilteredPath(*a_begin, m_nPathRenamePart).GetFilteredSubstring();
					CString strUnfilteredName = strFilteredPart;
		
					// Filter the name through all filters.
					for (iterator iter=m_vFilters.begin(); ; ++iter)
					{
						(*iter)->FilterPath(strFilteredPart, *a_begin, strUnfilteredName);
						if (iter == iterLastPastFilteredDependant)
							break;
					}
				}

				break;
			}
		}

		// We filter and insert in the ouput container the result the files between "first" and "last".
		for (; a_first!=a_last; ++a_first)
		{
			// Get the filtered part of the path.
			CFilteredPath ffnFileName(*a_first, m_nPathRenamePart);
			CString strFilteredPart = ffnFileName.GetFilteredSubstring();

			// Filter the name through all filters.
			CString strUnfilteredName = strFilteredPart;
			for (iterator iter=m_vFilters.begin(); iter!=m_vFilters.end(); ++iter)
				(*iter)->FilterPath(strFilteredPart, *a_first, strUnfilteredName);

			// Insert in the output container.
			ffnFileName.SetFilteredSubstring(strFilteredPart);
			*b_result = ffnFileName;
			++b_result;
		}

		// Tell all filters we're done.
		for (iterator iter=m_vFilters.begin(); iter!=m_vFilters.end(); ++iter)
			(*iter)->OnEndRenamingList();

		// Restaure the locale.
		_tsetlocale(LC_ALL, strLocaleBak);

		return b_result;
	}

// Implementation
private:
	static IFilter* CloneFilter(const IFilter* filter);

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

			BOOST_STATIC_ASSERT(CFilteredPath::renameVersion == 100);
			m_nRenameWhat = 0;
			if (strRenameWhat.Find(_T("LastDir")) >= 0)
				m_nRenameWhat = CFilteredPath::renameLastFolder;
			else
			{
				if (strRenameWhat.Find(_T("Root")) >= 0)
					m_nRenameWhat |= CFilteredPath::renameRoot;
				if (strRenameWhat.Find(_T("Dirs")) >= 0)
					m_nRenameWhat |= CFilteredPath::renameFoldersPath;
				if (strRenameWhat.Find(_T("File")) >= 0)
					m_nRenameWhat |= CFilteredPath::renameFilename;
				if (strRenameWhat.Find(_T("Ext")) >= 0)
					m_nRenameWhat |= CFilteredPath::renameExtension;
			}
		}

		// Functions that are not used.
	protected:
		virtual void FilterPath(CString& strFileName, const CPath& fnOriginalFilename, const CString& strUnfilteredName) { ASSERT(FALSE); }
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

	typedef vector<IFilter*>::iterator iterator;
	typedef vector<IFilter*>::const_iterator const_iterator;

	unsigned m_nPathRenamePart;	// Part to be renamed from the path; A set of flags from ERenamePartFlags

	vector<IFilter*> m_vFilters;
};
