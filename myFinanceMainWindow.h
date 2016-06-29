#ifndef MYFINANCEMAINWINDOW_H
#define MYFINANCEMAINWINDOW_H
#include "myGlobal.h"

#include <QMainWindow>
#include <QtWidgets/QTreeView>
#include <QWidget>
#include <QLabel>
#include <QAction>
#include <QMenu>
#include <QContextMenuEvent>
#include "myAssetModel.h"
#include "myExchangeListModel.h"

#include "myStockCodeName.h"
#include "myDatabaseDatatype.h"
#include "myFinanceTreeVeiwContextMenu.h"

namespace Ui {
class myFinanceMainWindow;
}

class myFinanceMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit myFinanceMainWindow(QWidget *parent = 0);
    ~myFinanceMainWindow();

    void doExchange(const myExchangeUI& uiSetup, bool isSetDisable = false);
    void doDividend(const myDividends &divident, const myAssetData &dbAssetData, bool isIntrest);
    myAssetModel *getAssetModel() { return assetModel;}

private slots:
    void on_exchange_clicked();
    void on_new_account_clicked();

    void on_reflash_clicked();

    void on_updatePrice_clicked();

private:
    Ui::myFinanceMainWindow *ui;
    ///
    /// \brief assetModel --> treeView
    /// \brief exchangeModel --> listView
    ///
    myAssetModel *assetModel;
    //assetChangeDelegate *delegate;
    myExchangeListModel *exchangeModel;

    myStockCodeName *stockCode; //用于更新ui状态
    QLabel statusLabel;

    myFinanceTreeVeiwContextMenu treeViewContextMenu;

    QMenu *editExchange;
    QAction *modifyExchange;
    QAction *deleteExchange;

private slots:
    void priceDataReflashed();
    void codeDataReady();

    /// treeView
    void treeViewContextMenuSlot(const QPoint& pt);


    /// listView
    void listViewContextMenu(const QPoint& pt);
    void modifyExchange_clicked();
    void deleteExchange_clicked();

public:
    /// treeView
    void doChangeAssetDirectly(const myIndexShell *node, changeType type, void *data, const QString &info);
    void doUpDown(const myIndexShell *node, bool isUp);
};

#endif // MYFINANCEMAINWINDOW_H
