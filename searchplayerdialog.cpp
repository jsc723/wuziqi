#include "searchplayerdialog.h"
#include "ui_searchplayerdialog.h"
#include <QDebug>
#include <QMessageBox>
#include <QDesktopWidget>
searchPlayerDialog::searchPlayerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::searchPlayerDialog)
{
    ui->setupUi(this);
    double fac = QApplication::desktop()->height()/1080.0;
    resetGrid(ui->label,fac,fac);
    resetGrid(ui->pushButton,fac,fac);
    resetGrid(ui->tableWidget,fac,fac);
    resize(width()*fac,height()*fac);

    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setColumnWidth(0,ui->tableWidget->width()/2-1);
    ui->tableWidget->setColumnWidth(1,ui->tableWidget->width()/2-1);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->horizontalHeader()->setVisible(false);

    setWindowTitle(tr("联机对战"));
}

searchPlayerDialog::~searchPlayerDialog()
{
    delete ui;
}

void searchPlayerDialog::on_pushButton_clicked()
{
    for(int i=0;i<ui->tableWidget->rowCount();i++)
    {
        if(ui->tableWidget->item(i,0)->isSelected())
        {
            int objid = ui->tableWidget->item(0,i)->text().toInt();
            ui->pushButton->setEnabled(false);
            emit getIDToConnect(ui->tableWidget->item(0,i)->text().toInt());
            return;
        }
    }
    QMessageBox::information(this," ",tr("请选择一位玩家"),QMessageBox::Ok);
}

void searchPlayerDialog::reset()
{
    ui->pushButton->setEnabled(false);
    while(ui->tableWidget->rowCount()!=0)
        ui->tableWidget->removeRow(0);
}

void searchPlayerDialog::addID(int id,QString name)
{
    qDebug("addID for id %d",id);
    ui->tableWidget->insertRow(0);
    ui->tableWidget->setItem(0,0,new QTableWidgetItem(QString::number(id)));
    ui->tableWidget->setItem(0,1,new QTableWidgetItem(name));
    ui->pushButton->setEnabled(true);
}

void searchPlayerDialog::enableBtn(bool enabled)
{
    ui->pushButton->setEnabled(enabled);
}
