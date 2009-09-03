// SimpleExt.cpp : Implementation of DLL Exports.


// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL, 
//      run nmake -f SimpleExtps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "SimpleExt.h"

#include "SimpleExt_i.c"
#include "SimpleShlExt.h"


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_SimpleShlExt, CSimpleShlExt)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		_Module.Init(ObjectMap, hInstance, &LIBID_SIMPLEEXTLib);
		DisableThreadLibraryCalls(hInstance);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
		_Module.Term();
	return TRUE;    // ok
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
	return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
	// If we're on NT, add ourselves to the list of approved shell extensions.

	// Note that you should *NEVER* use the overload of CRegKey::SetValue with
	// 4 parameters.  It lets you set a value in one call, without having to 
	// call CRegKey::Open() first.  However, that version of SetValue() has a
	// bug in that it requests KEY_ALL_ACCESS to the key.  That will fail if the
	// user is not an administrator.  (The code should request KEY_WRITE, which
	// is all that's necessary.)

	if ( 0 == (GetVersion() & 0x80000000UL) )
	{
		CRegKey reg;
		LONG    lRet;

		lRet = reg.Open ( HKEY_LOCAL_MACHINE,
			_T("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"),
			KEY_SET_VALUE );

		if ( ERROR_SUCCESS != lRet )
			return E_ACCESSDENIED;

		TCHAR szReg[]= _T("{A64BBF5F-1250-4083-924C-B79661B75AAE}");
		lRet = reg.SetValue(_T("Rename-It! extension"), REG_SZ,szReg,lstrlen(szReg));

		if ( ERROR_SUCCESS != lRet )
			return E_ACCESSDENIED;
	}

	// registers object, typelib and all interfaces in typelib
	return _Module.RegisterServer(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
	// If we're on NT, remove ourselves from the list of approved shell extensions.
	// Note that if we get an error along the way, I don't bail out since I want
	// to do the normal ATL unregistration stuff too.

	if (!(GetVersion() & 0x80000000UL))
	{
		CRegKey reg;
		LONG    lRet;

		lRet = reg.Open ( HKEY_LOCAL_MACHINE,
			_T("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"),
			KEY_SET_VALUE );

		if ( ERROR_SUCCESS == lRet )
			lRet = reg.DeleteValue ( _T("{A64BBF5F-1250-4083-924C-B79661B75AAE}") );
	}

	return _Module.UnregisterServer(TRUE);
}
