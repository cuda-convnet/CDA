// CUDPDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "CDA.h"
#include "CUDPDlg.h"
#include "afxdialogex.h"

//Macro Definition
#define WM_USER_MSG_ONRECEIVEBUFFER		(WM_USER + 1)
#define WM_USER_MSG_ONSERVERDISCONNECT	(WM_USER + 2)

// Variable Definition
CUDPDlg* g_pUDPDlg = nullptr;

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
	m_pSocket = NULL;

	m_hListen = NULL;
	m_bConnect = false; 
	m_bExit = false;
	m_bShareInfo = false;
	g_pUDPDlg = this;

	m_uSendCount = 0;
	m_uRecvCount = 0;
	m_Brush.CreateSolidBrush(RGB(255, 255, 255));
}

// CUDPDlg 初始化窗口形状
void CUDPDlg::InitWindowSharp()
{
	GetClientRect(&m_cWindowRect);
}

// CUDPDlg 初始化控件
void CUDPDlg::InitControls()
{
	BYTE byHostIP[4] = { 0 };

	// 设置本地/远程IP地址
	CCSocketBase::SetLocalIP(NULL, NULL);	// 本机远程IP地址
	CCSocketBase::SetLocalPort(0);			// 本机远程端口
	CCSocketBase::GetLocalIPAddr();			// 获取本机IP地址
	BreakIpAddress(CCSocketBase::GetLocalIP(), byHostIP);	// 拆分本机IP地址
	m_cHostIPAdc.SetAddress(byHostIP[0], byHostIP[1], byHostIP[2], byHostIP[3]);	// 设置本机IP地址
	m_cRemoteIPAdc.SetAddress(byHostIP[0], byHostIP[1], byHostIP[2], byHostIP[3]);	// 设置远程IP地址

	// 设置本地/远程端口号
	m_cHostPortEdt.SetWindowTextW(_T("6000"));	// 默认本机端口号
	m_cRemotePortEdt.SetWindowTextW(_T("6001"));	// 默认远程端口号

	// 关闭发送按钮使能
	m_cSendBtn.EnableWindow(FALSE);

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

// CUDPDlg 初始化Socket
void CUDPDlg::InitSocket()
{
	CCSocketBase::CCSocketBaseLibInit();	// 初始化Socket库
}

// CUDPDlg 释放Socket
void CUDPDlg::ReleaseSocket()
{
	CCSocketBase::CCSocketBaseLibRelease();	// 释放Socket库
}

// CUDPDlg 开启侦听接收线程
void CUDPDlg::StartListenThread()
{
	unsigned int uThreadID = 0;
	m_hListen = (HANDLE)::_beginthreadex(NULL, 0, (_beginthreadex_proc_type)OnHandleListenThread, this, 0, &uThreadID);
}

// CUDPDlg UDP接收数据缓冲线程
unsigned int CALLBACK CUDPDlg::OnHandleListenThread(LPVOID lpParameters)
{
	while (true)
	{
		char chRecvBuf[SOB_TCP_RECV_BUFFER] = { 0 };
		UINT uRecvCount = 0;
		char chIP[SOB_IP_LENGTH] = { 0 };
		USHORT sPort = 0;
		int nRet = 0;

		g_pUDPDlg->m_csThreadSafe.Enter();
		if (g_pUDPDlg->m_bExit)
		{
			g_pUDPDlg->m_csThreadSafe.Leave();
			break;
		}
		g_pUDPDlg->m_csThreadSafe.Leave();

		while (true)
		{
			g_pUDPDlg->m_csThreadSafe.Enter();
			if (!g_pUDPDlg->m_bShareInfo)
			{
				g_pUDPDlg->m_bShareInfo = true;
				g_pUDPDlg->m_csThreadSafe.Leave();
				break;
			}
			g_pUDPDlg->m_csThreadSafe.Leave();
			Sleep(10);
		}

		nRet = g_pUDPDlg->m_pSocket->CCSocketBaseUDPRecvBuffer(chRecvBuf, SOB_TCP_RECV_BUFFER, uRecvCount, chIP, sPort);
		if (nRet == SOB_RET_OK)
		{
			if (strcmp("", chRecvBuf))
			{
				g_pUDPDlg->m_uRecvCount = uRecvCount;
				memset(g_pUDPDlg->m_chRecvBuf, 0, sizeof(g_pUDPDlg->m_chRecvBuf));
				memcpy_s(g_pUDPDlg->m_chRecvBuf, sizeof(g_pUDPDlg->m_chRecvBuf), chRecvBuf, uRecvCount);
				::PostMessageA(g_pUDPDlg->GetSafeHwnd(), WM_USER_MSG_ONRECEIVEBUFFER, (WPARAM)0, (LPARAM)0);
			}
			else
			{
				g_pUDPDlg->m_bShareInfo = false;
			}
		}
		else if (nRet == SOB_RET_CLOSE)
		{
			g_pUDPDlg->m_bShareInfo = false;

			// 关闭Socket通信
			g_pUDPDlg->m_pSocket->CCSocketBaseDisConnect();

			// 发送服务器关闭消息
			::PostMessageA(g_pUDPDlg->GetSafeHwnd(), WM_USER_MSG_ONSERVERDISCONNECT, (WPARAM)0, (LPARAM)0);

			break;
		}
		else
		{
			g_pUDPDlg->m_bShareInfo = false;
		}

		Sleep(10);
	}

	return 0;
}

// CUDPDlg 拆分IP地址
void CUDPDlg::BreakIpAddress(const char * pArr, BYTE byArr[4])
{
	char* p = NULL;
	char* pNext = NULL;
	char* pStr = NULL;
	int nSize = 0;
	vector<string> vecStr;

	nSize = strlen(pArr) + 1;
	pStr = new char[nSize];
	memset(pStr, 0, nSize);
	strcpy_s(pStr, nSize, pArr);

	vecStr.clear();

	p = strtok_s(pStr, ".", &pNext);
	while (p != NULL)
	{
		vecStr.push_back(p);
		p = strtok_s(NULL, ".", &pNext);
	}

	for (auto i = 0; i < 4; ++i)
	{
		byArr[i] = atoi(vecStr[i].c_str());
	}

	delete[] pStr;
	pStr = NULL;
}

// CUDPDlg 检查IP地址
bool CUDPDlg::CheckIpAddress(const char * pArr)
{
	char* p = NULL;
	char* pNext = NULL;
	char* pStr = NULL;
	int nSize = 0;
	vector<string> vecStr;
	bool bRet = true;

	nSize = strlen(pArr) + 1;
	pStr = new char[nSize];
	memset(pStr, 0, nSize);
	strcpy_s(pStr, nSize, pArr);

	vecStr.clear();

	p = strtok_s(pStr, ".", &pNext);
	while (p != NULL)
	{
		vecStr.push_back(p);
		p = strtok_s(NULL, ".", &pNext);
	}

	for (auto iter = vecStr.begin(); iter != vecStr.end(); ++iter)
	{
		if (!strcmp("", iter->c_str()) || atoi(iter->c_str()) < 0 || atoi(iter->c_str()) > 255)
		{
			bRet = false;
			break;
		}

	}

	delete[] pStr;
	pStr = NULL;

	return bRet;
}

// CUDPDlg 拆分空格字符串
void CUDPDlg::BreakSpace(const unsigned char * pStr, vector<string>& vecStr)
{
	unsigned char* pString = const_cast<unsigned char*>(pStr);
	char* pNewString = nullptr;
	int nSize = 0;
	char* pTemp = nullptr;
	char* pArr = nullptr;

	nSize = strlen((char*)pString) + 1;
	pNewString = new char[nSize];
	memset(pNewString, 0, nSize);
	memcpy_s(pNewString, nSize, pString, strlen((char*)pString));

	vecStr.clear();

	pTemp = strtok_s(pNewString, " ", &pArr);
	while (pTemp)
	{
		vecStr.push_back(pTemp);
		pTemp = strtok_s(NULL, " ", &pArr);
	}

	delete[] pNewString;
	pNewString = nullptr;
}

// CUDPDlg 16进制转换
void CUDPDlg::Convert2Hex(vector<string>& vecStr, unsigned char * pStr, int nSize)
{
	char* pArr = nullptr;
	int nDelta = 0;

	pArr = new char[nSize];
	memset(pArr, 0, nSize);

	for (auto iter = vecStr.begin(); iter != vecStr.end(); ++iter)
	{
		char* pTemp = const_cast<char*>((*iter).c_str());
		for (size_t i = 0; (i <= (*iter).size()) && ((i + 1) <= (*iter).size()); i += 2, pTemp += 2)
		{
			if (((*pTemp >= '0' && *pTemp <= '9') || (*pTemp >= 'A' && *pTemp <= 'F') || (*pTemp >= 'a' && *pTemp <= 'f'))
				&& ((*(pTemp + 1) >= '0' && *(pTemp + 1) <= '9') || (*(pTemp + 1) >= 'A' && *(pTemp + 1) <= 'F') || (*(pTemp + 1) >= 'a' && *(pTemp + 1) <= 'f')))
			{
				char ch = 0;
				char cl = 0;

				if (*pTemp >= '0' && *pTemp <= '9')
				{
					ch = *pTemp - 48;
				}
				else if (*pTemp >= 'A' && *pTemp <= 'F')
				{
					ch = *pTemp - 55;
				}
				else
				{
					ch = *pTemp - 87;
				}

				if (*(pTemp + 1) >= '0' && *(pTemp + 1) <= '9')
				{
					cl = *(pTemp + 1) - 48;
				}
				else if (*(pTemp + 1) >= 'A' && *(pTemp + 1) <= 'F')
				{
					cl = *(pTemp + 1) - 55;
				}
				else
				{
					cl = *(pTemp + 1) - 87;
				}

				*(pArr + nDelta) = ch * 16 + cl;
				nDelta++;
				if (nDelta >= nSize)
				{
					break;
				}

			}

		}

	}

	m_uSendCount = nDelta;
	memcpy_s(pStr, nSize, pArr, nSize);

	delete[] pArr;
	pArr = nullptr;
}

// CUDPDlg 接收UDP Socket发送消息
LRESULT CUDPDlg::OnRecvSocketBufferMsg(WPARAM wParam, LPARAM lParam)
{
	CThreadSafe ThreadSafe(m_csThreadSafe.GetCriticalSection());

	CString csHostIP = _T("");
	CString csHostPort = _T("");
	char chServerInfo[MAX_PATH] = { 0 };
	int nLen = 0;

	SYSTEMTIME syTime = { 0 };

	USES_CONVERSION;

	GetLocalTime(&syTime);
	GetDlgItemTextW(IDC_IPADDRESS_NET_UDP_IP, csHostIP);
	GetDlgItemTextW(IDC_EDIT_NET_UDP_PORT, csHostPort);
	sprintf_s(chServerInfo, "[%s:%s] %02d:%02d:%02d", T2A(csHostIP), T2A(csHostPort), syTime.wHour, syTime.wMinute, syTime.wSecond);

	// 16进制显示数据
	if (TRUE == m_cRecvHexCbx.GetCheck())
	{
		CString csRecvBuff = _T("");
		for (int i = 0; i < m_uRecvCount; ++i)
		{
			CString csTemp = _T("");
			csTemp.Format(_T("%02X "), m_chRecvBuf[i]);
			csRecvBuff += csTemp;
		}

		memset(m_chRecvBuf, 0, sizeof(m_chRecvBuf));
		strcpy_s((char*)m_chRecvBuf, sizeof(m_chRecvBuf), T2A(csRecvBuff));
	}

	// 接收区文本显示
	nLen = m_cRecvEdt.GetWindowTextLengthW();
	if (nLen >= 30000)
	{
		m_cRecvEdt.SetWindowTextW(_T(""));
		nLen = -1;
	}

	m_cRecvEdt.SetSel(nLen, nLen);
	m_cRecvEdt.ReplaceSel(A2T(chServerInfo));
	m_cRecvEdt.ReplaceSel(A2T("\r\n"));
	m_cRecvEdt.ReplaceSel(A2T((char*)m_chRecvBuf));
	m_cRecvEdt.ReplaceSel(A2T("\r\n"));

	m_bShareInfo = false;

	return 0;
}

// CUDPDlg 接收UDP Socket断开
LRESULT CUDPDlg::OnUDPDisConnectMsg(WPARAM wParam, LPARAM lParam)
{
	int nLen = 0;

	// 接收区文本提示
	nLen = m_cRecvEdt.GetWindowTextLengthW();
	if (nLen >= 30000)
	{
		m_cRecvEdt.SetWindowTextW(_T(""));
		nLen = -1;
	}

	m_cRecvEdt.SetSel(nLen, nLen);
	m_cRecvEdt.ReplaceSel(_T("远程连接已关闭\r\n"));
	m_cRecvEdt.ReplaceSel(_T("关闭端口成功\r\n"));

	::CloseHandle(m_hListen);
	m_hListen = NULL;

	m_bExit = false;

	m_cHostPortEdt.SetReadOnly(FALSE);
	m_cSendBtn.EnableWindow(FALSE);

	m_bConnect = false;
	m_cOpenBtn.SetWindowTextW(_T("打开"));

	return 0;
}

void CUDPDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS_NET_UDP_IP, m_cHostIPAdc);
	DDX_Control(pDX, IDC_EDIT_NET_UDP_PORT, m_cHostPortEdt);
	DDX_Control(pDX, IDC_BUTTON_NET_UDP_OPEN, m_cOpenBtn);
	DDX_Control(pDX, IDC_CHECK_NET_UDP_RECV_HEX, m_cRecvHexCbx);
	DDX_Control(pDX, IDC_CHECK_NET_UDP_SEND_HEX, m_cSendHexCbx);
	DDX_Control(pDX, IDC_BUTTON_NET_UDP_RECV_CLEAR, m_cRecvClearBtn);
	DDX_Control(pDX, IDC_BUTTON_NET_UDP_SEND_CLEAR, m_cSendClearBtn);
	DDX_Control(pDX, IDC_EDIT_NET_UDP_RECV, m_cRecvEdt);
	DDX_Control(pDX, IDC_EDIT_NET_UDP_SEND, m_cSendEdt);
	DDX_Control(pDX, IDC_IPADDRESS_NET_UDP_DEST_IP, m_cRemoteIPAdc);
	DDX_Control(pDX, IDC_EDIT_NET_NET_UDP_DEST_PORT, m_cRemotePortEdt);
	DDX_Control(pDX, IDC_BUTTON_NET_UDP_SEND, m_cSendBtn);
}


BEGIN_MESSAGE_MAP(CUDPDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_NET_UDP_OPEN, &CUDPDlg::OnBnClickedButtonNetUdpOpen)
	ON_BN_CLICKED(IDC_BUTTON_NET_UDP_RECV_CLEAR, &CUDPDlg::OnBnClickedButtonNetUdpRecvClear)
	ON_BN_CLICKED(IDC_BUTTON_NET_UDP_SEND_CLEAR, &CUDPDlg::OnBnClickedButtonNetUdpSendClear)
	ON_BN_CLICKED(IDC_BUTTON_NET_UDP_SEND, &CUDPDlg::OnBnClickedButtonNetUdpSend)
	ON_MESSAGE(WM_USER_MSG_ONRECEIVEBUFFER, &CUDPDlg::OnRecvSocketBufferMsg)
	ON_MESSAGE(WM_USER_MSG_ONSERVERDISCONNECT, &CUDPDlg::OnUDPDisConnectMsg)
END_MESSAGE_MAP()


// CUDPDlg 消息处理程序


BOOL CUDPDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	InitWindowSharp();
	InitSocket();
	InitControls();

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


void CUDPDlg::OnClose()
{
	SAFE_DELETE(m_pSocket);
	ReleaseSocket();
	CDialogEx::OnClose();
}

// CUDPDlg 打开
void CUDPDlg::OnBnClickedButtonNetUdpOpen()
{
	bool bRet = false;
	CString csHostIP = _T("");
	CString csHostPort = _T("");
	int nHostPort = 0;

	USES_CONVERSION;
	
	if (!m_bConnect)
	{
		// CSocketBase实例是否存在
		if (m_pSocket == NULL)
		{
			m_pSocket = new CCSocketBase();
		}

		// 获取本地连接IP地址
		GetDlgItemTextW(IDC_IPADDRESS_NET_UDP_IP, csHostIP);

		// 本地IP地址是否为空
		if (!CheckIpAddress(T2A(csHostIP)))
		{
			MessageBox(_T("本地IP地址不合法!"), _T("警告"), MB_OK | MB_ICONWARNING);
			return;
		}

		// 获取远程连接端口号
		m_cHostPortEdt.GetWindowTextW(csHostPort);

		nHostPort = atoi(T2A(csHostPort));
		if (nHostPort < 0 || nHostPort > 65535)
		{
			MessageBox(_T("本地端口号不合法!"), _T("警告"), MB_OK | MB_ICONWARNING);
			return;
		}

		int nLen = 0;

		// 接收区文本提示
		nLen = m_cRecvEdt.GetWindowTextLengthW();
		if (nLen >= 30000)
		{
			m_cRecvEdt.SetWindowTextW(_T(""));
			nLen = -1;
		}

		bRet = m_pSocket->CCSocketBaseUDPBindOnPort(T2A(csHostIP), nHostPort);
		if (!bRet)
		{
			m_cRecvEdt.SetSel(nLen, nLen);
			m_cRecvEdt.ReplaceSel(_T("打开端口失败，端口已被占用\r\n"));
			return;
		}

		m_cRecvEdt.SetSel(nLen, nLen);
		m_cRecvEdt.ReplaceSel(_T("打开端口成功\r\n"));

		StartListenThread();

		m_cHostPortEdt.SetReadOnly(TRUE);
		m_cSendBtn.EnableWindow(TRUE);

		m_bConnect = true;
		m_cOpenBtn.SetWindowTextW(_T("关闭"));
	}
	else
	{
		m_pSocket->CCSocketBaseDisConnect();

		int nLen = 0;

		// 接收区文本提示
		nLen = m_cRecvEdt.GetWindowTextLengthW();
		if (nLen >= 30000)
		{
			m_cRecvEdt.SetWindowTextW(_T(""));
			nLen = -1;
		}

		m_cRecvEdt.SetSel(nLen, nLen);
		m_cRecvEdt.ReplaceSel(_T("关闭端口成功\r\n"));

		m_bExit = true;
		::WaitForSingleObject(m_hListen, INFINITE);
		::CloseHandle(m_hListen);
		m_hListen = NULL;

		m_bExit = false;

		m_cHostPortEdt.SetReadOnly(FALSE);
		m_cSendBtn.EnableWindow(FALSE);

		m_bConnect = false;
		m_cOpenBtn.SetWindowTextW(_T("打开"));
	}

}

// CUDPDlg 清空接收区
void CUDPDlg::OnBnClickedButtonNetUdpRecvClear()
{
	m_cRecvEdt.SetWindowTextW(_T(""));
}

// CUDPDlg 清空发送区
void CUDPDlg::OnBnClickedButtonNetUdpSendClear()
{
	m_cSendEdt.SetWindowTextW(_T(""));
}

// CUDPDlg 发送
void CUDPDlg::OnBnClickedButtonNetUdpSend()
{
	unsigned char chSendBuf[SOB_TCP_SEND_BUFFER] = { 0 };
	int nRet = 0;

	USES_CONVERSION;

	// 获取准备发送的内容
	CString csSendBuf;

	m_cSendEdt.GetWindowTextW(csSendBuf);
	memset(chSendBuf, 0, sizeof(chSendBuf));
	strcpy_s((char*)chSendBuf, sizeof(chSendBuf), T2A(csSendBuf));

	// 16进制发送数据
	if (TRUE == m_cSendHexCbx.GetCheck())
	{
		vector<string> vecSendBuf;

		BreakSpace(chSendBuf, vecSendBuf);
		memset(chSendBuf, 0, sizeof(chSendBuf));
		Convert2Hex(vecSendBuf, chSendBuf, sizeof(chSendBuf));
	}

	// 消息区显示发送的内容
	char chServerInfo[MAX_PATH] = { 0 };
	int nLen = 0;
	SYSTEMTIME syTime = { 0 };

	CString csRemoteIP = _T("");
	CString csRemotePort = _T("");
	int nRemotePort = 0;

	// 获取本地连接IP地址
	GetDlgItemTextW(IDC_IPADDRESS_NET_UDP_DEST_IP, csRemoteIP);

	// 远程IP地址是否为空
	if (!CheckIpAddress(T2A(csRemoteIP)))
	{
		MessageBox(_T("远程IP地址不合法!"), _T("警告"), MB_OK | MB_ICONWARNING);
		return;
	}

	// 获取远程连接端口号
	m_cRemotePortEdt.GetWindowTextW(csRemotePort);

	nRemotePort = atoi(T2A(csRemotePort));
	if (nRemotePort < 0 || nRemotePort > 65535)
	{
		MessageBox(_T("远程端口号不合法!"), _T("警告"), MB_OK | MB_ICONWARNING);
		return;
	}

	GetLocalTime(&syTime);
	sprintf_s(chServerInfo, "[%s:%d] %02d:%02d:%02d", T2A(csRemoteIP), nRemotePort, syTime.wHour, syTime.wMinute, syTime.wSecond);

	// 接收区文本显示
	nLen = m_cRecvEdt.GetWindowTextLengthW();
	if (nLen >= 30000)
	{
		m_cRecvEdt.SetWindowTextW(_T(""));
		nLen = -1;
	}

	m_cRecvEdt.SetSel(nLen, nLen);
	m_cRecvEdt.ReplaceSel(A2T(chServerInfo));
	m_cRecvEdt.ReplaceSel(A2T("\r\n"));
	m_cRecvEdt.ReplaceSel(A2T((char*)chSendBuf));
	m_cRecvEdt.ReplaceSel(A2T("\r\n"));

	// 16进制发送数据
	if (TRUE == m_cSendHexCbx.GetCheck())
	{
		nRet = m_pSocket->CCSocketBaseUDPSendBuffer(T2A(csRemoteIP), nRemotePort, (char*)chSendBuf, m_uSendCount);
		if (nRet == SOB_RET_OK)
		{
			// 发送消息成功响应...
		}
	}
	else
	{
		nRet = m_pSocket->CCSocketBaseUDPSendBuffer(T2A(csRemoteIP), nRemotePort, (char*)chSendBuf, SOB_TCP_SEND_BUFFER);
		if (nRet == SOB_RET_OK)
		{
			// 发送消息成功响应...
		}
	}
	
}
