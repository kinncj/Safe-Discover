#pragma once

#include <QObject>
#include <QStringList>
#include <QFile>

class LogManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStringList sessionLog READ sessionLog NOTIFY sessionLogChanged)
    Q_PROPERTY(bool persistLogs READ persistLogs WRITE setPersistLogs NOTIFY persistLogsChanged)

public:
    static LogManager *instance();

    QStringList sessionLog() const { return m_sessionLog; }
    bool persistLogs() const { return m_persistLogs; }
    void setPersistLogs(bool persist);

    Q_INVOKABLE void log(const QString &message);
    Q_INVOKABLE void clearSession();

Q_SIGNALS:
    void sessionLogChanged();
    void persistLogsChanged();
    void newLogEntry(const QString &entry);

private:
    explicit LogManager(QObject *parent = nullptr);
    void writeToFile(const QString &entry);
    void openLogFile();

    static LogManager *s_instance;
    QStringList m_sessionLog;
    bool m_persistLogs = false;
    std::unique_ptr<QFile> m_logFile;
};
