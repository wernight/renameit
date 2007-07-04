#pragma once

#include "FilterWizard.h"

// CAppendWizard dialog

namespace Wizard
{
	class CAppendWizard : public CDialog, public CFilterWizard
	{
		DECLARE_DYNAMIC(CAppendWizard)

	public:
		CAppendWizard(CWnd* pParent = NULL);   // standard constructor
		virtual ~CAppendWizard();

	// CFilterWizard
		shared_ptr<IFilter> Execute(IPreviewFileList& pflSamples);

	// Dialog Data
		enum { IDD = IDD_WIZ_APPEND };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()
	};
}
