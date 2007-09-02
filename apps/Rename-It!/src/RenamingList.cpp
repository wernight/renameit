#include "StdAfx.h"
#include "RenamingList.h"
#include "OrientedGraph.h"
#include <math.h>
#include "../resource.h"
#include "KTM.h"

CRenamingList::CRenamingList(void)
	: m_nWarnings(0)
	, m_nErrors(0)
{
	m_fOnProgress = boost::bind(&CRenamingList::DefaultProgressCallback, this, _1, _2, _3);
}

CRenamingList::CRenamingList(const CFileList& flBefore, const CFileList& flAfter)
	: m_nWarnings(0)
	, m_nErrors(0)
{
	Create(flBefore, flAfter);
}

CRenamingList::~CRenamingList(void)
{
}

void CRenamingList::Create(const CFileList& flBefore, const CFileList& flAfter)
{
	if (flBefore.GetFileCount() != flAfter.GetFileCount())
		throw logic_error("The number of files before and after renaming must be the same.");

	// Combine the two lists into a renaming list.
	m_vRenamingOperations.resize(flBefore.GetFileCount());
	m_vProblems.resize(flBefore.GetFileCount());
	for (int i=0; i<flBefore.GetFileCount(); ++i)
		m_vRenamingOperations[i] = CRenamingOperation(flBefore[i], flAfter[i]);
}

bool CRenamingList::IsUsingKtm() const
{
	KTMTransaction ktm;
	return ktm.GetTransaction() != NULL;
}

bool CRenamingList::Check()
{
	// Pre-condition: Foreach file to rename A: A.before != A.after.

	// Declarations.
	const int	nFilesCount = (int) m_vRenamingOperations.size();
	CFileFind	ffFileFind;

	// Reinitialize the error list.
	ASSERT(m_vProblems.size() == m_vRenamingOperations.size());
	m_nErrors = m_nWarnings = 0;
	BOOST_FOREACH(COperationProblem& problem, m_vProblems)
	{
		problem.nErrorLevel = levelNone;
		problem.nErrorCode = errNoError;
		problem.strMessage.Empty();
	}

	// Change the locale to match the file system stricmp().
	CString strLocaleBak = _tsetlocale(LC_CTYPE, NULL);
	_tsetlocale(LC_CTYPE, _T(""));
	
	{
		// TODO: Try with std::string instead of CString to see which one is faster.
		
		// Create a map of file names (in lower case) associated to the operation index.
		set<CString> setBeforeLower;
		for (int i=0; i<nFilesCount; ++i)
		{
			// Report progress
			m_fOnProgress(stageChecking, i*30/nFilesCount, 100);

			CString strName = m_vRenamingOperations[i].pathBefore.GetFullPath();
			setBeforeLower.insert( strName.MakeLower() );
		}	
	
		// Check for file conflicts.
		map<CString, int> mapAfterLower;
		for (int i=0; i<nFilesCount; ++i)
		{
			// Report progress
			m_fOnProgress(stageChecking, 30 + i*50/nFilesCount, 100);
	
			// If that file isn't already marked as conflicting with another,
			// test if it's going to conflict with another file.
			if (m_vProblems[i].nErrorCode == errConflict)
				continue;

			CString strAfterLower = m_vRenamingOperations[i].pathAfter.GetFullPath();
			strAfterLower.MakeLower();
			
			/* Detect if it's going to be renamed to a file that already exists
			 * but that is not part of the files to rename...
			 */
			if (ffFileFind.FindFile(strAfterLower)		// The destination exists on the disk.
				&& setBeforeLower.find(strAfterLower) == setBeforeLower.end())	// and it's not going to be renamed.
			{
				// No it is not, so it will conflict with the existing file on the disk.
				CString strErrorMsg;
				strErrorMsg.LoadString(IDS_CONFLICT_WITH_EXISTING);
				SetProblem(i, errConflict, strErrorMsg);
			}
			/* If it's not going to conflict with a file not part of the files to rename,
			 * check if it conflicts with files that are going to be renamed...
			 */
			else
			{
				// Check if two files are going to have the same new file name.
				map<CString, int>::const_iterator iterFound = mapAfterLower.find(strAfterLower);
				if (iterFound != mapAfterLower.end())
				{
					// Conflict found: Two files are going to be renamed to the same new file name.
					CString strErrorMsg;

					AfxFormatString1(strErrorMsg, IDS_CONFLICT_SAME_AFTER, m_vRenamingOperations[iterFound->second].pathBefore.GetDisplayPath());
					SetProblem(i, errConflict, strErrorMsg);

					AfxFormatString1(strErrorMsg, IDS_CONFLICT_SAME_AFTER, m_vRenamingOperations[i].pathBefore.GetDisplayPath());
					SetProblem(iterFound->second, errConflict, strErrorMsg);
				}
				else
				{// We add this file to the map.
					mapAfterLower.insert( pair<CString, int>(strAfterLower, i) );
				}
			}
		} // end: conflict check for-loop
	}

	// Check if file name is valid
	// and Check if file still exists
	for (int i=0; i<nFilesCount; ++i)
	{
		// Report progress
		m_fOnProgress(stageChecking, 80 + i*20/nFilesCount, 100);

		// Check if the file still exists
		if (!ffFileFind.FindFile( m_vRenamingOperations[i].pathBefore.GetFullPath() ))
		{
			CString strErrorMsg;
			strErrorMsg.LoadString(IDS_REMOVED_FROM_DISK);
			SetProblem(i, errFileMissing, strErrorMsg);
		}

		// Check the file name.
		{
			COperationProblem problem = CheckName(
				m_vRenamingOperations[i].pathAfter.GetFileName(),
				m_vRenamingOperations[i].pathAfter.GetFileNameWithoutExtension(),
				true);

			if (problem.nErrorLevel != levelNone)
			{
				// Some problem found.
				SetProblem(i, problem.nErrorCode, problem.strMessage);
			}
		}

		// Check that the directory starts and ends by a baclslash.
		CString strDirectory = m_vRenamingOperations[i].pathAfter.GetDirectoryName().Mid(m_vRenamingOperations[i].pathAfter.GetPathRoot().GetLength() - 1);
		if (strDirectory.IsEmpty() ||
			strDirectory[0] != '\\' ||
			strDirectory[strDirectory.GetLength() - 1] != '\\')
		{
			CString strErrorMsg;
			strErrorMsg.LoadString(IDS_START_END_BACKSLASH_MISSING);
			SetProblem(i, errBackslashMissing, strErrorMsg);
		}

		// Check if the directories name is valid.
		BOOST_FOREACH(CString strDirectory, m_vRenamingOperations[i].pathAfter.GetDirectories())
		{
			// Find the directory's name before the '.' (yes an extension is also counted for directories)
			CString strDirectoryWithoutExtension;
			int nPos = strDirectory.Find('.');
			if (nPos != -1)
				strDirectoryWithoutExtension = strDirectory.Left(nPos);
			else
				strDirectoryWithoutExtension = strDirectory;

			// Check the name.
			COperationProblem problem = CheckName(strDirectory, strDirectoryWithoutExtension, false);
			if (problem.nErrorLevel != levelNone)
			{
				// Some problem found.
				SetProblem(i, problem.nErrorCode, problem.strMessage);
				break;
			}
		} // end: directory name validity check.

		// Check if the path if over MAX_PATH.
		if (m_vRenamingOperations[i].pathAfter.GetDisplayPath().GetLength() >= MAX_PATH)
		{
			CString strErrorMsg;
			strErrorMsg.LoadString(IDS_LONGUER_THAN_MAX_PATH);
			SetProblem(i, errLonguerThanMaxPath, strErrorMsg);
		}
	}

	// Restaure the locale.
	_tsetlocale(LC_CTYPE, strLocaleBak);

	// Post condition.
	ASSERT(m_vProblems.size() == m_vRenamingOperations.size());

	// Report the errors.
	return m_nErrors == 0 && m_nWarnings == 0;
}

CRenamingList::COperationProblem CRenamingList::CheckName(const CString& strName, const CString& strNameWithoutExtension, bool bIsFileName)
{
	// Invalid path characters might include ASCII/Unicode characters 1 through 31, as well as quote ("), less than (<), greater than (>), pipe (|), backspace (\b), null (\0) and tab (\t).
	if (strName.IsEmpty()	// Empty name.
		|| strName.FindOneOf(_T("\\/:*?\"<>|\b\t")) != -1	// Forbidden characters.
		|| strName.Right(0) == _T("."))	// The OS doesn't support files/directories ending by a dot.
	{
		// Warning: Invalid.
		COperationProblem problem;
		if (bIsFileName)
		{
			problem.nErrorLevel = levelError;
			problem.nErrorCode = errInvalidFileName;
			problem.strMessage.LoadString(IDS_INVALID_FILE_NAME);
		}
		else
		{
			problem.nErrorLevel = levelError;
			problem.nErrorCode = errInvalidDirectoryName;
			problem.strMessage.LoadString(IDS_INVALID_DIRECTORY_NAME);
		}
		return problem;
	}

	// Look for some other characters that are invalid: 0x00 to 0x20.
	for (int i=0; i<strName.GetLength(); ++i)
	{
		// ASCII/Unicode characters 1 through 31, as well as null (\0), are not allowed.
		if (strName[i] < 0x20)
		{
			// Windows XP rejects any file name with a character below 32.
			COperationProblem problem;
			if (bIsFileName)
			{
				problem.nErrorLevel = levelError;
				problem.nErrorCode = errInvalidFileName;
				problem.strMessage.LoadString(IDS_INVALID_FILE_NAME);
			}
			else
			{
				problem.nErrorLevel = levelError;
				problem.nErrorCode = errInvalidDirectoryName;
				problem.strMessage.LoadString(IDS_INVALID_DIRECTORY_NAME);
			}
			return problem;
		}
	}

	// Check if it's a name to avoid.

	// Over 120 characters (maximum allowed from the Explorer)
	if (strName.GetLength() > 256)
	{
		// Warning: Too long.
		if (bIsFileName)
		{
			COperationProblem problem;
			problem.nErrorLevel = levelWarning;
			problem.nErrorCode = errRiskyFileName;
			problem.strMessage.LoadString(IDS_RISKY_FNAME_TOO_LONG);
			return problem;
		}
		//else
		//{
		//}
	}

	// The following reserved words cannot be used as the name of a file:
	// CON, PRN, AUX, CLOCK$, NUL, COM1, COM2, ..., COM9, LPT1, LPT2, ..., LPT9.
	// Also, reserved words followed by an extension—for example, NUL.tx7—are invalid file names.
	const int MAX_INVALID_NAME_LENGTH = 6;	// = strlen("CLOCK$")
	if ((strNameWithoutExtension.GetLength() <= MAX_INVALID_NAME_LENGTH	// Quickly remove any file name that can't be invalid...
			&& (   strNameWithoutExtension.CompareNoCase(_T("CON")) == 0	// to quickly detect system reserved file names.
				|| strNameWithoutExtension.CompareNoCase(_T("PRN")) == 0
				|| strNameWithoutExtension.CompareNoCase(_T("AUX")) == 0
				|| strNameWithoutExtension.CompareNoCase(_T("CLOCK$")) == 0 
				|| strNameWithoutExtension.CompareNoCase(_T("NUL")) == 0 
				|| (strNameWithoutExtension.GetLength() == 4
					&& (_tcsncicmp(strNameWithoutExtension, _T("COM"), 3) == 0 || _tcsncicmp(strNameWithoutExtension, _T("LPT"), 3) == 0)
					&& (strNameWithoutExtension[3] >= _T('1') && strNameWithoutExtension[3] <= _T('9'))
				   )
				)
		   )
	   )
	{
		// Warning: Reserved file name.
		COperationProblem problem;
		if (bIsFileName)
		{
			problem.nErrorLevel = levelWarning;
			problem.nErrorCode = errRiskyFileName;
			problem.strMessage.LoadString(IDS_RISKY_FNAME_RESERVED);
		}
		else
		{
			problem.nErrorLevel = levelWarning;
			problem.nErrorCode = errRiskyDirectoryName;
			problem.strMessage.LoadString(IDS_RISKY_DIRNAME_RESERVED);
		}
		return problem;
	}

	// Starting or ending by one or more spaces.
	if (strName.GetAt(0) == _T(' ')			// Files/Folders starting by a space is not good.
		|| strName.Right(0) == _T(' '))		// Files/Folders ending by a space is not good.
	{
		// Warning: Spaces before or after.
		COperationProblem problem;
		if (bIsFileName)
		{
			problem.nErrorLevel = levelWarning;
			problem.nErrorCode = errRiskyFileName;
			problem.strMessage.LoadString(IDS_RISKY_FNAME_TRIM_SPACES);
		}
		else
		{
			problem.nErrorLevel = levelWarning;
			problem.nErrorCode = errRiskyDirectoryName;
			problem.strMessage.LoadString(IDS_RISKY_DIRNAME_TRIM_SPACES);
		}
		return problem;
	}

	return COperationProblem();
}

bool CRenamingList::PerformRenaming()
{
	// Avoid possible strange behaviours for empty lists.
	if (m_vRenamingOperations.size() == 0)
		return true;

	// Pre-conditions checking.
#ifdef _DEBUG
	BOOST_FOREACH(CRenamingOperation operation, m_vRenamingOperations)
	{
		// Each operation changes something.
		ASSERT(operation.pathBefore != operation.pathAfter);	// Note that this is case sensitive.
	}
#endif

	// Create an oriented graph of conflicting renamings:
	Beroux::Math::OrientedGraph	graph;
	set<CString, path_compare<CString>> setDeleteIfEmptyDirectories;
	{
		const int nFilesCount = (int) m_vRenamingOperations.size();

		// Change the locale to match the file system stricmp().
		CString strLocaleBak = _tsetlocale(LC_CTYPE, NULL);
		_tsetlocale(LC_CTYPE, _T(""));

		// Create a first version of an oriented graph of conflicting renamings:
		// Node(i) -edge-to-> Node(j)   <=>   File(i).originalFileName == File(j).newFileName (compare no case)
		//                              <=>   File(i) must be renamed before File(j)
		map<CString, int> mapAfterLower;
		
		// Create a node for each renaming operation,
		// and the mapAfterLower.
		for (int i=0; i<nFilesCount; ++i)	
		{
			// Report progress
			m_fOnProgress(stagePreRenaming, i*20/nFilesCount, 100);

			graph.AddNode(i);

			CString strAfter = m_vRenamingOperations[i].pathAfter.GetFullPath();
			mapAfterLower.insert( pair<CString, int>(strAfter.MakeLower(), i) );
		}

		// Define the successors in the graph,
		// and add folders to later erase if they're empty.
		for (int i=0; i<nFilesCount; ++i)
		{
			// Report progress
			m_fOnProgress(stagePreRenaming, 20 + i*80/nFilesCount, 100);

			// Look if a node with a file-name-after has the same name as this node file-name-before.
			CString strBefore = m_vRenamingOperations[i].pathBefore.GetFullPath();
			map<CString, int>::const_iterator iterFound = mapAfterLower.find(strBefore.MakeLower());
			if (iterFound != mapAfterLower.end())
			{
				graph[i].AddSuccessor(iterFound->second);
				
				// Assertion: There can be at most one successor,
				// else it means that two files (or more) will have the same destination name.

				// Check if their are par of a cycle (only nodes that have a successor and an antecedent may form a cycle).
				if (graph[i].HasSuccessor() && graph[i].HasAntecedent())
				{
					for (int j=graph[i].GetSuccessor(0); graph[j].HasSuccessor(); j=graph[j].GetSuccessor(0))
						if (i == j)	// Cycle found: Node(i) --> Node(i).successor --> ... --> Node(i).
						{
							// Add a new node, to make a temporary rename that solves the problem.
							// It is fixes the problem because the temporary file doesn't exist yet,
							// so it doesn't conflict.

							// Originally rename A -> B
							CPath fnFinal = m_vRenamingOperations[graph[i].GetSuccessor(0)].pathAfter;

							CString strRandomName = _T("~");
							for (int k=0; k<15; ++k)
								strRandomName += _T("0123456789ABCDEF")[rand()%16];
							CPath fnTemp = fnFinal.GetFullPath() + strRandomName;

							// Rename A -> TMP
							m_vRenamingOperations[graph[i].GetSuccessor(0)].pathAfter = fnTemp;

							// Then rename TMP -> B
							AddRenamingOperation( CRenamingOperation(fnTemp, fnFinal) );

							// Add Node(i) --> Node(TMP), because File(i) must be renamed before File(TMP).
							graph.AddNode((int)m_vRenamingOperations.size() - 1);
							graph[i].RemoveSuccessor(0);
							graph[i].AddSuccessor((int)m_vRenamingOperations.size() - 1);

							break;
						}
				} // end if cycle detection.
			} // end if successor found.

			// Mark folders the should be erased after renaming if they are empty.
			CString strParentPath = m_vRenamingOperations[i].pathBefore.GetPathRoot();
			BOOST_FOREACH(CString strDirectoryName, m_vRenamingOperations[i].pathBefore.GetDirectories())
			{
				// Get the full parent directory's path.
				strParentPath += strDirectoryName.MakeLower();
				strParentPath.AppendChar('\\');

				// Add to the set.
				setDeleteIfEmptyDirectories.insert(strParentPath);
			}
		}

		// Restaure the locale.
		_tsetlocale(LC_CTYPE, strLocaleBak);
	}

	// Create a new transaction.
	KTMTransaction ktm;

	// Rename files in topological order.
	int nDone = 0;
	int nTotal = (int) m_vRenamingOperations.size();
	m_fOnProgress(stageRenaming, 0, nTotal);	// Inform we start renaming.
	bool bError = false;
	for (int i=0; i<nTotal; ++i)
		if (!graph[i].HasAntecedent())
		{
			int nIndex = i;
			while (true)
			{
				bool bAfterPathCreation = true;	// We suppose the creation of the new parent tree path worked.

				// Check that every parent directory exists, or create it.
				CString strParentPath = m_vRenamingOperations[nIndex].pathAfter.GetPathRoot();
				BOOST_FOREACH(CString strDirectoryName, m_vRenamingOperations[nIndex].pathAfter.GetDirectories())
				{
					// Keep the parent directory of this parent directory.
					CString strParentParentPath = strParentPath;

					// Get the full parent directory's path.
					strParentPath += strDirectoryName;
					strParentPath.AppendChar('\\');

					// Check if the parent path exists.
					WIN32_FIND_DATA fd;
					HANDLE hFind = ktm.FindFirstFileEx(strParentPath + '.', FindExInfoStandard, &fd, FindExSearchNameMatch/*FindExSearchLimitToDirectories*/, NULL, 0);
					if (hFind != INVALID_HANDLE_VALUE)
					{// This parent directory already exist.
						FindClose(hFind);

						// Check if the case is changed.
						if (strDirectoryName != fd.cFileName)
						{
							// Change the case.
							if (!ktm.MoveFileEx(
									strParentParentPath + fd.cFileName,
									strParentPath,
									0))
							{
								m_fOnRenamed(nIndex, GetLastError());
								bError = true;
							}
						}
					}
					else
					{
						DWORD dwLastError = GetLastError();

						if (dwLastError == ERROR_FILE_NOT_FOUND)
						{
							// Create the parent directory.
							if (!ktm.CreateDirectoryEx(NULL, strParentPath, NULL))
							{
								// Could not create the directory, let's report this error.
								m_fOnRenamed(nIndex, GetLastError());
								bError = true;

								// Now we exit and continue on the next file.
								bAfterPathCreation = false;
								break;
							}
						}
						else
						{
							// Some other problem, report it.
							m_fOnRenamed(nIndex, dwLastError);
							bError = true;

							// Now we exit and continue on the next file.
							bAfterPathCreation = false;
							break;
						}
					}
				}

				// Rename file.
				if (bAfterPathCreation)
				{
					if (!ktm.MoveFileEx(
							m_vRenamingOperations[nIndex].pathBefore.GetFullPath(),
							m_vRenamingOperations[nIndex].pathAfter.GetFullPath(),
							MOVEFILE_COPY_ALLOWED))
					{
						m_fOnRenamed(nIndex, GetLastError());
						bError = true;
					}
					else
						m_fOnRenamed(nIndex, 0);
				}

				// Report progress
				m_fOnProgress(stageRenaming, ++nDone, nTotal);

				// Rename its successors.
				if (graph[nIndex].HasSuccessor())
					nIndex = graph[nIndex].GetSuccessor(0);
				else
					break;
			}
		}

	// Delete emptied folders (folders that are empty after renaming).
	// Note that the set is ordered so that the longuest path comes first,
	// which implies that subfolders are removed prior to checking their parent folder.
	BOOST_FOREACH(CString strDirectory, setDeleteIfEmptyDirectories)
	{
		// Remove ONLY a non-empty directory.
		if (!ktm.RemoveDirectory(strDirectory))
		{
			DWORD dwErrorCode = ::GetLastError();
			/** Some possible error codes include:
			 * ERROR_ACCESS_DENIED		For folders maked as readonly, and probably for folders with other security settings.
			 * ERROR_FILE_CORRUPT		When the directory cannot be accessed because of some problem.
			 * ERROR_DIR_NOT_EMPTY		Directory not empty.
			 * ERROR_FILE_NOT_FOUND		Directory doesn't exist.
			 */
			if (dwErrorCode != ERROR_DIR_NOT_EMPTY)
			{// Warning: ERROR_ACCESS_DENIED may be reported for non-empty dir that are protected.
				// TODO: Report this error as a WARNING.
				bError = true;
			}
		}
	}

	if (bError)
	{
		// TODO: Possibly commit or roll-back depending on the user's choice.
		if (AfxMessageBox(_T("Errors.\nPress YES to commit or NO roll-back."), MB_YESNO) == IDYES)
			VERIFY(ktm.Commit());
		else
			VERIFY(ktm.RollBack());
		return false;
	}
	else
		return ktm.Commit();
}
