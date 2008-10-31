#include "StdAfx.h"
#include "RenamingList.h"
#include "../resource.h"
#include "ScopedLocale.h"
#include "IOOperation/CreateDirectoryOperation.h"
#include "IOOperation/RenameOperation.h"
#include "IOOperation/RemoveEmptyDirectoryOperation.h"
#include "Math/RandomMT.h"

using namespace Beroux::IO::Renaming::IOOperation;

namespace Beroux{ namespace IO{ namespace Renaming
{

CRenamingList::CRenamingList(void)
	: m_nWarnings(0)
	, m_nErrors(0)
{
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
	if (flBefore.GetCount() != flAfter.GetCount())
		throw logic_error("The number of files before and after renaming must be the same.");

	// Combine the two lists into a renaming list.
	m_vRenamingOperations.resize(flBefore.GetCount());
	m_vProblems.resize(flBefore.GetCount());
	for (int i=0; i<flBefore.GetCount(); ++i)
		m_vRenamingOperations[i] = CRenamingOperation(flBefore[i], flAfter[i]);
}

bool CRenamingList::Check()
{
	// Pre-conditions
#ifdef _DEBUG
	BOOST_FOREACH(CRenamingOperation& operation, m_vRenamingOperations)
	{
		// Each operation must be a Unicode path.
		ASSERT(operation.pathBefore.GetPath().Left(4) == "\\\\?\\");
		ASSERT(operation.pathAfter.GetPath().Left(4) == "\\\\?\\");

		// Each operation changes something.
		ASSERT(operation.pathBefore != operation.pathAfter);
	}
#endif

	// Declarations.
	const unsigned nRenamingOperationCount = (unsigned)m_vRenamingOperations.size();
	if (nRenamingOperationCount == 0)
		return true;	// Nothing to rename.

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
	CScopedLocale scopeLocale(_T(""));
	
	{
		// First pass: Preparation.
		set<CString> setBeforeLower;
		for (unsigned i=0; i<nRenamingOperationCount; ++i)
		{
			// Report progress
			OnProgressChanged(stageChecking, i*20/nRenamingOperationCount, 100);

			// Create a map of file names (in lower case) associated to the operation index.
			CString strName = m_vRenamingOperations[i].pathBefore.GetPath();
			setBeforeLower.insert( strName.MakeLower() );
		}	
	
		// Checking loop.
		TRenamingOperationSet vRenamingOperations = m_vRenamingOperations;
		CDirectoryCaseUnifier dirCaseUnifier(vRenamingOperations);
		map<CString, int> mapAfterLower;
		for (unsigned i=0; i<nRenamingOperationCount; ++i)
		{
			// Report progress
			OnProgressChanged(stageChecking, 20 + i*70/nRenamingOperationCount, 100);
	
			// Check for file conflicts.
			CheckFileConflict(i, setBeforeLower, mapAfterLower);

			// Check if the file/folder still exists
			if (!CPath::PathFileExists( m_vRenamingOperations[i].pathBefore.GetPath() ))
			{
				CString strErrorMsg;
				strErrorMsg.LoadString(IDS_REMOVED_FROM_DISK);
				SetProblem(i, errFileMissing, strErrorMsg);
			}

			// Check the file/folder name.
			COperationProblem problem = CheckName(
				m_vRenamingOperations[i].pathAfter.GetFileName(),
				m_vRenamingOperations[i].pathAfter.GetFileNameWithoutExtension(),
				true);
			if (problem.nErrorLevel != levelNone)
			{
				// Some problem found.
				SetProblem(i, problem.nErrorCode, problem.strMessage);
			}

			// Check the directory name.
			CheckDirectoryPath(i);

			// Check folders' case consistency
			dirCaseUnifier.ProcessOperation(i);
		} // end: checking loop.

		// Report operations with not unified directory case.
		for (unsigned i=0; i<nRenamingOperationCount; ++i)
		{
			// Report progress
			OnProgressChanged(stageChecking, 90 + i*10/nRenamingOperationCount, 100);

			// Check if the case is not unified.
			if (vRenamingOperations[i].pathAfter != m_vRenamingOperations[i].pathAfter)
			{
				// Report the problem.
				CString strMessage;
				strMessage.LoadString(IDS_INCONSISTENT_DIRECTORY_CASE);
				SetProblem(i, errDirCaseInconsistent, strMessage);
			} // end: Check if the case differ.
		}
	}

	// Post condition.
	ASSERT(m_vProblems.size() == m_vRenamingOperations.size());

	// Report the errors.
	return m_nErrors == 0 && m_nWarnings == 0;
}

void CRenamingList::CheckFileConflict(int nOperationIndex, const set<CString>& setBeforeLower, map<CString, int>& mapAfterLower)
{
	// If that file isn't already marked as conflicting with another,
	// test if it's going to conflict with another file.
	if (m_vProblems[nOperationIndex].nErrorCode == errConflict)
		return;

	CString strAfterLower = m_vRenamingOperations[nOperationIndex].pathAfter.GetPath();
	strAfterLower.MakeLower();
	
	// Detect if it's going to be renamed to a file that already exists
	// but that is not part of the files to rename...
	if (CPath::PathFileExists(strAfterLower)	// The destination exists on the disk.
		&& setBeforeLower.find(strAfterLower) == setBeforeLower.end())	// and it's not going to be renamed.
	{
		// No it is not, so it will conflict with the existing file on the disk.
		CString strErrorMsg;
		strErrorMsg.LoadString(IDS_CONFLICT_WITH_EXISTING);
		SetProblem(nOperationIndex, errConflict, strErrorMsg);
	}
	// If it's not going to conflict with a file not part of the files to rename,
	// check if it conflicts with files that are going to be renamed...
	else
	{
		// Check if two files are going to have the same new file name.
		map<CString, int>::const_iterator iterFound = mapAfterLower.find(strAfterLower);
		if (iterFound != mapAfterLower.end())
		{
			// Conflict found: Two files are going to be renamed to the same new file name.
			CString strErrorMsg;

			AfxFormatString1(strErrorMsg, IDS_CONFLICT_SAME_AFTER, m_vRenamingOperations[iterFound->second].pathBefore.GetPath());
			SetProblem(nOperationIndex, errConflict, strErrorMsg);

			AfxFormatString1(strErrorMsg, IDS_CONFLICT_SAME_AFTER, m_vRenamingOperations[nOperationIndex].pathBefore.GetPath());
			SetProblem(iterFound->second, errConflict, strErrorMsg);
		}
		else
		{// We add this file to the map.
			mapAfterLower.insert( pair<CString, int>(strAfterLower, nOperationIndex) );
		}
	}
}

void CRenamingList::CheckDirectoryPath(int nOperationIndex)
{
	const CPath* pOperationPathAfter = &m_vRenamingOperations[nOperationIndex].pathAfter;

	// Check that the directory starts and ends by a backslash.
	const CString& strRoot = pOperationPathAfter->GetPathRoot();
	const CString& strDir = pOperationPathAfter->GetDirectoryName();

	if (strRoot.IsEmpty() || 
		strRoot[strRoot.GetLength() - 1] != '\\' ||
		(!strDir.IsEmpty() && strDir[strDir.GetLength() - 1] != '\\'))
	{
		CString strErrorMsg;
		strErrorMsg.LoadString(IDS_START_END_BACKSLASH_MISSING);
		SetProblem(nOperationIndex, errBackslashMissing, strErrorMsg);
	}

	// Check if the directories names are valid.
	BOOST_FOREACH(CString strDirectory, pOperationPathAfter->GetDirectories())
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
			SetProblem(nOperationIndex, problem.nErrorCode, problem.strMessage);
			break;
		}
	} // end: directory name validity check.

	// Check if the path if over MAX_PATH.
	if (CPath::MakeSimplePath(pOperationPathAfter->GetPath()).GetLength() >= MAX_PATH)
	{
		CString strErrorMsg;
		strErrorMsg.LoadString(IDS_LONGUER_THAN_MAX_PATH);
		SetProblem(nOperationIndex, errLonguerThanMaxPath, strErrorMsg);
	}
}

CRenamingList::COperationProblem CRenamingList::CheckName(const CString& strName, const CString& strNameWithoutExtension, bool bIsFileName)
{
	ASSERT(strName.Left(strNameWithoutExtension.GetLength()) == strNameWithoutExtension);

	// Empty name.
	if (strName.IsEmpty())
	{
		// Warning: Invalid.
		COperationProblem problem;
		if (bIsFileName)
		{
			problem.nErrorLevel = levelError;
			problem.nErrorCode = errInvalidFileName;
			problem.strMessage.LoadString(IDS_EMPTY_FILE_NAME);
		}
		else
		{
			problem.nErrorLevel = levelError;
			problem.nErrorCode = errInvalidDirectoryName;
			problem.strMessage.LoadString(IDS_EMPTY_DIRECTORY_NAME);
		}
		return problem;
	}

	// Invalid path characters might include ASCII/Unicode characters 1 through 31, as well as quote ("), less than (<), greater than (>), pipe (|), backspace (\b), null (\0) and tab (\t).
	if (strName.FindOneOf(_T("\\/:*?\"<>|\b\t")) != -1	// Forbidden characters.
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
	if (strName.GetLength() > 120)
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
		else
		{
			COperationProblem problem;
			problem.nErrorLevel = levelWarning;
			problem.nErrorCode = errRiskyDirectoryName;
			problem.strMessage.LoadString(IDS_RISKY_DIRNAME_TOO_LONG);
			return problem;
		}
	}

	// The following reserved words cannot be used as the name of a file:
	// CON, PRN, AUX, CLOCK$, NUL, COM1, COM2, ..., COM9, LPT1, LPT2, ..., LPT9.
	// Also, reserved words followed by an extension—for example, NUL.tx7—are invalid file names.
	const int MAX_INVALID_NAME_LENGTH = 6; // = strlen("CLOCK$");
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
	ASSERT(!strName.IsEmpty());
	if (strName[0] == _T(' ') ||						// Files/Directories starting by a space is not good.
		strName[strName.GetLength() - 1] == _T(' '))	// Files/Directories ending by a space is not good.
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

void CRenamingList::SetProblem(int nOperationIndex, EErrorCode nErrorCode, CString strMessage)
{
	COperationProblem& operationProblem = m_vProblems[nOperationIndex];

	// Errors should always go up and keep the highest one only.
	if (nErrorCode > operationProblem.nErrorCode)
	{
		// Find the error level from the error code.
		EErrorLevel nLevel;
		BOOST_STATIC_ASSERT(errCount == 10);
		switch (nErrorCode)
		{
		case errDirCaseInconsistent:
		case errLonguerThanMaxPath:
		case errRiskyFileName:
		case errRiskyDirectoryName:
			nLevel = levelWarning;
			break;

		default:
			nLevel = levelError;
			break;
		}

		// Update error counters.
		switch (nLevel)
		{
		case levelWarning:	++m_nWarnings; break;
		case levelError:	++m_nErrors; break;
		}

		// Save the problem in the report.
		operationProblem.nErrorLevel = nLevel;
		operationProblem.nErrorCode = nErrorCode;
		operationProblem.strMessage = strMessage;
		ASSERT((operationProblem.nErrorLevel==levelNone) ^ (operationProblem.nErrorCode!=errNoError)); // no error <=> no error code, an error <=> error code set
		ASSERT((operationProblem.nErrorLevel==levelNone) ^ !operationProblem.strMessage.IsEmpty());	// no error <=> no error message, an error <=> error message set
	}
}

bool CRenamingList::PerformRenaming(CKtmTransaction& ktm) const
{
	// Avoid possible strange behaviors for empty lists.
	if (m_vRenamingOperations.size() == 0)
		return true;

	// Transform the set of renaming operations into an ordered list of
	// operations arranged so that by performing all the operations
	// one after another is possible and will result in performing all the
	// renaming operations asked.
	vector<shared_ptr<CIOOperation> > vOperationList = PrepareRenaming();

	// Failover to non-KTM when ::GetLastError() == ERROR_RM_NOT_ACTIVE
	// TODO: See how to alert the user or even tell him before
	//       renaming that KTM will or will not be supported.
	//       Could use the FindFile to detect before renaming.
	//       Could also probably use GetVolumeInformation().
	// FIXME: When KTM is supported on the system but not on some of its
	//        file systems (like the case here), the error report dialog
	//        still proposes to roll back even though it can't.
	// POSSIBLE SOLUTION: Try with KTM (showing some logo somewhere before),
	//                    and if it fails, ask the user to possibly retry
	//                    without KTM.
	// TODO: Add a command-line option to specify whenever KTM should be used or not.

	// Rename files order.
	OnProgressChanged(stageRenaming, 0, (int)vOperationList.size());	// Inform we start renaming.

	bool bError = false;
	for (unsigned i=0; i<vOperationList.size(); ++i)
	{
		// Perform the operation.
		CIOOperation::EErrorLevel nErrorLevel = vOperationList[i]->Perform(ktm);
		OnIOOperationPerformed(*vOperationList[i], nErrorLevel);
		if (nErrorLevel != CIOOperation::elSuccess)
			bError = true;

		// Report progress.
		OnProgressChanged(stageRenaming, i, (int)vOperationList.size());	// Inform we start renaming.
	}

	return !bError;
}

void CRenamingList::OnIOOperationPerformed(const CIOOperation& ioOperation, CIOOperation::EErrorLevel nErrorLevel) const
{
	IOOperationPerformed(*this, ioOperation, nErrorLevel);
}

void CRenamingList::OnProgressChanged(EStage nStage, int nDone, int nTotal) const
{
	ProgressChanged(*this, nStage, nDone, nTotal);
}

vector<shared_ptr<CIOOperation> > CRenamingList::PrepareRenaming() const
{
	bool bRenamingDirectories = (::GetFileAttributes(m_vRenamingOperations[0].GetPathBefore().GetPath()) & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;

	// Pre-conditions checking.
#ifdef _DEBUG
	BOOST_FOREACH(const CRenamingOperation& operation, m_vRenamingOperations)
	{
		// Each operation must be a Unicode path.
		ASSERT(operation.pathBefore.GetPath().Left(4) == "\\\\?\\");
		ASSERT(operation.pathAfter.GetPath().Left(4) == "\\\\?\\");

		// Each operation changes something.
		ASSERT(operation.pathBefore != operation.pathAfter);	// Note that this is case sensitive.

		// Must either rename only files or only directories.
		bool bIsDirectories = (::GetFileAttributes(operation.pathBefore.GetPath()) & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
		ASSERT(bIsDirectories == bRenamingDirectories);
	}
#endif

	TRenamingOperationSet vRenamingOperations = m_vRenamingOperations;
	if (bRenamingDirectories)
		return PrepareDirectoryRenaming(vRenamingOperations);
	else
		return PrepareFileRenaming(vRenamingOperations);
}

vector<shared_ptr<CIOOperation> > CRenamingList::PrepareFileRenaming(TRenamingOperationSet& vRenamingOperations) const
{
	///////////////////////////////////////////////////////////////////
	// Create an oriented graph of conflicting renaming operations
	const int nRenamingOperationCount = (int) vRenamingOperations.size();

	// Change the locale to match the file system stricmp().
	CScopedLocale scopeLocale(_T(""));

	// Create a first version of an oriented graph of conflicting renaming operations:
	// Node(i) -edge-to-> Node(j)   <=>   File(i).originalFileName == File(j).newFileName (compare no case)
	//                              <=>   File(i) must be renamed before File(j)
	Beroux::Math::OrientedGraph	graph;
	map<CString, int> mapAfterLower;

	// Create a node for each renaming operation,
	// and the mapAfterLower.
	for (int i=0; i<nRenamingOperationCount; ++i)	
	{
		// Report progress
		OnProgressChanged(stagePreRenaming, i*20/nRenamingOperationCount, 100);

		graph.AddNode(i);

		CString strAfter = vRenamingOperations[i].pathAfter.GetPath();
		mapAfterLower.insert( pair<CString, int>(strAfter.MakeLower(), i) );
	}

	// Unify folders' case (1/2): Find the length of the shortest path.
	CDirectoryCaseUnifier dirCaseUnifier(vRenamingOperations);

	// Directories to remove if empty (ordered set by longest path first).
	set<CString, path_compare<CString> > setDeleteIfEmptyDirectories;

	// Define the successors in the graph,
	// and add folders to later erase if they're empty.
	for (int i=0; i<nRenamingOperationCount; ++i)
	{
		// Report progress
		OnProgressChanged(stagePreRenaming, 20 + i*75/nRenamingOperationCount, 100);

		// Definitions
		CPath* proBefore = &vRenamingOperations[i].pathBefore;
		CPath* proAfter = &vRenamingOperations[i].pathAfter;

		// Look if there is a node with name-after equal to this node's name-before
		// (meaning that there should be an edge from the this node to the found node).
		// We ignore when the operation `i` has the same name a before and after
		// (since the checking would then be handled by the file system).
		CString strBefore = proBefore->GetPath();
		map<CString, int>::const_iterator iterFound = mapAfterLower.find(strBefore.MakeLower());
		if (iterFound != mapAfterLower.end() && iterFound->second != i)
		{
			// Node(i) -edge-to-> Node(j)
			graph[i].AddSuccessor(iterFound->second);

			// Assertion: There can be at most one successor,
			// else it means that two files (or more) will have the same destination name.

			// Check if their are part of a cycle (only nodes that have a successor and an antecedent may form a cycle).
			if (graph[i].HasSuccessor() && graph[i].HasAntecedent())
			{
				for (int j=graph[i].GetSuccessor(0); graph[j].HasSuccessor(); j=graph[j].GetSuccessor(0))
					if (i == j)	// Cycle found: Node(i) --> Node(i).successor --> ... --> Node(i).
					{
						// Add a new node, to make a temporary rename that solves the problem.
						// It is fixes the problem because the temporary file doesn't exist yet,
						// so it doesn't conflict.

						// Originally rename A -> B
						int nOperationIndex = graph[i].GetSuccessor(0);
						CPath pathFinal = vRenamingOperations[nOperationIndex].pathAfter;

						// Find a temporary name to rename A -> TMP -> B.
						CString strRandomName;
						{
							ASSERT(pathFinal.GetPath()[pathFinal.GetPath().GetLength() - 1] != '\\');
							strRandomName = pathFinal.GetPath() + _T("~TMP");

							CString strRandomNameFormat = strRandomName + _T("%d");
							for (int k=2; CPath::PathFileExists(strRandomName) || mapAfterLower.find(strRandomName) != mapAfterLower.end(); ++k)
								strRandomName.Format(strRandomNameFormat, k);
						}
						CPath pathTemp = strRandomName;

						// Rename A -> TMP
						vRenamingOperations[nOperationIndex].pathAfter = pathTemp;
						// (Optional operation since mapAfterLower.find(X.before) == This operation.)
						//CString strAfter = pathTemp.GetPath();
						//mapAfterLower.insert( pair<CString, int>(strAfter.MakeLower(), nOperationIndex) );

						// Then rename TMP -> B
						vRenamingOperations.push_back( CRenamingOperation(pathTemp, pathFinal) );
						int nSecondOperationIndex = (int)vRenamingOperations.size() - 1;
						CString strAfter = pathTemp.GetPath();
						mapAfterLower[strAfter.MakeLower()] = nSecondOperationIndex;

						// Since we changed the vector array, the memory location may have been changed also.
						proBefore = &vRenamingOperations[i].pathBefore;
						proAfter = &vRenamingOperations[i].pathAfter;

						// Add Node(i) --> Node(TMP), because File(i) must be renamed before File(TMP).
						graph.AddNode(nSecondOperationIndex);
						graph[i].RemoveSuccessor(0);
						graph[i].AddSuccessor(nSecondOperationIndex);

						break;
					}
			} // end if cycle detection.
		} // end if successor found.

		// Add directories to remove later if they're empty.
		if (proBefore->GetDirectoryName().CompareNoCase(proAfter->GetDirectoryName()) != 0 &&
			!DirectoryIsEmpty(proBefore->GetDirectoryName()))
		{
			CString strParentPath = proBefore->GetPathRoot();
			BOOST_FOREACH(CString strDirectoryName, proBefore->GetDirectories())
			{
				// Get the full parent directory's path.
				strParentPath += strDirectoryName.MakeLower();
				strParentPath.AppendChar('\\');

				// Add to the set.
				setDeleteIfEmptyDirectories.insert(strParentPath);
			}
		}

		// Unify folders' case 2/2
		dirCaseUnifier.ProcessOperation(i);
	} // end for each operations index `i`.

	///////////////////////////////////////////////////////////////////
	// Finally, create an ordered map of elements to rename so that the
	// longest path comes first.
	map<CString, int, path_compare<CString> > mapRenamingOperations;
	typedef std::pair<CString, int> ro_pair_t;
	{
		const int nTotal = (int) vRenamingOperations.size();
		for (int i=0; i<nTotal; ++i)
		{
			// Report progress
			OnProgressChanged(stagePreRenaming, 95 + i*5/nTotal, 100);

			if (!graph[i].HasAntecedent())
				mapRenamingOperations.insert( ro_pair_t(vRenamingOperations[i].pathBefore.GetPath(), i) );
		}
	}

	// Create a list of operations index that'll provide the files to rename
	// in topological order.
	vector<int> vOrderedOperationsIndexes;
	BOOST_FOREACH(ro_pair_t pair, mapRenamingOperations)
	{
		int nIndex = pair.second;
		while (true)
		{
			vOrderedOperationsIndexes.push_back(nIndex);

			// Rename its successors.
			if (graph[nIndex].HasSuccessor())
				nIndex = graph[nIndex].GetSuccessor(0);
			else
				break;
		}
	}

	// Create the list of operations
	vector<shared_ptr<CIOOperation> > vOrderedOperationList;

	// Add renaming operation that would change the case of existing
	// folders if different from the existing one.
	dirCaseUnifier.AddRenamingOperations(vOrderedOperationList);

	// Add requested renaming operations.
	BOOST_FOREACH(unsigned nIndex, vOrderedOperationsIndexes)
	{
		const CRenamingOperation& renamingOperation = vRenamingOperations[nIndex];

		// Check that every parent directory exists, or create it.
		vOrderedOperationList.push_back(shared_ptr<CIOOperation>(
			new CCreateDirectoryOperation(renamingOperation.pathAfter.GetDirectoryName())
			));

		// Rename file.
		vOrderedOperationList.push_back(shared_ptr<CIOOperation>(
			new CRenameOperation(renamingOperation.pathBefore.GetPath(), renamingOperation.pathAfter.GetPath())
			));
	}

	// Delete emptied folders (folders that are empty after renaming).
	// Note that the set is ordered so that the longest path comes first,
	// which implies that sub-folders are removed prior to checking their parent folder.
	// Mark folders that should be erased after renaming if they are empty.
	BOOST_FOREACH(CString& strDirectoryPath, setDeleteIfEmptyDirectories)
	{
		// Remove ONLY a non-empty directory.
		vOrderedOperationList.push_back(shared_ptr<CIOOperation>(
			new CRemoveEmptyDirectoryOperation(strDirectoryPath)
			));
	}

	return vOrderedOperationList;
}

vector<shared_ptr<CIOOperation> > CRenamingList::PrepareDirectoryRenaming(TRenamingOperationSet& vRenamingOperations) const
{
	const int nRenamingOperationCount = (int) vRenamingOperations.size();

	// Report progress
	OnProgressChanged(stagePreRenaming, 0, 100);

	// Change the locale to match the file system stricmp().
	CScopedLocale scopeLocale(_T(""));

	// Create the list of operations
	vector<shared_ptr<CIOOperation> > vOrderedOperationList;

	// Create a unique temporary folder like follow:
	// - On the same drive as the directories to rename,
	// - As a child from of the shortest common directory (before),
	// - Non-existing name in that directory (before and after renaming).
	CTempDirectoryMap mapTempDirectories = FindTempDirectories(vRenamingOperations);

	for (CTempDirectoryMap::const_iterator iter=mapTempDirectories.begin(); iter!=mapTempDirectories.end(); ++iter)
		vOrderedOperationList.push_back(shared_ptr<CIOOperation>(
			new CCreateDirectoryOperation(iter->second.first)
			));

	// Unify folders' case
	CDirectoryCaseUnifier dirCaseUnifier(vRenamingOperations);
	for (int i=0; i<nRenamingOperationCount; ++i)
	{
		// Report progress
		OnProgressChanged(stagePreRenaming, 0 + i*20/nRenamingOperationCount, 100);

		dirCaseUnifier.ProcessOperation(i);
	}

	// Add renaming operation that would change the case of existing
	// folders if different from the existing one.
	dirCaseUnifier.AddRenamingOperations(vOrderedOperationList);

	// For directories only changing their case, directly add the renaming operation.
	BOOST_FOREACH(const CRenamingOperation& renamingOperation, vRenamingOperations)
	{
		const CPath& pathBefore = renamingOperation.GetPathBefore().GetPath();
		const CPath& pathAfter = renamingOperation.GetPathAfter().GetPath();

		if (pathBefore.GetPath().CompareNoCase(pathAfter.GetPath()) == 0)
		{
			vOrderedOperationList.push_back(shared_ptr<CIOOperation>(
				new CRenameOperation(pathBefore, pathAfter)
				));
		}
	}

	// Report progress
	OnProgressChanged(stagePreRenaming, 20, 100);

	// Prepare a map moving the directories flat and then moving them to their destination path.
	typedef map<CString, shared_ptr<CRenameOperation>, path_compare<CString> > CFirstRenameOperationMap;	// Order so that the longest path comes first.
	typedef map<CString, shared_ptr<CRenameOperation>, path_reverse_compare<CString> > CSecondRenameOperationMap;	// Order so that the shortest path comes first.
	CFirstRenameOperationMap mapFirstOperation;	// Make all directory flat (= in one folder).
	CSecondRenameOperationMap mapSecondOperation;	// Make all directories to their final place.

	BOOST_FOREACH(const CRenamingOperation& renamingOperation, vRenamingOperations)
	{
		const CPath& pathBefore = renamingOperation.GetPathBefore();
		const CPath& pathAfter = renamingOperation.GetPathAfter();

		if (pathBefore.GetPath().CompareNoCase(pathAfter.GetPath()) != 0)
		{
			CTempDirectoryMap::const_iterator iter = mapTempDirectories.find(pathBefore.GetPathRoot());
			ASSERT(iter != mapTempDirectories.end());
			const CString& strTempPath = iter->second.first;
			unsigned nCommonPathLength = iter->second.second;
			ASSERT(strTempPath[strTempPath.GetLength() - 1] == '\\');

			// Convert the path to a flat name.
			CString strAfterFlatName = pathBefore.GetPath().Mid(nCommonPathLength);
			strAfterFlatName.Replace('\\', '_');
			ASSERT(strAfterFlatName.Find('/') == -1);
			CPath pathTempName = strTempPath + strAfterFlatName;

			// Prepare the operation before_path --> temp_dir\flat_name.
			mapFirstOperation.insert( pair<CString, shared_ptr<CRenameOperation> >(
				pathBefore.GetPath(),
				shared_ptr<CRenameOperation>(new CRenameOperation(pathBefore, pathTempName))
				));

			// Prepare the operation temp_dir\flat_name --> after_path.
			mapSecondOperation.insert( pair<CString, shared_ptr<CRenameOperation> >(
				pathAfter.GetPath(),
				shared_ptr<CRenameOperation>(new CRenameOperation(pathTempName, pathAfter))
				));
		}
	}

	// Report progress
	OnProgressChanged(stagePreRenaming, 70, 100);

	// Move all other(=not only changing case) directories as a direct child of the
	// temporary directory. Use a meaningful name, example:
	// ...\Foo\Bar  --> ...\Temp\Foo_Bar
	for (CFirstRenameOperationMap::const_iterator iter=mapFirstOperation.begin(); iter!=mapFirstOperation.end(); ++iter)
	{
		const shared_ptr<CRenameOperation>& ioOperation = iter->second;

		// Rename to the destination directory.
		vOrderedOperationList.push_back(ioOperation);
	}
	mapFirstOperation.clear();

	// Report progress
	OnProgressChanged(stagePreRenaming, 75, 100);

	// Move all other(=not only changing case) directories to their destination
	// folder after creating the missing parent directories, by increasing
	// "after" path length (to avoid creating missing folders that should be
	// created by another renaming operation).
	for (CSecondRenameOperationMap::const_iterator iter=mapSecondOperation.begin(); iter!=mapSecondOperation.end(); ++iter)
	{
		const shared_ptr<CRenameOperation>& ioOperation = iter->second;

		// Mark to create the destination directory if it doesn't exist.
		CString strParentDirectory = ioOperation->GetPathAfter().GetDirectoryName();
		if (!CPath::PathFileExists(strParentDirectory))
			vOrderedOperationList.push_back(shared_ptr<CIOOperation>(
				new CCreateDirectoryOperation(strParentDirectory)
				));

		// Rename to the destination directory.
		vOrderedOperationList.push_back(ioOperation);
	}
	mapSecondOperation.clear();

	// Report progress
	OnProgressChanged(stagePreRenaming, 80, 100);

	// Delete temporary directory if empty.
	for (CTempDirectoryMap::const_iterator iter=mapTempDirectories.begin(); iter!=mapTempDirectories.end(); ++iter)
		vOrderedOperationList.push_back(shared_ptr<CIOOperation>(
			new CRemoveEmptyDirectoryOperation(iter->second.first)
			));

	// Report progress
	OnProgressChanged(stagePreRenaming, 82, 100);

	// Delete the parents of the renamed directories if they're empty.
	set<CString, path_compare<CString> > setDeleteIfEmptyDirectories;	// Directories to remove if empty (ordered set by longest path first).

	BOOST_FOREACH(const CRenamingOperation& renamingOperation, vRenamingOperations)
	{
		CString strDirectoryBefore = renamingOperation.GetPathBefore().GetDirectoryName();
		CString strDirectoryAfter = renamingOperation.GetPathAfter().GetDirectoryName();

		if (strDirectoryBefore.CompareNoCase(strDirectoryAfter) != 0 &&
			!DirectoryIsEmpty(strDirectoryBefore))
		{
			CString strParentPath = renamingOperation.GetPathBefore().GetPathRoot();
			BOOST_FOREACH(CString strDirectoryName, renamingOperation.GetPathBefore().GetDirectories())
			{
				// Get the full parent directory's path.
				strParentPath += strDirectoryName.MakeLower();
				strParentPath.AppendChar('\\');

				// Add to the set.
				setDeleteIfEmptyDirectories.insert(strParentPath);
			}
		}
	}

	// Report progress
	OnProgressChanged(stagePreRenaming, 95, 100);

	BOOST_FOREACH(CString& strDirectoryPath, setDeleteIfEmptyDirectories)
	{
		// Remove ONLY a non-empty directory.
		vOrderedOperationList.push_back(shared_ptr<CIOOperation>(
			new CRemoveEmptyDirectoryOperation(strDirectoryPath)
			));
	}

	// Report progress
	OnProgressChanged(stagePreRenaming, 100, 100);

	// Return list of operations.
	return vOrderedOperationList;
}

CRenamingList::CTempDirectoryMap CRenamingList::FindTempDirectories(const TRenamingOperationSet& vRenamingOperations)
{
	CTempDirectoryMap mapTempDirectories;

	// Find the shortest path per drive.
	BOOST_FOREACH(const CRenamingOperation& renamingOperation, vRenamingOperations)
	{
		CString strRoot = renamingOperation.GetPathBefore().GetPathRoot();
		CString strDirectoryPath = renamingOperation.GetPathBefore().GetDirectoryName();
		if (strDirectoryPath[strDirectoryPath.GetLength() - 1] == '\\')
			// Remove trailing backslash (\).
			strDirectoryPath = strDirectoryPath.Left(strDirectoryPath.GetLength() - 1);
		CTempDirectoryMap::iterator iter = mapTempDirectories.find(strRoot);
		if (iter == mapTempDirectories.end())
		{
			// Add that directory path for this drive.
			mapTempDirectories[strRoot] = pair<CString, unsigned>(strDirectoryPath, 0);
		}
		else
		{
			// Find the longest common path.
			while (iter->second.first.CompareNoCase(strDirectoryPath.Left(iter->second.first.GetLength())) != 0) 
			{
				// Move to the parent folder.
				int nPos = iter->second.first.ReverseFind('\\');
				ASSERT(nPos != -1);	// The root should ALWAYS be a match in the worse case.
				iter->second = pair<CString, unsigned>(iter->second.first.Left(nPos), 0);
			}
		}
	}

	// Save the length of the shortest common path.
	for (CTempDirectoryMap::iterator iter=mapTempDirectories.begin(); iter!=mapTempDirectories.end(); ++iter)
		iter->second.second = iter->second.first.GetLength() + 1;	// +1 to include the '\\'.


	Beroux::Math::CRandomMT random;
	random.Randomize();

	// Find a unique temporary folder name per drive.
	for (CTempDirectoryMap::iterator iter=mapTempDirectories.begin(); iter!=mapTempDirectories.end(); ++iter)
	{
GenerateNewTemp:
		// Generate a random temporary directory name.
		CString strTempPath = iter->second.first;
		ASSERT(strTempPath[strTempPath.GetLength() - 1] != '\\');
		strTempPath += _T("\\renameit-");
		for (int i=1; ; ++i)
		{
			for (int n=0; n<4; ++n)
				strTempPath += (char) random.RandomRange('0', '9');
			if (i >= 2)
				break;
			strTempPath += '-';
		}

		// Check that it's not existing.
		if (CPath::PathFileExists(strTempPath))
			goto GenerateNewTemp;

		// Check that the temporary path will not exist after renaming.
		unsigned nTempPathLength = strTempPath.GetLength();
		BOOST_FOREACH(const CRenamingOperation& renamingOperation, vRenamingOperations)
		{
			if (strTempPath.CompareNoCase(renamingOperation.GetPathAfter().GetPath().Left(nTempPathLength)) == 0)
				goto GenerateNewTemp;
		}

		// Accept that directory as temporary path for that drive.
		strTempPath += '\\';
		iter->second.first = strTempPath;
	}

	return mapTempDirectories;
}

int CRenamingList::FindShortestDirectoryPathAfter(TRenamingOperationSet& vRenamingOperations)
{
	int nMinAfterDirIndex = 0;
	int nMinAfterDirLength = vRenamingOperations[nMinAfterDirIndex].pathAfter.GetDirectoryName().GetLength();

	// Find the shortest path.
	const int nRenamingOperationCount = (int)vRenamingOperations.size();
	for (int i=0; i<nRenamingOperationCount; ++i)
	{
		int nDirLength = vRenamingOperations[i].pathAfter.GetDirectoryName().GetLength();

		if (nDirLength < nMinAfterDirLength)
		{
			nMinAfterDirIndex = i;
			nMinAfterDirLength = nDirLength;
		}
	}

	return nMinAfterDirIndex;
}

bool CRenamingList::DirectoryIsEmpty(const CString& strDirectoryPath, CKtmTransaction* pKTM /*= NULL*/)
{
	ASSERT(strDirectoryPath.Right(1) == '\\');

	WIN32_FIND_DATA fd;
	HANDLE hFindFile;

	if (pKTM != NULL)
		hFindFile = pKTM->FindFirstFileEx(strDirectoryPath + _T("*.*"), FindExInfoStandard, &fd, FindExSearchNameMatch, NULL, 0);
	else
		hFindFile = FindFirstFileEx(strDirectoryPath + _T("*.*"), FindExInfoStandard, &fd, FindExSearchNameMatch, NULL, 0);

	if (hFindFile == INVALID_HANDLE_VALUE)
	{
		DWORD dwErrorCode = ::GetLastError();
#ifdef _DEBUG
		// Get error message
		LPTSTR lpMsgBuf = NULL;
		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dwErrorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL );
		CString strErrorMessage = lpMsgBuf;
		LocalFree( lpMsgBuf );
#endif
		ASSERT(dwErrorCode == ERROR_PATH_NOT_FOUND);
		return false;
	}
	else
	{
		do
		{
			if (_tcscmp(fd.cFileName, _T(".")) != 0 &&
				_tcscmp(fd.cFileName, _T("..")) != 0)
			{
				::FindClose(hFindFile);
				return false;
			}
		} while (::FindNextFile(hFindFile, &fd));

		::FindClose(hFindFile);
		return true;
	}
}

CRenamingList::CDirectoryCaseUnifier::CDirectoryCaseUnifier(TRenamingOperationSet& roList)
: m_vRenamingOperations(roList)
, m_nMinAfterDirIndex(FindShortestDirectoryPathAfter(roList))
{
	// Insert the shortest path to the map (to improve a little the speed).
	CString strShortestDirAfter = roList[m_nMinAfterDirIndex].pathAfter.GetDirectoryName();
	m_mapDirsCase.insert( pair<CString, DIR_CASE>(
		strShortestDirAfter.Left(strShortestDirAfter.GetLength() - 1),
		DIR_CASE(m_nMinAfterDirIndex, strShortestDirAfter.GetLength())) );
}

void CRenamingList::CDirectoryCaseUnifier::ProcessOperation(int nIndex)
{
	CPath& pathAfter = m_vRenamingOperations[nIndex].pathAfter;

	// Create a copy of the folder name.
	CString strDirAfter = pathAfter.GetDirectoryName();
	ASSERT(strDirAfter[strDirAfter.GetLength() - 1] == '\\');
	strDirAfter = strDirAfter.Left(strDirAfter.GetLength() - 1);

	// For each parent directory name (starting from GetDirectoryName()).
	const int nMinDirAfterLength = m_vRenamingOperations[m_nMinAfterDirIndex].pathAfter.GetDirectoryName().GetLength() - 1; // note we don't count the last '\' since we remove it.
	while (strDirAfter.GetLength() >= nMinDirAfterLength)
	{
		// If the directory is not in the map.
		map<CString, DIR_CASE, dir_case_compare>::iterator iter = m_mapDirsCase.find(strDirAfter);
		if (iter == m_mapDirsCase.end())
		{
			// Add it to the map.
			m_mapDirsCase.insert( pair<CString, DIR_CASE>(
				strDirAfter,
				DIR_CASE(nIndex, strDirAfter.GetLength())) );
		}
		else
		{
			// Check if the case differ.
			if (iter->first != strDirAfter)
			{
				// If this renaming operation's directory path length > map's length,
				// then it means that that map's case should prevail.
				if (strDirAfter.GetLength() >= iter->second.nMinDirLength)
				{
					// Change the RO's case.
					pathAfter = iter->first + pathAfter.GetPath().Mid(iter->first.GetLength());
				}
				else
				{
					// Change the map's case and update the map's min length.
					// Since changing the case doesn't affect this operation's order in the map, we can const_cast<>.
					const_cast<CString&>(iter->first) = strDirAfter;
					iter->second.nMinDirLength = strDirAfter.GetLength();

					// Change the map's case, all RO's in the map.
					BOOST_FOREACH(int nROIndex, iter->second.vnOperationsIndex)
					{
						m_vRenamingOperations[nROIndex].pathAfter = strDirAfter + m_vRenamingOperations[nROIndex].pathAfter.GetPath().Mid(strDirAfter.GetLength());
					}
				}
			} // end: Check if the case differ.

			// Add the RO's index.
			iter->second.vnOperationsIndex.push_back(nIndex);
		} // end if the directory is in the map.

		// Go to the next parent folder.
		int nPos = strDirAfter.ReverseFind('\\');
		if (nPos == -1)
			break;
		strDirAfter = strDirAfter.Left(nPos);
	} // end while.
}

void CRenamingList::CDirectoryCaseUnifier::AddRenamingOperations(TPreparedIOOperations& ioList) const
{
	// Add renaming operation that would change the case of existing
	// folders if different from the existing one (part of the folder case unification process).
	for (CDirectoryCaseUnifier::TDirCaseMap::const_iterator iter = m_mapDirsCase.begin(); iter != m_mapDirsCase.end(); ++iter)
	{
		// If the folder currently exists with another case,
		if (CPath::PathFileExists(iter->first))
		{
			CString strCurrentPathName = CPath::FindPathCase(iter->first);
			if (strCurrentPathName != iter->first)
			{
				ioList.push_back(shared_ptr<CIOOperation>(
					new CRenameOperation(strCurrentPathName, iter->first)
					));
			}
		}
	}
}

}}}
