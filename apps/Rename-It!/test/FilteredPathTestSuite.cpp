#include "StdAfx.h"
#include <boost/test/unit_test.hpp>
#include "IO/Renaming/Filter/FilteredPath.h"

using namespace Beroux::IO::Renaming;
using namespace Beroux::IO::Renaming::Filter;


BOOST_AUTO_TEST_SUITE(FilteredPathTestSuite);

BOOST_AUTO_TEST_CASE(GetSingleFilteredSubstring)
{
	BOOST_STATIC_ASSERT(CFilteredPath::renameVersion == 100);

	// Single parts of a file path.
	{
		CPath path(_T("C:\\ParentFolder\\..Sub.Folder\\.File.Name.Extension"));
		BOOST_REQUIRE_EQUAL(".File.Name.Extension", path.GetFileName());

		BOOST_CHECK_EQUAL("C:\\", CFilteredPath(path, CFilteredPath::renameRoot).GetFilteredSubstring());
		BOOST_CHECK_EQUAL("ParentFolder\\..Sub.Folder\\.File.Name.Extension", CFilteredPath(path, CFilteredPath::renameFoldersPath).GetFilteredSubstring());
		BOOST_CHECK_EQUAL(".File.Name.Extension", CFilteredPath(path, CFilteredPath::renameLastFolder).GetFilteredSubstring());
		BOOST_CHECK_EQUAL(".File.Name", CFilteredPath(path, CFilteredPath::renameFilename).GetFilteredSubstring());
		BOOST_CHECK_EQUAL(".Extension", CFilteredPath(path, CFilteredPath::renameExtension).GetFilteredSubstring());
	}

	// Single parts of a directory path.
	{
		CPath path(_T("C:\\ParentFolder\\..Sub.Folder\\.Directory.Name.Extension"));
		BOOST_REQUIRE_EQUAL(".Directory.Name.Extension", path.GetFileName());

		BOOST_CHECK_EQUAL("C:\\", CFilteredPath(path, CFilteredPath::renameRoot).GetFilteredSubstring());
		BOOST_CHECK_EQUAL("ParentFolder\\..Sub.Folder\\.Directory.Name.Extension", CFilteredPath(path, CFilteredPath::renameFoldersPath).GetFilteredSubstring());
		BOOST_CHECK_EQUAL(".Directory.Name.Extension", CFilteredPath(path, CFilteredPath::renameLastFolder).GetFilteredSubstring());
		BOOST_CHECK_EQUAL(".Directory.Name", CFilteredPath(path, CFilteredPath::renameFilename).GetFilteredSubstring());
		BOOST_CHECK_EQUAL(".Extension", CFilteredPath(path, CFilteredPath::renameExtension).GetFilteredSubstring());
	}
}

BOOST_AUTO_TEST_CASE(GetComposedFilteredSubstring)
{
	BOOST_STATIC_ASSERT(CFilteredPath::renameVersion == 100);

	// Grouped parts.
	CPath path(_T("\\\\WORKGROUP\\public\\A\\B\\C\\Folder.Ext"));
	BOOST_CHECK_EQUAL("\\\\WORKGROUP\\public\\A\\B\\C\\Folder.Ext", CFilteredPath(path, CFilteredPath::renameRoot | CFilteredPath::renameFoldersPath).GetFilteredSubstring());
	BOOST_CHECK_EQUAL("Folder.Ext", CFilteredPath(path, CFilteredPath::renameFilename | CFilteredPath::renameExtension).GetFilteredSubstring());
	BOOST_CHECK_EQUAL("\\\\WORKGROUP\\public\\A\\B\\C\\Folder", CFilteredPath(path, CFilteredPath::renameRoot | CFilteredPath::renameFoldersPath | CFilteredPath::renameFilename).GetFilteredSubstring());
}

BOOST_AUTO_TEST_CASE(ExcludesUnicodeRoot)
{
	{
		CFilteredPath filteredPath(CPath(_T("\\\\?\\C:\\")),
			CFilteredPath::renameRoot);
		BOOST_CHECK_EQUAL("C:\\", filteredPath.GetFilteredSubstring());

		filteredPath.SetFilteredSubstring(_T("X:\\"));
		BOOST_CHECK_EQUAL("X:\\", filteredPath.GetFilteredSubstring());
		BOOST_CHECK_EQUAL("\\\\?\\X:\\", filteredPath.GetFilteredPath());

		filteredPath.SetFilteredSubstring(_T("\\\\foo\\bar\\hi\\"));
		BOOST_CHECK_EQUAL("\\\\foo\\bar\\hi\\", filteredPath.GetFilteredSubstring());
		BOOST_CHECK_EQUAL("\\\\?\\UNC\\foo\\bar\\hi", static_cast<CPath>(filteredPath).GetPath());
	}

	{
		CFilteredPath filteredPath(CPath(_T("\\\\?\\UNC\\Network\\")),
			CFilteredPath::renameRoot);
		BOOST_CHECK_EQUAL("\\\\Network\\", filteredPath.GetFilteredSubstring());

		filteredPath.SetFilteredSubstring(_T("\\\\foo\\"));
		BOOST_CHECK_EQUAL("\\\\foo\\", filteredPath.GetFilteredSubstring());
		BOOST_CHECK_EQUAL("\\\\?\\UNC\\foo\\", filteredPath.GetFilteredPath());

		filteredPath.SetFilteredSubstring(_T("C:\\hi"));
		BOOST_CHECK_EQUAL("C:\\hi", filteredPath.GetFilteredSubstring());
		BOOST_CHECK_EQUAL("\\\\?\\C:\\hi", filteredPath.GetFilteredPath());
	}
}

BOOST_AUTO_TEST_CASE(GetStrangePathFilteredSubstring)
{
	// Empty path.
	{
		CPath path(_T(""));
		BOOST_CHECK_EQUAL("", CFilteredPath(path, CFilteredPath::renameFilename).GetFilteredSubstring());
	}

	// Only the root.
	{
		CPath path(_T("C:\\"));
		BOOST_CHECK_EQUAL("", CFilteredPath(path, CFilteredPath::renameFilename).GetFilteredSubstring());
		BOOST_CHECK_EQUAL("", CFilteredPath(path, CFilteredPath::renameExtension).GetFilteredSubstring());
		BOOST_CHECK_EQUAL("", CFilteredPath(path, CFilteredPath::renameFoldersPath | CFilteredPath::renameFilename).GetFilteredSubstring());
		BOOST_CHECK_EQUAL("", CFilteredPath(path, CFilteredPath::renameLastFolder).GetFilteredSubstring());
	}

	{
		CPath path(_T("\\\\?\\UNC\\Network\\"));
		BOOST_CHECK_EQUAL("", CFilteredPath(path, CFilteredPath::renameFilename).GetFilteredSubstring());
		BOOST_CHECK_EQUAL("", CFilteredPath(path, CFilteredPath::renameExtension).GetFilteredSubstring());
		BOOST_CHECK_EQUAL("", CFilteredPath(path, CFilteredPath::renameFoldersPath | CFilteredPath::renameFilename).GetFilteredSubstring());
		BOOST_CHECK_EQUAL("", CFilteredPath(path, CFilteredPath::renameLastFolder).GetFilteredSubstring());
	}
}

BOOST_AUTO_TEST_CASE(BadSelectionPart)
{
	CPath pathSimple(_T("C:\\FileName.txt"));
	BOOST_CHECK_THROW(CFilteredPath(pathSimple, 0), std::exception);
	BOOST_CHECK_THROW(CFilteredPath(pathSimple, CFilteredPath::renameLastFolder | CFilteredPath::renameFilename), std::exception);
	BOOST_CHECK_THROW(CFilteredPath(pathSimple, CFilteredPath::renameVersion), std::exception);
}

BOOST_AUTO_TEST_CASE(SetFilteredSubstring)
{
	// ... on a single folder.
	{
		CFilteredPath filteredPath(CPath(_T("\\\\?\\C:\\ParentFolder\\..Sub.Folder\\.File.Name.Extension")),
			CFilteredPath::renameLastFolder);

		filteredPath.SetFilteredSubstring(_T("Foo"));
		BOOST_CHECK_EQUAL("Foo", filteredPath.GetFilteredSubstring());

		filteredPath.SetFilteredSubstring(_T("foo\\bar"));
		BOOST_CHECK_EQUAL("foo\\bar", filteredPath.GetFilteredSubstring());
	}

	// ... on a composed selection.
	{
		CFilteredPath filteredPath(CPath(_T("\\\\?\\C:\\ParentFolder\\..Sub.Folder\\.File.Name.Extension")),
			CFilteredPath::renameRoot | CFilteredPath::renameFoldersPath);

		filteredPath.SetFilteredSubstring(_T("Foo"));
		BOOST_CHECK_EQUAL("Foo", filteredPath.GetFilteredSubstring());

		filteredPath.SetFilteredSubstring(_T("foo\\bar"));
		BOOST_CHECK_EQUAL("foo\\bar", filteredPath.GetFilteredSubstring());

		filteredPath.SetFilteredSubstring(_T("C:\\foo\\bar\\"));
		BOOST_CHECK_EQUAL("C:\\foo\\bar\\", filteredPath.GetFilteredSubstring());
	}

	// Test setting the selection part when there only a C:\\ at first.
	{
		CFilteredPath filteredPath(CPath(_T("\\\\?\\C:\\")),
			CFilteredPath::renameFilename);

		filteredPath.SetFilteredSubstring(_T("hello there"));
		BOOST_CHECK_EQUAL("hello there", filteredPath.GetFilteredSubstring());
		BOOST_CHECK_EQUAL("\\\\?\\C:\\hello there", filteredPath.GetFilteredPath());
	}

	{
		CFilteredPath filteredPath(CPath(_T("C:\\")),
			CFilteredPath::renameLastFolder);
		BOOST_CHECK_EQUAL("", filteredPath.GetFilteredSubstring());

		filteredPath.SetFilteredSubstring(_T("hello there"));
		BOOST_CHECK_EQUAL("hello there", filteredPath.GetFilteredSubstring());
		BOOST_CHECK_EQUAL("C:\\hello there", filteredPath.GetFilteredPath());
	}
}

BOOST_AUTO_TEST_SUITE_END();
