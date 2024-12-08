#include "stdafx.h"
#include "CConnectSocket.h"
#include "SocketClientDlg.h"
#include <thread>
#include <mutex>

// 수신 처리 및 종료 동기화를 위한 뮤텍스
std::mutex receiveMutex;
std::mutex closeMutex;

CConnectSocket::CConnectSocket()
{
}

CConnectSocket::~CConnectSocket()
{
}

void CConnectSocket::OnClose(int nErrorCode)
{
    // 소켓 종료 시 동기화 처리
    std::lock_guard<std::mutex> lock(closeMutex);

    ShutDown();
    Close();

    CSocket::OnClose(nErrorCode);  // 기본 클래스의 OnClose 호출

    AfxMessageBox(_T("ERROR: Disconnected from server!"));
    ::PostQuitMessage(0);
}

void CConnectSocket::OnReceive(int nErrorCode)
{
    TCHAR szBuffer[1024];
    ::ZeroMemory(szBuffer, sizeof(szBuffer));

    if (Receive(szBuffer, sizeof(szBuffer) - 1) > 0) {
        CSocketClientDlg* pMain = CSocketClientDlg::pMainInstance;
        if (pMain && ::IsWindow(pMain->GetSafeHwnd())) {
            CString* pReceivedMessage = new CString(szBuffer);
            // PostMessage로 메인 스레드에서 UI 업데이트 처리
            pMain->PostMessage(WM_USER + 1, (WPARAM)pReceivedMessage);
        }
    }

    CSocket::OnReceive(nErrorCode);
}

