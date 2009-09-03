#if !defined(AFX_MENUBUTTON_H__335C8BE1_3A55_11D2_91E2_444553540000__INCLUDED_)
#define AFX_MENUBUTTON_H__335C8BE1_3A55_11D2_91E2_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

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


// MenuButton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMenuButton window

class CMenuButton : public CButton
{
// Construction
public:
	CMenuButton();

// Attributes
public:

// Operations
public:
    DWORD DoSingleClick(CMenu &menu);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMenuButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMenuButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMenuButton)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MENUBUTTON_H__335C8BE1_3A55_11D2_91E2_444553540000__INCLUDED_)
//
///EOF
