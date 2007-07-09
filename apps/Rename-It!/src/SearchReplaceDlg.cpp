/* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/
#include "StdAfx.h"
#include "SearchReplaceDlg.h"

#include "SyntaxColor/RegExpSearch.h"
#include "SyntaxColor/RegExpReplace.h"
#include "SyntaxColor/WildcardsSearch.h"
#include "SyntaxColor/WildcardsReplace.h"
#include "SyntaxColor/ID3Tags.h"
#include "SyntaxColor/GeneralMacros.h"
#include "SyntaxColor/Enumeration.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSearchReplaceDlg dialog


CSearchReplaceDlg::CSearchReplaceDlg(CSearchReplaceFilter& filter, IPreviewFileList& previewSamples, CWnd* pParent /*=NULL*/)
	: CDialog(CSearchReplaceDlg::IDD, pParent)
	, m_bInitialized(false)
	, m_filter(filter)
	, m_nUse(0)
	, m_bUse(FALSE)
	, m_bCaseSensitive(TRUE)
	, m_bAllOccurences(TRUE)
	, m_strBefore("")
	, m_strReplace("")
	, m_strSearch("")
	, m_bSeries(FALSE)
	, m_nSerieStart(1)
	, m_nSerieStep(1)
	, m_bMatchWholeText(FALSE)
	, m_bChangeCase(FALSE)
	, m_nChangeCase(0)
	, m_pToolTip(NULL)
	, m_bAdvanced(FALSE)
	, m_previewSamples(previewSamples)
{
	m_strBefore = previewSamples.GetOriginalFileName().GetFilteredSubstring();

	m_strSearch = m_filter.GetSearch();
	m_strReplace = m_filter.GetReplace();
	m_bAllOccurences = !m_filter.IsReplaceOnce();
	m_bCaseSensitive = m_filter.IsCaseSensitive();
	m_bMatchWholeText = m_filter.IsMatchWholeText();
	m_bUse = m_filter.GetUse() != CSearchReplaceFilter::useNone;
	if (m_bUse)
		m_nUse = m_filter.GetUse();
	m_bChangeCase = m_filter.GetChangeCase() != CSearchReplaceFilter::caseNone;
	if (m_bChangeCase)
		m_nChangeCase = m_filter.GetChangeCase();

	m_bSeries = m_filter.IsSeriesEnabled();
	m_nSerieStart = m_filter.GetSeriesStart();
	m_nSerieStep = m_filter.GetSeriesStep();

	m_bID3Tag = m_filter.IsID3TagEnabled();

	// Is it an advanced filter?
	m_bAdvanced = (m_bMatchWholeText || m_bAllOccurences);

	m_hIcon = AfxGetApp()->LoadIcon(IDI_RENAMER);

	// Create the syntax colourer.
	ASSERT(scRegExpSearch == m_vSyntaxColor.size()); m_vSyntaxColor.push_back(new SyntaxColor::CRegExpSearch());
	ASSERT(scRegExpReplace == m_vSyntaxColor.size()); m_vSyntaxColor.push_back(new SyntaxColor::CRegExpReplace());
	ASSERT(scWildcardsSearch == m_vSyntaxColor.size()); m_vSyntaxColor.push_back(new SyntaxColor::CWildcardsSearch());
	ASSERT(scWildcardsReplace == m_vSyntaxColor.size()); m_vSyntaxColor.push_back(new SyntaxColor::CWildcardsReplace());
	ASSERT(scID3Tag == m_vSyntaxColor.size()); m_vSyntaxColor.push_back(new SyntaxColor::CID3Tags());
	ASSERT(scGeneralMacros == m_vSyntaxColor.size()); m_vSyntaxColor.push_back(new SyntaxColor::CGeneralMacros());
	ASSERT(scEnumeration == m_vSyntaxColor.size()); m_vSyntaxColor.push_back(new SyntaxColor::CEnumeration());
	ASSERT(scCount == m_vSyntaxColor.size());
}

CSearchReplaceDlg::~CSearchReplaceDlg()
{
	delete m_pToolTip;

	for (vector<SyntaxColor::CSyntaxColor*>::const_iterator iter=m_vSyntaxColor.begin(); iter!=m_vSyntaxColor.end(); ++iter)
		delete *iter;
	m_vSyntaxColor.clear();
}

void CSearchReplaceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSearchReplaceDlg)
	DDX_Control(pDX, IDC_SEARCH_RICHEDIT, m_ctlSearchRichEdit);
	DDX_Control(pDX, IDC_REPLACE_RICHEDIT, m_ctlReplaceRichEdit);
	DDX_Control(pDX, IDC_SEARCHBUTTON, m_ctlSearchButton);
	DDX_Control(pDX, IDC_REPLACEBUTTON, m_ctlReplaceButton);
	DDX_Text(pDX, IDC_SEARCH_RICHEDIT, m_strSearch);
	DDX_Text(pDX, IDC_REPLACE_RICHEDIT, m_strReplace);
	DDX_Text(pDX, IDC_BEFORE, m_strBefore);
	DDX_Check(pDX, IDC_MATCHCASE, m_bCaseSensitive);
	DDX_Check(pDX, IDC_ALL_OCCURRENCES_CHECK, m_bAllOccurences);
	DDX_CBIndex(pDX, IDC_USE_COMBO, m_nUse);
	DDX_Check(pDX, IDC_USE_CHECK, m_bUse);
	DDX_Check(pDX, IDC_SERIES_CHECK, m_bSeries);
	DDX_Check(pDX, IDC_ID3TAG_CHECK, m_bID3Tag);
	DDX_Text(pDX, IDC_SERIE_START_EDIT, m_nSerieStart);
	DDX_Text(pDX, IDC_SERIE_STEP_EDIT, m_nSerieStep);
	DDX_Check(pDX, IDC_WHOLE_TEXT, m_bMatchWholeText);
	DDX_Check(pDX, IDC_CASE_CHECK, m_bChangeCase);
	DDX_CBIndex(pDX, IDC_CASE_COMBO, m_nChangeCase);
	DDX_Control(pDX, IDC_SERIE_START_SPIN, m_ctrlSeriesStartSpin);
	DDX_Control(pDX, IDC_SERIE_STEP_SPIN, m_ctrlSeriesStepSpin);
	DDX_Check(pDX, IDC_ADVANCED_CHECK, m_bAdvanced);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSearchReplaceDlg, CDialog)
	ON_EN_CHANGE(IDC_SEARCH_RICHEDIT, &CSearchReplaceDlg::OnEnChangeSearch)
	ON_EN_CHANGE(IDC_REPLACE_RICHEDIT, &CSearchReplaceDlg::OnEnChangeReplace)
	ON_EN_CHANGE(IDC_BEFORE, OnChangeBefore)
	ON_BN_CLICKED(IDC_REPLACEBUTTON, OnReplaceButton)
	ON_BN_CLICKED(IDC_SEARCHBUTTON, OnSearchButton)
	ON_BN_CLICKED(IDC_MATCHCASE, OnMatchcase)
	ON_BN_CLICKED(IDC_ALL_OCCURRENCES_CHECK, OnBnClickedAllOccurencesCheck)
	ON_BN_CLICKED(IDC_USE_CHECK, OnBnClickedUseCheck)
	ON_CBN_SELCHANGE(IDC_USE_COMBO, OnCbnSelchangeUseCombo)
	ON_BN_CLICKED(IDC_CASE_CHECK, OnBnClickedCaseCheck)
	ON_CBN_SELCHANGE(IDC_CASE_COMBO, OnCbnSelchangeCaseCombo)
	ON_BN_CLICKED(IDC_SERIES_BUTTON, OnBnClickedSeriesButton)
	ON_BN_CLICKED(IDC_SERIES_CHECK, OnChangeSeries)
	ON_EN_CHANGE(IDC_SERIE_START_EDIT, &CSearchReplaceDlg::OnChangeSeries)
	ON_EN_CHANGE(IDC_SERIE_STEP_EDIT, &CSearchReplaceDlg::OnChangeSeries)
	ON_BN_CLICKED(IDC_ID3TAG_CHECK, OnBnClickedID3TagCheck)
	ON_BN_CLICKED(IDC_ADVANCED_CHECK, OnBnClickedAdvancedCheck)
	ON_BN_CLICKED(IDC_WHOLE_TEXT, OnMatchWholeText)
	ON_BN_CLICKED(IDC_ADVANCED_CHECK, &CSearchReplaceDlg::OnBnClickedAdvancedCheck)
	ON_WM_HELPINFO()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSearchReplaceDlg message handlers

BOOL CSearchReplaceDlg::OnInitDialog() 
{
	__super::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Set invite strings.
	m_ctlSearchRichEdit.SetInviteText( IDS_SEARCH_INVITE );
	m_ctlReplaceRichEdit.SetInviteText( IDS_REPLACE_INVITE );

	// Set up the tooltip
	m_pToolTip = new CToolTipCtrl();
	VERIFY( m_pToolTip->Create(this) );
	VERIFY( m_pToolTip->AddTool(GetDlgItem(IDC_ID3TAG_CHECK), IDS_TT_ID3) );
	VERIFY( m_pToolTip->AddTool(GetDlgItem(IDC_BEFORE), IDS_TT_BEFORE) );
	VERIFY( m_pToolTip->AddTool(GetDlgItem(IDC_AFTER), IDS_TT_AFTER) );
	VERIFY( m_pToolTip->AddTool(GetDlgItem(IDC_ALL_OCCURRENCES_CHECK), IDS_TT_ALLOCCURENCES) );
	m_pToolTip->Activate(TRUE);

	// Set up the spins
	m_ctrlSeriesStartSpin.SetRange(UD_MINVAL, UD_MAXVAL);
	m_ctrlSeriesStepSpin.SetRange(UD_MINVAL, UD_MAXVAL);

	// Be alterted when the text change.
	m_ctlSearchRichEdit.SetEventMask(ENM_CHANGE);
	m_ctlReplaceRichEdit.SetEventMask(ENM_CHANGE);

	// Now initialized.
	m_bInitialized = true;

	// Update filter.
	UpdateView();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

#pragma warning( disable : 4800 )  // Performance Warning

void CSearchReplaceDlg::OnEnChangeSearch() 
{
	UpdateView();
}

void CSearchReplaceDlg::OnEnChangeReplace()
{
	UpdateView();
}

void CSearchReplaceDlg::OnChangeBefore() 
{
	UpdateView();
}

void CSearchReplaceDlg::OnMatchcase() 
{
	UpdateView();
}

void CSearchReplaceDlg::OnBnClickedAllOccurencesCheck() 
{
	UpdateView();
}

#pragma warning( default : 4800 )  // Performance Warning

void CSearchReplaceDlg::OnSearchButton() 
{
	CMenu	menu;
	DWORD dwSelectionMade;
	CString replaceText;

	if (!UpdateData())
		return;

	// Show menu
	switch (m_nUse)
	{
	case CSearchReplaceFilter::useRegExp:
		VERIFY( menu.LoadMenu(IDR_SEARCH_REGEXP) );
		break;

	case CSearchReplaceFilter::useWildcards:
		VERIFY( menu.LoadMenu(IDR_SEARCH_WILD) );
		break;

	default:
		ASSERT(FALSE);
		return;
	}
	dwSelectionMade = m_ctlSearchButton.DoSingleClick(menu);

	// Process command
	switch(dwSelectionMade)
	{
	case 0:	// Cancelled
		replaceText.Empty();
		break;
	case ID_ANYCHARACTER:
		replaceText = _T(".");
		break;
	case ID_CHARACTERINRANGE:
		replaceText = _T("[]");
		break;
	case ID_CHARACTERNOTINRANGE:
		replaceText = _T("[^]");
		break;
	case ID_BEGINNGINGOFLINE:
		replaceText = _T("^");
		break;
	case ID_ENDOFLINE:
		replaceText = _T("$");
		break;
	case ID_SEARCHREGEXP_NEWLINE:
		replaceText = _T("\\n");
		break;
	case ID_TAGGEDEXPRESSION:
		replaceText = _T("()");
		break;
	case ID_OR:
		replaceText = _T("|");
		break;
	case ID_0ORMOREMATCHES:
		replaceText = _T("*");
		break;
	case ID_1ORMOREMATCHES:
		replaceText = _T("+");
		break;
	case ID_ALPHANUMERICCHARACTER:
		replaceText = _T("[0-9A-Za-z]");
		break;
	case ID_ALPHABETICCHARACTER:
		replaceText = _T("[A-Za-z]");
		break;
	case ID_DECIMALDIGIT:
		replaceText = _T("[0-9]");
		break;
	case ID_WILD_0ORMORE:
		replaceText = _T("*");
		break;
	case ID_WILD_1MATCH:
		replaceText = _T("?");
		break;
	default:
		ASSERT(false);
	}

	long nStartSel, nEndSel;
	m_ctlSearchRichEdit.GetSel(nStartSel, nEndSel);

	if (!replaceText.IsEmpty())
	{
		m_ctlSearchRichEdit.ReplaceSel( replaceText );
		nStartSel = nStartSel + replaceText.GetLength();
		nEndSel = nStartSel;
	}

	// Move back..
	PrevDlgCtrl();
	m_ctlSearchRichEdit.SetSel(nStartSel, nEndSel);
}

void CSearchReplaceDlg::OnReplaceButton() 
{
	CMenu menu;
    CMenu *pmenuPopup;
	DWORD dwSelectionMade;
	CString replaceText;

	if (!UpdateData(TRUE))
		return;

	// Show menu
    VERIFY( menu.LoadMenu(IDR_REPLACE) );
	pmenuPopup = menu.GetSubMenu(0);
    ASSERT(pmenuPopup != NULL);
	if (!m_bUse || m_nUse != CSearchReplaceFilter::useRegExp)
		pmenuPopup->EnableMenuItem(8, MF_BYPOSITION | MF_GRAYED);
	if (!m_bUse || m_nUse != CSearchReplaceFilter::useWildcards)
		pmenuPopup->EnableMenuItem(9, MF_BYPOSITION | MF_GRAYED);
	dwSelectionMade = m_ctlReplaceButton.DoSingleClick(menu);
	
	// Process command
	switch (dwSelectionMade)
	{
	case 0:	// User cancelled
		replaceText.Empty();
		break;
	case ID_REPLACE_FILEDIR:
		replaceText = _T("$(FileDir)");
		break;
	case ID_REPLACE_FILENAME:
		replaceText = _T("$(FileName)");
		break;
	case ID_REPLACE_FILEEXT:
		replaceText = _T("$(FileExt)");
		break;
	case ID_REPLACE_UNFILTEREDNAME:
		replaceText = _T("$(UnfilteredName)");
		break;
	case ID_REPLACE_FILTEREDNAME:
		replaceText = _T("$(FilteredName)");
		break;
	// ID3 Tag Reader
	case ID_ID3TAGREADER_TRACK:
		m_bID3Tag = true;
		replaceText = _T("$(ID3Track)");
		break;
	case ID_ID3TAGREADER_ARTIST:
		m_bID3Tag = true;
		replaceText = _T("$(ID3Artist)");
		break;
	case ID_ID3TAGREADER_TITLE:
		m_bID3Tag = true;
		replaceText = _T("$(ID3Title)");
		break;
	case ID_ID3TAGREADER_ALBUM:
		m_bID3Tag = true;
		replaceText = _T("$(ID3Album)");
		break;
	case ID_ID3TAGREADER_YEAR:
		m_bID3Tag = true;
		replaceText = _T("$(ID3Year)");
		break;
	case ID_ID3TAGREADER_COMMENT:
		m_bID3Tag = true;
		replaceText = _T("$(ID3Comment)");
		break;
	case ID_ID3TAGREADER_GENRE:
		m_bID3Tag = true;
		replaceText = _T("$(ID3Genre)");
		break;
	// RegExp
	case ID_REGEXP_FINDWHATTEXT:
		replaceText = _T("\\0");
		break;
	case ID_REGEXP_TAGGEDEXPRESSION1:
		replaceText = _T("\\1");
		break;
	case ID_REGEXP_TAGGEDEXPRESSION2:
		replaceText = _T("\\2");
		break;
	case ID_REGEXP_TAGGEDEXPRESSION3:
		replaceText = _T("\\3");
		break;
	case ID_REGEXP_TAGGEDEXPRESSION4:
		replaceText = _T("\\4");
		break;
	case ID_REGEXP_TAGGEDEXPRESSION5:
		replaceText = _T("\\5");
		break;
	case ID_REGEXP_TAGGEDEXPRESSION6:
		replaceText = _T("\\6");
		break;
	case ID_REGEXP_TAGGEDEXPRESSION7:
		replaceText = _T("\\7");
		break;
	case ID_REGEXP_TAGGEDEXPRESSION8:
		replaceText = _T("\\8");
		break;
	case ID_REGEXP_TAGGEDEXPRESSION9:
		replaceText = _T("\\9");
		break;
	// Wildcard
	case ID_TAGGETBYSTAR:
		replaceText = _T("*");
		break;
	default:
		ASSERT(false);
	}
	UpdateData(FALSE);

	long nStartSel, nEndSel;
	m_ctlReplaceRichEdit.GetSel(nStartSel, nEndSel);

	if (!replaceText.IsEmpty())
	{
		m_ctlReplaceRichEdit.ReplaceSel( replaceText );
		nStartSel = nStartSel + replaceText.GetLength();
		nEndSel = nStartSel;
	}

	// Move back..
	PrevDlgCtrl();
	m_ctlReplaceRichEdit.SetSel(nStartSel, nEndSel);
}

void CSearchReplaceDlg::OnBnClickedUseCheck()
{
	UpdateView();
}

void CSearchReplaceDlg::OnCbnSelchangeUseCombo()
{
	OnBnClickedUseCheck();
}

void CSearchReplaceDlg::OnBnClickedCaseCheck()
{
	UpdateView();
}

void CSearchReplaceDlg::OnCbnSelchangeCaseCombo()
{
	OnBnClickedCaseCheck();
}

void CSearchReplaceDlg::OnBnClickedSeriesButton()
{
	AfxMessageBox(IDS_SERIES_HELP, MB_ICONINFORMATION);
}

void CSearchReplaceDlg::OnChangeSeries()
{
	if (!m_bInitialized)
		return;

	UpdateData();

	// If no replace string is provided, and "Series" is selected
	if (m_bSeries && m_strReplace.IsEmpty())
	{
		// then set replace replacement string by "Image %03d".
		m_strReplace.LoadString(IDS_SERIES_DEFAULT_REPLACE);
		UpdateData(FALSE);
		OnEnChangeReplace();
	}

	UpdateView();
}

void CSearchReplaceDlg::OnBnClickedID3TagCheck()
{
	UpdateView();
}

void CSearchReplaceDlg::OnBnClickedAdvancedCheck()
{
	UpdateView();
}

void CSearchReplaceDlg::UpdateView()
{
	UpdateData();

	// Update renaming sample.
	UpdateSample();

	// Colour the syntax.
	ColorSearchText();
	ColorReplacementText();
}

void CSearchReplaceDlg::ColorSearchText()
{
	// Color the search text syntax.
	SyntaxColor::CSyntaxColor::ClearColoring(m_ctlSearchRichEdit);
	if (m_bUse && m_nUse == CSearchReplaceFilter::useRegExp)
		m_vSyntaxColor[scRegExpSearch]->ColorSyntax(m_ctlSearchRichEdit);
	if (m_bUse && m_nUse == CSearchReplaceFilter::useWildcards)
		m_vSyntaxColor[scWildcardsSearch]->ColorSyntax(m_ctlSearchRichEdit);
}

void CSearchReplaceDlg::ColorReplacementText()
{
	// Color the replace text syntax.
	SyntaxColor::CSyntaxColor::ClearColoring(m_ctlReplaceRichEdit);
	if (m_bUse && m_nUse == CSearchReplaceFilter::useRegExp)
		m_vSyntaxColor[scRegExpReplace]->ColorSyntax(m_ctlReplaceRichEdit);
	if (m_bUse && m_nUse == CSearchReplaceFilter::useWildcards)
		m_vSyntaxColor[scWildcardsReplace]->ColorSyntax(m_ctlReplaceRichEdit);

	if (m_bSeries)
		m_vSyntaxColor[scEnumeration]->ColorSyntax(m_ctlReplaceRichEdit);

	m_vSyntaxColor[scGeneralMacros]->ColorSyntax(m_ctlReplaceRichEdit);

	if (m_bID3Tag)
		m_vSyntaxColor[scID3Tag]->ColorSyntax(m_ctlReplaceRichEdit);
}

void CSearchReplaceDlg::UpdateSample()
{
	// Enable disable items
	m_ctlSearchButton.EnableWindow(m_bUse);		// Drop-down menu

	GetDlgItem(IDC_USE_COMBO)->EnableWindow(m_bUse);

	GetDlgItem(IDC_WHOLE_TEXT)->EnableWindow(m_bAdvanced && (!m_bUse || m_nUse != CSearchReplaceFilter::useRegExp));	// If using regexp, disable the "whole text" option

	GetDlgItem(IDC_CASE_COMBO)->EnableWindow(m_bChangeCase);

	GetDlgItem(IDC_SERIE_START_EDIT)->EnableWindow(m_bSeries);
	GetDlgItem(IDC_SERIE_STEP_EDIT)->EnableWindow(m_bSeries);
	m_ctrlSeriesStartSpin.EnableWindow(m_bSeries);
	m_ctrlSeriesStepSpin.EnableWindow(m_bSeries);

	GetDlgItem(IDC_ALL_OCCURRENCES_CHECK)->EnableWindow(m_bAdvanced);

	// For disabled controls, change their value to the default value
	if (!GetDlgItem(IDC_WHOLE_TEXT)->IsWindowEnabled())
	{
		m_bMatchWholeText = false;
		GetDlgItem(IDC_WHOLE_TEXT)->SendMessage(BM_SETCHECK, BST_UNCHECKED);
	}
	if (!GetDlgItem(IDC_ALL_OCCURRENCES_CHECK)->IsWindowEnabled())
	{
		m_bAllOccurences = false;
		GetDlgItem(IDC_ALL_OCCURRENCES_CHECK)->SendMessage(BM_SETCHECK, BST_UNCHECKED);
	}

	// Update the filter
	m_filter.SetSearch(m_strSearch);
	m_filter.SetReplace(m_strReplace);

	if (m_bUse)
		m_filter.SetUse((CSearchReplaceFilter::EUse) m_nUse);
	else
		m_filter.SetUse(CSearchReplaceFilter::useNone);

	m_filter.SetCaseSensitive(m_bCaseSensitive != 0);

	m_filter.SetReplaceOnce(m_bAllOccurences == 0);

	m_filter.SetMatchWholeText(m_bMatchWholeText != 0);

	if (m_bChangeCase)
		m_filter.SetChangeCase((CSearchReplaceFilter::EChangeCase) m_nChangeCase);
	else
		m_filter.SetChangeCase(CSearchReplaceFilter::caseNone);

	m_filter.SetSeriesEnabled(m_bSeries != 0);
	m_filter.SetSeriesStart(m_nSerieStart);
	m_filter.SetSeriesStep(m_nSerieStep);

	m_filter.SetID3TagEnabled(m_bID3Tag != 0);

	// Update the sample
	GetDlgItem(IDC_AFTER)->SetWindowText( m_previewSamples.PreviewRenaming(&m_filter).GetFilteredSubstring() );
}

void CSearchReplaceDlg::OnMatchWholeText()
{
	UpdateView();
}

BOOL CSearchReplaceDlg::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_NCLBUTTONDOWN:
		{
			CString str;
			str.Format(_T(" %d \n"), pMsg->message);
			TRACE(str);
//			CFrameWnd* pFrameWnd = GetTopLevelFrame();
//			pFrameWnd->m_bHelpMode = true;
//			->m_pszHelpFilePath
//			pFrameWnd->OnContextHelp();
//			CWinApp::OnContextHelp();
//			this->SendMessage(WM_COMMAND, 0, ID_CONTEXT_HELP);

		}
	}
	if (NULL != m_pToolTip)
		m_pToolTip->RelayEvent(pMsg);

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CSearchReplaceDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
	// TODO: Add your message handler code here and/or call default
/*	if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
		return ::HtmlHelp(
			(HWND)pHelpInfo->hItemHandle,
			_T("\\Rename-It!.chm::/ctrlhlp.txt"),
			HH_TP_HELP_WM_HELP,
			(DWORD)(LPVOID)myarray) != NULL;
		return ::HtmlHelp(
			(HWND)pHelpInfo->hItemHandle,
			_T("\\Rename-It!.chm::/ctrlhlp.txt"),
			HELP_CONTENTS,
			0) != NULL;
	}
	return TRUE;*/

	return CDialog::OnHelpInfo(pHelpInfo);
}

void CSearchReplaceDlg::HtmlHelp(DWORD_PTR dwData, UINT nCmd)
{
	// TODO: Add your specialized code here and/or call the base class
	CDialog::HtmlHelp(dwData, nCmd);
}

HRESULT CSearchReplaceDlg::get_accHelpTopic(BSTR *pszHelpFile, VARIANT varChild, long *pidTopic)
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialog::get_accHelpTopic(pszHelpFile, varChild, pidTopic);
}

void CSearchReplaceDlg::OnDestroy()
{
	m_bInitialized = false;
	CDialog::OnDestroy();
}
