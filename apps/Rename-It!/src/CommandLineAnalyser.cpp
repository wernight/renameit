#include "StdAfx.h"
#include "CommandLineAnalyser.h"
#include "../resource.h"

CCommandLineAnalyser::CCommandLineAnalyser(void) :
	m_pos(NULL)
{
	Reset();
}

CCommandLineAnalyser::~CCommandLineAnalyser(void)
{
	// Clean the memory
	Reset();
}

void CCommandLineAnalyser::Reset(void)
{
	// Reset to the default values
	m_bCmdLineExecMode = false;
	m_strFilterFile.Empty();
	for (POSITION pos = m_lstPaths.GetHeadPosition(); pos!=NULL; )
		delete m_lstPaths.GetNext(pos);
	m_lstPaths.RemoveAll();
	m_bLog = false;
	m_strLogFile.Empty();
}

bool CCommandLineAnalyser::AnalyseCommandLine(void)
{
	// Reset to the default values
	Reset();

	// Get each argument of the command-line
	int argc;
	TCHAR** argv;
#ifndef _UNICODE
	argc = __argc;
	argv = __argv;
#else
	argv = CommandLineToArgvW(GetCommandLineW(), &argc);
#endif

	// Use a state machine to analyse the command-line
	enum EStates
	{
		stateReadingNextArgument,
		stateReadingFilterFile,
		stateReadingPath,
		stateReadingLogFile,
	} nState = stateReadingNextArgument;
	for (int i=1; i<argc; ++i)
	{
		switch (nState)
		{
		case stateReadingPath:
			if (_tcsicmp(argv[i], _T("/R")) == 0)
			{
				m_lstPaths.GetTail()->bRecursive = true;
				break;
			}
			if (_tcsicmp(argv[i], _T("/A")) == 0)
			{
				m_lstPaths.GetTail()->bAllFiles = true;
				break;
			}
		case stateReadingNextArgument:
			if (_tcsicmp(argv[i], _T("/X")) == 0)
				m_bCmdLineExecMode = true;
			else if (_tcsicmp(argv[i], _T("/F")) == 0)
				nState = stateReadingFilterFile;
			else if (_tcsicmp(argv[i], _T("/LOG")) == 0)
				nState = stateReadingLogFile;
			else
			{
				PathElement *pe = new PathElement();
				pe->strPath = argv[i];
				m_lstPaths.AddTail(pe);
				nState = stateReadingPath;
			}
			break;

		case stateReadingFilterFile:
			m_strFilterFile = argv[i];
			nState = stateReadingNextArgument;
			break;

		case stateReadingLogFile:
			m_strLogFile = argv[i];
			m_bLog = true;
			nState = stateReadingNextArgument;
			break;
		}
	}
#ifdef _UNICODE
	LocalFree(argv);
#endif

	// Check that the terminating state is authorised
	switch (nState)
	{
	case stateReadingNextArgument:
	case stateReadingPath:
		return true;	// Everything ok

	case stateReadingFilterFile:
		AfxMessageBox(IDS_CMDLINE_FILTER_FILE_MISSING, MB_ICONERROR);
		break;

	case stateReadingLogFile:
		AfxMessageBox(IDS_CMDLINE_LOG_FILE_MISSING, MB_ICONERROR);
		break;

	default:
		ASSERT(false);
		AfxMessageBox(_T("Unhandled error in CommandLineAnalyser"), MB_ICONERROR);
	}

	Reset();	// Clean memory
	return false;
}
