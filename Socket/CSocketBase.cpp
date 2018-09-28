#include "CSocketBase.h"

#include <Windows.h>
#include <Ws2tcpip.h>
#include <process.h>

#pragma warning(disable:4996)

char CCSocketBase::m_pcLocalIP[SOB_IP_LENGTH] = { 0 };
USHORT CCSocketBase::m_sLocalPort = 0;

// CCSocketBase 构造函数
CCSocketBase::CCSocketBase()
{
	m_socket = NULL;
	m_SocketWriteEvent = NULL;
	m_SocketReadEvent = NULL;

	m_bIsConnected = false;
	m_sMaxCount = SOB_DEFAULT_MAX_CLIENT;

	memset(m_pcRemoteIP, 0, SOB_IP_LENGTH);
	m_sRemotePort = 0;

	memset(m_pcHostIP, 0, SOB_IP_LENGTH);
	m_sHostPort = 0;
}

// CCSocketBase 析构函数
CCSocketBase::~CCSocketBase()
{
	if (m_socket)
	{
		closesocket(m_socket);
		m_socket = NULL;
	}

	if (m_SocketWriteEvent)
	{
		WSACloseEvent(m_SocketWriteEvent);
		m_SocketWriteEvent = NULL;
	}

	if (m_SocketReadEvent)
	{
		WSACloseEvent(m_SocketReadEvent);
		m_SocketReadEvent = NULL;
	}

}

// CCSocketBase 初始化Socket
void CCSocketBase::CCSocketBaseLibInit()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int nErr;

	wVersionRequested = MAKEWORD(2, 2);

	nErr = WSAStartup(wVersionRequested, &wsaData);	// 初始化SOCKET环境
	if (nErr != 0)
	{
		return;
	}
}

// CCSocketBase 释放Socket
void CCSocketBase::CCSocketBaseLibRelease()
{
	WSACleanup();	// 清理SOCKET环境
}

// CCSocketBase 创建TCP套接字
SOCKET CCSocketBase::CreateTCPSocket()
{
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	// TCP Socket

	// 如果创建出错，我们置其为空
	if (s == INVALID_SOCKET)
	{
		s = NULL;
		m_nLastWSAError = WSAGetLastError();
	}

	// 创建异步事件
	if (m_SocketWriteEvent == NULL)
	{
		m_SocketWriteEvent = WSACreateEvent();
	}

	if (m_SocketReadEvent == NULL)
	{
		m_SocketReadEvent = WSACreateEvent();
	}

	// 设置SOCKET属性
	const char chOpt = 1;
	int nRet = setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &chOpt, sizeof(char));

	return s;
}

// CCSocketBase 创建UDP套接字
SOCKET CCSocketBase::CreateUDPSocket()
{
	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);	// UDP Socket

	// 如果创建出错，我们置其为空
	if (s == INVALID_SOCKET)
	{
		s = NULL;
		m_nLastWSAError = WSAGetLastError();
	}

	// 创建异步事件
	if (m_SocketWriteEvent == NULL)
	{
		m_SocketWriteEvent = WSACreateEvent();
	}

	if (m_SocketReadEvent == NULL)
	{
		m_SocketReadEvent = WSACreateEvent();
	}

	return s;
}

// CCSocketBase 设置接收数据超时时长
void CCSocketBase::CCSocketBaseSetRecvTimeOut(UINT uiMSec)
{
	UINT uiMseTimeOut = uiMSec;
	setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&uiMseTimeOut, sizeof(uiMseTimeOut));
}

// CCSocketBase 设置发送数据超时时长
void CCSocketBase::CCSocketBaseSetSendTimeOut(UINT uiMSec)
{
	UINT uiMseTimeOut = uiMSec;
	setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&uiMseTimeOut, sizeof(uiMseTimeOut));
}

// CCSocketBase 设置接收数据数组长度
void CCSocketBase::CCSocketBaseSetRecvBufferSize(UINT uiByte)
{
	UINT uiBufferSize = uiByte;
	setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char*)&uiBufferSize, sizeof(uiBufferSize));
}

// CCSocketBase 设置发送数据数组长度
void CCSocketBase::CCSocketBaseSetSendBufferSize(UINT uiByte)
{
	UINT uiBufferSize = uiByte;
	setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (char*)&uiBufferSize, sizeof(uiBufferSize));
}

// CCSocketBase 获取Socket句柄
SOCKET CCSocketBase::CCSocketBaseGetRawSocket() const
{
	return m_socket;
}

// CCSocketBase 绑定Socket套接字
bool CCSocketBase::CCSocketBaseAttachRawSocket(SOCKET s, bool bIsConnected)
{
	// 托管SOCKET
	m_socket = s;

	// 对于托管的SOCKET，默认其连接
	m_bIsConnected = bIsConnected;

	// 创建异步事件
	if (m_SocketWriteEvent == NULL)
	{
		m_SocketWriteEvent = WSACreateEvent();
	}

	if (m_SocketReadEvent == NULL)
	{
		m_SocketReadEvent = WSACreateEvent();
	}

	// 获取远端信息
	if (bIsConnected)
	{
		struct sockaddr_in addrPeer;
		int nLen = sizeof(addrPeer);
		if (getpeername(s, (struct sockaddr*)&addrPeer, &nLen) == SOCKET_ERROR)
		{
			return false;
		}

		InetNtopA(AF_INET, &addrPeer.sin_addr, m_pcRemoteIP, SOB_IP_LENGTH);
		InetNtopW(AF_INET, &addrPeer.sin_addr, m_pwcRemoteIP, SOB_IP_LENGTH);

		m_sRemotePort = ntohs(addrPeer.sin_port);
	}

	return true;
}

// CCSocketBase 分离服务端端口
void CCSocketBase::CCSocketBaseDettachRawSocket()
{
	WSAEventSelect(m_socket, m_SocketWriteEvent, 0);
	WSAEventSelect(m_socket, m_SocketReadEvent, 0);

	WSACloseEvent(m_SocketWriteEvent);
	WSACloseEvent(m_SocketReadEvent);
	m_SocketWriteEvent = NULL;
	m_SocketReadEvent = NULL;

	m_bIsConnected = false;
	m_socket = NULL;
}

// CCSocketBase 获取远程端IP地址(ASCII)
const char * CCSocketBase::CCSocketBaseGetRemoteIP() const
{
	return m_pcRemoteIP;
}

// CCSocketBase 获取远程端IP地址(Unicode)
const wchar_t * CCSocketBase::CCSocketBaseGetRemoteIPW() const
{
	return m_pwcRemoteIP;
}

// CCSocketBase 获取远端IP地址(ULONG)
ULONG CCSocketBase::CCSocketBaseGetRemoteIPUL() const
{
	return inet_addr(m_pcRemoteIP);
}

// CCSocketBase 获取远端端口号
USHORT CCSocketBase::CCSocketBaseGetRemotePort() const
{
	return m_sRemotePort;
}

// CCSocketBase 获取连接状态
bool CCSocketBase::CCSocketBaseIsConnected() const
{
	return m_bIsConnected;
}

// CCSocketBase 删除SocketBase类
void CCSocketBase::CCSocketBaseDestory()
{
	this->~CCSocketBase();
}

// CCSocketBase 绑定服务端端口
bool CCSocketBase::CCSocketBaseBindOnPort(USHORT uPort)
{
	// 如果socket无效则新建，为了可以重复调用
	if (m_socket == NULL)
	{
		m_socket = CreateTCPSocket();
	}

	// 开启本地监听
	SOCKADDR_IN addrLocal;
	memset(&addrLocal, 0, sizeof(addrLocal));

	addrLocal.sin_family = AF_INET;
	addrLocal.sin_addr.s_addr = htonl(INADDR_ANY);
	addrLocal.sin_port = htons(uPort);

	m_sHostPort = uPort;

	// 绑定
	int nRet = bind(m_socket, (PSOCKADDR)&addrLocal, sizeof(addrLocal));

	// 唯一的原因是端口被占用
	if (nRet == SOCKET_ERROR)
	{
		m_nLastWSAError = WSAGetLastError();
		return false;
	}

	return true;
}

// CCSocketBase 监听服务端端口
bool CCSocketBase::CCSocketBaseListen()
{
	// 监听
	int nRet = listen(m_socket, 5);

	// 唯一的原因是端口被占用
	if (nRet == SOCKET_ERROR)
	{
		m_nLastWSAError = WSAGetLastError();
		return false;
	}

	return true;
}

// CCSocketBase 接收客户端连接请求
bool CCSocketBase::CCSocketBaseAccept(HANDLE_ACCEPT_THREAD pThreadFunc, HANDLE_ACCEPT_CALLBACK pCallback, DWORD dwUser, BOOL * pExitFlag, USHORT nLoopTimeOutSec)
{
	// 注册连接事件
	WSAResetEvent(m_SocketReadEvent);           // 清除之前尚未处理的事件
	WSAEventSelect(m_socket, m_SocketReadEvent, FD_ACCEPT | FD_CLOSE);

	// 等待连接
	while ((pExitFlag == NULL ? TRUE : !(*pExitFlag)))
	{
		DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketReadEvent, FALSE, nLoopTimeOutSec * 1000, FALSE);

		if (dwRet == WSA_WAIT_EVENT_0)
		{
			// 如果网络事件发生
			WSANETWORKEVENTS wsaEvents;
			memset(&wsaEvents, 0, sizeof(wsaEvents));

			WSAResetEvent(m_SocketReadEvent);
			WSAEnumNetworkEvents(m_socket, m_SocketReadEvent, &wsaEvents);

			if ((wsaEvents.lNetworkEvents & FD_ACCEPT) &&
				(wsaEvents.iErrorCode[FD_ACCEPT_BIT] == 0))
			{
				// 记录远程地址
				SOCKADDR_IN addrRemote;
				memset(&addrRemote, 0, sizeof(addrRemote));
				int nAddrSize = sizeof(addrRemote);

				SOCKET sockRemote = accept(m_socket, (PSOCKADDR)&addrRemote, &nAddrSize);

				// 无效连接
				if (sockRemote == INVALID_SOCKET)
				{
					m_nLastWSAError = WSAGetLastError();

					// 本线程持续监听
					continue;
				}

				// 如果定义线程函数则开启新线程
				if (pThreadFunc)
				{
					// 开启新处理线程
					HANDLE hThread;
					unsigned unThreadID;
					S_CLIENTINFO sClientInfo = { 0 };

					memset(&sClientInfo.SocketAddr, 0, sizeof(sClientInfo.SocketAddr));
					sClientInfo.Socket = sockRemote;
					sClientInfo.SocketAddr = addrRemote;

					hThread = (HANDLE)_beginthreadex(NULL, 0, pThreadFunc, (void*)(&sClientInfo), 0, &unThreadID);

					// 已经不需要此HANDLE
					CloseHandle(hThread);
				}
				else if (pCallback)		// 如果定义回调则进行回调
				{
					pCallback(&addrRemote, sockRemote, dwUser);
				}
			}
		}
		else
		{
			//等待超时，重新开始
			continue;
		}
	}

	return true;
}

// CCSocketBase 发送缓冲数据(缓冲应该比待接受量要大一点才安全)<发送全部数据>
int CCSocketBase::CCSocketBaseSendOnce(SOCKET Socket, char * pSendBuffer, USHORT nTimeOutSec)
{
	bool bIsTimeOut = false;

	// 发送前注册事件
	WSAResetEvent(m_SocketWriteEvent);
	WSAEventSelect(Socket, m_SocketWriteEvent, FD_WRITE | FD_CLOSE);

	// 尝试发送
	int nRet = send(Socket, pSendBuffer, (int)strlen(pSendBuffer), NULL);

	if (nRet == SOCKET_ERROR)
	{
		m_nLastWSAError = WSAGetLastError();

		// 遭遇阻塞
		if (m_nLastWSAError == WSAEWOULDBLOCK)
		{
			DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketWriteEvent, FALSE, nTimeOutSec * 1000, FALSE);

			// 如果网络事件发生
			WSANETWORKEVENTS wsaEvents;
			memset(&wsaEvents, 0, sizeof(wsaEvents));

			if (dwRet == WSA_WAIT_EVENT_0)
			{
				WSAResetEvent(m_SocketWriteEvent);
				WSAEnumNetworkEvents(Socket, m_SocketWriteEvent, &wsaEvents);

				// 如果发送可以进行并且没有错误发生
				if ((wsaEvents.lNetworkEvents & FD_WRITE) &&
					(wsaEvents.iErrorCode[FD_WRITE_BIT] == 0))
				{
					// 再次发送文本
					nRet = (int)send(Socket, pSendBuffer, (int)strlen(pSendBuffer), NULL);

					if (nRet > 0)
					{
						// 如果发送字节大于0，表明发送成功
						return SOB_RET_OK;
					}
				}
			}
			else
			{
				// 超时
				bIsTimeOut = true;
			}
		}
		else
		{

		}
	}
	else
	{
		// 第一次便发送成功
		return SOB_RET_OK;
	}

	// 如果超时
	if (bIsTimeOut)
	{
		return SOB_RET_TIMEOUT;
	}

	// 如果上述发送失败
	m_nLastWSAError = WSAGetLastError();

	return SOB_RET_FAIL;
}

// CCSocketBase 发送缓冲数据(缓冲应该比待接受量要大一点才安全)<发送一定数据>
int CCSocketBase::CCSocketBaseSendBuffer(SOCKET Socket, char * pSendBuffer, UINT uiBufferSize, USHORT nTimeOutSec)
{
	bool bIsTimeOut = false;

	// 发送前注册事件
	WSAResetEvent(m_SocketWriteEvent);
	WSAEventSelect(Socket, m_SocketWriteEvent, FD_WRITE | FD_CLOSE);

	// 发送量计数
	int nSent = 0;

	// 总发送次数，为发送次数定一个限额
	int nSendTimes = 0;
	int nSendLimitTimes = (int)((float)uiBufferSize / 500 + 1.5);		// 假定当前每次发送肯定不少于500字节
	UINT uiLeftBuffer = uiBufferSize;									// 未发送完的缓存大小

	// 发送游标
	char* pcSentPos = pSendBuffer;

	// 直到所有的缓冲都发送完毕
	while (nSent < (int)uiBufferSize)
	{
		// 检查发送次数是否超限
		if (nSendTimes > nSendLimitTimes)
		{
			break;
		}

		int nRet = send(Socket, pcSentPos, uiLeftBuffer, NULL);

		if (nRet == SOCKET_ERROR)
		{
			m_nLastWSAError = WSAGetLastError();

			// 遭遇阻塞
			if (m_nLastWSAError == WSAEWOULDBLOCK)
			{
				DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketWriteEvent, FALSE, nTimeOutSec * 1000, FALSE);

				// 如果网络事件发生
				WSANETWORKEVENTS wsaEvents;
				memset(&wsaEvents, 0, sizeof(wsaEvents));

				if (dwRet == WSA_WAIT_EVENT_0)
				{
					WSAResetEvent(m_SocketWriteEvent);
					WSAEnumNetworkEvents(Socket, m_SocketWriteEvent, &wsaEvents);

					// 如果发送可以进行并且没有错误发生
					if ((wsaEvents.lNetworkEvents & FD_WRITE) &&
						(wsaEvents.iErrorCode[FD_WRITE_BIT] == 0))
					{
						// 再次发送文本
						nRet = send(Socket, pcSentPos, uiLeftBuffer, NULL);

						if (nRet > 0)
						{
							// 如果发送字节大于0，表明发送成功
							nSendTimes++;

							nSent += nRet;
							uiLeftBuffer -= nRet;
							pcSentPos += nRet;
						}
						else
						{
							m_nLastWSAError = WSAGetLastError();

							// 如果接收到事件发送时也阻塞了，则等待一个周期后重试
							if (m_nLastWSAError == WSAEWOULDBLOCK)
							{
								Sleep(1);
							}
							else
							{
								// 对于其他错误，直接退出
								break;
							}
						}
					}
				}
				else
				{
					// 超时
					bIsTimeOut = true;
					break;
				}
			}
			else
			{
				// 遇到阻塞之外的错误，直接退出
				break;
			}
		}
		else
		{
			// 发送成功，累加发送量，更新游标
			nSendTimes++;

			nSent += nRet;
			uiLeftBuffer -= nRet;
			pcSentPos += nRet;
		}
	}

	// 如果发送完成
	if (nSent == uiBufferSize)
	{
		return SOB_RET_OK;
	}

	// 如果超时
	if (bIsTimeOut)
	{
		return SOB_RET_TIMEOUT;
	}

	// 没能成功返回
	m_nLastWSAError = WSAGetLastError();

	return SOB_RET_FAIL;
}

// CCSocketBase 接收缓冲数据(缓冲应该比待接受量要大一点才安全)<接收全部数据>
int CCSocketBase::CCSocketBaseRecvOnce(SOCKET Socket, char * pRecvBuffer, UINT uiBufferSize, UINT & uiRecv, USHORT nTimeOutSec)
{
	bool bIsTimeOut = false;

	// 发送前注册事件
	WSAResetEvent(m_SocketReadEvent);
	WSAEventSelect(Socket, m_SocketReadEvent, FD_READ | FD_CLOSE);

	// 尝试接收
	int nRet = recv(Socket, pRecvBuffer, uiBufferSize, NULL);

	if (nRet == SOCKET_ERROR)
	{
		m_nLastWSAError = WSAGetLastError();

		// 遭遇阻塞
		if (m_nLastWSAError == WSAEWOULDBLOCK)
		{
			DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketReadEvent, FALSE, nTimeOutSec * 1000, FALSE);

			// 如果网络事件发生
			WSANETWORKEVENTS wsaEvents;
			memset(&wsaEvents, 0, sizeof(wsaEvents));

			if (dwRet == WSA_WAIT_EVENT_0)
			{
				WSAResetEvent(m_SocketReadEvent);
				int nEnum = WSAEnumNetworkEvents(Socket, m_SocketReadEvent, &wsaEvents);

				// 如果接受可以进行并且没有错误发生
				if ((wsaEvents.lNetworkEvents & FD_READ) &&
					(wsaEvents.iErrorCode[FD_READ_BIT] == 0))
				{
					// 再次接受文本
					nRet = recv(Socket, pRecvBuffer, uiBufferSize, NULL);

					if (nRet > 0)
					{
						// 如果发送字节大于0，表明发送成功
						uiRecv = nRet;
						return SOB_RET_OK;
					}
				}
			}
			else
			{
				bIsTimeOut = true;
			}
		}
		else
		{
		}
	}
	else
	{
		// 第一次便接收成功
		uiRecv = nRet;
		return SOB_RET_OK;
	}

	// 如果超时
	if (bIsTimeOut)
	{
		return SOB_RET_TIMEOUT;
	}

	// 如果上述接收失败
	m_nLastWSAError = WSAGetLastError();

	return SOB_RET_FAIL;
}

// CCSocketBase 接收缓冲数据(缓冲应该比待接受量要大一点才安全)<接收一定数据>
int CCSocketBase::CCSocketBaseRecvBuffer(SOCKET Socket, char * pRecvBuffer, UINT uiBufferSize, UINT uiRecvSize, USHORT nTimeOutSec)
{
	bool bIsTimeOut = false;

	// 接收前注册事件
	WSAResetEvent(m_SocketReadEvent);
	WSAEventSelect(Socket, m_SocketReadEvent, FD_READ | FD_CLOSE);

	// 接收量计数
	int nReceived = 0;

	// 总接收次数，为接收次数定一个限额
	int nRecvTimes = 0;
	int nRecvLimitTimes = (int)((float)uiRecvSize / 500 + 1.5);      // 假定当前每次接收肯定不少于500字节

	// 接收游标
	char* pcRecvPos = pRecvBuffer;

	// 直到接收到足够指定量的缓冲
	while (nReceived < (int)uiRecvSize)
	{
		// 检查接收次数是否超限
		if (nRecvTimes > nRecvLimitTimes)
		{
			break;
		}

		int nRet = recv(Socket, pcRecvPos, uiBufferSize, NULL);

		if (nRet == SOCKET_ERROR)
		{
			m_nLastWSAError = WSAGetLastError();

			// 遭遇阻塞
			if (m_nLastWSAError == WSAEWOULDBLOCK)
			{
				DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketReadEvent, FALSE, nTimeOutSec * 1000, FALSE);

				// 如果网络事件发生
				WSANETWORKEVENTS wsaEvents;
				memset(&wsaEvents, 0, sizeof(wsaEvents));

				if (dwRet == WSA_WAIT_EVENT_0)
				{
					WSAResetEvent(m_SocketReadEvent);
					WSAEnumNetworkEvents(Socket, m_SocketReadEvent, &wsaEvents);

					// 如果接收可以进行并且没有错误发生
					if ((wsaEvents.lNetworkEvents & FD_READ) &&
						(wsaEvents.iErrorCode[FD_READ_BIT] == 0))
					{
						// 再次接收
						nRet = recv(Socket, pcRecvPos, uiBufferSize, NULL);

						if (nRet > 0)
						{
							// 如果接收字节大于0，表明发送成功
							nRecvTimes++;

							nReceived += nRet;
							uiBufferSize -= nRet;
							pcRecvPos += nRet;
						}
						else
						{
							m_nLastWSAError = WSAGetLastError();

							// 如果接收到事件发送时也阻塞了，则等待一个周期后重试
							if (m_nLastWSAError == WSAEWOULDBLOCK)
							{
								Sleep(1);
							}
							else
							{
								// 对于其他错误，直接退出
								break;
							}
						}
					}
				}
				else
				{
					// 超时
					bIsTimeOut = true;
					break;
				}
			}
			else
			{
				// 遇到阻塞之外的错误，直接退出
				break;
			}
		}
		else if (nRet == 0)		// 对方主动关闭连接
		{
			break;
		}
		else
		{
			// 接收成功，累加接受量，更新游标
			nRecvTimes++;

			nReceived += nRet;
			uiBufferSize -= nRet;
			pcRecvPos += nRet;
		}
	}

	// 如果接收完成
	if (nReceived == uiRecvSize)
	{
		return SOB_RET_OK;
	}

	// 如果超时
	if (bIsTimeOut)
	{
		return SOB_RET_TIMEOUT;
	}

	// 没能成功返回
	m_nLastWSAError = WSAGetLastError();

	return SOB_RET_FAIL;
}

// CCSocketBase 发送服务器连接请求(无参数调用表示重连)
bool CCSocketBase::CCSocketBaseConnect(const char * pcRemoteIP, USHORT sPort, USHORT nTimeOutSec)
{
	// 如果socket无效则新建，为了可以重复调用
	if (m_socket == NULL)
	{
		m_socket = CreateTCPSocket();
	}

	// 设定远程IP
	SOCKADDR_IN addrRemote;
	memset(&addrRemote, 0, sizeof(addrRemote));

	// 如果需要，更新参数，否则表示用当前IP重连
	if (pcRemoteIP != NULL && sPort != 0)
	{
		memset(m_pcRemoteIP, 0, SOB_IP_LENGTH);
		strcpy(m_pcRemoteIP, pcRemoteIP);

		m_sRemotePort = sPort;
	}

	addrRemote.sin_family = AF_INET;
	addrRemote.sin_addr.S_un.S_addr = inet_addr(m_pcRemoteIP);
	addrRemote.sin_port = htons(m_sRemotePort);

	// 注册连接事件
	WSAResetEvent(m_SocketWriteEvent);           // 清除之前尚未处理的事件
	WSAEventSelect(m_socket, m_SocketWriteEvent, FD_CONNECT | FD_CLOSE);

	// 进行连接
	int nRet = connect(m_socket, (SOCKADDR*)&addrRemote, sizeof(addrRemote));

	if (nRet == SOCKET_ERROR)
	{
		m_nLastWSAError = WSAGetLastError();

		// 如果发生阻塞
		if (m_nLastWSAError == WSAEWOULDBLOCK)
		{
			DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketWriteEvent, FALSE, nTimeOutSec * 1000, FALSE);

			// 如果网络事件发生
			WSANETWORKEVENTS wsaEvents;
			memset(&wsaEvents, 0, sizeof(wsaEvents));

			if (dwRet == WSA_WAIT_EVENT_0)
			{
				WSAResetEvent(m_SocketWriteEvent);
				WSAEnumNetworkEvents(m_socket, m_SocketWriteEvent, &wsaEvents);

				// 如果连接接受并且没有错误发生
				if ((wsaEvents.lNetworkEvents & FD_CONNECT) &&
					(wsaEvents.iErrorCode[FD_CONNECT_BIT] == 0))
				{
					// 实际的调试经历告诉我，这里如果不停顿，会报出令人费解的10022错误
					// 对于同一socket，两次connect间隔太短看来是不行的,至于这里的Magic Number的50，这是实践出的结果，Sleep(1)的一个时间片似乎不够
					Sleep(50);
					nRet = connect(m_socket, (SOCKADDR*)&addrRemote, sizeof(addrRemote));

					if (nRet == SOCKET_ERROR)
					{
						m_nLastWSAError = WSAGetLastError();

						// 此种情况实际上已经连接
						if (m_nLastWSAError == WSAEISCONN)
						{
							m_bIsConnected = true;
						}
					}
					else    // 阻塞后连接成功
					{
						m_bIsConnected = true;
					}
				}
			}
		}
		else
		{
			m_bIsConnected = false;
		}

	}
	else
	{
		// 第一次尝试便连接成功
		m_bIsConnected = true;
	}

	// 如果连接失败
	if (!m_bIsConnected)
	{
		closesocket(m_socket);
		m_socket = NULL;
	}

	// 返回接续结果
	return m_bIsConnected;
}

// CCSocketBase 尝试重新连接服务器
bool CCSocketBase::CCSocketBaseReConnect()
{
	CCSocketBaseDisConnect();

	Sleep(100);

	return CCSocketBaseConnect();
}

// CCSocketBase 断开与服务器的连接
void CCSocketBase::CCSocketBaseDisConnect()
{
	if (m_socket == NULL)
	{
		return;
	}

	// 如果尚在连接状态则关闭连接
	if (m_bIsConnected)
	{
		shutdown(m_socket, SD_BOTH);

		m_bIsConnected = false;
	}

	closesocket(m_socket);
	m_socket = NULL;
}

// CCSocketBase 发送缓冲数据(缓冲应该比待接受量要大一点才安全)<发送全部数据>
int CCSocketBase::CCSocketBaseSendOnce(char * pSendBuffer, USHORT nTimeOutSec)
{
	bool bIsTimeOut = false;

	// 检查连接状态
	if (!m_bIsConnected)
	{
		return SOB_RET_FAIL;
	}

	// 发送前注册事件
	WSAResetEvent(m_SocketWriteEvent);
	WSAEventSelect(m_socket, m_SocketWriteEvent, FD_WRITE | FD_CLOSE);

	// 尝试发送
	int nRet = send(m_socket, pSendBuffer, (int)strlen(pSendBuffer), NULL);

	if (nRet == SOCKET_ERROR)
	{
		m_nLastWSAError = WSAGetLastError();

		// 遭遇阻塞
		if (m_nLastWSAError == WSAEWOULDBLOCK)
		{
			DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketWriteEvent, FALSE, nTimeOutSec * 1000, FALSE);

			// 如果网络事件发生
			WSANETWORKEVENTS wsaEvents;
			memset(&wsaEvents, 0, sizeof(wsaEvents));

			if (dwRet == WSA_WAIT_EVENT_0)
			{
				WSAResetEvent(m_SocketWriteEvent);
				WSAEnumNetworkEvents(m_socket, m_SocketWriteEvent, &wsaEvents);

				// 如果发送可以进行并且没有错误发生
				if ((wsaEvents.lNetworkEvents & FD_WRITE) &&
					(wsaEvents.iErrorCode[FD_WRITE_BIT] == 0))
				{
					// 再次发送文本
					nRet = (int)send(m_socket, pSendBuffer, (int)strlen(pSendBuffer), NULL);

					if (nRet > 0)
					{
						// 如果发送字节大于0，表明发送成功
						return SOB_RET_OK;
					}
				}
			}
			else
			{
				// 超时
				bIsTimeOut = true;
			}
		}
		else
		{
			m_bIsConnected = false;
		}
	}
	else
	{
		// 第一次便发送成功
		return SOB_RET_OK;
	}

	// 如果超时
	if (bIsTimeOut)
	{
		return SOB_RET_TIMEOUT;
	}

	// 如果上述发送失败
	m_nLastWSAError = WSAGetLastError();

	return SOB_RET_FAIL;
}

// CCSocketBase 发送缓冲数据(缓冲应该比待接受量要大一点才安全)<发送一定数据>
int CCSocketBase::CCSocketBaseSendBuffer(char * pSendBuffer, UINT uiBufferSize, USHORT nTimeOutSec)
{
	bool bIsTimeOut = false;

	// 检查连接状态
	if (!m_bIsConnected)
	{
		return SOB_RET_FAIL;
	}

	// 发送前注册事件
	WSAResetEvent(m_SocketWriteEvent);
	WSAEventSelect(m_socket, m_SocketWriteEvent, FD_WRITE | FD_CLOSE);

	// 发送量计数
	int nSent = 0;

	// 总发送次数，为发送次数定一个限额
	int nSendTimes = 0;
	int nSendLimitTimes = (int)((float)uiBufferSize / 500 + 1.5);      // 假定当前每次发送肯定不少于500字节
	UINT uiLeftBuffer = uiBufferSize;									// 未发送完的缓存大小

	// 发送游标
	char* pcSentPos = pSendBuffer;

	// 直到所有的缓冲都发送完毕
	while (nSent < (int)uiBufferSize)
	{
		// 检查发送次数是否超限
		if (nSendTimes > nSendLimitTimes)
		{
			break;
		}

		int nRet = send(m_socket, pcSentPos, uiLeftBuffer, NULL);

		if (nRet == SOCKET_ERROR)
		{
			m_nLastWSAError = WSAGetLastError();

			// 遭遇阻塞
			if (m_nLastWSAError == WSAEWOULDBLOCK)
			{
				DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketWriteEvent, FALSE, nTimeOutSec * 1000, FALSE);

				// 如果网络事件发生
				WSANETWORKEVENTS wsaEvents;
				memset(&wsaEvents, 0, sizeof(wsaEvents));

				if (dwRet == WSA_WAIT_EVENT_0)
				{
					WSAResetEvent(m_SocketWriteEvent);
					WSAEnumNetworkEvents(m_socket, m_SocketWriteEvent, &wsaEvents);

					// 如果发送可以进行并且没有错误发生
					if ((wsaEvents.lNetworkEvents & FD_WRITE) &&
						(wsaEvents.iErrorCode[FD_WRITE_BIT] == 0))
					{
						// 再次发送文本
						nRet = send(m_socket, pcSentPos, uiLeftBuffer, NULL);

						if (nRet > 0)
						{
							// 如果发送字节大于0，表明发送成功
							nSendTimes++;

							nSent += nRet;
							uiLeftBuffer -= nRet;
							pcSentPos += nRet;
						}
						else
						{
							m_nLastWSAError = WSAGetLastError();

							// 如果接收到事件发送时也阻塞了，则等待一个周期后重试
							if (m_nLastWSAError == WSAEWOULDBLOCK)
							{
								Sleep(1);
							}
							else
							{
								// 对于其他错误，直接退出
								break;
							}
						}
					}
				}
				else
				{
					// 超时
					bIsTimeOut = true;
					break;
				}
			}
			else
			{
				// 遇到阻塞之外的错误，直接退出
				m_bIsConnected = false;
				break;
			}
		}
		else
		{
			// 发送成功，累加发送量，更新游标
			nSendTimes++;

			nSent += nRet;
			uiLeftBuffer -= nRet;
			pcSentPos += nRet;
		}
	}

	// 如果发送完成
	if (nSent == uiBufferSize)
	{
		return SOB_RET_OK;
	}

	// 如果超时
	if (bIsTimeOut)
	{
		return SOB_RET_TIMEOUT;
	}

	// 没能成功返回
	m_nLastWSAError = WSAGetLastError();

	return SOB_RET_FAIL;
}

// CCSocketBase 接收缓冲数据(缓冲应该比待接受量要大一点才安全)<接收全部数据>
int CCSocketBase::CCSocketBaseRecvOnce(char * pRecvBuffer, UINT uiBufferSize, UINT & uiRecv, USHORT nTimeOutSec)
{
	bool bIsTimeOut = false;

	// 检查连接状态
	if (!m_bIsConnected)
	{
		return SOB_RET_FAIL;
	}

	// 发送前注册事件
	WSAResetEvent(m_SocketReadEvent);
	WSAEventSelect(m_socket, m_SocketReadEvent, FD_READ | FD_CLOSE);

	// 尝试接收
	int nRet = recv(m_socket, pRecvBuffer, uiBufferSize, NULL);

	if (nRet == SOCKET_ERROR)
	{
		m_nLastWSAError = WSAGetLastError();

		// 遭遇阻塞
		if (m_nLastWSAError == WSAEWOULDBLOCK)
		{
			DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketReadEvent, FALSE, nTimeOutSec * 1000, FALSE);

			// 如果网络事件发生
			WSANETWORKEVENTS wsaEvents;
			memset(&wsaEvents, 0, sizeof(wsaEvents));

			if (dwRet == WSA_WAIT_EVENT_0)
			{
				WSAResetEvent(m_SocketReadEvent);
				int nEnum = WSAEnumNetworkEvents(m_socket, m_SocketReadEvent, &wsaEvents);

				// 如果接受可以进行并且没有错误发生
				if ((wsaEvents.lNetworkEvents & FD_READ) &&
					(wsaEvents.iErrorCode[FD_READ_BIT] == 0))
				{
					// 再次接受文本
					nRet = recv(m_socket, pRecvBuffer, uiBufferSize, NULL);

					if (nRet > 0)
					{
						// 如果发送字节大于0，表明发送成功
						uiRecv = nRet;
						return SOB_RET_OK;
					}
				}
			}
			else
			{
				bIsTimeOut = true;
			}
		}
		else
		{
			m_bIsConnected = false;
		}
	}
	else
	{
		// 第一次便接收成功
		uiRecv = nRet;
		return SOB_RET_OK;
	}

	// 如果超时
	if (bIsTimeOut)
	{
		return SOB_RET_TIMEOUT;
	}

	// 如果上述接收失败
	m_nLastWSAError = WSAGetLastError();

	return SOB_RET_FAIL;
}

// CCSocketBase 接收缓冲数据(缓冲应该比待接受量要大一点才安全)<接收一定数据>
int CCSocketBase::CCSocketBaseRecvBuffer(char * pRecvBuffer, UINT uiBufferSize, UINT uiRecvSize, USHORT nTimeOutSec)
{
	bool bIsTimeOut = false;

	// 检查连接状态
	if (!m_bIsConnected)
	{
		return SOB_RET_FAIL;
	}

	// 接收前注册事件
	WSAResetEvent(m_SocketReadEvent);
	WSAEventSelect(m_socket, m_SocketReadEvent, FD_READ | FD_CLOSE);

	// 接收量计数
	int nReceived = 0;

	// 总接收次数，为接收次数定一个限额
	int nRecvTimes = 0;
	int nRecvLimitTimes = (int)((float)uiRecvSize / 500 + 1.5);      //假定当前每次接收肯定不少于500字节

	// 接收游标
	char* pcRecvPos = pRecvBuffer;

	// 直到接收到足够指定量的缓冲
	while (nReceived < (int)uiRecvSize)
	{
		// 检查接收次数是否超限
		if (nRecvTimes > nRecvLimitTimes)
		{
			break;
		}

		int nRet = recv(m_socket, pcRecvPos, uiBufferSize, NULL);

		if (nRet == SOCKET_ERROR)
		{
			m_nLastWSAError = WSAGetLastError();

			// 遭遇阻塞
			if (m_nLastWSAError == WSAEWOULDBLOCK)
			{
				DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketReadEvent, FALSE, nTimeOutSec * 1000, FALSE);

				// 如果网络事件发生
				WSANETWORKEVENTS wsaEvents;
				memset(&wsaEvents, 0, sizeof(wsaEvents));

				if (dwRet == WSA_WAIT_EVENT_0)
				{
					WSAResetEvent(m_SocketReadEvent);
					WSAEnumNetworkEvents(m_socket, m_SocketReadEvent, &wsaEvents);

					// 如果接收可以进行并且没有错误发生
					if ((wsaEvents.lNetworkEvents & FD_READ) &&
						(wsaEvents.iErrorCode[FD_READ_BIT] == 0))
					{
						// 再次接收
						nRet = recv(m_socket, pcRecvPos, uiBufferSize, NULL);

						if (nRet > 0)
						{
							// 如果接收字节大于0，表明发送成功
							nRecvTimes++;

							nReceived += nRet;
							uiBufferSize -= nRet;
							pcRecvPos += nRet;
						}
						else
						{
							m_nLastWSAError = WSAGetLastError();

							// 如果接收到事件发送时也阻塞了，则等待一个周期后重试
							if (m_nLastWSAError == WSAEWOULDBLOCK)
							{
								Sleep(1);
							}
							else
							{
								// 对于其他错误，直接退出
								break;
							}
						}
					}
				}
				else
				{
					// 超时
					bIsTimeOut = true;
					break;
				}
			}
			else
			{
				// 遇到阻塞之外的错误，直接退出
				m_bIsConnected = false;
				break;
			}
		}
		else if (nRet == 0)		// 对方主动关闭连接
		{
			m_bIsConnected = false;
			break;
		}
		else
		{
			// 接收成功，累加接受量，更新游标
			nRecvTimes++;

			nReceived += nRet;
			uiBufferSize -= nRet;
			pcRecvPos += nRet;
		}
	}

	// 如果接收完成
	if (nReceived == uiRecvSize)
	{
		return SOB_RET_OK;
	}

	// 如果超时
	if (bIsTimeOut)
	{
		return SOB_RET_TIMEOUT;
	}

	// 没能成功返回
	m_nLastWSAError = WSAGetLastError();

	return SOB_RET_FAIL;
}

// CCSocketBase 网址转换为IP地址
bool CCSocketBase::ResolveAddressToIp(const char * pcAddress, char * pcIp)
{
	addrinfo adiHints, *padiResult;
	int	nRet;

	memset(&adiHints, 0, sizeof(addrinfo));

	// 仅解析IPV4的地址
	adiHints.ai_flags = AI_CANONNAME;
	adiHints.ai_family = AF_INET;
	adiHints.ai_socktype = SOCK_STREAM;
	adiHints.ai_protocol = IPPROTO_TCP;

	// 转换地址
	nRet = ::getaddrinfo(pcAddress, NULL, &adiHints, &padiResult);

	// 检查结果
	if (nRet != 0)
	{
		freeaddrinfo(padiResult);
		return false;
	}

	// 拷贝结果,只拷贝第一个
	if (padiResult->ai_addr != NULL)
	{
		::strcpy(pcIp, inet_ntoa(((sockaddr_in*)padiResult->ai_addr)->sin_addr));
	}

	freeaddrinfo(padiResult);

	return true;
}

// CCSocketBase 获取本机IP地址
void CCSocketBase::GetLocalIPAddr()
{
	char chHostName[MAX_PATH] = { 0 };

	if (gethostname(chHostName, sizeof(chHostName)) == 0)
	{
		hostent* pHostent = gethostbyname(chHostName);
		hostent& he = *pHostent;
		sockaddr_in sa;
		for (int nAdapter = 0; he.h_addr_list[nAdapter]; nAdapter++)
		{
			memset(m_pcLocalIP, 0, SOB_IP_LENGTH);
			memcpy(&sa.sin_addr.s_addr, he.h_addr_list[nAdapter], he.h_length);
			strcpy(m_pcLocalIP, inet_ntoa(*(struct in_addr *)he.h_addr_list[nAdapter]));
		}
	}

}

// CCSocketBase 访问本机IP地址(静态)
const char * CCSocketBase::GetLocalIP()
{
	return m_pcLocalIP;
}

// CCSocketBase 访问本机端口号(静态)
USHORT CCSocketBase::GetLocalPort()
{
	return m_sLocalPort;
}

// CCSocketBase 设置本机IP地址(静态)
void CCSocketBase::SetLocalIP(const char * pLocalIP, int nSize)
{
	memset(m_pcLocalIP, 0, SOB_IP_LENGTH);
	memcpy_s(m_pcLocalIP, SOB_IP_LENGTH, pLocalIP, nSize);
}

// CCSocketBase 设置本机端口号(静态)
void CCSocketBase::SetLocalPort(USHORT sLocalPort)
{
	m_sLocalPort = sLocalPort;
}
