#pragma once
#include "IO/Renaming/FileList.h"
#include "IO/Renaming/RenamingList.h"
#include "IO/Renaming/RenamingError.h"
#include "IO/FailoverKtmTransaction.h"
#include "Helper/RandomMT.h"
#include <sstream>

using namespace Beroux::IO;
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

	CString GetTempFolder() const
	{
		return m_strTempDir;
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

		// Create the parent directories.
		CString strParentPath = CPath(pathBefore).GetPathRoot();
		BOOST_FOREACH(CString strDirectoryName, CPath(pathBefore).GetDirectories())
		{
			// Get the full parent directory's path.
			strParentPath += strDirectoryName + '\\';

			if (!CPath::PathFileExists(strParentPath))
				::CreateDirectory(strParentPath, NULL);
		}

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

		// Create the parent directories.
		CString strParentPath = CPath(pathBefore).GetPathRoot();
		BOOST_FOREACH(CString strDirectoryName, CPath(pathBefore).GetDirectories())
		{
			// Get the full parent directory's path.
			strParentPath += strDirectoryName + '\\';

			if (!CPath::PathFileExists(strParentPath))
				::CreateDirectory(strParentPath, NULL);
		}

		// Create the directory
		::CreateDirectory(pathBefore, NULL);

		// Add to the list.
		m_flBefore.AddFile(pathBefore);
		m_flAfter.AddFile(pathAfter);
	}

	CRenamingList MakeRenamingList() const
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

	void RandomizeOperationsOrder(unsigned long seed)
	{
		Beroux::Math::RandomMT random(seed);
		for (int i=0; i<m_flBefore.GetFileCount(); ++i)
		{
			unsigned nIndex = random.RandomRange(0, m_flBefore.GetFileCount() - 1);

			CPath pathBefore = m_flBefore.GetFile(nIndex);
			CPath pathAfter = m_flAfter.GetFile(nIndex);
			m_flBefore.RemoveFile(nIndex);
			m_flAfter.RemoveFile(nIndex);
			m_flBefore.AddFile(pathBefore);
			m_flAfter.AddFile(pathBefore);
		}
	}

	// Rename all the files and return true on success.
	bool PerformRenaming(bool bUseTransactions=true)
	{
		m_ossRenameErrors.reset(new ostringstream());
		CFailoverKtmTransaction ktm(NULL, 0, NULL, NULL, bUseTransactions);
		CRenamingList renamingList = MakeRenamingList();
		renamingList.SetRenameErrorCallback(bind(&CFileListGenerator::OnRenameError, this, _1));

		if (!renamingList.Check())
		{
			PrintCheckingErrors(renamingList);
			return false;
		}

		if (!renamingList.PerformRenaming(ktm))
		{
			ktm.Commit();
			return false;
		}

		if (!ktm.Commit())
			return false;

		if (!AreAllRenamed())
			return false;

		return true;
	}

	string GetRenamingErrors() const
	{
		return m_ossRenameErrors->str();
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
	void PrintCheckingErrors(const CRenamingList& renamingList)
	{
		if (renamingList.GetErrorCount() != 0 || renamingList.GetWarningCount() != 0)
		{
			*m_ossRenameErrors << "Checking failed (" << renamingList.GetErrorCount() << " errors, " << renamingList.GetWarningCount() << " warnings):" << endl;

			for (int i=0; i<renamingList.GetCount(); ++i)
				if (renamingList.GetOperationProblem(i).nErrorLevel != CRenamingList::levelNone)
					*m_ossRenameErrors 
					<< '`' << CStringToString(renamingList.GetRenamingOperation(i).GetPathBefore().GetFileName()) << '`'
					<< " --> "
					<< '`' << CStringToString(renamingList.GetRenamingOperation(i).GetPathAfter().GetFileName()) << '`'
					<< ": " << CStringToString(renamingList.GetOperationProblem(i).strMessage)
					<< endl;

			*m_ossRenameErrors << endl;
		}
	}

	void OnRenameError(const Beroux::IO::Renaming::IRenameError& renameError)
	{
		*m_ossRenameErrors << "OnRenameError(): ";
		if (typeid(renameError) == typeid(CRenamingError))
		{
			const CRenamingError& renErr = static_cast<const CRenamingError&>(renameError);
			*m_ossRenameErrors 
				<< '`' << CStringToString(renErr.GetPathNameBefore().GetPath()) << '`'
				<< " --> "
				<< '`' << CStringToString(renErr.GetPathNameAfter().GetPath()) << '`'
				<< ": " << CStringToString(renErr.GetErrorMessage());
		}
		else
			*m_ossRenameErrors << "Unknown error type.";
		*m_ossRenameErrors << endl;
	}

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
					DeleteAllInFolder(ff.GetFilePath() + '\\');
					::RemoveDirectory(ff.GetFilePath());
				}
				else
					::DeleteFile(ff.GetFilePath());
			}
		}
	}

	static string CStringToString(const CString& source)
	{
#ifdef _UNICODE
		size_t count = 0;
		char dest[256];
		wcstombs_s(&count, dest, (LPCWSTR)source, source.GetLength());
		dest[source.GetLength()] = '\0';
		return dest;
#else
		return (LPCSTR)source;
#endif
	}

	CString m_strTempDir;
	CFileList m_flBefore;
	CFileList m_flAfter;
	shared_ptr<ostringstream> m_ossRenameErrors;
};
