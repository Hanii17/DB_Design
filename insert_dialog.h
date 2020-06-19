#ifndef INSERT_DIALOG_H
#define INSERT_DIALOG_H

#include <QDialog>
#include"dbdal.h"
namespace Ui {
class insert_Dialog;
}

class insert_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit insert_Dialog(DBDAL *dbh,QWidget *parent = nullptr);
    ~insert_Dialog();
signals:
    void backFromInsert();
private slots:
    void on_insert_pushButton_clicked();

    void on_back_pushButton_2_clicked();

private:
    Ui::insert_Dialog *ui;
    DBDAL *dbhelper;
};

#endif // INSERT_DIALOG_H
