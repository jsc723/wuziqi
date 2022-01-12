#include "signupdialog.h"
#include "ui_signupdialog.h"
#include <QMessageBox>
#include <QTcpSocket>
#include "global.h"

SignUpDialog::SignUpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SignUpDialog),
    socket(new QTcpSocket())
{
    ui->setupUi(this);
    startTimer(1000);
    timerEvent(NULL);
    connect(global.socketThread, SIGNAL(hello()),this,SLOT(slot_hello()));
    //connect(global.socket,SIGNAL(readyRead()),this,SLOT(read_data()));
}

SignUpDialog::~SignUpDialog()
{
    delete ui;
}

void SignUpDialog::on_buttonBox_accepted()
{
    QString userName = ui->textEditUser->toPlainText();
    QString password = ui->textEditPassword->toPlainText();
    QString password_r = ui->textEditPasswordR->toPlainText();
    WZQMessage msg;
    msg.op = WZQMessage::SIGNUP;
    msg.s1 = userName.toStdString();
    msg.s2 = password.toStdString();
    global.cmd_queue.push(msg);
}
void SignUpDialog::timerEvent(QTimerEvent *event)
{
//    qDebug() << (int)global.socket->state() << "\n";
//    if(global.socket->state() == global.socket->ConnectedState) {
//        ui->label_info->setText(tr("已连接"));
//    } else {
//        ui->label_info->setText(tr("未连接"));
//    }
//    if(global.socket->waitForReadyRead(10)) {
//        while(global.socket->bytesAvailable()) {
//            read_data();
//        }
//    }
}
void SignUpDialog::read_data()
{
//    QString msg = global.socket->readAll();
//    qDebug() << msg;
}

void SignUpDialog::slot_hello()
{
    qDebug() << "received hello";
}

