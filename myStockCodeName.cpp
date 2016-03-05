#include "myStockCodeName.h"
#include <QtDebug>
myStockCodeName::myStockCodeName()
    : manager(nullptr), ntRequest("")
{
    manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
}

myStockCodeName::~myStockCodeName() {
    if (manager)
        delete manager;
}

void myStockCodeName::replyFinished(QNetworkReply* data) {
    switch(requestType) {
    case E_RequestTpye::REQUEST_CODE:
        break;
    default:
        break;
    }
}

void myStockCodeName::getStockCode() {
    ntRequest.setUrl("http://quote.eastmoney.com/stocklist.html");
    manager->get(ntRequest);
}

void myStockCodeName::getStockAbbreviation() {

}
