#ifndef MYFINANCEMAINWINDOW_H
#define MYFINANCEMAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets/QTreeView>
#include <QWidget>
#include <QLabel>
#include "myAssetModel.h"

#include "myStockCodeName.h"

namespace Ui {
class myFinanceMainWindow;
}

class myFinanceMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit myFinanceMainWindow(myStockCodeName *inStockCode, QWidget *parent = 0);
    ~myFinanceMainWindow();

private slots:
    void on_exchange_clicked();
    void on_new_account_clicked();

    void on_reflash_clicked();

    void on_updatePrice_clicked();

private:
    Ui::myFinanceMainWindow *ui;
    myAssetModel *assetModel;

    myStockCodeName *stockCode;
    QLabel statusLabel;

private slots:
    void priceDataReflashed();
    void codeDataReady();
};

#endif // MYFINANCEMAINWINDOW_H
