#pragma once
#include "pcre.h"

namespace SyntaxColor
{
	class CSyntaxColor
	{
	public:
		CSyntaxColor(void)
		{
		}

		virtual ~CSyntaxColor(void)
		{
		}

		// Reset to the default character format.
		static void ClearColoring(CRichEditCtrl& ctrl)
		{
			// Save the selection.
			CHARRANGE crOriginalSelection;
			ctrl.GetSel(crOriginalSelection);

			// Restaure the default style.
			CHARFORMAT2 cf;
			ZeroMemory(&cf, sizeof(CHARFORMAT2));
			cf.cbSize = sizeof(CHARFORMAT2);
			cf.dwMask = CFM_COLOR | CFM_BACKCOLOR;
			cf.dwEffects = CFE_AUTOCOLOR | CFE_AUTOBACKCOLOR;

			ctrl.SetSel(0, ctrl.GetTextLength());
			ctrl.SetSelectionCharFormat(cf);
			ctrl.SetDefaultCharFormat(cf);

			// Restaure the selection.
			ctrl.SetSel(crOriginalSelection);
		}

		/**
		 * Colour the ctrl using the specific rules.
		 * Colour codes for Rename-It! should be:
		 * - Blue: General special meaning character short sequence (1 or 2 characters) that can represent some characters.
		 * - Green: Capture/Use some string from Search to Replace.
		 * - Brown: Long sequence being just a single character.
		 * - Hollow on Dark blue: Some characters that don't represent any character and don't capture.
		 * - Red: Error
		 */
		virtual void ColorSyntax(CRichEditCtrl& ctrl) = 0;

	protected:
		struct COMPILED_SYNTAX_COLORING
		{
			pcre* regexpCompiled;
			pcre_extra* pcreExtra;
			CHARFORMAT2 cfFormat;
			vector<COMPILED_SYNTAX_COLORING> vChilds;
		};

		static COMPILED_SYNTAX_COLORING BuildSyntaxColoring(const char* szRegExpPattern, COLORREF crTextColor=NULL, COLORREF crBackColor=NULL, int nPcreOptions=0)
		{
			COMPILED_SYNTAX_COLORING csc;

			// Compile the RegExp.
			const char *pchError;
			int nErrorOffset;
			csc.regexpCompiled = pcre_compile(szRegExpPattern, PCRE_NO_AUTO_CAPTURE | nPcreOptions, &pchError, &nErrorOffset, NULL);
			if (csc.regexpCompiled  != NULL)
			{
				// A pattern can be studied for better execution time in pcre_exec.
				csc.pcreExtra = pcre_study(csc.regexpCompiled, 0, &pchError);
			}
			else
				ASSERT(FALSE);	// Invalid RegExp

			// Create the CHARFORMAT2.
			ZeroMemory(&csc.cfFormat, sizeof(CHARFORMAT2));
			csc.cfFormat.cbSize = sizeof(CHARFORMAT2);
			csc.cfFormat.dwMask = CFM_COLOR | CFM_BACKCOLOR;

			if (crTextColor == NULL)
				csc.cfFormat.dwEffects |= CFE_AUTOCOLOR;
			else
				csc.cfFormat.crTextColor = crTextColor;

			if (crBackColor == NULL)
				csc.cfFormat.dwEffects |= CFM_BACKCOLOR;
			else
				csc.cfFormat.crBackColor = crBackColor;

			return csc;
		}

		// Apply some RegExp rules to colour.
		static void ColorSyntaxUsingRules(CRichEditCtrl& ctrl, const vector<COMPILED_SYNTAX_COLORING>& vColouringRules, int nStart=0, int nEnd=-1)
		{
			// Are we done?
			if (vColouringRules.empty())
				return;

			// Load the text being analysed.
			CString strSubject;
			ctrl.GetWindowText(strSubject);
			if (nEnd >= nStart)
				strSubject = strSubject.Mid(nStart, nEnd-nStart);
			else
				strSubject = strSubject.Mid(nStart);

			// Save the selection.
			CHARRANGE crOriginalSelection;
			ctrl.GetSel(crOriginalSelection);

			// Colour using the syntax colouring rules.
			char szSubjectA[1024];
#ifdef _UNICODE
			szSubjectA[WideCharToMultiByte(CP_ACP, 0, strSubject, -1, szSubjectA, sizeof(szSubjectA), NULL, NULL)] = '\0';
#else
			strcpy(szSubjectA, strSubject);
#endif
			for (vector<COMPILED_SYNTAX_COLORING>::const_iterator iter=vColouringRules.begin(); iter!=vColouringRules.end(); ++iter)
			{
				const int OFFSETS_SIZE = 3*20;
				int nvOffsets[OFFSETS_SIZE];
				int nStartOffset = 0;
				while (true)
				{
					int nCount = pcre_exec(iter->regexpCompiled, iter->pcreExtra, szSubjectA, (int)strlen(szSubjectA), nStartOffset, 0, nvOffsets, OFFSETS_SIZE);
					if (nCount < 1)
						break;	// Not found;

					// Found a matching element.
					int nMatchStart = nvOffsets[0];
					int nMatchEnd = nvOffsets[1];

					// Select the matching element.
					ctrl.SetSel(nStart+nMatchStart, nStart+nMatchEnd);

					// Change the format.
					CHARFORMAT2 cf = iter->cfFormat;
					ctrl.SetSelectionCharFormat(cf);

					// Recursively call the childrens
					ColorSyntaxUsingRules(ctrl, iter->vChilds, nStart+nMatchStart, nStart+nMatchEnd);

					// Search for the next one.
					ASSERT(nMatchEnd > nStartOffset);
					nStartOffset = nMatchEnd;
				}
			}

			// Restaure the selection.
			ctrl.SetSel(crOriginalSelection);
		}
	};
}
