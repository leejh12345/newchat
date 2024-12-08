#include "stdafx.h"
#include "CListenSocket.h"
#include "CClientSocket.h"
#include "SocketServerDlg.h"
#include <mutex> // 멀티스레드 보호를 위해 mutex 사용
#include <thread>

std::mutex clientListMutex;

CListenSocket::CListenSocket() {}

CListenSocket::~CListenSocket() {
    // 모든 클라이언트 소켓 삭제
    std::lock_guard<std::mutex> lock(clientListMutex);  // 클라이언트 리스트 접근 보호

    for (auto client : m_ClientList) {
        if (client) {
            client->ShutDown();
            client->Close();
            delete client;
        }
    }
    m_ClientList.clear();
}

void CListenSocket::OnAccept(int nErrorCode) {
    CClientSocket* pClient = new CClientSocket();
    if (Accept(*pClient)) {
        pClient->SetListenSocket(this);

        {
            std::lock_guard<std::mutex> lock(clientListMutex);
            m_ClientList.push_back(pClient);
        }

        // 클라이언트 IP와 포트를 사용자 목록에 추가
        CString strIPAddress;
        UINT uPortNumber = 0;
        pClient->GetPeerName(strIPAddress, uPortNumber);

        CString clientInfo;
        clientInfo.Format(_T("%s:%d"), strIPAddress, uPortNumber);

        // 메인 다이얼로그에 사용자 추가
        if (CSocketServerDlg::pMainInstance && ::IsWindow(CSocketServerDlg::pMainInstance->GetSafeHwnd())) {
            CSocketServerDlg::pMainInstance->clientList->AddString(clientInfo);
        }
    }
    else {
        delete pClient;
    }

    CAsyncSocket::OnAccept(nErrorCode);
}


void CListenSocket::SendAllClients(const CString& message) {
    for (auto client : m_ClientList) {
        if (client && client->IsConnected()) {
            
            std::thread sendThread([client, message]() {
                client->Send((LPCTSTR)message, message.GetLength() * sizeof(TCHAR));
                });

            
            sendThread.detach();
        }
    }
}

void CListenSocket::CloseClientSocket(CClientSocket* pClient) {
    std::lock_guard<std::mutex> lock(clientListMutex);  // 클라이언트 리스트 접근 보호

    m_ClientList.remove(pClient);
    if (pClient) {
        pClient->ShutDown();
        pClient->Close();
        delete pClient;
    }
}

void CListenSocket::UpdateUI()
{
    if (CSocketServerDlg::pMainInstance && ::IsWindow(CSocketServerDlg::pMainInstance->GetSafeHwnd())) {
        CSocketServerDlg::pMainInstance->clientList->ResetContent(); // 기존 목록 초기화

        for (const auto& client : m_ClientList) {
            CString strIPAddress;
            UINT uPortNumber = 0;
            client->GetPeerName(strIPAddress, uPortNumber);

            CString clientKey;
            clientKey.Format(_T("%s:%d"), strIPAddress, uPortNumber);

            CString displayText;
            if (aliasMap.find(clientKey) != aliasMap.end()) {
                // 별명이 있는 경우 별명 표시
                displayText.Format(_T("[%s - %s]"), strIPAddress, aliasMap[clientKey]);
            }
            else {
                // 별명이 없을 경우 기본 IP:Port로 표시
                displayText.Format(_T("[%s:%d]"), strIPAddress, uPortNumber);
            }

            CSocketServerDlg::pMainInstance->clientList->AddString(displayText);
        }
    }
}

