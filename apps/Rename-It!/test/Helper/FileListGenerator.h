#pragma once
#include "IO/Renaming/FileList.h"

using namespace Beroux::IO::Renaming;

class CFileListGenerator
{
public:
	CFileListGenerator()
	{
		TCHAR szAllowed[] = _T("0123456789ABCDEF");
		unsigned nAllowed = sizeof(szAllowed)/sizeof(szAllowed[0]) - 1;

		// Get a new temporary folder.
		::GetTempPath(MAX_PATH, m_strTempDir.GetBuffer(MAX_PATH));
		m_strTempDir.ReleaseBuffer();

		m_strTempDir += _T("renameit_unit_test~");
		for (int i=0; i<5; ++i)
		{
			char ch = szAllowed[rand()%nAllowed];
			m_strTempDir += ch;
		}
		m_strTempDir += _T('\\');

		::CreateDirectory(m_strTempDir, NULL);
	}

	~CFileListGenerator()
	{
		CleanUp();
		::RemoveDirectory(m_strTempDir);
	}

	CFileList& GetBeforeFileList()
	{
		return m_flBefore;
	}

	CFileList& GetAfterFileList()
	{
		return m_flAfter;
	}

	void AddFile(const CString& fileNameBefore, const CString& fileNameAfter)
	{
		// Make full path.
		CString pathBefore = m_strTempDir + fileNameBefore;
		CString pathAfter = m_strTempDir + fileNameAfter;

		// Create the file name before.
		::CloseHandle(::CreateFile(pathBefore,
			GENERIC_WRITE,
			FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, CREATE_NEW, 
			FILE_ATTRIBUTE_TEMPORARY,
			NULL));

		// Add to the list.
		m_flBefore.AddFile(pathBefore);
		m_flAfter.AddFile(pathAfter);
	}

	CRenamingList MakeRenamingList()
	{
		return CRenamingList(m_flBefore, m_flAfter);
	}

	// Remove all files/folder in the list.
	// This does not remove the created files.
	void Clear()
	{
		for (int i=m_flBefore.GetFileCount() - 1; i>=0; --i)
		{
			m_flBefore.RemoveFile(i);
			m_flAfter.RemoveFile(i);
		}
	}

	// Delete all files and folders.
	void CleanUp()
	{
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

	// Returns true if all files/folders have have been renamed.
	bool AreAllRenamed() const
	{
		for (int i=0; i<m_flBefore.GetFileCount(); ++i)
		{
			// We don't check that the path before is removed
			// because it's not a must. Ex: Cyclic renaming.
			if (!CPath::PathFileExists(m_flAfter[i].GetPath()))
				return false;
		}
		return true;
	}

private:
	CString m_strTempDir;
	CFileList m_flBefore;
	CFileList m_flAfter;
};
