import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import ca.kinncj.SafeDiscover

Kirigami.ScrollablePage {
    id: page

    title: i18n("Firmware Updates")
    icon.name: "device-notifier"

    actions: [
        Kirigami.Action {
            text: i18n("Refresh")
            icon.name: "view-refresh"
            onTriggered: {
                FirmwareBackend.refreshMetadata()
                FirmwareBackend.loadDevices()
                FirmwareBackend.checkUpdates()
            }
        }
    ]

    header: ColumnLayout {
        spacing: 0

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: FirmwareBackend.errorMessage.length > 0
            text: FirmwareBackend.errorMessage
            type: Kirigami.MessageType.Error
            showCloseButton: true
        }

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: FirmwareBackend.updateCount > 0
            text: i18np("1 firmware update available", "%1 firmware updates available", FirmwareBackend.updateCount)
            type: Kirigami.MessageType.Positive
        }
    }

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing

        QQC2.BusyIndicator {
            Layout.alignment: Qt.AlignHCenter
            running: FirmwareBackend.loading
            visible: running
        }

        Repeater {
            model: FirmwareBackend

            delegate: FirmwareDeviceCard {
                Layout.fillWidth: true
                deviceName: model.deviceName
                vendor: model.vendor
                currentVersion: model.currentVersion
                updateVersion: model.updateVersion
                hasUpdate: model.hasUpdate
                summary: model.summary
                plugin: model.plugin

                onUpdateRequested: FirmwareBackend.updateDevice(index)
            }
        }

        Kirigami.PlaceholderMessage {
            Layout.fillWidth: true
            visible: FirmwareBackend.rowCount() === 0 && !FirmwareBackend.loading
            text: i18n("No firmware devices found")
            explanation: i18n("Click Refresh to scan for devices")
            icon.name: "device-notifier"
        }
    }

    footer: OutputLogPanel {
        id: logPanel
    }

    Component.onCompleted: {
        FirmwareBackend.loadDevices()
        FirmwareBackend.checkUpdates()
    }

    Connections {
        target: FirmwareBackend

        function onOperationFinished(success, message) {
            page.showPassiveNotification(message, success ? 3000 : 5000)
        }
    }
}
