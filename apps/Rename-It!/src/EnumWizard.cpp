#include "StdAfx.h"
#include "../resource.h"
#include "EnumWizard.h"

namespace Wizard
{
	// CEnumWizard dialog

	IMPLEMENT_DYNAMIC(CEnumWizard, CDialog)

	CEnumWizard::CEnumWizard(CWnd* pParent /*=NULL*/)
		: CDialog(CEnumWizard::IDD, pParent)
	{

	}

	CEnumWizard::~CEnumWizard()
	{
	}

	shared_ptr<IFilter> CEnumWizard::Execute(IPreviewFileList& pflSamples)
	{
		if (DoModal() != IDOK)
			return shared_ptr<IFilter>();

		ASSERT(false);
		return shared_ptr<IFilter>();
	}

	void CEnumWizard::DoDataExchange(CDataExchange* pDX)
	{
		CDialog::DoDataExchange(pDX);
	}


	BEGIN_MESSAGE_MAP(CEnumWizard, CDialog)
	END_MESSAGE_MAP()


	// CEnumWizard message handlers
}
