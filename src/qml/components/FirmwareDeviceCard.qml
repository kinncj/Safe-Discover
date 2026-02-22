import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.Card {
    id: deviceCard

    property string deviceName: ""
    property string vendor: ""
    property string currentVersion: ""
    property string updateVersion: ""
    property bool hasUpdate: false
    property string summary: ""
    property string plugin: ""

    signal updateRequested()

    banner.title: deviceName
    banner.titleIcon: "device-notifier"

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            text: summary
            wrapMode: Text.Wrap
            visible: summary.length > 0
            Layout.fillWidth: true
        }

        GridLayout {
            columns: 2
            columnSpacing: Kirigami.Units.largeSpacing
            rowSpacing: Kirigami.Units.smallSpacing
            Layout.fillWidth: true

            QQC2.Label {
                text: i18n("Vendor:")
                font.bold: true
                color: Kirigami.Theme.disabledTextColor
            }
            QQC2.Label { text: vendor || i18n("Unknown") }

            QQC2.Label {
                text: i18n("Current Version:")
                font.bold: true
                color: Kirigami.Theme.disabledTextColor
            }
            QQC2.Label { text: currentVersion || i18n("N/A") }

            QQC2.Label {
                text: i18n("Available Update:")
                font.bold: true
                color: Kirigami.Theme.disabledTextColor
                visible: hasUpdate
            }
            QQC2.Label {
                text: updateVersion
                color: Kirigami.Theme.positiveTextColor
                visible: hasUpdate
            }

            QQC2.Label {
                text: i18n("Plugin:")
                font.bold: true
                color: Kirigami.Theme.disabledTextColor
            }
            QQC2.Label { text: plugin || i18n("N/A") }
        }
    }

    footer: QQC2.ToolBar {
        visible: hasUpdate

        RowLayout {
            anchors.fill: parent

            Item { Layout.fillWidth: true }

            QQC2.Button {
                text: i18n("Update Firmware")
                icon.name: "update-none"
                onClicked: deviceCard.updateRequested()
            }
        }
    }
}
