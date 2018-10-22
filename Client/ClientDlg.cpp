
// ClientDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Client.h"
#include "ClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CClientDlg �Ի���
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


// CClientDlg ��Ϣ�������

BOOL CClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	SetDlgItemText(IDC_EDIT_IP, _T("127.0.0.1"));
	SetDlgItemInt(IDC_EDIT_PORT, 8888);
	SetBtnStatus(0);

	InitSocket();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialogEx::OnSysCommand(nID, lParam);
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CClientDlg::InitSocket()
{
	WSADATA wsaData = {0};
	if(0 != WSAStartup(MAKEWORD(2,2),&wsaData))
	{
		m_Sta_Tip.SetWindowText(_T("socket ��ʼ��ʧ��"));
		return ;
	}
}

void CClientDlg::OnBnClickedButton2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

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
		m_Sta_Tip.SetWindowText(_T("�����׽���ʧ��"));
		SetBtnStatus(0);
		return 0;
	}

	int nPort = GetDlgItemInt(IDC_EDIT_PORT);
	CString strIp;
	GetDlgItemText(IDC_EDIT_IP, strIp);
	sockaddr_in saServer;
	saServer.sin_family = AF_INET; //��ַ����  
	saServer.sin_port = htons(nPort); //ע��ת��Ϊ�������  
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
				m_Sta_Tip.SetWindowText(_T("����Ŀ�����������ܾ����޷�����"));
				nLastError = nError;
			}
			else if (nLastError != nError)
			{
				int nError = WSAGetLastError();
				CString strErrorMsg;
				strErrorMsg.Format(_T("��������[%d]...."), nError);
				m_Sta_Tip.SetWindowText(strErrorMsg);
				nLastError = nError;
			}
			Sleep(100);
			continue;
		}
		LockConnectStatus(true);
		m_Sta_Tip.SetWindowText(_T("������"));
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
	CString strText;
	GetDlgItemText(IDC_EDIT_SEND,strText);
	CStringA sendText = CT2A(strText);
	if( SOCKET_ERROR == send(m_ClientSocket, sendText, sendText.GetLength(), 0))
	{
		int nError = WSAGetLastError();
		CString strErrorMsg;
		strErrorMsg.Format(_T("����ʧ��:%d"),nError);
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
