#pragma once

#include "RenamingManager.h"

/**
 * Report errors and ask the user to fix them.
 */
class CReport
{
// Construction
public:
	CReport(void);
	~CReport(void);

// Operations
	/**
	 * Search for possible renaming errors and if some errors have been detected,
	 * ask the user to fix them.
	 * @param[in,out] renamingList	A renaming list.
	 * @return True if there are no more errors (meaning there was none or they have been fixed).
	 */
	bool SearchAndFixErrors(CRenamingManager& renamingList);
};
