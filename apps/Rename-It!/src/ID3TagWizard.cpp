#include "StdAfx.h"
#include "../resource.h"
#include "ID3TagWizard.h"

namespace Beroux{ namespace IO{ namespace Renaming{ namespace Filter{ namespace Wizard
{
	// CID3TagWizard dialog

	IMPLEMENT_DYNAMIC(CID3TagWizard, CDialog)

	CID3TagWizard::CID3TagWizard(CWnd* pParent /*=NULL*/)
		: CDialog(CID3TagWizard::IDD, pParent)
	{

	}

	CID3TagWizard::~CID3TagWizard()
	{
	}

	shared_ptr<IFilter> CID3TagWizard::Execute(IPreviewFileList& pflSamples)
	{
		if (DoModal() != IDOK)
			return shared_ptr<IFilter>();

		ASSERT(false);
		return shared_ptr<IFilter>();
	}

	void CID3TagWizard::DoDataExchange(CDataExchange* pDX)
	{
		CDialog::DoDataExchange(pDX);
	}


	BEGIN_MESSAGE_MAP(CID3TagWizard, CDialog)
	END_MESSAGE_MAP()


	// CID3TagWizard message handlers
}}}}}
