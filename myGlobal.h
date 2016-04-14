#ifndef MYGLOBAL_H
#define MYGLOBAL_H

#ifdef Q_OS_LINUX
#   define  STR(x)  QString(x)
#else
#   define  STR(x)  QString::fromLocal8Bit(x)
#endif

class myGlobal
{
public:
    myGlobal();
};

#endif // MYGLOBAL_H
