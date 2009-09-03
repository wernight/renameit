#pragma once

#include "FilterWizard.h"

namespace Beroux{ namespace IO{ namespace Renaming{ namespace Filter{ namespace Wizard
{
	// CEnumWizard dialog
	class CEnumWizard : public CDialog, public CFilterWizard
	{
		DECLARE_DYNAMIC(CEnumWizard)

	public:
		CEnumWizard(CWnd* pParent = NULL);   // standard constructor
		virtual ~CEnumWizard();

	// CFilterWizard
		shared_ptr<IFilter> Execute(IPreviewFileList& pflSamples);

	// Dialog Data
		enum { IDD = IDD_WIZ_ENUM };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()
	};
}}}}}
