#include "assetChangeDelegate.h"

assetChangeDelegate::assetChangeDelegate(QObject *parent)
{

}

void assetChangeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
           const QModelIndex &index) const {
    QItemDelegate::paint(painter, option, index);
}
QWidget *assetChangeDelegate::createEditor(QWidget *parent,
           const QStyleOptionViewItem &option,
           const QModelIndex &index) const {
    return nullptr;
}
void assetChangeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {

}
void assetChangeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
           const QModelIndex &index) const {

}
