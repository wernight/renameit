#pragma once
#include "../SearchReplaceFilter.h"
#include "../IPreviewFileList.h"

namespace Beroux{ namespace IO{ namespace Renaming{ namespace Filter{ namespace Wizard
{
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
		 * @return The generated CSearchReplaceFilter or null if the user canceled or in case of an error.
		 */
		virtual shared_ptr<Beroux::IO::Renaming::Filter::IFilter> Execute(IPreviewFileList& pflSamples) = 0;
	};
}}}}}
