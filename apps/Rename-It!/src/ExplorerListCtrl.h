#if !defined(AFX_DRAGDROPLISTCTRL_H__17F6D56D_591C_4319_B0EB_9D6D9B45BC79__INCLUDED_)
#define AFX_DRAGDROPLISTCTRL_H__17F6D56D_591C_4319_B0EB_9D6D9B45BC79__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DragDropListCtrl.h : header file
//

#define NM_ELMOVEDITEM	0x002001
struct NMELMOVEITEM
{
	NMHDR	hdr;
	UINT	nFrom;
	UINT	nTo;
};

/////////////////////////////////////////////////////////////////////////////
// CExplorerListCtrl window

class CExplorerListCtrl : public CListCtrl
{
protected:
	CDWordArray			m_anDragIndexes;
	int					m_nDropIndex;
	CImageList*			m_pDragImage;
	int					m_nPrevDropIndex;
	UINT				m_uPrevDropState;
	DWORD				m_dwStyle;

	enum EScrollDirection
	{
		scrollNull,
		scrollUp,
		scrollDown
	};
	EScrollDirection	m_ScrollDirection;
	UINT				m_uScrollTimer;

// Construction
public:
	CExplorerListCtrl();
	bool IsDragging() const { return m_pDragImage != NULL; }

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExplorerListCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CExplorerListCtrl();

protected:
	void DropItem();
	void RestorePrevDropItemState();
	void UpdateSelection(int nDropIndex);
	void SetScrollTimer(EScrollDirection ScrollDirection);
	void KillScrollTimer();
	CImageList* CreateDragImageEx(LPPOINT lpPoint);
	
// Generated message map functions
protected:
	//{{AFX_MSG(CExplorerListCtrl)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DRAGDROPLISTCTRL_H__17F6D56D_591C_4319_B0EB_9D6D9B45BC79__INCLUDED_)
