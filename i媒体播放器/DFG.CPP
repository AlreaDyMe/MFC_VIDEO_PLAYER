// dfg.cpp : implementation file
//

#include "stdafx.h"
#include "VideoPlayer.h"
#include "dfg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// dfg dialog


dfg::dfg(CWnd* pParent /*=NULL*/)
	: CDialog(dfg::IDD, pParent)
{
	//{{AFX_DATA_INIT(dfg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void dfg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(dfg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(dfg, CDialog)
	//{{AFX_MSG_MAP(dfg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// dfg message handlers
