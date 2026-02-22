#include "toolchecker.h"

#include <QStandardPaths>

ToolChecker *ToolChecker::s_instance = nullptr;

ToolChecker::ToolChecker(QObject *parent)
    : QObject(parent)
{
    m_paruAvailable = !QStandardPaths::findExecutable(QStringLiteral("paru")).isEmpty();
    m_flatpakAvailable = !QStandardPaths::findExecutable(QStringLiteral("flatpak")).isEmpty();
    m_fwupdmgrAvailable = !QStandardPaths::findExecutable(QStringLiteral("fwupdmgr")).isEmpty();
    m_konsoleAvailable = !QStandardPaths::findExecutable(QStringLiteral("konsole")).isEmpty();
}

ToolChecker *ToolChecker::instance()
{
    if (!s_instance) {
        s_instance = new ToolChecker();
    }
    return s_instance;
}
