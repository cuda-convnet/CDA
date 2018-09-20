// CUDPDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "CDA.h"
#include "CUDPDlg.h"
#include "afxdialogex.h"


// CUDPDlg 对话框

IMPLEMENT_DYNAMIC(CUDPDlg, CDialogEx)

CUDPDlg::CUDPDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_UDP, pParent)
{
	ConstructionExtra();
}

CUDPDlg::~CUDPDlg()
{
	m_Brush.DeleteObject();
}

// CUDPDlg 构造函数
void CUDPDlg::ConstructionExtra()
{
	m_Brush.CreateSolidBrush(RGB(255, 255, 255));
}

// CUDPDlg 初始化窗口形状
void CUDPDlg::InitWindowSharp()
{
	GetClientRect(&m_cWindowRect);
}

// CUDPDlg 窗口重绘
void CUDPDlg::RePaintWindow(CDC & dc)
{
	CDC MemDC;
	CBitmap MemBitmap;
	CRect MemRect;

	GetClientRect(&MemRect);

	MemDC.CreateCompatibleDC(&dc);
	MemBitmap.CreateCompatibleBitmap(&dc, MemRect.Width(), MemRect.Height());

	MemDC.SelectObject(&MemBitmap);
	MemDC.FillSolidRect(&MemRect, RGB(255, 255, 255));

	dc.BitBlt(m_cWindowRect.left, m_cWindowRect.top, m_cWindowRect.Width(), m_cWindowRect.Height(), &MemDC, MemRect.left, MemRect.top, SRCCOPY);

	MemBitmap.DeleteObject();
	MemDC.DeleteDC();
}

void CUDPDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CUDPDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CUDPDlg 消息处理程序


BOOL CUDPDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	InitWindowSharp();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CUDPDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	RePaintWindow(dc);
}


HBRUSH CUDPDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	if (nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkColor(RGB(255, 255, 255));
		return m_Brush;
	}

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}
