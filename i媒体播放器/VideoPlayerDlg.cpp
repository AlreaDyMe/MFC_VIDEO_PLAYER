// VideoPlayerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VideoPlayer.h"
#include "VideoPlayerDlg.h"
#include "infodlg.h"
#include "Digitalvideo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };
	// ClassWizard generated virtual function overrides
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	// 处理控件IDC_SUMMAND_EDIT和变量m_editSummand之间的数据交换   
	
	// 处理控件IDC_ADDEND_EDIT和变量m_editAddend之间的数据交换   
	//DDX_Text(pDX, IDC_ADDEND_EDIT, m_editAddend);
	// 处理控件IDC_SUM_EDIT和变量m_editSum之间的数据交换   
	//DDX_Text(pDX, IDC_SUM_EDIT, m_editSum);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVideoPlayerDlg dialog

CVideoPlayerDlg::CVideoPlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVideoPlayerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_Open = FALSE;
	m_Loop = FALSE;
	m_FullScreen = FALSE;
	m_Sound = TRUE;
	m_flag = 0;
	m_str = "";
}

void CVideoPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVideoPlayerDlg)
	DDX_Control(pDX, IDC_SCROLLBAR, m_slider);
	DDX_Control(pDX, IDC_SLIDER_VOLUMN, m_slider_volumn);
	DDX_Control(pDX, IDC_CHANGE_SPEED, m_comboSpeed);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CVideoPlayerDlg, CDialog)
	//{{AFX_MSG_MAP(CVideoPlayerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_OPEN, OnOpen)
	ON_BN_CLICKED(IDC_INFORMATION, OnInformation)
	ON_BN_CLICKED(IDC_STOP, OnStop)
	ON_BN_CLICKED(IDC_PAUSE, OnPause)
	ON_BN_CLICKED(IDC_PLAY, OnPlay)
	ON_BN_CLICKED(IDC_NEXT, OnNext)
	ON_BN_CLICKED(IDC_PREV, OnPrev)
	ON_WM_TIMER()
	ON_COMMAND(IDC_LOOP, OnLoop)
	ON_WM_HSCROLL()
	ON_WM_LBUTTONDBLCLK()
	ON_BN_CLICKED(IDC_CLOSE, OnClose)
	//}}AFX_MSG_MAP
	ON_STN_CLICKED(IDC_DISPLAYBOX2, &CVideoPlayerDlg::OnStnClickedDisplaybox2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_VOLUMN, &CVideoPlayerDlg::OnNMCustomdrawSliderVilumn)
	ON_CBN_SELCHANGE(IDC_CHANGE_SPEED, &CVideoPlayerDlg::OnCbnSelchangeChangeSpeed)
	ON_STN_DBLCLK(IDC_DISPLAYBOX2, &CVideoPlayerDlg::OnStnDblclickDisplaybox2)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVideoPlayerDlg message handlers

BOOL CVideoPlayerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, FALSE);		// Set small icon	
	// TODO: Add extra initialization here
	//创建用于显示动画的对话框窗口,但是还不用显示
   	Display = new CDialog;
    Display->Create(IDD_DISPLAY);
	SetTimer(1,100,NULL);

	GetDlgItem(IDC_SCROLLBAR)->EnableWindow(FALSE);//用来控制界面显示不显示滑动条，可以在不使用时关闭。
	GetDlgItem(IDC_SLIDER_VOLUMN)->EnableWindow(TRUE);//用来控制界面显示不显示滑动条，可以在不使用时关闭。
	m_slider.SetRange(0, 100);
	m_slider.SetTicFreq(1);
	m_slider_volumn.SetRange(0, 10);
	m_slider_volumn.SetTicFreq(1);
	m_slider_volumn.SetPos(8);
	// 为组合框控件的列表框添加列表项
	m_comboSpeed.InsertString(0,"x 1");
	m_comboSpeed.InsertString(1,"x 2");
	m_comboSpeed.InsertString(2, "x 3");
	m_comboSpeed.SetCurSel(0);// 默认选择第一项

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CVideoPlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}
void CVideoPlayerDlg::OnPaint() 
{
	if (IsIconic())//最小化
	{
		CPaintDC dc(this); // device context for painting
		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);
		// Center icon in client rectangle
		int xIcon = GetSystemMetrics(SM_CXICON);
		int yIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - xIcon + 1) / 2;
		int y = (rect.Height() - yIcon + 1) / 2;
	}
	else
	{
		CDialog::OnPaint();
	}
}
HCURSOR CVideoPlayerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}
void CVideoPlayerDlg::OnOpen() 
{
	// TODO: Add your control notification handler code here
	if(m_Open)
	{
		AfxMessageBox("请在打开视频文件前先关闭原来的文件");
		m_Video.Pause();
		return;
	}
	CString Filter, Filter1, Filter2,str;
	Filter = "*dat|*.dat|*avi|*.avi|*mp4|*.mp4||";

	//得到窗口尺寸
	CRect rect,itemrect;
	this->GetWindowRect(rect);
	CFileDialog FileDlg (TRUE, NULL, NULL, OFN_HIDEREADONLY, Filter);
	if (FileDlg.DoModal () == IDOK)//单击文件选择对话框的"确定"按钮
	{
	    //CWnd *m_wnd=GetDlgItem(IDC_DISPLAYBOX);
        CWnd *m_wnd=GetDlgItem(IDC_DISPLAYBOX2);
		m_wnd->GetWindowRect(itemrect);
	    if (!m_Video.Open(FileDlg.GetPathName(),m_wnd,0,0,itemrect.Width(),itemrect.Height()))//打开视频文件
		{
	         MessageBox ("无法打开该文件!", "Error", MB_OK | MB_ICONSTOP);
	         return;
		}		
		m_str = FileDlg.GetFileName();
		
		m_Video.SetSpeed(1020);
	    m_Open=TRUE;
		SetDlgItemText(IDC_STATIC_NAME, m_str);
		GetDlgItem(IDC_SCROLLBAR)->EnableWindow(TRUE);//用来控制界面显示不显示滑动条
		
	}
}
void CVideoPlayerDlg::OnStop() 
{
	m_Video.Stop();
}
void CVideoPlayerDlg::OnPause() 
{
	m_Video.Pause();
}
void CVideoPlayerDlg::OnPlay() 
{
	m_Video.Play(m_Loop,m_FullScreen);
}

void CVideoPlayerDlg::OnNext() 
{
	m_Video.Forward(1);
	m_slider.SetScrollPos(IDC_SCROLLBAR,m_Video.GetFrame()*100/m_Video.GetFrames());
}

void CVideoPlayerDlg::OnPrev() 
{
	m_Video.Backward(1);
	m_slider.SetScrollPos(IDC_SCROLLBAR,m_Video.GetFrame()*100/m_Video.GetFrames(),1);
}

void CVideoPlayerDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	CWnd *m_Wnd=GetDlgItem(IDC_PLAY);
	m_Wnd->EnableWindow(m_Open);//如果m_Open为真
	m_Wnd=GetDlgItem(IDC_PAUSE);
	m_Wnd->EnableWindow(m_Open);
	m_Wnd=GetDlgItem(IDC_STOP);
	m_Wnd->EnableWindow(m_Open);
	m_Wnd=GetDlgItem(IDC_NEXT);
	m_Wnd->EnableWindow(m_Open);
	m_Wnd=GetDlgItem(IDC_LAST);
	m_Wnd->EnableWindow(m_Open);
	if(m_Open)
	{
		CString Info;
		Info.Format("%d",m_Video.GetFrame());
		if(!m_Video.GetPause()){
			int frame;
			frame = (m_Video.GetFrame()*100)/m_Video.GetFrames();
		    m_slider.SetScrollPos(IDC_SCROLLBAR,frame,1);
		}
		m_Wnd = GetDlgItem(IDC_DISPLAY);
		CClientDC dc(m_Wnd);
		//
		CRect rect;
		CBrush brush(RGB(255,255,255)),*OldBrush;
		OldBrush=(CBrush *)dc.SelectObject(brush);

		m_Wnd->GetClientRect(rect);
		dc.SetTextColor(RGB(255,0,0));
		dc.SetBkColor(RGB(255,255,255));
		dc.FillRect(rect,&brush);
		CSize size=dc.GetTextExtent(Info);
		dc.TextOut((rect.Width()-size.cx)/2,(rect.Height()-size.cy)/2,Info);
		dc.SelectObject(&OldBrush);
	 }
	CDialog::OnTimer(nIDEvent);
}
void CVideoPlayerDlg::OnLoop() 
{
	// TODO: Add your command handler code here
	m_Loop=!m_Loop;
}
void CVideoPlayerDlg::OnSound() 
{
	// TODO: Add your command handler code here
	m_Sound=!m_Sound;
}

void CVideoPlayerDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScroll) 
{
	int frame;
	switch (pScroll->GetDlgCtrlID())
	{
	case IDC_SLIDER_VOLUMN:
		pScroll->SetScrollPos(nPos);
		if (m_Open) {
			//m_Video.setVolume(nPos);
			CString str;
			str.Format("音量:%d", ((int)nPos));
			SetDlgItemText(IDC_VOLUMN_VALUE, str);
			pScroll->SetScrollPos(nPos);
		}
		break;
	case IDC_SCROLLBAR:
		switch (nSBCode)
		{
		case SB_THUMBPOSITION://拖动滑动块
			pScroll->SetScrollPos(nPos);
			if (m_Open) {
				m_Video.Pause();
				frame = (nPos*m_Video.GetFrames()) / 100;
				m_Video.SeekTo(frame);
				pScroll->SetScrollPos(nPos);
			}
			break;
		case SB_LINELEFT:
			frame = 100 / m_Video.GetFrames();
			if (frame <= 1)frame = 1;
			pScroll->SetScrollPos(pScroll->GetScrollPos() - frame);
			if (m_Open) {
				m_Video.Pause();
				frame = (pScroll->GetScrollPos()*m_Video.GetFrames()) / 100;
				m_Video.SeekTo(frame);
			}
			break;
		case SB_LINERIGHT:
			frame = 100 / m_Video.GetFrames();
			if (frame <= 1)frame = 1;
			pScroll->SetScrollPos(pScroll->GetScrollPos() + frame);
			if (m_Open) {
				m_Video.Pause();
				frame = (pScroll->GetScrollPos()*m_Video.GetFrames()) / 100;
				m_Video.SeekTo(frame);
				m_Video.Play();
			}
			break;
		case SB_THUMBTRACK:
		{
			pScroll->SetScrollPos(nPos);
			m_Video.Pause();
			break;
		}
		case SB_PAGELEFT:
		{
			int pos = pScroll->GetScrollPos();
			frame = 5 * 100 / m_Video.GetFrames();
			if (frame <= 1)frame = 1;
			pScroll->SetScrollPos(pScroll->GetScrollPos() - frame);
			if (m_Open) {
				m_Video.Pause();
				frame = (pScroll->GetScrollPos()*m_Video.GetFrames()) / 100;
				m_Video.SeekTo(frame);
			}
			break;
		}
		case SB_PAGERIGHT:
		{
			int pos = pScroll->GetScrollPos();
			frame = 5 * 100 / m_Video.GetFrames();
			if (frame <= 1)frame = 1;
			pScroll->SetScrollPos(pScroll->GetScrollPos() + frame);
			if (m_Open) {
				m_Video.Pause();
				frame = (pScroll->GetScrollPos()*m_Video.GetFrames()) / 100;
				m_Video.SeekTo(frame);
			}
			break;
		}
		default:
			break;
		}
		break;
	default:
		break;
	}
	
	CDialog::OnHScroll(nSBCode, nPos, pScroll);
}
void CVideoPlayerDlg::OnInformation() 
{
	if(!m_Open){
		AfxMessageBox("当前没有视频文件");
		return ;
	}
	Cinfodlg dlg;
	CString str;
	str.Format("%d 帧/分",m_Video.GetSpeed());
	dlg.m_strspeed = str;
	str.Format("%d 帧",m_Video.GetFrames());
	dlg.m_strframe = str;
	str.Format("宽 %d 高 %d",m_Video.GetWidth(),m_Video.GetHeight());
	dlg.m_strrect  = str;
	//SetDlgItemText(IDC_FRAMES,str);
	SetDlgItemText(IDC_PLAY_SOUND,str);
	dlg.m_strname  = m_str;
	dlg.DoModal();
}
void CVideoPlayerDlg::OnClose() 
{
		m_Video.Close();
		m_Open = 0;
}



void CVideoPlayerDlg::OnNMCustomdrawSliderVilumn(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 10;
}


void CVideoPlayerDlg::OnCbnSelchangeChangeSpeed()
{
	// TODO:获取组合框控件的列表框中选中项的索引   
	int nSel = m_comboSpeed.GetCurSel();
	m_Video.SetSpeed((nSel+1)*1020);
}


void CVideoPlayerDlg::OnStnClickedDisplaybox2()
{
	// TODO:实现单击屏幕暂停
	m_Video.Pause();
}

void CVideoPlayerDlg::OnStnDblclickDisplaybox2()
{
	// TODO: 实现双击全屏播放
	m_Video.OnFullscreen();
}
