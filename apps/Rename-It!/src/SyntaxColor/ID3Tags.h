#pragma once
#include "SyntaxColor.h"

namespace SyntaxColor
{
	class CID3Tags :
		public CSyntaxColor
	{
	public:
		CID3Tags(void)
		{
			if (m_vCompiledSyntaxColoring.empty())
				CreateCompiledSyntaxColoring();
		}

		virtual ~CID3Tags(void)
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
				"("	"\\$\\(ID3Track\\)"
				"|"	"\\$\\(ID3Artist\\)"
				"|" "\\$\\(ID3Title\\)"
				"|" "\\$\\(ID3Album\\)"
				"|" "\\$\\(ID3Year\\)"
				"|" "\\$\\(ID3Comment\\)"
				"|" "\\$\\(ID3Genre\\)"
				")", RGB(0x00, 0x00, 0x80), RGB(0x80, 0xC0, 0xFF), PCRE_CASELESS ));
		}

		static vector<COMPILED_SYNTAX_COLORING> m_vCompiledSyntaxColoring;
	};
}
