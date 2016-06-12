#include "AssetCode2Type.h"

AssetCode2Type *AssetCode2Type::instance = nullptr;
AssetCode2Type::AssetCode2Type() {
    code2typeTable.insert(STR("����ӯ"), STR("���һ���"));
    code2typeTable[STR("cash")] = STR("�ֽ�");
}

AssetCode2Type::~AssetCode2Type() {

}

QString AssetCode2Type::getAssetType(const QString assetCode) {
    if (code2typeTable.end() != code2typeTable.find(assetCode)) {
        return code2typeTable[assetCode];
    } else if (STR("sh.") == assetCode.left(3) || STR("sz.") == assetCode.left(3)) {
        if (STR("60") == assetCode.mid(3, 2)
         || STR("00") == assetCode.mid(3, 2)
         || STR("30") == assetCode.mid(3, 2)) {
            return STR("��Ʊ");
        }
    } else {
        return STR("");
    }
    return STR("");
}
