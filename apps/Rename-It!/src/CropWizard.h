#pragma once

#include "FilterWizard.h"

// CCropWizard dialog

namespace Beroux{ namespace IO{ namespace Renaming{ namespace Filter{ namespace Wizard
{
	class CCropWizard : public CDialog, public CFilterWizard
	{
		DECLARE_DYNAMIC(CCropWizard)

	public:
		CCropWizard(CWnd* pParent = NULL);   // standard constructor
		virtual ~CCropWizard();

	// CFilterWizard
		shared_ptr<IFilter> Execute(IPreviewFileList& pflSamples);

	// Dialog Data
		enum { IDD = IDD_WIZ_CROP };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()
	};
}}}}}
