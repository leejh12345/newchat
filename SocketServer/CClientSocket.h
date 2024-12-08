#pragma once

class CClientSocket : public CSocket
{
public:
    CClientSocket();
    virtual ~CClientSocket();

    CAsyncSocket* m_pListenSocket;
    void SetListenSocket(CAsyncSocket* pSocket);
    void OnClose(int nErrorCode);
    void OnReceive(int nErrorCode);

    bool IsConnected() const; // IsConnected() 메소드 추가 선언
};
