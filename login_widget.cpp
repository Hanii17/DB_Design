#include "login_widget.h"
#include "ui_login_widget.h"
#include<QDebug>
#include<QSqlQuery>
#include<windows.h>
Login_Widget::Login_Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Login_Widget)
{
    ui->setupUi(this);
    ui->pswd_lineEdit_2->setEchoMode(QLineEdit::Password);
    homewindow=new HomeWindow(&dbhelper);
   // Sleep(2000);
    connect(homewindow,SIGNAL(back_from_home()),this,SLOT(deal_from_home()));
}

Login_Widget::~Login_Widget()
{
    delete ui;
    delete homewindow;
}




void Login_Widget::on_login_pushButton_clicked()
{
    if(ui->ID_lineEdit->text().isEmpty())
    {
        QMessageBox::warning(this,"Warning","收银员编号不允许为空！");
        return;
    }
    if(ui->pswd_lineEdit_2->text().isEmpty())
    {
        QMessageBox::warning(this,"Warning","请输入密码！");
        return;
    }
      if(!dbhelper.connectDatabase())
      {qDebug()<<"数据库连接失败";
          return;}
      QSqlQuery query;

      //准备SQL语句
      query.prepare("select StPswd from Staff where Stno=:id");
      QString id=this->ui->ID_lineEdit->text();
      query.bindValue(":id",id);

      //执行查询语句
      if(!query.exec())
      {
          QMessageBox::critical(this,"Error","登录失败！系统错误！");
          return;
      }


    int initialpos= query.at();

      int size;
      if(query.last())
      {
          size=query.at()+1;
      }
      else
      {
          size=0;
      }
      query.seek(initialpos);
      qDebug()<<"大小"<<size;
      if(size==0)
      {
          QMessageBox::warning(this,"Warning","不存在的收银员编号!");
          dbhelper.disconnectDatabase();
          return;
      }
      QString password;
      while (query.next()) {

          password=query.value(0).toString();
          qDebug()<<password;

      }
      dbhelper.disconnectDatabase();

     qDebug()<<password.size();
      if(password!=this->ui->pswd_lineEdit_2->text())
      {
          QMessageBox::critical(this,"Error","密码错误!");
          return;
      }
      user=this->ui->ID_lineEdit->text();
      homewindow->setusr(user);

      QMessageBox::information(this,"Success","登录成功!");
      this->hide();
      homewindow->show();

}

void Login_Widget::deal_from_home()
{
     homewindow->hide();
     this->show();
}
