
// ServerDlg.h : 头文件
//

#pragma once

#include "afxwin.h"
#include <Winsock2.h>
#pragma comment(lib,"Ws2_32.lib")

// CServerDlg 对话框
struct CClientInfo
{
	SOCKET sSocket;
	CString strIp;
	u_short nPort;
	CString GetShowText();
};

class CServerDlg : public CDialogEx
{
// 构造
public:
	CServerDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CServerDlg();
// 对话框数据
	enum { IDD = IDD_SERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	
public:
	afx_msg void OnBnClickedButton1();
	void InitSocket();
	SOCKET m_ServerSocket;
	void OnSocketConnected(SOCKET sClientSocket,sockaddr_in * saClient);
	void OnSocketDisconnect(SOCKET aClientSocket);
	CListBox m_ClientListBox;
	CArray<CClientInfo> m_ClientArray;
	CListBox m_RecvMsgListBox;
	CRITICAL_SECTION m_CS_ClientArray;
	void LockClientArray(){EnterCriticalSection(&m_CS_ClientArray);}
	void UnLockClientArray(){LeaveCriticalSection(&m_CS_ClientArray);}
	afx_msg void OnBnClickedButton3();
};
