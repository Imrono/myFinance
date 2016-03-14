#ifndef MYASSETMODEL_H
#define MYASSETMODEL_H

#include <QtSql/QSqlDatabase>
#include <QAbstractItemModel>

#include "myAssetNode.h"
#include "myStockPrice.h"
#include "myDatabaseDatatype.h"

//#include "myFinanceMainWindow.h"
//class myFinanceMainWindow;

class myAssetModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    myAssetModel(QObject *parent);
    ~myAssetModel();

    QModelIndex index(int row, int column,
                      const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const;

    void doExchange(exchangeData data);
    void doReflashAssetData();
    void doUpdatePrice();
    void doReflash();

    myAssetNode *getRootNode() { return rootNode;}

signals:
    void priceDataReflashed();

private:
    myAssetNode *nodeFromIndex(const QModelIndex &index) const;
    myAssetNode *rootNode;

    myStockPrice stockPrice;
   // myFinanceMainWindow *pWin;
    float currentPrice(const QMap<QString, sinaRealTimeData> *priceMap, const QString assetCode) const;
    void qDebugNodeData();

private slots:
    void updatePriceFinish();
};

#endif // MYASSETMODEL_H
