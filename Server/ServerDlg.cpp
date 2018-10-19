
// ServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "ServerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CServerDlg 对话框



CServerDlg::CServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CServerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_ServerSocket  = INVALID_SOCKET;
	InitializeCriticalSection(&m_CS_ClientArray);
}
CServerDlg::~CServerDlg()
{
	DeleteCriticalSection(&m_CS_ClientArray);
}
void CServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CLIENT_LIST, m_ClientListBox);
	DDX_Control(pDX, IDC_LIST_RECV, m_RecvMsgListBox);
}

BEGIN_MESSAGE_MAP(CServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CServerDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, &CServerDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// CServerDlg 消息处理程序

BOOL CServerDlg::OnInitDialog()
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

	InitSocket();

	SetDlgItemInt(IDC_EDIT_PORT,10103);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CServerDlg::OnPaint()
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
HCURSOR CServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

CString CClientInfo::GetShowText()
{
	CString strItemText;
	strItemText.Format(_T("%s:%d"),strIp,nPort);
	return strItemText;
}

void CServerDlg::OnSocketConnected(SOCKET sClientSocket,sockaddr_in * saClient)
{
	u_short uPort =  ntohs(((sockaddr_in *)saClient)->sin_port);
	CString strIp = CA2T(inet_ntoa(((sockaddr_in *)saClient)->sin_addr));
	CClientInfo aClientInfo;
	aClientInfo.nPort = uPort;
	aClientInfo.strIp = strIp;
	aClientInfo.sSocket = sClientSocket;

	LockClientArray();
	m_ClientArray.Add(aClientInfo);

	int nIndexInserted = m_ClientListBox.AddString(aClientInfo.GetShowText());
	m_ClientListBox.SetItemData(nIndexInserted,aClientInfo.sSocket);
	UnLockClientArray();


}

void CServerDlg::OnSocketDisconnect(SOCKET aClientSocket)
{
	LockClientArray();
	for(int i = 0;i<m_ClientArray.GetCount();i++)
	{
		CClientInfo aClientInfo = m_ClientArray.GetAt(i);
		if(aClientInfo.sSocket == aClientSocket)
		{
			m_ClientListBox.DeleteString(m_ClientListBox.FindString(0,aClientInfo.GetShowText()));
			m_ClientArray.RemoveAt(i);
			break;
		}
	}
	UnLockClientArray();
}

UINT __cdecl ClientRecvThreadPorc( LPVOID pParam )
{
	CServerDlg * pThis = (CServerDlg * ) pParam;
	while(1)
	{
		timeval tv;
		tv.tv_sec  = 5;
		tv.tv_usec = 0;

		fd_set fdsets;
		FD_ZERO(&fdsets);
		//Init fdsets
		pThis->LockClientArray();
		int nClientCount = pThis->m_ClientArray.GetCount();
		int nSelectCount = 0;
		for(int i = 0;i < nClientCount;i++)
		{
			CClientInfo aClientInfo = pThis->m_ClientArray.GetAt(i);
			nSelectCount ++;
			FD_SET(aClientInfo.sSocket,&fdsets);
		}
		pThis->UnLockClientArray();
		if(nSelectCount == 0)
		{
			Sleep(500);
			continue;
		}
		int nCanReadCount = select(0,&fdsets,NULL,NULL,&tv);
		pThis->LockClientArray();
		CArray<CClientInfo> RecvArray;
		for(int i = 0;i < nClientCount;i++)
		{
			CClientInfo aClientInfo = pThis->m_ClientArray.GetAt(i);
			if(FD_ISSET(aClientInfo.sSocket,&fdsets))
			{
				RecvArray.Add(aClientInfo);
			}
		}
		pThis->UnLockClientArray();
		for(int i = 0;i < RecvArray.GetCount();i++)
		{
			CClientInfo aClientInfo = RecvArray.GetAt(i);
			char szRecvBuf[1024] = {0};
			int nRecvRet = recv(aClientInfo.sSocket,szRecvBuf,1024,0);
			if(nRecvRet == SOCKET_ERROR)
			{
				pThis->OnSocketDisconnect(aClientInfo.sSocket);
				int nError = ::WSAGetLastError();
				CString strErrorMsg;
				strErrorMsg.Format(_T("服务器Recv失败:%d"),nError);
				AfxMessageBox(strErrorMsg);
			}
			else
				pThis->m_RecvMsgListBox.AddString((LPCTSTR)szRecvBuf);
		}
	}
	return 0;
}

UINT __cdecl ServerListenThreadPorc( LPVOID pParam )
{
	CServerDlg * pThis = (CServerDlg *) pParam;
	SOCKET sServerSocket = pThis->m_ServerSocket;


	while(1)
	{
		sockaddr_in saClient = {0};
		int  nClientSocketLen = sizeof(saClient);
		SOCKET sClientSocket = accept(sServerSocket,(sockaddr *)&saClient,&nClientSocketLen);
		if(sClientSocket ==  INVALID_SOCKET)
		{
			int nError = ::WSAGetLastError();
			AfxMessageBox(_T("accept失败"));
			break;
		}
		else
		{
			pThis->OnSocketConnected(sClientSocket,&saClient);

		}
	}
	return 0;
}


void CServerDlg::OnBnClickedButton1()
{
	int nPort = GetDlgItemInt(IDC_EDIT_PORT);
	// TODO: 在此添加控件通知处理程序代码
	m_ServerSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);
	if(m_ServerSocket == INVALID_SOCKET)
	{
		AfxMessageBox(_T("创建套接字失败"));
		return ;
	}
	sockaddr_in saServer;
	saServer.sin_family = AF_INET; //地址家族  
    saServer.sin_port = htons(nPort); //注意转化为网络节序  
    saServer.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");  
	if(SOCKET_ERROR == bind(m_ServerSocket,(SOCKADDR *)&saServer,sizeof(saServer)))
	{
		AfxMessageBox(_T("绑定失败"));
		return ;
	}
	if(SOCKET_ERROR == listen(m_ServerSocket,SOMAXCONN))
	{
		AfxMessageBox(_T("监听失败啊"));
		return ;
	}
	GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
	AfxBeginThread(ServerListenThreadPorc,this);
	AfxBeginThread(ClientRecvThreadPorc,this);
}

void CServerDlg::InitSocket()
{
	WSADATA wsaData = {0};
	if(0 != WSAStartup(MAKEWORD(2,2),&wsaData))
	{
		AfxMessageBox(_T("socket 初始化失败"));
		return ;
	}
}

void CServerDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_ClientListBox.GetSelCount() <= 0)
	{
		AfxMessageBox(_T("请选中右边的客户端进行发送"));
		return ;
	}
	CString strSend;
	GetDlgItemText(IDC_EDIT2,strSend);
	for(int i = 0;i < m_ClientListBox.GetCount();i++)
	{
		if(m_ClientListBox.GetSel(i) > 0)
		{
			SOCKET aClientSocket = m_ClientListBox.GetItemData(i);
			if( SOCKET_ERROR == send(aClientSocket,(const char * )strSend.GetBuffer(),strSend.GetLength() * sizeof(TCHAR),0))
			{
				int nError = ::WSAGetLastError();
				CString strError;
				strError.Format(_T("send  失败%d"),nError);
				AfxMessageBox(strError);
			}
		}
	}
}

