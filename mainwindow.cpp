#include "mainwindow.h"
#include "board.h"
#include "ui_mainwindow.h"
#include "signupdialog.h"
#include <ctime>
#include <cstdlib>
#include <QThread>
#include <QTextCodec>
#include <QLabel>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QMessageBox>
#include <QScrollBar>
#include <QPalette>
#include <QUdpSocket>
#include <QDesktopWidget>
#include <QTimer>
#include <QTime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    udpSocket(new QUdpSocket(this)),
    searchDlg(new searchPlayerDialog(this)),
    signupDlg(new SignUpDialog(this)),
    statusLabel(new QLabel(this)),
    port(45654),
    busy(false),
    twoPlayer(false),
    finished(false),
    isConnected(false),
    bindOppid(-1),
    bindOppName(),
    sizeNow(Normal),
    title(),
    myScore(0),
    oppScore(0),
    board(),
    work(NULL)
{
    ui->setupUi(this);
    timer = new QTimer(this);

    fac = QApplication::desktop()->height()/1080.0;

    udpSocket->bind(port,QUdpSocket::ShareAddress|QUdpSocket::ReuseAddressHint);
    connect(udpSocket,SIGNAL(readyRead()),this,SLOT(processPendingDatagrams()));

    srand((unsigned)time(0));
    id = rand()%1000;
    setSize(sizeNow);
    ui->sizeButton->setText(sizeNow==Extend?tr("<<"):tr(">>"));
    QPalette pal = ui->textBrowser->palette();
    pal.setColor(QPalette::Base,QColor(200,200,200,100));
    ui->textBrowser->setPalette(pal);

    title = tr("连珠高手 4.0 - ID%1").arg(id);
    setWindowTitle(title);
    ui->textBrowser->setCurrentFont(QFont("Time New Roman",11));

    ui->statusBar->addWidget(statusLabel);
    //ui->menuBar->addMenu(tr("File"));

    connect(searchDlg,SIGNAL(getIDToConnect(int)),this,SLOT(sendInvition(int)));
    connect(ui->userNameLineEdit,SIGNAL(textChanged(QString)),this,SLOT(setTitle(QString)));

    connect(timer, SIGNAL(timeout()),this,SLOT(showReport()));
    timer->start(500);

    initializeBoard();
}

void resetGrid(QWidget *widget,double factorx,double factory)
{
    int widgetX = widget->x();
    int widgetY = widget->y();
    int widgetWid = widget->width();
    int widgetHei = widget->height();
    int nWidgetX = (int)(widgetX*factorx);
    int nWidgetY = (int)(widgetY*factory);
    int nWidgetWid = (int)(widgetWid*factorx);
    int nWidgetHei = (int)(widgetHei*factory);
    widget->setGeometry(nWidgetX,nWidgetY,nWidgetWid,nWidgetHei);
}
void MainWindow::showReport()
{
    if(busy && !twoPlayer && !isConnected) {
        char c = board.toColOnBoard(board.nowWorkingOn.x);
        int r = board.toRowOnBoard(board.nowWorkingOn.y);
        int per = (15*board.nowWorkingOn.x + board.nowWorkingOn.y)*100/255;
        statusLabel->setText(tr("电脑思考中,正在思考%1%2 (%3\%)").arg(c).arg(r).arg(per));
    }
    timer->start(500);
}

void MainWindow::initializeBoard()
{
    resetGrid(ui->textBrowser,fac,fac);
    resetGrid(ui->textEdit,fac,fac);
    resetGrid(ui->userNameLineEdit,fac,fac);
    resetGrid(ui->label,fac,fac);
    resetGrid(ui->sendButton,fac,fac);
    resetGrid(ui->sizeButton,fac,fac);
    resetGrid(ui->netLabel,fac,fac);
    nowBoard = QPixmap(740*fac,760*fac);
    nowBoard.fill(Qt::white);
    emptyBoard = QPixmap(620*fac,620*fac);
    emptyBoard.fill(QColor(0,200,100).light(130));
    QPainter painter(&emptyBoard);
    for(int i=0;i<15;i++)
    {
        QString str = "ABCDEFGHIJKLMNO";
        painter.drawText(QPoint((i*40+15)*fac,610*fac),tr("%1").arg(str[i]));
        painter.drawText(QPoint(590*fac,(i*40+25)*fac),tr("%1").arg(15-i));

        painter.drawLine(20*fac,(i*40+20)*fac,580*fac,(i*40+20)*fac);
        painter.drawLine((i*40+20)*fac,20*fac,(i*40+20)*fac,580*fac);
    }
    black = QPixmap(40*fac,40*fac);
    white = QPixmap(40*fac,40*fac);
    black.fill(Qt::transparent);
    white.fill(Qt::transparent);
    QPainter bp(&black),wp(&white);
    bp.setBrush(Qt::black);
    wp.setBrush(Qt::white);
    bp.drawEllipse(QPoint(20*fac,20*fac),17*fac,17*fac);
    wp.drawEllipse(QPoint(20*fac,20*fac),17*fac,17*fac);

    flag = QPixmap(30*fac,30*fac);
    flag.fill(Qt::transparent);
    QPainter fp(&flag);
    fp.setBrush(Qt::red);
    fp.drawEllipse(QPoint(7*fac,7*fac),7*fac,7*fac);

    board.first  = USR;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    switch( QMessageBox::information( this, tr(" "),tr("退出游戏？"), tr("Yes"), tr("No"),0, 1 ) )
    {
    case 0:
        event->accept();
        break;
    case 1:
    default:
        event->ignore();
        break;
    }
    if(isConnected)on_actionDisconnect_triggered();
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this),tempPainter(&nowBoard);
    qreal x,y,w,h;
    tempPainter.translate(XX*fac,YY*fac);
    tempPainter.drawPixmap(0,0,emptyBoard);
    for(int i=0;i<board.count;i++)
    {
        tempPainter.drawPixmap(board.steps[i].x*40*fac,board.steps[i].y*40*fac,(i%2)?white:black);
        if(i<board.count-1)
        {
            x = (board.steps[i].x*40.0+12.0)*fac;
            y = (board.steps[i].y*40.0+15.0)*fac;
            w = 18*fac;
            h = 12*fac;
            tempPainter.setPen((i%2)?Qt::black:Qt::white);
            tempPainter.drawText(x,y,w,h,Qt::AlignCenter,tr("%1").arg(i+1));
        }
    }
    tempPainter.setPen(Qt::black);
    if(board.count>0)
        tempPainter.drawPixmap((board.steps[board.count-1].x*40+13)*fac
                ,(board.steps[board.count-1].y*40+14)*fac,flag);
    painter.drawPixmap(0,0,nowBoard);

}
void MainWindow::refresh()
{
    if(board.winner()!=0||board.count==225)
    {
        printResult();
        ui->mainToolBar->actions()[1]->setEnabled(false);
        busy = false;
        return;
    }
    busy = false;
    statusLabel->setText(tr(""));
    delete work;
    work = NULL;
    repaint();
    //board.print();
}



void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(finished||busy)
    {
        event->ignore();
        return;
    }
    if(event->button()==Qt::RightButton&&!isConnected)
    {
        on_actionHuiqi_triggered();
        return;
    }
    if(event->button()!=Qt::LeftButton)
        return;


    double xx = (event->x()-XX*fac)/(40.0*fac);
    double yy = (event->y()-YY*fac)/(40.0*fac);
    if(!inRange(xx,yy)) {busy=false;return;}
    int x = xx;
    int y = yy;
    busy = true;
    int player = board.nextPlayer(USR);
    if(!board.putAndUpdateNeighbor(x,y,player))
    {
        event->ignore();
        busy = false;
        return;
    }

    board.steps[board.count-1] = Point(x,y);
    if(twoPlayer)
        statusLabel->setText(player==USR?tr("轮到白下"):tr("轮到黑下"));
    else if(isConnected)
        statusLabel->setText(tr("等待对方下子"));
    else
        statusLabel->setText(tr("电脑思考中"));
    ui->mainToolBar->actions()[2]->setEnabled(false);
    ui->mainToolBar->actions()[3]->setEnabled(false);

    repaint();

    if(board.winner()!=0||board.count==225)
    {
        ui->mainToolBar->actions()[1]->setEnabled(false);
        busy = false;
        statusLabel->setText(tr(""));
        sendMessage(Move,x,y,player);
        printResult();
        return;
    }

    if(twoPlayer)
    {
        busy = false;
        return;
    }
    if(isConnected)
    {
        sendMessage(Move,x,y,player);
        busy = true;
        return;
    }
    repaint();
    work = new WorkerThread(&board,this);
    connect(work, SIGNAL(done()), this, SLOT(refresh()));
    work->start();

    /*
    if(board.winner()!=0||board.count==225)
    {
        printResult();
        ui->toolButton->setEnabled(false);
        ui->mainToolBar->actions()[1]->setEnabled(false);
        busy = false;
        return;
    }
    busy = false;
    statusLabel->setText(tr(""));
*/
}

/*----------------------------------------printResult------------------------------------------------------*/
//print the result of the game
void MainWindow::printResult()
{
    finished = true;
    statusLabel->setText(tr(" "));
    if(twoPlayer)
    {
        if(board.winner()==USR)
            QMessageBox::warning(this,tr(" "),tr("黑棋赢了"),QMessageBox::Ok);
        else if(board.winner()==COM)
            QMessageBox::warning(this,tr(""),tr("白棋赢了"),QMessageBox::Ok);
        else
            QMessageBox::warning(this,tr(" "),tr("平局"),QMessageBox::Ok);
    }
    else if(isConnected)
    {
        if(board.winner()==USR)
        {
            QMessageBox::warning(this,tr(" "),tr("你赢了！"),QMessageBox::Ok);
            myScore++;
        }
        else if(board.winner()==COM)
        {
            QMessageBox::warning(this,tr(" "),tr("你输了"),QMessageBox::Ok);
            oppScore++;
        }
        else
            QMessageBox::warning(this,tr(" "),tr("平局"),QMessageBox::Ok);
        QString p1 = tr("ID%1").arg(id),p2=tr("ID%1").arg(bindOppid);
        if(ui->userNameLineEdit->text()!="") p1 = ui->userNameLineEdit->text();
        if(bindOppName!="") p2 = bindOppName;
        ui->textBrowser->append(tr("比分<%1:%2>=%3:%4\n").arg(p1).arg(p2).arg(myScore).arg(oppScore));
    }
    else
    {
        if(board.winner()==USR)
            QMessageBox::warning(this,tr(" "),tr("你赢了！"),QMessageBox::Ok);
        else if(board.winner()==COM)
            QMessageBox::warning(this,tr(" "),tr("电脑赢了！"),QMessageBox::Ok);
        else
            QMessageBox::warning(this,tr(" "),tr("平局"),QMessageBox::Ok);
    }
    return;
}

void MainWindow::setSize(WindowSizeType size)
{
    sizeNow = size;
    if(size==Normal)
    {
        setMaximumSize(740*fac,760*fac);
        setMinimumSize(740*fac,760*fac);
        resize(740*fac,760*fac);
        ui->textBrowser->hide();
        ui->textEdit->hide();
        ui->sendButton->hide();
    }
    else if(size==Extend)
    {
        setMaximumSize(1040*fac,760*fac);
        setMinimumSize(1040*fac,760*fac);
        resize(1040*fac,760*fac);
        ui->textBrowser->show();
        ui->textEdit->show();
        ui->sendButton->show();
    }
    update();
}

void MainWindow::appendMsg(QString msg, int senderId, QString usrName)
{
    QString time = QDateTime::currentDateTime().toString("hh:mm");
    ui->textBrowser->verticalScrollBar()->setValue(ui->textBrowser->verticalScrollBar()->maximum());
    ui->textBrowser->append("["+usrName+ " ID"+QString::number(senderId)+" "+time+"]");
    ui->textBrowser->append(msg);
}
/*----------------------------------------paintEvent-------------------------------------------------------*/


void MainWindow::on_actionNew_triggered(bool forceful = false)
{
    ui->mainToolBar->actions()[1]->setEnabled(true);
    statusLabel->setText(tr(""));
    if(!forceful){
        if(QMessageBox::question(this,tr("新游戏"),tr("开始新游戏？"),
                                 QMessageBox::Yes,QMessageBox::No)==QMessageBox::No)
            return;
    }
    if(isConnected&&!forceful)
    {
        sendMessage(WantNewGame);
        return;
    }
    ui->mainToolBar->actions()[2]->setEnabled(true);
    ui->mainToolBar->actions()[3]->setEnabled(true);
    board.reset();
    finished = false;
    busy = false;
    if(board.first==COM&&!isConnected)
    {
        board.compInput();      /*Computer's turn*/
    }
    update();
}

void MainWindow::on_actionHuiqi_triggered()
{
    if(board.count<=0)
    {
        QMessageBox::warning(this,tr("警告"),tr("不能再悔棋了"),QMessageBox::Ok);
        return;
    }
    if(isConnected)
    {
        sendMessage(WantHuiqi);
        return;
    }
    board.huiqi(twoPlayer);
    update();
}

void MainWindow::on_actionGame_triggered()
{
    if(board.first==USR&&!twoPlayer)
    {
        ui->mainToolBar->actions()[2]->setText(tr("电脑先下"));
        if(board.dif)ui->mainToolBar->actions()[3]->setText(tr("困难模式"));
        else ui->mainToolBar->actions()[3]->setText(tr("简单模式"));
        ui->mainToolBar->actions()[3]->setEnabled(true);
        board.first = COM;
        board.compInput();      /*Computer's turn*/
    }
    else if(board.first==COM)
    {
        ui->mainToolBar->actions()[2]->setText(tr("双人游戏"));
        ui->mainToolBar->actions()[3]->setEnabled(false);
        board.first = USR;
        twoPlayer = true;
        board.reset();
    }
    else
    {
        ui->mainToolBar->actions()[2]->setText(tr("玩家先下"));
        if(board.dif)ui->mainToolBar->actions()[3]->setText(tr("困难模式"));
        else ui->mainToolBar->actions()[3]->setText(tr("简单模式"));
        ui->mainToolBar->actions()[3]->setEnabled(true);
        twoPlayer = false;
        board.first = USR;
        board.reset();
    }
    update();
}

void MainWindow::on_actionDiff_triggered()
{
    board.dif = !board.dif;
    if(board.dif)ui->mainToolBar->actions()[3]->setText(tr("困难模式"));
    else ui->mainToolBar->actions()[3]->setText(tr("简单模式"));
}

void MainWindow::on_actionConnect_triggered()
{
    sendMessage(SearchPlayer);
    searchDlg->reset();
    searchDlg->exec();
}

void MainWindow::setTitle(QString name)
{
    setWindowTitle(title+" "+name);
}

void MainWindow::processPendingDatagrams()
{
    while(udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        int oppid,objid;
        int allowh;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(),datagram.size());
        QDataStream in(&datagram,QIODevice::ReadOnly);
        QString msg,usrName,usrName2="";
        int messageType,oppx=2,oppy=2,player;
        in >> messageType >> oppid>>usrName;
        if(oppid==id) return;
        if(usrName!="") usrName2 = "("+usrName+")";
        switch(messageType)
        {
        case SearchPlayer:
            if(!isConnected) sendMessage(AbleToConnect);
            break;
        case AbleToConnect:
            processAbleConnect(oppid,usrName);
            break;
        case WantNewConnection:
            in >> objid;
            qDebug("id%d Receive want from id%d with objid%d",id,oppid,objid);
            if(objid!=id) return;
            allowh = QMessageBox::question(this,tr("联机请求"),tr("ID%1%2想要与你联机，是否同意？").arg(oppid).arg(usrName2)\
                                           ,QMessageBox::Yes,QMessageBox::No);
            if(allowh==QMessageBox::Yes)
            {
                sendMessage(AcceptNewConnection);
                processNewConnection(oppid,usrName);
            }
            else
                sendMessage(RefuseNewConnection);
            break;
        case AcceptNewConnection:
            if(checkBind(oppid)) return;
            searchDlg->close();
            processNewConnection(oppid,usrName);
            break;
        case RefuseNewConnection:
            if(checkBind(oppid)) return;
            bindOppid = -1;
            bindOppName = "";
            QMessageBox::information(this,tr("请求被拒绝"),tr("ID%1%2拒绝与你联机").arg(oppid).arg(usrName2),QMessageBox::Ok);
            searchDlg->enableBtn(true);
            break;

        case Disconnect:
            if(checkBind(oppid)) return;
            QMessageBox::information(this,tr("连接断开"),tr("ID%1%2断开了与你的连接").arg(oppid).arg(usrName2),QMessageBox::Ok);
            processDisconnect();
            break;
        case Move:
            if(checkBind(oppid)) return;
            in >> oppx >> oppy >> player;
            player = player==USR?COM:USR;
            board.put(oppx,oppy,player);
            board.steps[board.count-1] = Point(oppx,oppy);
            statusLabel->setText(tr("轮到你下"));
            repaint();
            if(oppid!=id) busy = false;
            if(board.winner()!=0||board.count==225)
            {
                printResult();
                //ui->toolButton->setEnabled(false);
                ui->mainToolBar->actions()[1]->setEnabled(false);
                busy = false;
                return;
            }
            break;
        case WantHuiqi:
            if(checkBind(oppid)) return;
            allowh = QMessageBox::question(this,tr(" "),tr("id%1%2请求悔棋，是否同意？").arg(oppid).arg(usrName2)\
                                           ,QMessageBox::Yes,QMessageBox::No);
            if(allowh==QMessageBox::Yes)
            {
                sendMessage(AcceptHuiqi);
                board.huiqi(true);
                busy = !busy;
                update();
            }
            else
                sendMessage(RefuseHuiqi);
            break;
        case AcceptHuiqi:
            if(checkBind(oppid)) return;
            board.huiqi(true);
            busy = !busy;
            update();
            break;
        case RefuseHuiqi:
            if(checkBind(oppid)) return;
            QMessageBox::information(this,tr(" "),tr("对方不同意你悔棋"),QMessageBox::Ok);
            break;
        case WantNewGame:
            if(checkBind(oppid)) return;
            allowh = QMessageBox::question(this,tr(" "),tr("id%1%2要求开新局，是否同意？").arg(oppid).arg(usrName2)\
                                           ,QMessageBox::Yes,QMessageBox::No);
            if(allowh==QMessageBox::Yes)
            {
                sendMessage(AcceptNewGame);
                ui->mainToolBar->actions()[1]->setEnabled(true);
                board.reset();
                finished = false;
                busy = false;
                update();
            }
            else
                sendMessage(RefuseNewGame);
            break;
        case AcceptNewGame:
            if(checkBind(oppid)) return;
            ui->mainToolBar->actions()[1]->setEnabled(true);
            board.reset();
            finished = false;
            busy = false;
            update();
            break;
        case RefuseNewGame:
            if(checkBind(oppid)) return;
            QMessageBox::information(this,tr(" "),tr("对方不同意开新局"),QMessageBox::Ok);
            break;
        case Message:
            in >> msg;
            appendMsg(msg,oppid,usrName);
            break;

        }
    }
}

void MainWindow::processNewConnection(int oppid,QString usrName)
{
    if(isConnected) return;
    isConnected = true;
    bindOppid = oppid;
    bindOppName = usrName;
    on_actionNew_triggered(true);
    ui->netLabel->setText(tr("已连接 id%1 %2").arg(bindOppid).arg(bindOppName));
    ui->mainToolBar->actions()[2]->setEnabled(false);
    ui->mainToolBar->actions()[3]->setEnabled(false);
    ui->mainToolBar->actions()[4]->setEnabled(false);
    ui->mainToolBar->actions()[5]->setEnabled(true);
}

void MainWindow::processDisconnect()
{
    isConnected = false;
    bindOppid = -1;
    bindOppName = "";
    myScore = oppScore = 0;
    ui->mainToolBar->actions()[2]->setEnabled(true);
    ui->mainToolBar->actions()[3]->setEnabled(true);
    ui->mainToolBar->actions()[4]->setEnabled(true);
    ui->mainToolBar->actions()[5]->setEnabled(false);
    on_actionNew_triggered(true);
    ui->netLabel->setText(tr(""));
    statusLabel->setText(tr(""));
}


void MainWindow::processAbleConnect(int oppid, QString oppName)
{
    searchDlg->addID(oppid,oppName);
}

void MainWindow::sendInvition(int oppid)
{
    bindOppid = oppid;
    sendMessage(WantNewConnection);
}


void MainWindow::sendMessage(MessageType type, int x, int y, int player, QString msg)
{
    QByteArray data;
    QDataStream out(&data,QIODevice::WriteOnly);
    QString usrName = ui->userNameLineEdit->text();
    out << type <<id<<usrName;
    switch(type)
    {
    case Message:
        out<<msg<<usrName;
        break;
    case WantNewConnection:
        out<<bindOppid;
    case Move:
        out<<x<<y<<player;
        break;
    default:
        break;
    }
    udpSocket->writeDatagram(data,data.length(),QHostAddress::Broadcast,port);
}

void MainWindow::on_actionDisconnect_triggered()
{
    if(QMessageBox::information( this, tr(" "),tr("断开连接？"), tr("Yes"), tr("No"),0, 1 )==1)
        return;
    processDisconnect();
    sendMessage(Disconnect);
}



void MainWindow::on_sendButton_clicked()
{
    QString msg = ui->textEdit->toPlainText();
    if(msg=="") return;
    ui->textEdit->clear();
    ui->textEdit->setFocus();
    appendMsg(msg,id,ui->userNameLineEdit->text());
    sendMessage(Message,0,0,0,msg);
}

void MainWindow::on_sizeButton_clicked()
{
    if(sizeNow==Extend)
    {
        setSize(Normal);
        ui->sizeButton->setText(tr(">>"));
    }
    else if(sizeNow==Normal)
    {
        setSize(Extend);
        ui->sizeButton->setText(tr("<<"));
    }
}

void MainWindow::on_actionCalculate_triggered()
{
    work = new WorkerThread(&board,this);
    connect(work, SIGNAL(done()), this, SLOT(refresh()));
    work->start();
}

void MainWindow::on_actionsignup_triggered()
{
    signupDlg->exec();
}

