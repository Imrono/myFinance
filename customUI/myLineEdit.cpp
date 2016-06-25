#include "myLineEdit.h"

#include <QFocusEvent>
#include <QRegExpValidator>
#include <QDebug>

myLineEdit::myLineEdit(QWidget *parent) : QLineEdit(parent)
{
    init();
}
myLineEdit::myLineEdit(const QString &str, const float val, QWidget *parent) : QLineEdit(parent),
    defaultString(str), originalValue(val)
{
    init();
}
myLineEdit::~myLineEdit() {

}

void myLineEdit::init() {
    defaultFont    = this->font();
    defaultPalette = this->palette();

    defaultRegExp.setPattern(".*");
    intRegExp.setPattern("[1-9][0-9]*$");
    decimal3RegExp.setPattern("[1-9][0-9]+(\\.[0-9]{0,3})?$");
    intOrDecimal3 = intRegExp;

    regValidator = new QRegExpValidator(defaultRegExp, this);
    this->setValidator(regValidator);

    this->setText(defaultString);
}

void myLineEdit::setDefaultString(const QString &str) {
    defaultString = str;

    setDefaultStringStyle();
    this->setText(defaultString);
}

void myLineEdit::setOriginalValue(const float val) {
    originalValue = val;
}

/// SLOTS
void myLineEdit::textHasChanged(const QString & text) {
    qDebug() << "myLineEdit" << text;
    if (defaultString == this->text()) {
        currentValue = 0;
    } else {
        currentValue = text.toInt();
    }
}

void myLineEdit::focusInEvent(QFocusEvent *e) {
    this->setPalette(defaultPalette);
    this->setFont(defaultFont);

    if (this->text() == defaultString) {
        this->setText("");
        regValidator->setRegExp(intOrDecimal3);
    }
    return QLineEdit::focusInEvent(e);
}

void myLineEdit::focusOutEvent(QFocusEvent *e) {
    if (this->text() == "") {
        regValidator->setRegExp(defaultRegExp);

        setDefaultStringStyle();
        this->setText(defaultString);
    }
    return QLineEdit::focusOutEvent(e);
}
void myLineEdit::setDefaultStringStyle() {
    QPalette pal;
    pal.setColor(QPalette::Text, QColor("gray"));
    this->setPalette(pal);
    QFont font;
    font.setBold(true);
    font.setItalic(true);
    this->setFont(font);
}

void myLineEdit::setValue(int val) {
    this->setPalette(defaultPalette);
    this->setFont(defaultFont);
    this->setText(QString("%1").arg(val));
}
