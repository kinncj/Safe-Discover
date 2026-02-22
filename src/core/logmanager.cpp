#include "logmanager.h"

#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>

LogManager *LogManager::s_instance = nullptr;

LogManager::LogManager(QObject *parent)
    : QObject(parent)
{
}

LogManager *LogManager::instance()
{
    if (!s_instance) {
        s_instance = new LogManager();
    }
    return s_instance;
}

void LogManager::log(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString entry = QStringLiteral("[%1] %2").arg(timestamp, message);

    m_sessionLog.append(entry);
    Q_EMIT sessionLogChanged();
    Q_EMIT newLogEntry(entry);

    if (m_persistLogs) {
        writeToFile(entry);
    }
}

void LogManager::clearSession()
{
    m_sessionLog.clear();
    Q_EMIT sessionLogChanged();
}

void LogManager::setPersistLogs(bool persist)
{
    if (m_persistLogs == persist) {
        return;
    }
    m_persistLogs = persist;
    Q_EMIT persistLogsChanged();

    if (persist) {
        openLogFile();
    } else {
        m_logFile.reset();
    }
}

void LogManager::openLogFile()
{
    QString logDir = QStandardPaths::writableLocation(QStandardPaths::StateLocation)
                     + QStringLiteral("/logs");
    QDir().mkpath(logDir);

    QString filename = logDir + QStringLiteral("/")
                       + QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"))
                       + QStringLiteral(".log");

    m_logFile = std::make_unique<QFile>(filename);
    m_logFile->open(QIODevice::Append | QIODevice::Text);
}

void LogManager::writeToFile(const QString &entry)
{
    if (!m_logFile || !m_logFile->isOpen()) {
        openLogFile();
    }
    if (m_logFile && m_logFile->isOpen()) {
        QTextStream stream(m_logFile.get());
        stream << entry << "\n";
        stream.flush();
    }
}
