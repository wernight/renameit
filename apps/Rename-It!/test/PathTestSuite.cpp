#include "StdAfx.h"
#include <boost/test/unit_test.hpp>
#include "IO/Renaming/Path.h"

using namespace Beroux::IO::Renaming;

struct CPathFixture
{
	CPath m_pathEmpty;
	CPath m_pathSimple;
	CPath m_pathSimpleUnc;
	CPath m_pathNetwork;
	CPath m_pathNetworkUnc;
	CPath m_pathSimpleDirectory;
	CPath m_pathNetworkFileNoExtUnc;
	CPath m_pathNetworkExtNoFile;

	CPathFixture() :
		m_pathEmpty(),
		m_pathSimple(_T("C:\\ParentFolder\\..Sub.Folder\\.File.Name.Extension")),
		m_pathSimpleUnc(_T("\\\\?\\C:\\ParentFolder\\..Sub.Folder\\.File.Name.Extension")),
		m_pathNetwork(_T("\\\\Network\\ShareName\\ParentFolder\\..Sub.Folder\\.File.Name.Extension")),
		m_pathNetworkUnc(_T("\\\\?\\UNC\\Network\\Share Name\\Parent Folder\\..Sub.Folder\\.File.Name.Extension")),
		m_pathSimpleDirectory(_T("C:\\ParentFolder\\..Sub.Folder\\")),
		m_pathNetworkFileNoExtUnc(_T("\\\\?\\UNC\\Network\\Share.Name\\File")),
		m_pathNetworkExtNoFile(_T("\\\\UNC\\.ShareName\\.ext"))
	{
	}
};

BOOST_FIXTURE_TEST_SUITE(PathTestSuite, CPathFixture);

BOOST_AUTO_TEST_CASE(TrimEndingBackslash)
{
	BOOST_CHECK_EQUAL("C:\\foo", CPath(_T("C:\\foo\\")).GetPath());
	BOOST_CHECK_EQUAL("C:\\", CPath(_T("C:\\")).GetPath());

	BOOST_CHECK_EQUAL("\\\\?\\UNC\\", CPath(_T("\\\\?\\UNC\\")).GetPath());
	BOOST_CHECK_EQUAL("\\\\?\\UNC\\Network\\", CPath(_T("\\\\?\\UNC\\Network\\")).GetPath());
	BOOST_CHECK_EQUAL("\\\\?\\UNC\\Network\\ShareName\\", CPath(_T("\\\\?\\UNC\\Network\\ShareName\\")).GetPath());
	BOOST_CHECK_EQUAL("\\\\?\\UNC\\Network\\ShareName\\Dir", CPath(_T("\\\\?\\UNC\\Network\\ShareName\\Dir\\")).GetPath());
}

BOOST_AUTO_TEST_CASE(CopyConstructor)
{
	CPath copy(m_pathNetworkUnc);
	BOOST_CHECK_EQUAL(copy, m_pathNetworkUnc);
	BOOST_CHECK_EQUAL(copy.GetPath(), m_pathNetworkUnc.GetPath());
	BOOST_CHECK_EQUAL(copy.GetPathRoot(), m_pathNetworkUnc.GetPathRoot());
	BOOST_CHECK_EQUAL(copy.GetDirectoryName(), m_pathNetworkUnc.GetDirectoryName());
	BOOST_CHECK_EQUAL(copy.GetFileNameWithoutExtension(), m_pathNetworkUnc.GetFileNameWithoutExtension());
}

BOOST_AUTO_TEST_CASE(GetPath)
{
	BOOST_CHECK( m_pathEmpty.GetPath().IsEmpty() );
	BOOST_CHECK_EQUAL( m_pathSimple.GetPath(), _T("C:\\ParentFolder\\..Sub.Folder\\.File.Name.Extension") );
	BOOST_CHECK_EQUAL( m_pathSimpleUnc.GetPath(), _T("\\\\?\\C:\\ParentFolder\\..Sub.Folder\\.File.Name.Extension") );
	BOOST_CHECK_EQUAL( m_pathNetwork.GetPath(), _T("\\\\Network\\ShareName\\ParentFolder\\..Sub.Folder\\.File.Name.Extension") );
	BOOST_CHECK_EQUAL( m_pathNetworkUnc.GetPath(), _T("\\\\?\\UNC\\Network\\Share Name\\Parent Folder\\..Sub.Folder\\.File.Name.Extension") );
	BOOST_CHECK_EQUAL( m_pathSimpleDirectory.GetPath(), _T("C:\\ParentFolder\\..Sub.Folder") );
	BOOST_CHECK_EQUAL( m_pathNetworkFileNoExtUnc.GetPath(), _T("\\\\?\\UNC\\Network\\Share.Name\\File") );
	BOOST_CHECK_EQUAL( m_pathNetworkExtNoFile.GetPath(), _T("\\\\UNC\\.ShareName\\.ext") );
}

BOOST_AUTO_TEST_CASE(GetPathRoot)
{
	BOOST_CHECK( m_pathEmpty.GetPathRoot().IsEmpty() );
	BOOST_CHECK_EQUAL( m_pathSimple.GetPathRoot(), _T("C:\\") );
	BOOST_CHECK_EQUAL( m_pathSimpleUnc.GetPathRoot(), _T("\\\\?\\C:\\") );
	BOOST_CHECK_EQUAL( m_pathNetwork.GetPathRoot(), _T("\\\\Network\\ShareName\\") );
	BOOST_CHECK_EQUAL( m_pathNetworkUnc.GetPathRoot(), _T("\\\\?\\UNC\\Network\\Share Name\\") );
	BOOST_CHECK_EQUAL( m_pathSimpleDirectory.GetPathRoot(), _T("C:\\") );
	BOOST_CHECK_EQUAL( m_pathNetworkFileNoExtUnc.GetPathRoot(), _T("\\\\?\\UNC\\Network\\Share.Name\\") );
	BOOST_CHECK_EQUAL( m_pathNetworkExtNoFile.GetPathRoot(), _T("\\\\UNC\\.ShareName\\") );
}

BOOST_AUTO_TEST_CASE(GetDirectoryName)
{
	BOOST_CHECK( m_pathEmpty.GetDirectoryName().IsEmpty() );
	BOOST_CHECK_EQUAL( m_pathSimple.GetDirectoryName(), _T("C:\\ParentFolder\\..Sub.Folder\\") );
	BOOST_CHECK_EQUAL( m_pathSimpleUnc.GetDirectoryName(), _T("\\\\?\\C:\\ParentFolder\\..Sub.Folder\\") );
	BOOST_CHECK_EQUAL( m_pathNetwork.GetDirectoryName(), _T("\\\\Network\\ShareName\\ParentFolder\\..Sub.Folder\\") );
	BOOST_CHECK_EQUAL( m_pathNetworkUnc.GetDirectoryName(), _T("\\\\?\\UNC\\Network\\Share Name\\Parent Folder\\..Sub.Folder\\") );
	BOOST_CHECK_EQUAL( m_pathSimpleDirectory.GetDirectoryName(), _T("C:\\ParentFolder\\") );
	BOOST_CHECK_EQUAL( m_pathNetworkFileNoExtUnc.GetDirectoryName(), _T("\\\\?\\UNC\\Network\\Share.Name\\") );
	BOOST_CHECK_EQUAL( m_pathNetworkExtNoFile.GetDirectoryName(), _T("\\\\UNC\\.ShareName\\") );
}

BOOST_AUTO_TEST_CASE(GetFileName)
{
	BOOST_CHECK( m_pathEmpty.GetFileName().IsEmpty() );
	BOOST_CHECK_EQUAL( m_pathSimple.GetFileName(), _T(".File.Name.Extension") );
	BOOST_CHECK_EQUAL( m_pathSimpleUnc.GetFileName(), _T(".File.Name.Extension") );
	BOOST_CHECK_EQUAL( m_pathNetwork.GetFileName(), _T(".File.Name.Extension") );
	BOOST_CHECK_EQUAL( m_pathNetworkUnc.GetFileName(), _T(".File.Name.Extension") );
	BOOST_CHECK_EQUAL( m_pathSimpleDirectory.GetFileName(), _T("..Sub.Folder") );
	BOOST_CHECK_EQUAL( m_pathNetworkFileNoExtUnc.GetFileName(), _T("File") );
	BOOST_CHECK_EQUAL( m_pathNetworkExtNoFile.GetFileName(), _T(".ext") );
}

BOOST_AUTO_TEST_CASE(GetFileNameWithoutExtension)
{
	BOOST_CHECK( m_pathEmpty.GetFileNameWithoutExtension().IsEmpty() );
	BOOST_CHECK_EQUAL( m_pathSimple.GetFileNameWithoutExtension(), _T(".File.Name") );
	BOOST_CHECK_EQUAL( m_pathSimpleUnc.GetFileNameWithoutExtension(), _T(".File.Name") );
	BOOST_CHECK_EQUAL( m_pathNetwork.GetFileNameWithoutExtension(), _T(".File.Name") );
	BOOST_CHECK_EQUAL( m_pathNetworkUnc.GetFileNameWithoutExtension(), _T(".File.Name") );
	BOOST_CHECK_EQUAL( m_pathSimpleDirectory.GetFileNameWithoutExtension(), _T("..Sub") );
	BOOST_CHECK_EQUAL( m_pathNetworkFileNoExtUnc.GetFileNameWithoutExtension(), _T("File") );
	BOOST_CHECK_EQUAL( m_pathNetworkExtNoFile.GetFileNameWithoutExtension(), _T(".ext") );
}

BOOST_AUTO_TEST_CASE(GetExtension)
{
	BOOST_CHECK( m_pathEmpty.GetExtension().IsEmpty() );
	BOOST_CHECK_EQUAL( m_pathSimple.GetExtension(), _T(".Extension") );
	BOOST_CHECK_EQUAL( m_pathSimpleUnc.GetExtension(), _T(".Extension") );
	BOOST_CHECK_EQUAL( m_pathNetwork.GetExtension(), _T(".Extension") );
	BOOST_CHECK_EQUAL( m_pathNetworkUnc.GetExtension(), _T(".Extension") );
	BOOST_CHECK_EQUAL( m_pathSimpleDirectory.GetExtension(), _T(".Folder") );
	BOOST_CHECK( m_pathNetworkFileNoExtUnc.GetExtension().IsEmpty() );
	BOOST_CHECK( m_pathNetworkExtNoFile.GetExtension().IsEmpty() );
}

BOOST_AUTO_TEST_CASE(PathFileExists)
{
	// Create a path.
#define C_DIR		_T("C:\\REMOVE_ME_._RIT_TEST_FOLDER")
#define C_FILENAME	_T("...Filé")
	const CString strDir = C_DIR;
	const CString strFile = C_DIR _T("\\") C_FILENAME;
	const CString strFileUnc = _T("\\\\?\\") C_DIR _T("\\") C_FILENAME;
	::CreateDirectory(strDir, NULL);
	FILE *f = NULL;
	_tfopen_s(&f, strFile, _T("w"));
	fclose(f);

	// Test exists.
	BOOST_CHECK( CPath::PathFileExists(strFile) );
	BOOST_CHECK( CPath::PathFileExists(strFileUnc) );

	// Test doesn't exists with wildcards.
	BOOST_CHECK( !CPath::PathFileExists(strFile + _T("*")));
	CString strFileUncCopy = strFileUnc;
	strFileUncCopy.SetAt(10, '*');
	BOOST_CHECK( !CPath::PathFileExists(strFileUncCopy));

	// Remove the file.
	::DeleteFile(strFile);

	// Test not exist.
	BOOST_CHECK( !CPath::PathFileExists(strFile) );
	BOOST_CHECK( !CPath::PathFileExists(strFileUnc) );

	// Remove the directory.
	::RemoveDirectory(strDir);
}

BOOST_AUTO_TEST_CASE(MakeUnicodePath)
{
	BOOST_CHECK_EQUAL( CPath::MakeUnicodePath(_T("C:\\Dir\\File")), _T("\\\\?\\C:\\Dir\\File") );
	BOOST_CHECK_EQUAL( CPath::MakeUnicodePath(_T("\\\\?\\C:\\Dir\\File")), _T("\\\\?\\C:\\Dir\\File") );

	BOOST_CHECK_EQUAL( CPath::MakeUnicodePath(_T("\\\\Network\\ShareName\\Dir\\File")), _T("\\\\?\\UNC\\Network\\ShareName\\Dir\\File") );
	BOOST_CHECK_EQUAL( CPath::MakeUnicodePath(_T("\\\\?\\UNC\\Network\\ShareName\\Dir\\File")), _T("\\\\?\\UNC\\Network\\ShareName\\Dir\\File") );

	BOOST_CHECK_EQUAL( CPath::MakeUnicodePath(_T("\\\\Network")), _T("\\\\?\\UNC\\Network") );
}

BOOST_AUTO_TEST_CASE(MakeSimplePath)
{
	BOOST_CHECK_EQUAL( CPath::MakeSimplePath(_T("\\\\?\\c:\\Dir\\File")), _T("c:\\Dir\\File") );
	BOOST_CHECK_EQUAL( CPath::MakeSimplePath(_T("C:\\Dir\\File")), _T("C:\\Dir\\File"));

	BOOST_CHECK_EQUAL( CPath::MakeSimplePath(_T("\\\\?\\UNC\\Network\\Dir\\File")), _T("\\\\Network\\Dir\\File") );
	BOOST_CHECK_EQUAL( CPath::MakeSimplePath(_T("\\\\Network\\Dir\\File")), _T("\\\\Network\\Dir\\File") );

	BOOST_CHECK_EQUAL( CPath::MakeSimplePath(_T("\\\\?\\UNC\\Network")), _T("\\\\Network") );
}

BOOST_AUTO_TEST_CASE(FSCompare)
{
	BOOST_CHECK( CPath::FSCompare(_T("Abc"), _T("ABC")) == 0 );
}

BOOST_AUTO_TEST_CASE(GetDirectories)
{
	vector<CString> m_vdirSimple = m_pathSimple.GetDirectories();
	BOOST_CHECK_EQUAL( (unsigned)m_vdirSimple.size(), 2 );
	BOOST_CHECK_EQUAL( m_vdirSimple[0], _T("ParentFolder") );
	BOOST_CHECK_EQUAL( m_vdirSimple[1], _T("..Sub.Folder") );

	vector<CString> m_vdirNetworkUnc = m_pathNetworkUnc.GetDirectories();
	BOOST_CHECK_EQUAL( (unsigned)m_vdirNetworkUnc.size(), 2 );
	BOOST_CHECK_EQUAL( m_vdirNetworkUnc[0], _T("Parent Folder") );
	BOOST_CHECK_EQUAL( m_vdirNetworkUnc[1], _T("..Sub.Folder") );

	BOOST_CHECK_EQUAL( (unsigned)m_pathNetworkExtNoFile.GetDirectories().size(), 0 );
}

BOOST_AUTO_TEST_CASE(OpEqual)
{
	BOOST_CHECK( m_pathSimple == m_pathSimple );
	BOOST_CHECK( m_pathSimpleUnc == m_pathSimpleUnc );

	BOOST_CHECK( !(m_pathSimple == m_pathSimpleUnc) );
}

BOOST_AUTO_TEST_CASE(OpDifferent)
{
	BOOST_CHECK( !(m_pathSimple != m_pathSimple) );

	BOOST_CHECK( m_pathSimple != m_pathSimpleUnc );
}

BOOST_AUTO_TEST_CASE(DisplayPath)
{
	BOOST_CHECK_EQUAL("\\\\Network\\Share Name\\Parent Folder\\..Sub.Folder\\.File.Name.Extension", m_pathNetworkUnc.GetDisplayPath());
	BOOST_CHECK_EQUAL(m_pathSimple.GetDisplayPath(), m_pathSimpleUnc.GetDisplayPath());
}

BOOST_AUTO_TEST_CASE(CopyingWeirdPath)
{
	CPath path(_T("no_root_after"));
	BOOST_CHECK_EQUAL("no_root_after", path.GetPath());
	BOOST_CHECK_EQUAL("no_root_after", path.GetDisplayPath());

	CPath unicode = CPath::MakeUnicodePath(path.GetPath());
	BOOST_CHECK_EQUAL("\\\\?\\no_root_after", unicode.GetPath());
	BOOST_CHECK_EQUAL("no_root_after", unicode.GetDisplayPath());

	CPath copy = unicode;
	BOOST_CHECK_EQUAL("\\\\?\\no_root_after", copy.GetPath());
	BOOST_CHECK_EQUAL("no_root_after", copy.GetDisplayPath());
}

//  BOOST_AUTO_TEST_CASE(EllispisedDisplayPath)
// 	{
// 		BOOST_CHECK_EQUAL("", m_pathNetworkUnc.GetDisplayPath(42));
// 		BOOST_CHECK_EQUAL(m_pathNetworkUnc.GetDisplayPath(), m_pathNetworkUnc.GetDisplayPath(m_pathNetworkUnc.GetPath().GetLength()));
// 
// 		for (int i=0; i<200; ++i)
// 			BOOST_CHECK_EQUAL(m_pathSimple.GetDisplayPath(i), m_pathSimpleUnc.GetDisplayPath(i));
// 
// 		CPath pathLongPath = _T("\\\\?\\C\\Hello\\aaaaa\\b\\ccccc\\ddddddddd\\eeeeeeeeee\\fff\\g\\h\\iiiiiiiiiiiii\\ j j j j j \\kkkkkkkkkk\\llllllllll\\mmmmmmmmmmmmmmm\\nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn\\oooooooo\\pppppppppp\\qqqqqqqqqqqq\\rrrrrrrrrrrrrrr\\sssssssssssssss\\tttttttttttt\\uuuuuuuuuuuuuuuu\\vvvvvvvvvvvvvvvvvvv\\wwwwwwwwwwww\\xxxxxxxx..x.x.x.x.x.x.x.x.x.x\\yyyyyyyyyyy...yy.y\\zzzzzzzzzzzzzzzzzzz.zzz");
// 		int nLongPathMaxDisplayLength = pathLongPath.GetDisplayPath().GetLength();
// 		BOOST_CHECK(nLongPathMaxDisplayLength > pathLongPath.GetPath().GetLength() - 10);
// 		for (int i=0; i<1000; ++i)
// 		{
// 			// Generate a path.
// 			if (i < nLongPathMaxDisplayLength)
// 			{
// 				BOOST_CHECK_EQUAL(i, pathLongPath.GetDisplayPath(i).GetLength());
// 			}
// 			else
// 			{
// 				BOOST_CHECK_EQUAL(nLongPathMaxDisplayLength, pathLongPath.GetDisplayPath(i).GetLength());
// 			}
// 		}
// 	}

BOOST_AUTO_TEST_SUITE_END();
