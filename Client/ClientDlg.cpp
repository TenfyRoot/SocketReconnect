
// ClientDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Client.h"
#include "ClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CClientDlg 对话框
UINT WINAPIV ClientRecvThread(LPVOID pParam)
{
	CClientDlg * pThis = (CClientDlg *)pParam;
	return pThis->ClientRecvThreadProc();
}

UINT WINAPIV ClientConnectThread(LPVOID pParam)
{
	CClientDlg * pThis = (CClientDlg *)pParam;
	return pThis->ClientConnectThreadProc();
}


CClientDlg::CClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CClientDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_ClientSocket = INVALID_SOCKET;
	m_bClientConnectThreadWorking = false;
	m_bClientRecvThreadWorking = false;
	m_bConnected = false;
	m_pClientConnectThread = NULL;
	m_pClientRecvThread = NULL;
	InitializeCriticalSection(&m_CS_ConnectStatus);
}

CClientDlg::~CClientDlg()
{
	m_bClientConnectThreadWorking = false;
	WaitForSingleObject(m_pClientConnectThread, 500);
	m_bClientRecvThreadWorking = false;
	WaitForSingleObject(m_pClientRecvThread, 500);

	DeleteCriticalSection(&m_CS_ConnectStatus);
}

void CClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST2, m_ClientRecvListBox);
	DDX_Control(pDX, IDC_STATIC_TIP, m_Sta_Tip);
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_EDIT_SEND, &CClientDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CClientDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON3, &CClientDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON_UNCONNECT, &CClientDlg::OnBnClickedButtonUnconnect)
END_MESSAGE_MAP()


// CClientDlg 消息处理程序

BOOL CClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	SetDlgItemText(IDC_EDIT_IP, _T("127.0.0.1"));
	SetDlgItemInt(IDC_EDIT_PORT, 8888);
	SetBtnStatus(0);

	InitSocket();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialogEx::OnSysCommand(nID, lParam);
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CClientDlg::InitSocket()
{
	WSADATA wsaData = {0};
	if(0 != WSAStartup(MAKEWORD(2,2),&wsaData))
	{
		m_Sta_Tip.SetWindowText(_T("socket 初始化失败"));
		return ;
	}
}

void CClientDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码

}

void CClientDlg::OnBnClickedButtonConnect()
{
	SetBtnStatus(1);
	m_pClientConnectThread = AfxBeginThread(ClientConnectThread, this);
	m_pClientRecvThread = AfxBeginThread(ClientRecvThread, this);
	
}

UINT CClientDlg::ClientConnectThreadProc()
{
	if (m_ClientSocket != INVALID_SOCKET)
		closesocket(m_ClientSocket);
	m_ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_ClientSocket == INVALID_SOCKET)
	{
		m_Sta_Tip.SetWindowText(_T("创建套接字失败"));
		SetBtnStatus(0);
		return 0;
	}

	int nPort = GetDlgItemInt(IDC_EDIT_PORT);
	CString strIp;
	GetDlgItemText(IDC_EDIT_IP, strIp);
	sockaddr_in saServer;
	saServer.sin_family = AF_INET; //地址家族  
	saServer.sin_port = htons(nPort); //注意转化为网络节序  
	saServer.sin_addr.S_un.S_addr = inet_addr(CT2A(strIp));

	int nLastError = 0;

	m_bClientConnectThreadWorking = true;
	while (m_bClientConnectThreadWorking)
	{
		LockConnectStatus();
		if (m_bConnected)
		{
			UnLockConnectStatus();
			Sleep(200);
			continue;
		}
		UnLockConnectStatus();
		
		if (SOCKET_ERROR == connect(m_ClientSocket, (SOCKADDR *)&saServer, sizeof(saServer)))
		{
			int nError = WSAGetLastError();
			if (nLastError != nError && nError == 10056)//Socket is already connected
			{
				closesocket(m_ClientSocket);
				m_ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				nLastError = nError;
			}
			else if (nLastError != nError && nError == 10061)
			{
				m_Sta_Tip.SetWindowText(_T("由于目标计算机积极拒绝，无法连接"));
				nLastError = nError;
			}
			else if (nLastError != nError)
			{
				int nError = WSAGetLastError();
				CString strErrorMsg;
				strErrorMsg.Format(_T("正在连接[%d]...."), nError);
				m_Sta_Tip.SetWindowText(strErrorMsg);
				nLastError = nError;
			}
			Sleep(100);
			continue;
		}
		LockConnectStatus(true);
		m_Sta_Tip.SetWindowText(_T("已连接"));
		SetBtnStatus(2);
		nLastError = 0;
	}
	if (m_ClientSocket != INVALID_SOCKET)
	{
		closesocket(m_ClientSocket);
	}
	m_bConnected = false;
	return 0;
}

UINT CClientDlg::ClientRecvThreadProc()
{
	m_bClientRecvThreadWorking = true;
	while (m_bClientRecvThreadWorking)
	{
		LockConnectStatus();
		if (!m_bConnected)
		{
			UnLockConnectStatus();
			Sleep(200);
			continue;
		}
		UnLockConnectStatus();

		char szBuf[256] = { 0 };
		if (SOCKET_ERROR == recv(m_ClientSocket, szBuf, 250, 0))
		{
			int nError = WSAGetLastError();
			if (nError != 0)
			{
				LockConnectStatus(false);
				continue;
			}
		}
		m_ClientRecvListBox.AddString((LPCTSTR)szBuf);
	}
	return 0;
}

void CClientDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
	CString strText;
	GetDlgItemText(IDC_EDIT_SEND,strText);
	CStringA sendText = CT2A(strText);
	if( SOCKET_ERROR == send(m_ClientSocket, sendText, sendText.GetLength(), 0))
	{
		int nError = WSAGetLastError();
		CString strErrorMsg;
		strErrorMsg.Format(_T("发送失败:%d"),nError);
		m_Sta_Tip.SetWindowText(strErrorMsg);	
	}
	GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
}


void CClientDlg::OnBnClickedButtonUnconnect()
{
	m_bClientConnectThreadWorking = false;
	WaitForSingleObject(m_pClientConnectThread, 500);
	m_bClientRecvThreadWorking = false;
	WaitForSingleObject(m_pClientRecvThread, 500);

	SetBtnStatus(0);
}

void CClientDlg::SetBtnStatus(int nFlag)
{
	if (nFlag == 0)
	{
		GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(true);
		GetDlgItem(IDC_BUTTON_UNCONNECT)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON3)->EnableWindow(false);
	}
	else if (nFlag == 1)
	{
		GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_UNCONNECT)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON3)->EnableWindow(false);
	}
	else
	{
		GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_UNCONNECT)->EnableWindow(true);
		GetDlgItem(IDC_BUTTON3)->EnableWindow(true);
	}
	
}
