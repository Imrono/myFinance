#include "assetChangeDelegate.h"
#include "myAssetModel.h"

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
    index.model()->data(index, Qt::DisplayRole);
    static_cast<const myAssetModel*>(index.model())->nodeFromIndex(index);
}
void assetChangeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
           const QModelIndex &index) const {

}
