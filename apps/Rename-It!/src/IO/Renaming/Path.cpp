#include "StdAfx.h"
#include "Path.h"

namespace Beroux{ namespace IO{ namespace Renaming
{ 
	ostream& operator <<(ostream& os, const CPath& value)
	{
		char szBuffer[2048];
		szBuffer[WideCharToMultiByte(CP_ACP, 0, value.GetPath(), -1, szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]), NULL, NULL)] = '\0';
		os << szBuffer;

		return os;
	}
}}}
