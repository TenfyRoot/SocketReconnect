
// ClientDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Client.h"
#include "ClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CClientDlg �Ի���



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


// CClientDlg ��Ϣ��������

BOOL CClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵������ӵ�ϵͳ�˵��С�

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

	// TODO: �ڴ����Ӷ���ĳ�ʼ������

	SetDlgItemText(IDC_EDIT_IP, _T("127.0.0.1"));
	SetDlgItemInt(IDC_EDIT_PORT, 8888);
	GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);

	InitSocket();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի���������С����ť������Ҫ����Ĵ���
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
	// TODO: �ڴ����ӿؼ�֪ͨ�����������

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
			strError.Format(_T("�ͻ���recvʧ��:%d"),nError);
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
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	m_Sta_Tip.SetWindowText(_T(""));
	GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(FALSE);
	m_ClientSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);
	if (m_ClientSocket == INVALID_SOCKET)
	{
		m_Sta_Tip.SetWindowText(_T("�����׽���ʧ��"));
		GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(TRUE);
		return;
	}

	int nPort = GetDlgItemInt(IDC_EDIT_PORT);
	CString strIp;
	GetDlgItemText(IDC_EDIT_IP,strIp);

	sockaddr_in saServer;
	saServer.sin_family = AF_INET; //��ַ����  
    saServer.sin_port = htons(nPort); //ע��ת��Ϊ�������  
    saServer.sin_addr.S_un.S_addr = inet_addr(CT2A(strIp));  
	if(SOCKET_ERROR == connect(m_ClientSocket,(SOCKADDR *)&saServer,sizeof(saServer)))
	{
		int nError = WSAGetLastError();
		CString strError;
		strError.Format(_T("����ʧ��:%d"),nError);
		m_Sta_Tip.SetWindowText(strError);
		GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(TRUE);
		return ;
	}
	GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
	AfxBeginThread(RecvThreadProc,this);
}


void CClientDlg::OnBnClickedButton3()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
	CString strText;
	GetDlgItemText(IDC_EDIT_SEND,strText);
	if( SOCKET_ERROR == send(m_ClientSocket,(const char * )strText.GetBuffer(),strText.GetLength() * sizeof(TCHAR),0))
	{
		int nError = WSAGetLastError();
		CString strErrorMsg;
		strErrorMsg.Format(_T("����ʧ��:%d"),nError);
		m_Sta_Tip.SetWindowText(strErrorMsg);	
	}
	GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
}