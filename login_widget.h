#ifndef LOGIN_WIDGET_H
#define LOGIN_WIDGET_H

#include <QWidget>
#include"dbdal.h"
#include<QString>
#include<QMessageBox>
#include"homewindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Login_Widget; }
QT_END_NAMESPACE

class Login_Widget : public QWidget
{
    Q_OBJECT

public:
    Login_Widget(QWidget *parent = nullptr);
    ~Login_Widget();

private slots:
    void on_login_pushButton_clicked();
    void deal_from_home();
private:
    Ui::Login_Widget *ui;
 DBDAL dbhelper;
    HomeWindow* homewindow;
    QString user;
};
#endif // LOGIN_WIDGET_H
