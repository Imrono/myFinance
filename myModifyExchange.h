#ifndef MYMODIFYEXCHANGE_H
#define MYMODIFYEXCHANGE_H
#include "myGlobal.h"

#include <QDialog>

#include "myDatabaseDatatype.h"

namespace Ui {
class myModifyExchange;
}

class myModifyExchange : public QDialog
{
    Q_OBJECT

public:
    explicit myModifyExchange(QWidget *parent = 0);
    ~myModifyExchange();

    void setUI(const myExchangeData &exchangeData);
    void setUI4Delete();
    bool isRollback();

    myExchangeData getData();

private slots:
    void on_lineEditCode_textChanged(const QString &str);

    void on_buttonBox_accepted();

    void on_spinBoxPrice_valueChanged(double price);

    void on_spinBoxMoney_valueChanged(double money);

private:
    Ui::myModifyExchange *ui;
    myExchangeData data;
};

#endif // MYMODIFYEXCHANGE_H
