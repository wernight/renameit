#pragma once

#include "RenamingList.h"

namespace Beroux{ namespace IO{ namespace Renaming
{
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
		 * Display the renaming errors with their meaning and ask the user to fix them.
		 * @param[in,out] renamingList	A renaming list.
		 * @return True if there are no more errors (meaning there was none or they have been fixed).
		 */
		bool ShowReportFixErrors(CRenamingList& renamingList);
	};
}}}
