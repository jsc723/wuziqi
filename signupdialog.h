#ifndef SIGNUPDIALOG_H
#define SIGNUPDIALOG_H

#include <QDialog>
#include <QTcpSocket>

namespace Ui {
class SignUpDialog;
}

class SignUpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SignUpDialog(QWidget *parent = NULL);
    ~SignUpDialog();

private slots:
    void on_buttonBox_accepted();
    void read_data();
    void slot_hello();
    //void connect_status_changed();

private:
    virtual void timerEvent( QTimerEvent *event);
    Ui::SignUpDialog *ui;
    QTcpSocket *socket;

};

#endif // SIGNUPDIALOG_H
