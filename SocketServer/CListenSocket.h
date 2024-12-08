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

    // ����� Ŭ���̾�Ʈ ���� ����Ʈ
    std::list<CClientSocket*> m_ClientList;

    // ����� Ŭ���̾�Ʈ ���� ��ο��� �޽��� ����
    void SendAllClients(const CString& message);
    void CloseClientSocket(CClientSocket* pClient);

protected:
    virtual void OnAccept(int nErrorCode);
    
};