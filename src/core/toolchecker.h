#pragma once

#include <QObject>

class ToolChecker : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool paruAvailable READ paruAvailable CONSTANT)
    Q_PROPERTY(bool flatpakAvailable READ flatpakAvailable CONSTANT)
    Q_PROPERTY(bool fwupdmgrAvailable READ fwupdmgrAvailable CONSTANT)
    Q_PROPERTY(bool konsoleAvailable READ konsoleAvailable CONSTANT)

public:
    static ToolChecker *instance();

    bool paruAvailable() const { return m_paruAvailable; }
    bool flatpakAvailable() const { return m_flatpakAvailable; }
    bool fwupdmgrAvailable() const { return m_fwupdmgrAvailable; }
    bool konsoleAvailable() const { return m_konsoleAvailable; }

private:
    explicit ToolChecker(QObject *parent = nullptr);
    static ToolChecker *s_instance;

    bool m_paruAvailable = false;
    bool m_flatpakAvailable = false;
    bool m_fwupdmgrAvailable = false;
    bool m_konsoleAvailable = false;
};
