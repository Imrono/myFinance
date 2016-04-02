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

private:
    Ui::myModifyExchange *ui;
    myExchangeData data;
};

#endif // MYMODIFYEXCHANGE_H
