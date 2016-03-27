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

    bool doExchange(const exchangeData data);
    bool checkExchange(const exchangeData &data, QString &abnormalInfo);
    bool doReflashAssetData();
    void doUpdatePrice();
    float doGetTotalAsset();
    void doReflash();
    bool doChangeAssetDirectly(const myAssetNode *node, changeType type, QVariant data);
    bool doInsertAccount(myAccountData data);

    bool doUpDown(bool isUp, myAssetNode *node);

    myAssetNode *getRootNode() { return &rootNode;}
    myAssetNode *nodeFromIndex(const QModelIndex &index) const;

signals:
    void priceDataReflashed();

private:
    ///
    /// \brief rootNode:提供帐户与资产的关系
    /// \brief stockPrice:提供股票的价格
    ///
    myAssetNode rootNode;
    myStockPrice stockPrice;

    float currentPrice(const QMap<QString, sinaRealTimeData> *priceMap, const QString assetCode) const;
    void qDebugNodeData();

private slots:
    void updatePriceFinish();
};

#endif // MYASSETMODEL_H
