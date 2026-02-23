import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.OverlaySheet {
    id: detailsPane

    property string packageName: ""
    property string packageVersion: ""
    property bool packageInstalled: false
    property var details: ({})
    property int packageIndex: -1

    signal installRequested(int index)
    signal removeRequested(int index)

    title: packageName

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.largeSpacing

        // Header
        RowLayout {
            spacing: Kirigami.Units.largeSpacing

            Kirigami.Icon {
                source: "package-x-generic"
                implicitWidth: Kirigami.Units.iconSizes.large
                implicitHeight: Kirigami.Units.iconSizes.large
            }

            ColumnLayout {
                spacing: 0
                QQC2.Label {
                    text: packageName
                    font.bold: true
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.2
                }
                QQC2.Label {
                    text: packageVersion
                    color: Kirigami.Theme.disabledTextColor
                }
            }

            Item { Layout.fillWidth: true }

            QQC2.Button {
                text: packageInstalled ? i18n("Remove") : i18n("Install")
                icon.name: packageInstalled ? "edit-delete" : "download"
                onClicked: {
                    if (packageInstalled) {
                        removeRequested(packageIndex)
                    } else {
                        installRequested(packageIndex)
                    }
                    detailsPane.close()
                }
            }

            QQC2.ToolButton {
                icon.name: "dialog-close"
                text: i18n("Close")
                display: QQC2.AbstractButton.IconOnly
                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
                onClicked: detailsPane.close()
            }
        }

        Kirigami.Separator { Layout.fillWidth: true }

        // Details grid
        GridLayout {
            columns: 2
            columnSpacing: Kirigami.Units.largeSpacing
            rowSpacing: Kirigami.Units.smallSpacing
            Layout.fillWidth: true

            Repeater {
                model: Object.keys(detailsPane.details)

                delegate: Item {
                    // Two children per item placed in the grid
                }
            }

            // Manual detail rows for common fields
            Repeater {
                model: {
                    var items = [];
                    var d = detailsPane.details;
                    var keys = Object.keys(d);
                    for (var i = 0; i < keys.length; i++) {
                        items.push({key: keys[i], value: d[keys[i]]});
                    }
                    return items;
                }

                delegate: ColumnLayout {
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    spacing: 2

                    QQC2.Label {
                        text: modelData.key
                        font.bold: true
                        color: Kirigami.Theme.disabledTextColor
                    }
                    QQC2.Label {
                        text: String(modelData.value)
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
