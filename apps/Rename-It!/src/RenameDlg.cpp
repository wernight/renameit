// RenameDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RenameIt.h"
#include "RenameDlg.h"


// CRenameDlg dialog

IMPLEMENT_DYNAMIC(CRenameDlg, CDialog)
CRenameDlg::CRenameDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRenameDlg::IDD, pParent)
	, m_strOriginalFileName(_T(""))
	, m_strNewFileName(_T(""))
{
}

CRenameDlg::~CRenameDlg()
{
}

void CRenameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_ORIGINAL_EDIT, m_strOriginalFileName);
	DDX_Text(pDX, IDC_NEW_EDIT, m_strNewFileName);
}


BEGIN_MESSAGE_MAP(CRenameDlg, CDialog)
END_MESSAGE_MAP()


// CRenameDlg message handlers

void CRenameDlg::OnOK()
{
	if (!UpdateData(TRUE))
		return;
	m_strNewFileName.TrimLeft();
	UpdateData(FALSE);
	CDialog::OnOK();
}
