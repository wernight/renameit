#include <cxxtest/TestSuite.h>
#include "Helper/FileListGenerator.h"
#include "../resource.h"

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

	void testSingleFolderRenaming()
	{
		// Prepare
		CFileListGenerator fileListGenerator;
		fileListGenerator.AddDirectory(_T("dir_before"), _T("dir_after"));

		// Rename
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

	void testCyclicFolderRenaming()
	{
		// Prepare
		CFileListGenerator fileListGenerator;
		fileListGenerator.AddDirectory(_T("d"),		_T("a"));
		fileListGenerator.AddDirectory(_T("ccc"),	_T("d"));
		fileListGenerator.AddDirectory(_T("a"),		_T("bb"));
		fileListGenerator.AddDirectory(_T("bb"),	_T("ccc"));

		// Rename
		if (!fileListGenerator.PerformRenaming())
			TS_FAIL(fileListGenerator.GetRenamingErrors().c_str());
	}

	void testEmboxedFolderRenaming()
	{
		// Prepare
		CFileListGenerator fileListGenerator;
		fileListGenerator.AddDirectory(_T("a\\b\\c\\d"),	_T("a\\b2\\c2\\d2"));
		fileListGenerator.AddDirectory(_T("a\\b"),			_T("a\\b2"));
		fileListGenerator.AddDirectory(_T("a\\b\\c"),		_T("a\\b2\\c2"));

		// Rename
		if (!fileListGenerator.PerformRenaming(false))
			TS_FAIL(fileListGenerator.GetRenamingErrors().c_str());
	}
	
	void testEmboxedFolderFilesRenaming()
	{
		// Prepare
		CFileListGenerator fileListGenerator;
		fileListGenerator.AddFile(_T("a\\b\\c\\d\\file1"),	_T("a\\b2\\c2\\d2\\file1"));
		fileListGenerator.AddFile(_T("a\\b\\file2"),		_T("a\\b2\\file2"));
		fileListGenerator.AddFile(_T("a\\b\\c\\file3"),		_T("a\\b2\\c2\\file3"));

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

	void testExistingFolderCaseUnification2()
	{
		// Prepare
		CFileListGenerator fileListGenerator;
		fileListGenerator.CreateDirectory(_T("dirA\\dirB"));
		fileListGenerator.AddDirectory(_T("dirA\\dir1"), _T("DIRA\\dir1"));
		fileListGenerator.AddDirectory(_T("dirC\\dirD\\dir2"), _T("DIRA\\DIRb\\dir2"));

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

	void testFileCheckingError()
	{
		// Test almost every possible bad file name issuing an error.

		// Prepare
		CFileListGenerator fileListGenerator;
		// Conflict
		fileListGenerator.AddFile(_T("file1_a"), _T("file_after"));
		fileListGenerator.AddFile(_T("file1_b"), _T("file_after"));
		// Cannot replaces existing
		fileListGenerator.CreateFile(_T("file2A_existing_after"));
		fileListGenerator.CreateDirectory(_T("file2B_existing_after"));
		fileListGenerator.AddFile(_T("file2A"), _T("file2A_existing_after"));
		fileListGenerator.AddFile(_T("file2B"), _T("file2B_existing_after"));
		// Missing
		fileListGenerator.GetBeforeFileList().AddPath(fileListGenerator.GetTempDirectory() + _T("file_existing_before"));
		fileListGenerator.GetAfterFileList().AddPath(fileListGenerator.GetTempDirectory() + _T("file_existing_after"));
		// Invalid name: Empty
		fileListGenerator.AddFile(_T("empty_test"), _T("changed_below"));
		fileListGenerator.GetAfterFileList().RemoveFile(fileListGenerator.GetAfterFileList().GetCount() - 1);
		fileListGenerator.GetAfterFileList().AddPath(_T("\\\\?\\"));
		// Invalid name: Invalid character
		fileListGenerator.AddFile(_T("file3"), _T(":_file3_after"));
		fileListGenerator.AddFile(_T("file4"), _T("file4_after."));
		fileListGenerator.AddFile(_T("file5"), _T("file5_\n_after"));
		fileListGenerator.AddFile(_T("file6"), _T("file6._>_"));
		// Path: No root
		fileListGenerator.AddFile(_T("file7"), _T("no_root_after"));
		fileListGenerator.GetAfterFileList().RemoveFile(fileListGenerator.GetAfterFileList().GetCount() - 1);
		fileListGenerator.GetAfterFileList().AddPath(_T("no_root_after"));
		// Path: Invalid name
		fileListGenerator.AddFile(_T("file8"), _T("\\\\empty_folder_after"));
		fileListGenerator.AddFile(_T("file9"), _T("invalid_folder_name_|_after"));
		// File open without shared renaming access.
		fileListGenerator.AddDirectory(_T("file10"), _T("file10_after"));
		HANDLE hFile = ::CreateFile(fileListGenerator.GetTempDirectory() + _T("file10"),
			GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, 
			FILE_ATTRIBUTE_TEMPORARY,
			NULL);
		TS_ASSERT(hFile != INVALID_HANDLE_VALUE);

		// Check
		CRenamingList renamingList = fileListGenerator.MakeRenamingList();
		TS_ASSERT_EQUALS(0, renamingList.GetErrorCount());
		TS_ASSERT_EQUALS(0, renamingList.GetWarningCount());

		TS_ASSERT(!renamingList.Check());
		TS_ASSERT_EQUALS(fileListGenerator.GetBeforeFileList().GetCount(), renamingList.GetErrorCount());
		TS_ASSERT_EQUALS(0, renamingList.GetWarningCount());
		for (int i=0; i<renamingList.GetCount(); ++i)
			if (renamingList.GetOperationProblem(i).nErrorLevel != CRenamingList::levelError)
				TS_FAIL(i);

		// Close the file.
		::CloseHandle(hFile);
	}

	void testFileCheckingWarning()
	{
		// Test almost every possible bad file name issuing a warning.

		// Prepare
		CFileListGenerator fileListGenerator;
		// Long name.
		fileListGenerator.AddFile(_T("file1"), _T("121 characters looooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooong"));
		// Reserved name.
		fileListGenerator.AddFile(_T("file2"), _T("COM3.txt"));
		// Not trimmed.
		fileListGenerator.AddFile(_T("file3"), _T(" file3"));
		fileListGenerator.AddFile(_T("file4"), _T("file4 "));
		// Long path
		fileListGenerator.AddFile(_T("over_max_path"), _T("000000000011111111112222222222333333333344444444445555555555666666666677777777778888888888999999999900000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000111111111122222222223333333333444444444455555555556666666666"));
		// Inconsistent parent case.
		fileListGenerator.AddFile(_T("file5"), _T("parent\\ok\\file5"));
		fileListGenerator.AddFile(_T("file6"), _T("PARENT\\ok\\file6"));

		// Check
		CRenamingList renamingList = fileListGenerator.MakeRenamingList();
		TS_ASSERT_EQUALS(0, renamingList.GetErrorCount());
		TS_ASSERT_EQUALS(0, renamingList.GetWarningCount());

		TS_ASSERT(!renamingList.Check());
		TS_ASSERT_EQUALS(0, renamingList.GetErrorCount());
		TS_ASSERT_EQUALS(fileListGenerator.GetBeforeFileList().GetCount(), renamingList.GetWarningCount());
		for (int i=0; i<renamingList.GetCount(); ++i)
			if (renamingList.GetOperationProblem(i).nErrorLevel != CRenamingList::levelWarning)
				TS_FAIL(i);
	}

	void testDirectoryCheckingError()
	{
		// Like testFileCheckingError

		// Prepare
		CFileListGenerator fileListGenerator;
		// Conflict
		fileListGenerator.AddDirectory(_T("file1"), _T("file_after"));
		fileListGenerator.AddDirectory(_T("file2"), _T("file_after"));
		// Cannot replaces existing
		fileListGenerator.CreateFile(_T("file2A_existing_after"));
		fileListGenerator.CreateDirectory(_T("file2B_existing_after"));
		fileListGenerator.AddDirectory(_T("file2A"), _T("file2A_existing_after"));
		fileListGenerator.AddDirectory(_T("file2B"), _T("file2B_existing_after"));
		// Missing
		fileListGenerator.GetBeforeFileList().AddPath(fileListGenerator.GetTempDirectory() + _T("file_existing_before"));
		fileListGenerator.GetAfterFileList().AddPath(fileListGenerator.GetTempDirectory() + _T("file_existing_after"));
		// Invalid name: Empty
		fileListGenerator.AddDirectory(_T("empty_test"), _T("changed_below"));
		fileListGenerator.GetAfterFileList().RemoveFile(fileListGenerator.GetAfterFileList().GetCount() - 1);
		fileListGenerator.GetAfterFileList().AddPath(_T("\\\\?\\"));
		// Invalid name: Invalid character
		fileListGenerator.AddDirectory(_T("file3"), _T(":_file3_after"));
		fileListGenerator.AddDirectory(_T("file4"), _T("file4_after."));
		fileListGenerator.AddDirectory(_T("file5"), _T("file5_\n_after"));
		fileListGenerator.AddDirectory(_T("file6"), _T("file6._>_"));
		// Path: No root
		fileListGenerator.AddDirectory(_T("file7"), _T("no_root_after"));
		fileListGenerator.GetAfterFileList().RemoveFile(fileListGenerator.GetAfterFileList().GetCount() - 1);
		fileListGenerator.GetAfterFileList().AddPath(_T("no_root_after"));
		// Path: Invalid name
		fileListGenerator.AddDirectory(_T("file8"), _T("\\\\empty_folder_after"));
		fileListGenerator.AddDirectory(_T("file9"), _T("invalid_folder_name_|_after"));
		// File open without shared renaming access.
		fileListGenerator.AddDirectory(_T("file10"), _T("file10_after"));
		HANDLE hFile = ::CreateFile(fileListGenerator.GetTempDirectory() + _T("file10"),
			GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, 
			FILE_ATTRIBUTE_TEMPORARY,
			NULL);
		TS_ASSERT(hFile != INVALID_HANDLE_VALUE);

		// Check
		CRenamingList renamingList = fileListGenerator.MakeRenamingList();
		TS_ASSERT_EQUALS(0, renamingList.GetErrorCount());
		TS_ASSERT_EQUALS(0, renamingList.GetWarningCount());

		TS_ASSERT(!renamingList.Check());
		TS_ASSERT_EQUALS(fileListGenerator.GetBeforeFileList().GetCount(), renamingList.GetErrorCount());
		TS_ASSERT_EQUALS(0, renamingList.GetWarningCount());
		for (int i=0; i<renamingList.GetCount(); ++i)
			if (renamingList.GetOperationProblem(i).nErrorLevel != CRenamingList::levelError)
				TS_FAIL(i);

		// Close the file.
		::CloseHandle(hFile);
	}

	void testDirectoryCheckingWarning()
	{
		// Like testFileCheckingWarning

		// Prepare
		CFileListGenerator fileListGenerator;
		// Long name.
		fileListGenerator.AddDirectory(_T("file1"), _T("121 characters looooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooong"));
		// Reserved name.
		fileListGenerator.AddDirectory(_T("file2"), _T("COM3.txt"));
		// Not trimmed.
		fileListGenerator.AddDirectory(_T("file3"), _T(" file3"));
		fileListGenerator.AddDirectory(_T("file4"), _T("file4 "));
		// Long path
		fileListGenerator.AddDirectory(_T("over_max_path"), _T("000000000011111111112222222222333333333344444444445555555555666666666677777777778888888888999999999900000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000111111111122222222223333333333444444444455555555556666666666"));
		// Inconsistent parent case.
		fileListGenerator.AddDirectory(_T("file5"), _T("parent\\ok\\file5"));
		fileListGenerator.AddDirectory(_T("file6"), _T("PARENT\\ok\\file6"));

		// Check
		CRenamingList renamingList = fileListGenerator.MakeRenamingList();
		TS_ASSERT_EQUALS(0, renamingList.GetErrorCount());
		TS_ASSERT_EQUALS(0, renamingList.GetWarningCount());

		TS_ASSERT(!renamingList.Check());
		TS_ASSERT_EQUALS(0, renamingList.GetErrorCount());
		TS_ASSERT_EQUALS(fileListGenerator.GetBeforeFileList().GetCount(), renamingList.GetWarningCount());
		for (int i=0; i<renamingList.GetCount(); ++i)
			if (renamingList.GetOperationProblem(i).nErrorLevel != CRenamingList::levelWarning)
				TS_FAIL(i);
	}

	void testCompressedFileRenaming()
	{
		// Prepare
		CFileListGenerator fileListGenerator;
		fileListGenerator.AddFile(_T("file_before"), _T("file_after"), FILE_ATTRIBUTE_COMPRESSED);

		// Rename
		if (!fileListGenerator.PerformRenaming())
			TS_FAIL(fileListGenerator.GetRenamingErrors().c_str());
	}

	void testEncryptedFileRenaming()
	{
		// Prepare
		CFileListGenerator fileListGenerator;
		fileListGenerator.AddFile(_T("file_before"), _T("file_after"), FILE_ATTRIBUTE_ENCRYPTED);

		// Rename
		if (!fileListGenerator.PerformRenaming())
			TS_FAIL(fileListGenerator.GetRenamingErrors().c_str());
	}

	void testRenamingToAnotherDrive()
	{
		// TODO: Rename from one drive to another (removable if existing).
	}

	void testNetworkDriveFileRenaming()
	{
		// TODO: Rename:
		// - network --> network
		// - network --> local disk
		// - local disk --> network
	}

	void testSubstituteDriveFileRenaming()
	{
		// Done by "subst X: C:\Folder\".

		// TODO
	}
};
