import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import ca.kinncj.SafeDiscover

Kirigami.ApplicationWindow {
    id: root

    title: i18n("Safe Discover")
    minimumWidth: Kirigami.Units.gridUnit * 40
    minimumHeight: Kirigami.Units.gridUnit * 30

    globalDrawer: Kirigami.GlobalDrawer {
        title: i18n("Safe Discover")
        titleIcon: "system-software-install"
        isMenu: false
        modal: false

        actions: [
            Kirigami.Action {
                text: i18n("Pacman")
                icon.name: "package-x-generic"
                onTriggered: pageStack.replace(repoPageComponent)
            },
            Kirigami.Action {
                text: i18n("AUR")
                icon.name: "package-available"
                enabled: ToolChecker.paruAvailable
                onTriggered: pageStack.replace(aurPageComponent)
            },
            Kirigami.Action {
                text: i18n("Flatpak")
                icon.name: "flatpak-discover"
                enabled: ToolChecker.flatpakAvailable
                onTriggered: pageStack.replace(flatpakPageComponent)
            },
            Kirigami.Action {
                text: i18n("Firmware")
                icon.name: "device-notifier"
                enabled: ToolChecker.fwupdmgrAvailable
                onTriggered: pageStack.replace(firmwarePageComponent)
            },
            Kirigami.Action {
                text: i18n("KDE Add-ons")
                icon.name: "preferences-desktop-plasma"
                onTriggered: pageStack.replace(kdeAddonsPageComponent)
            },
            Kirigami.Action {
                separator: true
            },
            Kirigami.Action {
                text: i18n("Updates")
                icon.name: "update-none"
                onTriggered: pageStack.replace(updatesPageComponent)
            },
            Kirigami.Action {
                separator: true
            },
            Kirigami.Action {
                text: i18n("Settings")
                icon.name: "configure"
                onTriggered: pageStack.replace(settingsPageComponent)
            }
        ]
    }

    pageStack.initialPage: repoPageComponent

    Component {
        id: repoPageComponent
        RepoPage {}
    }

    Component {
        id: aurPageComponent
        AurPage {}
    }

    Component {
        id: flatpakPageComponent
        FlatpakPage {}
    }

    Component {
        id: firmwarePageComponent
        FirmwarePage {}
    }

    Component {
        id: kdeAddonsPageComponent
        KdeAddonsPage {}
    }

    Component {
        id: updatesPageComponent
        UpdatesPage {}
    }

    Component {
        id: settingsPageComponent
        SettingsPage {}
    }

    // Status banner for pacman lock
    footer: StatusBanner {
        visible: CommandRunner.pacmanLocked
        text: i18n("Pacman database is locked. Another package operation may be in progress.")
        type: Kirigami.MessageType.Warning
    }
}
