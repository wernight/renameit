#pragma once
#include "SyntaxColor.h"

namespace SyntaxColor
{
	class CEnumeration :
		public CSyntaxColor
	{
	public:
		CEnumeration(void)
		{
			if (m_vCompiledSyntaxColoring.empty())
				CreateCompiledSyntaxColoring();
		}

		virtual ~CEnumeration(void)
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
				"(%%|%[-+0 ]?[0-9]*d)" ));
				m_vCompiledSyntaxColoring.back().vChilds.push_back( BuildSyntaxColoring(
					"%%", RGB(0x00, 0x00, 0x80), RGB(0x80, 0xC0, 0xFF) ));
				m_vCompiledSyntaxColoring.back().vChilds.push_back( BuildSyntaxColoring(
					"%[-+0 ]?[0-9]*d", RGB(0x00, 0x00, 0x80), RGB(0x80, 0xC0, 0xFF) ));
		}

		static vector<COMPILED_SYNTAX_COLORING> m_vCompiledSyntaxColoring;
	};
}
