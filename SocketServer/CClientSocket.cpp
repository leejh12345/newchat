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

// IsConnected() 메소드 정의
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

#include <sstream>  // std::wstringstream 사용을 위해

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
                // 별명 설정 처리
                CString aliasName = szBuffer;
                aliasName.Delete(0, 6); // 'alias:' 제거

                if (pServerSocket) {
                    pServerSocket->aliasMap[clientKey] = aliasName; // 별명 저장
                    pServerSocket->UpdateUI(); // UI 갱신
                }

                strTmp.Format(_T("[%s:%d] 별명이 '%s'로 설정되었습니다."), strIPAddress, uPortNumber, aliasName);
            }
            else {
                // 별명이 있는 경우 별명 표시
                if (pServerSocket && pServerSocket->aliasMap.find(clientKey) != pServerSocket->aliasMap.end()) {
                    strTmp.Format(_T("[%s - %s]: %s"), strIPAddress, pServerSocket->aliasMap[clientKey], szBuffer);
                }
                else {
                    // 별명이 없을 경우 기본 표시
                    strTmp.Format(_T("[%s:%d]: %s"), strIPAddress, uPortNumber, szBuffer);
                }
            }

            // 메시지를 리스트박스에 추가
            CSocketServerDlg* pMain = CSocketServerDlg::pMainInstance;
            if (pMain && ::IsWindow(pMain->GetSafeHwnd())) {
                CString* pMessage = new CString(strTmp);
                pMain->PostMessage(WM_USER + 1, (WPARAM)pMessage);
            }

            // 다른 클라이언트로 메시지 브로드캐스트
            if (pServerSocket) {
                pServerSocket->SendAllClients(strTmp);
            }
        }

        CSocket::OnReceive(nErrorCode);
        });

    receiveThread.detach();
}
