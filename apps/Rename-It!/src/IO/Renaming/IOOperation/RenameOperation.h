#pragma once
#include "IOOperation.h"

namespace Beroux{ namespace IO{ namespace Renaming{ namespace IOOperation
{
	// Perform a MoveFileEx()
	class CRenameOperation : public CIOOperation
	{
	// Construction
	public:
		CRenameOperation(const CPath& pathNameBefore, const CPath& pathNameAfter) :
		  m_pathNameBefore(pathNameBefore),
		  m_pathNameAfter(pathNameAfter)
		{
		}

	// Attributes
		const CPath& GetPathBefore() const {return m_pathNameBefore; }

		const CPath& GetPathAfter() const {return m_pathNameAfter; }

	// Overrides
		virtual EErrorLevel Perform(CKtmTransaction& ktm)
		{
			if (ktm.MoveFileEx(m_pathNameBefore.GetPath(), m_pathNameAfter.GetPath(), MOVEFILE_COPY_ALLOWED))
				return elSuccess;
			else
				return elError;
		}

	// Implementation
	private:
		CPath m_pathNameBefore;
		CPath m_pathNameAfter;
	};
}}}}
