#include <cxxtest/TestSuite.h>
#include "../../Rename-It!/src/Path.h"

class PathTestSuite : public CxxTest::TestSuite 
{
public:
	void testDefaultConstructor()
	{
		CPath emptyPath;
		TS_ASSERT( emptyPath.GetPath().IsEmpty() );
		TS_ASSERT( emptyPath.GetPathRoot().IsEmpty() );
		TS_ASSERT( emptyPath.GetDirectoryName().IsEmpty() );
		TS_ASSERT( emptyPath.GetFileName().IsEmpty() );
		TS_ASSERT( emptyPath.GetFileNameWithoutExtension().IsEmpty() );
		TS_ASSERT( emptyPath.GetExtension().IsEmpty() );
	}

	void testSimplePath()
	{
		CPath samplePath( _T("C:\\ParentFolder\\SubFolder\\FileName.Extension") );
		TS_ASSERT_EQUALS( samplePath.GetPath(), _T("C:\\ParentFolder\\SubFolder\\FileName.Extension") );
		TS_ASSERT_EQUALS( samplePath.GetPathRoot(), _T("C:\\") );
		TS_ASSERT_EQUALS( samplePath.GetDirectoryName(), _T("C:\\ParentFolder\\SubFolder\\") );
		TS_ASSERT_EQUALS( samplePath.GetFileName(), _T("FileName.Extension") );
		TS_ASSERT_EQUALS( samplePath.GetFileNameWithoutExtension(), _T("FileName") );
		TS_ASSERT_EQUALS( samplePath.GetExtension(), _T(".Extension") );
	}
};

