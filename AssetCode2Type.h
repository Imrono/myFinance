#ifndef ASSETCODE2TYPE_H
#define ASSETCODE2TYPE_H

#include <QString>
#include <QMap>

#include "myGlobal.h"

class AssetCode2Type
{
public:
    ~AssetCode2Type();

    QString getAssetType(const QString assetCode);

    static AssetCode2Type *getInstance() {
        if (instance)
            return instance;
        else
            return initial();
    }
    static AssetCode2Type *initial() {
        if (instance)
            return instance;
        else
            return new AssetCode2Type();
    }

private:
    AssetCode2Type();
    static AssetCode2Type *instance;

    QMap<QString, QString> code2typeTable;

    class CGarbo            //它的唯一工作就是在析构函数中删除CSingleton的实例
    {
    public:
        ~CGarbo() {
            if(AssetCode2Type::instance)
                delete AssetCode2Type::instance;
        }
    };
    static CGarbo Garbo;    //定义一个静态成员变量，程序结束时，系统会自动调用它的析构函数
};

#endif // ASSETCODE2TYPE_H
