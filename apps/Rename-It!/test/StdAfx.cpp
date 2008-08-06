#include "StdAfx.h"

ostream& operator<<(ostream& os, const CString& value)
{
#ifdef _UNICODE
	char szBuffer[1024];
	szBuffer[WideCharToMultiByte(CP_ACP, 0, value, -1, szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]), NULL, NULL)] = '\0';
	os << (LPCSTR)szBuffer;
#else
	os << (LPCSTR)value;
#endif

	return os;
}
