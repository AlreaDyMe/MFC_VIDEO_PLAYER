#if !defined(AFX_DFG_H__5CB74240_8F95_11D4_B67F_444553540000__INCLUDED_)
#define AFX_DFG_H__5CB74240_8F95_11D4_B67F_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// dfg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// dfg dialog

class dfg : public CDialog
{
// Construction
public:
	dfg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(dfg)
	enum { IDD = IDD_DIALOG1 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(dfg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(dfg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DFG_H__5CB74240_8F95_11D4_B67F_444553540000__INCLUDED_)
