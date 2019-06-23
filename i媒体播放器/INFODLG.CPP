// infodlg.cpp : implementation file
//


#include "stdafx.h"
#include "VideoPlayer.h"
#include "infodlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Cinfodlg dialog


Cinfodlg::Cinfodlg(CWnd* pParent /*=NULL*/)
	: CDialog(Cinfodlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(Cinfodlg)
	m_strframe = _T("");
	m_strname = _T("");
	m_strrect = _T("");
	m_strspeed = _T("");
	//}}AFX_DATA_INIT
}


void Cinfodlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Cinfodlg)
	DDX_Text(pDX, IDC_EDITFRAMES, m_strframe);
	DDX_Text(pDX, IDC_EDITNAME, m_strname);
	DDX_Text(pDX, IDC_EDITRECT, m_strrect);
	DDX_Text(pDX, IDC_EDITSPEED, m_strspeed);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Cinfodlg, CDialog)
	//{{AFX_MSG_MAP(Cinfodlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Cinfodlg message handlers
