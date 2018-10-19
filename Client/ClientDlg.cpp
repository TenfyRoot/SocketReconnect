
// ClientDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Client.h"
#include "ClientDlg.h"
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


// CClientDlg 对话框



CClientDlg::CClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CClientDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_ClientSocket = INVALID_SOCKET;
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
	GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);

	InitSocket();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

UINT __cdecl RecvThreadProc( LPVOID pParam )
{
	CClientDlg * pThis = (CClientDlg * ) pParam;
	while(1)
	{
		char szBuf[256] = {0};
		if(SOCKET_ERROR == recv(pThis->m_ClientSocket,szBuf,250,0))
		{
			int nError = WSAGetLastError();
			CString strError;
			strError.Format(_T("客户端recv失败:%d"),nError);
			pThis->m_Sta_Tip.SetWindowText(strError);
			pThis->GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(TRUE);
			break;
		}
		else
		{
			pThis->m_ClientRecvListBox.AddString((LPCTSTR)szBuf);
		}
	}
	return 0;
}

void CClientDlg::OnBnClickedButtonConnect()
{
	// TODO: 在此添加控件通知处理程序代码
	m_Sta_Tip.SetWindowText(_T(""));
	GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(FALSE);
	m_ClientSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);
	if (m_ClientSocket == INVALID_SOCKET)
	{
		m_Sta_Tip.SetWindowText(_T("创建套接字失败"));
		GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(TRUE);
		return;
	}

	int nPort = GetDlgItemInt(IDC_EDIT_PORT);
	CString strIp;
	GetDlgItemText(IDC_EDIT_IP,strIp);

	sockaddr_in saServer;
	saServer.sin_family = AF_INET; //地址家族  
    saServer.sin_port = htons(nPort); //注意转化为网络节序  
    saServer.sin_addr.S_un.S_addr = inet_addr(CT2A(strIp));  
	if(SOCKET_ERROR == connect(m_ClientSocket,(SOCKADDR *)&saServer,sizeof(saServer)))
	{
		int nError = WSAGetLastError();
		CString strError;
		strError.Format(_T("连接失败:%d"),nError);
		m_Sta_Tip.SetWindowText(strError);
		GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(TRUE);
		return ;
	}
	GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
	AfxBeginThread(RecvThreadProc,this);
}


void CClientDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
	CString strText;
	GetDlgItemText(IDC_EDIT_SEND,strText);
	if( SOCKET_ERROR == send(m_ClientSocket,(const char * )strText.GetBuffer(),strText.GetLength() * sizeof(TCHAR),0))
	{
		int nError = WSAGetLastError();
		CString strErrorMsg;
		strErrorMsg.Format(_T("发送失败:%d"),nError);
		m_Sta_Tip.SetWindowText(strErrorMsg);	
	}
	GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
}
