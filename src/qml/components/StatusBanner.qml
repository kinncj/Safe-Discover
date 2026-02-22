import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.InlineMessage {
    id: banner

    property alias text: banner.text

    Layout.fillWidth: true
    visible: false
    showCloseButton: true
}
