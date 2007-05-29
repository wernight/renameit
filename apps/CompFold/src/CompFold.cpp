// CompFold.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

unsigned CompareFolders(const wstring& strFolder1, const wstring& strFolder2);
vector<wstring> Dir(const wstring& strFolder);
string WstringToString(const wstring& wstr);

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 3)
	{
		cout << "You must provide 2 folders as an argument, and nothing more or less." << endl;
		cout << "COMPFOLD directory_1 directory_2" << endl;
	}
	else
	{
		wstring strFolder1 = argv[1];
		wstring strFolder2(argv[2]);
		return CompareFolders(strFolder1, strFolder2);
	}
	return 0;
}

// Compare two folders and return the number of files that don't match.
unsigned CompareFolders(const wstring& strFolder1, const wstring& strFolder2)
{
	// Get a list of files to compare: FilesList = Files_in_folder1 + Files_in_folder2
	vector<wstring> vFilesList1 = Dir(strFolder1);
	vector<wstring> vFilesList2 = Dir(strFolder2);
	vector<wstring> vFilesList(vFilesList1);
	for (vector<wstring>::iterator iter=vFilesList2.begin(); iter!=vFilesList2.end(); ++iter)
	{
		// If the file/folder *iter from Folder2 is not in Folder1
		if (find(vFilesList.begin(), vFilesList.end(), *iter) == vFilesList.end())
		{
			// Add that file/folder to the list
			vFilesList.push_back(*iter);
		}
	}

	// Add '\' at the end of the folders
	wstring strFullFolder1(strFolder1);
	if (strFullFolder1[strFullFolder1.length() - 1] != _T('\\'))
		strFullFolder1 += _T("\\");

	wstring strFullFolder2(strFolder2);
	if (strFullFolder2[strFullFolder2.length() - 1] != _T('\\'))
		strFullFolder2 += _T("\\");

	// For each file of the list
	unsigned nMismatch = 0;
	for (vector<wstring>::iterator iter=vFilesList.begin(); iter!=vFilesList.end(); ++iter)
	{
		// If it is a folder
		if ((*iter)[(*iter).length() - 1] == '\\')
		{
			// Do a recursive comparaison
			nMismatch += CompareFolders(strFullFolder1 + *iter, strFullFolder2 + *iter);
			continue;
		}

		// Open both files
		wstring strFile1 = strFullFolder1;
		strFile1 += *iter;
		FILE* f1 = _wfopen(strFile1.c_str(), _T("rb"));

		wstring strFile2 = strFullFolder2;
		strFile2 += *iter;
		FILE* f2 = _wfopen(strFile2.c_str(), _T("rb"));

		// Check that the file is in both folders
		if (find(vFilesList1.begin(), vFilesList1.end(), *iter) == vFilesList1.end())
		{
			// If this file could be opened in both folders,
			// it means that the case is different only
			if (f1 != NULL && f2 != NULL)
				cout << "\"" << WstringToString(*iter) << "\" has case mismatch in \"" << WstringToString(strFolder1) << "\"." << endl;
			else
				cout << "\"" << WstringToString(*iter) << "\" is missing in \"" << WstringToString(strFolder1) << "\"." << endl;
			++nMismatch;
		}
		else if (find(vFilesList2.begin(), vFilesList2.end(), *iter) == vFilesList2.end())
		{
			// If this file could be opened in both folders,
			// it means that the case is different only
			if (f1 != NULL && f2 != NULL)
				cout << "\"" << WstringToString(*iter) << "\" has case mismatch in \"" << WstringToString(strFolder2) << "\"." << endl;
			else
				cout << "\"" << WstringToString(*iter) << "\" is missing in \"" << WstringToString(strFolder2) << "\"." << endl;
			++nMismatch;
		}
		else
		{
			if (f1 == NULL)
			{
				cout << "Cannot open the file: " << WstringToString(strFile1) << endl;
				++nMismatch;
			}
			else if (f2 == NULL)
			{
				cout << "Cannot open the file: " << WstringToString(strFile2) << endl;
				++nMismatch;
			}
			else
			{
				// Compare the content of both files
				bool bMatch = true;
				while (bMatch && (!feof(f1) && !ferror(f1)) || (!feof(f2) && !ferror(f2)))
				{
					char	lpBuff1[1024],
							lpBuff2[1024];
					size_t nReadSize1 = fread(lpBuff1, sizeof(char), sizeof(lpBuff1)/sizeof(char), f1);
					size_t nReadSize2 = fread(lpBuff2, sizeof(char), sizeof(lpBuff2)/sizeof(char), f2);
					if (nReadSize1 != nReadSize2 || memcmp(lpBuff1, lpBuff2, nReadSize1) != 0)
						bMatch = false;
				}
				bMatch &= feof(f1) && feof(f2);

				if (!bMatch)
				{
					cout << "\"" << WstringToString(strFile1) << "\" and \"" << WstringToString(strFile2) << "\" differ." << endl;
					++nMismatch;
				}
			}
		}

		// Close files
		if (f1 != NULL)
			fclose(f1);
		if (f2 != NULL)
			fclose(f2);
	}

	return nMismatch;
}

// Return a list a files in a folder.
vector<wstring> Dir(const wstring& strFolder)
{
	vector<wstring> vDir;

	// Search "strFolder\*.*"
	wstring wstrWildcardSearch(strFolder);
	if (wstrWildcardSearch[wstrWildcardSearch.length() - 1] != _T('\\'))
		wstrWildcardSearch += _T("\\");
	wstrWildcardSearch += _T("*.*");

	// Start the directory listing
	HANDLE hFindFile;
	WIN32_FIND_DATA fd;
	if ((hFindFile = FindFirstFile(wstrWildcardSearch.c_str(), &fd)) != INVALID_HANDLE_VALUE)
		do
		{
			if (_tcscmp(fd.cFileName, _T(".")) != 0 &&
				_tcscmp(fd.cFileName, _T("..")) != 0 &&
				_tcscmp(fd.cFileName, _T(".svn")) != 0)
			{
				// File or Folder?
				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					wstring wstrFolderName(fd.cFileName);
					wstrFolderName += L"\\";
					vDir.push_back(wstrFolderName);
				}
				else
					vDir.push_back(fd.cFileName);
			}
		} while (FindNextFile(hFindFile, &fd));
	FindClose(hFindFile);

	return vDir;
}

string WstringToString(const wstring& wstr)
{
	// Convert to a char*
	string str;
	for (wstring::const_iterator iter=wstr.begin(); iter!=wstr.end(); ++iter)
		if ((*iter & 0x00FF) == *iter)
			str += (char)( *iter & 0xFF );
		else
			str += "?";
	return str;
}