#include <cxxtest/TestSuite.h>
#include "IO/Renaming/RenamingList.h"
#include "Helper/FileListGenerator.h"
#include <iostream>

#ifdef _UNICODE
# define tcout wcout
#else
# define tcout cout
#endif

using namespace Beroux::IO;
using namespace Beroux::IO::Renaming;

class RenamingListTestSuite : public CxxTest::TestSuite 
{
public:
	RenamingListTestSuite()
	{
		// Set the instance to that resources can be loaded (i.e., for TString::LoadString() to work).
		HINSTANCE hInstance = (HINSTANCE) GetModuleHandle(NULL);
		AfxSetResourceHandle(hInstance);
	}

	void setUp()
	{
	}

	void tearDown()
	{
	}

	void testSingleFileRenaming()
	{
		// Prepare
		CFileListGenerator fileListGenerator;
		CKtmTransaction ktm;
		fileListGenerator.AddFile(_T("a.tmp"), _T("a-after.tmp"));
		CRenamingList renList = fileListGenerator.MakeRenamingList();

		// Rename
		TS_ASSERT(renList.Check());
		TS_ASSERT(renList.PerformRenaming(ktm));
		TS_ASSERT(ktm.Commit());

		// Check success.
		TS_ASSERT(fileListGenerator.AreAllRenamed());
	}

	void testEmptyRenaming()
	{
		// Replace
		CKtmTransaction ktm;
		CFileList before, after;
		CRenamingList renList(before, after);

		// Rename nothing
		TS_ASSERT(renList.Check());
		TS_ASSERT(renList.PerformRenaming(ktm));
		TS_ASSERT(ktm.Commit());
	}

	void testCyclicFileRenaming()
	{
		// Prepare
		CFileListGenerator fileListGenerator;
		CKtmTransaction ktm;
		fileListGenerator.AddFile(_T("1.tmp"), _T("2.tmp"));
		fileListGenerator.AddFile(_T("4.tmp"), _T("5.tmp"));
		fileListGenerator.AddFile(_T("2.tmp"), _T("3.tmp"));
		fileListGenerator.AddFile(_T("3.tmp"), _T("4.tmp"));
		fileListGenerator.AddFile(_T("5.tmp"), _T("1.tmp"));
		CRenamingList renList = fileListGenerator.MakeRenamingList();

		// Rename
		TS_ASSERT(renList.Check());
		TS_ASSERT(renList.PerformRenaming(ktm));
		TS_ASSERT(ktm.Commit());

		// Check success.
		TS_ASSERT(fileListGenerator.AreAllRenamed());
	}

private:
	void OnRenamed(const Beroux::IO::Renaming::CPath& pathNameBefore, const Beroux::IO::Renaming::CPath& pathNameAfter)
	{
	}

	void OnRenameError(const Beroux::IO::Renaming::IRenameError& renameError)
	{
	}

	void OnProgress(Beroux::IO::Renaming::CRenamingList::EStage nStage, int nDone, int nTotal)
	{
	}
};
