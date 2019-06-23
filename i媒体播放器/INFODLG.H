#if !defined(AFX_INFODLG_H__51095CA1_7D0A_11D4_92FF_5254AB10AACA__INCLUDED_)
#define AFX_INFODLG_H__51095CA1_7D0A_11D4_92FF_5254AB10AACA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// infodlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// Cinfodlg dialog

class Cinfodlg : public CDialog
{
// Construction
public:
	Cinfodlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(Cinfodlg)
	enum { IDD = IDD_DISPLAY };
	CString	m_strframe;
	CString	m_strname;
	CString	m_strrect;
	CString	m_strspeed;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Cinfodlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(Cinfodlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INFODLG_H__51095CA1_7D0A_11D4_92FF_5254AB10AACA__INCLUDED_)
