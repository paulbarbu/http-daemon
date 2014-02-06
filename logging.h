#ifndef LOGGING_H
#define LOGGING_H

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
void qtOutputToLog(QtMsgType type, const QMessageLogContext &context, const QString &m);
#else
void qtOutputToLog(QtMsgType type, const char *msg);
#endif

#endif // LOGGING_H
