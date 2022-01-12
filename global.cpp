#include "global.h"
#include <QDebug>
Global global(NULL);

void initialization() {
    //初始化套接字库
    WORD w_req = MAKEWORD(2, 2);//版本号
    WSADATA wsadata;
    int err;
    err = WSAStartup(w_req, &wsadata);
    if (err != 0) {
        qDebug() << "WSA init failed";
    }
    //检测版本号
    if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
        qDebug() << "WSA bad version";
        WSACleanup();
    }
}


Global::Global(QObject *parent): QObject(parent), cmd_queue()
{
    initialization();
    socketThread = new SocketThread();
    socketThread->start();
}

void SocketThread::run()
{
    qDebug() << "run()";
    while(true) {
        QString msg = socket->recvMsg(10);
        if(msg.size()) {
            qDebug() << msg;
            if(msg == "hello") {
                emit hello();
            }
            continue;
        }
        if(global.cmd_queue.size()) {
            WZQMessage cmd = global.cmd_queue.pop();
            int res;
            if((res = socket->sendMsg(QString(cmd.serialize().c_str()), 10) < 0)) {
                qDebug() << "sendMsg error: " << res;
            }
            continue;
        }
        sleep(1);
    }
}


