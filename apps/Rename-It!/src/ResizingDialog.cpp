////////////////////////////////////////////////////////////////////////
// ResizingDialog.cpp : implementation file
//
// The base class for the dialog box you want to allow resizing
// Use SetConrolInfo() to determine how each control behaves when
// the user resize the dialog box.
// (The "Windows default" is ANCHORE_TOP | ANCHORE_LEFT)
//
// e.g. For a right aligned OK button you'll probably call:
// SetControlInfo(IDOK, ANCHORE_RIGHT)
// For a text control that needs to resize with the dialog you may do:
// SetControlInfo(IDD_MYEDITOR, RESIZE_BOTH)
//
// Note: The dialog box "remebers" its size on destroy and the next time
// you launch it, it'll set the dialog size back to the previous size.
// If you don't like this behavior, call SetRememberSize(FALSE)
//
// LIMITATIONS:
// 1) This class does not handle overlapping controls, 
//    e.g., you cannot place two controls one (RESIZE_VER) and the other
//    with (RESIZE_VER | ANCHORE_BOTTOM) one below the other, they may ovelapp.
//
// 2) This class does not remember the mode of the dialog (Maximized/Minimized)
//	  it would be easy to add this feature, though.

#include "stdafx.h"
#include "ResizingDialog.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CResizingDialog dialog

CResizingDialog::CResizingDialog(UINT nIDTemplate, CWnd* pParentWnd) : 
				CDialog(nIDTemplate,pParentWnd)
{
	m_minWidth = m_minHeight = 0;	// flag that GetMinMax wasn't called yet
	m_old_cx = m_old_cy = 0;
	m_bSizeChanged = FALSE;
	m_nIDTemplate = nIDTemplate;
}

void CResizingDialog::SetControlInfo(WORD CtrlId,WORD Anchore)			
{
	if(Anchore == ANCHORE_LEFT)
		return; // Do nothing

	// Add resizing behaviour for the control
	DWORD c_info = CtrlId | (Anchore << 16);
	m_control_info.Add(c_info);
}

BEGIN_MESSAGE_MAP(CResizingDialog, CDialog)
	//{{AFX_MSG_MAP(CResizingDialog)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////
// CResizingDialog message handlers



//////////////////////////////////////////////////////////////////////////
// OnInitDialog()
//
BOOL CResizingDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	return FALSE;  // return TRUE  unless you set the focus to a control
}

void CResizingDialog::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
        if (nType == SIZE_MINIMIZED)
            return;

        int dx = cx - m_old_cx;
	int dy = cy - m_old_cy;

        if(m_old_cx)
	{
		// Move and Size the controls using the information
		// we got in SetControlInfo()
		//
		m_bSizeChanged = TRUE;
		CRect WndRect;
		CWnd *pWnd;
		DWORD c_info;
		short Anchore;
		for(int i = 0; i < m_control_info.GetSize(); i++)
		{
			c_info = m_control_info[i];
			pWnd = GetDlgItem(LOWORD(c_info));
			if(!pWnd)
			{
				TRACE("Control ID - %d NOT FOUND!!\n",LOWORD(c_info));
				continue;
			}

			if(!HIWORD(c_info))
				continue; // do nothing if anchored to top and or left

			Anchore = HIWORD(c_info);
			pWnd->GetWindowRect(&WndRect);  ScreenToClient(&WndRect);
			
			if(Anchore & RESIZE_HOR)
				WndRect.right += dx;
			else if(Anchore & ANCHORE_RIGHT)
				WndRect.OffsetRect(dx,0);

			if(Anchore & RESIZE_VER)
				WndRect.bottom += dy;
			else if(Anchore & ANCHORE_BOTTOM)
				WndRect.OffsetRect(0,dy);

			pWnd->MoveWindow(&WndRect);
		}
                RedrawWindow();
	}
	m_old_cx = cx;
	m_old_cy = cy;
}

void CResizingDialog::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    if (!m_minWidth) // first time
		{
		CDialog::OnGetMinMaxInfo(lpMMI);
		return;
		}
    lpMMI->ptMinTrackSize.x = m_minWidth;
    lpMMI->ptMinTrackSize.y = m_minHeight;
}

void CResizingDialog::OnDestroy() 
{
	m_minWidth = m_minHeight = m_old_cx = m_old_cy = 0;
	m_bSizeChanged = FALSE;
	CDialog::OnDestroy();
}


int CResizingDialog::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// Remember the original size so later we can calculate
	// how to place the controls on dialog Resize
	m_minWidth  = lpCreateStruct->cx;
	m_minHeight = lpCreateStruct->cy;
	return 0;
}
