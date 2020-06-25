#include "dbdal.h"
#include<QDebug>
DBDAL::DBDAL()
{
    db = QSqlDatabase::addDatabase("QODBC");
     qDebug()<<"ODBC driver?"<<db.isValid();
     QString dsn = QString::fromLocal8Bit("SQLserverDBS");//数据源名称
    db.setHostName("192.168.242.1");//数据库主机名
    db.setDatabaseName(dsn);//数据源
    db.setUserName("hzySQL");
    db.setPassword("201867gg");
}

bool DBDAL::connectDatabase()
{
    bool ret = db.open();//连接数据库
   return ret;
}

void DBDAL::disconnectDatabase()
{
    db.close();//关闭数据库
    db.removeDatabase("mkdb");
}
