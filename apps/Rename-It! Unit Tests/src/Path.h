#include <cxxtest/TestSuite.h>
#include "../../Rename-It!/src/Path.h"

class PathTestSuite : public CxxTest::TestSuite 
{
public:
	CPath m_pathEmpty;
	CPath m_pathSimple;
	CPath m_pathSimpleUnc;
	CPath m_pathNetwork;
	CPath m_pathNetworkUnc;
	CPath m_pathSimpleDirectory;
	CPath m_pathNetworkFileNoExtUnc;
	CPath m_pathNetworkExtNoFile;

	PathTestSuite() :
		m_pathEmpty(),
		m_pathSimple(_T("C:\\ParentFolder\\..Sub.Folder\\.File.Name.Extension")),
		m_pathSimpleUnc(_T("\\\\?\\C:\\ParentFolder\\..Sub.Folder\\.File.Name.Extension")),
		m_pathNetwork(_T("\\\\Network\\ParentFolder\\..Sub.Folder\\.File.Name.Extension")),
		m_pathNetworkUnc(_T("\\\\?\\UNC\\Network\\ParentFolder\\..Sub.Folder\\.File.Name.Extension")),
		m_pathSimpleDirectory(_T("C:\\ParentFolder\\..Sub.Folder\\")),
		m_pathNetworkFileNoExtUnc(_T("\\\\?\\UNC\\Network\\File")),
		m_pathNetworkExtNoFile(_T("\\\\UNC\\.ext"))
	{
	}

	void setUp()
	{
	}

	void tearDown()
	{
	}

	void testGetPath()
	{
		TS_ASSERT( m_pathEmpty.GetPath().IsEmpty() );
		TS_ASSERT_EQUALS( m_pathSimple.GetPath(), _T("C:\\ParentFolder\\..Sub.Folder\\.File.Name.Extension") );
		TS_ASSERT_EQUALS( m_pathSimpleUnc.GetPath(), _T("\\\\?\\C:\\ParentFolder\\..Sub.Folder\\.File.Name.Extension") );
		TS_ASSERT_EQUALS( m_pathNetwork.GetPath(), _T("\\\\Network\\ParentFolder\\..Sub.Folder\\.File.Name.Extension") );
		TS_ASSERT_EQUALS( m_pathNetworkUnc.GetPath(), _T("\\\\?\\UNC\\Network\\ParentFolder\\..Sub.Folder\\.File.Name.Extension") );
		TS_ASSERT_EQUALS( m_pathSimpleDirectory.GetPath(), _T("C:\\ParentFolder\\..Sub.Folder") );
		TS_ASSERT_EQUALS( m_pathNetworkFileNoExtUnc.GetPath(), _T("\\\\?\\UNC\\Network\\File") );
		TS_ASSERT_EQUALS( m_pathNetworkExtNoFile.GetPath(), _T("\\\\UNC\\.ext") );
	}

	void testGetPathRoot()
	{
		TS_ASSERT( m_pathEmpty.GetPathRoot().IsEmpty() );
		TS_ASSERT_EQUALS( m_pathSimple.GetPathRoot(), _T("C:\\") );
		TS_ASSERT_EQUALS( m_pathSimpleUnc.GetPathRoot(), _T("\\\\?\\C:\\") );
		TS_ASSERT_EQUALS( m_pathNetwork.GetPathRoot(), _T("\\\\Network\\") );
		TS_ASSERT_EQUALS( m_pathNetworkUnc.GetPathRoot(), _T("\\\\?\\UNC\\Network\\") );
		TS_ASSERT_EQUALS( m_pathSimpleDirectory.GetPathRoot(), _T("C:\\") );
		TS_ASSERT_EQUALS( m_pathNetworkFileNoExtUnc.GetPathRoot(), _T("\\\\?\\UNC\\Network\\") );
		TS_ASSERT_EQUALS( m_pathNetworkExtNoFile.GetPathRoot(), _T("\\\\UNC\\") );
	}

	void testGetDirectoryName()
	{
		TS_ASSERT( m_pathEmpty.GetDirectoryName().IsEmpty() );
		TS_ASSERT_EQUALS( m_pathSimple.GetDirectoryName(), _T("C:\\ParentFolder\\..Sub.Folder\\") );
		TS_ASSERT_EQUALS( m_pathSimpleUnc.GetDirectoryName(), _T("\\\\?\\C:\\ParentFolder\\..Sub.Folder\\") );
		TS_ASSERT_EQUALS( m_pathNetwork.GetDirectoryName(), _T("\\\\Network\\ParentFolder\\..Sub.Folder\\") );
		TS_ASSERT_EQUALS( m_pathNetworkUnc.GetDirectoryName(), _T("\\\\?\\UNC\\Network\\ParentFolder\\..Sub.Folder\\") );
		TS_ASSERT_EQUALS( m_pathSimpleDirectory.GetDirectoryName(), _T("C:\\ParentFolder\\") );
		TS_ASSERT_EQUALS( m_pathNetworkFileNoExtUnc.GetDirectoryName(), _T("\\\\?\\UNC\\Network\\") );
		TS_ASSERT_EQUALS( m_pathNetworkExtNoFile.GetDirectoryName(), _T("\\\\UNC\\") );
	}

	void testGetFileName()
	{
		TS_ASSERT( m_pathEmpty.GetFileName().IsEmpty() );
		TS_ASSERT_EQUALS( m_pathSimple.GetFileName(), _T(".File.Name.Extension") );
		TS_ASSERT_EQUALS( m_pathSimpleUnc.GetFileName(), _T(".File.Name.Extension") );
		TS_ASSERT_EQUALS( m_pathNetwork.GetFileName(), _T(".File.Name.Extension") );
		TS_ASSERT_EQUALS( m_pathNetworkUnc.GetFileName(), _T(".File.Name.Extension") );
		TS_ASSERT_EQUALS( m_pathSimpleDirectory.GetFileName(), _T("..Sub.Folder") );
		TS_ASSERT_EQUALS( m_pathNetworkFileNoExtUnc.GetFileName(), _T("File") );
		TS_ASSERT_EQUALS( m_pathNetworkExtNoFile.GetFileName(), _T(".ext") );
	}

	void testGetFileNameWithoutExtension()
	{
		TS_ASSERT( m_pathEmpty.GetFileNameWithoutExtension().IsEmpty() );
		TS_ASSERT_EQUALS( m_pathSimple.GetFileNameWithoutExtension(), _T(".File.Name") );
		TS_ASSERT_EQUALS( m_pathSimpleUnc.GetFileNameWithoutExtension(), _T(".File.Name") );
		TS_ASSERT_EQUALS( m_pathNetwork.GetFileNameWithoutExtension(), _T(".File.Name") );
		TS_ASSERT_EQUALS( m_pathNetworkUnc.GetFileNameWithoutExtension(), _T(".File.Name") );
		TS_ASSERT_EQUALS( m_pathSimpleDirectory.GetFileNameWithoutExtension(), _T("..Sub") );
		TS_ASSERT_EQUALS( m_pathNetworkFileNoExtUnc.GetFileNameWithoutExtension(), _T("File") );
		TS_ASSERT( m_pathNetworkExtNoFile.GetFileNameWithoutExtension().IsEmpty() );
	}

	void testGetExtension()
	{
		TS_ASSERT( m_pathEmpty.GetExtension().IsEmpty() );
		TS_ASSERT_EQUALS( m_pathSimple.GetExtension(), _T(".Extension") );
		TS_ASSERT_EQUALS( m_pathSimpleUnc.GetExtension(), _T(".Extension") );
		TS_ASSERT_EQUALS( m_pathNetwork.GetExtension(), _T(".Extension") );
		TS_ASSERT_EQUALS( m_pathNetworkUnc.GetExtension(), _T(".Extension") );
		TS_ASSERT_EQUALS( m_pathSimpleDirectory.GetExtension(), _T(".Folder") );
		TS_ASSERT( m_pathNetworkFileNoExtUnc.GetExtension().IsEmpty() );
		TS_ASSERT_EQUALS( m_pathNetworkExtNoFile.GetExtension(), _T(".ext") );
	}

	void testPathFileExists()
	{
		// Create a path.
#define C_DIR		_T("C:\\REMOVE_ME_._RIT_TEST_FOLDER")
#define C_FILENAME	_T("...Filé")
		const LPCTSTR szDir = C_DIR;
		const LPCTSTR szFile = C_DIR _T("\\") C_FILENAME;
		const LPCTSTR szFileUnc = _T("\\\\?\\") C_DIR _T("\\") C_FILENAME;
		::CreateDirectory(szDir, NULL);
		FILE *f = NULL;
		_tfopen_s(&f, szFile, _T("w"));
		fclose(f);

		// Test exists.
		TS_ASSERT( CPath::PathFileExists(szFile) );
		TS_ASSERT( CPath::PathFileExists(szFileUnc) );

		// Remove the file.
		::DeleteFile(szFile);

		// Test not exist.
		TS_ASSERT( !CPath::PathFileExists(szFile) );
		TS_ASSERT( !CPath::PathFileExists(szFileUnc) );

		// Remove the directory.
		::RemoveDirectory(szDir);
	}

	void testMakeUnicodePath()
	{
		TS_ASSERT_EQUALS( CPath::MakeUnicodePath(_T("C:\\Dir\\File")), _T("\\\\?\\C:\\Dir\\File") );
		TS_ASSERT_EQUALS( CPath::MakeUnicodePath(_T("\\\\?\\C:\\Dir\\File")), _T("\\\\?\\C:\\Dir\\File") );

		TS_ASSERT_EQUALS( CPath::MakeUnicodePath(_T("\\\\Network\\Dir\\File")), _T("\\\\?\\UNC\\Network\\Dir\\File") );
		TS_ASSERT_EQUALS( CPath::MakeUnicodePath(_T("\\\\?\\UNC\\Network\\Dir\\File")), _T("\\\\?\\UNC\\Network\\Dir\\File") );

		TS_ASSERT_EQUALS( CPath::MakeUnicodePath(_T("\\\\Network")), _T("\\\\?\\UNC\\Network") );
	}

	void testMakeSimplePath()
	{
		TS_ASSERT_EQUALS( CPath::MakeSimplePath(_T("\\\\?\\c:\\Dir\\File")), _T("c:\\Dir\\File") );
		TS_ASSERT_EQUALS( CPath::MakeSimplePath(_T("C:\\Dir\\File")), _T("C:\\Dir\\File"));

		TS_ASSERT_EQUALS( CPath::MakeSimplePath(_T("\\\\?\\UNC\\Network\\Dir\\File")), _T("\\\\Network\\Dir\\File") );
		TS_ASSERT_EQUALS( CPath::MakeSimplePath(_T("\\\\Network\\Dir\\File")), _T("\\\\Network\\Dir\\File") );

		TS_ASSERT_EQUALS( CPath::MakeSimplePath(_T("\\\\?\\UNC\\Network")), _T("\\\\Network") );
	}

	void testFSCompare()
	{
		TS_ASSERT( CPath::FSCompare(_T("Abc"), _T("ABC")) == 0 );
	}

	void testGetDirectories()
	{
		vector<CString> m_vdirSimple = m_pathSimple.GetDirectories();
		TS_ASSERT_EQUALS( (unsigned)m_vdirSimple.size(), 2 );
		TS_ASSERT_EQUALS( m_vdirSimple[0], _T("ParentFolder") );
		TS_ASSERT_EQUALS( m_vdirSimple[1], _T("..Sub.Folder") );

		vector<CString> m_vdirNetworkUnc = m_pathNetworkUnc.GetDirectories();
		TS_ASSERT_EQUALS( (unsigned)m_vdirNetworkUnc.size(), 2 );
		TS_ASSERT_EQUALS( m_vdirNetworkUnc[0], _T("ParentFolder") );
		TS_ASSERT_EQUALS( m_vdirNetworkUnc[1], _T("..Sub.Folder") );

		TS_ASSERT_EQUALS( (unsigned)m_pathNetworkExtNoFile.GetDirectories().size(), 0 );
	}

	void testOpEqual()
	{
		TS_ASSERT( m_pathSimple == m_pathSimple );
		TS_ASSERT( m_pathSimpleUnc == m_pathSimpleUnc );

		TS_ASSERT( !(m_pathSimple == m_pathSimpleUnc) );
	}

	void testOpDifferent()
	{
		TS_ASSERT( !(m_pathSimple != m_pathSimple) );

		TS_ASSERT( m_pathSimple != m_pathSimpleUnc );
	}
};
