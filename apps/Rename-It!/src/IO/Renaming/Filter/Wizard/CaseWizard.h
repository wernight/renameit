#pragma once

#include "FilterWizard.h"
#include "../SearchReplaceFilter.h"
#include "../../Path.h"

namespace Beroux{ namespace IO{ namespace Renaming{ namespace Filter{ namespace Wizard
{
	// CCaseWizard dialog
	class CCaseWizard : public CDialog, public CFilterWizard
	{
		DECLARE_DYNAMIC(CCaseWizard)

	public:
		CCaseWizard(CWnd* pParent = NULL);   // standard constructor
		virtual ~CCaseWizard();

	// CFilterWizard
		shared_ptr<IFilter> Execute(IPreviewFileList& pflSamples);

	// Dialog Data
		enum { IDD = IDD_WIZ_CASE };

	private:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		virtual BOOL OnInitDialog();

		DECLARE_MESSAGE_MAP()

		/**
		 * Updates the "after" renaming text box.
		 */
	    void OnUpdatePreview();

		/**
		 * Generate the resulting filter for the currently selected wizard settings.
		 * @exception std::runtime_error Invalid settings are currently selected.
		 * @return The generated filter.
		 */
		shared_ptr<IFilter> GenerateFilter();

		const Beroux::IO::Renaming::Filter::IPreviewFileList* m_pPreviewSamples;
		int m_nCaseType;
		CString	m_strBefore;
	};
}}}}}
