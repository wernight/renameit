#include "StdAfx.h"
#include <boost/test/unit_test.hpp>
#include "IO/Renaming/Filter/FilteredPath.h"

using namespace Beroux::IO::Renaming;
using namespace Beroux::IO::Renaming::Filter;


BOOST_AUTO_TEST_SUITE(FilteredPathTestSuite);

BOOST_AUTO_TEST_CASE(SingleFilteredSubstring)
{
	BOOST_STATIC_ASSERT(CFilteredPath::renameVersion == 100);

	// Single parts.
	{
		CPath path(_T("\\\\?\\C:\\ParentFolder\\..Sub.Folder\\.File.Name.Extension"));
		BOOST_CHECK_EQUAL("C:\\", CFilteredPath(path, CFilteredPath::renameRoot).GetFilteredSubstring());
		BOOST_CHECK_EQUAL("ParentFolder\\..Sub.Folder", CFilteredPath(path, CFilteredPath::renameFoldersPath).GetFilteredSubstring());
		BOOST_CHECK_EQUAL("..Sub.Folder", CFilteredPath(path, CFilteredPath::renameLastFolder).GetFilteredSubstring());
		BOOST_CHECK_EQUAL(".File.Name", CFilteredPath(path, CFilteredPath::renameFilename).GetFilteredSubstring());
		BOOST_CHECK_EQUAL("Extension", CFilteredPath(path, CFilteredPath::renameExtension).GetFilteredSubstring());
	}
}

BOOST_AUTO_TEST_CASE(ComposedFilteredSubstring)
{
	BOOST_STATIC_ASSERT(CFilteredPath::renameVersion == 100);

	// Grouped parts.
	{
		CPath path(_T("\\\\WORKGROUP\\public\\A\\B\\C\\FileName.Ext"));
		BOOST_CHECK_EQUAL("\\\\WORKGROUP\\public\\A\\B\\C", CFilteredPath(path, CFilteredPath::renameRoot ||CFilteredPath::renameFoldersPath).GetFilteredSubstring());
		BOOST_CHECK_EQUAL("FileName.Ext", CFilteredPath(path, CFilteredPath::renameFilename || CFilteredPath::renameExtension).GetFilteredSubstring());
		BOOST_CHECK_EQUAL("\\\\WORKGROUP\\public\\A\\B\\C\\FileName", CFilteredPath(path, CFilteredPath::renameRoot ||CFilteredPath::renameFoldersPath || CFilteredPath::renameFilename).GetFilteredSubstring());
	}
}

BOOST_AUTO_TEST_CASE(StrangePathFilteredSubstring)
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
		BOOST_CHECK_EQUAL("", CFilteredPath(path, CFilteredPath::renameFoldersPath || CFilteredPath::renameFilename).GetFilteredSubstring());
		BOOST_CHECK_EQUAL("", CFilteredPath(path, CFilteredPath::renameLastFolder).GetFilteredSubstring());
	}

	// No folders.
	{
		CPath path(_T("\\\\?\\Network\\ShareName\\FileName.Ext"));
		BOOST_CHECK_EQUAL("", CFilteredPath(path, CFilteredPath::renameFilename).GetFilteredSubstring());
		BOOST_CHECK_EQUAL("", CFilteredPath(path, CFilteredPath::renameFoldersPath).GetFilteredSubstring());
		BOOST_CHECK_EQUAL("", CFilteredPath(path, CFilteredPath::renameLastFolder).GetFilteredSubstring());
		BOOST_CHECK_EQUAL("", CFilteredPath(path, CFilteredPath::renameRoot || CFilteredPath::renameFoldersPath).GetFilteredSubstring());
	}
}

BOOST_AUTO_TEST_CASE(BadSelectionPart)
{
	CPath pathSimple(_T("C:\\FileName.txt"));
	BOOST_CHECK_THROW(CFilteredPath(pathSimple, 0), exception);
	BOOST_CHECK_THROW(CFilteredPath(pathSimple, CFilteredPath::renameLastFolder || CFilteredPath::renameFilename), exception);
	BOOST_CHECK_THROW(CFilteredPath(pathSimple, CFilteredPath::renameVersion), exception);
}

// TODO: Test setting the selection part especially when there only a C:\\ at first.

BOOST_AUTO_TEST_SUITE_END();
