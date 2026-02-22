#pragma once

#include <QObject>
#include <QVariantList>

class CommandRunner;
class PacmanBackend;
class AurBackend;
class FlatpakBackend;
class FirmwareBackend;

class UpdateManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(int currentStep READ currentStep NOTIFY currentStepChanged)
    Q_PROPERTY(int totalSteps READ totalSteps CONSTANT)
    Q_PROPERTY(QString currentStepLabel READ currentStepLabel NOTIFY currentStepLabelChanged)
    Q_PROPERTY(int pacmanUpdates READ pacmanUpdates NOTIFY pacmanUpdatesChanged)
    Q_PROPERTY(int aurUpdates READ aurUpdates NOTIFY aurUpdatesChanged)
    Q_PROPERTY(int flatpakUpdates READ flatpakUpdates NOTIFY flatpakUpdatesChanged)
    Q_PROPERTY(int firmwareUpdates READ firmwareUpdates NOTIFY firmwareUpdatesChanged)
    Q_PROPERTY(QVariantList pacmanUpdateList READ pacmanUpdateList NOTIFY pacmanUpdatesChanged)
    Q_PROPERTY(QVariantList aurUpdateList READ aurUpdateList NOTIFY aurUpdatesChanged)
    Q_PROPERTY(QVariantList flatpakUpdateList READ flatpakUpdateList NOTIFY flatpakUpdatesChanged)
    Q_PROPERTY(QVariantList firmwareUpdateList READ firmwareUpdateList NOTIFY firmwareUpdatesChanged)
    Q_PROPERTY(bool checking READ checking NOTIFY checkingChanged)

public:
    explicit UpdateManager(CommandRunner *runner,
                           PacmanBackend *pacman,
                           AurBackend *aur,
                           FlatpakBackend *flatpak,
                           FirmwareBackend *firmware,
                           QObject *parent = nullptr);

    bool running() const { return m_running; }
    int currentStep() const { return m_currentStep; }
    int totalSteps() const { return 4; }
    QString currentStepLabel() const { return m_currentStepLabel; }
    int pacmanUpdates() const { return m_pacmanUpdateList.size(); }
    int aurUpdates() const { return m_aurUpdateList.size(); }
    int flatpakUpdates() const { return m_flatpakUpdateList.size(); }
    int firmwareUpdates() const { return m_firmwareUpdateList.size(); }
    QVariantList pacmanUpdateList() const { return m_pacmanUpdateList; }
    QVariantList aurUpdateList() const { return m_aurUpdateList; }
    QVariantList flatpakUpdateList() const { return m_flatpakUpdateList; }
    QVariantList firmwareUpdateList() const { return m_firmwareUpdateList; }
    bool checking() const { return m_checking; }

    Q_INVOKABLE void checkAllUpdates();
    Q_INVOKABLE void runSafeUpdate();
    Q_INVOKABLE void cancel();

    static QVariantList parsePacmanQuOutput(const QString &output);
    static QVariantList parseFlatpakUpdatesOutput(const QString &output);
    static QVariantList parseFirmwareUpdatesJson(const QString &json);

Q_SIGNALS:
    void runningChanged();
    void currentStepChanged();
    void currentStepLabelChanged();
    void pacmanUpdatesChanged();
    void aurUpdatesChanged();
    void flatpakUpdatesChanged();
    void firmwareUpdatesChanged();
    void checkingChanged();
    void updateFinished(bool success, const QString &summary);
    void stepOutput(const QString &line, bool isStderr);

private:
    void runStep(int step);
    void setStepLabel(const QString &label);

    CommandRunner *m_runner;
    PacmanBackend *m_pacman;
    AurBackend *m_aur;
    FlatpakBackend *m_flatpak;
    FirmwareBackend *m_firmware;

    bool m_running = false;
    bool m_checking = false;
    bool m_cancelled = false;
    int m_currentStep = 0;
    QString m_currentStepLabel;
    QVariantList m_pacmanUpdateList;
    QVariantList m_aurUpdateList;
    QVariantList m_flatpakUpdateList;
    QVariantList m_firmwareUpdateList;
};
