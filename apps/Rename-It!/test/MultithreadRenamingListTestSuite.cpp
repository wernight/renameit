#include "StdAfx.h"
#include <boost/test/unit_test.hpp>
#include "IO/FailoverKtmTransaction.h"
#include "IO/Renaming/IOOperation/RenameOperation.h"
#include "IO/Renaming/MultithreadRenamingList.h"
#include <iostream>

using namespace Beroux::IO;
using namespace Beroux::IO::Renaming;

BOOST_AUTO_TEST_SUITE(MultithreadRenamingListTestSuite);

class CMultithreadRenamingListFixture
{
public:
	CString m_strTempDir;
	CFileList m_vTestFilesBefore;
	HANDLE m_hOpenTestFile;
	shared_ptr<CRenamingList> m_simpleRenamingList;			// Should succeed.
	shared_ptr<CRenamingList> m_impossibleRenamingList;		// Should fail at checking.
	shared_ptr<CRenamingList> m_failingRenamingList;		// Should fail during renaming.
	unsigned m_nOnRenamed;
	unsigned m_nOnErrors;
	unsigned m_nOnProgress;
	unsigned m_nOnDone;

	CMultithreadRenamingListFixture()
	{
		// Set the instance to that resources can be loaded (i.e., for TString::LoadString() to work).
		HINSTANCE hInstance = (HINSTANCE) GetModuleHandle(NULL);
		AfxSetResourceHandle(hInstance);

		// Get the temporary folder.
		::GetTempPath(MAX_PATH, m_strTempDir.GetBuffer(MAX_PATH));
		m_strTempDir.ReleaseBuffer();
		m_strTempDir += "RiT_MRL_UnitTest\\";
		::CreateDirectory(m_strTempDir, NULL);

		// Create basic file list.
		m_vTestFilesBefore.AddPath(m_strTempDir + _T("a.tmp"));
		m_vTestFilesBefore.AddPath(m_strTempDir + _T("b.tmp"));
		m_vTestFilesBefore.AddPath(m_strTempDir + _T("c.tmp"));

		setUp();
	}

	~CMultithreadRenamingListFixture()
	{
		tearDown();

		::RemoveDirectory(m_strTempDir);
	}

	void setUp()
	{
		CString strCannotRenameFileName = m_strTempDir + _T("cannot_rename.tmp");

		// Create the temporary test files.
		for (int i=0; i<m_vTestFilesBefore.GetCount(); ++i)
		{
			::CloseHandle(::CreateFile(m_vTestFilesBefore.GetPath(i).GetPath(),
				GENERIC_WRITE,
				FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL, CREATE_NEW, 
				FILE_ATTRIBUTE_TEMPORARY,
				NULL));
		}

		// And one more that we keep open.
		m_hOpenTestFile = ::CreateFile(strCannotRenameFileName,
			GENERIC_WRITE,
			0,
			NULL,
			CREATE_NEW,
			FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
			NULL);

		// Create the simple renaming list.
		{
			CFileList after;
			after.AddPath(m_strTempDir + _T("a_after.tmp"));
			after.AddPath(m_strTempDir + _T("b_after.tmp"));
			after.AddPath(m_strTempDir + _T("c_after.tmp"));

			m_simpleRenamingList.reset( new CRenamingList(m_vTestFilesBefore, after) );
		}

		// Create the checking failure renaming list.
		{
			CFileList after;
			after.AddPath(m_strTempDir + _T("a_after.tmp"));
			after.AddPath(m_strTempDir + _T("b_after.tmp"));
			after.AddPath(m_strTempDir + _T("b_after.tmp"));

			m_impossibleRenamingList.reset( new CRenamingList(m_vTestFilesBefore, after) );
		}

		// Create the renaming failure renaming list.
		{
			CFileList before = m_vTestFilesBefore;
			before.AddPath(strCannotRenameFileName);

			CFileList after;
			after.AddPath(m_strTempDir + _T("a_after.tmp"));
			after.AddPath(m_strTempDir + _T("b_after.tmp"));
			after.AddPath(m_strTempDir + _T("c_after.tmp"));
			after.AddPath(m_strTempDir + _T("cannot_rename_after.tmp"));

			m_failingRenamingList.reset( new CRenamingList(before, after) );
		}
	}

	void tearDown()
	{
		// Close the open file.
		::CloseHandle(m_hOpenTestFile);

		// Remove all test files.
		CFileFind ff;
		BOOL bHasMore = ff.FindFile(m_strTempDir + _T("*.*"));
		while (bHasMore)
		{
			bHasMore = ff.FindNextFile();

			if (!ff.IsDirectory())
				::DeleteFile(ff.GetFilePath());
		}
	}
};

BOOST_FIXTURE_TEST_CASE(BasicFunction, CMultithreadRenamingListFixture)
{
	CFailoverKtmTransaction ktm;
	CMultithreadRenamingList mrl;
	BOOST_CHECK_EQUAL(CMultithreadRenamingList::resultNotStarted, mrl.GetRenamingResult());

	// Start renaming
	mrl.Start(*m_simpleRenamingList, ktm);
	BOOST_CHECK(!mrl.IsDone());
	BOOST_CHECK_EQUAL(CMultithreadRenamingList::resultInProgress, mrl.GetRenamingResult());

	// Wait until it's done.
	for (int i=0; !mrl.IsDone(); ++i)
	{
		// Wait
		Sleep(10);
		if (i > 300)
		{
			BOOST_CHECK(false);
			break;
		}
	}
	BOOST_CHECK(ktm.Commit());

	// Check success.
	BOOST_CHECK_EQUAL(CMultithreadRenamingList::resultSuccess, mrl.GetRenamingResult());
	if (mrl.GetRenamingResult() == CMultithreadRenamingList::resultCheckingFailed)
		// Display the errors.
		PrintCheckingErrors(*m_simpleRenamingList);
}

BOOST_FIXTURE_TEST_CASE(CheckingFailure, CMultithreadRenamingListFixture)
{
	CFailoverKtmTransaction ktm;
	CMultithreadRenamingList mrl;
	mrl.Start(*m_impossibleRenamingList, ktm);
	mrl.WaitForTerminaison();
	ktm.Commit();
	BOOST_CHECK(mrl.IsDone());
	BOOST_CHECK_EQUAL(CMultithreadRenamingList::resultCheckingFailed, mrl.GetRenamingResult());
}

BOOST_FIXTURE_TEST_CASE(RenamingFailure, CMultithreadRenamingListFixture)
{
	CFailoverKtmTransaction ktm;
	CMultithreadRenamingList mrl;
	mrl.Start(*m_failingRenamingList, ktm);
	mrl.WaitForTerminaison();
	ktm.Commit();
	BOOST_CHECK(mrl.IsDone());
	BOOST_CHECK_EQUAL(CMultithreadRenamingList::resultRenamingFailed, mrl.GetRenamingResult());
	if (mrl.GetRenamingResult() == CMultithreadRenamingList::resultCheckingFailed)
		// Display the errors.
		PrintCheckingErrors(*m_failingRenamingList);
}

BOOST_FIXTURE_TEST_CASE(Callbacks, CMultithreadRenamingListFixture)
{
	CFailoverKtmTransaction ktm;
	CMultithreadRenamingList mrl;

	// Register callbacks.
	m_nOnRenamed = 0;
	m_nOnErrors = 0;
	m_nOnProgress = 0;
	m_nOnDone = 0;
	m_failingRenamingList->IOOperationPerformed.connect( bind(&MultithreadRenamingListTestSuite::OnIOOperationPerformed, this, _1, _2, _3) );
	m_failingRenamingList->ProgressChanged.connect( bind(&MultithreadRenamingListTestSuite::OnProgressChanged, this, _1, _2, _3, _4) );
	mrl.Done.connect( bind(&MultithreadRenamingListTestSuite::OnDone, this, _1) );

	// Rename the files.
	mrl.Start(*m_failingRenamingList, ktm);
	mrl.WaitForTerminaison();
	ktm.Commit();
	if (mrl.GetRenamingResult() == CMultithreadRenamingList::resultCheckingFailed)
		// Display the errors.
		PrintCheckingErrors(*m_failingRenamingList);

	// Check callbacks.
	TS_ASSERT_LESS_THAN(m_nOnRenamed, m_failingRenamingList->GetCount());
	TS_ASSERT_DIFFERS(0, m_nOnErrors);
	TS_ASSERT_DIFFERS(0, m_nOnProgress);
	BOOST_CHECK_EQUAL(1, m_nOnDone);
}

BOOST_FIXTURE_TEST_CASE(MultipleRenaming, CMultithreadRenamingListFixture)
{
	// Same mrl.
	CMultithreadRenamingList mrl;

	for (int i=0; i<3; ++i)
	{
		CFailoverKtmTransaction ktm;
		mrl.Start(*m_simpleRenamingList, ktm);
		mrl.WaitForTerminaison();
		BOOST_CHECK_EQUAL(CMultithreadRenamingList::resultSuccess, mrl.GetRenamingResult());
		BOOST_CHECK(ktm.RollBack());
	}

	// Use the same KTM also.
	{
		CFailoverKtmTransaction ktm;
	
		mrl.Start(*m_simpleRenamingList, ktm);
		mrl.WaitForTerminaison();
		BOOST_CHECK_EQUAL(CMultithreadRenamingList::resultSuccess, mrl.GetRenamingResult());

		mrl.Start(*m_simpleRenamingList, ktm);
		mrl.WaitForTerminaison();
		TS_ASSERT_DIFFERS(CMultithreadRenamingList::resultSuccess, mrl.GetRenamingResult());

		BOOST_CHECK(ktm.Commit());
	}
}

BOOST_FIXTURE_TEST_CASE(Cancel, CMultithreadRenamingListFixture)
{
	CFailoverKtmTransaction ktm;
	CMultithreadRenamingList mrl;

	// Start renaming
	mrl.Start(*m_simpleRenamingList, ktm);
	mrl.Cancel();
	BOOST_CHECK(!mrl.IsDone());

	// Wait until it's done.
	mrl.WaitForTerminaison();
	BOOST_CHECK(ktm.RollBack());

	// Check success.
	BOOST_CHECK_EQUAL(CMultithreadRenamingList::resultCancelled, mrl.GetRenamingResult());
}

BOOST_AUTO_TEST_SUITE_END();
