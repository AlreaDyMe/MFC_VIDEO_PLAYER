// VideoPlayer.h : main header file for the VIDEOPLAYER application
//

#if !defined(AFX_VIDEOPLAYER_H__B35A40C7_7DB7_11D3_860F_F6E44D6C9F3C__INCLUDED_)
#define AFX_VIDEOPLAYER_H__B35A40C7_7DB7_11D3_860F_F6E44D6C9F3C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CVideoPlayerApp:
// See VideoPlayer.cpp for the implementation of this class
//

class CVideoPlayerApp : public CWinApp
{
public:
	CVideoPlayerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVideoPlayerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CVideoPlayerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIDEOPLAYER_H__B35A40C7_7DB7_11D3_860F_F6E44D6C9F3C__INCLUDED_)
