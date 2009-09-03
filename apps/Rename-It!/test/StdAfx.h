#pragma once

#include "../StdAfx.h"
#include <boost/test/unit_test.hpp>
#include <WinIoCtl.h>	// Used to have DeviceIoControl and FSCTL_SET_COMPRESSION.

// Displays CString as a string in test results.
ostream& operator<<(ostream& os, const CString& value);
