import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.PromptDialog {
    id: dialog

    property string action: ""
    property string packageName: ""
    property int packageIndex: -1

    signal confirmed(string action, int index)

    title: action === "install" ? i18n("Confirm Installation") : i18n("Confirm Removal")
    subtitle: action === "install"
        ? i18n("Do you want to install %1?", packageName)
        : i18n("Do you want to remove %1?", packageName)

    standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel

    onAccepted: {
        confirmed(action, packageIndex)
    }
}
