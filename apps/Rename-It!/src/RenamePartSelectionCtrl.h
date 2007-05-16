#pragma once
#include <vector>

// Notification message sent every time the part to be renamed is modified
#define RPS_SELCHANGE 0x1000

// CRenamePartSelectionCtrl

class CRenamePartSelectionCtrl : public CRichEditCtrl
{
	DECLARE_DYNAMIC(CRenamePartSelectionCtrl)

public:
	CRenamePartSelectionCtrl();
	virtual ~CRenamePartSelectionCtrl();

	// Parts to be renamed
	enum ERenamePartFlags {
		renameFolders = 0x01,
		renameFilename = 0x02,
		renameExtension = 0x04
	};

	// Retrieve the selected parts to be renamed
	UINT GetRenameParts() const {
		return m_nRenamePart;
	}

	// Define the selected parts to be renamed
	void SetRenameParts(UINT nRenamePart) {
		std::vector<POSSIBLE_SELECTION>::iterator iter;
		for (iter=m_vPossibleSelections.begin(); iter!=m_vPossibleSelections.end(); ++iter)
			if ((*iter).nRenamePart == nRenamePart)
				SetSel((*iter).cr);
	}

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	virtual void PreSubclassWindow();
	afx_msg void OnEnSelchange(NMHDR *pNMHDR, LRESULT *pResult);

	struct POSSIBLE_SELECTION {
		CHARRANGE	cr;
		UINT		nRenamePart;

		POSSIBLE_SELECTION(LONG nStartChar, LONG nEndChar, int nRenamePart)
		{
			this->cr.cpMin = nStartChar;
			this->cr.cpMax = nEndChar;
			this->nRenamePart = nRenamePart;
		}
	};
	std::vector<POSSIBLE_SELECTION> m_vPossibleSelections;
	UINT	m_nRenamePart;
};
