#include "StdAfx.h"
#include <boost/test/unit_test.hpp>
#include "Helper/FileListGenerator.h"
#include "../resource.h"

using namespace Beroux::IO;
using namespace Beroux::IO::Renaming;

BOOST_AUTO_TEST_SUITE(RenamingListTestSuite);

struct CRenamingListFixture
{
	CRenamingListFixture()
	{
		// Set the instance to that resources can be loaded (i.e., for TString::LoadString() to work).
		HINSTANCE hInstance = (HINSTANCE) GetModuleHandle(NULL);
		AfxSetResourceHandle(hInstance);
	}
};

void testCheckingError(bool bTestFileRenaming)
{
	// Test almost every possible bad file name issuing an error.

	// Prepare
	CFileListGenerator fileListGenerator;
	function<void (const CString&, const CString&)> fAddOperation;
	if (bTestFileRenaming)
		fAddOperation = bind(&CFileListGenerator::AddFile, ref(fileListGenerator), _1, _2, FILE_ATTRIBUTE_NORMAL);
	else
		fAddOperation = bind(&CFileListGenerator::AddDirectory, ref(fileListGenerator), _1, _2);

	// Conflict
	fAddOperation(_T("before1A"), _T("file1_after"));
	fAddOperation(_T("before1B"), _T("file1_after"));
	// Cannot replaces existing
	fileListGenerator.CreateFile(_T("file2A_existing_after"));
	fileListGenerator.CreateDirectory(_T("file2B_existing_after"));
	fAddOperation(_T("file2A"), _T("file2A_existing_after"));
	fAddOperation(_T("file2B"), _T("file2B_existing_after"));
	// Missing
	fileListGenerator.GetBeforeFileList().AddPath(fileListGenerator.GetTempDirectory() + _T("file_existing_before"));
	fileListGenerator.GetAfterFileList().AddPath(fileListGenerator.GetTempDirectory() + _T("file_existing_after"));
	// Invalid name: Empty
	fAddOperation(_T("empty_test"), _T("changed_below"));
	fileListGenerator.GetAfterFileList().SetPath(fileListGenerator.GetAfterFileList().GetCount() - 1, _T("\\\\?\\"));
	// Invalid name: Invalid character
	fAddOperation(_T("before3A"), _T(":_after3"));
	fAddOperation(_T("before3B"), _T("after3."));
	fAddOperation(_T("before3C"), _T("after3_\n_"));
	fAddOperation(_T("before3D"), _T("after3.>"));
	// Path: No root
	fAddOperation(_T("before7"), _T("no_root_after"));
	fileListGenerator.GetAfterFileList().SetPath(fileListGenerator.GetAfterFileList().GetCount() - 1, _T("no_root_after"));
	// Invalid parent name
	fAddOperation(_T("before8A"), _T("\\\\empty_folder_after\\after8"));
	fAddOperation(_T("before8B"), _T("invalid_folder_name_|_\\after8"));
	// File open without shared renaming access.
	HANDLE hOpenFile = NULL;
	if (bTestFileRenaming)
	{
		fAddOperation(_T("file10"), _T("file10_after"));

		hOpenFile = ::CreateFile(fileListGenerator.GetTempDirectory() + _T("file10"),
			GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, 
			FILE_ATTRIBUTE_TEMPORARY,
			NULL);
		BOOST_CHECK(hOpenFile != INVALID_HANDLE_VALUE);
	}

	// Check
	CRenamingList renamingList = fileListGenerator.MakeRenamingList();
	BOOST_CHECK_EQUAL(0, renamingList.GetErrorCount());
	BOOST_CHECK_EQUAL(0, renamingList.GetWarningCount());

	BOOST_CHECK(!renamingList.Check());
	BOOST_CHECK_EQUAL(fileListGenerator.GetBeforeFileList().GetCount(), renamingList.GetErrorCount());
	BOOST_CHECK_EQUAL(0, renamingList.GetWarningCount());
	for (int i=0; i<renamingList.GetCount(); ++i)
		if (renamingList.GetOperationProblem(i).nErrorLevel != CRenamingList::levelError)
			BOOST_FAIL(i);

	// Close the file.
	if (hOpenFile != NULL)
		::CloseHandle(hOpenFile);
}

void testCheckingWarning(bool bTestFileRenaming)
{
	// Test almost every possible bad file name issuing a warning.

	// Prepare
	CFileListGenerator fileListGenerator;
	function<void (const CString&, const CString&)> fAddOperation;
	if (bTestFileRenaming)
		fAddOperation = bind(&CFileListGenerator::AddFile, ref(fileListGenerator), _1, _2, FILE_ATTRIBUTE_NORMAL);
	else
		fAddOperation = bind(&CFileListGenerator::AddDirectory, ref(fileListGenerator), _1, _2);

	// Long name.
	fAddOperation(_T("before1A"), _T("121 characters looooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooong"));
	// Long parent name
	fAddOperation(_T("before1B"), _T("121 characters looooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooong\\hello"));
	// Reserved name.
	fAddOperation(_T("before2A"), _T("COM3.txt"));
	fAddOperation(_T("before2B"), _T("LPT1\\after2B"));
	// Not trimmed.
	fAddOperation(_T("before3A"), _T(" file3A"));
	fAddOperation(_T("before3B"), _T("file3B "));
	// Long path
	fAddOperation(_T("over_max_path"), _T("000000000011111111112222222222333333333344444444445555555555666666666677777777778888888888999999999900000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000111111111122222222223333333333444444444455555555556666666666"));
	// Inconsistent parent case.
	fAddOperation(_T("before4A"), _T("parent\\ok\\before4A"));
	fAddOperation(_T("before4B"), _T("PARENT\\ok\\before4B"));

	// Check
	CRenamingList renamingList = fileListGenerator.MakeRenamingList();
	BOOST_CHECK_EQUAL(0, renamingList.GetErrorCount());
	BOOST_CHECK_EQUAL(0, renamingList.GetWarningCount());

	BOOST_CHECK(!renamingList.Check());
	BOOST_CHECK_EQUAL(0, renamingList.GetErrorCount());
	BOOST_CHECK_EQUAL(fileListGenerator.GetBeforeFileList().GetCount(), renamingList.GetWarningCount());
	for (int i=0; i<renamingList.GetCount(); ++i)
		if (renamingList.GetOperationProblem(i).nErrorLevel != CRenamingList::levelWarning)
			BOOST_FAIL(i);
}

CString GetAnotherDriveRoot(unsigned nDriveTypeFlags = DRIVE_FIXED)
{
	// Get the current drive to exclude.
	CFileListGenerator fileListGenerator;
	char chSourceDrive = CPath::MakeSimplePath(fileListGenerator.GetTempDirectory()).GetAt(0);

	// List all available drives.
	CStringList strDriveList;
	{
		TCHAR szDrives[512];
		DWORD dwLength = ::GetLogicalDriveStrings(sizeof(szDrives)/sizeof(szDrives[0]), szDrives);
		if (dwLength > 0 && dwLength < sizeof(szDrives)/sizeof(szDrives[0]))
		{
			for (DWORD i=0; i<dwLength; i+=_tcslen(&szDrives[i]) + 1)
				strDriveList.AddTail(&szDrives[i]);
		}
	}

	// Get a drive that matches the provided flag.
	for (POSITION pos=strDriveList.GetHeadPosition(); pos!=NULL; )
	{
		CString strDriveRoot = strDriveList.GetNext(pos);

		// Skip the source drive.
		if (strDriveRoot[0] == chSourceDrive)
			continue;

		if (::GetDriveType(strDriveRoot) == nDriveTypeFlags)
			return strDriveRoot;
	}

	return _T("");
}

void testRenameToAnotherDrive(bool renameFile)
{
	// Find a drive to do the test.
	CString strDestinationDrive = GetAnotherDriveRoot();
	if (strDestinationDrive.IsEmpty())
		BOOST_FAIL("No other drive could be found to make the test.");
	CString strDestinationDirectory = strDestinationDrive + _T("rename-it_unit_test.tmp");
	CString strDestinationPath = strDestinationDirectory + _T("\\after.tmp");

	// Prepare
	CFileListGenerator fileListGenerator;
	if (renameFile)
		fileListGenerator.AddFile(_T("before"), _T("xxxx"));
	else
		fileListGenerator.AddDirectory(_T("before"), _T("xxxx"));
	fileListGenerator.GetAfterFileList().SetPath(0, strDestinationPath);

	// Rename
	if (!fileListGenerator.PerformRenaming())
		BOOST_FAIL(fileListGenerator.GetRenamingErrors().c_str());

	// Delete the temporary files/folders.
	if (renameFile) {
		BOOST_CHECK(::DeleteFile(strDestinationPath));
	} else {
		BOOST_CHECK(::RemoveDirectory(strDestinationPath));
	}
	BOOST_CHECK(::RemoveDirectory(strDestinationDirectory));
}

void testNetworkDriveRenaming(bool bTestFileRenaming)
{
	// Rename:
	// - local disk --> network
	// - network --> local disk

	// Get a network root.
	CString strNetworkRoot = _T("\\\\localhost\\public\\");
	CString strNetworkDirectory = strNetworkRoot + _T("rename-it_unit_test_tmp");
	CString strNetworkPath1After = strNetworkDirectory + _T("\\1.tmp");
	CString strNetworkPath2Before = strNetworkRoot + _T("2.tmp");

	// Prepare
	CFileListGenerator fileListGenerator;
	if (bTestFileRenaming)
		fileListGenerator.AddFile(_T("before1"), _T("xxxx"));
	else
		fileListGenerator.AddDirectory(_T("before1"), _T("xxxx"));
	fileListGenerator.GetAfterFileList().SetPath(fileListGenerator.GetAfterFileList().GetCount() - 1, strNetworkPath1After);

	fileListGenerator.AddDirectory(_T("xxxx"), _T("after2"));
	if (bTestFileRenaming)
	{
		BOOST_CHECK(::CloseHandle(::CreateFile(strNetworkPath2Before,
			GENERIC_WRITE,
			FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, CREATE_NEW, 
			FILE_ATTRIBUTE_TEMPORARY,
			NULL)));
	}
	else
		BOOST_CHECK(::CreateDirectory(strNetworkPath2Before, NULL));
	fileListGenerator.GetBeforeFileList().SetPath(fileListGenerator.GetBeforeFileList().GetCount() - 1, strNetworkPath2Before);

	// Rename
	if (!fileListGenerator.PerformRenaming())
		BOOST_FAIL(fileListGenerator.GetRenamingErrors().c_str());

	// Delete the temporary files/folders.
	if (bTestFileRenaming) {
		::DeleteFile(strNetworkPath1After);
		::DeleteFile(strNetworkPath2Before);
	} else {
		::RemoveDirectory(strNetworkPath1After);
		::RemoveDirectory(strNetworkPath2Before);
	}
	::RemoveDirectory(strNetworkDirectory);
}

BOOST_FIXTURE_TEST_CASE(SingleFileRenaming, CRenamingListFixture)
{
	// Prepare
	CFileListGenerator fileListGenerator;
	fileListGenerator.AddFile(_T("a.tmp"), _T("a-after.tmp"));

	// Rename
	if (!fileListGenerator.PerformRenaming())
		BOOST_FAIL(fileListGenerator.GetRenamingErrors().c_str());
}

BOOST_FIXTURE_TEST_CASE(EmptyRenaming, CRenamingListFixture)
{
	// Prepare
	CFileListGenerator fileListGenerator;

	// Rename nothing
	if (!fileListGenerator.PerformRenaming())
		BOOST_FAIL(fileListGenerator.GetRenamingErrors().c_str());
}

BOOST_FIXTURE_TEST_CASE(SingleFolderRenaming, CRenamingListFixture)
{
	// Prepare
	CFileListGenerator fileListGenerator;
	fileListGenerator.AddDirectory(_T("dir_before"), _T("dir_after"));

	// Rename
	if (!fileListGenerator.PerformRenaming())
		BOOST_FAIL(fileListGenerator.GetRenamingErrors().c_str());
}

BOOST_FIXTURE_TEST_CASE(CyclicFileRenaming, CRenamingListFixture)
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
		BOOST_FAIL(fileListGenerator.GetRenamingErrors().c_str());
}

BOOST_FIXTURE_TEST_CASE(CyclicFolderRenaming, CRenamingListFixture)
{
	// Prepare
	CFileListGenerator fileListGenerator;
	fileListGenerator.AddDirectory(_T("d"),		_T("a"));
	fileListGenerator.AddDirectory(_T("ccc"),	_T("d"));
	fileListGenerator.AddDirectory(_T("a"),		_T("bb"));
	fileListGenerator.AddDirectory(_T("bb"),	_T("ccc"));

	// Rename
	if (!fileListGenerator.PerformRenaming())
		BOOST_FAIL(fileListGenerator.GetRenamingErrors().c_str());
}

BOOST_FIXTURE_TEST_CASE(EmboxedFolderRenaming, CRenamingListFixture)
{
	// Prepare
	CFileListGenerator fileListGenerator;
	fileListGenerator.AddDirectory(_T("a\\b\\c\\d"),	_T("a\\b2\\c2\\d2"));
	fileListGenerator.AddDirectory(_T("a\\b"),			_T("a\\b2"));
	fileListGenerator.AddDirectory(_T("a\\b\\c"),		_T("a\\b2\\c2"));

	// Rename
	if (!fileListGenerator.PerformRenaming(false))
		BOOST_FAIL(fileListGenerator.GetRenamingErrors().c_str());
}

BOOST_FIXTURE_TEST_CASE(EmboxedFolderFilesRenaming, CRenamingListFixture)
{
	// Prepare
	CFileListGenerator fileListGenerator;
	fileListGenerator.AddFile(_T("a\\b\\c\\d\\file1"),	_T("a\\b2\\c2\\d2\\file1"));
	fileListGenerator.AddFile(_T("a\\b\\file2"),		_T("a\\b2\\file2"));
	fileListGenerator.AddFile(_T("a\\b\\c\\file3"),		_T("a\\b2\\c2\\file3"));

	// Rename
	if (!fileListGenerator.PerformRenaming(false))
		BOOST_FAIL(fileListGenerator.GetRenamingErrors().c_str());
}

BOOST_FIXTURE_TEST_CASE(ComplexFolderRenaming, CRenamingListFixture)
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
			BOOST_FAIL(fileListGenerator.GetRenamingErrors().c_str());
	}
}

BOOST_FIXTURE_TEST_CASE(ComplifiedFolderRenaming, CRenamingListFixture)
{
	// A simple renaming operation made complex.

	// Prepare
	CFileListGenerator fileListGenerator;
	fileListGenerator.AddDirectory(_T("a"), _T("b"));
	fileListGenerator.AddDirectory(_T("a/a"), _T("b/b"));
	fileListGenerator.AddDirectory(_T("a/a/a"), _T("b/b/b"));

	// Rename
	if (!fileListGenerator.PerformRenaming())
		BOOST_FAIL(fileListGenerator.GetRenamingErrors().c_str());
}

BOOST_FIXTURE_TEST_CASE(Complified2FolderRenaming, CRenamingListFixture)
{
	// Prepare
	CFileListGenerator fileListGenerator;
	fileListGenerator.AddDirectory(_T("a"), _T("b"));
	fileListGenerator.AddDirectory(_T("a\\b"), _T("b\\c"));
	fileListGenerator.CreateFile(_T("a\\file1"));
	fileListGenerator.CreateFile(_T("a\\b\\file2"));

	// Rename
	if (!fileListGenerator.PerformRenaming())
		BOOST_FAIL(fileListGenerator.GetRenamingErrors().c_str());

	// Check
	BOOST_CHECK(CPath::PathFileExists(fileListGenerator.GetTempDirectory() + _T("b\\file1")));
	BOOST_CHECK(CPath::PathFileExists(fileListGenerator.GetTempDirectory() + _T("b\\c\\file2")));
}

BOOST_FIXTURE_TEST_CASE(IdenticalParentFolderRenaming, CRenamingListFixture)
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
		BOOST_FAIL(fileListGenerator.GetRenamingErrors().c_str());

	// Check
	BOOST_CHECK(CPath::PathFileExists(fileListGenerator.GetTempDirectory() + _T("B\\file1")));
	BOOST_CHECK(CPath::PathFileExists(fileListGenerator.GetTempDirectory() + _T("A\\B\\file2")));
	BOOST_CHECK(CPath::PathFileExists(fileListGenerator.GetTempDirectory() + _T("A\\file3")));
}

BOOST_FIXTURE_TEST_CASE(NewFoldersCaseUnfication, CRenamingListFixture)
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
		BOOST_FAIL(fileListGenerator.GetRenamingErrors().c_str());

	// Check case
	CString strExpectFullUnifiedDirectoryPath = fileListGenerator.GetTempDirectory() + _T("A\\b\\c");
	CString strResultDirectoryPath = CPath::FindPathCase(strExpectFullUnifiedDirectoryPath);
	BOOST_CHECK_EQUAL(strExpectFullUnifiedDirectoryPath, strResultDirectoryPath);
}

BOOST_FIXTURE_TEST_CASE(ExistingFolderCaseUnification, CRenamingListFixture)
{
	// Prepare
	CFileListGenerator fileListGenerator;
	fileListGenerator.CreateDirectory(_T("dirA\\dirB"));
	fileListGenerator.AddFile(_T("dirA\\file1"), _T("DIRA\\file1"));
	fileListGenerator.AddFile(_T("dirC\\dirD\\file2"), _T("DIRA\\DIRb\\file2"));

	// Rename
	if (!fileListGenerator.PerformRenaming())
		BOOST_FAIL(fileListGenerator.GetRenamingErrors().c_str());

	// Check the case.
	CString strExpectFullUnifiedDirectoryPath = fileListGenerator.GetTempDirectory() + _T("DIRA\\DIRb");
	CString strResultDirectoryPath = CPath::FindPathCase(strExpectFullUnifiedDirectoryPath);
	BOOST_CHECK_EQUAL(strExpectFullUnifiedDirectoryPath, strResultDirectoryPath);
}

BOOST_FIXTURE_TEST_CASE(ExistingFolderCaseUnification2, CRenamingListFixture)
{
	// Prepare
	CFileListGenerator fileListGenerator;
	fileListGenerator.CreateDirectory(_T("dirA\\dirB"));
	fileListGenerator.AddDirectory(_T("dirA\\dir1"), _T("DIRA\\dir1"));
	fileListGenerator.AddDirectory(_T("dirC\\dirD\\dir2"), _T("DIRA\\DIRb\\dir2"));

	// Rename
	if (!fileListGenerator.PerformRenaming())
		BOOST_FAIL(fileListGenerator.GetRenamingErrors().c_str());

	// Check the case.
	CString strExpectFullUnifiedDirectoryPath = fileListGenerator.GetTempDirectory() + _T("DIRA\\DIRb");
	CString strResultDirectoryPath = CPath::FindPathCase(strExpectFullUnifiedDirectoryPath);
	BOOST_CHECK_EQUAL(strExpectFullUnifiedDirectoryPath, strResultDirectoryPath);
}

BOOST_FIXTURE_TEST_CASE(RemoveRenamedEmptyParentsFolders, CRenamingListFixture)
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
			BOOST_FAIL(fileListGenerator.GetRenamingErrors().c_str());

		// Check
		BOOST_CHECK(!CPath::PathFileExists(fileListGenerator.GetTempDirectory() + _T("a1\\b1\\c1")));
		BOOST_CHECK(!CPath::PathFileExists(fileListGenerator.GetTempDirectory() + _T("a1\\b1")));
		BOOST_CHECK(CPath::PathFileExists(fileListGenerator.GetTempDirectory() + _T("a1")));
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
			BOOST_FAIL(fileListGenerator.GetRenamingErrors().c_str());

		// Check
		BOOST_CHECK(!CPath::PathFileExists(fileListGenerator.GetTempDirectory() + _T("a1\\b1\\c1")));
		BOOST_CHECK(!CPath::PathFileExists(fileListGenerator.GetTempDirectory() + _T("a1\\b1")));
		BOOST_CHECK(CPath::PathFileExists(fileListGenerator.GetTempDirectory() + _T("a1")));
	}
}

BOOST_FIXTURE_TEST_CASE(FileCheckingError, CRenamingListFixture)
{
	testCheckingError(true);
}

BOOST_FIXTURE_TEST_CASE(FileCheckingWarning, CRenamingListFixture)
{
	testCheckingWarning(true);
}

BOOST_FIXTURE_TEST_CASE(DirectoryCheckingError, CRenamingListFixture)
{
	testCheckingError(false);
}

BOOST_FIXTURE_TEST_CASE(DirectoryCheckingWarning, CRenamingListFixture)
{
	testCheckingWarning(false);
}

BOOST_FIXTURE_TEST_CASE(CompressedFileRenaming, CRenamingListFixture)
{
	// Prepare
	CFileListGenerator fileListGenerator;
	fileListGenerator.AddFile(_T("file_before"), _T("file_after"), FILE_ATTRIBUTE_COMPRESSED);

	// Rename
	if (!fileListGenerator.PerformRenaming())
		BOOST_FAIL(fileListGenerator.GetRenamingErrors().c_str());
}

BOOST_FIXTURE_TEST_CASE(EncryptedFileRenaming, CRenamingListFixture)
{
	// Prepare
	CFileListGenerator fileListGenerator;
	fileListGenerator.AddFile(_T("file_before"), _T("file_after"), FILE_ATTRIBUTE_ENCRYPTED);

	// Rename
	if (!fileListGenerator.PerformRenaming())
		BOOST_FAIL(fileListGenerator.GetRenamingErrors().c_str());
}

BOOST_FIXTURE_TEST_CASE(RenameFileToAnotherDrive, CRenamingListFixture)
{
	// Rename a file from one drive to another.

	// Find a drive to do the test.
	CString strDestinationDrive = GetAnotherDriveRoot();
	if (strDestinationDrive.IsEmpty())
		BOOST_FAIL("No other drive could be found to make the test.");
	else
	{
		CString strDestinationDirectory = strDestinationDrive + _T("rename-it_unit_test.tmp");
		CString strDestinationPath = strDestinationDirectory + _T("\\after.tmp");

		// Prepare
		CFileListGenerator fileListGenerator;
		fileListGenerator.AddFile(_T("before"), _T("xxxx"));
		fileListGenerator.GetAfterFileList().SetPath(0, strDestinationPath);

		// Rename
		if (!fileListGenerator.PerformRenaming())
			BOOST_FAIL(fileListGenerator.GetRenamingErrors().c_str());

		// Delete the temporary files/folders.
		::DeleteFile(strDestinationPath);
		::RemoveDirectory(strDestinationDirectory);
	}
}

BOOST_FIXTURE_TEST_CASE(RenameDirectoryToAnotherDrive, CRenamingListFixture)
{
	// Renaming a directory from one drive to another should not be possible.

	// Find a drive to do the test.
	CString strDestinationDrive = GetAnotherDriveRoot();
	if (strDestinationDrive.IsEmpty())
		BOOST_FAIL("No other drive could be found to make the test.");
	else
	{
		CString strDestinationPath = strDestinationDrive + _T("after.tmp");

		// Prepare
		CFileListGenerator fileListGenerator;
		fileListGenerator.AddDirectory(_T("before"), _T("xxxx"));
		fileListGenerator.GetAfterFileList().SetPath(0, strDestinationPath);

		// Rename
		CRenamingList renamingList = fileListGenerator.MakeRenamingList();
		BOOST_CHECK(renamingList.Check() == false);
		BOOST_CHECK(renamingList.GetErrorCount() == 1);
	}
}

BOOST_FIXTURE_TEST_CASE(NetworkDriveFileRenaming, CRenamingListFixture)
{
	testNetworkDriveRenaming(true);
}

BOOST_FIXTURE_TEST_CASE(NetworkDriveDirectoryRenaming, CRenamingListFixture)
{
	testNetworkDriveRenaming(false);
}

BOOST_FIXTURE_TEST_CASE(SubstituteDriveFileRenaming, CRenamingListFixture)
{
	// Done by "subst X: C:\Folder\".

	// TODO
}

BOOST_AUTO_TEST_SUITE_END();
