#ifndef WEBSOCKET_H
#define WEBSOCKET_H
#include <QTcpSocket>
#include <QString>
#include <winsock2.h>

#define ETIMEDOUT 110

class WebSocket
{
    //friend class SocketThread;

public:
    WebSocket();
    // 发送数据
    int sendMsg(QString sendData, int timeout = -1);
    // 接收数据
    QString recvMsg(int timeout = -1);
    // 断开连接
    void disConnect();

    enum ErrorType {ParamError = 3001, TimeoutError, PeerCloseError, MallocError};

private:
    // 读超时检测函数，不含读操作
    int readTimeout(unsigned int wait_mseconds);
    // 写超时检测函数, 不包含写操作
    int writeTimeout(unsigned int wait_mseconds);
    // 每次从缓冲区中读取n个字符
    int readn(const void *buf, int count);
    // 每次往缓冲区写入n个字符
    int writen(const void *buf, int count);
    SOCKET m_socket;
};

#endif // WEBSOCKET_H
