#include "stdafx.h"
#include "CConnectSocket.h"
#include "SocketClientDlg.h"
#include <thread>
#include <mutex>

// ���� ó�� �� ���� ����ȭ�� ���� ���ؽ�
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
    // ���� ���� �� ����ȭ ó��
    std::lock_guard<std::mutex> lock(closeMutex);

    ShutDown();
    Close();

    CSocket::OnClose(nErrorCode);  // �⺻ Ŭ������ OnClose ȣ��

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
            // PostMessage�� ���� �����忡�� UI ������Ʈ ó��
            pMain->PostMessage(WM_USER + 1, (WPARAM)pReceivedMessage);
        }
    }

    CSocket::OnReceive(nErrorCode);
}

