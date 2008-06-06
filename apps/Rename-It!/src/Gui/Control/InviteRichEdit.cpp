// AskEdit.cpp : fichier d'implémentation
//

#include "stdafx.h"
#include "InviteRichEdit.h"
#include ".\inviteedit.h"


namespace Gui{ namespace Control
{

// CInviteRichEdit

IMPLEMENT_DYNAMIC(CInviteRichEdit, CEdit)
CInviteRichEdit::CInviteRichEdit() :
	m_bDisplayInvite(true),
	m_strInvite(_T(""))
{
}

CInviteRichEdit::~CInviteRichEdit()
{
}


BEGIN_MESSAGE_MAP(CInviteRichEdit, CEdit)
	ON_CONTROL_REFLECT(EN_SETFOCUS, OnEnSetfocus)
	ON_CONTROL_REFLECT(EN_KILLFOCUS, OnEnKillfocus)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// Gestionnaires de messages CInviteRichEdit

void CInviteRichEdit::SetInviteText(CString &strInvite)
{
	m_strInvite = strInvite;
}

void CInviteRichEdit::SetInviteText(UINT nID)
{
	m_strInvite.LoadString( nID );
}

CString& CInviteRichEdit::GetInviteText()
{
	return m_strInvite;
}

void CInviteRichEdit::OnEnSetfocus()
{
	// Remove invite text if user select the edit box.
	if (m_bDisplayInvite)
	{
		m_bDisplayInvite = false;
		Invalidate();
	}
}

void CInviteRichEdit::OnEnKillfocus()
{
	bool bDisplayInvite = (GetWindowTextLength() == 0);

	// Display invite text only if no text is entered by user.
	if (bDisplayInvite != m_bDisplayInvite)
	{
		m_bDisplayInvite = bDisplayInvite;
		Invalidate();
	}
}

void CInviteRichEdit::OnPaint()
{
	if (!m_bDisplayInvite || GetWindowTextLength() > 0)
	{
		CRichEditCtrl::OnPaint();
		return;
	}
	else if (GetWindowTextLength() > 0)
	{
		m_bDisplayInvite = true;
		CRichEditCtrl::OnPaint();
		return;
	}

	CPaintDC dc(this); // device context for painting
	CFont	*pfontBackup;
	CRect	rect;

	// Set font color
	dc.SetTextColor( GetSysColor(COLOR_GRAYTEXT) );
	dc.SetBkColor( GetSysColor(COLOR_WINDOW) );

	// Set font face
	pfontBackup = dc.SelectObject( GetParent()->GetFont() );

	// Fill background in white
	GetWindowRect(rect);
	ScreenToClient(rect);
	rect.DeflateRect(2, 2, 2, 2);
	dc.FillSolidRect(rect, GetSysColor(COLOR_WINDOW));

	// Display invite message
	CRichEditCtrl::GetRect(rect);
	dc.ExtTextOut(rect.left, rect.top, ETO_CLIPPED|ETO_OPAQUE, rect, m_strInvite, NULL);

	// Clean up
	dc.SelectObject( pfontBackup );
	// Don't call CRichEditCtrl::OnPaint() to display messages
}

}}
