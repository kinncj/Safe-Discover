import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import ca.kinncj.SafeDiscover

ColumnLayout {
    id: logPanel

    property bool expanded: false
    property alias outputText: outputArea.text

    spacing: 0

    QQC2.ToolBar {
        Layout.fillWidth: true

        RowLayout {
            anchors.fill: parent

            QQC2.Label {
                text: i18n("Output Log")
                font.bold: true
            }

            Item { Layout.fillWidth: true }

            QQC2.ToolButton {
                icon.name: "edit-clear-all"
                text: i18n("Clear")
                display: QQC2.AbstractButton.IconOnly
                onClicked: {
                    outputArea.text = ""
                    LogManager.clearSession()
                }

                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
            }

            QQC2.ToolButton {
                icon.name: logPanel.expanded ? "arrow-down" : "arrow-up"
                text: logPanel.expanded ? i18n("Collapse") : i18n("Expand")
                display: QQC2.AbstractButton.IconOnly
                onClicked: logPanel.expanded = !logPanel.expanded

                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
            }
        }
    }

    QQC2.ScrollView {
        Layout.fillWidth: true
        Layout.preferredHeight: logPanel.expanded ? Kirigami.Units.gridUnit * 12 : Kirigami.Units.gridUnit * 4
        visible: logPanel.expanded || outputArea.text.length > 0

        Behavior on Layout.preferredHeight {
            NumberAnimation { duration: 200 }
        }

        QQC2.TextArea {
            id: outputArea
            readOnly: true
            font.family: "monospace"
            font.pointSize: Kirigami.Theme.smallFont.pointSize
            wrapMode: Text.Wrap
            background: Rectangle {
                color: Kirigami.Theme.alternateBackgroundColor
            }
        }
    }

    Connections {
        target: LogManager

        function onNewLogEntry(entry) {
            outputArea.text += entry + "\n"
            // Auto-scroll to bottom
            outputArea.cursorPosition = outputArea.text.length
        }
    }
}
