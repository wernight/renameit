#pragma once
#include "../../KtmTransaction.h"

namespace Beroux{ namespace IO{ namespace Renaming{ namespace IOOperation
{ 
	/**
	 * An abstract operations needed to perform the files&folders renaming.
	 */
	class CIOOperation
	{
	// Construction
	public:
		enum EErrorLevel
		{
			elSuccess,	// No error detected.
			elWarning,	// This is a warning, so the renaming should still be possible.
			elError,	// This is an error, so the renaming would be impossible.
		};

		virtual ~CIOOperation() {}

	// Attributes

	// Operations
		/**
		 * Perform the operation with the given KTM.
		 * \param ktm The KTM to use to perform the I/O operation.
		 * \return elSuccess on success.
		 * \note Call GetErrorMessage() to get the error reason on failure.
		 */
		virtual EErrorLevel Perform(CKtmTransaction& ktm) = 0;

		/**
		 * Explanatory reason of the last failure with an error code for reference.
		 * If the operation didn't fail previously the behavior is undefined
		 * but usually it should return a consistent answer.
		 */
		virtual CString GetErrorMessage() const
		{
			DWORD dwLastError = ::GetLastError();
			CString strErrorMessage;
			strErrorMessage.Format(_T("Error %d: %s"), dwLastError, GetErrorMessage(dwLastError));
			return strErrorMessage;
		}

	protected:
		static CString GetErrorMessage(DWORD dwErrorCode)
		{
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
	};
}}}}
