#pragma once

#include "CSerialPortBase.h"


// CSerialPortDlg 对话框

class CSerialPortDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSerialPortDlg)

public:
	CSerialPortDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CSerialPortDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SERIALPORT_DIALOG };
#endif

// 成员
private:
	CRect m_cWindowRect;

private:
	CBrush m_Brush;

private:
	CCSerialPortBase m_cSerialPort;

// 成员函数
public:
	void ConstructionExtra();
	void InitWindowSharp();
	void InitControls();
	void RePaintWindow(CDC& dc);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CComboBox m_cSerialPortNameCbx;
	CComboBox m_cSerialPortBaudCbx;
	CComboBox m_cSerialPortDataBitCbx;
	CComboBox m_cSerialPortStopBitCbx;
	CComboBox m_cSerialPortCheckBitCbx;
	CButton m_cSerialPortRefreshBtn;
	CButton m_cSerialPortOpenBtn;
};
