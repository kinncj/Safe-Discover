#pragma once

#include <QObject>
#include <QProcess>
#include <QHash>

class CommandRunner : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool pacmanLocked READ pacmanLocked NOTIFY pacmanLockedChanged)

public:
    enum ExecMode {
        Embedded = 0,
        Terminal = 1
    };
    Q_ENUM(ExecMode)

    static CommandRunner *instance();

    bool busy() const;
    bool pacmanLocked() const;

    Q_INVOKABLE int run(const QString &program, const QStringList &args,
                        ExecMode mode = Embedded, int timeoutMs = 60000);
    Q_INVOKABLE int runPrivileged(const QString &program, const QStringList &args,
                                  ExecMode mode = Embedded, int timeoutMs = 60000);
    Q_INVOKABLE void cancel(int jobId);

    // Synchronous convenience for backend parsing
    struct Result {
        int exitCode;
        QString stdout;
        QString stderr;
    };
    Result runSync(const QString &program, const QStringList &args, int timeoutMs = 30000);

Q_SIGNALS:
    void busyChanged();
    void pacmanLockedChanged();
    void outputLine(int jobId, const QString &line, bool isStderr);
    void jobFinished(int jobId, int exitCode, const QString &errorString);

private:
    explicit CommandRunner(QObject *parent = nullptr);
    void checkPacmanLock();
    int nextJobId();

    static CommandRunner *s_instance;
    QHash<int, QProcess *> m_jobs;
    int m_nextJobId = 1;
    bool m_pacmanLocked = false;
};
