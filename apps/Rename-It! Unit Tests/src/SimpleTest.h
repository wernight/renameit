#include <cxxtest/TestSuite.h>

class PathTestSuite2 : public CxxTest::TestSuite 
{
public:
	void testDefaultConstructor()
	{
		TS_ASSERT( true );
	}

	void testSimplePath()
	{
		TS_ASSERT_EQUALS( 1, 1 );
	}
};

