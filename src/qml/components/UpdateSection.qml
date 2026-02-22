import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

ColumnLayout {
    id: root

    required property string title
    required property string iconSource
    property var updateList: []
    property int updateCount: updateList.length
    property bool expanded: updateCount > 0
    property bool showVersionArrow: true

    spacing: 0
    Layout.fillWidth: true

    // Clickable header
    QQC2.ItemDelegate {
        Layout.fillWidth: true

        contentItem: RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Icon {
                source: root.iconSource
                implicitWidth: Kirigami.Units.iconSizes.small
                implicitHeight: Kirigami.Units.iconSizes.small
            }

            QQC2.Label {
                text: root.title
                font.bold: true
                Layout.fillWidth: true
            }

            QQC2.Label {
                text: root.updateCount > 0
                    ? i18np("1 update", "%1 updates", root.updateCount)
                    : i18n("Up to date")
                color: root.updateCount > 0
                    ? Kirigami.Theme.neutralTextColor
                    : Kirigami.Theme.positiveTextColor
            }

            Kirigami.Icon {
                source: root.expanded ? "arrow-up" : "arrow-down"
                implicitWidth: Kirigami.Units.iconSizes.small
                implicitHeight: Kirigami.Units.iconSizes.small
            }
        }

        onClicked: root.expanded = !root.expanded
    }

    // Collapsible package list
    ColumnLayout {
        visible: root.expanded && root.updateCount > 0
        Layout.fillWidth: true
        Layout.leftMargin: Kirigami.Units.largeSpacing * 2
        spacing: 0

        Repeater {
            model: root.updateList

            QQC2.ItemDelegate {
                Layout.fillWidth: true
                enabled: false

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    QQC2.Label {
                        text: modelData.name
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }

                    QQC2.Label {
                        visible: root.showVersionArrow && modelData.currentVersion !== ""
                        text: modelData.currentVersion ?? ""
                        color: Kirigami.Theme.disabledTextColor
                        font.family: "monospace"
                    }

                    QQC2.Label {
                        visible: root.showVersionArrow && modelData.currentVersion !== "" && modelData.newVersion !== ""
                        text: "→"
                        color: Kirigami.Theme.disabledTextColor
                    }

                    QQC2.Label {
                        visible: modelData.newVersion !== ""
                        text: modelData.newVersion ?? ""
                        color: Kirigami.Theme.disabledTextColor
                        font.family: "monospace"
                    }
                }
            }
        }
    }
}
