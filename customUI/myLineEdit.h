#ifndef MYLINEEDIT_H
#define MYLINEEDIT_H

#include <QLineEdit>
#include <QRegExpValidator>
#include <QFont>
#include <QPalette>

class myLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    myLineEdit(QWidget *parent = 0);
    myLineEdit(const QString &str, const float val, QWidget *parent = 0);
    ~myLineEdit();

    void setDefaultString(const QString &str);
    void setOriginalValue(const float val);
    int getCurrentValue() { return currentValue;}
    void setStyleIntDecimal(bool isInt) {
        intOrDecimal3 = isInt ? intRegExp : decimal3RegExp;
    }
    void setValue(int val);
    void textHasChanged(const QString & text);

private:
    float   originalValue;
    QString defaultString;
    int   currentValue;

    QRegExp intOrDecimal3;
    QRegExp defaultRegExp;
    QRegExp intRegExp;
    QRegExp decimal3RegExp;
    QRegExpValidator *regValidator;

    QFont defaultFont;
    QPalette defaultPalette;


    void init();

protected:
    virtual void focusInEvent(QFocusEvent *e);
    virtual void focusOutEvent(QFocusEvent *e);

    void setDefaultStringStyle();
};

#endif // MYLINEEDIT_H
