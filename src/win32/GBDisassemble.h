#if !defined(AFX_GBDISASSEMBLE_H__3EFD5B47_6DBF_4F63_8F91_A9511EC590EB__INCLUDED_)
#define AFX_GBDISASSEMBLE_H__3EFD5B47_6DBF_4F63_8F91_A9511EC590EB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// -*- C++ -*-
// GBDisassemble.h : header file
//

#include "IUpdate.h"
#include "ResizeDlg.h"

/////////////////////////////////////////////////////////////////////////////
// GBDisassemble dialog

class GBDisassemble : public ResizeDlg, IUpdateListener
{
	// Construction
public:
	//bool autoUpdate;	// If these two variables are defined here, the dialog will crash!
	//u16  address;		// There might be a bug within MFC.
	virtual void update();
	void refresh();
	GBDisassemble(CWnd*pParent = NULL);  // standard constructor

	// Dialog Data
	//{{AFX_DATA(GBDisassemble)
	enum { IDD = IDD_GB_DISASSEMBLE };
	CEdit    m_address;
	CListBox m_list;
	BOOL     m_c;
	BOOL     m_h;
	BOOL     m_n;
	BOOL     m_z;
	//}}AFX_DATA
	bool autoUpdate;
	int  count;
	u16  address;
	u16  lastAddress;

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(GBDisassemble)
protected:
	virtual void DoDataExchange(CDataExchange*pDX);   // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

	// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(GBDisassemble)
	afx_msg void OnClose();
	afx_msg void OnRefresh();
	afx_msg void OnNext();
	afx_msg void OnGo();
	afx_msg void OnGopc();
	afx_msg void OnAutoUpdate();
	virtual BOOL OnInitDialog();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar*pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GBDISASSEMBLE_H__3EFD5B47_6DBF_4F63_8F91_A9511EC590EB__INCLUDED_)
