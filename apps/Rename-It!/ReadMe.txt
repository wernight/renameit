Rename-It!

Folders:
	/						RenameIt folder.
	/Install/				NSIS installation program and some files to
							include in release versions.
	/Plug-ins/				Source code of plug-ins.
	/Plug-ins/SDK/			Go there to create a new plug-in.
	/res/					Rename-It.exe resource folder.
	/SimpleExt/				The Shell extension to create a Rename-It
							shortcut in the Explorer contect menu.
	../bin/					Folder created during compile to contain binaries.
	../tmp/					Folder created during compile to contain temp files.

Important files:
	/ToDo.txt				Things that should be done.
	/RenameIt.sln			Main project solution file for Visual Studio 7.
	/Install/History.txt	Changes made from one version to another.
	/Install/NSIS.NSI		Nullsoft Scriptable Install System file.
							This file contains source code for installation.
	/Plug-ins/SDK/VC7/SDK.sdl	Solution file to create new plug-ins.

FAQ:
	[Q] How to start changing the Rename-It program's code?
	Open the RenameIt.sln and look a ToDo.txt to see things that
	would be cool to upgrade.

	[Q] How to create a new filter?
	Each filter is in fact, a plug-in. If you have VisualStudio, open
	Plug-ins/SDK/VC7/SDK.sln and add your things then compile. If you
	want to use another software, best would be to see FilterDLL.h in
	Plug-ins/SDK/Other and implement those functions in your language.
	Once compiled, copy the generated DLL file in the Rename-It Plug-ins
	sub folder and start Rename-It to test it.

	[Q] What is the file XXXX used for?
	Most files have some commented sections at start. Some times the
	comments are in the header file, sometimes in the CPP file.

	[Q] I have only Visual Studio 6 or earlier, what can I do?
	Since I didn't kept the old files for VC6 you'll have to create a
	new project and add the files to it. May be some part of the soft
	will have to be changed to fit VC6, that why a DetectNonDotNetCC.h
	file has been provided.
