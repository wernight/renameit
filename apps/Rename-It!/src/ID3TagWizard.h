#pragma once

#include "FilterWizard.h"

// CID3TagWizard dialog

namespace Wizard
{
	class CID3TagWizard : public CDialog, public CFilterWizard
	{
		DECLARE_DYNAMIC(CID3TagWizard)

	public:
		CID3TagWizard(CWnd* pParent = NULL);   // standard constructor
		virtual ~CID3TagWizard();

	// CFilterWizard
		shared_ptr<IFilter> Execute(IPreviewFileList& pflSamples);

	// Dialog Data
		enum { IDD = IDD_WIZ_ID3TAG };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()
	};
}
