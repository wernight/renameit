#include <cxxtest/TestSuite.h>
#include "Helper/FileListGenerator.h"

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
		fileListGenerator.AddFile(_T("a.tmp"), _T("a-after.tmp"));

		// Rename
		if (!fileListGenerator.PerformRenaming())
			TS_FAIL(fileListGenerator.GetRenamingErrors().c_str());
	}

	void testEmptyRenaming()
	{
		// Prepare
		CFileListGenerator fileListGenerator;

		// Rename nothing
		if (!fileListGenerator.PerformRenaming())
			TS_FAIL(fileListGenerator.GetRenamingErrors().c_str());
	}

	void testCyclicFileRenaming()
	{
		// Prepare
		CFileListGenerator fileListGenerator;
		fileListGenerator.AddFile(_T("1.tmp"), _T("2.tmp"));
		fileListGenerator.AddFile(_T("4.tmp"), _T("5.tmp"));
		fileListGenerator.AddFile(_T("2.tmp"), _T("3.tmp"));
		fileListGenerator.AddFile(_T("3.tmp"), _T("4.tmp"));
		fileListGenerator.AddFile(_T("5.tmp"), _T("1.tmp"));

		// Rename
		if (!fileListGenerator.PerformRenaming())
			TS_FAIL(fileListGenerator.GetRenamingErrors().c_str());
	}

	void testSingleFolderRenaming()
	{
		// Prepare
		CFileListGenerator fileListGenerator;
		fileListGenerator.AddDirectory(_T("dir_before"), _T("dir_after"));

		// Rename
		if (!fileListGenerator.PerformRenaming())
			TS_FAIL(fileListGenerator.GetRenamingErrors().c_str());
	}
	
	void testCyclicFolderRenaming()
	{
		// Prepare
		CFileListGenerator fileListGenerator;
		fileListGenerator.AddDirectory(_T("d"), _T("a"));
		fileListGenerator.AddDirectory(_T("ccc"), _T("d"));
		fileListGenerator.AddDirectory(_T("a"), _T("bb"));
		fileListGenerator.AddDirectory(_T("bb"), _T("ccc"));

		// Rename
		if (!fileListGenerator.PerformRenaming())
			TS_FAIL(fileListGenerator.GetRenamingErrors().c_str());
	}

	void testEmboxedFolderRenaming()
	{
		// Prepare
		CFileListGenerator fileListGenerator;
		fileListGenerator.AddDirectory(_T("a\\b\\c\\d"), _T("a\\b2\\c2\\d2"));
		fileListGenerator.AddDirectory(_T("a\\b"), _T("a\\b2"));
		fileListGenerator.AddDirectory(_T("a\\b\\c"), _T("a\\b2\\c2"));

		// Rename
		if (!fileListGenerator.PerformRenaming(false))
			TS_FAIL(fileListGenerator.GetRenamingErrors().c_str());
	}
	
	void testComplexFolderRenaming()
	{
		for (int i=0; i<5; ++i)
		{
			// Prepare
			CFileListGenerator fileListGenerator;
			fileListGenerator.AddDirectory(_T("a\\b\\c\\d"), _T("a\\B\\d2"));
			fileListGenerator.AddDirectory(_T("a\\b"), _T("a\\b2"));
			fileListGenerator.AddDirectory(_T("a\\b\\c"), _T("a\\b2\\c2"));
			fileListGenerator.AddDirectory(_T("a\\b2"), _T("a\\B\\c2"));
			fileListGenerator.AddDirectory(_T("a\\b\\c22"), _T("a\\B"));
			fileListGenerator.AddDirectory(_T("a\\b\\c\\d2"), _T("a\\B\\c22"));

			// Randomize
			fileListGenerator.RandomizeOperationsOrder(i);

			// Rename
			if (!fileListGenerator.PerformRenaming())
				TS_FAIL(fileListGenerator.GetRenamingErrors().c_str());
		}
	}
	
	void testComplifiedFolderRenaming()
	{
		// A simple renaming operation made complex.

		// Prepare
		CFileListGenerator fileListGenerator;
		fileListGenerator.AddDirectory(_T("a"), _T("b"));
		fileListGenerator.AddDirectory(_T("a/a"), _T("b/b"));
		fileListGenerator.AddDirectory(_T("a/a/a"), _T("b/b/b"));

		// Rename
		if (!fileListGenerator.PerformRenaming())
			TS_FAIL(fileListGenerator.GetRenamingErrors().c_str());
	}

	void testComplified2FolderRenaming()
	{
		// Prepare
		CFileListGenerator fileListGenerator;
		fileListGenerator.AddDirectory(_T("a"), _T("b"));
		fileListGenerator.AddDirectory(_T("a\\b"), _T("b\\c"));
		fileListGenerator.CreateFile(_T("a\\file1"));
		fileListGenerator.CreateFile(_T("a\\b\\file2"));

		// Rename
		if (!fileListGenerator.PerformRenaming())
			TS_FAIL(fileListGenerator.GetRenamingErrors().c_str());

		// Check
		TS_ASSERT(CPath::PathFileExists(fileListGenerator.GetTempDirectory() + _T("b\\file1")));
		TS_ASSERT(CPath::PathFileExists(fileListGenerator.GetTempDirectory() + _T("b\\c\\file2")));
	}

	void testIdenticalParentFolderRenaming()
	{
		// Prepare
		CFileListGenerator fileListGenerator;
		fileListGenerator.AddDirectory(_T("A"), _T("B"));
		fileListGenerator.AddDirectory(_T("B"), _T("A\\B"));
		fileListGenerator.AddDirectory(_T("C"), _T("A"));
		fileListGenerator.CreateFile(_T("A\\file1"));
		fileListGenerator.CreateFile(_T("B\\file2"));
		fileListGenerator.CreateFile(_T("C\\file3"));

		// Rename
		if (!fileListGenerator.PerformRenaming())
			TS_FAIL(fileListGenerator.GetRenamingErrors().c_str());

		// Check
		TS_ASSERT(CPath::PathFileExists(fileListGenerator.GetTempDirectory() + _T("B\\file1")));
		TS_ASSERT(CPath::PathFileExists(fileListGenerator.GetTempDirectory() + _T("A\\B\\file2")));
		TS_ASSERT(CPath::PathFileExists(fileListGenerator.GetTempDirectory() + _T("A\\file3")));
	}

	void testNewFoldersCaseUnfication()
	{
		// There should be another difference: When the case differ, the case of
		// the directory being renamed prevails on the case of some directory
		// that are parent of another. Meaning that for two operations:
		// - "C:\A\B\" --> "C:\A\b\"       (1)
		// - "C:\A\B\C\" --> "C:\A\B\c\"   (2)
		// The case of "B" as defined by the operation (1) prevails on the case
		// as defined in the operation (2). So in the end it'll be "C:\A\b\c\".

		// Prepare
		CFileListGenerator fileListGenerator;
		fileListGenerator.AddDirectory(_T("A\\B"), _T("A\\b"));
		fileListGenerator.AddDirectory(_T("A\\B\\C"), _T("A\\B\\c"));

		// Rename
		if (!fileListGenerator.PerformRenaming())
			TS_FAIL(fileListGenerator.GetRenamingErrors().c_str());

		// Check case
		CString strExpectFullUnifiedDirectoryPath = fileListGenerator.GetTempDirectory() + _T("A\\b\\c");
		CString strResultDirectoryPath = CPath::FindPathCase(strExpectFullUnifiedDirectoryPath);
		TS_ASSERT_EQUALS(strExpectFullUnifiedDirectoryPath, strResultDirectoryPath);
	}

	void testExistingFolderCaseUnification()
	{
		// Prepare
		CFileListGenerator fileListGenerator;
		fileListGenerator.CreateDirectory(_T("dirA\\dirB"));
		fileListGenerator.AddFile(_T("dirA\\file1"), _T("DIRA\\file1"));
		fileListGenerator.AddFile(_T("dirC\\dirD\\file2"), _T("DIRA\\DIRb\\file2"));

		// Rename
		if (!fileListGenerator.PerformRenaming())
			TS_FAIL(fileListGenerator.GetRenamingErrors().c_str());

		// Check the case.
		CString strExpectFullUnifiedDirectoryPath = fileListGenerator.GetTempDirectory() + _T("DIRA\\DIRb");
		CString strResultDirectoryPath = CPath::FindPathCase(strExpectFullUnifiedDirectoryPath);
		TS_ASSERT_EQUALS(strExpectFullUnifiedDirectoryPath, strResultDirectoryPath);
	}

	void testRemoveRenamedEmptyParentsFolders()
	{
		// After the renaming is complete and only when it's successful, check
		// if some files have moves from one folder A to a new folder B,
		// and if there are no more files in the folder A after renaming,
		// then if the user wants to delete the folder A (list all the folders
		// in the same situation ?).
		//
		// This test applies to files and folders renaming.

		// Test with files
		{
			// Prepare
			CFileListGenerator fileListGenerator;
			fileListGenerator.AddFile(_T("a1\\b1\\c1\\d1"), _T("a2"));
			fileListGenerator.AddFile(_T("a1\\b1\\c2"), _T("a3"));
			fileListGenerator.AddFile(_T("a1\\b2"), _T("a1\\b3"));

			// Rename
			if (!fileListGenerator.PerformRenaming())
				TS_FAIL(fileListGenerator.GetRenamingErrors().c_str());

			// Check
			TS_ASSERT(!CPath::PathFileExists(fileListGenerator.GetTempDirectory() + _T("a1\\b1\\c1")));
			TS_ASSERT(!CPath::PathFileExists(fileListGenerator.GetTempDirectory() + _T("a1\\b1")));
			TS_ASSERT(CPath::PathFileExists(fileListGenerator.GetTempDirectory() + _T("a1")));
		}

		// Test with folders
		{
			// Prepare
			CFileListGenerator fileListGenerator;
			fileListGenerator.AddDirectory(_T("a1\\b1\\c1\\d1"), _T("a2"));
			fileListGenerator.AddDirectory(_T("a1\\b1\\c2"), _T("a3"));
			fileListGenerator.AddDirectory(_T("a1\\b2"), _T("a1\\b3"));

			// Rename
			if (!fileListGenerator.PerformRenaming())
				TS_FAIL(fileListGenerator.GetRenamingErrors().c_str());

			// Check
			TS_ASSERT(!CPath::PathFileExists(fileListGenerator.GetTempDirectory() + _T("a1\\b1\\c1")));
			TS_ASSERT(!CPath::PathFileExists(fileListGenerator.GetTempDirectory() + _T("a1\\b1")));
			TS_ASSERT(CPath::PathFileExists(fileListGenerator.GetTempDirectory() + _T("a1")));
		}
	}

	void testUnicodePath()
	{
		// Test to rename path provided with \\?\...

		// TODO
	}

	void testFileChecking()
	{
		// TODO: Test almost every possible bad file name issuing warnings or errors.
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

	void testRenamingToAnotherDrive()
	{
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
};
