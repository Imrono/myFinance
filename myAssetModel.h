#ifndef MYASSETMODEL_H
#define MYASSETMODEL_H

#include <QtSql/QSqlDatabase>
#include <QAbstractItemModel>

#include "myAssetNode.h"
#include "myDatabaseDatatype.h"

//#include "myFinanceMainWindow.h"
//class myFinanceMainWindow;

class myAssetModel : public QAbstractItemModel
{
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
    void doReflash();
    myAssetNode *getRootNode() { return rootNode;}

private:
    myAssetNode *nodeFromIndex(const QModelIndex &index) const;
    myAssetNode *rootNode;

   // myFinanceMainWindow *pWin;
    void qDebugNodeData();
};

#endif // MYASSETMODEL_H
