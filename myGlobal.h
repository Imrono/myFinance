#ifndef MYGLOBAL_H
#define MYGLOBAL_H

#ifdef __linux__
#   define  STR(x)  QString(x)
#   define  nullptr NULL
#else
#   define  STR(x)  QString::fromLocal8Bit(x)
#endif

class myGlobal
{
public:
    myGlobal();
};

#endif // MYGLOBAL_H
