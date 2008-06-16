////////////////////////////////////////////////////////////////////////////
// SizingDialog.cpp :  implementation file
//
// Copyright 2005 Xia Xiongjun( ÏÄÐÛ¾ü ), All Rights Reserved.
//
// E-mail: xj-14@163.com
//
// This source file may be copyed, modified, redistributed  by any means
// PROVIDING that this notice and the authors name and all copyright notices
// remain intact, and PROVIDING it is NOT sold for profit without the authors
// expressed written consent. The author accepts no liability for any 
// damage/loss of business that this product may cause.
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SizingDialog.h"
#include ".\SizingDialog.h"

////////////////////////////////////////////////////////////////////////////
// class CCtrlInfo

CCtrlInfo::CCtrlInfo(LPCRECT rcInit /*= NULL*/,
		double dRateLeft /*= 0.0*/,  double dRateRight  /*= 0.0*/, 
		double dRateTop  /*= 0.0*/,  double dRateBottom /*= 0.0*/)
	: m_rcInit    (rcInit)
	, m_dRateLeft (dRateLeft),  m_dRateRight  (dRateRight)
	, m_dRateTop  (dRateTop),   m_dRateBottom (dRateBottom)
{
}

CCtrlInfo::~CCtrlInfo(void)
{
}

BOOL CCtrlInfo::Modify(LPCRECT rcInit,
		double dRateLeft /*= 0.0*/,  double dRateRight    /*= 0.0*/, 
		double dRateTop  /*= 0.0*/,  double dRateBottom   /*= 0.0*/) 
{
	m_rcInit    = rcInit;
	m_dRateLeft = dRateLeft;  m_dRateRight  = dRateRight;
	m_dRateTop  = dRateTop;   m_dRateBottom = dRateBottom;

	return TRUE;
}

BOOL CCtrlInfo::Modify(const CWnd* pCtrl, const CWnd* pParent, 
		LPCTSTR lpszString, char chSpr /*= '+'*/)
{
	ASSERT(::IsWindow(pCtrl->m_hWnd));
	ASSERT(::IsWindow(pParent->m_hWnd));
	ASSERT(::IsChild(pParent->m_hWnd, pCtrl->m_hWnd));

	pCtrl->GetWindowRect(&m_rcInit);
	pParent->ScreenToClient(&m_rcInit);

	// XR: Get position information from Initialization string
	CString str = lpszString;
	str.Remove(' ');
	str.MakeUpper();

	int nLen = str.GetLength();
	int posSpr = -1;
	int posSprNext = 0;
	while(posSprNext != -1)
	{
		CString strSection;
		posSprNext = str.Find(chSpr, posSpr + 1);  // XR: default separator between sections is '+'
		if (posSprNext == -1)
			strSection = str.Mid(posSpr + 1, nLen - posSpr - 1);
		else
			strSection = str.Mid(posSpr + 1, posSprNext - posSpr - 1);
		
		if (!this->ExtractOptions(strSection, pCtrl, pParent))
			return FALSE;

		posSpr = posSprNext;
	}
	return TRUE;
}

BOOL CCtrlInfo::ExtractOptions(LPCTSTR lpszSection, 
		const CWnd* pCtrl /*= NULL*/, const CWnd* pParent /*= NULL*/) 
{
	CString strWord;
	double dCoef;
	this->GetCoefficientAndWord(lpszSection, strWord, dCoef);

	if (strWord.IsEmpty())
		strWord = _T("R");
	
	// XR: synchronize with the right/bottom border of the window in 
	// horizontal/vertical direction.
	// XR: dCoef is the factor which you can specify to determine the degree
	// how the controls' position is affected by the parent window border.
	// dCoef is set to 1.0 when you don't specify a value for it.
	if (strWord == _T("X") || strWord == _T("Y") || strWord == _T("XY"))
	{
		if (strWord != _T("Y"))
			m_dRateLeft = m_dRateRight = dCoef;
		if (strWord != _T("X"))
			m_dRateTop = m_dRateBottom = dCoef;
	}

	// XR: only the right/bottom border of the controls will synchronize with 
	// the right/bottom border of the window in horizontal/vertical direction.
	else if (strWord == _T("C")  || strWord == _T("CXY") ||
		     strWord == _T("CX") || strWord == _T("CY"))
	{
		if (strWord != _T("CY"))
		{
			m_dRateLeft = 0.0;
			m_dRateRight = dCoef;
		}
		if (strWord != _T("CX"))
		{
			m_dRateTop = 0.0;
			m_dRateBottom = dCoef;
		}
	}

	// XR: keep the position and size of the controls and the window's client area 
	// be in same proportion when resizing the window.
	else if (strWord == _T("R")  || strWord == _T("RXY") ||	
			 strWord == _T("RX") || strWord == _T("RY"))
	{
		ASSERT(::IsWindow(pCtrl->m_hWnd));
		ASSERT(::IsWindow(pParent->m_hWnd));
		ASSERT(::IsChild(pParent->m_hWnd, pCtrl->m_hWnd));

		CRect rcWnd;
		pParent->GetClientRect(&rcWnd);

		if (strWord != _T("RY"))
		{
			m_dRateLeft   = dCoef*(m_rcInit.left   - rcWnd.left)/rcWnd.Width();
			m_dRateRight  = dCoef*(m_rcInit.right  - rcWnd.left)/rcWnd.Width();
		}
		if (strWord != _T("RX"))
		{
			m_dRateTop    = dCoef*(m_rcInit.top    - rcWnd.top )/rcWnd.Height();
			m_dRateBottom = dCoef*(m_rcInit.bottom - rcWnd.top )/rcWnd.Height();
		}
	}

	// XR: Error
	else
	{
		CString strErr;
		strErr.Format(_T("'%s' is not predefined keyword"), strWord);
		MessageBox(NULL, strErr, _T("There is something wrong in the expression describing control position"), 
			MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	return TRUE;
}

// XR: Get coefficient and word from a string such as "0.5X" or "(-1.02)XY" or "(-12.36R)"
void CCtrlInfo::GetCoefficientAndWord(LPCTSTR lpszSection,
									  CString& strWord, double& dCoef) const
{
	strWord = lpszSection;
	dCoef   = 1.0;

	CString str(lpszSection);
	int nLen = str.GetLength();

	if (nLen < 1)			// XM: Access to a null string is denied in VC6
		return;

	CString strCoef;
	TCHAR ch = str.GetAt(0);	// XM: See the previous remark

	if ( ch == '(' )
	{
		int nIndex = str.ReverseFind( ')' );

		if (nIndex == -1)				// XR: No the other bracket
			return;
		if (nIndex == nLen -1)			// XR: All the string is bracketed
		{
			this->GetCoefficientAndWord	// XM: 
					(strWord.Mid(1, nLen-2), strWord, dCoef);
			return;
		}

		strWord = str.Right(nLen - nIndex -1);
		strCoef = str.Mid(1, nIndex - 1);
		if (!strCoef.IsEmpty())
			dCoef = ::_tstof(strCoef);

		return;
	}

	int i;
	for (i = 0; i < nLen; i++)
	{
		ch = str.GetAt(i);
		if (::isdigit(ch) || ch == '.' || ch == 'E'
			|| ch == '+' || ch == '-' || ::isspace(ch))
			strCoef += ch;		// XM: For VC6
		else
		{
			strWord = str.Right(nLen - i);
			break;
		}
	}
	if (i == nLen)
		strWord.Empty();
	if (!strCoef.IsEmpty())
		dCoef = ::_tstof(strCoef);
}


////////////////////////////////////////////////////////////////////////////
// CSizingDialog dialog

// XR: Global functions
BOOL IsGroupBox(const CWnd *pWnd);
void ResizeGroupBox(CWnd* pGBox, const CWnd* pParent, const CCtrlInfo* pInfo,
		double dx, double dy);
void GetGroupBoxRgn(const CWnd* pGBox, const CWnd* pParent, CRect* pRc);
void AdjGroupBoxRgn(CRect* pRc, int adj, int nAlign);
void GetWindowTextSize(const CWnd* pWnd, CSize& sz);


IMPLEMENT_DYNAMIC(CSizingDialog, CDialog)

CSizingDialog::CSizingDialog(UINT nIDTemplate, CWnd* pParentWnd /*= NULL*/)
	: CDialog(nIDTemplate, pParentWnd)
{
	m_rcInit.SetRectEmpty();
	ASSERT(m_mapPtToInfo.IsEmpty());
}

CSizingDialog::~CSizingDialog()
{
	POSITION pos = m_mapPtToInfo.GetStartPosition();
	while (pos != NULL)
	{
		HWND hCtrl;
		CCtrlInfo* pInfo;
		m_mapPtToInfo.GetNextAssoc(pos, (void *&)hCtrl, (void *&)pInfo);
		delete pInfo;
		pInfo = NULL;
	}
	m_mapPtToInfo.RemoveAll();
}

void CSizingDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSizingDialog, CDialog)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()

// XR: Add a single control or all the controls ( when nID == NULL).

void CSizingDialog::AddResizableCtrl(UINT nID, LPCTSTR lpszString /*= NULL*/)
{
	this->AddCtrl(nID, DT_STRING, lpszString);
}

void CSizingDialog::AddResizableCtrl(UINT nID,
		double dRateLeft, double dRateRight, double dRateTop, double dRateBottom) 
{
	double dData[4] = {dRateLeft, dRateRight, dRateTop, dRateBottom};
	this->AddCtrl(nID, DT_NUMBER, dData);
}

void CSizingDialog::AddCtrl(UINT nID, int nDataType, LPCVOID lpData)
{
	if (nID == NULL)		// XR: all the controls
	{
		for (HWND hWnd = ::GetWindow(this->m_hWnd, GW_CHILD); hWnd != NULL;
			hWnd = ::GetNextWindow(hWnd, GW_HWNDNEXT))
		{
			UINT nIDCur = ::GetDlgCtrlID(hWnd);
			ASSERT(nIDCur != 0);
			this->AddCtrl(nIDCur, nDataType, lpData);	// XR: add all controls in turn
		}
		return;
	}

	// XR: add a single control

	CWnd* pCtrl = this->GetDlgItem(nID);
	ASSERT(::IsWindow(pCtrl->m_hWnd));

	LPCTSTR lpszString = (LPCTSTR)lpData;
	double* pDouble	= (double *)lpData;
	double& dRateLeft   = *pDouble;		pDouble++;
	double& dRateRight  = *pDouble;		pDouble++;
	double& dRateTop    = *pDouble;		pDouble++;
	double& dRateBottom = *pDouble;

	CRect rcCtrl;
	pCtrl->GetWindowRect(&rcCtrl);
	this->ScreenToClient(&rcCtrl);

	// XR: Add a new record to CMap object or update an old record

	CCtrlInfo* pInfo;
	if (!m_mapPtToInfo.Lookup(pCtrl->m_hWnd, (void *&)pInfo))
		pInfo = new CCtrlInfo;

	if (nDataType == DT_STRING)
		pInfo->Modify(pCtrl, this, lpszString);
	else if (nDataType == DT_NUMBER)
		pInfo->Modify(rcCtrl, dRateLeft,dRateRight, dRateTop, dRateBottom);
	
	m_mapPtToInfo[pCtrl->m_hWnd] = pInfo;
}

// XR: Add or update a control-range with the same setting
// which IDs range from nIDStart to nIDEnd.

void CSizingDialog::ModifyResizableCtrlRange(UINT nIDStart, UINT nIDEnd, 
										 LPCTSTR lpszString /*= NULL*/) 
{
	this->AddCtrlRange(nIDStart, nIDEnd, DT_STRING, lpszString);
}

void CSizingDialog::AddResizableCtrlRange(UINT nIDStart, UINT nIDEnd, 
										 LPCTSTR lpszString /*= NULL*/)
{
	this->AddCtrlRange(nIDStart, nIDEnd, DT_STRING, lpszString);
}

void CSizingDialog::AddResizableCtrlRange(UINT nIDStart, UINT nIDEnd, 
		double dRateLeft, double dRateRight, double dRateTop, double dRateBottom)
{
	double dData[4] = {dRateLeft, dRateRight, dRateTop, dRateBottom};
	this->AddCtrlRange(nIDStart, nIDEnd, DT_NUMBER, dData);
}

void CSizingDialog::AddCtrlRange(UINT nIDStart, UINT nIDEnd, 
		int nDataType, LPCVOID lpData)
{
	ASSERT(nIDStart != nIDEnd);

	HWND hCtrlS, hCtrlE;
	int n;
	this->RearrangeID(nIDStart, nIDEnd, hCtrlS, hCtrlE, n);

	HWND hCtrlE_Next = ::GetNextWindow(hCtrlE, GW_HWNDNEXT);
	HWND hCtrl = hCtrlS;
	do{
		ASSERT(hCtrl != NULL);
		UINT nIDTmp = ::GetDlgCtrlID(hCtrl);
		this->AddCtrl(nIDTmp, nDataType, lpData);
		hCtrl = ::GetNextWindow(hCtrl, GW_HWNDNEXT);
	}while(hCtrl != hCtrlE_Next);
}

// XR: Add a special positioned control-range which IDs range from nIDStart to nIDEnd.

void CSizingDialog::AddResizableCtrlArray(UINT nIDStart, UINT nIDEnd,
		double dS /*= 0.0*/, double dR /*= 1.0*/, BOOL bHori/* = TRUE*/)
// Tips: Set dR = 0.0	 to keep Ctrls Close To Border
//		 Set dR = 1.0	 to keep Ctrls Uniform
//	     Set dR = 1.0e20 to keep Intervals Constant
{
	ASSERT(nIDStart != nIDEnd);

	HWND hCtrlS, hCtrlE;
	int n;
	this->RearrangeID(nIDStart, nIDEnd, hCtrlS, hCtrlE, n);

	// XR: calculate the moving parameters for the controls
	double dI = (1.0 - n*dS)/(n + 2*dR - 1);
	int i = 0;
	HWND hCtrlE_Next = ::GetNextWindow(hCtrlE, GW_HWNDNEXT);
	HWND hCtrl = hCtrlS;
	do{
		ASSERT(hCtrl != NULL);
		UINT nIDTmp = ::GetDlgCtrlID(hCtrl);
		if (bHori)
			this->AddResizableCtrl(nIDTmp,
				dR*dI + dS*i + dI*i, dR*dI + dS*(i + 1) + dI*i, 0.0, 0.0);
		else
			this->AddResizableCtrl(nIDTmp, 
				0.0, 0.0, dR*dI + dS*i + dI*i, dR*dI + dS*(i + 1) + dI*i);		
		i++;
		hCtrl = ::GetNextWindow(hCtrl, GW_HWNDNEXT);
	}while(hCtrl != hCtrlE_Next);
}

// XR: correct the tab order of two specified controls' ID, 
// get their handle and the number of controls between them
void CSizingDialog::RearrangeID(UINT& nReIDS, UINT& nReIDE, 
								HWND& hCtrlS, HWND& hCtrlE, int& nNum) const
{
	ASSERT(nReIDS != nReIDE);
	ASSERT(::IsChild(this->m_hWnd, this->GetDlgItem(nReIDS)->m_hWnd));
	ASSERT(::IsChild(this->m_hWnd, this->GetDlgItem(nReIDS)->m_hWnd));

	nNum = 0;
	for (HWND hCtrl = ::GetWindow(this->m_hWnd, GW_CHILD); hCtrl != NULL; 
		 hCtrl = ::GetNextWindow(hCtrl, GW_HWNDNEXT))
	{
		UINT nIDTmp = ::GetDlgCtrlID(hCtrl);
		if (nNum == 0 && (nIDTmp == nReIDS || nIDTmp == nReIDE))
		{							// XR: First of all
			nNum++;
			if (nIDTmp == nReIDE)	// XR: correct the tab order
			{
				nIDTmp = nReIDS; 
				nReIDS = nReIDE;
				nReIDE = nIDTmp;
			}
			continue;
		}
		if (nNum > 0)				// XR: processing...
		{
			nNum++;
			if (nIDTmp == nReIDE)	// XR: end
			{
				hCtrlS = this->GetDlgItem(nReIDS)->m_hWnd;
				hCtrlE = this->GetDlgItem(nReIDE)->m_hWnd;
				break;
			}
		}
	}
}

// CSizingDialog message handlers

int CSizingDialog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// XR: set the dialog to be resizable, add MINIMIZEBOX and MAXIMIZEBOX.
	this->ModifyStyle(NULL, WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

	// XR: save Initial window size to m_rcInit
	this->GetWindowRect(&m_rcInit);

	return 0;
}

void CSizingDialog::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// XR: set the minimum window size is initial size.
	lpMMI->ptMinTrackSize.x = m_rcInit.Width();
	lpMMI->ptMinTrackSize.y = m_rcInit.Height();

	CDialog::OnGetMinMaxInfo(lpMMI);
}

LRESULT CSizingDialog::OnNcHitTest(CPoint point)
{
	// XR: move the window when click.
	LRESULT nHitTest = CDialog::OnNcHitTest(point);	
	if (nHitTest == HTCLIENT)
		nHitTest = HTCAPTION;

	return nHitTest;
}

void CSizingDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if (nType != SIZE_MINIMIZED)
		this->ResizeControls();
}

//////////////////////////////////////////////////////
// XR: moving the controls

void CSizingDialog::ResizeControls()
{
	// XR: size of the dialog moving
	CRect rcDlg;
	this->GetWindowRect(&rcDlg);
	int dx = rcDlg.Width()  - m_rcInit.Width();
	int dy = rcDlg.Height() - m_rcInit.Height();

	// XR: move the controls
	POSITION pos = m_mapPtToInfo.GetStartPosition();
	while (pos != NULL)
	{
		HWND hCtrl;
		CCtrlInfo* pInfo;
		m_mapPtToInfo.GetNextAssoc(pos, (void *&)hCtrl, (void *&)pInfo);

		TRACE("this: 0x%xd\n", this);

		TRACE("0x%xd\n", ::GetDlgItem(m_hWnd, IDCANCEL));
		TRACE("0x%xd\n", ::GetDlgItem(m_hWnd, IDOK));
		TRACE("0x%xd\n", hCtrl);

		ASSERT(::IsWindow(hCtrl));
		ASSERT(::IsChild(this->m_hWnd, hCtrl));

		CWnd* pCtrl = FromHandle(hCtrl);

		if (::IsGroupBox(pCtrl))
		{
			::ResizeGroupBox(pCtrl, this, pInfo, dx, dy);
			continue;
		}

		CRect rcCtrlOld;
		pCtrl->GetWindowRect(&rcCtrlOld);
		this->ScreenToClient(&rcCtrlOld);

		CRect rcCtrl = pInfo->m_rcInit;
		rcCtrl.left   += LONG(dx*pInfo->m_dRateLeft);
		rcCtrl.right  += LONG(dx*pInfo->m_dRateRight);
		rcCtrl.top    += LONG(dy*pInfo->m_dRateTop);
		rcCtrl.bottom += LONG(dy*pInfo->m_dRateBottom);

//		BOOL bVisible = ::IsWindowVisible(pCtrl->m_hWnd);
		pCtrl->SetRedraw(FALSE);
		pCtrl->MoveWindow(&rcCtrl);
// 		if (!bVisible)
// 			continue;
		pCtrl->SetRedraw(TRUE);
		
		this->InvalidateCtrlBorder(&rcCtrlOld, &rcCtrl);
		pCtrl->InvalidateRect(NULL);
	}
}

void CSizingDialog::InvalidateCtrlBorder(	// XR: erase lpRectOld - lpRectNew
	LPCRECT lpRectOld, LPCRECT lpRectNew, BOOL bErase /*= TRUE*/)
{
	CRect rcBorder;	

	// Top  Part
	if (lpRectNew->top > lpRectOld->top)
	{
		rcBorder.SetRect(lpRectOld->left, lpRectOld->top,
			lpRectOld->right, lpRectNew->top);
		this->InvalidateRect(&rcBorder, bErase);
	}

	// Bottom Part	
	if (lpRectNew->bottom < lpRectOld->bottom)
	{
		rcBorder.SetRect(lpRectOld->left, lpRectNew->bottom,
			lpRectOld->right, lpRectOld->bottom);
		this->InvalidateRect(&rcBorder, bErase);
	}

	// Left Part
	if (lpRectNew->left > lpRectOld->left)
	{
		rcBorder.SetRect(lpRectOld->left, lpRectOld->top,
			lpRectNew->left, lpRectOld->bottom);
		this->InvalidateRect(&rcBorder, bErase);
	}

	// Right Part
	if (lpRectNew->right < lpRectOld->right)
	{
		rcBorder.SetRect(lpRectNew->right, lpRectOld->top,
			lpRectOld->right, lpRectOld->bottom);
		this->InvalidateRect(&rcBorder, bErase);
	}
}


////////////////////////////////////////////////////////////////////////////
// XR: Global functions

void ResizeGroupBox(CWnd* pGBox, const CWnd* pParent, const CCtrlInfo* pInfo,
		double dx, double dy)
{
	ASSERT(::IsGroupBox(pGBox));
	ASSERT(::IsWindow(pParent->m_hWnd));
	ASSERT(::IsChild(pParent->m_hWnd, pGBox->m_hWnd));

	CRect rcGBoxOld[10];
	::GetGroupBoxRgn(pGBox, pParent, rcGBoxOld);

	CRect rcCtrl = pInfo->m_rcInit;
	rcCtrl.left   += LONG(dx*pInfo->m_dRateLeft);
	rcCtrl.right  += LONG(dx*pInfo->m_dRateRight);
	rcCtrl.top    += LONG(dy*pInfo->m_dRateTop);
	rcCtrl.bottom += LONG(dy*pInfo->m_dRateBottom);

//	BOOL bVisible = ::IsWindowVisible(pGBox->m_hWnd);
	pGBox->SetRedraw(FALSE);
	pGBox->MoveWindow(&rcCtrl);
//	if (!bVisible) 
//		return;
	pGBox->SetRedraw(TRUE);

	CRect rcGBox[10];
	::GetGroupBoxRgn(pGBox, pParent, rcGBox);

	for (int i = 1; i < 10; i++)
		((CSizingDialog *)pParent)->InvalidateCtrlBorder(&rcGBoxOld[i], &rcGBox[i]);

	// XR: update controls
	pGBox->InvalidateRect(NULL);
}

/////////////////////////////////////////////////////////////////////////
//
// void GetGroupBoxRgn(const CWnd* pGBox, const CWnd* pParent, CRect* pRc)
//
// XR: The meanings of pRc's elements in a GroupBox, 0 means whole rectangle
//
//                    ______________9____________
//                   |							 |
//                   7							 8
//                   |_____5_____[ 4 ]_____6_____|
//                   |	                         |
//                   |							 |
//                   |							 |
//                   |							 |
//                   |							 |
//                   |							 |
//                   |							 |
//                   2							 3
//                   |							 |
//                   |							 |
//                   |							 |
//                   |							 |
//                   |							 |
//                   |							 |
//                   |							 |
//                   |_____________1_____________|
//
//
/////////////////////////////////////////////////////////////////////////

void GetGroupBoxRgn(const CWnd* pGBox, const CWnd* pParent, CRect* pRc)
{
	ASSERT(::IsGroupBox(pGBox));
	ASSERT(::IsWindow(pParent->m_hWnd));
	ASSERT(::IsChild(pParent->m_hWnd, pGBox->m_hWnd));

	CRect rcGBox;
	pGBox->GetWindowRect(&rcGBox);
	pParent->ScreenToClient(&rcGBox);

	CSize sz;
	::GetWindowTextSize(pGBox, sz);

	// XR: whole rect
	pRc[0] = rcGBox;

	// XR: bottom line
	pRc[1].SetRect(		
		rcGBox.left,  rcGBox.bottom - 2,
		rcGBox.right, rcGBox.bottom);

	// XR: left line
	pRc[2].SetRect(		
		rcGBox.left,     rcGBox.top + sz.cy / 2,
		rcGBox.left + 2, rcGBox.bottom); 

	// XR: right line
	pRc[3].SetRect(
		rcGBox.right - 2, rcGBox.top + sz.cy / 2,
		rcGBox.right,     rcGBox.bottom); 

	// XR: text rect 
	CRect rcText(CPoint(0, 0), sz);

	int cxOffset = 9;

	LONG lStyle = ::GetWindowLong(pGBox->m_hWnd, GWL_STYLE);
	lStyle &= 0x00000300L;	// XR: extract the alignment property

	int nAlign = -1;		// XR: for later user
	if (lStyle == BS_CENTER)
	{
		cxOffset = (rcGBox.Width() - sz.cx) / 2;
		nAlign = 0;
	}
	else if (lStyle == BS_RIGHT)
	{
		cxOffset = rcGBox.Width() - sz.cx - 9;
		nAlign = 1;
	}

	rcText.OffsetRect(rcGBox.left + cxOffset - rcText.left,		// XM: For VC6
		rcGBox.top - rcText.top);
	//rcText.MoveToXY(rcGBox.left + cxOffset, rcGBox.top);		// XM: For VC6 or VC7
	rcText.InflateRect(2, 0, 2, 2);

	pRc[4] = rcText;

	// XR: left upper line
	pRc[5].SetRect(		
		rcGBox.left, rcGBox.top + sz.cy / 2,
		rcText.left, rcGBox.top + sz.cy / 2 + 2 );

	// XR: right upper line
	pRc[6].SetRect(		
		rcText.right, rcGBox.top + sz.cy / 2,
		rcGBox.right, rcGBox.top + sz.cy / 2 + 2 );


	// XR: Additional adjustment needed if the groupbox is not created 
	// with the default style

	int adj = 0;

	LONG lStyleEx = ::GetWindowLong(pGBox->m_hWnd, GWL_EXSTYLE);

	if ((lStyleEx & WS_EX_CLIENTEDGE) == WS_EX_CLIENTEDGE)   
		adj += 2;
	if ((lStyleEx & WS_EX_DLGMODALFRAME) == WS_EX_DLGMODALFRAME) 
		adj += 3;
	if ((lStyleEx & WS_EX_STATICEDGE) == WS_EX_STATICEDGE &&
		(lStyleEx & WS_EX_DLGMODALFRAME) != WS_EX_DLGMODALFRAME) 
		adj += 1;

	if (adj != 0)
	{
		::AdjGroupBoxRgn(pRc, adj, nAlign);
	}
	else
	{
		for (int i = 7; i < 10; i++)
			pRc[i].SetRectEmpty();
	}
}

void AdjGroupBoxRgn(CRect* pRc, int a, int nAlign)
{
	ASSERT(nAlign >= -1 && nAlign <= 1);

	pRc[1].InflateRect( 0,         a,		   0,	0);
	pRc[2].InflateRect( 0,        -a,		   a,   0);
	pRc[3].InflateRect( a,        -a,		   0,   0);
	pRc[4].InflateRect( a*nAlign, -a,  -a*nAlign,   a);
	pRc[5].InflateRect( 0,        -a,  -a*nAlign,   a);
	pRc[6].InflateRect( a*nAlign, -a,		   0,   a);

	pRc[7].SetRect(pRc[0].left, pRc[0].top, pRc[0].left + a, pRc[5].top);
	pRc[8].SetRect(pRc[0].right - a, pRc[0].top, pRc[0].right, pRc[6].top);
	pRc[9].SetRect(pRc[0].left, pRc[0].top, pRc[0].right, pRc[0].top + a);
}

void GetWindowTextSize(const CWnd* pWnd, CSize& sz)
{
	ASSERT(::IsWindow(pWnd->m_hWnd));

	CString str;
	pWnd->GetWindowText(str);
	if (str.IsEmpty())
		str = _T("a");		// XR: To avoid getting zero valued sz.

	CClientDC dc((CWnd *)pWnd);
	CFont* pFont = pWnd->GetFont();
	CFont* pFontOld = dc.SelectObject(pFont);

	sz = dc.GetTextExtent(str);
	
	dc.SelectObject(pFontOld);
}

BOOL IsGroupBox(const CWnd *pWnd)
{
	HWND hWnd = pWnd->m_hWnd;
	ASSERT(::IsWindow(hWnd));

	CString str;
	::GetClassName(hWnd, str.GetBuffer(100), 100);
	str.ReleaseBuffer();

	if (str != _T("Button"))
		return FALSE;

	UINT nStyle = ((CButton *)pWnd)->GetButtonStyle();
	nStyle &= 0x0000000FL;

	return nStyle == BS_GROUPBOX;
}
