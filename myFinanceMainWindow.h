#ifndef MYFINANCEMAINWINDOW_H
#define MYFINANCEMAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets/QTreeView>
#include <QWidget>
#include "myAssetModel.h"

#include "myStockCodeName.h"

namespace Ui {
class myFinanceMainWindow;
}

class myFinanceMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit myFinanceMainWindow(QWidget *parent = 0);
    ~myFinanceMainWindow();

private slots:
    void on_exchange_clicked();
    void on_new_account_clicked();

    void on_reflash_clicked();

private:
    Ui::myFinanceMainWindow *ui;
    myAssetModel *assetModel;

    myStockCodeName stockCode;
};

#endif // MYFINANCEMAINWINDOW_H
