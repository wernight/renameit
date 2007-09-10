// NotAddedFilesDlg.cpp : fichier d'implémentation
//

#include "stdafx.h"
#include "RenameIt.h"
#include "NotAddedFilesDlg.h"
#include ".\notaddedfilesdlg.h"


// Boîte de dialogue CNotAddedFilesDlg

IMPLEMENT_DYNAMIC(CNotAddedFilesDlg, CDialog)
CNotAddedFilesDlg::CNotAddedFilesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNotAddedFilesDlg::IDD, pParent)
{
}

CNotAddedFilesDlg::~CNotAddedFilesDlg()
{
}

void CNotAddedFilesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_ctlFilesList);
}


BEGIN_MESSAGE_MAP(CNotAddedFilesDlg, CDialog)
END_MESSAGE_MAP()


// Gestionnaires de messages CNotAddedFilesDlg

BOOL CNotAddedFilesDlg::OnInitDialog()
{
	CString		str;
	POSITION	posFile, posError;
	int			nIndex;

	CDialog::OnInitDialog();

	// Set columns
	str.LoadString(IDS_COLUMN_FILE_NAME);
	m_ctlFilesList.InsertColumn(0, str, NULL, 250);
	str.LoadString(IDS_COLUMN_ERROR_MESSAGE);
	m_ctlFilesList.InsertColumn(1, str, NULL, 115);

	// Add file names & error messages
	posFile = m_slFileNames.GetHeadPosition();
	posError = m_slErrorMessages.GetHeadPosition();
	while (posFile != NULL)
	{
		if ((nIndex = m_ctlFilesList.InsertItem(0, m_slFileNames.GetNext(posFile))) == -1)
		{
			MsgBoxUnhandledError(__FILE__, __LINE__);
			m_slErrorMessages.GetNext(posError);
		}
		else
			m_ctlFilesList.SetItemText(nIndex, 1, m_slErrorMessages.GetNext(posError));
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION : les pages de propriétés OCX devraient retourner FALSE
}

bool CNotAddedFilesDlg::AddFile(LPCTSTR szFileName, LPCTSTR szErrorMessage)
{
	m_slFileNames.AddTail(szFileName);
	m_slErrorMessages.AddTail(szErrorMessage);

	return true;
}

bool CNotAddedFilesDlg::AddFile(LPCTSTR szFileName, UINT nResourceString)
{
	CString strMessage;
	if (!strMessage.LoadString(nResourceString))
	{
		ASSERT(false);
		return AddFile(szFileName, _T("???"));
	}
	else
		return AddFile(szFileName, strMessage);
}

// Return true if the dialog has file names and error messages.
bool CNotAddedFilesDlg::HasErrors()
{
	return m_slFileNames.IsEmpty() != 0;
}

// Clear the file names and error messages list.
void CNotAddedFilesDlg::ClearList()
{
	m_slFileNames.RemoveAll();
	m_slErrorMessages.RemoveAll();
}