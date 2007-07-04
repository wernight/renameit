#pragma once
#include "FileName.h"
#include "FileList.h"

/**
 * Handle the renaming of a set of renaming operations.
 * The renaming order is handled internally.
 */
class CRenamingList
{
// Definitions
public:
	enum EErrorFlag
	{
		errMissingFlag		= 0x0001,	// The original file is missing from the storage.
		errConflictFlag		= 0x0002,	// The renaming preview shows a conflict with other files (existing or in the renaming list).
		errBadNameFlag		= 0x0004,	// The previewed new file name should be avoided.
		errInvalidNameFlag	= 0x0008,	// The previewed new file name is invalid.
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

		CRenamingOperation(const CFileName& before, const CFileName& after) :
			fnBefore(before),
			fnAfter(after)
		{
		}
			
		CFileName	fnBefore;
		CFileName	fnAfter;
	};

// Construction
	CRenamingList(void);
	CRenamingList(const CFileList& flBefore, const CFileList& flAfter);
	~CRenamingList(void);

	void Create(const CFileList& flBefore, const CFileList& flAfter);

// Methods to access and modify the list
	/**
	 * @return Number of renaming operations to perform.
	 */
	int GetCount() const {
		return (int) m_vRenamingOperations.size();
	}

	const CRenamingOperation& GetRenamingOperation(int nIndex) const {
		return m_vRenamingOperations.at(nIndex);
	}

	void AddRenamingOperation(const CRenamingOperation& roRenaming) {
		m_vRenamingOperations.push_back(roRenaming);
	}

	void AddRenamingOperation(const CFileName& fnBefore, const CFileName& fnAfter) {
		AddRenamingOperation( CRenamingOperation(fnBefore, fnAfter) );
	}

	void SetRenamingOperation(int nIndex, const CRenamingOperation& roRenaming) {
		m_vRenamingOperations.at(nIndex) = roRenaming;
	}

	void RemoveRenamingOperation(int nIndex) {
		if (nIndex < 0 || nIndex >= (int) m_vRenamingOperations.size())
			throw out_of_range("Index is out of range.");
		m_vRenamingOperations.erase(m_vRenamingOperations.begin() + nIndex);
	}

	const CRenamingOperation& operator[](int nIndex) const {
		return m_vRenamingOperations.at(nIndex);
	}

// Attributes
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

// Operations

	/**
	 * Search for possible renaming problems before doing the renaming.
	 * @return A list of errors that is of same size as the files list,
	 *         with for each file (of the same index) the value is a set of errors flags of EErrorFlag.
	 */
	vector<unsigned> FindErrors() const;

	/**
	 * Perform the renaming of all the file.
	 */
	bool PerformRenaming();

// Implementation
private:
	bool RenameFile(int nIndex);

	// Default progress callback that does nothing.
	void DefaultProgressCallback(EStage nStage, int nDone, int nTotal) {}

	vector<CRenamingOperation>	m_vRenamingOperations;

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
