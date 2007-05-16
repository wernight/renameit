#pragma once
#include "SyntaxColor.h"

namespace SyntaxColor
{
	class CWildcardsSearch :
		public CSyntaxColor
	{
	public:
		CWildcardsSearch(void)
		{
			if (m_vCompiledSyntaxColoring.empty())
				CreateCompiledSyntaxColoring();
		}

		virtual ~CWildcardsSearch(void)
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
				"\\?", RGB(0x00, 0x00, 0x80), RGB(0x80, 0xC0, 0xFF) ));
			m_vCompiledSyntaxColoring.push_back( BuildSyntaxColoring(
				"\\*", RGB(0xFF, 0xFF, 0xFF), RGB(0x00, 0xC0, 0x00) ));
		}

		static vector<COMPILED_SYNTAX_COLORING> m_vCompiledSyntaxColoring;
	};
}
