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

	void testSingleFolderRenaming()
	{
		// Prepare
		CFileListGenerator fileListGenerator;
		CKtmTransaction ktm;
		fileListGenerator.AddFolder(_T("dir_before"), _T("dir_after"));
		CRenamingList renList = fileListGenerator.MakeRenamingList();

		// Rename
		TS_ASSERT(renList.Check());
		if (!renList.Check())
			PrintCheckingErrors(renList);
		TS_ASSERT(renList.PerformRenaming(ktm));
		TS_ASSERT(ktm.Commit());

		// Check success.
		TS_ASSERT(fileListGenerator.AreAllRenamed());
	}
	
	void testCyclicFolderRenaming()
	{
		// TODO
	}

	void testEmboxedFolderRenaming()
	{
		// Ex: Adding "C:\A\B\" may add the folders:
		// - C:\A\B\
		// - C:\A\B\C1\
		// - C:\A\B\C2\
		// The folders "B", "C1", and "C2" are renamed; "A" is not.

		// TODO
	}
	
	void testCyclicEmboxedFolderRenaming()
	{
		// TODO
	}
	
	void testRemoveRenamedEmptyParentsFolders()
	{
		// After the renaming is complete and only when it's successful, check
		// if some files have moves from one folder A to a new folder B,
		// and if there are no more files in the folder A after renaming,
		// then if the user wants to delete the folder A (list all the folders
		// in the same situation ?).
		//
		// By definition, this test only applies when renaming files.

		// TODO
	}
	
	void testFoldersCaseUnfication()
	{
		// There should be another difference: When the case differ, the case of
		// the directory being renamed prevails on the case of some directory
		// that are parent of another. Meaning that for two operations:
		// - "C:\A\B\" --> "C:\A\b\"       (1)
		// - "C:\A\B\C\" --> "C:\A\B\c\"   (2)
		// The case of "B" as defined by the operation (1) prevails on the case
		// as defined in the operation (2). So in the end it'll be "C:\A\b\c\".

		// TODO
	}

	void testUnicodePath()
	{
		// Test to rename path provided with \\?\...

		// TODO
	}

	void testFolderChecking()
	{
		// Make similar checking as for files.
		//
		// When moving a directory, the destination must be on the same drive.
		//
		// Directories cannot contain '/' but the path can contain '\'.
		//
		// Windows Me/98/95:  [The full directory path] must not exceed MAX_PATH characters.
		//
		// For every renamed/created folder, if a file of the same name exists,
		// deny it. If a folder of the same name exists, then deny it (later
		// we'll allow to override).

		// TODO
	}

	void testCompressedFileRenaming()
	{
		// TODO
	}

	void testEncryptedFileRenaming()
	{
		// TODO
	}

	void testNetworkDriveFileRenaming()
	{
		// TODO
	}

	void testSubstituteDriveFileRenaming()
	{
		// Done by "subst X: C:\Folder\".

		// TODO
	}

private:
	void PrintCheckingErrors(const CRenamingList& renamingList)
	{
		tcout << "Checking failed:" << endl;

		for (int i=0; i<renamingList.GetCount(); ++i)
			if (renamingList.GetOperationProblem(i).nErrorLevel != CRenamingList::levelNone)
				tcout 
				<< '`' << (LPCTSTR)renamingList.GetRenamingOperation(i).GetPathBefore().GetFileName() << '`'
				<< " --> "
				<< '`' << (LPCTSTR)renamingList.GetRenamingOperation(i).GetPathAfter().GetFileName() << '`'
				<< ": " << (LPCTSTR)renamingList.GetOperationProblem(i).strMessage
				<< endl;

		tcout << endl;
	}

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
