#pragma once
#include "SyntaxColor.h"

namespace SyntaxColor
{
	class CGeneralMacros :
		public CSyntaxColor
	{
	public:
		CGeneralMacros(void)
		{
			if (m_vCompiledSyntaxColoring.empty())
				CreateCompiledSyntaxColoring();
		}

		virtual ~CGeneralMacros(void)
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
				"("	"\\$\\(FileDir\\)"
				"|"	"\\$\\(FileName\\)"
				"|" "\\$\\(FileExt\\)"
				"|" "\\$\\(UnfilteredName\\)"
				"|" "\\$\\(FilteredName\\)"
				")", RGB(0x00, 0x00, 0x80), RGB(0x80, 0xC0, 0xFF), PCRE_CASELESS ));
		}

		static vector<COMPILED_SYNTAX_COLORING> m_vCompiledSyntaxColoring;
	};
}
