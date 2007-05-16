#pragma once
#include "SyntaxColor.h"

namespace SyntaxColor
{
	class CRegExpReplace :
		public CSyntaxColor
	{
	public:
		CRegExpReplace(void)
		{
			if (m_vCompiledSyntaxColoring.empty())
				CreateCompiledSyntaxColoring();
		}

		virtual ~CRegExpReplace(void)
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
				"\\\\[\\\\0-9]" ));
				// "\0" to "\9"
				m_vCompiledSyntaxColoring.back().vChilds.push_back( BuildSyntaxColoring(
					"\\\\[0-9]", RGB(0xFF, 0xFF, 0xFF), RGB(0x00, 0xC0, 0x00) ));
				// "\\"
				m_vCompiledSyntaxColoring.back().vChilds.push_back( BuildSyntaxColoring(
					"\\\\\\\\", RGB(0x00, 0x00, 0x80), RGB(0x80, 0xC0, 0xFF) ));
		}

		static vector<COMPILED_SYNTAX_COLORING> m_vCompiledSyntaxColoring;
	};
}
