#include "commandrunner.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTimer>

CommandRunner *CommandRunner::s_instance = nullptr;

CommandRunner::CommandRunner(QObject *parent)
    : QObject(parent)
{
}

CommandRunner *CommandRunner::instance()
{
    if (!s_instance) {
        s_instance = new CommandRunner();
    }
    return s_instance;
}

bool CommandRunner::busy() const
{
    return !m_jobs.isEmpty();
}

bool CommandRunner::pacmanLocked() const
{
    return m_pacmanLocked;
}

void CommandRunner::checkPacmanLock()
{
    bool locked = QFile::exists(QStringLiteral("/var/lib/pacman/db.lck"));
    if (locked != m_pacmanLocked) {
        m_pacmanLocked = locked;
        Q_EMIT pacmanLockedChanged();
    }
}

int CommandRunner::nextJobId()
{
    return m_nextJobId++;
}

int CommandRunner::run(const QString &program, const QStringList &args,
                       ExecMode mode, int timeoutMs)
{
    checkPacmanLock();

    int jobId = nextJobId();

    if (mode == Terminal) {
        QString konsole = QStandardPaths::findExecutable(QStringLiteral("konsole"));
        if (konsole.isEmpty()) {
            Q_EMIT jobFinished(jobId, -1, QStringLiteral("konsole not found"));
            return jobId;
        }

        QStringList termArgs;
        termArgs << QStringLiteral("--hold") << QStringLiteral("-e");

        QString fullCmd = program;
        for (const auto &arg : args) {
            fullCmd += QStringLiteral(" ") + arg;
        }
        termArgs << fullCmd;

        QProcess::startDetached(konsole, termArgs);
        Q_EMIT jobFinished(jobId, 0, QString());
        return jobId;
    }

    // Embedded mode
    auto *proc = new QProcess(this);
    m_jobs.insert(jobId, proc);

    bool wasBusy = busy();

    connect(proc, &QProcess::readyReadStandardOutput, this, [this, proc, jobId]() {
        while (proc->canReadLine()) {
            QString line = QString::fromUtf8(proc->readLine()).trimmed();
            Q_EMIT outputLine(jobId, line, false);
        }
    });

    connect(proc, &QProcess::readyReadStandardError, this, [this, proc, jobId]() {
        while (proc->canReadLine()) {
            QString line = QString::fromUtf8(proc->readLine()).trimmed();
            Q_EMIT outputLine(jobId, line, true);
        }
    });

    connect(proc, &QProcess::finished, this, [this, proc, jobId](int exitCode, QProcess::ExitStatus status) {
        // Read any remaining output
        QString remaining = QString::fromUtf8(proc->readAllStandardOutput()).trimmed();
        if (!remaining.isEmpty()) {
            for (const auto &line : remaining.split(QLatin1Char('\n'))) {
                Q_EMIT outputLine(jobId, line, false);
            }
        }
        QString remainingErr = QString::fromUtf8(proc->readAllStandardError()).trimmed();
        if (!remainingErr.isEmpty()) {
            for (const auto &line : remainingErr.split(QLatin1Char('\n'))) {
                Q_EMIT outputLine(jobId, line, true);
            }
        }

        QString errorStr;
        if (status == QProcess::CrashExit) {
            errorStr = QStringLiteral("Process crashed");
        }

        m_jobs.remove(jobId);
        proc->deleteLater();
        Q_EMIT jobFinished(jobId, exitCode, errorStr);

        if (m_jobs.isEmpty()) {
            Q_EMIT busyChanged();
        }
        checkPacmanLock();
    });

    if (timeoutMs > 0) {
        QTimer::singleShot(timeoutMs, proc, [this, proc, jobId]() {
            if (m_jobs.contains(jobId)) {
                proc->kill();
                Q_EMIT jobFinished(jobId, -1, QStringLiteral("Timed out"));
            }
        });
    }

    proc->start(program, args);

    if (!proc->waitForStarted(5000)) {
        m_jobs.remove(jobId);
        Q_EMIT jobFinished(jobId, -1, QStringLiteral("Failed to start: ") + proc->errorString());
        proc->deleteLater();
        return jobId;
    }

    if (!wasBusy) {
        Q_EMIT busyChanged();
    }

    return jobId;
}

int CommandRunner::runPrivileged(const QString &program, const QStringList &args,
                                 ExecMode mode, int timeoutMs)
{
    QString helper = QStandardPaths::findExecutable(
        QStringLiteral("safe-discover-helper.sh"),
        {QStringLiteral("/usr/libexec")});

    if (helper.isEmpty()) {
        // Fallback: look next to the binary
        helper = QCoreApplication::applicationDirPath() + QStringLiteral("/../safe-discover-helper.sh");
        if (!QFileInfo::exists(helper)) {
            int jobId = nextJobId();
            Q_EMIT jobFinished(jobId, -1, QStringLiteral("Helper script not found"));
            return jobId;
        }
    }

    QStringList pkexecArgs;
    pkexecArgs << helper << program << args;

    return run(QStringLiteral("pkexec"), pkexecArgs, mode, timeoutMs);
}

void CommandRunner::cancel(int jobId)
{
    if (auto *proc = m_jobs.value(jobId)) {
        proc->terminate();
        QTimer::singleShot(3000, proc, [proc]() {
            if (proc->state() != QProcess::NotRunning) {
                proc->kill();
            }
        });
    }
}

CommandRunner::Result CommandRunner::runSync(const QString &program, const QStringList &args, int timeoutMs)
{
    QProcess proc;
    proc.start(program, args);
    proc.waitForFinished(timeoutMs);

    return Result{
        proc.exitCode(),
        QString::fromUtf8(proc.readAllStandardOutput()),
        QString::fromUtf8(proc.readAllStandardError())
    };
}
