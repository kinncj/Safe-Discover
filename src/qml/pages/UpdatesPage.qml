import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import ca.kinncj.SafeDiscover

Kirigami.ScrollablePage {
    id: page

    title: i18n("Updates")
    icon.name: "update-none"

    actions: [
        Kirigami.Action {
            text: i18n("Check for Updates")
            icon.name: "view-refresh"
            enabled: !UpdateManager.checking && !UpdateManager.running
            onTriggered: UpdateManager.checkAllUpdates()
        }
    ]

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing

        // Checking indicator
        QQC2.BusyIndicator {
            Layout.alignment: Qt.AlignHCenter
            running: UpdateManager.checking
            visible: running
        }

        // Update sections with package lists
        UpdateSection {
            title: i18n("Pacman")
            iconSource: "package-x-generic"
            updateList: UpdateManager.pacmanUpdateList
        }

        UpdateSection {
            title: i18n("AUR")
            iconSource: "package-available"
            updateList: UpdateManager.aurUpdateList
            visible: ToolChecker.paruAvailable
        }

        UpdateSection {
            title: i18n("Flatpak")
            iconSource: "flatpak-discover"
            updateList: UpdateManager.flatpakUpdateList
            showVersionArrow: false
            visible: ToolChecker.flatpakAvailable
        }

        UpdateSection {
            title: i18n("Firmware")
            iconSource: "device-notifier"
            updateList: UpdateManager.firmwareUpdateList
            visible: ToolChecker.fwupdmgrAvailable
        }

        Kirigami.Separator { Layout.fillWidth: true }

        // Safe Update section
        ColumnLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            QQC2.Label {
                text: i18n("Safe Update")
                font.bold: true
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.2
            }

            QQC2.Label {
                text: i18n("Updates all package sources sequentially: pacman → AUR → Flatpak → Firmware. Stops on any error.")
                wrapMode: Text.Wrap
                color: Kirigami.Theme.disabledTextColor
                Layout.fillWidth: true
            }

            // Progress section
            ColumnLayout {
                Layout.fillWidth: true
                visible: UpdateManager.running
                spacing: Kirigami.Units.smallSpacing

                QQC2.ProgressBar {
                    Layout.fillWidth: true
                    from: 0
                    to: UpdateManager.totalSteps
                    value: UpdateManager.currentStep
                }

                QQC2.Label {
                    text: UpdateManager.currentStepLabel
                    color: Kirigami.Theme.disabledTextColor
                }
            }

            RowLayout {
                spacing: Kirigami.Units.smallSpacing

                QQC2.Button {
                    text: i18n("Run Safe Update")
                    icon.name: "update-none"
                    enabled: !UpdateManager.running && !UpdateManager.checking
                             && (UpdateManager.pacmanUpdates > 0 || UpdateManager.aurUpdates > 0
                                 || UpdateManager.flatpakUpdates > 0 || UpdateManager.firmwareUpdates > 0)
                    onClicked: UpdateManager.runSafeUpdate()
                }

                QQC2.Button {
                    text: i18n("Cancel")
                    icon.name: "dialog-cancel"
                    visible: UpdateManager.running
                    onClicked: UpdateManager.cancel()
                }
            }
        }
    }

    footer: OutputLogPanel {
        id: logPanel
        expanded: UpdateManager.running
    }

    Connections {
        target: UpdateManager

        function onUpdateFinished(success, summary) {
            page.showPassiveNotification(summary, success ? 5000 : 8000)
        }
    }
}
