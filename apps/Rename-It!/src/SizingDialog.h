////////////////////////////////////////////////////////////////////////////
// SizingDialog.h
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

#pragma once

////////////////////////////////////////////////////////////////////////////
// class CCtrlInfo

class CCtrlInfo
{
// XR: Constructor and destructor
public:
	CCtrlInfo(LPCRECT rcInit = NULL,
		double dRateLeft = 0.0,  double dRateRight    = 0.0, 
		double dRateTop  = 0.0,  double dRateBottom   = 0.0);
	virtual ~CCtrlInfo(void);

// XR: Operations
public:
	BOOL Modify(const CWnd* pCtrl, const CWnd* pParent, 
		LPCTSTR lpszString, char chSpr = '+');
	BOOL Modify(LPCRECT rcInit,
		double dRateLeft = 0.0,  double dRateRight    = 0.0, 
		double dRateTop  = 0.0,  double dRateBottom   = 0.0);

// XR: Data
public:
	CRect  m_rcInit;
	double m_dRateLeft;
	double m_dRateRight;
	double m_dRateTop;
	double m_dRateBottom;

// XR: Implementations
protected:
	BOOL ExtractOptions(LPCTSTR lpszSection, 
		const CWnd* pCtrl = NULL, const CWnd* pParent = NULL);
	void GetCoefficientAndWord(LPCTSTR lpszSection,
		CString& strWord, double& dCoef) const;
};

////////////////////////////////////////////////////////////////////////////
// CSizingDialog dialog

#include "afxcoll.h"	// XR: For CMapPtrToPtr

#define DT_STRING 0
#define DT_NUMBER 1

class CSizingDialog : public CDialog
{
	DECLARE_DYNAMIC(CSizingDialog)

// XR: Constructor and destructor
public:
	explicit CSizingDialog(UINT nIDTemplate, CWnd* pParentWnd = NULL);	// constructor													
	virtual ~CSizingDialog();

// XR: Operations
public:
	void AddResizableCtrl(UINT nID, LPCTSTR lpszString = NULL);
	void AddResizableCtrl(UINT nID,
		double dRateLeft, double dRateRight, 
		double dRateTop, double dRateBottom);

	void AddResizableCtrlRange(UINT nIDStart, UINT nIDEnd, 
		LPCTSTR lpszString = NULL);
	void ModifyResizableCtrlRange(UINT nIDStart, UINT nIDEnd, 
		LPCTSTR lpszString = NULL);
	void AddResizableCtrlRange(UINT nIDStart, UINT nIDEnd, 
		double dRateLeft, double dRateRight, 
		double dRateTop, double dRateBottom);

	void AddResizableCtrlArray(UINT nIDStart, UINT nIDEnd,
		double dSelfGain = 0.0, double dBorderIntervalRate = 1.0, BOOL bHori = TRUE);
	
// XR: Implementations
private:
	void AddCtrl(UINT nID, int nDataType, LPCVOID lpData);
	void AddCtrlRange(UINT nIDStart, UINT nIDEnd, int nDataType, LPCVOID lpData);
	void RearrangeID(UINT& nReIDS, UINT& nReIDE, 
		HWND& hCtrlS, HWND& hCtrlE, int& nNum) const;
	void ResizeControls();

public:
	void InvalidateCtrlBorder(
		LPCRECT lpRectOld, LPCRECT lpRectNew, BOOL bErase = TRUE);

// XR: Implementations
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	DECLARE_MESSAGE_MAP()

// Data
private:
	CMapPtrToPtr m_mapPtToInfo;	// 
	CRect m_rcInit;
};
