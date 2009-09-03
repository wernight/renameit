#pragma once

namespace Beroux{ namespace Util
{
	class CCommandLineAnalyser
	{
	// Definitions
	public:
		struct PathElement
		{
			PathElement(void) : bRecursive(false), bAllFiles(false) {}

			CString	strPath;		// Path to a file or folder
			bool	bRecursive;		// Include sub-folders?
			bool	bAllFiles;		// Include hidden and system files?
		};

	// Construction
		CCommandLineAnalyser(void);
		~CCommandLineAnalyser(void);

		bool AnalyseCommandLine(void);

	// Attributes
		bool IsCommandLineExecutionMode(void) {
			return m_bCmdLineExecMode;
		}

		CString GetFilterFile(void) const { return m_strFilterFile; }

		PathElement* GetFirstPathElement(void)
		{
			m_pos = m_lstPaths.GetHeadPosition();
			if (m_pos == NULL)
				return NULL;
			else
				return m_lstPaths.GetNext(m_pos);
		}

		PathElement* GetNextPathElement(void)
		{
			if (m_pos == NULL)
				return NULL;
			else
				return m_lstPaths.GetNext(m_pos);
		}

		bool IsLoggingEnabled(void) const { return m_bLog; }

		CString GetLogFile(void) const { return m_strLogFile; }

	// Operations

	// Implementation
	private:
		void Reset(void);

		bool		m_bCmdLineExecMode;

		CString		m_strFilterFile;	// RIT filter file to apply

		CTypedPtrList<CPtrList, PathElement*> m_lstPaths;	// Files/Folders to rename

		bool		m_bLog;
		CString		m_strLogFile;

		// Used to iterate through the m_lstPaths.
		POSITION	m_pos;
	};
}}
