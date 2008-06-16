#include "StdAfx.h"
#include "RegExpSearch.h"

namespace SyntaxColor
{
	vector<CSyntaxColor::COMPILED_SYNTAX_COLORING> CRegExpSearch::m_vCompiledSyntaxColoring;

	void CRegExpSearch::ColorSyntax(CRichEditCtrl& ctrl)
	{
		// TODO: Matching closing bracket.
		//CHARFORMAT2 cfClosingBracket = myCHARFORMAT2(NULL, RGB(0x00, 0xFF, 0xFF));

		// Colour using our RegExp rules.
		ColorSyntaxUsingRules(ctrl, m_vCompiledSyntaxColoring);

		// Compile the search string to look for compilation errors.
		CString strSubject;
		ctrl.GetWindowText(strSubject);
#ifdef _UNICODE
		char szPatternA[1024];
		szPatternA[WideCharToMultiByte(CP_ACP, 0, strSubject, -1, szPatternA, sizeof(szPatternA)/sizeof(szPatternA[0]), NULL, NULL)] = '\0';
#else
		const char* szPatternA = strSubject;
#endif
		const char *pchError = NULL;
		int nErrorOffset = 0;
		if (pcre_compile(szPatternA, 0, &pchError, &nErrorOffset, NULL) == NULL)
		{
			// Save the selection.
			CHARRANGE crOriginalSelection;
			ctrl.GetSel(crOriginalSelection);

			// Colour the error in red.
			CHARFORMAT2 cfError;
			ZeroMemory(&cfError, sizeof(CHARFORMAT2));
			cfError.cbSize = sizeof(CHARFORMAT2);
			cfError.dwMask = CFM_COLOR | CFM_BACKCOLOR;
			cfError.crTextColor = RGB(0xFF, 0xFF, 0xFF);
			cfError.crBackColor = RGB(0xFF, 0x00, 0x00);

			if (nErrorOffset >= strSubject.GetLength())
				ctrl.SetSel(nErrorOffset-1, nErrorOffset);
			else
				ctrl.SetSel(nErrorOffset, nErrorOffset+1);
			ctrl.SetSelectionCharFormat(cfError);

			// Restore the selection.
			ctrl.SetSel(crOriginalSelection);
		}
	}

	// Make a set of (REGEXP_PATTERN, CHARFORMAT2) use it to colour.
	void CRegExpSearch::CreateCompiledSyntaxColoring()
	{
		// Groups subexpressions for capturing
		m_vCompiledSyntaxColoring.push_back( BuildSyntaxColoring(
			"[()]", RGB(0xFF, 0xFF, 0xFF), RGB(0x00, 0xC0, 0x00) ));
		// Groups subexpressions without capturing (cluster)
		m_vCompiledSyntaxColoring.push_back( BuildSyntaxColoring(
			"\\(\\?:[^)]*\\)", NULL, RGB(0xC0, 0xC0, 0x00) ));
		// Special characters inside
//		m_vCompiledSyntaxColoring.push_back( BuildSyntaxColoring(
//			"\\|", RGB(0xFF, 0xFF, 0xFF), RGB(0x00, 0xC0, 0x00) ));

		// Anchors
		m_vCompiledSyntaxColoring.push_back( BuildSyntaxColoring(
			"[\\^$]", RGB(0xFF, 0xFF, 0xFF), RGB(0x66, 0x99, 0xCC) ));
		m_vCompiledSyntaxColoring.push_back( BuildSyntaxColoring(
			"\\\\[\\\\bBAZzG]" ));
			m_vCompiledSyntaxColoring.back().vChilds.push_back( BuildSyntaxColoring(
				"\\\\[bBAZzG]", RGB(0xFF, 0xFF, 0xFF), RGB(0x66, 0x99, 0xCC) ));

		// Special meaning characters (outside brackets)
		m_vCompiledSyntaxColoring.push_back( BuildSyntaxColoring(
			"\\|", RGB(0xFF, 0xFF, 0xFF), RGB(0x00, 0xC0, 0x00) ));
		m_vCompiledSyntaxColoring.push_back( BuildSyntaxColoring(
			"\\.", NULL, RGB(0x80, 0xC0, 0xFF) ));

		// Quantifiers
		m_vCompiledSyntaxColoring.push_back( BuildSyntaxColoring(
			"[*+?]", NULL, RGB(0x80, 0xC0, 0xFF) ));
		m_vCompiledSyntaxColoring.push_back( BuildSyntaxColoring(
			"\\{[0-9]+(?:,[0-9]*)?\\}", NULL, RGB(0x80, 0xC0, 0xFF) ));

		// Escapings (outside brackets)
		m_vCompiledSyntaxColoring.push_back( BuildSyntaxColoring(
			"\\\\"	"(" "[^0xcNpP]"				// General escaping
					"|" "0[0-7]{0,2}"			// \037     Any octal ASCII value
					"|" "x\\{[0-9a-fA-F]{0,}}"	// \x{263a} A wide hexadecimal value
					"|"	"x[0-9a-fA-F]{0,2}"		// \x7f     Any hexadecimal ASCII value
					"|" "c."					// \cx      Control-x
					"|" "N{[a-zA-Z]*}"			// \N{name} A named character
					// Character classes
					"|" "[pP]."					// (Non-)Match P-named (Unicode) property
					"|" "[pP]\\{[a-zA-Z]*}"		// (Non-)Match Unicode property with long name
					")", RGB(0x00, 0x00, 0x80), RGB(0xE0, 0xA0, 0x60) ));
		m_vCompiledSyntaxColoring.push_back( BuildSyntaxColoring(
			"\\\\"	"[^0xcNpP]", NULL, RGB(0x80, 0xC0, 0xFF) ));

		// Brackets
		m_vCompiledSyntaxColoring.push_back( BuildSyntaxColoring(
			"\\[" "\\^?" "(" "[^\\[\\^\\\\]" // Something not "[^\"
						 "|" "\\\\" "." // Or "\x"
						 "|" "(" "\\[\\^?:([^\\]:\\\\]|\\\\.)+:\\]" "|" "\\[" ")" // Or "[:xxx:]" or else "[x"
						 ")"
						 "(" "[^\\[\\]\\\\]" // Something not "[]\"
						 "|" "\\\\" "." // Or "\x"
						 "|" "(" "\\[:\\^?([^\\]:\\\\]|\\\\.)+:\\]" "|" "\\[" ")" // Or "[:xxx:]" or else "[x"
						 ")*" "\\]", NULL, RGB(0xFF, 0xC0, 0x80) ));
		// Escapings inside brackets
			m_vCompiledSyntaxColoring.back().vChilds.push_back( BuildSyntaxColoring(
				"(" "\\[:\\^?(alpha|alnum|ascii|blank|cntrl|digit|graph|lower|print|punct|space|upper|word|xdigit):]"	// POSIX
				"|" "\\\\"	"(" "[^0xcNpP]"				// General escape sequences
							"|" "0[0-7]{0,2}"			// \037     Any octal ASCII value
							"|" "x\\{[0-9a-fA-F]{0,}}"	// \x{263a} A wide hexadecimal value
							"|"	"x[0-9a-fA-F]{0,2}"		// \x7f     Any hexadecimal ASCII value
							"|" "c."						// \cx      Control-x
							"|" "N{[a-zA-Z]*}"			// \N{name} A named character
							// Character classes
							"|" "[pP]."					// (Non-)Match P-named (Unicode) property
							"|" "[pP]\\{[a-zA-Z]*}"		// (Non-)Match Unicode property with long name
							")"
				")", NULL, RGB(0xE0, 0xA0, 0x60) ));
	}
}
