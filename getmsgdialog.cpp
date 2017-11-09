#include "getmsgdialog.h"
#include "ui_getmsgdialog.h"

getMsgDialog::getMsgDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::getMsgDialog)
{
    ui->setupUi(this);
}

getMsgDialog::~getMsgDialog()
{
    delete ui;
}

void getMsgDialog::on_cancelBtn_clicked()
{
    ui->lineEdit->setText("");
    hide();
}

void getMsgDialog::on_okBtn_clicked()
{
    emit sendMessage(ui->lineEdit->text());
    ui->lineEdit->setText("");
    hide();
}
