#pragma once
#include "Path.h"
#include "FileList.h"

/**
 * Handle the renaming of a set of renaming operations.
 *
 * Let note A->B the renaming operation of a file named "A" to "B"; here are some features supported:
 * - The renaming order is handled internally: A->B, B->C will rename B first and then A.
 * - Temporary file names for cycles: A->B, B->A will rename by using a temporary file name A->T, B->A, T->B.
 * 
 * Can detect renaming problem before renaming files, including:
 * - System forbidden file names and most dangerous file names[1]
 * - When two files are going to have to same file name
 * - When files have been removed
 *
 * [1]: Note that "C:\program.txt" when "C:\Program Files\" exits is not checked by should give a warning
 *      See http://www.cs.rpi.edu/courses/fall01/os/CreateProcess.html
 *
 * @todo Check for more dangerous file names and check if files are opened by another process.
 */
class CRenamingList
{
// Definitions
public:
	enum EErrorLevel
	{
		levelNone,		// No error detected.
		levelWarning,	// This is a warning, so the renaming is still possible.
		levelError,		// This is an error, so the renaming would be impossible.
	};

	/**
	 * The higher, the higher the error level.
	 */
	enum EErrorCode
	{
		errNoError,				// No error.
		errRiskyFileName,		// The new file name should be avoided.
		errRiskyDirectoryName,	// The new directory name should be avoided.
		errLonguerThanMaxPath,	// The full path length is >= than MAX_PATH=260 chars long (it may not work properly with some applications).
		errConflict,			// The renaming preview shows a conflict with other files (existing or in the renaming list).
		errInvalidFileName,		// The new file name is invalid.
		errInvalidDirectoryName,// The new directory name is invalid.
		errBackslashMissing,	// The path must start and end by a backslash (\).
		errFileMissing,			// The original file is missing from the storage.
		errCount
	};

	enum EStage
	{
		stageChecking,		// It's looking for possibly problems.
		stagePreRenaming,	// It's preparing the renaming (but no renaming done at this stage).
		stageRenaming,		// It's renaming the files.
	};
	enum {
		stageCount = 3		// Number of possible stages.
	};

	struct CRenamingOperation
	{
		CRenamingOperation()
		{
		}

		CRenamingOperation(const CPath& before, const CPath& after) :
			pathBefore(before),
			pathAfter(after)
		{
		}
			
		CPath	pathBefore;
		CPath	pathAfter;
	};

	struct COperationProblem
	{
		COperationProblem()
			: nErrorLevel(levelNone)
			, nErrorCode(errNoError)
		{
		}

		EErrorLevel nErrorLevel;	// How serious is the problem (if there is any).
		EErrorCode nErrorCode;		// A code representing the general error reason.
		CString strMessage;			// A short message describing the problem's reasons (in the user language).
	};

// Construction
	CRenamingList(void);
	CRenamingList(const CFileList& flBefore, const CFileList& flAfter);
	~CRenamingList(void);

	void Create(const CFileList& flBefore, const CFileList& flAfter);

// Attributes
	/**
	 * @return Number of renaming operations to perform.
	 */
	int GetCount() const { return (int) m_vRenamingOperations.size(); }

	int GetWarningCount() const { return m_nWarnings; }

	int GetErrorCount() const { return m_nErrors; }

	/**
	 * See the declaration of m_fOnRenamed for more details about
	 * the callback function arguments.
	 * You can create a method void MyCallback(int nIndex, DWORD dwErrorCode);
	 * and then use boost::bind(&MyClass::MyCallBack, &myClassInstance, _1, _2);
     */
	void SetRenamedCallback(const boost::function<void (int nIndex, DWORD dwErrorCode)>& fOnRenamed) {
		m_fOnRenamed = fOnRenamed;
	}

	/**
	 * Used to track progress of renaming and other long operations.
	 * See the declaration of m_fOnProgress for more details about
	 * the callback function arguments.
     */
	void SetProgressCallback(const boost::function<void (EStage nStage, int nDone, int nTotal)>& fOnRenameProgress) {
		m_fOnProgress = fOnRenameProgress;
	}

	/**
	 * Return the problems found for a specific operation.
	 * Call Check() to update the problems' list.
	 */
	inline const COperationProblem& GetOperationProblem(int nIndex)
	{
		return m_vProblems[nIndex];
	}

// Methods to access and modify the list
	const CRenamingOperation& GetRenamingOperation(int nIndex) const {
		return m_vRenamingOperations.at(nIndex);
	}

	void AddRenamingOperation(const CRenamingOperation& roRenaming) {
		m_vRenamingOperations.push_back(roRenaming);
		m_vProblems.push_back( COperationProblem() );
	}

	void AddRenamingOperation(const CPath& pathBefore, const CPath& pathAfter) {
		AddRenamingOperation( CRenamingOperation(pathBefore, pathAfter) );
	}

	void SetRenamingOperation(int nIndex, const CRenamingOperation& roRenaming) {
		m_vRenamingOperations.at(nIndex) = roRenaming;
	}

	void RemoveRenamingOperation(int nIndex) {
		if (nIndex < 0 || nIndex >= (int) m_vRenamingOperations.size())
			throw out_of_range("Index is out of range.");

		switch (m_vProblems[nIndex].nErrorLevel)
		{
		case levelWarning:	--m_nWarnings; break;
		case levelError:	--m_nErrors; break;
		}

		m_vRenamingOperations.erase(m_vRenamingOperations.begin() + nIndex);
		m_vProblems.erase(m_vProblems.begin() + nIndex);
		ASSERT(m_vRenamingOperations.size() == m_vProblems.size());
	}

	const CRenamingOperation& operator[](int nIndex) const {
		return m_vRenamingOperations.at(nIndex);
	}

	/**
	 * Tells wether it's using the Vista KTM (Kernel Transaction Manager) to make
	 * the renaming transactions.
	 */
	bool IsUsingKtm() const;

// Operations
	/**
	 * Search for possible renaming problems before doing the renaming.
	 * After calling Check, you can GetOperationProblems().
	 * @return True when no problems (neither errors or warnings) have been detected.
	 */
	bool Check();

	/**
	 * Perform the renaming of all the file.
	 * When KTM is used and the operation fails, no file is renamed.
	 * @return True on success, false if one or more files couldn't be renamed.
	 */
	bool PerformRenaming();

// Implementation
private:
	// A comparaison used to order such a set such as the longuest path come first.
	template <class _Tp>
	struct path_compare : public binary_function<_Tp, _Tp, bool>
	{
		bool operator()(const _Tp& __x, const _Tp& __y) const
		{
			if (__x.GetLength() != __y.GetLength())
				return __x.GetLength() > __y.GetLength();
			else
				return __x < __y;
		}
	};

	static COperationProblem CheckName(const CString& strName, const CString& strNameWithoutExtension, bool bIsFileName);

	void SetProblem(int nOperationIndex, EErrorCode nErrorCode, CString strMessage)
	{
		vector<COperationProblem>::iterator iter = m_vProblems.begin() + nOperationIndex;

		// Errors should always go up and keep the highest one only.
		if (nErrorCode > iter->nErrorCode)
		{
			// Find the error level from the error code.
			EErrorLevel nLevel;
			BOOST_STATIC_ASSERT(errCount == 9);
			switch (nErrorCode)
			{
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
			iter->nErrorLevel = nLevel;
			iter->nErrorCode = nErrorCode;
			iter->strMessage = strMessage;
			ASSERT((iter->nErrorLevel==levelNone) ^ (iter->nErrorCode!=errNoError)); // no error <=> no error code, an error <=> error code set
			ASSERT((iter->nErrorLevel==levelNone) ^ !iter->strMessage.IsEmpty());	// no error <=> no error message, an error <=> error message set
		}
	}

	// Default progress callback that does nothing.
	void DefaultProgressCallback(EStage nStage, int nDone, int nTotal) {}

	vector<CRenamingOperation> m_vRenamingOperations;

	vector<COperationProblem> m_vProblems;

	int m_nWarnings;

	int m_nErrors;

	/**
	 * A callback function called after a file was renamed (also when it could not be renamed).
	 * @param[in] nIndex		Index of the operation.
	 * @param[in] dwErrorCode	0 on success or a Windows error code returned by GetLastError().
	 */
	boost::function<void (int nIndex, DWORD dwErrorCode)> m_fOnRenamed;

	/**
	 * A callback function called during the renaming to indicate progress.
	 * @param[in] nStage		The current long operation being performed.
	 * @param[in] nDone			The number of files renamed (with or without problem).
	 * @param[in] nTotal		The total number of files to be rename.
	 */
	boost::function<void (EStage nStage, int nDone, int nTotal)> m_fOnProgress;
};
