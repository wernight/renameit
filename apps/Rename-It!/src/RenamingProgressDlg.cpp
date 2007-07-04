// ProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RenamingProgressDlg.h"


// CProgressDlg dialog

IMPLEMENT_DYNAMIC(CProgressDlg, CDialog)

CProgressDlg::CProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProgressDlg::IDD, pParent)
	, m_nStage((CRenamingList::EStage) 0)
	, m_bEnableCancel(false)
	, m_nDone(0)
	, m_nTotal(1)
	, m_bDone(false)
{
}

CProgressDlg::~CProgressDlg()
{
}

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_STATIC, m_ctlProgressStatic);
	DDX_Control(pDX, IDC_PROGRESS1, m_ctlProgress[0]);
	DDX_Control(pDX, IDC_PROGRESS2, m_ctlProgress[1]);
	DDX_Control(pDX, IDC_PROGRESS3, m_ctlProgress[2]);
}


BEGIN_MESSAGE_MAP(CProgressDlg, CDialog)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CProgressDlg message handlers

void CProgressDlg::OnCancel()
{
	if (m_bEnableCancel)
		CDialog::OnCancel();
}

void CProgressDlg::OnOK()
{
	OnCancel();
}

BOOL CProgressDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_bRefreshEverything = true;

	// Use default title?
	if (m_strDialogTitle.IsEmpty())
		GetWindowText(m_strDialogTitle);

	// Use default caption?
	if (m_strProgressCaption.IsEmpty())
		m_ctlProgressStatic.GetWindowText(m_strProgressCaption);

	// Create a timer that will display the current progress.
	SetTimer(0, 200, NULL);

	// Update the progress before displaying the window.
	UpdateProgress();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CProgressDlg::OnDestroy()
{
	// Destroy the progress update timer.
	KillTimer(0);
	m_bDone = false;

	CDialog::OnDestroy();
}

void CProgressDlg::OnTimer(UINT_PTR nIDEvent)
{
	// Update the displayed progress.
	UpdateProgress();

	// When complete, exit.
	if (m_bDone)
		CDialog::OnOK();

	CDialog::OnTimer(nIDEvent);
}

void CProgressDlg::UpdateProgress()
{
	CString strText;

	// Update all the window.
	if (m_bRefreshEverything)
	{
		SetWindowText(m_strDialogTitle);

		// Enable/Disable cancelling possibility.
		GetDlgItem(IDCANCEL)->EnableWindow(m_bEnableCancel);
		// We mark the preview ones as completed.
		for (int i=0; i<m_nStage; ++i)
		{
			m_ctlProgress[i].SetRange32(0, 100);
			m_ctlProgress[i].SetPos(100);
		}

		// We mark the following ones as not done.
		for (int i=m_nStage+1; i<CRenamingList::stageCount; ++i)
		{
			m_ctlProgress[i].SetRange32(0, 100);
			m_ctlProgress[i].SetPos(0);
		}

		// Set the total of the current stage.
		m_ctlProgress[m_nStage].SetRange32(0, (int)m_nTotal);

		m_bRefreshEverything = false;
	}

	// Update the progression caption.
	CString strCurrentProgressCaption = m_strProgressCaption;
	strText.Format(_T("%d"), m_nDone); strCurrentProgressCaption.Replace(_T("$(Done)"), strText);
	strText.Format(_T("%d"), m_nTotal); strCurrentProgressCaption.Replace(_T("$(Total)"), strText);
	strText.Format(_T("%d"), (int)(m_nDone*100.0f/m_nTotal + 0.5f)); strCurrentProgressCaption.Replace(_T("$(Percents)"), strText);
	m_ctlProgressStatic.GetWindowText(strText);
	if (strText != strCurrentProgressCaption)
		m_ctlProgressStatic.SetWindowText(strCurrentProgressCaption);

	// Update the progression bar.
	m_ctlProgress[m_nStage].SetPos(m_nDone);
}

INT_PTR CProgressDlg::DoModal()
{
	// Don't show the dialog if it's a very fast operation.
	Sleep(100);
	if (m_bDone)
		return IDOK;
	else
		return CDialog::DoModal();
}
