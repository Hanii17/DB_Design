#include "homewindow.h"
#include "ui_homewindow.h"
#include<QDebug>


#include<QDate>
#include<QSqlDriver>
HomeWindow::HomeWindow(DBDAL *dbh,QString id,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::HomeWindow)
{
    usr=id;
    dbhelper=dbh;
    cashamount=0;
    count=0;
    count_forVIP=0;
    if_guest=false;
    is_vip=false;
    is_single_good=false;
    if_show_staff=false;
    if_time_fixed=false;
    guest.cardID="0";
    ui->setupUi(this);
    timer=new QTimer();
    this->ui->staff_label->setText(usr);//显示收银员编号

     model=new QSqlQueryModel();
    //设置密码的输入框为密码模式
    ui->new_pswd_lineEdit_6->setEchoMode(QLineEdit::Password);
    ui->new_again_pswd_lineEdit_7->setEchoMode(QLineEdit::Password);

    //设置收银时的表格
    this->ui->cash_tableWidget->setColumnCount(5);//设置列数
     //设置表头内容
    QStringList header;
   header<<"商品货号"<<"商品编号"<<"商品名"<<"会员卡号"<<"收取费用";
    ui->cash_tableWidget->setHorizontalHeaderLabels(header);
    //设置表宽度自适应
   ui->cash_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);



   //设置会员卡表
   ui->vip_info_tableWidget->setColumnCount(5);//列数
   //表头内容
   QStringList header1;
   header1<<"会员卡号"<<"会员姓名"<<"会员电话"<<"会员卡到期日期"<<"累计消费额";
   ui->vip_info_tableWidget->setHorizontalHeaderLabels(header1);
   //设置表宽度自适应
   ui->vip_info_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

  //设置商品价格表
   ui->money_tableWidget->setColumnCount(4);
   //表头内容
   QStringList header2;
   header2<<"商品货号"<<"商品编号"<<"商品名"<<"商品单价";
   ui->money_tableWidget->setHorizontalHeaderLabels(header2);
   //设置表宽度自适应
   ui->money_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

   //插入窗口初始化
    insert_dialog=new insert_Dialog(dbhelper);


    //设置定时器
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
    delete model;
    dbhelper->disconnectDatabase();

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
    if(if_guest)
    {
        QMessageBox::warning(this,"error","请确保结算完成");
        return;
    }
  if(this->ui->guestID_lineEdit->text().isEmpty()||
          this->ui->guestPH_lineEdit->text().isEmpty())
  {
      QMessageBox::warning(this,"Warning","请填入顾客信息!");
      return;
  }
  guest.name=this->ui->guestID_lineEdit->text();
  guest.phnumber=this->ui->guestPH_lineEdit->text();
  this->if_guest=true;
  QMessageBox::information(this,"success","顾客信息已保存");

}


//搜索商品并计算价格(应用事务)
void HomeWindow::on_pushButton_clicked()
{
    dbhelper->disconnectDatabase();
    bool result1=false;
    bool result2=false;
    bool result3=false;
    if(this->if_guest){
   if(this->ui->Atno_lineEdit->text().isEmpty() ||this->ui->goodsno_lineEdit->text().isEmpty()\
             || ui->amount_lineEdit->text().isEmpty())
   {
       QMessageBox::warning(this,"Warning","请填入商品信息!");
       return;
   }

   if(!dbhelper->connectDatabase())
   {qDebug()<<"数据库连接失败";
       return;}
   QSqlQuery query1;
       QSqlQuery query;
       QSqlQuery query2;
       int size;

       //准备查询商品信息的SQL语句
query.prepare("select Commodity.CAtno Atnumber,Commodity.Cno Number,Commodity.Cname Name,Commodity.Cprice price,Commodity.Camount\
            from Commodity\
     where Commodity.CAtno= :atno\
           and Commodity.Cno= :no");
QString atno=this->ui->Atno_lineEdit->text();
QString no  =this->ui->goodsno_lineEdit->text();
qDebug()<<atno<<"   "<<no;
query.bindValue(":atno",atno);
query.bindValue(":no",no);
result2=query.exec();
int initialpos= query.at();
       if(query.last())
       {
           size=query.at()+1;
       }
       else
       {
           size=0;
       }
       query.seek(initialpos);
       if(size==0)
       {
           QMessageBox::critical(this,"Error","不存在的商品!");
           dbhelper->disconnectDatabase();
           return;
       }
       query.first();
       if(query.value(4).toString().toInt()<this->ui->amount_lineEdit->text().toInt())//如果商品不足
       {
           result2=false;
           QMessageBox::warning(this,"warning","商品数量超过库存");
       }



     //事务部分，向BUY表插入信息，向Guest表插入信息
   if(dbhelper->db.driver()->hasFeature(QSqlDriver::Transactions))//如果数据库支持事务
   {
      dbhelper->db.transaction();//开启事务


   //准备插入Buy表的SQL语句
   query1.prepare("insert into Buy values(?,?,?,?,?,?,?)");
   query1.bindValue(0,this->ui->Atno_lineEdit->text());
   query1.bindValue(1,this->ui->goodsno_lineEdit->text());
    query1.bindValue(2,guest.name);
    query1.bindValue(3,guest.phnumber);
    query1.bindValue(5,this->guest.cardID);
    query1.bindValue(6,this->ui->amount_lineEdit->text().toInt());
    QDateTime curr=QDateTime::currentDateTime();
    QString timestr=curr.toString("yyyy-MM-dd hh:mm:ss");

    if(!if_time_fixed){
    query1.bindValue(4,timestr);
    fixed_time=timestr;
    if_time_fixed=true;
    }
    else query1.bindValue(4,fixed_time);

    //准备插入顾客表的SQL语句
    query2.prepare("insert into Guest values(?,?)");
    query2.bindValue(0,guest.name);
    query2.bindValue(1,guest.phnumber);

   result3=query2.exec();
   result1=query1.exec();


   //如果都执行成功则进行事务提交，否则进行事务回滚
   if(result1&&result2)
   {
       QSqlQuery query3=query;

        query.first();
      //显示商品单价
       ui->money_tableWidget->insertRow(ui->money_tableWidget->rowCount());
       count=ui->money_tableWidget->rowCount()-1;
       ui->money_tableWidget->setItem(count,0,new QTableWidgetItem(ui->Atno_lineEdit->text()));
       ui->money_tableWidget->item(count,0)->setTextAlignment(Qt::AlignHCenter| Qt::AlignVCenter);
       ui->money_tableWidget->setItem(count,1,new QTableWidgetItem(ui->goodsno_lineEdit->text()));
       ui->money_tableWidget->item(count,1)->setTextAlignment(Qt::AlignHCenter| Qt::AlignVCenter);
       ui->money_tableWidget->setItem(count,2,new QTableWidgetItem(query.value(2).toString()));
       ui->money_tableWidget->item(count,2)->setTextAlignment(Qt::AlignHCenter| Qt::AlignVCenter);
       ui->money_tableWidget->setItem(count,3,new QTableWidgetItem(query.value(3).toString()));
       ui->money_tableWidget->item(count,3)->setTextAlignment(Qt::AlignHCenter| Qt::AlignVCenter);
       //计算价格
       int p;
       query3.first();
        p=query3.value(3).toString().toInt();
        p=p*(ui->amount_lineEdit->text().toInt());//单价*数量
         //该商品总的价格
       if(is_vip)//会员打9折
       {
           p=p*0.9;
       }
       cashamount+=p;

        query3.first();
       //将结算信息显示在界面上
       ui->cash_tableWidget->insertRow(ui->cash_tableWidget->rowCount());
       count=ui->cash_tableWidget->rowCount()-1;
       ui->cash_tableWidget->setItem(count,0,new QTableWidgetItem(ui->Atno_lineEdit->text()));
       ui->cash_tableWidget->item(count,0)->setTextAlignment(Qt::AlignHCenter| Qt::AlignVCenter);
       ui->cash_tableWidget->setItem(count,1,new QTableWidgetItem(ui->goodsno_lineEdit->text()));
       ui->cash_tableWidget->item(count,1)->setTextAlignment(Qt::AlignHCenter| Qt::AlignVCenter);
       ui->cash_tableWidget->setItem(count,2,new QTableWidgetItem(query3.value(2).toString()));
       ui->cash_tableWidget->item(count,2)->setTextAlignment(Qt::AlignHCenter| Qt::AlignVCenter);
       ui->cash_tableWidget->setItem(count,3,new QTableWidgetItem(guest.cardID));
       ui->cash_tableWidget->item(count,3)->setTextAlignment(Qt::AlignHCenter| Qt::AlignVCenter);
       ui->cash_tableWidget->setItem(count,4,new QTableWidgetItem(QString::number(p)));
       ui->cash_tableWidget->item(count,4)->setTextAlignment(Qt::AlignHCenter| Qt::AlignVCenter);

       dbhelper->db.commit();
   }
   else
   {
       dbhelper->db.rollback();
       if(!result1)//插入失败
       {
           QMessageBox::critical(this,"Error","Buy表信息插入失败");
           dbhelper->disconnectDatabase();
           return;
       }
       else if(!result2){
           QMessageBox::critical(this,"error","操作失败！");
           dbhelper->disconnectDatabase();
           return;
       }
   }
   }

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
     bool result1=false;
     bool result2=false;
     if(!dbhelper->connectDatabase())
     {qDebug()<<"数据库连接失败";
         return;}
     QSqlQuery query;//准备插入数据
     QSqlQuery query3;
     QSqlQuery query4;

     //开启事务
     if(dbhelper->db.driver()->hasFeature(QSqlDriver::Transactions))//如果数据库支持事务
     {
        dbhelper->db.transaction();//开启事务

     query.prepare("insert into Cash values(?,?,?,?,?,?,?)");
     query.bindValue(0,this->usr);
     query.bindValue(1,guest.name);
      query.bindValue(2,guest.phnumber);
      query.bindValue(3,fixed_time);
      query.bindValue(4,"1");
      query.bindValue(5,QString::number(cashamount));
      query.bindValue(6,"1");
            result2=query.exec();
      //查询是否属于会员
     QSqlQuery query1;
     query1.prepare("select* from VipMem\
                    where Vname=:vname and Vphno=:vphno");
     QString vname=guest.name;
      QString vphno=guest.phnumber;
             query1.bindValue(":vname",vname);
             query1.bindValue(":vphno",vphno);
     query1.exec();
     int initialpos= query1.at();
     int size;
            if(query1.last())
            {
                size=query1.at()+1;
            }
            else
            {
                size=0;
            }
            query1.seek(initialpos);
            if(size==0)
            {

                //非会员一次消费满1000元就免费赠送会员卡
                if(cashamount>=1000)
                {
                    query4.prepare("select MAX(CONVERT(int,VipMem.Cardno))\
                                   from VipMem");
                    query4.exec();
                    query4.first();
                   int tmp= query4.value(0).toString().toInt();
                         tmp+=1;
                    query3.prepare("insert into VipMem\
                                   values(?,?,?,?,?) ");
                    query3.bindValue(0,QString::number(tmp));
                    query3.bindValue(1,guest.name);
                    query3.bindValue(2,guest.phnumber);
                    QDateTime scurr=QDateTime::currentDateTime();
                    QDateTime date1=scurr.addYears(1);
                    QString stimestr=date1.toString("yyyy-MM-dd hh:mm:ss");
                    query3.bindValue(3,stimestr);
                    query3.bindValue(4,0);
                    result1=query3.exec();
                }
                else result1=true;
            }
            else result1=true;



      if(result1&&result2)//插入成功
      {
          if_guest=false;
          is_vip=false;
          if_time_fixed=false;
          count = ui->cash_tableWidget->rowCount()-1;
         while(count>=0){
          ui->cash_tableWidget->removeRow(count);
          count--;
         }
         count = ui->money_tableWidget->rowCount()-1;
         while(count>=0){
          ui->money_tableWidget->removeRow(count);
          count--;
         }
         cashamount=0;
          count =0;
          QMessageBox::about(this,"success","结算成功！");
          dbhelper->db.commit();
          dbhelper->disconnectDatabase();
      }

      else{
          dbhelper->db.rollback();

          if(!result2){
          QMessageBox::critical(this,"Error","插入Cash表失败");
          dbhelper->disconnectDatabase();
          return;}
          if(!result1)
          {
              QMessageBox::critical(this,"Error","自动发放会员卡失败");
              dbhelper->disconnectDatabase();
              return;
          }
      }
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
    dbhelper->disconnectDatabase();
    is_single_good=false;
    if(!dbhelper->connectDatabase())
    {qDebug()<<"数据库连接失败";
        return;}


    //准备查询商品信息的SQL语句
    //只查询一种
    //查询货号
    if(!ui->seararch_atno_lineEdit->text().isEmpty()\
            &&ui->search_no_lineEdit_2->text().isEmpty()\
            &&ui->search_name_lineEdit_3->text().isEmpty())
    {
   query_search_goods.prepare("select * from Commodity\
          where Commodity.CAtno= :atno\
                ");
   QString atno=this->ui->seararch_atno_lineEdit->text();
   query_search_goods.bindValue(":atno",atno);
}

    //查询编号
  else if(ui->seararch_atno_lineEdit->text().isEmpty()\
          &&!ui->search_no_lineEdit_2->text().isEmpty()\
          &&ui->search_name_lineEdit_3->text().isEmpty())
    {
        query_search_goods.prepare("select * from Commodity\
               where Commodity.Cno= :no");
        QString no  =this->ui->search_no_lineEdit_2->text();
       query_search_goods.bindValue(":no",no);
    }

    //查询名称
    else if(ui->seararch_atno_lineEdit->text().isEmpty()\
            &&ui->search_no_lineEdit_2->text().isEmpty()\
            &&!ui->search_name_lineEdit_3->text().isEmpty())
    {
        query_search_goods.prepare("select * from Commodity\
               where Commodity.Cname= :name");
        QString name  =this->ui->search_name_lineEdit_3->text();
       query_search_goods.bindValue(":name",name);
    }


    //三个都查询
    else if(!ui->seararch_atno_lineEdit->text().isEmpty()\
            &&!ui->search_no_lineEdit_2->text().isEmpty()\
            &&!ui->search_name_lineEdit_3->text().isEmpty()) {
        query_search_goods.prepare("select * from Commodity\
               where Commodity.CAtno= :atno and Commodity.Cno= :no\
                    and  Commodity.Cname= :name");
                QString no  =this->ui->search_no_lineEdit_2->text();
               query_search_goods.bindValue(":no",no);
        QString atno=this->ui->seararch_atno_lineEdit->text();
        query_search_goods.bindValue(":atno",atno);
        QString name  =this->ui->search_name_lineEdit_3->text();
       query_search_goods.bindValue(":name",name);
    }


    //查询名称和编号
    else if(ui->seararch_atno_lineEdit->text().isEmpty())
    {
        query_search_goods.prepare("select * from Commodity\
               where Commodity.Cno= :no and Commodity.Cname= :name");
                QString name  =this->ui->search_name_lineEdit_3->text();
               query_search_goods.bindValue(":name",name);
        QString no=this->ui->search_no_lineEdit_2->text();
        query_search_goods.bindValue(":no",no);
    }

    //查询货号和名称
    else if(ui->search_no_lineEdit_2->text().isEmpty())
    {
        query_search_goods.prepare("select * from Commodity\
               where Commodity.CAtno= :atno and Commodity.Cname= :name");
                QString name  =this->ui->search_name_lineEdit_3->text();
               query_search_goods.bindValue(":name",name);
        QString atno=this->ui->seararch_atno_lineEdit->text();
        query_search_goods.bindValue(":atno",atno);
    }

    //查询货号和编号
    else if(ui->search_name_lineEdit_3->text().isEmpty())
    {
        query_search_goods.prepare("select * from Commodity\
               where Commodity.CAtno= :atno and Commodity.Cno= :no");
                QString no  =this->ui->search_no_lineEdit_2->text();
               query_search_goods.bindValue(":no",no);
        QString atno=this->ui->seararch_atno_lineEdit->text();
        query_search_goods.bindValue(":atno",atno);


    }

    //查询全部
    else  {
        query_search_goods.prepare("select * from Commodity");
    }


QSqlQuery query1=query_search_goods;
   //执行查询语句
   //if(query.exec())//查询成功
  // {
       qDebug()<<"进入创建";
        // query1=query;
       query_search_goods.exec();
       model->setQuery(query_search_goods);
       //将查询结果输出到tableview中
      ui->search_tableView->setModel(model);
      //表格宽度自适应
      ui->search_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
   QMessageBox::information(this,"success","查询成功");
   //}
  /* else{
       QMessageBox::critical(this,"Error","操作失败！");
       dbhelper->disconnectDatabase();
       return;
   }*/
if(query1.exec()){
   int initialpos= query1.at();

     int size;
     if(query1.last())
     {
         size=query1.at()+1;
     }
     else
     {
         size=0;
     }
     query1.seek(initialpos);
   qDebug()<<size;
   if(size==0)
   {
       QMessageBox::warning(this,"Warning","不存在的商品!");
       dbhelper->disconnectDatabase();
       return;
   }
   if(size==1)
   {
       qDebug()<<"显示";
       while (query1.next()) {
           ui->show_atno_label_13->setText(query1.value(0).toString());
           ui->show_no_label_18->setText(query1.value(1).toString());
           ui->show_name_label_19->setText(query1.value(2).toString());
           ui->show_amout_label_20->setText(query1.value(3).toString());
           ui->show_price_label_21->setText(query1.value(4).toString());
       }
        is_single_good=true;

   }
}
  // dbhelper->disconnectDatabase();
}

//删除商品
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
              ui->show_atno_label_13->setText("");
              ui->show_no_label_18->setText("");
              ui->show_name_label_19->setText("");
              ui->show_amout_label_20->setText("");
              ui->show_price_label_21->setText("");
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

//修改数量
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

//办理会员卡
void HomeWindow::on_insert_vip_pushButton_5_clicked()
{
    if(ui->Vname_lineEdit_2->text().isEmpty()\
       ||ui->V_phone_lineEdit_3->text().isEmpty())
    {
         QMessageBox::warning(this,"Warning","未填入信息！");
         return;
    }
   else
    {
        if(!dbhelper->connectDatabase())
        {qDebug()<<"数据库连接失败";
            return;}
         QSqlQuery query;
         QSqlQuery query1;
         int idnum=0;
         //先检查输入的会员卡是否已办理
         query.prepare("select MAX(CONVERT(int,VipMem.Cardno))\
                       from VipMem");
         if(query.exec())//查询成功
         {
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
            if(size>0)
            {

                query.first();
                idnum=query.value(0).toString().toInt();

                      idnum+=1;

                              //办理会员卡，向会员表中插入数据
                               query1.prepare("insert into VipMem\
                                              values(?,?,?,?,0)");
                                query1.bindValue(0,QString::number(idnum));
                                query1.bindValue(1,ui->Vname_lineEdit_2->text());
                               query1.bindValue(2,ui->V_phone_lineEdit_3->text());
                               QDateTime curr=QDateTime::currentDateTime();
                               QDateTime date1=curr.addYears(1);
                               QString timestr=date1.toString("yyyy-MM-dd hh:mm:ss");
                               query1.bindValue(3,timestr);
                               if(query1.exec())//插入成功
                               {
                                      QMessageBox::information(this,"Success","会员卡办理成功！");
                                      dbhelper->disconnectDatabase();
                                      return;
                               }
                               else
                               {
                                   QMessageBox::critical(this,"Error","会员卡办理失败!");
                                   dbhelper->disconnectDatabase();
                                   return;
                               }
            }
            else
            {
                  QMessageBox::critical(this,"Error","办理会员卡失败!");
                  dbhelper->disconnectDatabase();
                  return;
            }
         }
         else
         {
             QMessageBox::critical(this,"Error","查询会员卡失败!");
             dbhelper->disconnectDatabase();
             return;
         }
    }
}

//查询会员卡到期时间
void HomeWindow::on_serch_vip_pushButton_6_clicked()
{
    if(ui->card_id_lineEdit->text().isEmpty())
    {
        QMessageBox::warning(this,"Warning","请填入会员卡号！");
        return;
    }
    else
    {
        if(!dbhelper->connectDatabase())
        {qDebug()<<"数据库连接失败";
            return;}
         QSqlQuery query;//根据会员卡查询
         query.prepare("select* from VipMem\
                       where Cardno=:cardno");
         QString cardno=ui->card_id_lineEdit->text();
         query.bindValue(":cardno",cardno);
         if(query.exec())//查询成功
         {
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
               if(size==0)
               {
                   QMessageBox::critical(this,"Error","不存在的会员卡！");
                   dbhelper->disconnectDatabase();
                   return;
               }
               else
               {
                   qDebug()<<"窗口显示";
                   //将会员信息显示在界面上
                    qDebug()<<"countvip"<<count_forVIP;
                   while (query.next()) {

                   ui->vip_info_tableWidget->insertRow(ui->vip_info_tableWidget->rowCount());
                    count_forVIP=ui->vip_info_tableWidget->rowCount()-1;
                   ui->vip_info_tableWidget->setItem(count_forVIP,0,new QTableWidgetItem(ui->card_id_lineEdit->text()));
                   ui->vip_info_tableWidget->item(count_forVIP,0)->setTextAlignment(Qt::AlignHCenter| Qt::AlignVCenter);
                   ui->vip_info_tableWidget->setItem(count_forVIP,1,new QTableWidgetItem(query.value(1).toString()));
                   ui->vip_info_tableWidget->item(count_forVIP,1)->setTextAlignment(Qt::AlignHCenter| Qt::AlignVCenter);
                   ui->vip_info_tableWidget->setItem(count_forVIP,2,new QTableWidgetItem(query.value(2).toString()));
                   ui->vip_info_tableWidget->item(count_forVIP,2)->setTextAlignment(Qt::AlignHCenter| Qt::AlignVCenter);
                   ui->vip_info_tableWidget->setItem(count_forVIP,3,new QTableWidgetItem(query.value(3).toString()));
                   ui->vip_info_tableWidget->item(count_forVIP,3)->setTextAlignment(Qt::AlignHCenter| Qt::AlignVCenter);
                   ui->vip_info_tableWidget->setItem(count_forVIP,4,new QTableWidgetItem(query.value(4).toString()));
                   ui->vip_info_tableWidget->item(count_forVIP,4)->setTextAlignment(Qt::AlignHCenter| Qt::AlignVCenter);

                   qDebug()<<"countvip"<<count_forVIP;

                   }


                   QMessageBox::information(this,"Success","查询成功");
                   dbhelper->disconnectDatabase();
                   return;
               }
         }
         else
         {
             QMessageBox::critical(this,"Error","查询失败！");
             dbhelper->disconnectDatabase();
             return;
         }

    }
}

//将会员卡延期一年
void HomeWindow::on_days_pushButton_5_clicked()
{
   if(ui->card_id_lineEdit->text().isEmpty())
   {
       QMessageBox::warning(this,"Warning","请选择对应的会员卡");
       return;
   }
   else
   {
       if(!dbhelper->connectDatabase())
       {qDebug()<<"数据库连接失败";
           return;}
       QSqlQuery query1;//先查询原来的到期时间
       query1.prepare("select convert(varchar(20),VipMem.Deadline,120)\
                      from VipMem\
                      where Cardno=cardno");
               QString cardno=ui->card_id_lineEdit->text();
       query1.bindValue(":cardno",cardno);
        QString olddate;
       if(query1.exec())
       {
           int initialpos= query1.at();

             int size;
             if(query1.last())
             {
                 size=query1.at()+1;
             }
             else
             {
                 size=0;
             }
             query1.seek(initialpos);
           if(size>0)
           {
               while (query1.next()) {
                    olddate=query1.value(0).toString();
               }

           }
           else
           {
               QMessageBox::critical(this,"Error","操作失败!");
               dbhelper->disconnectDatabase();
               return;
           }

       }
       else
       {
           QMessageBox::critical(this,"Error","操作失败!");
           dbhelper->disconnectDatabase();
           return;
       }
       qDebug()<<"olddate"<<olddate;
        QDateTime date=QDateTime::fromString(olddate,"yyyy-MM-dd hh:mm:ss");
        qDebug()<<"date"<<date;
        QDateTime newdate=date.addYears(1);
        qDebug()<<"newdate"<<newdate;
        QString deadline=newdate.toString("yyyy-MM-dd hh:mm:ss");
        qDebug()<<"deadline"<<deadline;
        QSqlQuery query;//会员卡表更新
        query.prepare("update VipMem\
                      set Deadline=:deadline\
                      where Cardno=:cardno");
       query.bindValue(":cardno",cardno);
       query.bindValue(":deadline",deadline);
        if(query.exec())//数据更新成功
        {
            QMessageBox::information(this,"Success","会员卡延期成功，当前会员卡到期时间为："+deadline);
            dbhelper->disconnectDatabase();
            return;

        }
        else
        {
            QMessageBox::critical(this,"Error","延期失败!");
            dbhelper->disconnectDatabase();
            return;
        }

   }
}

//回收会员卡
void HomeWindow::on_drop_vip_pushButton_8_clicked()
{
    if(ui->card_id_lineEdit->text().isEmpty())
    {
        QMessageBox::warning(this,"Warning","请选择对应的会员卡");
        return;
    }
    else
    {

        if(!dbhelper->connectDatabase())
        {qDebug()<<"数据库连接失败";
            return;}
        QSqlQuery query;//删除对应的会员卡
       query.prepare("delete \
                     from VipMem\
                     where Cardno=:cardno");
               QString cardno=ui->card_id_lineEdit->text();
        query.bindValue(":cardno",cardno);
       if(query.exec())//数据删除成功
       {
           QMessageBox::information(this,"Success","ID为"+cardno+"的会员卡删除成功");
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
    }


//查询收银员信息
void HomeWindow::on_update_staff_pushButton_5_clicked()
{
   if(ui->stno_lineEdit_4->text().isEmpty())
   {
       QMessageBox::warning(this,"Warning","请先填入收银员编号");
       if_show_staff=false;
       return;
   }
   else
   {

       if(!dbhelper->connectDatabase())
       {qDebug()<<"数据库连接失败";
           if_show_staff=false;
           return;}
       QSqlQuery query;//查询收银员信息
      query.prepare("select * from Staff\
                    where Staff.Stno=:stno");
      QString stno=ui->stno_lineEdit_4->text() ;
      query.bindValue(":stno",stno);
      if(query.exec())
      {
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
         if(size>0)
         { //显示收银员信息
             while (query.next()) {
                ui->staff_name_label_25->setText(query.value(1).toString());
                if(query.value(2).toInt()==0)
                ui->staff_sex_label_27->setText("女");
                else
                ui->staff_sex_label_27->setText("男");
                ui->staff_pswd_label_29->setText(query.value(3).toString());
             }
             if_show_staff=true;
            QMessageBox::information(this,"Success","身份信息已更新");
            dbhelper->disconnectDatabase();
            return;
         }
         else
         {
             QMessageBox::critical(this,"Error","不存在的身份编号");
             dbhelper->disconnectDatabase();
             if_show_staff=false;
             return;
         }
      }
      else
      {
          QMessageBox::critical(this,"Error","查询失败!");
          dbhelper->disconnectDatabase();
          if_show_staff=false;
          return;
      }

   }
}


//修改姓名
void HomeWindow::on_change_name_pushButton_6_clicked()
{
 if(if_show_staff)
 {
     bool ok;
     QString text=QInputDialog::getText(this,"修改姓名","请输入新的姓名:",\
                                        QLineEdit::Normal,ui->staff_name_label_25->text(),\
                                        &ok);
     if(ok && !text.isEmpty())
     {
         ui->staff_name_label_25->setText(text);
         if(!dbhelper->connectDatabase())
         {qDebug()<<"数据库连接失败";
             return;}
          QSqlQuery query;
          query.prepare("update Staff \
                        set Stname=:stname \
                        where Staff.Stno=:stno ");
            QString stno=ui->stno_lineEdit_4->text();
            query.bindValue(":stno",stno);
            QString stname=text;
            query.bindValue(":stname",stname);
            if(query.exec())//修改成功
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
 else
 {
     QMessageBox::warning(this,"Warning","请先选择一个身份信息");
     return;
 }

}



//修改性别
void HomeWindow::on_change_sex_pushButton_7_clicked()
{
    if(if_show_staff)
    {
          QStringList SexItems;
          SexItems<<"男"<<"女";
          bool ok;
          QString sexitem=QInputDialog::getItem(this,"修改性别","请选择性别",SexItems,\
                                                0,false,&ok);
          if(ok&&!sexitem.isEmpty())
          {
               ui->staff_sex_label_27->setText(sexitem);
               if(!dbhelper->connectDatabase())
               {qDebug()<<"数据库连接失败";
                   return;}
                QSqlQuery query;
                query.prepare("update Staff \
                              set Stsex=:stsex \
                              where Staff.Stno=:stno ");
                  QString stno=ui->stno_lineEdit_4->text();
                  query.bindValue(":stno",stno);
                  int stsex=(!sexitem.compare("男"))?1:0;
                  query.bindValue(":stsex",stsex);
                  if(query.exec())//修改成功
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
    else
    {
        QMessageBox::warning(this,"Warning","请先选择一个身份信息");
        return;
    }
}


//修改密码
void HomeWindow::on_change_pswd_pushButton_8_clicked()
{
    if(if_show_staff)
    {
        bool ok;
        QString text=QInputDialog::getText(this,"修改密码","请输入新的密码:",\
                                           QLineEdit::Normal,ui->staff_pswd_label_29->text(),\
                                           &ok);
        if(ok && !text.isEmpty())
        {
            ui->staff_pswd_label_29->setText(text);
            if(!dbhelper->connectDatabase())
            {qDebug()<<"数据库连接失败";
                return;}
             QSqlQuery query;
             query.prepare("update Staff \
                           set StPswd=:stpwd \
                           where Staff.Stno=:stno ");
               QString stno=ui->stno_lineEdit_4->text();
               query.bindValue(":stno",stno);
               QString stpwd=text;
               query.bindValue(":stpwd",stpwd);
               if(query.exec())//修改成功
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
}


//新增收银员信息
void HomeWindow::on_new_staff_pushButton_9_clicked()
{
  if(ui->new_staffID_lineEdit_4->text().isEmpty()\
     || ui->new_name_lineEdit_5->text().isEmpty()\
     || ui->new_again_pswd_lineEdit_7->text().isEmpty()\
      || ui->new_pswd_lineEdit_6->text().isEmpty() )
  {
      QMessageBox::warning(this,"error","请填写完整的信息");
      return;
  }
  else
  {
      if(!dbhelper->connectDatabase())
      {qDebug()<<"数据库连接失败";
          return;}

       QSqlQuery query;//检查编号是否重复
       query.prepare("select* from Staff\
                     where Stno=:stno");
        QString stno=ui->new_staffID_lineEdit_4->text();
         query.bindValue(":stno",stno);
       qDebug()<<ui->new_staffID_lineEdit_4->text();
       if(query.exec())//查询成功
       {
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

          if(size>0)
          {
              QMessageBox::critical(this,"Error","已存在的身份信息！");
              dbhelper->disconnectDatabase();
              return;
          }

          else//新增身份信息
          {
              if(ui->new_pswd_lineEdit_6->text().compare(\
                          ui->new_again_pswd_lineEdit_7->text())!=0)
              {
                  QMessageBox::warning(this,"Warning","两次输入的密码不一致");
                  dbhelper->disconnectDatabase();
                  return;
              }
              QSqlQuery query1;
              query1.prepare("insert into Staff\
                             values(?,?,?,?)");
               query1.bindValue(0,ui->new_staffID_lineEdit_4->text());
               query1.bindValue(1,ui->new_name_lineEdit_5->text());
              if(ui->man_radioButton->isChecked())
                  query1.bindValue(2,1);
              else
                  query1.bindValue(2,0);
              query1.bindValue(3,ui->new_pswd_lineEdit_6->text());

              if(query1.exec())//插入成功
              {
                     QMessageBox::information(this,"Success","新增身份信息成功");
                     dbhelper->disconnectDatabase();
                     return;
              }
              else
              {
                  QMessageBox::critical(this,"Error","新增身份信息失败!");
                  dbhelper->disconnectDatabase();
                  return;
              }
          }
  }
       else
       {
           QMessageBox::critical(this,"Error","操作失败");
           dbhelper->disconnectDatabase();
           return;
       }
}
}

//查询收银记录
void HomeWindow::on_search_record_pushButton_6_clicked()
{
    dbhelper->disconnectDatabase();
    QString stno,cdate;
    int duty=1;
     if(!dbhelper->connectDatabase())
     {qDebug()<<"数据库连接失败";

        return;}
QDate date;
    QSqlQuery query;
    //准备查询商品信息的SQL语句
    //只查询收银员编号
    if(!ui->search_staff_ID_lineEdit->text().isEmpty()\
      &&ui->search_date_lineEdit_2->text().isEmpty()\
      &&ui->search_duty_lineEdit_3->text().isEmpty())
  {
        query.prepare("select*\
                      from View_cash_money\
                      where View_cash_money.Stno=:stno");


                 stno=ui->search_staff_ID_lineEdit->text();
                 query.bindValue(":stno",stno);
  }

    //只查询日期
  else if(ui->search_staff_ID_lineEdit->text().isEmpty()\
          &&!ui->search_date_lineEdit_2->text().isEmpty()\
          &&ui->search_duty_lineEdit_3->text().isEmpty())
  {
        query.prepare("select*\
                      from View_cash_money\
                      where convert(date,View_cash_money.Cdate)=cdate");
                  date=QDate::fromString(ui->search_date_lineEdit_2->text(),"yyyy-MM-dd");
                 cdate=date.toString();
                 query.bindValue(":cdate",cdate);

  }
    //只查询班次
  else if(ui->search_staff_ID_lineEdit->text().isEmpty()\
          &&ui->search_date_lineEdit_2->text().isEmpty()\
          &&!ui->search_duty_lineEdit_3->text().isEmpty())
    {
        query.prepare("select*\
                      from View_cash_money\
                      where  View_cash_money.Duty=:duty");
                 duty=ui->search_duty_lineEdit_3->text().toUInt();
                 query.bindValue(":duty",duty);
    }

    //全都查询
    else if(!ui->search_staff_ID_lineEdit->text().isEmpty()\
            &&!ui->search_date_lineEdit_2->text().isEmpty()\
            &&!ui->search_duty_lineEdit_3->text().isEmpty())
    {
       query.prepare("select*\
                     from View_cash_money\
                     where View_cash_money.Stno=:stno\
                           and View_cash_money.Duty=:duty\
                           and convert(date,View_cash_money.Cdate)=cdate");

                stno=ui->search_staff_ID_lineEdit->text();
                duty=ui->search_duty_lineEdit_3->text().toUInt();
                date=QDate::fromString(ui->search_date_lineEdit_2->text(),"yyyy-MM-dd");
                cdate=date.toString();
                query.bindValue(":duty",duty);
                query.bindValue(":stno",stno);
                query.bindValue(":cdate",cdate);

    }

    //查询编号和日期
    else if(!ui->search_staff_ID_lineEdit->text().isEmpty()\
            &&!ui->search_date_lineEdit_2->text().isEmpty()\
            &&ui->search_duty_lineEdit_3->text().isEmpty())
    {
        query.prepare("select*\
                      from View_cash_money\
                      where View_cash_money.Stno=:stno\
                            and convert(date,View_cash_money.Cdate)=cdate");

                 stno=ui->search_staff_ID_lineEdit->text();
                 date=QDate::fromString(ui->search_date_lineEdit_2->text(),"yyyy-MM-dd");
                 cdate=date.toString();
                 query.bindValue(":duty",duty);
                 query.bindValue(":cdate",cdate);

    }
    //查询编号和班次
    else if(!ui->search_staff_ID_lineEdit->text().isEmpty()\
            &&ui->search_date_lineEdit_2->text().isEmpty()\
            &&!ui->search_duty_lineEdit_3->text().isEmpty())
    {
        query.prepare("select*\
                      from View_cash_money\
                      where View_cash_money.Stno=:stno\
                            and View_cash_money.Duty=:duty");
                 stno=ui->search_staff_ID_lineEdit->text();
                 duty=ui->search_duty_lineEdit_3->text().toUInt();
                 query.bindValue(":duty",duty);
                 query.bindValue(":stno",stno);

    }

    //查询日期和班次
    else if(ui->search_staff_ID_lineEdit->text().isEmpty()\
            &&!ui->search_date_lineEdit_2->text().isEmpty()\
            &&!ui->search_duty_lineEdit_3->text().isEmpty())
    {
        query.prepare("select*\
                      from View_cash_money\
                      where View_cash_money.Duty=:duty\
                            and convert(date,View_cash_money.Cdate)=cdate");
                 duty=ui->search_duty_lineEdit_3->text().toUInt();
                 date=QDate::fromString(ui->search_date_lineEdit_2->text(),"yyyy-MM-dd");
                 cdate=date.toString();
                 query.bindValue(":duty",duty);
                 query.bindValue(":cdate",cdate);

    }

    //无限制查询
    else{
        query.prepare("select*from View_cash_money");
    }
    if(query.exec())
    {
        QSqlQueryModel *model=new QSqlQueryModel();
        model->setQuery(query);
        //将查询结果输出到tableview中
       ui->show_cash_tableView->setModel(model);
       //表格宽度自适应
       ui->show_cash_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

       QMessageBox::information(this,"success","查询成功！");
       return;
    }
    else
    {
        QMessageBox::critical(this,"error","查询失败！");

        return;
    }
}

void HomeWindow::on_statement_pushButton_5_clicked()
{
    dbhelper->disconnectDatabase();
    emit back_from_home();
    qDebug()<<"发出返回信号";
}
