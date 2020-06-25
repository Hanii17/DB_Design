#include "insert_dialog.h"
#include "ui_insert_dialog.h"
#include<QMessageBox>
#include<QDebug>
#include<QSqlQuery>
#include<QString>
#include<QDebug>
insert_Dialog::insert_Dialog(DBDAL *dbh,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::insert_Dialog)
{
    ui->setupUi(this);
    dbhelper=dbh;

}

insert_Dialog::~insert_Dialog()
{
    delete ui;
}

//插入商品
void insert_Dialog::on_insert_pushButton_clicked()
{
  if(ui->ATno_lineEdit_5->text().isEmpty()||\
          ui->no_lineEdit->text().isEmpty()||\
          ui->name_lineEdit_2->text().isEmpty()||\
          ui->amount_lineEdit_3->text().isEmpty()||\
          ui->price_lineEdit_4->text().isEmpty())
  {
       QMessageBox::warning(this,"Warning","请填写完整的信息!");
       return;

  }
  else
  {
      if(!dbhelper->connectDatabase())
      {qDebug()<<"数据库连接失败";
          return;}
      QSqlQuery query;
      query.prepare("insert into Commodity\
                    values(?,?,?,?,?)");
     query.bindValue(0,this->ui->ATno_lineEdit_5->text());
     query.bindValue(1,this->ui->no_lineEdit->text());
     query.bindValue(2,this->ui->name_lineEdit_2->text());
      query.bindValue(3,ui->amount_lineEdit_3->text().toInt());
     // query.bindValue(4,ui->price_lineEdit_4->text().toFloat());
       query.bindValue(4,ui->price_lineEdit_4->text().toInt());
       qDebug()<<ui->price_lineEdit_4->text().toInt();

      if(!query.exec())//插入失败
      {
          QMessageBox::critical(this,"Error","商品表信息插入失败");
          dbhelper->disconnectDatabase();
          return;
      }
      dbhelper->disconnectDatabase();
      QMessageBox::information(this,"Success","插入成功!");

  }

}

//返回
void insert_Dialog::on_back_pushButton_2_clicked()
{
   emit backFromInsert();
}
