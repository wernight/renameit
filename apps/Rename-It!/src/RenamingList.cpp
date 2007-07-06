#include "StdAfx.h"
#include "RenamingList.h"
#include "OrientedGraph.h"
#include <math.h>
#include "../resource.h"

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

bool CRenamingList::Check()
{
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

	// Check for file conflicts.
	for (int i=0; i<nFilesCount; ++i)
	{
		// Report progress
		const float fBias = 100.0f;
		m_fOnProgress(stageChecking, (int)((log(i + fBias) - log(fBias))*100.0f/(log(nFilesCount + fBias) - log(fBias)) + 0.5f), 100);

		// If that file isn't already marked as conflicting with another,
		// test if it's going to conflict with another file.
		if (m_vProblems[i].nErrorCode != errConflict)
		{
			/* Detect if it's going to be renamed to a file that already exists
			   but that is not part of the files to rename... */
			if (ffFileFind.FindFile(m_vRenamingOperations[i].fnAfter.GetFullPath()) )	// But the destination exists on the disk
			{
				// Search if the new file name is in the part of the file to rename list.
				bool bFound = false;
				for (int j=0; j<nFilesCount; ++j)
					if (m_vRenamingOperations[j].fnBefore.FSCompare(m_vRenamingOperations[i].fnAfter) == 0)
					{
						// Yes it is (we are going if it conflicts after renaming below)
						bFound = true;
						break;
					}
				if (!bFound)
				{
					// No it is not, so it will conflict with the existing file on the disk.
					CString strErrorMsg;
					strErrorMsg.LoadString(IDS_CONFLICT_WITH_EXISTING);
					SetProblem(i, errConflict, strErrorMsg);
				}
			}

			/* If it doesn't conflict with a file not part of the
			   files to rename, check if it conflicts with files
			   that are going to be renamed... */
			if (m_vProblems[i].nErrorCode != errConflict)
			{
				// Check if two files are going to have the same new file name
				for (int j=i+1; j<nFilesCount; ++j)
					if (m_vRenamingOperations[i].fnAfter.FSCompare(m_vRenamingOperations[j].fnAfter) == 0)
					{
						// Conflict found: Two files are going to be renamed to the same new file name.
						CString strErrorMsg;

						AfxFormatString1(strErrorMsg, IDS_CONFLICT_SAME_AFTER, m_vRenamingOperations[j].fnBefore.GetFullPath());
						SetProblem(i, errConflict, strErrorMsg);

						AfxFormatString1(strErrorMsg, IDS_CONFLICT_SAME_AFTER, m_vRenamingOperations[i].fnBefore.GetFullPath());
						SetProblem(j, errConflict, strErrorMsg);
					}
			}
		}
	} // end: conflict check for-loop

	// Check if file name is valid
	// and Check if file still exists
	for (int i=0; i<nFilesCount; ++i)
	{
		// Report progress
		m_fOnProgress(stageChecking, 95 + i*5/nFilesCount, 100);

		// Check if the file still exists
		if (!ffFileFind.FindFile( m_vRenamingOperations[i].fnBefore.GetFullPath() ))
		{
			CString strErrorMsg;
			strErrorMsg.LoadString(IDS_REMOVED_FROM_DISK);
			SetProblem(i, errFileMissing, strErrorMsg);
		}

		CString strFilename = m_vRenamingOperations[i].fnAfter.GetFileName();
		CString strFullFilename = strFilename + m_vRenamingOperations[i].fnAfter.GetExtension();

		// Look for invalid file name (renaming impossible to one of those)
		if ((strFullFilename.IsEmpty())	// Empty filename (including extension)
			|| strFullFilename.FindOneOf(_T("\\/:*?\"<>|")) != -1)	// or, Forbidden characters
		{
			CString strErrorMsg;
			strErrorMsg.LoadString(IDS_INVALID_FILE_NAME);
			SetProblem(i, errInvalidName, strErrorMsg);
		}

		if (m_vProblems[i].nErrorCode != errInvalidName)
		{
			// Over 120 characters (maximum allowed from the Explorer)
			if (strFullFilename.GetLength() > 120)
			{
				CString strErrorMsg;
				strErrorMsg.LoadString(IDS_RISKY_FNAME_TOO_LONG);
				SetProblem(i, errRiskyName, strErrorMsg);
			}

			// Look for some other characters that are usually forbidden (but not explicitely).
			for (int j=0; j<strFullFilename.GetLength(); ++j)
			{
				if (strFullFilename[j] < 0x20)
				{
					// Windows XP rejects any file name with a character below 32.
					CString strErrorMsg;
					strErrorMsg.LoadString(IDS_RISKY_FNAME_CHARS);
					SetProblem(i, errRiskyName, strErrorMsg);
					break;
				}
			}

			// The following reserved words cannot be used as the name of a file:
			// CON, PRN, AUX, CLOCK$, NUL, COM1, COM2, ..., COM9, LPT1, LPT2, ..., LPT9.
			// Also, reserved words followed by an extension—for example, NUL.tx7—are invalid file names.
			const int MAX_INVALID_NAME_LENGTH = 6;	// = strlen("CLOCK$")
			if ((strFilename.GetLength() <= MAX_INVALID_NAME_LENGTH	// Quickly remove any file name that can't be invalid...
					&& (   strFilename.CompareNoCase(_T("CON")) == 0	// to quickly detect system reserved file names.
						|| strFilename.CompareNoCase(_T("PRN")) == 0
						|| strFilename.CompareNoCase(_T("AUX")) == 0
						|| strFilename.CompareNoCase(_T("CLOCK$")) == 0 
						|| strFilename.CompareNoCase(_T("NUL")) == 0 
						|| (strFilename.GetLength() == 4
							&& (_tcsncicmp(strFilename, _T("COM"), 3) == 0 || _tcsncicmp(strFilename, _T("LPT"), 3) == 0)
							&& (strFilename[3] >= _T('1') && strFilename[3] <= _T('9'))
						   )
						)
				   )
			   )
			{
				CString strErrorMsg;
				strErrorMsg.LoadString(IDS_RISKY_FNAME_RESERVED);
				SetProblem(i, errRiskyName, strErrorMsg);
			}

			// Starting or ending by one or more spaces.
			if (strFullFilename.GetAt(0) == _T(' ')			// Files starting by a space is not good.
				|| strFullFilename.Right(0) == _T(' '))		// Files ending by a space is not good.
			{
				CString strErrorMsg;
				strErrorMsg.LoadString(IDS_RISKY_FNAME_TRIM_SPACES);
				SetProblem(i, errRiskyName, strErrorMsg);
			}
		}
	}

	// Post condition.
	ASSERT(m_vProblems.size() == m_vRenamingOperations.size());

	// Report the errors.
	return m_nErrors == 0 && m_nWarnings == 0;
}

bool CRenamingList::PerformRenaming()
{
	// Avoid possible strange behaviours for empty lists.
	if (m_vRenamingOperations.size() == 0)
		return true;

	// Create an oriented graph of conflicting renamings:
	Beroux::Math::OrientedGraph	graph;
	{
		const int nFilesCount = (int) m_vRenamingOperations.size();

		// Create a first version of an oriented graph of conflicting renamings:
		// Node(i) -edge-to-> Node(j)   <=>   File(i).originalFileName == File(j).newFileName (compare no case)
		//                              <=>   File(i) must be renamed before File(j)
		for (int i=0; i<nFilesCount; ++i)
			graph.AddNode(i);
		for (int i=0; i<nFilesCount; ++i)
		{
			// Report progress
			m_fOnProgress(stagePreRenaming, i*95/nFilesCount, 100);

			for (int j=0; j<nFilesCount; ++j)
				if (i != j && m_vRenamingOperations[i].fnBefore.FSCompare(m_vRenamingOperations[j].fnAfter) == 0)
				{
					graph[i].AddSuccessor(j);
					
					// Assertion: There can be at most one successor,
					// else it means that two files (or more) will have the same destination name.
					break;	// A jump to speed-up the loop given the above assertion.
				}
		}

		// Find and remove cycles by adding new nodes.
		for (int i=0; i<nFilesCount; ++i)
		{
			// Report progress
			m_fOnProgress(stagePreRenaming, 95 + i*5/nFilesCount, 100);

			if (graph[i].HasSuccessor() && graph[i].HasAntecedent())
			{
				for (int j=graph[i].GetSuccessor(0); graph[j].HasSuccessor(); j=graph[j].GetSuccessor(0))
					if (i == j)	// Cycle found: Node(i) --> Node(i).successor --> ... --> Node(i).
					{
						// Add a new node, to make a temporary rename that solves the problem.
						// It is fixes the problem because the temporary file doesn't exist yet,
						// so it doesn't conflict.

						// Originally rename A -> B
						CFileName fnFinal = m_vRenamingOperations[graph[i].GetSuccessor(0)].fnAfter;

						CString strRandomName = _T("~");
						for (int k=0; k<15; ++k)
							strRandomName += (char)( rand()%('z'-'a') + 'a' );
						CFileName fnTemp = fnFinal.GetFullPath() + strRandomName;

						// Rename A -> TMP
						m_vRenamingOperations[graph[i].GetSuccessor(0)].fnAfter = fnTemp;

						// Then rename TMP -> B
						AddRenamingOperation( CRenamingOperation(fnTemp, fnFinal) );

						// Add Node(i) --> Node(TMP), because File(i) must be renamed before File(TMP).
						graph.AddNode((int)m_vRenamingOperations.size() - 1);
						graph[i].RemoveSuccessor(0);
						graph[i].AddSuccessor((int)m_vRenamingOperations.size() - 1);

						break;
					}
			} // end if cycle detection.
		}
	}

	// Rename files in topological order.
	int nDone = 0;
	int nTotal = (int) m_vRenamingOperations.size();
	m_fOnProgress(stageRenaming, 0, nTotal);	// Inform we start renaming.
	bool bError = false;
	for (int i=0; i<nTotal; ++i)
		if (!graph[i].HasAntecedent())
		{
			int j = i;
			while (true)
			{
				// Rename file.
				bError |= !RenameFile(j);

				// Report progress
				m_fOnProgress(stageRenaming, ++nDone, nTotal);

				// Rename its successors.
				if (graph[j].HasSuccessor())
					j = graph[j].GetSuccessor(0);
				else
					break;
			}
		}

	return !bError;
}

bool CRenamingList::RenameFile(int nIndex)
{
	ASSERT(m_vRenamingOperations[nIndex].fnBefore.GetDrive() == m_vRenamingOperations[nIndex].fnAfter.GetDrive());

	// Rename file
	if (MoveFile(m_vRenamingOperations[nIndex].fnBefore.GetFullPath(), m_vRenamingOperations[nIndex].fnAfter.GetFullPath()) == 0)
	{
		if (!m_fOnRenamed.empty())
			m_fOnRenamed(nIndex, GetLastError());
		return false;
	}

	if (!m_fOnRenamed.empty())
		m_fOnRenamed(nIndex, 0);
	return true;
}
