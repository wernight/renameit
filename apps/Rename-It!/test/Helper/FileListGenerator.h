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
	class CFileCreationException : public virtual boost::exception, public virtual std::exception
	{
	public:
		explicit CFileCreationException(const CString& filename);

		const CString& GetFilename() const;

		virtual const char* what() const;

	private:
		const CString& m_strFilename;
	};

	CFileListGenerator();

	~CFileListGenerator();

	CFileList& GetBeforeFileList();

	CFileList& GetAfterFileList();

	/// Temporary directory for this test, including an ending '\'.
	CString GetTempDirectory() const;

	/**
	 * Add a new file renaming operations.
	 * The file name before is created.
	 * Both file names provided should be relative to a temporary folder.
	 */
	void AddFile(const CString& strFileNameBefore, const CString& strFileNameAfter, DWORD dwFileAttributes = FILE_ATTRIBUTE_NORMAL);

	/**
	 * Add a new directory renaming operations.
	 * The directory name before is created.
	 * Both directory names provided should be relative to a temporary folder.
	 */
	void AddDirectory(const CString& strFolderNameBefore, const CString& strFolderNameAfter);

	/**
	 * Create a file path.
	 * The files will be removed when the CFileListGenerator is destroyed or cleaned.
	 */
	bool CreateFile(const CString& strFileName, DWORD dwFileAttributes = FILE_ATTRIBUTE_NORMAL);

	/**
	 * Create a folder path.
	 * The folders will be removed when the CFileListGenerator is destroyed or cleaned.
	 */
	void CreateDirectory(const CString& strFolderName);

	CRenamingList MakeRenamingList() const;

	/**
	 * Remove all files/folder in the list.
	 * This does not remove the created files.
	 */
	void Clear();

	/// Delete all files and folders.
	void CleanUp();

	void RandomizeOperationsOrder(unsigned long seed);

	/// Rename all the files and return true on success.
	bool PerformRenaming(bool bUseTransactions=true);
	
	bool HasRenamingErrors() const;

	string GetRenamingErrors() const;

	/// Returns true if all files/folders have have been renamed.
	bool AreAllRenamed() const;

private:
	void PrintCheckingErrors(const CRenamingList& renamingList);

	void OnIOOperationPerformed(const CRenamingList& sender, const CIOOperation& ioOperation, CIOOperation::EErrorLevel nErrorLevel);

	static void DeleteAllInFolder(const CString& strDir);
	
	static string CStringToString(const CString& source);

	CString m_strTempDir;
	CFileList m_flBefore;
	CFileList m_flAfter;
	ostringstream m_ossRenameErrors;
	bool m_bLastRenamingSuccessful;
};
