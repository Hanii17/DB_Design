#include "homewindow.h"
#include "ui_homewindow.h"
#include<QDebug>
#include<QSqlQuery>
#include<QSqlQueryModel>

HomeWindow::HomeWindow(DBDAL *dbh,QString id,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::HomeWindow)
{
    usr=id;
    dbhelper=dbh;
    cashamount=0;
    count=0;
    if_guest=false;
    is_vip=false;
    is_single_good=false;
    guest.cardID="0";
    qDebug()<<usr;
    ui->setupUi(this);
    timer=new QTimer();
    this->ui->staff_label->setText(usr);//显示收银员编号

    //设置收银时的表格
    this->ui->cash_tableWidget->setColumnCount(5);//设置列数
     //设置表头内容
    QStringList header;
   header<<"商品货号"<<"商品编号"<<"商品名"<<"会员卡号"<<"收取费用";
    ui->cash_tableWidget->setHorizontalHeaderLabels(header);
    //设置表宽度自适应
   ui->cash_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

   //插入窗口初始化
    insert_dialog=new insert_Dialog(dbhelper);
    timer->start(1000);//每隔1秒更新时间
    connect(timer,SIGNAL(timeout()),this,SLOT(timeupdate()));
    connect(ui->listWidget,SIGNAL(currentRowChanged(int)),ui->stackedWidget
            ,SLOT(setCurrentIndex(int)));
    connect(insert_dialog,SIGNAL(backFromInsert()),this,SLOT(dealBackSig()));

}

HomeWindow::~HomeWindow()
{
    delete ui;
    delete timer;
    delete insert_dialog;

}

void HomeWindow::setusr(QString st)
{
    this->usr=st;
    this->ui->staff_label->setText(usr);
}

void HomeWindow::timeupdate()
{
    QDateTime curr=QDateTime::currentDateTime();
    QString timestr=curr.toString("yyyy年-MM月-dd日 hh:mm:ss");
   this->ui->time_label->setText(timestr);

}

void HomeWindow::dealBackSig()
{
    this->show();
    insert_dialog->hide();
}
//确定顾客
void HomeWindow::on_pushButton_3_clicked()
{
  if(this->ui->guestID_lineEdit->text().isEmpty()||
          this->ui->guestPH_lineEdit->text().isEmpty())
  {
      QMessageBox::warning(this,"Warning","请填入顾客信息!");
      return;
  }
  guest.name=this->ui->guestID_lineEdit->text();
  guest.phnumber=this->ui->guestPH_lineEdit->text();
  this->if_guest=true;

}
//搜索商品并计算价格
void HomeWindow::on_pushButton_clicked()
{if(this->if_guest){
   if(this->ui->Atno_lineEdit->text().isEmpty() ||this->ui->goodsno_lineEdit->text().isEmpty())
   {
       QMessageBox::warning(this,"Warning","请填入商品信息!");
       return;
   }

   if(!dbhelper->connectDatabase())
   {qDebug()<<"数据库连接失败";
       return;}
   QSqlQuery query1;
   //准备插入Buy表的SQL语句
   query1.prepare("insert into Buy values(?,?,?,?,?,?,?)");
   query1.bindValue(0,this->ui->Atno_lineEdit->text());
   query1.bindValue(1,this->ui->goodsno_lineEdit->text());
    query1.bindValue(2,guest.name);
    query1.bindValue(3,guest.phnumber);
    query1.bindValue(4,this->guest.cardID);
    query1.bindValue(5,this->ui->amount_lineEdit->text().toInt());
    QDateTime curr=QDateTime::currentDateTime();
    QString timestr=curr.toString("yyyy-MM-dd hh:mm:ss");
    query1.bindValue(6,timestr);
   if(!query1.exec())//插入失败
   {
       QMessageBox::critical(this,"Error","Buy表信息插入失败");
       dbhelper->disconnectDatabase();
       return;
   }

    QSqlQuery query;
            //准备查询商品信息的SQL语句
   query.prepare("select Commodity.CAtno Atnumber,Commodity.Cno number,\
                Commodity.Cname Name Commodity.Cprice Price\
          from Commodity\
          where Commodity.CAtno= :atno\
                and Commodity.Cno= :no");
   QString atno=this->ui->Atno_lineEdit->text();
   QString no  =this->ui->goodsno_lineEdit->text();
   query.bindValue(":atno",atno);
   query.bindValue(":no",no);

   //执行查询语句
   if(query.exec())//查询成功
   {
       QSqlQueryModel *model=new QSqlQueryModel();
       model->setQuery(query);
       //将查询结果输出到tableview中
      ui->money_tableView->setModel(model);
      //表格宽度自适应
      ui->money_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

   }
   else{
       QMessageBox::critical(this,"error","操作失败！");
       dbhelper->disconnectDatabase();
       return;
   }
   //计算价格
   float p;
   while (query.next())
   {
     p=query.value(3).toString().toFloat();
    p=p*ui->amount_lineEdit->text().toInt();//单价*数量
   }   //该商品总的价格
   if(is_vip)//会员打9折
   {
       p=p*0.9;
   }
   cashamount+=p;
   //将结算信息显示在界面上
   ui->cash_tableWidget->setItem(count,0,new QTableWidgetItem(ui->Atno_lineEdit->text()));
   ui->cash_tableWidget->setItem(count,1,new QTableWidgetItem(ui->goodsno_lineEdit->text()));
   ui->cash_tableWidget->setItem(count,2,new QTableWidgetItem(query.value(2).toString()));
   ui->cash_tableWidget->setItem(count,3,new QTableWidgetItem(guest.cardID));
   ui->cash_tableWidget->setItem(count,4,new QTableWidgetItem(QString("%1").arg(p)));
   count++;
   dbhelper->disconnectDatabase();
   return;
}
    else{
        QMessageBox::warning(this,"Warning","请先确定顾客信息");
        return;
    }

}

//是否具有会员资格
void HomeWindow::on_pushButton_4_clicked()
{
    if(if_guest){
        if(ui->vip_lineEdit_2->text().isEmpty())
        {
            QMessageBox::warning(this,"Warning","请填入会员卡号!");
            return;
        }
        if(!dbhelper->connectDatabase())
        {qDebug()<<"数据库连接失败";
            return;}
        QSqlQuery query;//准备查询顾客表
        query.prepare("select VipMem.Vname,VipMem.Vphno\
                      from VipMem\
                      where VipMem.Cardno=:cardID");

         QString cardID=ui->vip_lineEdit_2->text();
         query.bindValue(":cardID",cardID);
        //执行查询语句
        if(query.exec())//查询成功
        {
            int size=query.size();
            if(size==0)
            {
                QMessageBox::critical(this,"Error","会员卡匹配失败!");
                dbhelper->disconnectDatabase();
                return;
            }
            QString name;
            QString phno;//查询人名是否和会员卡号匹配
            while (query.next()) {
                  name=query.value(0).toString();
                  phno=query.value(1).toString();
            }
            //匹配成功
            if(name.compare(guest.name)==0&&phno.compare(guest.phnumber))
            {
                   guest.cardID=this->ui->vip_lineEdit_2->text();
                   QMessageBox::information(this,"Success","会员卡匹配成功!");
                   is_vip=true;
                   dbhelper->disconnectDatabase();
                   return;
             }
            else
            {
                QMessageBox::critical(this,"Error","会员卡匹配失败!");
                guest.cardID="0";
                dbhelper->disconnectDatabase();
                return;
            }
        }
        else{
            QMessageBox::critical(this,"Error","操作失败！");
            dbhelper->disconnectDatabase();
            return;
        }

    }
    else
    {
        QMessageBox::warning(this,"Warning","请先确定顾客信息");
        return;
    }
}

//对一位客人进行结算
void HomeWindow::on_pushButton_2_clicked()
{
 if(if_guest){

     if(!dbhelper->connectDatabase())
     {qDebug()<<"数据库连接失败";
         return;}
     QSqlQuery query;//准备插入数据
     query.prepare("insert into Cash values(?,?,?,?,?,?,?)");
     query.bindValue(0,this->usr);
     query.bindValue(1,guest.name);
      query.bindValue(2,guest.phnumber);
      QDateTime curr=QDateTime::currentDateTime();
      QString timestr=curr.toString("yyyy-MM-dd hh:mm:ss");
      query.bindValue(3,timestr);
      query.bindValue(4,"1");
      query.bindValue(5,QString("%1").arg(cashamount));
      query.bindValue(6,"1");

      if(query.exec())//插入成功
      {
          if_guest=false;
          is_vip=false;
          ui->cash_tableWidget->clearContents();
          QMessageBox::about(this,"success","结算成功！");
      }

      else{
          QMessageBox::critical(this,"Error","操作失败！");
          dbhelper->disconnectDatabase();
          return;
      }

 }
else
 {
     QMessageBox::warning(this,"Warning","请先确定顾客信息");
     return;
 }
}

//查询商品信息
void HomeWindow::on_search_pushButton_6_clicked()
{
    is_single_good=false;
    if(!dbhelper->connectDatabase())
    {qDebug()<<"数据库连接失败";
        return;}

    QSqlQuery query;
    //准备查询商品信息的SQL语句
    //只查询一种
    //查询货号
    if(!ui->seararch_atno_lineEdit->text().isEmpty()\
            &&ui->search_no_lineEdit_2->text().isEmpty()\
            &&ui->search_name_lineEdit_3->text().isEmpty())
    {
   query.prepare("select * from Commodity\
          where Commodity.CAtno= :atno\
                ");
   QString atno=this->ui->seararch_atno_lineEdit->text();
   query.bindValue(":atno",atno);
}

    //查询编号
  else if(ui->seararch_atno_lineEdit->text().isEmpty()\
          &&!ui->search_no_lineEdit_2->text().isEmpty()\
          &&ui->search_name_lineEdit_3->text().isEmpty())
    {
        query.prepare("select * from Commodity\
               where Commodity.Cno= :no");
        QString no  =this->ui->search_no_lineEdit_2->text();
       query.bindValue(":no",no);
    }

    //查询名称
    else if(ui->seararch_atno_lineEdit->text().isEmpty()\
            &&ui->search_no_lineEdit_2->text().isEmpty()\
            &&!ui->search_name_lineEdit_3->text().isEmpty())
    {
        query.prepare("select * from Commodity\
               where Commodity.Cname= :name");
        QString name  =this->ui->search_name_lineEdit_3->text();
       query.bindValue(":name",name);
    }


    //三个都查询
    else if(!ui->seararch_atno_lineEdit->text().isEmpty()\
            &&!ui->search_no_lineEdit_2->text().isEmpty()\
            &&!ui->search_name_lineEdit_3->text().isEmpty()) {
        query.prepare("select * from Commodity\
               where Commodity.CAtno= :atno and Commodity.Cno= :no\
                    and  Commodity.Cname= :name");
                QString no  =this->ui->search_no_lineEdit_2->text();
               query.bindValue(":no",no);
        QString atno=this->ui->seararch_atno_lineEdit->text();
        query.bindValue(":atno",atno);
        QString name  =this->ui->search_name_lineEdit_3->text();
       query.bindValue(":name",name);
    }


    //查询名称和编号
    else if(ui->seararch_atno_lineEdit->text().isEmpty())
    {
        query.prepare("select * from Commodity\
               where Commodity.Cno= :no and Commodity.Cname= :name");
                QString name  =this->ui->search_name_lineEdit_3->text();
               query.bindValue(":name",name);
        QString no=this->ui->search_no_lineEdit_2->text();
        query.bindValue(":no",no);
    }

    //查询货号和名称
    else if(ui->search_no_lineEdit_2->text().isEmpty())
    {
        query.prepare("select * from Commodity\
               where Commodity.CAtno= :atno and Commodity.Cname= :name");
                QString name  =this->ui->search_name_lineEdit_3->text();
               query.bindValue(":name",name);
        QString atno=this->ui->seararch_atno_lineEdit->text();
        query.bindValue(":atno",atno);
    }

    //查询货号和编号
    else if(ui->search_name_lineEdit_3->text().isEmpty())
    {
        query.prepare("select * from Commodity\
               where Commodity.CAtno= :atno and Commodity.Cno= :no");
                QString no  =this->ui->search_no_lineEdit_2->text();
               query.bindValue(":no",no);
        QString atno=this->ui->seararch_atno_lineEdit->text();
        query.bindValue(":atno",atno);

    }

    //查询全部
    else  {
        query.prepare("select * from Commodity");
    }



   //执行查询语句
   if(query.exec())//查询成功
   {
       QSqlQueryModel *model=new QSqlQueryModel();
       model->setQuery(query);
       //将查询结果输出到tableview中
      ui->search_tableView->setModel(model);
      //表格宽度自适应
      ui->search_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

   }
   else{
       QMessageBox::critical(this,"Error","操作失败！");
       dbhelper->disconnectDatabase();
       return;
   }

   int size=query.size();
   if(size==0)
   {
       QMessageBox::warning(this,"Warning","不存在的商品!");
       dbhelper->disconnectDatabase();
       return;
   }
   if(size==1)
   {
       while (query.next()) {
           ui->show_atno_label_13->setText(query.value(0).toString());
           ui->show_no_label_18->setText(query.value(1).toString());
           ui->show_name_label_19->setText(query.value(2).toString());
           ui->show_amout_label_20->setText(query.value(3).toString());
           ui->show_price_label_21->setText(query.value(4).toString());
       }
        is_single_good=true;

   }
   dbhelper->disconnectDatabase();
}

void HomeWindow::on_drop_pushButton_7_clicked()
{


    if(!dbhelper->connectDatabase())
    {qDebug()<<"数据库连接失败";
        return;}

    QSqlQuery query;
    if(is_single_good)
    {
          query.prepare("delete from Commodity\
                        where Commodity.CAtno=:atno\
                              and Commodity.Cno=:no");
          QString atno=ui->show_atno_label_13->text();
          query.bindValue(":atno",atno);
          QString no=ui->show_no_label_18->text();
          query.bindValue(":no",no);
          if(query.exec())//删除成功
          {
              QMessageBox::information(this,"Success","删除成功!");
              dbhelper->disconnectDatabase();
              return;
          }
          else
          {
              QMessageBox::critical(this,"Error","删除失败!");
              dbhelper->disconnectDatabase();
              return;
          }

    }

     else
     {
         QMessageBox::warning(this,"Warning","请选定要删除的商品!");
         dbhelper->disconnectDatabase();
         return;
     }


}


//修改货号
void HomeWindow::on_up_atno_pushButton_5_clicked()
{
    if(is_single_good)
    {
        bool ok;
        QString text=QInputDialog::getText(this,"修改货号","请输入新的货号:",\
                                           QLineEdit::Normal,ui->show_atno_label_13->text(),\
                                           &ok);
        if(ok && !text.isEmpty())
        {
            ui->show_atno_label_13->setText(text);
            if(!dbhelper->connectDatabase())
            {qDebug()<<"数据库连接失败";
                return;}
             QSqlQuery query;
             query.prepare("update Commodity \
                            set Commodity.CAtno=:newatno\
                     where Commodity.CAtno=:atno\
                           and Commodity.Cno=:no ");
               QString atno=ui->show_atno_label_13->text();
               query.bindValue(":atno",atno);
               QString no=ui->show_no_label_18->text();
               query.bindValue(":no",no);
               QString newatno=text;
               query.bindValue(":newatno",newatno);
               if(query.exec())//删除成功
               {
                   QMessageBox::information(this,"Success","修改成功!");
                   dbhelper->disconnectDatabase();
                   return;
               }
               else
               {
                   QMessageBox::warning(this,"Warning","修改失败!");
                   dbhelper->disconnectDatabase();
                   return;
               }

        }
    }


    else{
        QMessageBox::warning(this,"Warning","请选定要修改的商品!");
        return;
    }

}

//修改编号
void HomeWindow::on_up_no_pushButton_8_clicked()
{
    if(is_single_good)
    {
        bool ok;
        QString text=QInputDialog::getText(this,"修改编号","请输入新的编号:",\
                                           QLineEdit::Normal,ui->show_no_label_18->text(),\
                                           &ok);
        if(ok && !text.isEmpty())
        {
            ui->show_no_label_18->setText(text);
            if(!dbhelper->connectDatabase())
            {qDebug()<<"数据库连接失败";
                return;}
             QSqlQuery query;
             query.prepare("update Commodity \
                            set Commodity.no=:newno\
                     where Commodity.CAtno=:atno\
                           and Commodity.Cno=:no ");
               QString atno=ui->show_atno_label_13->text();
               query.bindValue(":atno",atno);
               QString no=ui->show_no_label_18->text();
               query.bindValue(":no",no);
               QString newno=text;
               query.bindValue(":newno",newno);
               if(query.exec())//删除成功
               {
                   QMessageBox::information(this,"Success","修改成功!");
                   dbhelper->disconnectDatabase();
                   return;
               }
               else
               {
                   QMessageBox::critical(this,"Error","修改失败!");
                   dbhelper->disconnectDatabase();
                   return;
               }

        }
    }


    else{
        QMessageBox::warning(this,"Warning","请选定要修改的商品!");
        return;
    }
}
//修改商品名称
void HomeWindow::on_up_name_pushButton_9_clicked()
{
    if(is_single_good)
    {
        bool ok;
        QString text=QInputDialog::getText(this,"修改名称","请输入新的名称:",\
                                           QLineEdit::Normal,ui->show_name_label_19->text(),\
                                           &ok);
        if(ok && !text.isEmpty())
        {
            ui->show_name_label_19->setText(text);
            if(!dbhelper->connectDatabase())
            {qDebug()<<"数据库连接失败";
                return;}
             QSqlQuery query;
             query.prepare("update Commodity \
                            set Commodity.Cname=:newname\
                     where Commodity.CAtno=:atno\
                           and Commodity.Cno=:no ");
               QString atno=ui->show_atno_label_13->text();
               query.bindValue(":atno",atno);
               QString no=ui->show_no_label_18->text();
               query.bindValue(":no",no);
               QString newname=text;
               query.bindValue(":newname",newname);
               if(query.exec())//删除成功
               {
                   QMessageBox::information(this,"Success","修改成功!");
                   dbhelper->disconnectDatabase();
                   return;
               }
               else
               {
                   QMessageBox::critical(this,"Error","修改失败!");
                   dbhelper->disconnectDatabase();
                   return;
               }

        }
    }


    else{
        QMessageBox::warning(this,"Warning","请选定要修改的商品!");
        return;
    }

}

void HomeWindow::on_up_amout_pushButton_10_clicked()
{
    if(is_single_good)
    {
        bool ok;
        int amount=QInputDialog::getInt(this,"修改数量","请输入新的数量:",\
                                          ui->show_amout_label_20->text().toInt(&ok),\
                                           0,400,10,&ok);
        if(ok)
        {
            ui->show_amout_label_20->setText(QString::number(amount));
            if(!dbhelper->connectDatabase())
            {qDebug()<<"数据库连接失败";
                return;}
             QSqlQuery query;
             query.prepare("update Commodity \
                            set Commodity.Camount=:amount\
                     where Commodity.CAtno=:atno\
                           and Commodity.Cno=:no ");
               QString atno=ui->show_atno_label_13->text();
               query.bindValue(":atno",atno);
               QString no=ui->show_no_label_18->text();
               query.bindValue(":no",no);

               query.bindValue(":amount",amount);
               if(query.exec())
               {
                   QMessageBox::information(this,"success","修改成功!");
                   dbhelper->disconnectDatabase();
                   return;
               }
               else
               {
                   QMessageBox::warning(this,"error","修改失败!");
                   dbhelper->disconnectDatabase();
                   return;
               }

        }
    }


    else{
        QMessageBox::warning(this,"error","请选定要修改的商品!");
        return;
    }
}

//修改价格
void HomeWindow::on_up_price_pushButton_11_clicked()
{
    if(is_single_good)
    {
        bool ok;
        QString text=QInputDialog::getText(this,"修改价格","请输入新的价格:",\
                                           QLineEdit::Normal,ui->show_price_label_21->text(),\
                                           &ok);
        if(ok && !text.isEmpty())
        {
            ui->show_price_label_21->setText(text);
            if(!dbhelper->connectDatabase())
            {qDebug()<<"数据库连接失败";
                return;}
             QSqlQuery query;
             query.prepare("update Commodity \
                            set Commodity.Cprice=:newprice\
                     where Commodity.CAtno=:atno\
                           and Commodity.Cno=:no ");
               QString atno=ui->show_atno_label_13->text();
               query.bindValue(":atno",atno);
               QString no=ui->show_no_label_18->text();
               query.bindValue(":no",no);
               float newprice=text.toFloat();
               query.bindValue(":newname",newprice);
               if(query.exec())
               {
                   QMessageBox::information(this,"Success","修改成功!");
                   dbhelper->disconnectDatabase();
                   return;
               }
               else
               {
                   QMessageBox::critical(this,"Error","修改失败!");
                   dbhelper->disconnectDatabase();
                   return;
               }

        }
    }


    else{
        QMessageBox::warning(this,"Warning","请选定要修改的商品!");
        return;
    }
}

//加入新的商品，需要进入新的对话框
void HomeWindow::on_insert_pushButton_6_clicked()
{
   this->hide();
    insert_dialog->show();

}
