#pragma once

#ifndef __CSERIALPORTBASE_H__
#define __CSERIALPORTBASE_H__

//Include Window Header File
#include <Windows.h>

//Include C/C++ Header File
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <mmreg.h>
#include <wchar.h>
#include <tchar.h>
#include <time.h>
#include <mmsystem.h>

#include <iostream>
#include <map>
#include <vector>
#include <process.h>

using namespace std;

//Macro Definition
#define SERIALPORT_COMM_INPUT_BUFFER_SIZE	4096	// 串口通信输入缓冲区大小
#define SERIALPORT_COMM_OUTPUT_BUFFER_SIZE	4096	// 串口通信输出缓冲区大小

//Struct Definition
typedef struct
{
	CHAR chPort[MAX_PATH];	// 串口号
	DWORD dwBaudRate;		// 串口波特率
	BYTE byDataBits;		// 串口数据位
	BYTE byStopBits;		// 串口停止位
	BYTE byCheckBits;		// 串口校验位
}S_SERIALPORT_PROPERTY, *LPS_SERIALPORT_PROPERTY;

//Template Release
template<class T>
void SafeDelete(T*& t)
{
	if (nullptr != t)
	{
		delete t;
		t = nullptr;
	}
}

template<class T>
void SafeDeleteArray(T*& t)
{
	if (nullptr != t)
	{
		delete[] t;
		t = nullptr;
	}
}

template<class T>
void SafeRelease(T*& t)
{
	if (nullptr != t)
	{
		t->Release();
		t = nullptr;
	}
}

//Class Definition
class CCSerialPortBase
{
private:
	HANDLE m_hCOM;			// CCSerialPortBase SerialPort Handle(串口句柄)
	HANDLE m_hListenThread;	// CCSerialPortBase SerialPort Listen Thread Handle(串口监听线程句柄)

private:
	static bool m_sbExit;	// CCSerialPortBase Exit Flag(串口退出标志)
	CRITICAL_SECTION m_csCOMSync;	// CCSerialPortBase Critical Section Sync(串口异步接收临界区)

public:
	map<int, string> m_mapEnumCOM;	// CCSerialPortBase Enum SerialPort Map(串口枚举列表)

public:
	void EnumSerialPort();	// CCSerialPortBase 枚举串口

protected:
	bool CCSerialPortBaseCreate(const char* szPort);	// CCSerialPortBase 打开串口(串口名称)
	bool CCSerialPortBaseConfig(S_SERIALPORT_PROPERTY sCommProperty);	// CCSerialPortBase 配置串口

public:
	CCSerialPortBase();		// CCSerialPortBase 构造函数
	~CCSerialPortBase();	// CCSerialPortBase 析构函数

	bool CCSerialPortBaseInit(S_SERIALPORT_PROPERTY sCommProperty);	// CCSerialPortBase 初始化串口
	bool CCSerialPortBaseInitListen();	// CCSerialPortBase 初始化串口监听
	void CCSerialPortBaseClose();	// CCSerialPortBase 关闭串口
	void CCSerialPortBaseCloseListen();	// CCSerialPortBase 关闭串口监听

	static unsigned int CALLBACK OnReceiveBuffer(LPVOID lpParameters);	// CCSerialPortBase 串口接收线程

};

#endif // !__CSERIALPORTBASE_H__
