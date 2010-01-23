#include "StdAfx.h"

ostream& operator<<(ostream& os, const CString& value)
{
	char szBuffer[1024];
	szBuffer[WideCharToMultiByte(CP_ACP, 0, value, -1, szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]), NULL, NULL)] = '\0';
	os << (LPCSTR)szBuffer;

	return os;
}
