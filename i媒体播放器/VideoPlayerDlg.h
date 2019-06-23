// VideoPlayerDlg.h : header file
//

#if !defined(AFX_VIDEOPLAYERDLG_H__B35A40C9_7DB7_11D3_860F_F6E44D6C9F3C__INCLUDED_)
#define AFX_VIDEOPLAYERDLG_H__B35A40C9_7DB7_11D3_860F_F6E44D6C9F3C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CVideoPlayerDlg dialog

#include "DigitalVideo.h"

class CVideoPlayerDlg : public CDialog
{
// Construction
public:
	CVideoPlayerDlg(CWnd* pParent = NULL);	// standard constructor
// Dialog Data
	enum { IDD = IDD_VIDEOPLAYER_DIALOG };
	CSliderCtrl m_slider;
	CSliderCtrl m_slider_volumn;
	CComboBox m_comboSpeed;


	// ClassWizard generated virtual function overrides
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON    m_hIcon;
	CDigitalvideo   m_Video;
	BOOL     m_Open;
	BOOL     m_Sound;
	BOOL     m_Loop;
	BOOL     m_FullScreen;
	CString  m_str;
	CDialog  *Display;
	int      m_flag ;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnOpen();
	afx_msg void OnInformation();
	afx_msg void OnStop();
	afx_msg void OnPause();
	afx_msg void OnPlay();
	afx_msg void OnNext();
	afx_msg void OnPrev();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLoop();
	afx_msg void OnSound();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnClose();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnStnClickedDisplaybox2();
	afx_msg void OnNMCustomdrawSliderVilumn(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCbnSelchangeChangeSpeed();
	afx_msg void OnStnDblclickDisplaybox2();
};


#endif // !defined(AFX_VIDEOPLAYERDLG_H__B35A40C9_7DB7_11D3_860F_F6E44D6C9F3C__INCLUDED_)
