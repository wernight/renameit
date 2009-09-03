#pragma once
#include "SyntaxColor.h"

namespace SyntaxColor
{
	class CRegExpSearch :
		public CSyntaxColor
	{
	public:
		CRegExpSearch(void)
		{
			if (m_vCompiledSyntaxColoring.empty())
				CreateCompiledSyntaxColoring();
		}

		virtual ~CRegExpSearch(void)
		{
		}

		virtual void ColorSyntax(CRichEditCtrl& ctrl);

	private:
		void CreateCompiledSyntaxColoring();

		static vector<COMPILED_SYNTAX_COLORING> m_vCompiledSyntaxColoring;
	};
}
