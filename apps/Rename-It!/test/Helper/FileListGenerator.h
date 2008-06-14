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
			m_strTempDir += szAllowed[rand()%nAllowed];
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

	/**
	 * Add a new file renaming operations.
	 * The file name before is created.
	 * Both file names provided should be relative to a temporary folder.
	 */
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

	/**
	 * Add a new directory renaming operations.
	 * The directory name before is created.
	 * Both directory names provided should be relative to a temporary folder.
	 */
	void AddFolder(const CString& folderNameBefore, const CString& folderNameAfter)
	{
		// Make full path.
		CString pathBefore = m_strTempDir + folderNameBefore;
		CString pathAfter = m_strTempDir + folderNameAfter;

		// Create the directory name before (including missing parent folders).
		CString strParentPath = CPath(pathBefore).GetPathRoot();
		BOOST_FOREACH(CString strDirectoryName, CPath(pathBefore).GetDirectories())
		{
			// Get the full parent directory's path.
			strParentPath += strDirectoryName;

			if (!CPath::PathFileExists(strParentPath))
				::CreateDirectory(strParentPath, NULL);
		}

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
		DeleteAllInFolder(m_strTempDir);
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
	static void DeleteAllInFolder(const CString& strDir)
	{
		ASSERT(strDir[strDir.GetLength() - 1] == '\\');

		// Remove all test files.
		CFileFind ff;
		BOOL bHasMore = ff.FindFile(strDir + _T("*.*"));
		while (bHasMore)
		{
			bHasMore = ff.FindNextFile();

			if (ff.GetFileName() != "." && ff.GetFileName() != "..")
			{
				if (ff.IsDirectory())
				{
					DeleteAllInFolder(ff.GetFilePath());
					::RemoveDirectory(ff.GetFilePath());
				}
				else
					::DeleteFile(ff.GetFilePath());
			}
		}
	}

	CString m_strTempDir;
	CFileList m_flBefore;
	CFileList m_flAfter;
};
