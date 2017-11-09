#ifndef GETMSGDIALOG_H
#define GETMSGDIALOG_H

#include <QDialog>

namespace Ui {
class getMsgDialog;
}

class getMsgDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit getMsgDialog(QWidget *parent = 0);
    ~getMsgDialog();
signals:
    void sendMessage(QString msg);
    
private slots:
    void on_cancelBtn_clicked();

    void on_okBtn_clicked();

private:
    Ui::getMsgDialog *ui;
};

#endif // GETMSGDIALOG_H
