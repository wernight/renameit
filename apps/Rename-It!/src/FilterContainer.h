#pragma once
#include "IFilter.h"
#include "FileList.h"
#include "RenamePartSelectionCtrl.h"

/** 
 * Contain a set of filters.
 */
class CFilterContainer
{
// Constructors
public:
	CFilterContainer();
	virtual ~CFilterContainer();

// Attributes
	// A set of flags from CRenamePartSelectionCtrl::ERenamePartFlags
	void SetPathRenamePart(int nPathRenamePart){ m_nPathRenamePart = nPathRenamePart; }
	int GetPathRenamePart() const { return m_nPathRenamePart; }

	UINT GetFilterCount() const{ return (UINT) m_clFilters.GetCount(); }

// Operations
	void AddFilter(IFilter* filter);
	CString FilterString(const CFileName& fnPath) const;
	int ShowDialog(int nFilterIndex, const CFileName& fnSamplePath);

	void RemoveFilter(int nFilterIndex);
	void RemoveAllFilters();

	BOOL SaveFilters(const CString &filename);
	int LoadFilters(const CString &filename);

	IFilter* GetFilter(int nFilterIndex) const;
	void UpdateFilter(int nFilterIndex, IFilter* filter);

	// Swap filter A with filter B.
	BOOL SwapFilters(UINT nItemA, UINT nItemB);

	CFileList FilterFileNames(const CFileList& fileList) const;

	void GetFilteredPart(const CFileName& fnFullPath, CString& strBeforePartToRename, CString& strPartToRename, CString& strAfterPartToRename) const;

// Events
	void OnStartRenamingList() const;
	void OnEndRenamingList() const;

// Implementation
private:
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
				m_nRenameWhat |= CRenamePartSelectionCtrl::renameFolders;
			if (strRenameWhat.Find(_T("File")) >= 0)
				m_nRenameWhat |= CRenamePartSelectionCtrl::renameFilename;
			if (strRenameWhat.Find(_T("Ext")) >= 0)
				m_nRenameWhat |= CRenamePartSelectionCtrl::renameExtension;
		}

		// Functions that are not used.
		virtual void FilterPath(const CFileName& fnOriginalFilename, CString &strFilename) { ASSERT(FALSE); }
		virtual int ShowDialog(const CFileName& fnOriginalFilename, const CString& strFilename) { ASSERT(FALSE); return IDCANCEL; }
		virtual CString GetFilterCodeName() const { return _T("RenameWhat"); }
		virtual CString GetFilterName() const { return _T("RenameWhat"); }
		virtual CString GetFilterDescription() const { return _T("RenameWhat"); }
		virtual void GetArgs(CMapStringToString& mapArgs) const { ASSERT(FALSE); }
		virtual void LoadDefaultArgs() { ASSERT(FALSE); }
		virtual void OnStartRenamingList(ERenamePart nPathRenamePart) { ASSERT(FALSE); }
		virtual void OnStartRenamingFile(const CFileName& fnPath, const CString& strName) { ASSERT(FALSE); }
		virtual void OnEndRenamingFile() { ASSERT(FALSE); }
		virtual void OnEndRenamingList() { ASSERT(FALSE); }

	// Implementation
	private:
		int m_nRenameWhat;
	};

	int	m_nPathRenamePart;	// Part to be renamed from the path; A set of flags from CRenamePartSelectionCtrl::ERenamePartFlags

	CTypedPtrList<CPtrList, IFilter*> m_clFilters;
};
