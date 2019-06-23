// DigitalVideo.h: interface for the CDigitalvideo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIDEO_H__B35A40D1_7DB7_11D3_860F_F6E44D6C9F3C__INCLUDED_)
#define AFX_VIDEO_H__B35A40D1_7DB7_11D3_860F_F6E44D6C9F3C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <afxwin.h>
#include <mmsystem.h>
#include <mciavi.h>
#include <Digitalv.h>

class CDigitalvideo  //处理视频类
{
public:
	CDigitalvideo();//构造函数
	virtual ~CDigitalvideo();//构析函
	//视频操作函数
	//函数功能-打开视频文件，pWnd--将要包含视频播放窗口的主窗口，
	//left、top-视频播放窗口在主窗口中的左上角坐标
	BOOL Open (CString, CWnd *pWnd, int left = 0, //函数功能-打开视频文件
		int top = 0,int width = 20,int height = 20);
	void Close (void);//关闭视频文件
	BOOL Play (BOOL bLoop = FALSE, BOOL bFullScreen = FALSE);//函数功能-播放视频文件
	void Stop (void);   //停止播放
	void Pause (void);  //暂停播放
	void Forward (int);//快进
	void Backward (int);//后退
	BOOL SeekTo (int);//跳到某一位置,处于暂停状态
	//视频状态还回函数
	int GetWidth (void);//得到视频宽度
	int GetHeight (void);//获得视频高度
	void OnFullscreen();
	int GetFrames (void);//获得视频总帧数
	int GetFrame (void);//获得当前帧
	BOOL IsPlaying ();//是否正在播放
	int GetSpeed (void);//获得播放速度
	void SetSpeed (int);//设置播放速度
	BOOL GetPause (void);//是否暂停	

	void CDigitalvideo::SetSound(BOOL bSound);
	BOOL CDigitalvideo::GetSound(void);


	void setVolume(UINT volume);
	UINT Volume() const;

private://数据成员
	BOOL m_Open;//是否有媒体文件打开
	BOOL m_Pause;//是否暂停
	BOOL m_Playing;//是否正在播放
	WORD m_DeviceID;//播放设备ID
	WORD m_DeviceID1;//测试用播放设备ID
	BOOL m_FullScreen;
	int m_flag;
};

#endif // !defined(AFX_VIDEO_H__B35A40D1_7DB7_11D3_860F_F6E44D6C9F3C__INCLUDED_)
