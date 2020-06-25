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
#include<QSqlQueryModel>
#include<QSqlQuery>

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
    void on_pushButton_3_clicked();//确定顾客

    void on_pushButton_clicked();//搜索商品并计算价格(应用事务)

    void on_pushButton_4_clicked();

    void on_pushButton_2_clicked();//结算

    void on_search_pushButton_6_clicked();

    void on_drop_pushButton_7_clicked();

    void on_up_atno_pushButton_5_clicked();

    void on_up_no_pushButton_8_clicked();

    void on_up_name_pushButton_9_clicked();

    void on_up_amout_pushButton_10_clicked();

    void on_up_price_pushButton_11_clicked();

    void on_insert_pushButton_6_clicked();

    void on_insert_vip_pushButton_5_clicked();

    void on_serch_vip_pushButton_6_clicked();

    void on_days_pushButton_5_clicked();

    void on_drop_vip_pushButton_8_clicked();

    void on_update_staff_pushButton_5_clicked();

    void on_change_name_pushButton_6_clicked();

    void on_change_sex_pushButton_7_clicked();

    void on_change_pswd_pushButton_8_clicked();

    void on_new_staff_pushButton_9_clicked();

    void on_search_record_pushButton_6_clicked();

    void on_statement_pushButton_5_clicked();

signals:
      void back_from_home();
private:
    Ui::HomeWindow *ui;
    QString usr;
    QTimer *timer;
    bool if_guest;
    bool is_vip;
    bool is_single_good;
    bool if_show_staff;
    bool if_time_fixed;
   guests guest;
   DBDAL *dbhelper;
   int count;
   int count_forVIP;
   int cashamount;
   insert_Dialog *insert_dialog;
   QSqlQueryModel *model;
   QSqlQuery query_search_goods;
   QString fixed_time;

};

#endif // HOMEWINDOW_H
