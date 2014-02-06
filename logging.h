#ifndef LOGGING_H
#define LOGGING_H

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
void qtOutputToLog(QtMsgType type, const QMessageLogContext &context, const QString &m);
#else
void qtOutputToLog(QtMsgType type, const char *msg);
#endif


#ifdef Q_OS_WIN32
    #define openlog(...)

    #define LOG_EMERG 0
    #define LOG_ALERT 1
    #define LOG_CRIT 2
    #define LOG_ERR 3
    #define LOG_WARNING 4
    #define LOG_NOTICE 5
    #define LOG_INFO 6
    #define LOG_DEBUG 7

    void syslog(int priority, const char *format, ...);
#endif

#endif // LOGGING_H
