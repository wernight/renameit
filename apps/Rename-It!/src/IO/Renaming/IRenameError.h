#pragma once

namespace Beroux{ namespace IO{ namespace Renaming
{
	/**
	 * A kind of error during the renaming process.
	 */
	class IRenameError
	{
	// Definitions
	public:
		enum EErrorLevel
		{
			levelNone,		// No error detected.
			levelWarning,	// This is a warning, so the renaming is still possible.
			levelError,		// This is an error, so the renaming would be impossible.
		};

	// Construction
		IRenameError(EErrorLevel level) : m_nErrorLevel(level) {}

		virtual ~IRenameError() {}

	// Attributes
		EErrorLevel GetErrorLevel() const {
			return m_nErrorLevel;
		}

	// Implementation
	protected:
		static CString GetErrorMessage(DWORD dwErrorCode) {
			// Get error message
			LPTSTR lpMsgBuf = NULL;
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				dwErrorCode,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL );

			// Save it
			CString strErrorMessage = lpMsgBuf;

			// Free the buffer.
			LocalFree( lpMsgBuf );

			return strErrorMessage;
		}

	private:
		EErrorLevel m_nErrorLevel;
	};
}}}
