#pragma once

#include "SearchReplaceFilter.h"
#include "IPreviewFileList.h"

/**
 * Base class for the renaming wizards.
 */
class CFilterWizard
{
public:
	CFilterWizard(void);
	virtual ~CFilterWizard(void);

// Operations
	/**
	 * Display the wizard.
	 * @param pflSamples A list of sample file names that can be used to preview the effect of the filter.
	 * @return The generated CSearchReplaceFilter or null if the user cancelled or in case of an error.
	 */
	virtual shared_ptr<IFilter> Execute(IPreviewFileList& pflSamples) = 0;
};
