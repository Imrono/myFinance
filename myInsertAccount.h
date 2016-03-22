#ifndef MYINSERTACCOUNT_H
#define MYINSERTACCOUNT_H

#include <QDialog>

#include "myDatabaseDatatype.h"

namespace Ui {
class myInsertAccount;
}

class myInsertAccount : public QDialog
{
    Q_OBJECT

public:
    explicit myInsertAccount(QWidget *parent = 0);
    ~myInsertAccount();

    insertAccountData getData() {
        return data;
    }

private slots:
    void on_buttonBox_accepted();

private:
    Ui::myInsertAccount *ui;

    insertAccountData data;
};

#endif // MYINSERTACCOUNT_H
