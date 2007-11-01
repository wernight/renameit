/* Generated file, do not edit */

#ifndef CXXTEST_RUNNING
#define CXXTEST_RUNNING
#endif

#include "StdAfx.h"
#include <cxxtest/TestListener.h>
#include <cxxtest/TestTracker.h>
#include <cxxtest/TestRunner.h>
#include <cxxtest/RealDescriptions.h>

#include "c:/Users/Wernight/Projects/2007/Rename-It! 4/apps/Rename-It! Unit Tests/src/Path.h"

static PathTestSuite suite_PathTestSuite;

static CxxTest::List Tests_PathTestSuite = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_PathTestSuite( "c:/Users/Wernight/Projects/2007/Rename-It! 4/apps/Rename-It! Unit Tests/src/Path.h", 4, "PathTestSuite", suite_PathTestSuite, Tests_PathTestSuite );

static class TestDescription_PathTestSuite_testDefaultConstructor : public CxxTest::RealTestDescription {
public:
 TestDescription_PathTestSuite_testDefaultConstructor() : CxxTest::RealTestDescription( Tests_PathTestSuite, suiteDescription_PathTestSuite, 7, "testDefaultConstructor" ) {}
 void runTest() { suite_PathTestSuite.testDefaultConstructor(); }
} testDescription_PathTestSuite_testDefaultConstructor;

static class TestDescription_PathTestSuite_testSimplePath : public CxxTest::RealTestDescription {
public:
 TestDescription_PathTestSuite_testSimplePath() : CxxTest::RealTestDescription( Tests_PathTestSuite, suiteDescription_PathTestSuite, 18, "testSimplePath" ) {}
 void runTest() { suite_PathTestSuite.testSimplePath(); }
} testDescription_PathTestSuite_testSimplePath;

