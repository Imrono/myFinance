#ifndef MYGLOBAL_H
#define MYGLOBAL_H

#ifdef __linux__
#   define  STR(x)  QString(x)
#ifdef __x86_64__
#endif
#ifdef __i386__
#   define  nullptr NULL
#endif
#else
#   define  STR(x)  QString::fromLocal8Bit(x)
#endif

class myGlobal
{
public:
    myGlobal();
};

#endif // MYGLOBAL_H
