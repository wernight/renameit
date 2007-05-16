========================================================================
	WHAT IS IT?
========================================================================

By executing "run_all.bat" it will test if "Rename-It!.exe" renames some files correctly using the batch mode.


========================================================================
	HOW TO ADD A NEW TEST CASE
========================================================================

Create a folder "my_test_case" with the following structure.

Usually tests have the following structure:
	my_test_case\
	 +- filter.rit	Filter that will be applied on the files.
	 +- before\		Put there files/folders before renaming.
	 +- after.ref\	Put there files/folders as they should be after renaming.

Warning: Files in "before\" and "after.ref\" SHOULD be DIFFERENT.


========================================================================
	HOW IT WORKS
========================================================================

The filter is applyed to the files in the folder "before\" and then compared to the folder "after\". If one or more files/folders don't match, the test fails.


========================================================================
	UPDATES REMARQUES
========================================================================

"Rename-It!.exe" and "CompFold.exe" should be updated to the last version of those files.
