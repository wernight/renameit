// RenamePartSelectionCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "RenameIt.h"
#include "RenamePartSelectionCtrl.h"
#include "winuser.h"


// CRenamePartSelectionCtrl

IMPLEMENT_DYNAMIC(CRenamePartSelectionCtrl, CRichEditCtrl)

CRenamePartSelectionCtrl::CRenamePartSelectionCtrl()
{

}

CRenamePartSelectionCtrl::~CRenamePartSelectionCtrl()
{
}


BEGIN_MESSAGE_MAP(CRenamePartSelectionCtrl, CRichEditCtrl)
	ON_NOTIFY_REFLECT(EN_SELCHANGE, &CRenamePartSelectionCtrl::OnEnSelchange)
END_MESSAGE_MAP()

void CRenamePartSelectionCtrl::PreSubclassWindow()
{
	// Initialize the filename-part richedit
	SetTextMode(TM_PLAINTEXT);
	SetTextMode(TM_SINGLELEVELUNDO);
	SetEventMask( GetEventMask() | ENM_SELCHANGE );
	SetOptions(ECOOP_OR, ECO_SAVESEL);
	SetBackgroundColor(FALSE, GetSysColor(COLOR_3DFACE));

	CString strCaption;
	strCaption.LoadString(IDS_RENAME_PARTS);
	SetWindowText( strCaption );
	unsigned nPosPathStart = strCaption.Find('\\'),
			nPosPathEnd = strCaption.ReverseFind('\\') + 1,
			nPosFilenameStart = nPosPathEnd,
			nPosFilenameEnd = strCaption.ReverseFind('.'),
			nPosExtStart = nPosFilenameEnd + 1,
			nPosExtEnd = strCaption.GetLength();
	m_vPossibleSelections.push_back( POSSIBLE_SELECTION(nPosPathStart, nPosPathEnd, renameFolders) );
	m_vPossibleSelections.push_back( POSSIBLE_SELECTION(nPosFilenameStart, nPosFilenameEnd, renameFilename) );
	m_vPossibleSelections.push_back( POSSIBLE_SELECTION(nPosExtStart, nPosExtEnd, renameExtension) );
	m_vPossibleSelections.push_back( POSSIBLE_SELECTION(nPosPathStart, nPosFilenameEnd, renameFolders | renameFilename) );
	m_vPossibleSelections.push_back( POSSIBLE_SELECTION(nPosFilenameStart, nPosExtEnd, renameFilename | renameExtension) );
	m_vPossibleSelections.push_back( POSSIBLE_SELECTION(nPosPathStart, nPosExtEnd, renameFolders | renameFilename | renameExtension) );

	CRichEditCtrl::PreSubclassWindow();
}

// CRenamePartSelectionCtrl message handlers

void CRenamePartSelectionCtrl::OnEnSelchange(NMHDR *pNMHDR, LRESULT *pResult)
{
	SELCHANGE *pSelChange = reinterpret_cast<SELCHANGE *>(pNMHDR);
	// TODO:  The control will not send this notification unless you override the
	// CRichEditCtrl::OnInitDialog() function to send the EM_SETEVENTMASK message
	// to the control with the ENM_SELCHANGE flag ORed into the lParam mask.

	LONG cpMin = pSelChange->chrg.cpMin;
	LONG cpMax = pSelChange->chrg.cpMax;

	// If the selection starts before the first allowed range, or after the last one,
	// clamp it
	if (cpMin < m_vPossibleSelections.front().cr.cpMin)
		cpMin = m_vPossibleSelections.front().cr.cpMin;
	if (cpMax > m_vPossibleSelections.back().cr.cpMax)
		cpMax = m_vPossibleSelections.back().cr.cpMax;

	// Select the first possible-selection that contains the current selection
	std::vector<POSSIBLE_SELECTION>::iterator iter;
	for (iter=m_vPossibleSelections.begin(); iter!=m_vPossibleSelections.end(); ++iter)
	{
		// Is the possible selection part of the selection?
		if (cpMin >= (*iter).cr.cpMin &&
			cpMax <= (*iter).cr.cpMax)
		{
			// Is that range already different from the selected range?
			if ((*iter).cr.cpMin != pSelChange->chrg.cpMin ||
				(*iter).cr.cpMax != pSelChange->chrg.cpMax)
			{
				// Detect where to start the selection
				LONG nStartChar, nEndChar;
				GetSel(nStartChar, nEndChar);
				LONG nCursorPos = abs(nStartChar + nEndChar)/2;
				bool bCursorAtSelStart;
				if (nStartChar == (*iter).cr.cpMin)
					bCursorAtSelStart = true;
				else if  (nEndChar == (*iter).cr.cpMax)
					bCursorAtSelStart = false;
				else
					// If start and end are different,
					// find which one is clothes from the cursor's position
					bCursorAtSelStart = (abs(nStartChar + nEndChar)/2 < abs((*iter).cr.cpMin + (*iter).cr.cpMax)/2);

				if (bCursorAtSelStart)
					// Select that range
					SetSel((*iter).cr);
				else
					// Select that range starting by the end
					SetSel((*iter).cr.cpMax, (*iter).cr.cpMin);

				// Remember what part is going to be renamed
				m_nRenamePart = (*iter).nRenamePart;

				// Notify the parent
				NMHDR pnmh;
				pnmh.hwndFrom = m_hWnd;
				pnmh.idFrom = GetDlgCtrlID();
				pnmh.code = RPS_SELCHANGE;
				GetOwner()->SendMessage(WM_NOTIFY, (WPARAM) pnmh.idFrom, (LPARAM) &pnmh);
			}
			break;
		}
	}

	*pResult = 0;
}

