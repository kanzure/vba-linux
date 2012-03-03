// GBColorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "GBColorDlg.h"
#include "Reg.h"

extern int32 gbPaletteOption;
extern int   emulating;
extern int   systemCartridgeType;
extern u16   gbPalette[128];
extern u16   systemGbPalette[24];

static u16 defaultPalettes[][24] = {
	{
		0x7FFF, 0x56B5, 0x318C, 0x0000,  0x7FFF, 0x56B5, 0x318C, 0x0000,
	},
	{
		0x6200, 0x7E10, 0x7C10, 0x5000,  0x6200, 0x7E10, 0x7C10, 0x5000,
	},
	{
		0x4008, 0x4000, 0x2000, 0x2008,  0x4008, 0x4000, 0x2000, 0x2008,
	},
	{
		0x43F0, 0x03E0, 0x4200, 0x2200,  0x43F0, 0x03E0, 0x4200, 0x2200,
	},
	{
		0x43FF, 0x03FF, 0x221F, 0x021F,  0x43FF, 0x03FF, 0x221F, 0x021F,
	},
	{
		0x621F, 0x7E1F, 0x7C1F, 0x2010,  0x621F, 0x7E1F, 0x7C1F, 0x2010,
	},
	{
		0x621F, 0x401F, 0x001F, 0x2010,  0x621F, 0x401F, 0x001F, 0x2010,
	},
	{
		0x1314, 0x0E91, 0x0E0D, 0x2108,  0x1314, 0x0E91, 0x0E0D, 0x2108,
	}
};

/////////////////////////////////////////////////////////////////////////////
// GBColorDlg dialog

GBColorDlg::GBColorDlg(CWnd*pParent /*=NULL*/)
	: CDialog(GBColorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(GBColorDlg)
	which = -1;
	//}}AFX_DATA_INIT
	which = gbPaletteOption;
}

void GBColorDlg::DoDataExchange(CDataExchange*pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(GBColorDlg)
	DDX_Control(pDX, IDC_PREDEFINED, m_predefined);
	DDX_Radio(pDX, IDC_DEFAULT, which);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(GBColorDlg, CDialog)
//{{AFX_MSG_MAP(GBColorDlg)
ON_BN_CLICKED(IDC_DEFAULT, OnDefault)
ON_BN_CLICKED(IDC_RESET, OnReset)
ON_BN_CLICKED(IDC_USER1, OnUser1)
ON_BN_CLICKED(IDC_USER2, OnUser2)
ON_BN_CLICKED(ID_OK, OnOk)
ON_BN_CLICKED(ID_CANCEL, OnCancel)
ON_CBN_SELCHANGE(IDC_PREDEFINED, OnSelchangePredefined)
//}}AFX_MSG_MAP
ON_CONTROL_RANGE(BN_CLICKED, IDC_COLOR_BG0, IDC_COLOR_OB3, OnColorClicked)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// GBColorDlg message handlers

void GBColorDlg::OnDefault()
{
	setWhich(0);
}

void GBColorDlg::OnReset()
{
	int s = which * 8;
	colors[s++] = (0x1f) | (0x1f << 5) | (0x1f << 10);
	colors[s++] = (0x15) | (0x15 << 5) | (0x15 << 10);
	colors[s++] = (0x0c) | (0x0c << 5) | (0x0c << 10);
	colors[s++] = 0;

	colors[s++] = (0x1f) | (0x1f << 5) | (0x1f << 10);
	colors[s++] = (0x15) | (0x15 << 5) | (0x15 << 10);
	colors[s++] = (0x0c) | (0x0c << 5) | (0x0c << 10);
	colors[s]   = 0;
	setWhich(which);
}

void GBColorDlg::OnUser1()
{
	setWhich(1);
}

void GBColorDlg::OnUser2()
{
	setWhich(2);
}

void GBColorDlg::OnCancel()
{
	EndDialog(FALSE);
}

void GBColorDlg::OnOk()
{
	EndDialog(TRUE);
}

BOOL GBColorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	colorControls[0].SubclassDlgItem(IDC_COLOR_BG0, this);
	colorControls[1].SubclassDlgItem(IDC_COLOR_BG1, this);
	colorControls[2].SubclassDlgItem(IDC_COLOR_BG2, this);
	colorControls[3].SubclassDlgItem(IDC_COLOR_BG3, this);
	colorControls[4].SubclassDlgItem(IDC_COLOR_OB0, this);
	colorControls[5].SubclassDlgItem(IDC_COLOR_OB1, this);
	colorControls[6].SubclassDlgItem(IDC_COLOR_OB2, this);
	colorControls[7].SubclassDlgItem(IDC_COLOR_OB3, this);

	for (int i = 0; i < 24; i++)
	{
		colors[i] = systemGbPalette[i];
	}

	const char *names[] = {
		"Standard",
		"Blue Sea",
		"Dark Night",
		"Green Forest",
		"Hot Desert",
		"Pink Dreams",
		"Weird Colors",
		"Real Colors"
	};

	for (int j = 0; j < 8; j++)
	{
		int index = m_predefined.AddString(names[j]);
		m_predefined.SetItemData(index, j);
	}

	RECT cbSize;
	int  Height;

	m_predefined.GetClientRect(&cbSize);
	Height  = m_predefined.GetItemHeight(0);
	Height += m_predefined.GetItemHeight(0) * (10);

	// Note: The use of SM_CYEDGE assumes that we're using Windows '95
	// Now add on the height of the border of the edit box
	Height += GetSystemMetrics(SM_CYEDGE) * 2; // top & bottom edges

	// The height of the border of the drop-down box
	Height += GetSystemMetrics(SM_CYEDGE) * 2; // top & bottom edges

	// now set the size of the window
	m_predefined.SetWindowPos(NULL,
	                          0, 0,
	                          cbSize.right, Height,
	                          SWP_NOMOVE | SWP_NOZORDER);

	setWhich(which);

	CenterWindow();

	return TRUE; // return TRUE unless you set the focus to a control
	             // EXCEPTION: OCX Property Pages should return FALSE
}

void GBColorDlg::setWhich(int w)
{
	which = w;

	for (int i = 0; i < 8; i++)
	{
		colorControls[i].setColor(colors[which*8+i]);
	}
}

u16 *GBColorDlg::getColors()
{
	return colors;
}

void GBColorDlg::OnColorClicked(UINT id)
{
	id -= IDC_COLOR_BG0;

	u16 color = colors[which*8+id];

	COLORREF colorInit =
	    RGB((color & 0x1f) << 3, ((color >> 5) & 0x1f) << 3, ((color >> 10) & 0x1f) << 3);

	CColorDialog dlg(colorInit,
	                 CC_FULLOPEN | CC_ANYCOLOR, this);

	if (IDOK == dlg.DoModal())
	{
		COLORREF c = dlg.GetColor();

		colors[which*8+id] = (u16)((c >> 3) & 0x1f | ((c >> 11) & 0x1f) << 5 |
		                           ((c >> 19) & 0x1f) << 10);
		colorControls[id].setColor(colors[which*8+id]);
	}
}

int GBColorDlg::getWhich()
{
	return which;
}

void GBColorDlg::OnSelchangePredefined()
{
	int sel = m_predefined.GetCurSel();

	if (sel != -1)
	{
		int data = m_predefined.GetItemData(sel);
		for (int i = 0; i < 8; i++)
		{
			colorControls[i].setColor(defaultPalettes[data][i]);
			colors[which*8+i] = defaultPalettes[data][i];
		}
	}
}

