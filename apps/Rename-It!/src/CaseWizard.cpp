#include "StdAfx.h"
#include "../resource.h"
#include "CaseWizard.h"

namespace Wizard
{
	// CCaseWizard dialog

	IMPLEMENT_DYNAMIC(CCaseWizard, CDialog)

	CCaseWizard::CCaseWizard(CWnd* pParent /*=NULL*/)
		: CDialog(CCaseWizard::IDD, pParent)
		, m_nCaseType(-1)
		, m_strBefore(_T(""))
	{
	}

	CCaseWizard::~CCaseWizard()
	{
	}

	shared_ptr<IFilter> CCaseWizard::Execute(IPreviewFileList& pflSamples)
	{
		m_pPreviewSamples = &pflSamples;
		ASSERT(m_pPreviewSamples != NULL);

		// Show the case wizard's dialog.
		if (DoModal() != IDOK)
			return shared_ptr<IFilter>();	// The user cancelled.

		// Return the generated filter.
		return GenerateFilter();
	}

	void CCaseWizard::DoDataExchange(CDataExchange* pDX)
	{
		CDialog::DoDataExchange(pDX);
		DDX_Radio(pDX, IDC_LOWERCASE, m_nCaseType);
		DDX_Text(pDX, IDC_BEFORE, m_strBefore);
	}

	BOOL CCaseWizard::OnInitDialog()
	{
		__super::OnInitDialog();

		m_strBefore = m_pPreviewSamples->GetBeforePreviewRenaming().GetFilteredSubstring();
		UpdateData(FALSE);

		OnUpdatePreview();

		return TRUE;  // return TRUE unless you set the focus to a control
		// EXCEPTION: OCX Property Pages should return FALSE
	}

	BEGIN_MESSAGE_MAP(CCaseWizard, CDialog)
		ON_BN_CLICKED(IDC_LOWERCASE, OnUpdatePreview)
		ON_BN_CLICKED(IDC_NAMECASE, OnUpdatePreview)
		ON_BN_CLICKED(IDC_NATURALCASE, OnUpdatePreview)
		ON_BN_CLICKED(IDC_UPPERCASE, OnUpdatePreview)
		ON_BN_CLICKED(IDC_INVERTCASE, OnUpdatePreview)
	END_MESSAGE_MAP()


	// CCaseWizard message handlers

	void CCaseWizard::OnUpdatePreview()
	{
		shared_ptr<IFilter> filter = GenerateFilter();
		if (filter.get() == NULL)
		{
			// Disable the OK button.
			GetDlgItem(IDOK)->EnableWindow(FALSE);
			GetDlgItem(IDC_AFTER)->SetWindowText( m_strBefore );
		}
		else
		{
			// Update the sample.
			ASSERT(m_pPreviewSamples != NULL);
			CString strAfter = m_pPreviewSamples->PreviewRenaming(filter.get()).GetFilteredSubstring();

			GetDlgItem(IDOK)->EnableWindow(TRUE);
			GetDlgItem(IDC_AFTER)->SetWindowText( strAfter );
		}
	}

	shared_ptr<IFilter> CCaseWizard::GenerateFilter()
	{
		if (!UpdateData())
			return shared_ptr<IFilter>();

		// Load get the equivalent EChangeCase of the selected m_nCaseType.
		CSearchReplaceFilter::EChangeCase nChangeCase;
		switch (m_nCaseType)
		{
		case 0:	nChangeCase = CSearchReplaceFilter::caseLower; break;
		case 1:	nChangeCase = CSearchReplaceFilter::caseUpper; break;
		case 2:	nChangeCase = CSearchReplaceFilter::caseSentense; break;
		case 3:	nChangeCase = CSearchReplaceFilter::caseWord; break;
		case 4:	nChangeCase = CSearchReplaceFilter::caseInvert; break;
		default:
			nChangeCase = CSearchReplaceFilter::caseNone;
		}

		// Create the filter.
		shared_ptr<CSearchReplaceFilter> filter(new CSearchReplaceFilter());
		filter->SetChangeCase(nChangeCase);
		return filter;
	}
}
