#include "StdAfx.h"
#include "FileListGenerator.h"

CFileListGenerator::CFileCreationException::CFileCreationException(const CString& filename)
: std::exception("Failed to create the file")
, m_strFilename(filename)
{
}

const CString& CFileListGenerator::CFileCreationException::GetFilename() const
{
	return m_strFilename;
}
		
const char* CFileListGenerator::CFileCreationException::what() const
{
	static string what = (boost::format("Failed to add the file `%1%`") % m_strFilename.GetString()).str();
	return what.c_str();
}



CFileListGenerator::CFileListGenerator()
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

CFileListGenerator::~CFileListGenerator()
{
	CleanUp();
	::RemoveDirectory(m_strTempDir);
}

CFileList& CFileListGenerator::GetBeforeFileList()
{
	return m_flBefore;
}

CFileList& CFileListGenerator::GetAfterFileList()
{
	return m_flAfter;
}

CString CFileListGenerator::GetTempDirectory() const
{
	return m_strTempDir;
}

void CFileListGenerator::AddFile(const CString& strFileNameBefore, const CString& strFileNameAfter, DWORD dwFileAttributes)
{
	// Make full path.
	CString strPathBefore = m_strTempDir + strFileNameBefore;
	CString strPathAfter = m_strTempDir + strFileNameAfter;

	// Create the file path before.
	if (!CreateFile(strFileNameBefore, dwFileAttributes))
		BOOST_THROW_EXCEPTION(CFileCreationException(strFileNameBefore));

	// Add to the list.
	m_flBefore.AddPath(strPathBefore);
	m_flAfter.AddPath(strPathAfter);
}

void CFileListGenerator::AddDirectory(const CString& strFolderNameBefore, const CString& strFolderNameAfter)
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

bool CFileListGenerator::CreateFile(const CString& strFileName, DWORD dwFileAttributes)
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

void CFileListGenerator::CreateDirectory(const CString& strFolderName)
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

CRenamingList CFileListGenerator::MakeRenamingList() const
{
	return CRenamingList(m_flBefore, m_flAfter);
}

void CFileListGenerator::Clear()
{
	for (int i=m_flBefore.GetCount() - 1; i>=0; --i)
	{
		m_flBefore.RemovePath(i);
		m_flAfter.RemovePath(i);
	}
}

void CFileListGenerator::CleanUp()
{
	DeleteAllInFolder(m_strTempDir);
}

void CFileListGenerator::RandomizeOperationsOrder(unsigned long seed)
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

bool CFileListGenerator::PerformRenaming(bool bUseTransactions)
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

bool CFileListGenerator::HasRenamingErrors() const
{
	return !m_bLastRenamingSuccessful;
}

string CFileListGenerator::GetRenamingErrors() const
{
	return string("\n") + m_ossRenameErrors.str();
}

bool CFileListGenerator::AreAllRenamed() const
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

void CFileListGenerator::PrintCheckingErrors(const CRenamingList& renamingList)
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

void CFileListGenerator::OnIOOperationPerformed(const CRenamingList& sender, const CIOOperation& ioOperation, CIOOperation::EErrorLevel nErrorLevel)
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

void CFileListGenerator::DeleteAllInFolder(const CString& strDir)
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

string CFileListGenerator::CStringToString(const CString& source)
{
	size_t count = 0;
	char dest[256];
	wcstombs_s(&count, dest, (LPCWSTR)source, source.GetLength());
	dest[source.GetLength()] = '\0';
	return dest;
}
