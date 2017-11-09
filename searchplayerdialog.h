#ifndef SEARCHPLAYERDIALOG_H
#define SEARCHPLAYERDIALOG_H

#include <QDialog>

namespace Ui {
class searchPlayerDialog;
}

class searchPlayerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit searchPlayerDialog(QWidget *parent = 0);
    ~searchPlayerDialog();
    void reset();
    void addID(int id,QString name);
    void enableBtn(bool enabled);

signals:
    void getIDToConnect(int oppid);

private slots:
    void on_pushButton_clicked();

private:
    Ui::searchPlayerDialog *ui;

};
void resetGrid(QWidget *widget,double factorx,double factory);
#endif // SEARCHPLAYERDIALOG_H
