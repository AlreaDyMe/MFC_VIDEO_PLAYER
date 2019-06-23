; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CVideoPlayerDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "VideoPlayer.h"

ClassCount=6
Class1=CVideoPlayerApp
Class2=CVideoPlayerDlg
Class3=CAboutDlg

ResourceCount=5
Resource1=IDD_DISPLAY1
Resource2=IDD_ABOUTBOX
Resource3=IDD_VIDEOPLAYER_DIALOG
Class4=CSpeed
Class5=Cinfodlg
Class6=dfg
Resource4=IDD_PLAYER_DLG
Resource5=IDD_DISPLAY

[CLS:CVideoPlayerApp]
Type=0
HeaderFile=VideoPlayer.h
ImplementationFile=VideoPlayer.cpp
Filter=N

[CLS:CVideoPlayerDlg]
Type=0
HeaderFile=VideoPlayerDlg.h
ImplementationFile=VideoPlayerDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=IDC_OPEN

[CLS:CAboutDlg]
Type=0
HeaderFile=VideoPlayerDlg.h
ImplementationFile=VideoPlayerDlg.cpp
Filter=D
LastObject=CAboutDlg

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_VIDEOPLAYER_DIALOG]
Type=1
Class=CVideoPlayerDlg
ControlCount=11
Control1=IDC_OPEN,button,1342242817
Control2=IDC_STOP,button,1342242816
Control3=IDC_PAUSE,button,1342242816
Control4=IDC_PLAY,button,1342242816
Control5=IDC_NEXT,button,1342242816
Control6=IDC_PREV,button,1342242816
Control7=IDC_DISPLAY,static,1342177287
Control8=IDC_INFORMATION,button,1342242816
Control9=IDC_CLOSE,button,1342242816
Control10=IDC_SCROLLBAR,scrollbar,1342177280
Control11=IDC_DISPLAYBOX,static,1342177284

[DLG:IDD_DISPLAY]
Type=1
Class=Cinfodlg
ControlCount=9
Control1=IDC_STATIC,button,1342177287
Control2=IDC_STATIC,static,1342308352
Control3=IDC_STATIC,static,1342308352
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,static,1342308352
Control6=IDC_EDITNAME,edit,1476460672
Control7=IDC_EDITFRAMES,edit,1476460672
Control8=IDC_EDITRECT,edit,1476460672
Control9=IDC_EDITSPEED,edit,1476460672

[CLS:CSpeed]
Type=0
HeaderFile=Speed.h
ImplementationFile=Speed.cpp
BaseClass=CDialog
Filter=D
LastObject=CSpeed
VirtualFilter=dWC

[CLS:Cinfodlg]
Type=0
HeaderFile=infodlg.h
ImplementationFile=infodlg.cpp
BaseClass=CDialog
Filter=D
LastObject=Cinfodlg
VirtualFilter=dWC

[DLG:IDD_PLAYER_DLG]
Type=1
Class=?
ControlCount=12
Control1=IDC_OPEN,button,1342242817
Control2=IDC_STOP,button,1342242816
Control3=IDC_PAUSE,button,1342242816
Control4=IDC_PLAY,button,1342242816
Control5=IDC_NEXT,button,1342242816
Control6=IDC_LAST,button,1342242816
Control7=IDC_DISPLAY,static,1342177287
Control8=IDC_INFORMATION,button,1342242816
Control9=IDC_CLOSE,button,1342242816
Control10=IDC_SCROLLBAR,scrollbar,1342177280
Control11=IDC_DISPLAYBOX,static,1342177287
Control12=IDC_DISPLAYBOX2,static,1342177287

[CLS:dfg]
Type=0
HeaderFile=dfg.h
ImplementationFile=dfg.cpp
BaseClass=CDialog
Filter=D
LastObject=dfg

[DLG:IDD_DISPLAY1]
Type=1
Class=?
ControlCount=2
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816

