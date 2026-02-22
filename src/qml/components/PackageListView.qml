import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigami.delegates as KD

ListView {
    id: listView

    signal packageClicked(int index)

    clip: true
    currentIndex: -1

    delegate: QQC2.ItemDelegate {
        id: delegateItem

        width: listView.width

        contentItem: RowLayout {
            spacing: Kirigami.Units.smallSpacing

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0

                RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    QQC2.Label {
                        text: model.name
                        font.bold: true
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    QQC2.Label {
                        text: model.version
                        color: Kirigami.Theme.disabledTextColor
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                    }

                    QQC2.Label {
                        text: model.repository
                        color: Kirigami.Theme.linkColor
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                    }
                }

                QQC2.Label {
                    text: model.description
                    elide: Text.ElideRight
                    color: Kirigami.Theme.disabledTextColor
                    Layout.fillWidth: true
                }
            }

            Kirigami.Icon {
                source: "checkmark"
                implicitWidth: Kirigami.Units.iconSizes.small
                implicitHeight: Kirigami.Units.iconSizes.small
                visible: model.installed
                color: Kirigami.Theme.positiveTextColor
            }
        }

        onClicked: {
            listView.currentIndex = index
            listView.packageClicked(index)
        }
    }

    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent
        visible: listView.count === 0 && !listView.model.loading
        text: listView.model.searchQuery ? i18n("No packages found") : i18n("Search for packages")
        icon.name: "package-x-generic"
    }

    QQC2.BusyIndicator {
        anchors.centerIn: parent
        running: listView.model ? listView.model.loading : false
        visible: running
    }
}
