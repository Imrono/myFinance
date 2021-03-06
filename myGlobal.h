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
#   pragma warning (disable:4819)
#endif

// QT_NO_DEBUG_OUTPUT  -> QT qDebug() invalid
// QT_NO_DEBUG -> QT release version macro

#define MY_FT_BLACK(str)   "\033[30m"str"\033[0m"
#define MY_FT_RED(str)     "\033[31m"str"\033[0m"
#define MY_FT_GREEN(str)   "\033[32m"str"\033[0m"
#define MY_FT_YELLOW(str)  "\033[33m"str"\033[0m"
#define MY_FT_BLUE(str)    "\033[34m"str"\033[0m"

#define MY_FT_BOLD_BLACK(str)   "\033[1;30m"str"\033[0m"

#define MY_DEBUG_SQL(str) qDebug() << "\033[34m" << str << "\033[0m"
#define MY_DEBUG_ERROR(str) qWarning() << "\033[31m" << str.toUtf8().data() << "\033[0m"
#define MY_A_TRACE(str) qDebug() << "\033[1;30m" << str << "\033[0m"
#define MY_DEBUG_URL(str) qDebug() << "\033[33m" << str << "\033[0m"
#define MY_DEBUG_ASSET(str) qDebug() << "\033[32m" << str << "\033[0m"
#define MY_DEBUG_NODE_ASSET(node) MY_DEBUG_ASSET(node.toString().toUtf8().data())

#define MSEC_DAY 86400000

class myGlobal
{
public:
    myGlobal();
};

#endif // MYGLOBAL_H
