#pragma once

#include "CMemoryAnalysisDlg.h"

// CDataAnalysisDlg 对话框

class CDataAnalysisDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDataAnalysisDlg)

public:
	CDataAnalysisDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CDataAnalysisDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DATAANALYSIS_DIALOG };
#endif

// 成员
private:
	CRect m_cWindowRect;

private:
	CBrush m_Brush;

private:
	CMemoryAnalysisDlg m_cMemoryAnalysisDlg;

// 成员函数
public:
	void ConstructionExtra();
	void InitWindowSharp();
	void InitTabControl();
	void InitChildWindow();
	void RePaintWindow(CDC& dc);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CTabCtrl m_cTabDataTbc;
	afx_msg void OnTcnSelchangeTabData(NMHDR *pNMHDR, LRESULT *pResult);
};
