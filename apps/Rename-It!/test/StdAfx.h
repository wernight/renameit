#pragma once

#include "../StdAfx.h"
#include <boost/test/unit_test.hpp>

// Displays CString as a string in test results.
ostream& operator<<(ostream& os, const CString& value);
