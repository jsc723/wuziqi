#ifndef GLOBAL_H
#define GLOBAL_H
#include <QTcpSocket>
#include <QString>
#include <QThread>
#include "websocket.h"
#include "messagequeue.h"
#include "wzq_msg.h"

class SocketThread;

class Global: public QObject
{
    Q_OBJECT
public:
    Global(QObject *parent);
    virtual ~Global() {};

    QString session;
    SocketThread *socketThread;
    MessageQueue<WZQMessage> cmd_queue;
};
extern Global global;
void initialization();


class SocketThread : public QThread
{
    Q_OBJECT
public:
    SocketThread(QObject *parent = 0)
        : QThread(parent), socket(new WebSocket())
    {
        qDebug() << "socket thread";
    }
signals:
    void hello();
protected:
    void run();
private:
    WebSocket *socket;
};

#endif // GLOBAL_H
