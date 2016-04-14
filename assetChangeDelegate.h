#ifndef ASSETCHANGEDELEGATE_H
#define ASSETCHANGEDELEGATE_H
#include "myGlobal.h"

#include <QObject>
#include <QItemDelegate>

class assetChangeDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    assetChangeDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QWidget *createEditor(QWidget *parent,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
               const QModelIndex &index) const;
};

#endif // ASSETCHANGEDELEGATE_H
