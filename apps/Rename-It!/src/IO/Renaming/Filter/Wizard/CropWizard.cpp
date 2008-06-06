#include "StdAfx.h"
#include "../../../../../resource.h"
#include "CropWizard.h"

namespace Beroux{ namespace IO{ namespace Renaming{ namespace Filter{ namespace Wizard
{
	// CCropWizard dialog

	IMPLEMENT_DYNAMIC(CCropWizard, CDialog)

	CCropWizard::CCropWizard(CWnd* pParent /*=NULL*/)
		: CDialog(CCropWizard::IDD, pParent)
	{

	}

	CCropWizard::~CCropWizard()
	{
	}

	shared_ptr<IFilter> CCropWizard::Execute(IPreviewFileList& pflSamples)
	{
		if (DoModal() != IDOK)
			return shared_ptr<IFilter>();

		ASSERT(false);
		return shared_ptr<IFilter>();
	}

	void CCropWizard::DoDataExchange(CDataExchange* pDX)
	{
		CDialog::DoDataExchange(pDX);
	}


	BEGIN_MESSAGE_MAP(CCropWizard, CDialog)
	END_MESSAGE_MAP()


	// CCropWizard message handlers
}}}}}
