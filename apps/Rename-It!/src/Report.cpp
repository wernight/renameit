#include "StdAfx.h"
#include "Report.h"
#include "ReportDlg.h"

CReport::CReport(void)
{
}

CReport::~CReport(void)
{
}

bool CReport::ShowReportFixErrors(CRenamingList& renamingList, vector<unsigned>& uvErrorFlag)
{
	CReportDlg dlgReport(renamingList, uvErrorFlag);
	return dlgReport.DoModal() == IDOK;
}
