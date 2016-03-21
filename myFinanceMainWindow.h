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
    myExchangeListModel *exchangeModel;

    myStockCodeName *stockCode; //用于更新ui状态
    QLabel statusLabel;

    bool doExchange();

    QMenu *changeAsset;
    QAction *deleteAsset;
    QAction *insertAsset;
    QAction *modifyAsset;

    void contextMenuEvent(QContextMenuEvent *event);
    void doChangeAssetDirectly(changeType type);

private slots:
    void priceDataReflashed();
    void codeDataReady();
    void deleteAsset_clicked();
    void insertAsset_clicked();
    void modifyAsset_clicked();

    void treeViewContextMenu(const QPoint& pt);
};

#endif // MYFINANCEMAINWINDOW_H
