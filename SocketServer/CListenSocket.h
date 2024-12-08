#pragma once
#include <afxsock.h>
#include <list>
#include <map>
class CClientSocket;

class CListenSocket : public CAsyncSocket
{
public:
    CListenSocket();
    virtual ~CListenSocket();
    std::map<CString, CString> aliasMap;
    void UpdateUI();

    // 연결된 클라이언트 소켓 리스트
    std::list<CClientSocket*> m_ClientList;

    // 연결된 클라이언트 소켓 모두에게 메시지 전송
    void SendAllClients(const CString& message);
    void CloseClientSocket(CClientSocket* pClient);

protected:
    virtual void OnAccept(int nErrorCode);
    
};