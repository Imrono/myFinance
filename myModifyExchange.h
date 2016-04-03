#ifndef MYMODIFYEXCHANGE_H
#define MYMODIFYEXCHANGE_H

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

    void setUI(myExchangeData exchangeData);
    myExchangeData getData() {
        return data;
    }

private slots:
    void on_lineEditCode_textChanged(const QString &str);

    void on_buttonBox_accepted();

    void on_checkBoxRollback_clicked();

private:
    Ui::myModifyExchange *ui;
    myExchangeData data;

    int isRollback;
};

#endif // MYMODIFYEXCHANGE_H
