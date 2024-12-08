#include "stdafx.h"
#include "CListenSocket.h"
#include "CClientSocket.h"
#include "SocketServerDlg.h"
#include <mutex> // ��Ƽ������ ��ȣ�� ���� mutex ���
#include <thread>

std::mutex clientListMutex;

CListenSocket::CListenSocket() {}

CListenSocket::~CListenSocket() {
    // ��� Ŭ���̾�Ʈ ���� ����
    std::lock_guard<std::mutex> lock(clientListMutex);  // Ŭ���̾�Ʈ ����Ʈ ���� ��ȣ

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

        // Ŭ���̾�Ʈ IP�� ��Ʈ�� ����� ��Ͽ� �߰�
        CString strIPAddress;
        UINT uPortNumber = 0;
        pClient->GetPeerName(strIPAddress, uPortNumber);

        CString clientInfo;
        clientInfo.Format(_T("%s:%d"), strIPAddress, uPortNumber);

        // ���� ���̾�α׿� ����� �߰�
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
    std::lock_guard<std::mutex> lock(clientListMutex);  // Ŭ���̾�Ʈ ����Ʈ ���� ��ȣ

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
        CSocketServerDlg::pMainInstance->clientList->ResetContent(); // ���� ��� �ʱ�ȭ

        for (const auto& client : m_ClientList) {
            CString strIPAddress;
            UINT uPortNumber = 0;
            client->GetPeerName(strIPAddress, uPortNumber);

            CString clientKey;
            clientKey.Format(_T("%s:%d"), strIPAddress, uPortNumber);

            CString displayText;
            if (aliasMap.find(clientKey) != aliasMap.end()) {
                // ������ �ִ� ��� ���� ǥ��
                displayText.Format(_T("[%s - %s]"), strIPAddress, aliasMap[clientKey]);
            }
            else {
                // ������ ���� ��� �⺻ IP:Port�� ǥ��
                displayText.Format(_T("[%s:%d]"), strIPAddress, uPortNumber);
            }

            CSocketServerDlg::pMainInstance->clientList->AddString(displayText);
        }
    }
}

