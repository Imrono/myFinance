#ifndef MYFINANCETREEVEIWCONTEXTMENU_H
#define MYFINANCETREEVEIWCONTEXTMENU_H

#include <QMenu>
#include <QAction>

#include "myGlobal.h"
#include "myAssetNode.h"
#include "myDatabaseDatatype.h"

class myFinanceMainWindow;
class myFinanceTreeVeiwContextMenu : public QObject
{
    Q_OBJECT
    friend class myFinanceMainWindow;

public:
    myFinanceTreeVeiwContextMenu(QWidget *parent);
    ~myFinanceTreeVeiwContextMenu();

    void treeViewContextMenu(const myIndexShell *node);

private:
    enum upDownMenu {
        HAS_UP   = 1,
        HAS_DOWN = 2,
        HAS_UPDOWN = 3,
        HAS_NONE   = 0
    };

    QMenu *editAsset;

    QAction *modifyAccount;
    QAction *deleteAccount;
    QAction *insertAsset;
    QAction *addAssetList;
    QAction *addExchangeList;

    QAction *upAsset;
    QAction *downAsset;

    QAction *deleteAsset;
    QAction *modifyAsset;
    QAction *buyAsset;
    QAction *sellAsset;
    QAction *transferIn;
    QAction *transferOut;
    QAction *stockBonus;
    QAction *intrests;

    const myIndexShell *currentNode;
    myFinanceMainWindow *parent;

    bool analyzeStockFromFile(const QString &fileName, QList<myAssetData> &assetDataList);
    bool analyzeExchangeFromFile(const QString &fileName, QList<myExchangeData> &exchangeDataList);

private slots:
    void deleteAccount_clicked();
    void modifyAccount_clicked();
    void insertAsset_clicked();
    void addAssetList_clicked();
    void addExchangeList_clicked();

    void upAsset_clicked();
    void downAsset_clicked();

    void deleteAsset_clicked();
    void modifyAsset_clicked();
    void buyAsset_clicked();
    void sellAsset_clicked();
    void transferIn_clicked();
    void transferOut_clicked();
    void stockBonus_clicked();
    void intrests_clicked();

    void doChangeAssetDirectly(changeType type);
    void doUpDown(bool isUp);
    void doExchangeStock(const QString &type);

    void stockBonus_intrests(bool isInterest);

private:
    QString assetCodeWithMarket(const QString &assetCode);
};

#endif // MYFINANCETREEVEIWCONTEXTMENU_H
