#pragma once
#include "IO/Renaming/FileList.h"
#include "IO/Renaming/RenamingList.h"
#include "IO/Renaming/IOOperation/IOOperation.h"
#include "IO/Renaming/IOOperation/CreateDirectoryOperation.h"
#include "IO/Renaming/IOOperation/RemoveEmptyDirectoryOperation.h"
#include "IO/Renaming/IOOperation/RenameOperation.h"
#include "IO/FailoverKtmTransaction.h"
#include "Math/RandomMT.h"
#include <sstream>

using namespace Beroux::IO;
using namespace Beroux::IO::Renaming;
using namespace Beroux::IO::Renaming::IOOperation;

class CFileListGenerator
{
public:
	CFileListGenerator()
	: m_bLastRenamingSuccessful(false)
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

	/// Temporary directory for this test, including an ending '\'.
	CString GetTempDirectory() const
	{
		return m_strTempDir;
	}

	/**
	 * Add a new file renaming operations.
	 * The file name before is created.
	 * Both file names provided should be relative to a temporary folder.
	 */
	bool AddFile(const CString& strFileNameBefore, const CString& strFileNameAfter, DWORD dwFileAttributes = FILE_ATTRIBUTE_NORMAL)
	{
		// Make full path.
		CString strPathBefore = m_strTempDir + strFileNameBefore;
		CString strPathAfter = m_strTempDir + strFileNameAfter;

		// Create the file path before.
		if (!CreateFile(strFileNameBefore, dwFileAttributes))
			return false;

		// Add to the list.
		m_flBefore.AddPath(strPathBefore);
		m_flAfter.AddPath(strPathAfter);
		return true;
	}

	/**
	 * Add a new directory renaming operations.
	 * The directory name before is created.
	 * Both directory names provided should be relative to a temporary folder.
	 */
	void AddDirectory(const CString& strFolderNameBefore, const CString& strFolderNameAfter)
	{
		// Make full path.
		CString pathBefore = m_strTempDir + strFolderNameBefore;
		CString pathAfter = m_strTempDir + strFolderNameAfter;

		// Create the directory
		CreateDirectory(strFolderNameBefore);

		// Add to the list.
		m_flBefore.AddPath(pathBefore);
		m_flAfter.AddPath(pathAfter);
	}

	/**
	 * Create a file path.
	 * The files will be removed when the CFileListGenerator is destroyed or cleaned.
	 */
	bool CreateFile(const CString& strFileName, DWORD dwFileAttributes = FILE_ATTRIBUTE_NORMAL)
	{
		// Make full path.
		CString strFilePath = m_strTempDir + strFileName;
		CPath pathFilePath(strFilePath);

		// Create the parent directories.
		CreateDirectory(pathFilePath.GetDirectoryName().Mid(m_strTempDir.GetLength()));

		// Create the file name before.
		HANDLE hFile = ::CreateFile(strFilePath,
			GENERIC_WRITE,
			FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, CREATE_ALWAYS, 
			dwFileAttributes,
			NULL);

		if (hFile == INVALID_HANDLE_VALUE)
			return false;

		::CloseHandle(hFile);
		return true;
	}

	/**
	 * Create a folder path.
	 * The folders will be removed when the CFileListGenerator is destroyed or cleaned.
	 */
	void CreateDirectory(const CString& strFolderName)
	{
		if (strFolderName.IsEmpty())
			return;

		// Make full path.
		CString strFolderPath = m_strTempDir + strFolderName;
		CPath pathFolderPath = strFolderPath;

		// Create the parent directories.
		CString strParentPath = pathFolderPath.GetPathRoot();
		BOOST_FOREACH(CString strDirectoryName, pathFolderPath.GetDirectories())
		{
			// Get the full parent directory's path.
			strParentPath += strDirectoryName + '\\';

			if (!CPath::PathFileExists(strParentPath))
				::CreateDirectory(strParentPath, NULL);
		}

		// Create the directory
		::CreateDirectory(strFolderPath, NULL);
	}

	CRenamingList MakeRenamingList() const
	{
		return CRenamingList(m_flBefore, m_flAfter);
	}

	/**
	 * Remove all files/folder in the list.
	 * This does not remove the created files.
	 */
	void Clear()
	{
		for (int i=m_flBefore.GetCount() - 1; i>=0; --i)
		{
			m_flBefore.RemovePath(i);
			m_flAfter.RemovePath(i);
		}
	}

	/// Delete all files and folders.
	void CleanUp()
	{
		DeleteAllInFolder(m_strTempDir);
	}

	void RandomizeOperationsOrder(unsigned long seed)
	{
		Beroux::Math::CRandomMT random(seed);
		for (int i=0; i<m_flBefore.GetCount(); ++i)
		{
			unsigned nIndex = random.RandomRange(0, m_flBefore.GetCount() - 1);

			CPath pathBefore = m_flBefore.GetPath(nIndex);
			CPath pathAfter = m_flAfter.GetPath(nIndex);
			m_flBefore.RemovePath(nIndex);
			m_flAfter.RemovePath(nIndex);
			m_flBefore.AddPath(pathBefore);
			m_flAfter.AddPath(pathAfter);
		}
	}

	/// Rename all the files and return true on success.
	bool PerformRenaming(bool bUseTransactions=true)
	{
		m_bLastRenamingSuccessful = false;

		m_ossRenameErrors.str("");
		CFailoverKtmTransaction ktm(NULL, 0, NULL, NULL, bUseTransactions);
		CRenamingList renamingList = MakeRenamingList();
		renamingList.IOOperationPerformed.connect( bind(&CFileListGenerator::OnIOOperationPerformed, this, _1, _2, _3) );

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

		m_bLastRenamingSuccessful = true;
		return true;
	}

	bool HasRenamingErrors() const
	{
		return !m_bLastRenamingSuccessful;
	}

	string GetRenamingErrors() const
	{
		return string("\n") + m_ossRenameErrors.str();
	}

	/// Returns true if all files/folders have have been renamed.
	bool AreAllRenamed() const
	{
		for (int i=0; i<m_flBefore.GetCount(); ++i)
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
			m_ossRenameErrors << "Checking failed (" << renamingList.GetErrorCount() << " errors, " << renamingList.GetWarningCount() << " warnings):" << endl;

			for (int i=0; i<renamingList.GetCount(); ++i)
				if (renamingList.GetOperationProblem(i).nErrorLevel != CRenamingList::levelNone)
					m_ossRenameErrors 
					<< '`' << CStringToString(renamingList.GetRenamingOperation(i).GetPathBefore().GetFileName()) << '`'
					<< " --> "
					<< '`' << CStringToString(renamingList.GetRenamingOperation(i).GetPathAfter().GetFileName()) << '`'
					<< ": " << CStringToString(renamingList.GetOperationProblem(i).strMessage)
					<< endl;

			m_ossRenameErrors << endl;
		}
	}

	void OnIOOperationPerformed(const CRenamingList& sender, const CIOOperation& ioOperation, CIOOperation::EErrorLevel nErrorLevel)
	{
		if (nErrorLevel != CIOOperation::elSuccess)
		{
			m_ossRenameErrors << "- OnIOOperationPerformed(): ";

			if (nErrorLevel == CIOOperation::elWarning)
				m_ossRenameErrors << "WARNING: ";
			else
				m_ossRenameErrors << "ERROR: ";

			if (typeid(ioOperation) == typeid(CRenameOperation))
			{
				const CRenameOperation& renErr = static_cast<const CRenameOperation&>(ioOperation);
				m_ossRenameErrors 
					<< "CRenameOperation failed: "
					<< '"' << CStringToString(renErr.GetPathBefore().GetPath()) << '"'
					<< " --> "
					<< '"' << CStringToString(renErr.GetPathAfter().GetPath()) << '"'
					<< ": " << CStringToString(renErr.GetErrorMessage());
			}
			else if (typeid(ioOperation) == typeid(CCreateDirectoryOperation))
			{
				const CCreateDirectoryOperation& createDirOp = static_cast<const CCreateDirectoryOperation&>(ioOperation);
				m_ossRenameErrors 
					<< "CCreateDirectoryOperation failed: "
					<< '"' << CStringToString(createDirOp.GetDirectoryPath().GetPath()) << '"'
					<< ": " << CStringToString(createDirOp.GetErrorMessage());
			}
			else if (typeid(ioOperation) == typeid(CRemoveEmptyDirectoryOperation))
			{
				const CRemoveEmptyDirectoryOperation& delDirOp = static_cast<const CRemoveEmptyDirectoryOperation&>(ioOperation);
				m_ossRenameErrors 
					<< "CRemoveEmptyDirectoryOperation failed: "
					<< '"' << CStringToString(delDirOp.GetDirectoryPath()) << '"'
					<< ": " << CStringToString(delDirOp.GetErrorMessage());
			}
			else
				m_ossRenameErrors << "Unknown error type.";
			m_ossRenameErrors << endl;
		}
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
	ostringstream m_ossRenameErrors;
	bool m_bLastRenamingSuccessful;
};
