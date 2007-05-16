
///////////////////////////////////////////////////////////////////////
// #include "DetectNonDotNetCC.h"
//
#ifndef SET_PRE_DOTNET_CONDITIONAL_COMPILE_MACRO_SYMBOL
#define SET_PRE_DOTNET_CONDITIONAL_COMPILE_MACRO_SYMBOL

// Set symbol to indicate when using MSVC6 or earlier (MSVC7 (DotNet) is default for project)
#ifdef _MSC_VER
    #if !defined(USING_VC6_OR_EARLIER) && (_MSC_VER < 1300)
        #define USING_VC6_OR_EARLIER
    #endif
#endif

#endif // #ifndef SET_PRE_DOTNET_CONDITIONAL_COMPILE_MACRO_SYMBOL

