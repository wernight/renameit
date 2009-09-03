#include "StdAfx.h"
#include "../../../../../resource.h"
#include "AppendWizard.h"

namespace Beroux{ namespace IO{ namespace Renaming{ namespace Filter{ namespace Wizard
{

// CAppendWizard dialog

IMPLEMENT_DYNAMIC(CAppendWizard, CDialog)

CAppendWizard::CAppendWizard(CWnd* pParent /*=NULL*/)
	: CDialog(CAppendWizard::IDD, pParent)
{

}

CAppendWizard::~CAppendWizard()
{
}

shared_ptr<IFilter> CAppendWizard::Execute(IPreviewFileList& pflSamples)
{
	if (DoModal() != IDOK)
		return shared_ptr<IFilter>();

	ASSERT(false);
	return shared_ptr<IFilter>();
}

void CAppendWizard::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAppendWizard, CDialog)
END_MESSAGE_MAP()


// CAppendWizard message handlers

}}}}}
