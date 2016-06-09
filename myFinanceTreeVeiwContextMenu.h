#ifndef MYFINANCETREEVEIWCONTEXTMENU_H
#define MYFINANCETREEVEIWCONTEXTMENU_H

#include <QMenu>
#include <QAction>

#include "myGlobal.h"
#include "myAssetNode.h"

class myFinanceMainWindow;
class myFinanceTreeVeiwContextMenu : public QMenu
{
    Q_OBJECT

public:
    myFinanceTreeVeiwContextMenu(QWidget *parent = 0);

    void treeViewContextMenu(const myAssetNode *node);

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
    QAction *deleteAsset;
    QAction *modifyAsset;
    QAction *buyAsset;
    QAction *sellAsset;
    QAction *transferIn;
    QAction *transferOut;
    QAction *upAsset;
    QAction *downAsset;

    const myAssetNode *currentNode;
    myFinanceMainWindow *parent;

private slots:
    void deleteAccount_clicked();
    void modifyAccount_clicked();
    void insertAsset_clicked();
    void deleteAsset_clicked();
    void modifyAsset_clicked();
    void buyAsset_clicked();
    void sellAsset_clicked();
    void transferIn_clicked();
    void transferOut_clicked();
    void upAsset_clicked();
    void downAsset_clicked();

    void doChangeAssetDirectly(changeType type);
    void doUpDown(bool isUp);
};

#endif // MYFINANCETREEVEIWCONTEXTMENU_H
