#include "stdafx.h"
#include "SocketServer.h"
#include "CClientSocket.h"
#include "CListenSocket.h"
#include "SocketServerDlg.h"
#include <thread>
#include <mutex>
#include <sstream> 

CSocketServerDlg* CSocketServerDlg::pMainInstance = nullptr;

std::mutex receiveMutex;

int index = 0;
CString alias[100][2];

CClientSocket::CClientSocket() : m_pListenSocket(nullptr) {}

CClientSocket::~CClientSocket() {}

// IsConnected() �޼ҵ� ����
bool CClientSocket::IsConnected() const {
    return (m_hSocket != INVALID_SOCKET);
}

void CClientSocket::SetListenSocket(CAsyncSocket* pSocket)
{
    m_pListenSocket = pSocket;
}

void CClientSocket::OnClose(int nErrorCode)
{
    CSocket::OnClose(nErrorCode);

    if (m_pListenSocket != nullptr) {
        CListenSocket* pServerSocket = static_cast<CListenSocket*>(m_pListenSocket);
        pServerSocket->CloseClientSocket(this);
    }
}

#include <sstream>  // std::wstringstream ����� ����

void CClientSocket::OnReceive(int nErrorCode)
{
    std::thread receiveThread([this, nErrorCode]() {
        std::lock_guard<std::mutex> lock(receiveMutex);

        TCHAR szBuffer[1024] = { 0 };
        int nRead = Receive(szBuffer, sizeof(szBuffer) - sizeof(TCHAR));
        if (nRead > 0) {
            szBuffer[nRead / sizeof(TCHAR)] = _T('\0');

            CString strIPAddress;
            UINT uPortNumber = 0;
            GetPeerName(strIPAddress, uPortNumber);

            CString clientKey;
            clientKey.Format(_T("%s:%d"), strIPAddress, uPortNumber);

            CString strTmp;
            CListenSocket* pServerSocket = static_cast<CListenSocket*>(m_pListenSocket);

            if (CString(szBuffer).Find(L"alias:") != -1) {
                // ���� ���� ó��
                CString aliasName = szBuffer;
                aliasName.Delete(0, 6); // 'alias:' ����

                if (pServerSocket) {
                    pServerSocket->aliasMap[clientKey] = aliasName; // ���� ����
                    pServerSocket->UpdateUI(); // UI ����
                }

                strTmp.Format(_T("[%s:%d] ������ '%s'�� �����Ǿ����ϴ�."), strIPAddress, uPortNumber, aliasName);
            }
            else {
                // ������ �ִ� ��� ���� ǥ��
                if (pServerSocket && pServerSocket->aliasMap.find(clientKey) != pServerSocket->aliasMap.end()) {
                    strTmp.Format(_T("[%s - %s]: %s"), strIPAddress, pServerSocket->aliasMap[clientKey], szBuffer);
                }
                else {
                    // ������ ���� ��� �⺻ ǥ��
                    strTmp.Format(_T("[%s:%d]: %s"), strIPAddress, uPortNumber, szBuffer);
                }
            }

            // �޽����� ����Ʈ�ڽ��� �߰�
            CSocketServerDlg* pMain = CSocketServerDlg::pMainInstance;
            if (pMain && ::IsWindow(pMain->GetSafeHwnd())) {
                CString* pMessage = new CString(strTmp);
                pMain->PostMessage(WM_USER + 1, (WPARAM)pMessage);
            }

            // �ٸ� Ŭ���̾�Ʈ�� �޽��� ��ε�ĳ��Ʈ
            if (pServerSocket) {
                pServerSocket->SendAllClients(strTmp);
            }
        }

        CSocket::OnReceive(nErrorCode);
        });

    receiveThread.detach();
}
