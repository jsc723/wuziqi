#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>
#include <QThread>
#include <time.h>
#include <QTimer>
#include <fstream>
#include <iostream>

#include "board.h"
#include "getmsgdialog.h"
#include "searchplayerdialog.h"

/* Constants */
#define XX 60
#define YY 80
namespace Ui {
class MainWindow;
}
class QLabel;
class QUdpSocket;

enum MessageType{Message,\
                 SearchPlayer,AbleToConnect,\
                 WantNewConnection,AcceptNewConnection,RefuseNewConnection,\
                 WantHuiqi,RefuseHuiqi,AcceptHuiqi,\
                 Disconnect,Move,\
                 WantNewGame,AcceptNewGame,RefuseNewGame};
enum WindowSizeType{Normal,Extend};
class WorkerThread : public QThread
{
    Q_OBJECT
public:
    WorkerThread(Board *_b, QObject *parent = 0)
        : QThread(parent)
    {
        b = _b;
    }

protected:
    void run()
    {
        b->compInput();
        emit done();
    }
signals:
    void done();
private:
    Board *b;
};
class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    void printResult();

    void setSize(WindowSizeType size);
    void appendMsg(QString msg,int senderId,QString usrName = "");
    ~MainWindow();



    //-----------------------------------//
protected:
    void closeEvent(QCloseEvent *event);
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
private slots:

    void on_actionNew_triggered(bool forceful);

    void on_actionHuiqi_triggered();

    void on_actionGame_triggered();

    void on_actionDiff_triggered();

    void on_actionConnect_triggered();
    void setTitle(QString name);

    void processPendingDatagrams();
    void processNewConnection(int oppid, QString usrName);
    void processDisconnect();
    void processAbleConnect(int oppid,QString oppName);
    void sendInvition(int oppid);

    void sendMessage(MessageType type, int x=0, int y=0, int player = USR, QString msg = "");
    void on_actionDisconnect_triggered();

    void on_sendButton_clicked();

    void on_sizeButton_clicked();
    void refresh();
    void showReport();

    void on_actionLoad_triggered();

    void on_actionSave_triggered();

    void on_actionCalculate_triggered();

private:
    bool checkBind(int senderId){
        return !((bindOppid!=-1)&&(senderId==bindOppid));
    }
    void initializeBoard();
    Ui::MainWindow *ui;
    QPixmap emptyBoard,nowBoard,black,white,flag;
    Board board;
    QLabel *statusLabel;
    searchPlayerDialog *searchDlg;
    QString title;
    bool busy;
    bool twoPlayer;
    bool finished;
    bool isConnected;
    WindowSizeType sizeNow;
    QUdpSocket *udpSocket;
    qint16 port;
    int id;
    int bindOppid;
    QString bindOppName;
    int myScore,oppScore;
    double fac;
    QTimer *timer;
    WorkerThread *work;

};
void resetGrid(QWidget *widget,double factorx,double factory);

#endif // MAINWINDOW_H
