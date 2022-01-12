#include "websocket.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <QtEndian>
#include <errno.h>
using std::string;


WebSocket::WebSocket()
{
    //服务端地址客户端地址
    SOCKADDR_IN server_addr;
    //填充服务端信息
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.S_un.S_addr = inet_addr("192.168.1.201");
    server_addr.sin_port = htons(8888);
    //创建套接字
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(m_socket, (SOCKADDR *)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
        qDebug() << "cannot connect to server!";
        WSACleanup();
    }
}

/*
* readTimeout - 读超时检测函数，不含读操作
* @wait_mseconds: 等待超时微秒数，如果为0表示不检测超时
* 成功（未超时）返回0，失败返回-1，超时返回-1并且errno = ETIMEDOUT
*/
int WebSocket::readTimeout(unsigned int wait_mseconds)
{
    int ret = 0;
    fd_set read_fdset;

    FD_ZERO(&read_fdset);
    FD_SET(m_socket, &read_fdset);
    if (wait_mseconds > 0)
    {
        struct timeval timeout;

        timeout.tv_sec = wait_mseconds / 1000;
        timeout.tv_usec = wait_mseconds % 1000 * 1000;

        //select返回值三态
        //1 若timeout时间到（超时），没有检测到读事件 ret返回=0
        //2 若ret返回<0 &&  errno == EINTR 说明select的过程中被别的信号中断（可中断睡眠原理）
        //2-1 若返回-1，select出错
        //3 若ret返回值>0 表示有read事件发生，返回事件发生的个数
        do
        {
            ret = select(m_socket + 1, &read_fdset, NULL, NULL, &timeout);

        } while (ret < 0 && errno == EINTR);

        if (ret == 0)
        {
            errno = ETIMEDOUT;
            return -1;
        }
        else if (ret == 1)
        {
            return 0;
        }
    } else if (wait_mseconds < 0) {
        do
        {
            ret = select(m_socket + 1, &read_fdset, NULL, NULL, NULL);

        } while (ret < 0 && errno == EINTR);
        return 0;

    }
    return ret;
}

/*
* writeTimeout - 写超时检测函数，不含写操作
* @wait_mseconds: 等待超时微秒数，如果为0表示不检测超时
* 成功（未超时）返回0，失败返回-1，超时返回-1并且errno = ETIMEDOUT
*/
int WebSocket::writeTimeout(unsigned int wait_mseconds)
{
    int ret = 0;
    fd_set write_fdset;
    FD_ZERO(&write_fdset);
    FD_SET(m_socket, &write_fdset);
    if (wait_mseconds > 0)
    {
        struct timeval timeout;

        timeout.tv_sec = wait_mseconds / 1000;
        timeout.tv_usec = wait_mseconds % 1000 * 1000;
        do
        {
            ret = select(m_socket + 1, NULL, &write_fdset, NULL, &timeout);
        } while (ret < 0 && errno == EINTR);

        // 超时
        if (ret == 0)
        {
            errno = ETIMEDOUT;
            return -1;
        }
        else if (ret == 1)
        {
            return 0;	// 没超时
        }
    }else if (wait_mseconds < 0) {
        do
        {
            ret = select(m_socket + 1, NULL, &write_fdset, NULL, NULL);

        } while (ret < 0 && errno == EINTR);
        return 0;
    }

    return ret;
}

int WebSocket::sendMsg(QString sendData, int timeout)
{
    int ret = writeTimeout(timeout);
    if (ret == -1 && errno == ETIMEDOUT) {
        return TimeoutError;
    }
    string data = sendData.toStdString();
    int dataLen = data.size() + 4;
    // 添加的4字节作为数据头, 存储数据块长度
    unsigned char *netdata = (unsigned char *)malloc(dataLen);
    if (netdata == NULL)
    {
        printf("func sendMsg() mlloc Err\n ");
        return MallocError;
    }
    // 转换为网络字节序

    int netlen = qToBigEndian<int>(data.size());
    memcpy(netdata, &netlen, 4);
    memcpy(netdata + 4, data.c_str(), data.size());

    // 没问题返回发送的实际字节数, 应该 == 第二个参数: dataLen
    // 失败返回: -1
    ret = writen(netdata, dataLen);
    if (netdata != NULL)
    {
        free(netdata);
        netdata = NULL;
    }
    return ret;
}

QString WebSocket::recvMsg(int timeout)
{
    // 返回0 -> 没超时就接收到了数据, -1, 超时或有异常
    int ret = readTimeout(timeout);
    if (ret != 0)
    {
        if (ret == -1 && errno == ETIMEDOUT)
        {
            qDebug() << "readTimeout(timeout): Timeout";
            return QString();
        }
        else
        {
             qDebug() << "readTimeout(timeout): Error" << ret;
            return QString();
        }
    }
    int netdatalen = 0;
    ret = readn(&netdatalen, 4); //读包头 4个字节
    if (ret == -1)
    {
        printf("func readn() err:%d \n", ret);
        return QString();
    }
    else if (ret < 4)
    {
        printf("peer closed:%d \n", ret);
        return QString();
    }

    int n = qFromBigEndian<int>(netdatalen);
    // 根据包头中记录的数据大小申请内存, 接收数据
    char* tmpBuf = (char *)malloc(n + 1);
    if (tmpBuf == NULL)
    {
        printf("malloc() err \n");
        return QString();
    }

    ret = readn(tmpBuf, n); //根据长度读数据
    if (ret == -1)
    {
        printf("func readn() err:%d \n", ret);
        return QString();
    }
    else if (ret < n)
    {
        printf("func readn() err peer closed:%d \n", ret);
        return QString();
    }

    tmpBuf[n] = '\0'; //多分配一个字节内容，兼容可见字符串 字符串的真实长度仍然为n
    QString data = QString(tmpBuf);
    // 释放内存
    free(tmpBuf);

    return data;
}

// 每次从缓冲区中读取n个字符
int WebSocket::readn(const void *buf, int count)
{
    size_t nleft = count;
    ssize_t nread;
    char *bufp = (char*)buf;

    while (nleft > 0)
    {
        if ((nread = recv(m_socket, bufp, nleft, 0)) < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            return -1;
        }
        else if (nread == 0)
        {
            return count - nleft;
        }

        bufp += nread;
        nleft -= nread;
    }

    return count;
}

// 每次往缓冲区写入n个字符
int WebSocket::writen(const void *buf, int count)
{
    size_t nleft = count;
        ssize_t nwritten;
        char *bufp = (char*)buf;

        while (nleft > 0)
        {
            if ((nwritten = send(m_socket, bufp, nleft, 0)) < 0)
            {
                if (errno == EINTR)	// 被信号打断
                {
                    continue;
                }
                return -1;
            }
            else if (nwritten == 0)
            {
                continue;
            }

            bufp += nwritten;
            nleft -= nwritten;
        }

        return count;
}

void WebSocket::disConnect()
{
    closesocket(m_socket);
}

