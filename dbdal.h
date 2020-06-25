#ifndef DBDAL_H
#define DBDAL_H
#include<QSqlDatabase>

class DBDAL
{
public:
    DBDAL();
    bool connectDatabase();//打开数据库
    void disconnectDatabase();//关闭数据库

    public:
    QSqlDatabase db;
};

#endif // DBDAL_H
