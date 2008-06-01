#pragma once

namespace Gui{ namespace Control
{
	// CInviteRichEdit
	class CInviteRichEdit : public CRichEditCtrl
	{
		DECLARE_DYNAMIC(CInviteRichEdit)

	public:
		CInviteRichEdit();
		virtual ~CInviteRichEdit();

		// Invite text.
		void SetInviteText(CString &strInvite);
		void SetInviteText(UINT nID);
		CString& GetInviteText();

	protected:
		DECLARE_MESSAGE_MAP()

		bool	m_bDisplayInvite;
		CString	m_strInvite;

	public:
		afx_msg void OnEnSetfocus();
		afx_msg void OnEnKillfocus();
		afx_msg void OnPaint();
	};
}}
