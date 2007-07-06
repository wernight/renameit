#include "StdAfx.h"
#include "Report.h"
#include "ReportDlg.h"

CReport::CReport(void)
{
}

CReport::~CReport(void)
{
}

bool CReport::ShowReportFixErrors(CRenamingList& renamingList)
{
	CReportDlg dlgReport(renamingList);
	return dlgReport.DoModal() == IDOK;
}
