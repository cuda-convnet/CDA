// CRealCurveDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "CDA.h"
#include "CRealCurveDlg.h"
#include "afxdialogex.h"
#include "CThreadSafe.h"

//Macro Definition
#define WM_USER_TIMER_ONREFRESHRECVINFO	0

#define WM_USER_MSG_ONRECEIVEBUFFER		(WM_USER + 1)

// CRealCurveDlg 对话框

IMPLEMENT_DYNAMIC(CRealCurveDlg, CDialogEx)

CRealCurveDlg::CRealCurveDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REALCURVE_DIALOG, pParent)
{
	ConstructionExtra();
}

CRealCurveDlg::~CRealCurveDlg()
{
	m_Brush.DeleteObject();
}

// CRealCurveDlg 构造函数
void CRealCurveDlg::ConstructionExtra()
{
	m_hRecv = NULL;

	m_dwRecvCount = 0;
	memset(m_chRecvBuf, 0, sizeof(m_chRecvBuf));

	m_vecCurve1.clear();
	m_vecCurve2.clear();
	m_vecCurve3.clear();
	m_vecCurve4.clear();
	m_vecCurve5.clear();
	m_vecCurve6.clear();
	m_vecCurve7.clear();
	m_vecCurve8.clear();

	m_pCurveSeries1 = NULL;
	m_pCurveSeries2 = NULL;
	m_pCurveSeries3 = NULL;
	m_pCurveSeries4 = NULL;
	m_pCurveSeries5 = NULL;
	m_pCurveSeries6 = NULL;
	m_pCurveSeries7 = NULL;
	m_pCurveSeries8 = NULL;

	m_bShareInfo = false;
	InitializeCriticalSection(&m_csThreadSafe);

	m_Brush.CreateSolidBrush(RGB(255, 255, 255));
}

// CRealCurveDlg 初始化窗口形状
void CRealCurveDlg::InitWindowSharp()
{
	GetClientRect(&m_cWindowRect);
}

// CRealCurveDlg 初始化窗口控件
void CRealCurveDlg::InitControls()
{
	// 串口初始化
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

	// 辅助功能
	m_cCurve1Cbx.SetCheck(TRUE);
	m_cCurve2Cbx.SetCheck(TRUE);
	m_cCurve3Cbx.SetCheck(TRUE);
	m_cCurve4Cbx.SetCheck(TRUE);
	m_cCurve5Cbx.SetCheck(FALSE);
	m_cCurve6Cbx.SetCheck(FALSE);
	m_cCurve7Cbx.SetCheck(FALSE);
	m_cCurve8Cbx.SetCheck(FALSE);

	// 曲线初始化
	// 禁止控件自动刷新
	m_cChartCtrl.EnableRefresh(false);

	// 创建曲线坐标轴(底部、左侧)
	CChartStandardAxis* pAxisBot = m_cChartCtrl.CreateStandardAxis(CChartCtrl::BottomAxis);
	CChartStandardAxis* pAxisLef = m_cChartCtrl.CreateStandardAxis(CChartCtrl::LeftAxis);

	// 设置坐标轴范围
	pAxisBot->SetAutomatic(true);
	pAxisLef->SetAutomatic(true);

	// 设置坐标轴名称
	m_cChartCtrl.GetLeftAxis()->GetLabel()->SetText(_T("数值"));
	m_cChartCtrl.GetBottomAxis()->GetLabel()->SetText(_T("时间"));

	// 设置曲线标题名称
	m_cChartCtrl.GetTitle()->AddString(_T("串口曲线"));
	m_cChartCtrl.SetEdgeType(EDGE_SUNKEN);

	// 创建曲线
	m_pCurveSeries1 = m_cChartCtrl.CreateLineSerie();
	m_pCurveSeries2 = m_cChartCtrl.CreateLineSerie();
	m_pCurveSeries3 = m_cChartCtrl.CreateLineSerie();
	m_pCurveSeries4 = m_cChartCtrl.CreateLineSerie();
	m_pCurveSeries5 = m_cChartCtrl.CreateLineSerie();
	m_pCurveSeries6 = m_cChartCtrl.CreateLineSerie();
	m_pCurveSeries7 = m_cChartCtrl.CreateLineSerie();
	m_pCurveSeries8 = m_cChartCtrl.CreateLineSerie();

	m_pCurveSeries1->SetSeriesOrdering(poNoOrdering);
	m_pCurveSeries2->SetSeriesOrdering(poNoOrdering);
	m_pCurveSeries3->SetSeriesOrdering(poNoOrdering);
	m_pCurveSeries4->SetSeriesOrdering(poNoOrdering);
	m_pCurveSeries5->SetSeriesOrdering(poNoOrdering);
	m_pCurveSeries6->SetSeriesOrdering(poNoOrdering);
	m_pCurveSeries7->SetSeriesOrdering(poNoOrdering);
	m_pCurveSeries8->SetSeriesOrdering(poNoOrdering);

	// 开启控件自动刷新
	m_cChartCtrl.EnableRefresh(true);

}

// CRealCurveDlg 重绘窗口
void CRealCurveDlg::RePaintWindow(CDC & dc)
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

// CRealCurveDlg 检查串口设置
bool CRealCurveDlg::SerialPortConfigCheck()
{
	CString csSerialPortName;
	CString csSerialPortBaud;
	CString csSerialPortDataBits;
	CString csSerialPortStopBits;
	CString csSerialPortCheckBits;

	m_cSerialPortNameCbx.GetWindowTextW(csSerialPortName);
	m_cSerialPortBaudCbx.GetWindowTextW(csSerialPortBaud);
	m_cSerialPortDataBitCbx.GetWindowTextW(csSerialPortDataBits);
	m_cSerialPortStopBitCbx.GetWindowTextW(csSerialPortStopBits);
	m_cSerialPortCheckBitCbx.GetWindowTextW(csSerialPortCheckBits);

	if (_T("") == csSerialPortName)
	{
		MessageBoxW(_T("串口名未设置!"), _T("警告"), MB_OK | MB_ICONWARNING);
		return false;
	}

	if (_T("") == csSerialPortBaud)
	{
		MessageBoxW(_T("串口波特率未设置!"), _T("警告"), MB_OK | MB_ICONWARNING);
		return false;
	}

	if (_T("") == csSerialPortDataBits)
	{
		MessageBoxW(_T("串口数据位未设置!"), _T("警告"), MB_OK | MB_ICONWARNING);
		return false;
	}

	if (_T("") == csSerialPortStopBits)
	{
		MessageBoxW(_T("串口停止位未设置!"), _T("警告"), MB_OK | MB_ICONWARNING);
		return false;
	}

	if (_T("") == csSerialPortCheckBits)
	{
		MessageBoxW(_T("串口校验位未设置!"), _T("警告"), MB_OK | MB_ICONWARNING);
		return false;
	}

	return true;
}

// CRealCurveDlg 填充初始化结构体
void CRealCurveDlg::SerialPortSetValue(S_SERIALPORT_PROPERTY * pSerialPortInfo)
{
	CString csSerialPortName;
	CString csSerialPortBaud;
	CString csSerialPortDataBits;
	CString csSerialPortStopBits;
	CString csSerialPortCheckBits;

	m_cSerialPortNameCbx.GetWindowTextW(csSerialPortName);
	m_cSerialPortBaudCbx.GetWindowTextW(csSerialPortBaud);
	m_cSerialPortDataBitCbx.GetWindowTextW(csSerialPortDataBits);
	m_cSerialPortStopBitCbx.GetWindowTextW(csSerialPortStopBits);
	m_cSerialPortCheckBitCbx.GetWindowTextW(csSerialPortCheckBits);

	USES_CONVERSION;
	strcpy_s(pSerialPortInfo->chPort, T2A(csSerialPortName));
	pSerialPortInfo->dwBaudRate = _ttol(csSerialPortBaud);
	pSerialPortInfo->byDataBits = _ttoi(csSerialPortDataBits);

	if (1 == _ttoi(csSerialPortStopBits))
	{
		pSerialPortInfo->byStopBits = 0;
	}
	else if (2 == _ttoi(csSerialPortStopBits))
	{
		pSerialPortInfo->byStopBits = 2;
	}

	if (!strcmp("无校验", T2A(csSerialPortCheckBits)))
	{
		pSerialPortInfo->byCheckBits = 0;
	}
	else if (!strcmp("奇校验", T2A(csSerialPortCheckBits)))
	{
		pSerialPortInfo->byCheckBits = 1;
	}
	else if (!strcmp("偶校验", T2A(csSerialPortCheckBits)))
	{
		pSerialPortInfo->byCheckBits = 2;
	}

}

// CRealCurveDlg 设置打开串口窗口显示
void CRealCurveDlg::SerialPortSetOpenStatus()
{
	CString csPort;
	CString csBaud;

	// 打开串口按钮显示
	m_cSerialPortOpenBtn.SetWindowTextW(_T("关闭串口"));

	// 禁止按钮选择
	m_cSerialPortRefreshBtn.EnableWindow(FALSE);

	m_cSerialPortNameCbx.EnableWindow(FALSE);
	m_cSerialPortBaudCbx.EnableWindow(FALSE);
	m_cSerialPortDataBitCbx.EnableWindow(FALSE);
	m_cSerialPortStopBitCbx.EnableWindow(FALSE);
	m_cSerialPortCheckBitCbx.EnableWindow(FALSE);

	// 串口信息提示
	m_cSerialPortNameCbx.GetLBText(m_cSerialPortNameCbx.GetCurSel(), csPort);
	m_cSerialPortBaudCbx.GetLBText(m_cSerialPortBaudCbx.GetCurSel(), csBaud);
	m_cSerialInfoEdt.SetWindowTextW(_T("串口已打开!"));
	m_cSerialInfoPortEdt.SetWindowTextW(csPort);
	m_cSerialInfoBaudEdt.SetWindowTextW(csBaud);

}

// CRealCurveDlg 设置关闭串口窗口显示
void CRealCurveDlg::SerialPortSetCloseStatus()
{
	CString csPort;
	CString csBaud;

	// 打开串口按钮显示
	m_cSerialPortOpenBtn.SetWindowTextW(_T("打开串口"));

	// 禁止按钮选择
	m_cSerialPortRefreshBtn.EnableWindow(TRUE);

	m_cSerialPortNameCbx.EnableWindow(TRUE);
	m_cSerialPortBaudCbx.EnableWindow(TRUE);
	m_cSerialPortDataBitCbx.EnableWindow(TRUE);
	m_cSerialPortStopBitCbx.EnableWindow(TRUE);
	m_cSerialPortCheckBitCbx.EnableWindow(TRUE);

	// 串口信息提示
	m_cSerialPortNameCbx.GetLBText(m_cSerialPortNameCbx.GetCurSel(), csPort);
	m_cSerialPortBaudCbx.GetLBText(m_cSerialPortBaudCbx.GetCurSel(), csBaud);
	m_cSerialInfoEdt.SetWindowTextW(_T("串口已关闭!"));
	m_cSerialInfoPortEdt.SetWindowTextW(csPort);
	m_cSerialInfoBaudEdt.SetWindowTextW(csBaud);

}

// CRealCurveDlg 开始串口接收线程
bool CRealCurveDlg::SerialPortStartRecvThread()
{
	unsigned int uThreadID;

	m_hRecv = (HANDLE)::_beginthreadex(NULL, 0, (_beginthreadex_proc_type)OnReceiveBuffer, this, 0, &uThreadID);
	if (!m_hRecv)
	{
		return false;
	}

	return true;
}

// CRealCurveDlg 关闭串口接收线程
void CRealCurveDlg::SerialPortCloseRecvThread()
{
	if (NULL != m_hRecv)
	{
		m_bShareInfo = false;
		::WaitForSingleObject(m_hRecv, INFINITE);
		::CloseHandle(m_hRecv);
		m_hRecv = NULL;
	}

}

// CRealCurveDlg 接收定时器响应
void CRealCurveDlg::SerialPortRecvOnTimer()
{
	// 显示串口接收数据
	CString csRecvData;

	csRecvData.Format(_T("已接收:%ld"), m_dwRecvCount);
	m_cSerialInfoAllRecvStic.SetWindowTextW(csRecvData);
	m_cSerialInfoAllRecvStic.Invalidate(FALSE);
	m_cSerialInfoAllRecvStic.UpdateWindow();

}

// CRealCurveDlg 接收数据线程
unsigned int CALLBACK CRealCurveDlg::OnReceiveBuffer(LPVOID lpParameters)
{
	CRealCurveDlg* pUser = reinterpret_cast<CRealCurveDlg*>(lpParameters);

	while (true)
	{
		EnterCriticalSection(&pUser->m_cSerialPort.m_csCOMSync);
		if (!pUser->m_cSerialPort.m_bOpen)
		{
			LeaveCriticalSection(&pUser->m_cSerialPort.m_csCOMSync);
			break;
		}
		LeaveCriticalSection(&pUser->m_cSerialPort.m_csCOMSync);

		while (true)
		{
			EnterCriticalSection(&pUser->m_csThreadSafe);
			if (!pUser->m_bShareInfo)
			{
				pUser->m_bShareInfo = true;
				LeaveCriticalSection(&pUser->m_csThreadSafe);
				break;
			}
			LeaveCriticalSection(&pUser->m_csThreadSafe);
			Sleep(10);
		}

		if (pUser->m_cSerialPort.CCSerialPortBaseGetRecv())
		{
			memset(pUser->m_chRecvBuf, 0, sizeof(pUser->m_chRecvBuf));
			pUser->m_cSerialPort.CCSerialPortBaseGetRecvBuf(pUser->m_chRecvBuf, sizeof(pUser->m_chRecvBuf));
			pUser->m_cSerialPort.CCSerialPortBaseSetRecv(false);
			::PostMessageA(pUser->m_hWnd, WM_USER_MSG_ONRECEIVEBUFFER, (WPARAM)((LPVOID)(&pUser->m_chRecvBuf)), (LPARAM)0);
		}
		else
		{
			pUser->m_bShareInfo = false;
		}

	}

	return 0;
}

// CRealCurveDlg 接收数据消息响应
LRESULT CRealCurveDlg::OnRecvSerialPortBufferMsg(WPARAM wParam, LPARAM lParam)
{
	CThreadSafe ThreadSafe(&m_csThreadSafe);

	USES_CONVERSION;

	for (int i = 0; i < SERIALPORT_COMM_OUTPUT_BUFFER_SIZE - 21; ++i)
	{
		if (m_chRecvBuf[i] == 0xFF && m_chRecvBuf[i + 1] == 0x00 && m_chRecvBuf[i + 20] == 0xAA && m_chRecvBuf[i + 21] == 0x55)
		{
			INT16 uCurveVar[8] = { 0 };
		}

	}

	m_bShareInfo = false;
	return 0;
}

void CRealCurveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM_CURVE, m_cChartCtrl);
	DDX_Control(pDX, IDC_COMBO_CURVE_PORT, m_cSerialPortNameCbx);
	DDX_Control(pDX, IDC_COMBO_CURVE_BOAD, m_cSerialPortBaudCbx);
	DDX_Control(pDX, IDC_COMBO_CURVE_DATABIT, m_cSerialPortDataBitCbx);
	DDX_Control(pDX, IDC_COMBO_CURVE_STOPBIT, m_cSerialPortStopBitCbx);
	DDX_Control(pDX, IDC_COMBO_CURVE_CHECKBIT, m_cSerialPortCheckBitCbx);
	DDX_Control(pDX, IDC_BUTTON_CURVE_REFRESH, m_cSerialPortRefreshBtn);
	DDX_Control(pDX, IDC_BUTTON_CURVE_OPEN, m_cSerialPortOpenBtn);
	DDX_Control(pDX, IDC_CHECK_CURVE1, m_cCurve1Cbx);
	DDX_Control(pDX, IDC_CHECK_CURVE2, m_cCurve2Cbx);
	DDX_Control(pDX, IDC_CHECK_CURVE3, m_cCurve3Cbx);
	DDX_Control(pDX, IDC_CHECK_CURVE4, m_cCurve4Cbx);
	DDX_Control(pDX, IDC_CHECK_CURVE5, m_cCurve5Cbx);
	DDX_Control(pDX, IDC_CHECK_CURVE6, m_cCurve6Cbx);
	DDX_Control(pDX, IDC_CHECK_CURVE7, m_cCurve7Cbx);
	DDX_Control(pDX, IDC_CHECK_CURVE8, m_cCurve8Cbx);
	DDX_Control(pDX, IDC_STATIC_CURVE1_VALUE, m_cCurve1Stic);
	DDX_Control(pDX, IDC_STATIC_CURVE2_VALUE, m_cCurve2Stic);
	DDX_Control(pDX, IDC_STATIC_CURVE3_VALUE, m_cCurve3Stic);
	DDX_Control(pDX, IDC_STATIC_CURVE4_VALUE3, m_cCurve4Stic);
	DDX_Control(pDX, IDC_STATIC_CURVE5_VALUE, m_cCurve5Stic);
	DDX_Control(pDX, IDC_STATIC_CURVE6_VALUE, m_cCurve6Stic);
	DDX_Control(pDX, IDC_STATIC_CURVE7_VALUE, m_cCurve7Stic);
	DDX_Control(pDX, IDC_STATIC_CURVE8_VALUE, m_cCurve8Stic);
	DDX_Control(pDX, IDC_BUTTON_CURVE_CLEAR, m_cCurveClearBtn);
	DDX_Control(pDX, IDC_EDIT_CURVE_INFO_MESSAGE, m_cSerialInfoEdt);
	DDX_Control(pDX, IDC_EDIT_CURVE_INFO_PORT, m_cSerialInfoPortEdt);
	DDX_Control(pDX, IDC_EDIT_CURVE_INFO_BAUD, m_cSerialInfoBaudEdt);
	DDX_Control(pDX, IDC_STATIC_CURVE_RECEIVE_ALL, m_cSerialInfoAllRecvStic);
	DDX_Control(pDX, IDC_BUTTON_CURVE_EXPORT_DATA, m_cCurveExportBtn);
}


BEGIN_MESSAGE_MAP(CRealCurveDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_CURVE_REFRESH, &CRealCurveDlg::OnBnClickedButtonCurveRefresh)
	ON_BN_CLICKED(IDC_BUTTON_CURVE_OPEN, &CRealCurveDlg::OnBnClickedButtonCurveOpen)
	ON_BN_CLICKED(IDC_BUTTON_CURVE_CLEAR, &CRealCurveDlg::OnBnClickedButtonCurveClear)
	ON_BN_CLICKED(IDC_BUTTON_CURVE_EXPORT_DATA, &CRealCurveDlg::OnBnClickedButtonCurveExportData)
	ON_MESSAGE(WM_USER_MSG_ONRECEIVEBUFFER, &CRealCurveDlg::OnRecvSerialPortBufferMsg)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CRealCurveDlg 消息处理程序


BOOL CRealCurveDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	InitWindowSharp();
	InitControls();

	return TRUE;  // return TRUE unless you set the focus to a control
}


void CRealCurveDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	RePaintWindow(dc);
}


HBRUSH CRealCurveDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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


void CRealCurveDlg::OnClose()
{
	::KillTimer(this->GetSafeHwnd(), WM_USER_TIMER_ONREFRESHRECVINFO);
	DeleteCriticalSection(&m_csThreadSafe);
	CDialogEx::OnClose();
}


void CRealCurveDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case WM_USER_TIMER_ONREFRESHRECVINFO:
		SerialPortRecvOnTimer();
		break;
	default:
		break;
	}

	CDialogEx::OnTimer(nIDEvent);
}

// CRealCurveDlg 刷新串口
void CRealCurveDlg::OnBnClickedButtonCurveRefresh()
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
}

// CRealCurveDlg 打开串口
void CRealCurveDlg::OnBnClickedButtonCurveOpen()
{
	// 串口打开状态
	if (!m_cSerialPort.CCSerialPortBaseGetStatus())
	{
		// 打开串口
		if (!SerialPortConfigCheck())
		{
			return;
		}

		bool bRet = false;
		S_SERIALPORT_PROPERTY sSerialPortProperty = { 0 };

		// 填充串口参数
		SerialPortSetValue(&sSerialPortProperty);

		// 打开串口函数
		bRet = m_cSerialPort.CCSerialPortBaseOpenPort(sSerialPortProperty);
		if (!bRet)
		{
			// 打开失败
			MessageBoxW(_T("串口打开失败"), _T("警告"), MB_OK | MB_ICONWARNING);
			return;
		}

		// 开启串口收发线程
		SerialPortStartRecvThread();

		// 窗口显示函数
		SerialPortSetOpenStatus();

		// 开启接收计数定时器
		::SetTimer(this->GetSafeHwnd(), WM_USER_TIMER_ONREFRESHRECVINFO, 100, NULL);
	}
	else
	{
		// 关闭接收计数定时器
		::KillTimer(this->GetSafeHwnd(), WM_USER_TIMER_ONREFRESHRECVINFO);

		// 关闭串口
		m_cSerialPort.CCSerialPortBaseClosePort();

		// 关闭串口收发线程
		SerialPortCloseRecvThread();

		// 窗口显示函数
		SerialPortSetCloseStatus();
	}

}

// CRealCurveDlg 清除曲线
void CRealCurveDlg::OnBnClickedButtonCurveClear()
{
	
}

// CRealCurveDlg 导出数据
void CRealCurveDlg::OnBnClickedButtonCurveExportData()
{
	
}
