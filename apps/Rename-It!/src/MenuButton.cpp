/* -----------------98/08/24 04:53AM-----------------
 *Uses code from CodeGuru.Com...
 *The niffy way of using the menu in immediate mode are (directly) from
 *  Using CMenu::TrackPopupMenu as an Immediate Function - Randy More ("http://www.codeguru.com/menu/track_popup.shtml"),
 *General ownerdraw format from
 *  Flat owner drawn buttons - Girish Bharadwaj ("http://www.codeguru.com/buttonctrl/flat_button.shtml"),
 *and the idea for drawing the arrow from
 *  PushButton with bitmap & text - Michael Santoro ("http://www.codeguru.com/buttonctrl/bitmap_text_button.shtml")
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * I have no way to test if this will work with UNICODE, however it _SHOULD_. (:
 * This was compiled with VC5 and MFC4.2. Tested on Win95 OSR2 (no IE4 integration).
 * This code should compile cleanly under warning level 4.
 *
 * To Use: Simply include "MenuButton.h" and "MenuButton.cpp" in your project,
 * and include "MenuButton.h" in whatever file you wish to use it in ("stdafx.h" mayhaps).
 *
 * Then in the dialog editor, make a normal button, a suggested size is 12x14. The
 * little arrow gfx will be centered in the button however.  Make a handler for
 * a button click, and copy the sample usage code to there, then of course modify to suit your needs.
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Sample Usage Code:
//////////////////////////////////////////////////////////////////////////
//  //  //  //  //  //  //  //  //  //  //  //  //  //  //  //  //  //  //
//////////////////////////////////////////////////////////////////////////
//// -[Feral]---------------------------------------------------------------
//// pop up a menu, and add pre defined text to the edit ctrl.
//void CPP_Split::OnBUTTONNameFormatOptions()
//{
//    // -[Feral]-----------------------------------------------------------
//    // Vars and Assignment
//    CEdit *pEdit = (CEdit*)GetDlgItem(IDC_EDIT_NameFormat);
//
//
//    // -[Feral]-----------------------------------------------------------
//    // popup a menu and let the user make a selection...
//    DWORD dwSelectionMade = m_menubuttonNameFormatOptions.DoSingleClick(IDM_NameFormatOptions);
//
//    // -[Feral]-----------------------------------------------------------
//    // Do something with the selection
//    // -------------------------------------------------------------------
//    // The value of dwSelectionMade is the id of the command selected or 0 if no selection was made
//    switch(dwSelectionMade)
//    {
//        default:                        // Do nothing
//            break;
//        case `Menu Resource ID`:
//            pEdit->;ReplaceSel(_T("`Text to add at cursor, or replace selection with`") );
//            break;
//        case ID_PREDEFINEDPOPUP_SAMPLE1FILENAME1XEXTSPLITPIECE:       // "Sample #&1: Filename.1_X.Ext.SplitPiece",
//            pEdit->ReplaceSel(_T("<File>.<1_X>.<Ext>.SplitPiece") );
//            break;
//    }
//
//    // -[Feral]-----------------------------------------------------------
//    // Clean Up and Return
//}
//////////////////////////////////////////////////////////////////////////
//  //  //  //  //  //  //  //  //  //  //  //  //  //  //  //  //  //  //
//////////////////////////////////////////////////////////////////////////
 * --------------------------------------------------*/
// MenuButton.cpp : implementation file
//

#include "stdafx.h"
#include "MenuButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMenuButton

CMenuButton::CMenuButton()
{
}

CMenuButton::~CMenuButton()
{
}


BEGIN_MESSAGE_MAP(CMenuButton, CButton)
	//{{AFX_MSG_MAP(CMenuButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMenuButton message handlers

DWORD CMenuButton::DoSingleClick(CMenu &menu)
{
    // -[Feral]-----------------------------------------------------------
    // Vars and Assignment
    DWORD dwSelectionMade;

    CMenu *pmenuPopup = menu.GetSubMenu(0);
    ASSERT(pmenuPopup != NULL);
    POINT point;

    // -[Feral]-----------------------------------------------------------
    // get the point where the menu is to be displayed.
    // this is hte lower left corner of the control (button)
    CRect rect;
    GetWindowRect(rect);
    point.x = rect.right;
    point.y = rect.top-1;

    // -[Feral]-----------------------------------------------------------
    // show and track the menu
    dwSelectionMade = pmenuPopup->TrackPopupMenu( (TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD),
                                                  point.x, point.y, this
                                                );
    // -[Feral]-----------------------------------------------------------
    // Clean Up and Return
    pmenuPopup->DestroyMenu();
    // The value of dwSelectionMade is the id of the command selected or 0 if no selection was made
    return dwSelectionMade;
}

void CMenuButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    // -[Feral]-----------------------------------------------------------
    // VALIDATION:
    ASSERT(lpDrawItemStruct != NULL);

    // -[Feral]-----------------------------------------------------------
    // Vars and Assignment
    CRect rect      = lpDrawItemStruct->rcItem;
    CDC *pDC        = CDC::FromHandle(lpDrawItemStruct->hDC);
    UINT uiState    = lpDrawItemStruct->itemState;
    CPen pen;
    CPen *ppenOld   = NULL;

    // -[Feral]-----------------------------------------------------------
    // set the pen color based on if we are disabled or not
    if( (uiState&ODS_DISABLED) )
    {
        pen.CreatePen(PS_SOLID, 0, ::GetSysColor(COLOR_GRAYTEXT) );
    }
    else
    {
        pen.CreatePen(PS_SOLID, 0, ::GetSysColor(COLOR_BTNTEXT) );
    }


    // -[Feral]-----------------------------------------------------------
    // select the pen into the DC.
    ppenOld = pDC->SelectObject(&pen);


    // -[Feral]-----------------------------------------------------------
    // draw the border
    if( (uiState&ODS_SELECTED) )
    {
        pDC->DrawFrameControl(rect, DFC_BUTTON, DFCS_BUTTONPUSH|DFCS_PUSHED);
    }
    else
    {
        pDC->DrawFrameControl(rect, DFC_BUTTON, DFCS_BUTTONPUSH);
    }


    // -[Feral]-----------------------------------------------------------
    // Draw the Arrow...
    // Something like:
    // X
    // XX
    // XXX
    // XXXX
    // XXX
    // XX
    // X
    // In the Center of the button.
    CSize sizeArrow(4,7);
    CSize sizeOffsetTop(1,+1);          // size of the top stairsteps
    CSize sizeOffsetBottom(1,-1);       // size of the bottom stairsteps
    CPoint pointCenter( (rect.Width()/2), (rect.Height()/2) );
    CPoint pointStart( (pointCenter.x-(sizeArrow.cx/2) ), (pointCenter.y-(sizeArrow.cy/2) ) );
    CPoint pointStop ( (pointStart.x), (pointStart.y+sizeArrow.cy) );
    // -[Feral]-----------------------------------------------------------
    // start at the left and work to the right...
    for(int iInd=0; iInd<sizeArrow.cx; iInd++)
    {
        // -[Feral]-------------------------------------------------------
        // draw the lines
        pDC->MoveTo(pointStart);
        pDC->LineTo(pointStop);

        // -[Feral]-------------------------------------------------------
        // offset our points. (going right!)
        pointStart.Offset(sizeOffsetTop);
        pointStop .Offset(sizeOffsetBottom);
    }

    // -[Feral]-----------------------------------------------------------
    // Draw the focus rectangle if necessary.
    if( (uiState&ODS_FOCUS) )
    {
        CRect rectFocus(rect);
        rectFocus.DeflateRect(3,3);     // This looked prety close
        pDC->DrawFocusRect(rectFocus);
    }

    // -[Feral]-----------------------------------------------------------
    // Clean Up and Return
    pDC->SelectObject(ppenOld);         // Restore the pen
}

//
///EOF
