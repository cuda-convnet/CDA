// CSerialPortDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "CDA.h"
#include "CSerialPortDlg.h"
#include "afxdialogex.h"


// CSerialPortDlg 对话框

IMPLEMENT_DYNAMIC(CSerialPortDlg, CDialogEx)

CSerialPortDlg::CSerialPortDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SERIALPORT_DIALOG, pParent)
{
	ConstructionExtra();
}

CSerialPortDlg::~CSerialPortDlg()
{
	m_Brush.DeleteObject();
}

// CSerialPortDlg 构造函数
void CSerialPortDlg::ConstructionExtra()
{
	m_Brush.CreateSolidBrush(RGB(255, 255, 255));
}

// CSerialPortDlg 初始化窗口形状
void CSerialPortDlg::InitWindowSharp()
{
	GetClientRect(&m_cWindowRect);
}

// CSerialPortDlg 初始化控件
void CSerialPortDlg::InitControls()
{
	m_cSerialPort.EnumSerialPort();		// 枚举串口

	// 添加串口
	m_cSerialPortNameCbx.ResetContent();	// 清空Cbx
	for (auto iter = m_cSerialPort.m_mapEnumCOM.begin(); iter != m_cSerialPort.m_mapEnumCOM.end(); ++iter)
	{
		USES_CONVERSION;
		m_cSerialPortNameCbx.InsertString(iter->first, A2T((iter->second).c_str()));
	}

	m_cSerialPortNameCbx.SetCurSel(0);

	// 添加波特率
	m_cSerialPortBaudCbx.ResetContent();	// 清空Cbx
	m_cSerialPortBaudCbx.InsertString(0, _T("9600"));
	m_cSerialPortBaudCbx.InsertString(1, _T("14400"));
	m_cSerialPortBaudCbx.InsertString(2, _T("19200"));
	m_cSerialPortBaudCbx.InsertString(3, _T("38400"));
	m_cSerialPortBaudCbx.InsertString(4, _T("57600"));
	m_cSerialPortBaudCbx.InsertString(5, _T("115200"));
	m_cSerialPortBaudCbx.InsertString(6, _T("128000"));
	m_cSerialPortBaudCbx.InsertString(7, _T("256000"));
	m_cSerialPortBaudCbx.SetCurSel(5);

	// 添加数据位
	m_cSerialPortDataBitCbx.ResetContent();	// 清空Cbx
	m_cSerialPortDataBitCbx.InsertString(0, _T("5"));
	m_cSerialPortDataBitCbx.InsertString(1, _T("6"));
	m_cSerialPortDataBitCbx.InsertString(2, _T("7"));
	m_cSerialPortDataBitCbx.InsertString(3, _T("8"));
	m_cSerialPortDataBitCbx.SetCurSel(3);

	// 添加停止位
	m_cSerialPortStopBitCbx.ResetContent();	// 清空Cbx
	m_cSerialPortStopBitCbx.InsertString(0, _T("1"));
	m_cSerialPortStopBitCbx.InsertString(1, _T("2"));
	m_cSerialPortStopBitCbx.SetCurSel(0);

	// 添加校验位
	m_cSerialPortCheckBitCbx.ResetContent();	// 清空Cbx
	m_cSerialPortCheckBitCbx.InsertString(0, _T("无校验"));
	m_cSerialPortCheckBitCbx.InsertString(1, _T("奇校验"));
	m_cSerialPortCheckBitCbx.InsertString(2, _T("偶校验"));
	m_cSerialPortCheckBitCbx.SetCurSel(0);

}

// CSerialPortDlg 重绘窗口
void CSerialPortDlg::RePaintWindow(CDC & dc)
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

void CSerialPortDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_SERIAL_PORT, m_cSerialPortNameCbx);
	DDX_Control(pDX, IDC_COMBO_SERIAL_BOAD, m_cSerialPortBaudCbx);
	DDX_Control(pDX, IDC_COMBO_SERIAL_DATABIT, m_cSerialPortDataBitCbx);
	DDX_Control(pDX, IDC_COMBO_SERIAL_STOPBIT, m_cSerialPortStopBitCbx);
	DDX_Control(pDX, IDC_COMBO_SERIAL_CHECKBIT, m_cSerialPortCheckBitCbx);
	DDX_Control(pDX, IDC_BUTTON_SERIAL_REFRESH, m_cSerialPortRefreshBtn);
	DDX_Control(pDX, IDC_BUTTON_SERIAL_OPEN, m_cSerialPortOpenBtn);
}


BEGIN_MESSAGE_MAP(CSerialPortDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CSerialPortDlg 消息处理程序


BOOL CSerialPortDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	InitWindowSharp();
	InitControls();

	return TRUE;  // return TRUE unless you set the focus to a control
}


void CSerialPortDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	RePaintWindow(dc);
}


HBRUSH CSerialPortDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
