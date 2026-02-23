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

    function navigateTo(pageComponent) {
        if (pageStack.currentItem) {
            if (pageStack.currentItem.detailsPane) {
                pageStack.currentItem.detailsPane.close()
            }
            if (typeof pageStack.currentItem.cleanup === "function") {
                pageStack.currentItem.cleanup()
            }
        }
        // Pop sub-pages down to one, then replace.
        while (pageStack.depth > 1) {
            pageStack.pop()
        }
        pageStack.replace(pageComponent)
    }

    globalDrawer: Kirigami.GlobalDrawer {
        title: i18n("Safe Discover")
        titleIcon: "system-software-install"
        isMenu: false
        modal: false

        actions: [
            Kirigami.Action {
                text: i18n("Pacman")
                icon.name: "package-x-generic"
                onTriggered: navigateTo(repoPageComponent)
            },
            Kirigami.Action {
                text: i18n("AUR")
                icon.name: "package-available"
                enabled: ToolChecker.paruAvailable
                onTriggered: navigateTo(aurPageComponent)
            },
            Kirigami.Action {
                text: i18n("Flatpak")
                icon.name: "flatpak-discover"
                enabled: ToolChecker.flatpakAvailable
                onTriggered: navigateTo(flatpakPageComponent)
            },
            Kirigami.Action {
                text: i18n("Firmware")
                icon.name: "device-notifier"
                enabled: ToolChecker.fwupdmgrAvailable
                onTriggered: navigateTo(firmwarePageComponent)
            },
            Kirigami.Action {
                text: i18n("KDE Add-ons")
                icon.name: "preferences-desktop-plasma"
                onTriggered: navigateTo(kdeAddonsPageComponent)
            },
            Kirigami.Action {
                separator: true
            },
            Kirigami.Action {
                text: i18n("Updates")
                icon.name: "update-none"
                onTriggered: navigateTo(updatesPageComponent)
            },
            Kirigami.Action {
                separator: true
            },
            Kirigami.Action {
                text: i18n("Settings")
                icon.name: "configure"
                onTriggered: navigateTo(settingsPageComponent)
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
