#include <cxxtest/TestSuite.h>
#include "IO/Renaming/MultithreadRenamingList.h"
#include "../resource.h"

using namespace Beroux::IO::Renaming;

class MultithreadRenamingListTestSuite : public CxxTest::TestSuite 
{
public:
	CFileList m_vTestFilesBefore;
	vector<HANDLE> m_vTestFilesOpen;
	shared_ptr<CRenamingList> m_simpleRenamingList;			// Should succeed.
	shared_ptr<CRenamingList> m_impossibleRenamingList;		// Should fail at checking.
	shared_ptr<CRenamingList> m_failingRenamingList;		// Should fail during renaming.

	MultithreadRenamingListTestSuite()
	{
		m_vTestFilesBefore.AddFile(_T("a.tmp"));
		m_vTestFilesBefore.AddFile(_T("b.tmp"));
		m_vTestFilesBefore.AddFile(_T("c.tmp"));
	}

	void setUp()
	{
		LPCTSTR szCannotRenameFileName = _T("cannot_rename.tmp");

		// Create the temporary test files.
		for (int i=0; i<m_vTestFilesBefore.GetFileCount(); ++i)
			m_vTestFilesOpen.push_back(::CreateFile(m_vTestFilesBefore.GetFile(i).GetPath(), GENERIC_WRITE, FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE, NULL));
		m_vTestFilesOpen.push_back(::CreateFile(szCannotRenameFileName, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE, NULL));

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
			after.AddFile(m_vTestFilesBefore[0]);
			after.AddFile(m_vTestFilesBefore[0]);
			after.AddFile(m_vTestFilesBefore[0]);
			m_impossibleRenamingList.reset( new CRenamingList(m_vTestFilesBefore, after) );
		}

		// Create the renaming failure renaming list.
		{
			m_vTestFilesBefore.AddFile(szCannotRenameFileName);
			CFileList after;
			after.AddFile(_T("a_after.tmp"));
			after.AddFile(_T("b_after.tmp"));
			after.AddFile(_T("c_after.tmp"));
			after.AddFile(_T("cannot_rename_after.tmp"));
			m_failingRenamingList.reset( new CRenamingList(m_vTestFilesBefore, after) );
		}
	}

	void tearDown()
	{
		// Close and delete the test files.
		BOOST_FOREACH(HANDLE hFile, m_vTestFilesOpen)
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
