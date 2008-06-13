#include <cxxtest/TestSuite.h>
#include "IO/Renaming/MultithreadRenamingList.h"

using namespace Beroux::IO::Renaming;

// TODO: Add callback tests.
class MultithreadRenamingListTestSuite : public CxxTest::TestSuite 
{
public:
	CFileList m_vTestFilesBefore;
	set<HANDLE> m_testFilesOpen;
	shared_ptr<CRenamingList> m_simpleRenamingList;			// Should succeed.
	shared_ptr<CRenamingList> m_impossibleRenamingList;		// Should fail at checking.
	shared_ptr<CRenamingList> m_failingRenamingList;		// Should fail during renaming.

	MultithreadRenamingListTestSuite()
	{
		HINSTANCE hInstance = (HINSTANCE) GetModuleHandle(NULL);
		AfxSetResourceHandle(hInstance);

		m_vTestFilesBefore.AddFile(_T("a.tmp"));
		m_vTestFilesBefore.AddFile(_T("b.tmp"));
		m_vTestFilesBefore.AddFile(_T("c.tmp"));
	}

	void setUp()
	{
		LPCTSTR szCannotRenameFileName = _T("cannot_rename.tmp");

		// Create the temporary test files.
		for (int i=0; i<m_vTestFilesBefore.GetFileCount(); ++i)
			m_testFilesOpen.insert(::CreateFile(m_vTestFilesBefore.GetFile(i).GetPath(), GENERIC_WRITE, FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE, NULL));
		m_testFilesOpen.insert(::CreateFile(szCannotRenameFileName, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE, NULL));

		// Create the simple renaming list.
		{
			CFileList after;
			after.AddFile(_T("a_after.tmp"));
			after.AddFile(_T("b_after.tmp"));
			after.AddFile(_T("c_after.tmp"));

			m_simpleRenamingList.reset( new CRenamingList(m_vTestFilesBefore, after) );
		}

		// Create the checking failure renaming list.
		{
			CFileList after;
			after.AddFile(_T("a_after.tmp"));
			after.AddFile(_T("b_after.tmp"));
			after.AddFile(_T("b_after.tmp"));

			m_impossibleRenamingList.reset( new CRenamingList(m_vTestFilesBefore, after) );
		}

		// Create the renaming failure renaming list.
		{
			CFileList before = m_vTestFilesBefore;
			before.AddFile(szCannotRenameFileName);

			CFileList after;
			after.AddFile(_T("a_after.tmp"));
			after.AddFile(_T("b_after.tmp"));
			after.AddFile(_T("c_after.tmp"));
			after.AddFile(_T("cannot_rename_after.tmp"));

			m_failingRenamingList.reset( new CRenamingList(before, after) );
		}
	}

	void tearDown()
	{
		// Close and delete the test files.
		BOOST_FOREACH(HANDLE hFile, m_testFilesOpen)
			::CloseHandle(hFile);
	}

	void testBasicFunction()
	{
		CMultithreadRenamingList mrl;
		TS_ASSERT_EQUALS(CMultithreadRenamingList::resultNotStarted, mrl.GetRenamingResult());

		mrl.Start(*m_simpleRenamingList);
		TS_ASSERT(!mrl.IsDone());
		TS_ASSERT_EQUALS(CMultithreadRenamingList::resultInProgress, mrl.GetRenamingResult());

		while (!mrl.IsDone())
			Sleep(10); // Wait
		TS_ASSERT_EQUALS(CMultithreadRenamingList::resultSuccess, mrl.GetRenamingResult());
	}

	void testCheckingFailure()
	{
		CMultithreadRenamingList mrl;
		mrl.Start(*m_impossibleRenamingList);
		mrl.WaitForTerminaison();
		TS_ASSERT(mrl.IsDone());
		TS_ASSERT_EQUALS(CMultithreadRenamingList::resultCheckingFailed, mrl.GetRenamingResult());
	}

	void testRenamingFailure()
	{
		CMultithreadRenamingList mrl;
		mrl.Start(*m_failingRenamingList);
		mrl.WaitForTerminaison();
		TS_ASSERT(mrl.IsDone());
		TS_ASSERT_EQUALS(CMultithreadRenamingList::resultRenamingFailed, mrl.GetRenamingResult());
	}
};
