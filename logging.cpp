#include <syslog.h>

#include <QByteArray>
#include <QString>

#include "logging.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
void qtOutputToLog(QtMsgType type, const QMessageLogContext &context, const QString &m)
{
    Q_UNUSED(context);

    QByteArray localMsg = m.toLocal8Bit();
    char *msg = localMsg.data();
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
            abort();
    }
}
