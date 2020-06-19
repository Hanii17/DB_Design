#ifndef HOMEWINDOW_H
#define HOMEWINDOW_H

#include <QMainWindow>
#include<QString>
#include<QTimer>
#include<QDateTime>
#include<QMessageBox>
#include"dbdal.h"
#include<QInputDialog>
#include"insert_dialog.h"
namespace Ui {
class HomeWindow;
}
struct guests{
    QString name;
    QString phnumber;
    QString cardID;
};

class HomeWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit HomeWindow(DBDAL *dbh,QString id="",QWidget *parent = nullptr);
    ~HomeWindow();
    void setusr(QString st);
public slots:
    void timeupdate();
    void dealBackSig();
private slots:
    void on_pushButton_3_clicked();

    void on_pushButton_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_2_clicked();

    void on_search_pushButton_6_clicked();

    void on_drop_pushButton_7_clicked();

    void on_up_atno_pushButton_5_clicked();

    void on_up_no_pushButton_8_clicked();

    void on_up_name_pushButton_9_clicked();

    void on_up_amout_pushButton_10_clicked();

    void on_up_price_pushButton_11_clicked();

    void on_insert_pushButton_6_clicked();

private:
    Ui::HomeWindow *ui;
    QString usr;
    QTimer *timer;
    bool if_guest;
    bool is_vip;
    bool is_single_good;
   guests guest;
   DBDAL *dbhelper;
   int count;
   float cashamount;
   insert_Dialog *insert_dialog;

};

#endif // HOMEWINDOW_H
