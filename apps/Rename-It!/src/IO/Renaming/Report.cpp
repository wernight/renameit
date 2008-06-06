#include "StdAfx.h"
#include "Report.h"
#include "../../Gui/ReportDlg.h"

namespace Beroux{ namespace IO{ namespace Renaming
{

CReport::CReport(void)
{
}

CReport::~CReport(void)
{
}

bool CReport::ShowReportFixErrors(CRenamingList& renamingList)
{
	Gui::CReportDlg dlgReport(renamingList);
	return dlgReport.DoModal() == IDOK;
}

}}}
