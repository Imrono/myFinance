#ifndef MYASSETMODEL_H
#define MYASSETMODEL_H
#include "myGlobal.h"

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

    bool doDividend(const myDividends &divident, const myAssetData &nodeAssetData, myExchangeData &exchangeData);
    bool doExchange(const myExchangeData &exchangeData, bool reflash = true);
    bool checkExchange(const myExchangeData &data, QString &abnormalInfo);
    bool doReflashData(bool isAccount = true, bool isAsset = true);

    void doUpdatePrice();
    float doGetTotalAsset();
    float doGetSecurityAsset();

    void doReflash();
    bool doChangeAssetDirectly(const myAssetNode *node, changeType type, QVariant data);
    bool doInsertAccount(myAccountData data);

    bool doUpDown(bool isUp, const myAssetNode *node);

    const myAccountAssetRootNode &getRootNode() { return root;}
    myAssetNode *nodeFromIndex(const QModelIndex &index) const;

signals:
    void priceDataReflashed();

private:
    ///
    /// \brief rootNode:提供帐户与资产的关系
    /// \brief stockPrice:提供股票的价格
    ///
    myAccountAssetRootNode root;
    myStockPrice stockPrice;

    float currentPrice(const QMap<QString, sinaRealTimeData> *priceMap, const QString assetCode) const;
    void qDebugNodeData();

private slots:
    void updatePriceFinish();
};

#endif // MYASSETMODEL_H
