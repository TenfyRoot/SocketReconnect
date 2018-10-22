
// ClientDlg.h : ͷ�ļ�
//

#pragma once

#include <Winsock2.h>
#include "afxwin.h"
#pragma comment(lib,"Ws2_32.lib")
// CClientDlg �Ի���
class CClientDlg : public CDialogEx
{
// ����
public:
	CClientDlg(CWnd* pParent = NULL);	// ��׼���캯��
	~CClientDlg();
// �Ի�������
	enum { IDD = IDD_CLIENT_DIALOG };

	UINT ClientRecvThreadProc();
	UINT ClientConnectThreadProc();

private:
	BOOL m_bConnected;
	SOCKET m_ClientSocket;
	CWinThread *m_pClientConnectThread, *m_pClientRecvThread;
	BOOL m_bClientConnectThreadWorking, m_bClientRecvThreadWorking;

	void SetBtnStatus(int nFlag);
	CRITICAL_SECTION m_CS_ConnectStatus;
	void LockConnectStatus(BOOL bConnected) 
	{ 
		EnterCriticalSection(&m_CS_ConnectStatus); 
		m_bConnected = bConnected;
		LeaveCriticalSection(&m_CS_ConnectStatus);
	}
	void LockConnectStatus() { EnterCriticalSection(&m_CS_ConnectStatus); }
	void UnLockConnectStatus() { LeaveCriticalSection(&m_CS_ConnectStatus); }

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	void InitSocket();
	
public:
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnBnClickedButton3();
	CListBox m_ClientRecvListBox;
	CStatic m_Sta_Tip;
	afx_msg void OnBnClickedButtonUnconnect();
};
