===========================
 Usage
===========================

Provides many usefull functions for general purpose and for unit testing also.

For unit testing I recommend taking a look at (for Boost 1.35):
- C:\Program Files\boost\boost_1_35_0\libs\test\example\unit_test_example_05.cpp
- C:\Program Files\boost\boost_1_35_0\libs\test\example\unit_test_example_06.cpp


===========================
 Already Installed?
===========================

If you have a folder in named %PROGRAMFILES%\Boost, you probably already have a
version of Boost installed.


===========================
 Installation
===========================

This installation is using the recommended easier way on Windows by
using the Boost installer from BoostPro Computing.

1. Download the latest Boost installer from BoostPro Computing:
   http://www.boostpro.com/products/free

2. Install the setup:
   a. Select whatever mirror you like.
   b. It is advised to checked those checked with a cross below:
      [ ] Visual C++ 8.0 (Visual Studio 2005)
      [X] Visual C++ 9.0 (Visual Studio 2008)

      [ ] Mulithread Debug, DLL --DLL Debug, requires BOOST_DYN_LINK defined.
      [ ] Mulithread, DLL       --DLL Release, requires BOOST_DYN_LINK defined.
      [X] Mulithread            --LIB (for CRT DLL) Release, default in Release.
      [X] Mulithread Debug      --LIB (for CRT DLL) Debug, default in Debug.
      [X] Mulithread, static runtime       --LIB (for CRT LIB) Release.
      [ ] Mulithread Debug, static runtime --LIB (for CRT LIB) Debug.
      [ ] Single thread, static runtime
      [ ] Single thread Debug, static runtime
   c. Keep the default options on the next screen.
   d. Wait for the download and installation to complete. In case of error, restart.

3. Add Boost to the include and library folders of Visual Studio:
   a. Open Visual Studio
   b. Go to the menu Tools > Options...
   c. Go to "Project and Solutions" > "VC++ Directories"
   d. Add to your "Include files": C:\Program Files\boost\boost_1_38
   e. Add to your "Include files": C:\Program Files\boost\boost_1_38\boost\tr1
   f. Add to your "Library files": C:\Program Files\boost\boost_1_38\lib

4. (optional) Documentation shortcut:
   Create a shortcut to "C:\Program Files\boost\boost_*\more\index.htm"

5. Done.
