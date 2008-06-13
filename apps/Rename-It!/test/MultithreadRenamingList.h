#include <cxxtest/TestSuite.h>
#include "IO/Renaming/MultithreadRenamingList.h"
#include "IO/KTM.h"
#include <iostream>

#ifdef _UNICODE
# define tcout wcout
#else
# define tcout cout
#endif

using namespace Beroux::IO;
using namespace Beroux::IO::Renaming;

// TODO: Add callback tests.
class MultithreadRenamingListTestSuite : public CxxTest::TestSuite 
{
public:
	CString m_strTempDir;
	CFileList m_vTestFilesBefore;
	HANDLE m_hOpenTestFile;
	shared_ptr<CRenamingList> m_simpleRenamingList;			// Should succeed.
	shared_ptr<CRenamingList> m_impossibleRenamingList;		// Should fail at checking.
	shared_ptr<CRenamingList> m_failingRenamingList;		// Should fail during renaming.
	unsigned m_nRenamed;
	unsigned m_nErrors;
	unsigned m_nProgress;

	MultithreadRenamingListTestSuite()
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
		m_vTestFilesBefore.AddFile(m_strTempDir + _T("a.tmp"));
		m_vTestFilesBefore.AddFile(m_strTempDir + _T("b.tmp"));
		m_vTestFilesBefore.AddFile(m_strTempDir + _T("c.tmp"));
	}

	~MultithreadRenamingListTestSuite()
	{
		::RemoveDirectory(m_strTempDir);
	}

	void setUp()
	{
		CString strCannotRenameFileName = m_strTempDir + _T("cannot_rename.tmp");

		// Create the temporary test files.
		for (int i=0; i<m_vTestFilesBefore.GetFileCount(); ++i)
		{
			::CloseHandle(::CreateFile(m_vTestFilesBefore.GetFile(i).GetPath(),
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
			after.AddFile(m_strTempDir + _T("a_after.tmp"));
			after.AddFile(m_strTempDir + _T("b_after.tmp"));
			after.AddFile(m_strTempDir + _T("c_after.tmp"));

			m_simpleRenamingList.reset( new CRenamingList(m_vTestFilesBefore, after) );
		}

		// Create the checking failure renaming list.
		{
			CFileList after;
			after.AddFile(m_strTempDir + _T("a_after.tmp"));
			after.AddFile(m_strTempDir + _T("b_after.tmp"));
			after.AddFile(m_strTempDir + _T("b_after.tmp"));

			m_impossibleRenamingList.reset( new CRenamingList(m_vTestFilesBefore, after) );
		}

		// Create the renaming failure renaming list.
		{
			CFileList before = m_vTestFilesBefore;
			before.AddFile(strCannotRenameFileName);

			CFileList after;
			after.AddFile(m_strTempDir + _T("a_after.tmp"));
			after.AddFile(m_strTempDir + _T("b_after.tmp"));
			after.AddFile(m_strTempDir + _T("c_after.tmp"));
			after.AddFile(m_strTempDir + _T("cannot_rename_after.tmp"));

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

	void testBasicFunction()
	{
		KTMTransaction ktm;
		CMultithreadRenamingList mrl;
		TS_ASSERT_EQUALS(CMultithreadRenamingList::resultNotStarted, mrl.GetRenamingResult());

		// Start renaming
		mrl.Start(*m_simpleRenamingList, ktm);
		TS_ASSERT(!mrl.IsDone());
		TS_ASSERT_EQUALS(CMultithreadRenamingList::resultInProgress, mrl.GetRenamingResult());

		// Wait until it's done.
		for (int i=0; !mrl.IsDone(); ++i)
		{
			// Wait
			Sleep(10);
			if (i > 300)
			{
				TS_ASSERT(false);
				break;
			}
		}
		TS_ASSERT(ktm.Commit());

		// Check success.
		TS_ASSERT_EQUALS(CMultithreadRenamingList::resultSuccess, mrl.GetRenamingResult());
		if (mrl.GetRenamingResult() == CMultithreadRenamingList::resultCheckingFailed)
			// Display the errors.
			PrintCheckingErrors(*m_simpleRenamingList);
	}

	void testCheckingFailure()
	{
		KTMTransaction ktm;
		CMultithreadRenamingList mrl;
		mrl.Start(*m_impossibleRenamingList, ktm);
		mrl.WaitForTerminaison();
		ktm.Commit();
		TS_ASSERT(mrl.IsDone());
		TS_ASSERT_EQUALS(CMultithreadRenamingList::resultCheckingFailed, mrl.GetRenamingResult());
	}

	void testRenamingFailure()
	{
		KTMTransaction ktm;
		CMultithreadRenamingList mrl;
		mrl.Start(*m_failingRenamingList, ktm);
		mrl.WaitForTerminaison();
		ktm.Commit();
		TS_ASSERT(mrl.IsDone());
		TS_ASSERT_EQUALS(CMultithreadRenamingList::resultRenamingFailed, mrl.GetRenamingResult());
		if (mrl.GetRenamingResult() == CMultithreadRenamingList::resultCheckingFailed)
			// Display the errors.
			PrintCheckingErrors(*m_failingRenamingList);
	}

	void testCallbacks()
	{
		// Register callbacks.
		m_nRenamed = 0;
		m_nErrors = 0;
		m_nProgress = 0;
		m_failingRenamingList->SetRenamedCallback(boost::bind(&MultithreadRenamingListTestSuite::OnRenamed, this, _1, _2));
		m_failingRenamingList->SetRenameErrorCallback(boost::bind(&MultithreadRenamingListTestSuite::OnRenameError, this, _1));
		m_failingRenamingList->SetProgressCallback(boost::bind(&MultithreadRenamingListTestSuite::OnProgress, this, _1, _2, _3));

		// Rename the files.
		KTMTransaction ktm;
		CMultithreadRenamingList mrl;
		mrl.Start(*m_failingRenamingList, ktm);
		mrl.WaitForTerminaison();
		ktm.Commit();
		if (mrl.GetRenamingResult() == CMultithreadRenamingList::resultCheckingFailed)
			// Display the errors.
			PrintCheckingErrors(*m_failingRenamingList);

		// Check callbacks.
		TS_ASSERT_LESS_THAN(m_nRenamed, m_failingRenamingList->GetCount());
		TS_ASSERT_DIFFERS(0, m_nErrors);
		TS_ASSERT_DIFFERS(0, m_nProgress);
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
		++m_nRenamed;
	}

	void OnRenameError(const Beroux::IO::Renaming::IRenameError& renameError)
	{
		++m_nErrors;
	}

	void OnProgress(Beroux::IO::Renaming::CRenamingList::EStage nStage, int nDone, int nTotal)
	{
		++m_nProgress;
	}
};
