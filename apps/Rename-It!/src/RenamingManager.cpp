#include "StdAfx.h"
#include "RenamingManager.h"
#include "OrientedGraph.h"

CRenamingManager::CRenamingManager(void)
{
	m_fOnProgress = boost::bind(&CRenamingManager::DefaultProgressCallback, this, _1, _2, _3);
}

CRenamingManager::CRenamingManager(const CFileList& flBefore, const CFileList& flAfter)
{
	Create(flBefore, flAfter);
}

CRenamingManager::~CRenamingManager(void)
{
}

void CRenamingManager::Create(const CFileList& flBefore, const CFileList& flAfter)
{
	if (flBefore.GetFileCount() != flAfter.GetFileCount())
		throw logic_error("The number of files before and after renaming must be the same.");

	// Combine the two lists into a renaming list.
	m_vRenamingOperations.resize(flBefore.GetFileCount());
	for (int i=0; i<flBefore.GetFileCount(); ++i)
		m_vRenamingOperations[i] = CRenamingOperation(flBefore[i], flAfter[i]);
}

vector<unsigned> CRenamingManager::FindErrors() const
{
	// Declarations.
	const int	nFilesCount = (int) m_vRenamingOperations.size();
	CFileFind	ffFileFind;

	// Create a new error list.
	vector<unsigned> uvErrors;
	uvErrors.resize(nFilesCount);

	// Check for file conflicts.
	for (int i=0; i<nFilesCount; ++i)
	{
		// Report progress
		m_fOnProgress(stageChecking, i*95/nFilesCount, 100);

		// If that file isn't already marked as conflicting with another,
		// test if it's going to conflict with another file.
		if (!(uvErrors[i] & errConflictFlag))
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
						// Yes it is (we are going if it conflicts below)
						bFound = true;
						break;
					}
				if (!bFound)
				{
					// No it is not, so it will conflict with the existing file on the disk.
					uvErrors[i] |= errConflictFlag;
				}
			}

			/* If it doesn't conflict with a file not part of the
			   files to rename, check if it conflicts with files
			   that are going to be renamed... */
			if (!(uvErrors[i] & errConflictFlag))
			{
				// Check if two files are going to have the same new file name
				for (int j=i+1; j<nFilesCount; ++j)
					if (m_vRenamingOperations[i].fnAfter.FSCompare(m_vRenamingOperations[j].fnAfter) == 0)
					{
						// Conflict found
						uvErrors[i] |= errConflictFlag;
						uvErrors[j] |= errConflictFlag;
					}
			}
		}
	} // end: conflict check for-loop

	// Check if file name is valid
	// and Check if file still exists
	const int MAX_INVALID_NAME_LENGTH = (int) lstrlen(_T("CLOCK$"));
	for (int i=0; i<nFilesCount; ++i)
	{
		// Report progress
		m_fOnProgress(stageChecking, 95 + i*5/nFilesCount, 100);

		// Check if the file still exists
		if (!ffFileFind.FindFile( m_vRenamingOperations[i].fnBefore.GetFullPath() ))
		{
			uvErrors[i] |= errMissingFlag;
		}

		CString strFileName = m_vRenamingOperations[i].fnAfter.GetFileName();
		CString strExt = m_vRenamingOperations[i].fnAfter.GetExtension();
		CString strFullFileName = strFileName + strExt;

		// Look for invalid file name (renaming impossible to one of those)
		if ((strFullFileName.IsEmpty())	// Empty filename (including extension)
			|| strFullFileName.FindOneOf(_T("\\/:*?\"<>|")) != -1)	// or, Forbidden characters
		{
			uvErrors[i] |= errInvalidNameFlag;
		}

		if (!(uvErrors[i] & errInvalidNameFlag))
		{
			// Look for some other characters that are usually forbidden (but not explicitely).
			for (int j=0; j<strFullFileName.GetLength(); ++j)
			{
				if (strFullFileName[j] < 0x20)
				{
					uvErrors[i] |= errBadNameFlag;	// Windows XP rejects any file name with a character below 32.
					break;
				}
			}

			// Look for bad file name
			// The following reserved words cannot be used as the name of a file:
			// CON, PRN, AUX, CLOCK$, NUL, COM1, COM2, ..., COM9, LPT1, LPT2, ..., LPT9.
			// Also, reserved words followed by an extension—for example, NUL.tx7—are invalid file names.
			if (strFullFileName.GetLength() > 120			// Over 120 characters (maximum allowed from the Explorer)
				|| strFullFileName.Left(0) == _T(' ')		// Files starting by a space is not good.
				|| strFullFileName.Right(0) == _T(' ')		// Files ending by a space is not good.
				|| strFullFileName.GetAt(0) == _T('.')		// Files starting by a dot not allowed by the Explorer.
				|| (strFileName.GetLength() <= MAX_INVALID_NAME_LENGTH	// Fast remove any file name a bit long...
					&& (   strFileName.CompareNoCase(_T("CON")) == 0	// to quickly detect system reserved file names.
						|| strFileName.CompareNoCase(_T("PRN")) == 0
						|| strFileName.CompareNoCase(_T("AUX")) == 0
						|| strFileName.CompareNoCase(_T("CLOCK$")) == 0 
						|| strFileName.CompareNoCase(_T("NUL")) == 0 
						|| (strFileName.GetLength() == 4
							&& (_tcsncicmp(strFileName, _T("COM"), 3) == 0 || _tcsncicmp(strFileName, _T("LPT"), 3) == 0)
							&& (strFileName[3] >= _T('1') && strFileName[3] <= _T('9'))
						   )
						)
				   )
			   )
			{
				uvErrors[i] |= errBadNameFlag;
			}
		}
	}

	// Post condition.
	ASSERT(uvErrors.size() == uvErrors.size());

	// Report the errors.
	return uvErrors;
}

bool CRenamingManager::PerformRenaming()
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

bool CRenamingManager::RenameFile(int nIndex)
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
