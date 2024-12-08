
// SocketServerDlg.cpp: 구현 파일
//

#include "stdafx.h"
#include "SocketServer.h"
#include "SocketServerDlg.h"
#include "CClientSocket.h"
#include "afxdialogex.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();


// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSocketServerDlg 대화 상자



CSocketServerDlg::CSocketServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SOCKETSERVER_DIALOG, pParent), clientList(nullptr)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	pMainInstance = this;
}

void CSocketServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_List);
}

BEGIN_MESSAGE_MAP(CSocketServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_USER + 1, &CSocketServerDlg::OnAddMessageToList)  // 사용자 정의 메시지 핸들러 추가
END_MESSAGE_MAP()



// CSocketServerDlg 메시지 처리기

BOOL CSocketServerDlg::OnInitDialog() {
	CDialogEx::OnInitDialog();

	if (!dbManager.openDatabase("ChatDatabase.db")) {
		AfxMessageBox(_T("Failed to open or create database!"));
		return FALSE;
	}

	// 시스템 메뉴 설정...

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	// 리스트 박스 초기화 확인
	if (!m_List.GetSafeHwnd()) {
		AfxMessageBox(_T("Failed to initialize list box control."));
		return FALSE;
	}

	clientList = (CListBox*)GetDlgItem(IDC_CLIENT_LIST);

	if (m_ListenSocket.Create(21000, SOCK_STREAM)) {
		if (!m_ListenSocket.Listen()) {
			AfxMessageBox(_T("ERROR: Listen() return False"));
		}
	}
	else {
		AfxMessageBox(_T("ERROR: Failed to create server socket!"));
	}

	return TRUE;
}



void CSocketServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CSocketServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CSocketServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CSocketServerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	pMainInstance = nullptr;  // 대화 상자가 파괴될 때 정적 포인터를 null로 설정

	// 클라이언트 소켓 리스트를 반복하여 모든 클라이언트 연결 종료
	for (auto client : m_ListenSocket.m_ClientList) {
		if (client) {
			client->ShutDown();
			client->Close();
			delete client;
		}
	}
	m_ListenSocket.m_ClientList.clear();  // 리스트를 비워줍니다.

	m_ListenSocket.ShutDown();
	m_ListenSocket.Close();
}

LRESULT CSocketServerDlg::OnAddMessageToList(WPARAM wParam, LPARAM lParam) {
	CString* pMessage = (CString*)wParam;

	// 메시지를 리스트 박스에 추가
	int cnt = m_List.GetCount();
	m_List.InsertString(cnt, *pMessage);

	// 메시지를 데이터베이스에 저장
	std::string message = CT2A(*pMessage);
	if (!dbManager.saveMessage(message)) {
		AfxMessageBox(_T("Failed to save message to database."));
	}

	delete pMessage;
	return 0;
}

