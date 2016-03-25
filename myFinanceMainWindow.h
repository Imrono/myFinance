#ifndef MYFINANCEMAINWINDOW_H
#define MYFINANCEMAINWINDOW_H

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
#include "assetChangeDelegate.h"
#include "myDatabaseDatatype.h"

namespace Ui {
class myFinanceMainWindow;
}

class myFinanceMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit myFinanceMainWindow(myStockCodeName *inStockCode, QWidget *parent = 0);
    ~myFinanceMainWindow();

    myAssetModel *getAssetModel() {
        return assetModel;
    }
    myStockCodeName *getStockCode() {
        return stockCode;
    }

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

    bool doExchange();

    QMenu *editAsset;
    QAction *deleteAsset;
    QAction *insertAsset;
    QAction *modifyAsset;
    QAction *upAsset;
    QAction *downAsset;

    void contextMenuEvent(QContextMenuEvent *event);

private slots:
    void priceDataReflashed();
    void codeDataReady();
    void deleteAsset_clicked();
    void insertAsset_clicked();
    void modifyAsset_clicked();
    void upAsset_clicked();
    void downAsset_clicked();

    void treeViewContextMenu(const QPoint& pt);

private:
    void doChangeAssetDirectly(changeType type);

    enum upDownMenu {
        HAS_UP   = 1,
        HAS_DOWN = 2,
        HAS_UPDOWN = 3,
        HAS_NONE   = 0
    };
    void doUpDown(bool isUp);
};

#endif // MYFINANCEMAINWINDOW_H
