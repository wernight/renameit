// SimpleShlExt.h : Declaration of the CSimpleShlExt

#ifndef __SIMPLESHLEXT_H_
#define __SIMPLESHLEXT_H_

#include "resource.h"       // main symbols
#include <shlobj.h>
#include <comdef.h>

#include <atlstr.h> // ATL now supports CString

struct __declspec(uuid("000214e4-0000-0000-c000-000000000046")) IContextMenu;

/////////////////////////////////////////////////////////////////////////////
// CSimpleShlExt
class ATL_NO_VTABLE CSimpleShlExt : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSimpleShlExt, &CLSID_SimpleShlExt>,
	public IDispatchImpl<ISimpleShlExt, &IID_ISimpleShlExt, &LIBID_SIMPLEEXTLib>,
	public IShellExtInit,
	public IContextMenu
{
public:
	CSimpleShlExt();
	~CSimpleShlExt();

	DECLARE_REGISTRY_RESOURCEID(IDR_SIMPLESHLEXT)

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	BEGIN_COM_MAP(CSimpleShlExt)
		COM_INTERFACE_ENTRY(ISimpleShlExt)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IShellExtInit)
		COM_INTERFACE_ENTRY(IContextMenu)
	END_COM_MAP()

// ISimpleShlExt
private:
	CString m_strFilesBuff = L"";
	HBITMAP	m_hMenuBmp;
	/*LPTSTR*/ LPWSTR	m_szFiles;
	UINT	m_nFilesLength;

	void MyFormatMessage(UINT nResourceID, ...);

	

public:
	// IShellExtInit
	STDMETHOD(Initialize)(LPCITEMIDLIST, LPDATAOBJECT, HKEY);

	// IContextMenu
	STDMETHOD(GetCommandString)(UINT_PTR, UINT, UINT*, LPSTR, UINT);
	STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO);
	STDMETHOD(QueryContextMenu)(HMENU, UINT, UINT, UINT, UINT);
};

#endif //__SIMPLESHLEXT_H_
