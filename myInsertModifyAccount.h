#ifndef MYINSERTACCOUNT_H
#define MYINSERTACCOUNT_H

#include <QDialog>

#include "myDatabaseDatatype.h"

namespace Ui {
class myInsertAccount;
}

class myInsertModifyAccount : public QDialog
{
    Q_OBJECT

public:
    explicit myInsertModifyAccount(QWidget *parent = 0);
    ~myInsertModifyAccount();

    myAccountData getData() {
        return data;
    }
    void setUI(myAccountData accountData);

private slots:
    void on_buttonBox_accepted();

private:
    Ui::myInsertAccount *ui;

    myAccountData data;
};

#endif // MYINSERTACCOUNT_H
