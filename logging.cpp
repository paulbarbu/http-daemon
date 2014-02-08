#include "logging.h"

#ifndef Q_OS_WIN32
#include <syslog.h>
#endif

#include <QByteArray>
#include <QString>

#ifdef Q_OS_WIN32
#include <stdio.h>
#include <stdarg.h>

void syslog(int priority, const char *format, ...){
    Q_UNUSED(priority);

    va_list arg_list;
    va_start(arg_list, format);

    vprintf(format, arg_list);

    va_end(arg_list);
}
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
void qtOutputToLog(QtMsgType type, const QMessageLogContext &context, const QString &m)
{
    Q_UNUSED(context);

    QByteArray localMsg = m.toLocal8Bit();
    const char *msg = localMsg.data();
#else
void qtOutputToLog(QtMsgType type, const char *msg)
{
#endif

    switch(type){
        case QtDebugMsg:
            syslog(LOG_DEBUG, "Qt debug: %s", msg);
            break;
        case QtWarningMsg:
            syslog(LOG_WARNING, "Qt warning: %s", msg);
            break;
        case QtCriticalMsg:
            syslog(LOG_CRIT, "Qt critical: %s", msg);
            break;
        case QtFatalMsg:
            syslog(LOG_ALERT, "Qt fatal: %s", msg);
            exit(1);
            break;
        default:
            syslog(LOG_INFO, "Qt info: %s", msg);
            break;
    }
}
