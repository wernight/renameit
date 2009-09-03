#pragma once
#include "WildcardsSearch.h"

namespace SyntaxColor
{
	class CWildcardsReplace :
		public CSyntaxColor
	{
	public:
		CWildcardsReplace(void)
		{
			if (m_vCompiledSyntaxColoring.empty())
				CreateCompiledSyntaxColoring();
		}

		virtual ~CWildcardsReplace(void)
		{
		}

		virtual void ColorSyntax(CRichEditCtrl& ctrl)
		{
			// Colour using our RegExp rules.
			ColorSyntaxUsingRules(ctrl, m_vCompiledSyntaxColoring);
		}

	private:
		void CreateCompiledSyntaxColoring()
		{
			m_vCompiledSyntaxColoring.push_back( BuildSyntaxColoring(
				"\\*", RGB(0xFF, 0xFF, 0xFF), RGB(0x00, 0xC0, 0x00) ));
		}

		static vector<COMPILED_SYNTAX_COLORING> m_vCompiledSyntaxColoring;
	};
}
