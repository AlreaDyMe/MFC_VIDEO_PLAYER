// DigitalVideo.cpp: implementation of the CDigitalvideo class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "VideoPlayer.h"
#include "Digitalvideo.h"
#include "mmsystem.h"
#pragma comment(lib,"winmm.lib")   //加上这个

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDigitalvideo::CDigitalvideo()
{
	// 初始化成员变量
	m_Open = m_Pause = m_Playing = FALSE;
	m_DeviceID = 0;
}

CDigitalvideo::~CDigitalvideo()
{
	Close ();
}
BOOL CDigitalvideo::GetPause ()//是否暂停
{
	return m_Pause;
}
BOOL CDigitalvideo::Open (CString Filename, CWnd *pWnd, int left, int top,int width,int height)
{
	if (!m_Open){//如果m_Open=false，没有打开设备
		// 初始化成员变量
		m_Open = m_Pause = m_Playing = FALSE;
		m_DeviceID = 0;
		// 文件是否存在
		CFileStatus Status;
		if (!CFile::GetStatus (Filename, Status))
			return FALSE;

		// 打开设备
		MCI_DGV_OPEN_PARMS MCIOpen;
		MCIOpen.lpstrDeviceType = "mpegvideo";//设置播放设备为视频播放设备
		MCIOpen.lpstrElementName = (LPSTR) Filename.GetBuffer (0);//文件名
		MCIOpen.wDeviceID = NULL;
		MCIOpen.hWndParent = pWnd->m_hWnd;//可播放视频文件窗口指针
		MCIOpen.dwStyle = WS_CHILD | WS_VISIBLE;
		if (mciSendCommand (NULL, MCI_OPEN, MCI_WAIT | MCI_OPEN_TYPE |
			MCI_OPEN_ELEMENT | MCI_DGV_OPEN_PARENT | MCI_DGV_OPEN_WS, (DWORD)(LPVOID) &MCIOpen))
			return FALSE;
		m_DeviceID = MCIOpen.wDeviceID;

    ////////////////////////////////////////////////////////////////////////////////////////
		//MCIDEVICEID WINAPI mciGetDeviceID(LPCSTR pszDevice);
//		m_DeviceID1 =mciGetDeviceID(LPCSTR("AVIvideo"));
  
	////////////////////////////////////////////////////////////////////////////////////////
		m_Open = TRUE;

		// 将文件长度设置为帧
		MCI_DGV_SET_PARMS MCISet;
		MCISet.dwTimeFormat = MCI_FORMAT_FRAMES;
		if (mciSendCommand (m_DeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)(LPVOID) &MCISet)){
			Close ();
			return FALSE;
			}
		// 找到视频文件的开始位置
		mciSendCommand (m_DeviceID, MCI_SEEK, MCI_SEEK_TO_START, NULL);
		//将播放画面置于主窗口之中
		MCI_DGV_PUT_PARMS MCIPut;
		MCIPut.rc.left = left;
		MCIPut.rc.top  = top;
		MCIPut.rc.right = width;
		MCIPut.rc.bottom = height;
		if (mciSendCommand (m_DeviceID, MCI_PUT, MCI_WAIT | MCI_DGV_PUT_CLIENT |
			MCI_DGV_PUT_WINDOW | MCI_DGV_RECT, (DWORD)(LPVOID) &MCIPut)){
			Close ();
			return FALSE;
			}
		}
	return TRUE;
}
void CDigitalvideo::Close (void)
{
	if (m_Open){// 停止播放并关闭设备
		if (m_Playing || m_Pause)
			mciSendCommand (m_DeviceID, MCI_STOP, NULL, NULL);
		mciSendCommand (m_DeviceID, MCI_CLOSE, NULL, NULL);
		// 清空成员变量
		m_Open = m_Pause = m_Playing = FALSE;
		m_DeviceID = NULL;
		}
}
//获得视频文件窗口的宽度
int CDigitalvideo::GetWidth (void)
{
	if (m_Open){
		MCI_DGV_WHERE_PARMS MCIWhere;
		if (mciSendCommand (m_DeviceID, MCI_WHERE, MCI_DGV_WHERE_SOURCE,
			(DWORD)(LPVOID) &MCIWhere)){
			return 0;
			}
		return (int) MCIWhere.rc.right;
		}
	return 0;
}
//获得视频文件窗口的高度
int CDigitalvideo::GetHeight (void)
{
	if (m_Open){
		MCI_DGV_WHERE_PARMS MCIWhere;
		if (mciSendCommand (m_DeviceID, MCI_WHERE, MCI_DGV_WHERE_SOURCE, 
			(DWORD)(LPVOID) &MCIWhere)){
			return 0;
			}
		return (int) MCIWhere.rc.bottom;
		}
	return 0;
}
void CDigitalvideo::OnFullscreen()
{
	// TODO: Add your command handler code here
	MCI_DGV_WHERE_PARMS PlayParmsFull;
	if (m_FullScreen) m_flag |= MCI_MCIAVI_PLAY_FULLSCREEN;
	mciSendCommand(m_DeviceID, MCI_PLAY, m_flag, (DWORD)(LPVOID)&PlayParmsFull);
	m_FullScreen = !m_FullScreen;

}

BOOL CDigitalvideo::Play (BOOL bLoop, BOOL bFullScreen)
{
	if (m_Open){
		// 如果是视频的尾部则重新开始播放
		if (GetFrame () >= GetFrames ())
			mciSendCommand (m_DeviceID, MCI_SEEK, MCI_SEEK_TO_START, NULL);
		// 播放
		MCI_DGV_PLAY_PARMS MCIPlay;
		MCIPlay.dwCallback = NULL;
		MCIPlay.dwFrom = GetFrame ();
		DWORD dwFlags = MCI_FROM;
		if (bLoop)
			dwFlags |= MCI_DGV_PLAY_REPEAT;
		if (bFullScreen)
			dwFlags |= MCI_MCIAVI_PLAY_FULLSCREEN;
		if (mciSendCommand (m_DeviceID, MCI_PLAY, dwFlags, (DWORD)(LPVOID) &MCIPlay))
			return FALSE;
		m_Pause = FALSE;
		m_Playing = TRUE;
		return TRUE;
		}
	return FALSE;
}
void CDigitalvideo::Stop (void)
{
	if (m_Open && m_Playing){
		mciSendCommand (m_DeviceID, MCI_STOP, NULL, NULL);
		mciSendCommand (m_DeviceID, MCI_SEEK, MCI_SEEK_TO_START, NULL);
		m_Playing = FALSE;
		m_Pause = TRUE;
		}
}
void CDigitalvideo::Pause( void )
{
	if (m_Open ){
		MCI_DGV_PAUSE_PARMS MCIPause;
		if (mciSendCommand (m_DeviceID, MCI_PAUSE, 0, (DWORD)(LPVOID) &MCIPause))
			return;
		m_Pause = TRUE;
		}
}
int CDigitalvideo::GetFrames (void)
{
	if (m_Open){
		MCI_DGV_STATUS_PARMS MCIStatus;
		MCIStatus.dwItem = MCI_STATUS_LENGTH;
		if (mciSendCommand (m_DeviceID, MCI_STATUS, MCI_WAIT |
			MCI_STATUS_ITEM, (DWORD)(LPVOID) &MCIStatus))
			return 0;
		return (int) MCIStatus.dwReturn;
		}
	return 0;
}
int CDigitalvideo::GetFrame (void)
{
	if (m_Open){
		MCI_DGV_STATUS_PARMS MCIStatus;
		MCIStatus.dwItem = MCI_STATUS_POSITION;
		if (mciSendCommand (m_DeviceID, MCI_STATUS, MCI_WAIT |
			MCI_STATUS_ITEM, (DWORD)(LPVOID) &MCIStatus))
			return -1;
		return (int) MCIStatus.dwReturn;
		}
	return -1;
}
BOOL CDigitalvideo::IsPlaying ()
{
	if (m_Open)
	{
		MCI_DGV_STATUS_PARMS MCIStatus;
		MCIStatus.dwItem = MCI_STATUS_MODE;
		if (mciSendCommand (m_DeviceID, MCI_STATUS, MCI_WAIT |
			MCI_STATUS_ITEM, (DWORD)(LPVOID) &MCIStatus))
			return FALSE;
		if (MCIStatus.dwReturn == MCI_MODE_PLAY)
		{
			m_Playing = TRUE;
			m_Pause = FALSE;
			return TRUE;
		}
		else if (MCIStatus.dwReturn == MCI_MODE_PAUSE)
		{
			m_Playing = TRUE;
			m_Pause = FALSE;
			return TRUE;
		}
		else
		{
			m_Playing = FALSE;
			m_Pause = FALSE;
			return FALSE;
		}
	}
	return FALSE;
}
BOOL CDigitalvideo::SeekTo (int nFrame)
{
	if (m_Open){
		MCI_SEEK_PARMS MCISeek;
		MCISeek.dwTo = nFrame;
		if (m_Playing && !m_Pause)
			mciSendCommand (m_DeviceID, MCI_PAUSE, 0, NULL);
		if (mciSendCommand (m_DeviceID, MCI_SEEK, MCI_TO | 
			MCI_WAIT, (DWORD)(LPVOID) &MCISeek))
			return FALSE;
		if (m_Playing && !m_Pause)
			Play ();
		return TRUE;
		}
	return FALSE;
}
void CDigitalvideo::Forward (int nFrames)
{
	if (m_Open){
		if (m_Playing && !m_Pause)
			Pause ();
		MCI_DGV_STEP_PARMS MCIStep;
		MCIStep.dwFrames = nFrames;
		mciSendCommand (m_DeviceID, MCI_STEP, MCI_DGV_STEP_FRAMES, 
			(DWORD)(LPVOID) &MCIStep);
		}
}
void CDigitalvideo::Backward (int nFrames)
{
	if (m_Open){
		if (m_Playing && !m_Pause)
			Pause ();
		MCI_DGV_STEP_PARMS MCIStep;
		MCIStep.dwFrames = nFrames;
		mciSendCommand (m_DeviceID, MCI_STEP, MCI_DGV_STEP_REVERSE |
			MCI_DGV_STEP_FRAMES, (DWORD)(LPVOID) &MCIStep);
		}
}
int CDigitalvideo::GetSpeed (void)
{
	if (m_Open){
		MCI_DGV_STATUS_PARMS MCIStatus;
		MCIStatus.dwItem = MCI_DGV_STATUS_SPEED;
		if (mciSendCommand (m_DeviceID, MCI_STATUS, MCI_WAIT |
			MCI_STATUS_ITEM, (DWORD)(LPVOID) &MCIStatus))
			return -1;
		return (int) MCIStatus.dwReturn;
		}
	return -1;
}
void CDigitalvideo::SetSpeed (int nSpeed)
{
	if (m_Open){
		MCI_DGV_SET_PARMS MCISet;
		MCISet.dwSpeed = nSpeed;
		mciSendCommand (m_DeviceID, MCI_SET, MCI_WAIT |
			MCI_DGV_SET_SPEED, (DWORD)(LPVOID) &MCISet);
		}
}
void CDigitalvideo::SetSound (BOOL bSound)
{
	if (m_Open)
	{
		MCI_DGV_SET_PARMS MCISet;
		MCISet.dwAudio = MCI_SET_AUDIO_ALL;
		DWORD dwFlags = MCI_SET_AUDIO;
		if (bSound)
			dwFlags |= MCI_SET_ON;
		else
			dwFlags |= MCI_SET_OFF;
		mciSendCommand (m_DeviceID, MCI_SET, dwFlags, (DWORD)(LPVOID) &MCISet);
	}
}
BOOL CDigitalvideo::GetSound (void)
{
	if (m_Open)
	{
		MCI_DGV_STATUS_PARMS MCIStatus;
		MCIStatus.dwItem = MCI_DGV_STATUS_AUDIO;
		if (mciSendCommand (m_DeviceID, MCI_STATUS, MCI_WAIT | 
			MCI_STATUS_ITEM, (DWORD)(LPVOID) &MCIStatus))
			return FALSE;
		if (MCIStatus.dwReturn == MCI_ON)
			return TRUE;
		else
			return FALSE;
	}

	return FALSE;
}

void CDigitalvideo::setVolume(UINT volume)
{
	if (!m_DeviceID)
		return;
	MCI_DGV_SETAUDIO_PARMS mciParams = { 0 };
	mciParams.dwItem = MCI_DGV_SETAUDIO_VOLUME;
	mciParams.dwValue = volume;
	mciSendCommand(m_DeviceID, MCI_SETAUDIO, MCI_DGV_SETAUDIO_ITEM | MCI_DGV_SETAUDIO_VALUE, (DWORD)&mciParams);
}

UINT CDigitalvideo::Volume() const
{
	if (!m_DeviceID)
		return 0;
	MCI_STATUS_PARMS mciParams = { 0 };
	mciParams.dwItem = MCI_DGV_STATUS_VOLUME;
	mciSendCommand(m_DeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)&mciParams);
	return mciParams.dwReturn;
}