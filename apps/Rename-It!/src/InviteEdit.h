#pragma once


// CInviteEdit

class CInviteEdit : public CEdit
{
	DECLARE_DYNAMIC(CInviteEdit)

public:
	CInviteEdit();
	virtual ~CInviteEdit();

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
