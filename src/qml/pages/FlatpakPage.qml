import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import ca.kinncj.SafeDiscover

Kirigami.ScrollablePage {
    id: page

    title: i18n("Flatpak Applications")
    icon.name: "flatpak-discover"

    header: ColumnLayout {
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            spacing: Kirigami.Units.smallSpacing

            PackageSearchBar {
                Layout.fillWidth: true
                onSearchRequested: (query) => FlatpakBackend.search(query)
            }

            QQC2.ComboBox {
                id: remoteSelector
                model: FlatpakBackend.remotes
                currentIndex: FlatpakBackend.remotes.indexOf(FlatpakBackend.activeRemote)
                onCurrentTextChanged: {
                    if (currentText.length > 0) {
                        FlatpakBackend.activeRemote = currentText
                    }
                }

                QQC2.ToolTip.text: i18n("Flatpak Remote")
                QQC2.ToolTip.visible: hovered
            }
        }

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: FlatpakBackend.errorMessage.length > 0
            text: FlatpakBackend.errorMessage
            type: Kirigami.MessageType.Error
            showCloseButton: true
        }
    }

    PackageListView {
        id: packageList
        model: FlatpakBackend
        onPackageClicked: (index) => FlatpakBackend.fetchDetails(index)
    }

    PackageDetailsPane {
        id: detailsPane
    }

    ActionConfirmDialog {
        id: confirmDialog
        onConfirmed: (action, index) => {
            if (action === "install") {
                FlatpakBackend.confirmInstall(index)
            } else {
                FlatpakBackend.confirmRemove(index)
            }
        }
    }

    footer: OutputLogPanel {
        id: logPanel
    }

    Connections {
        target: FlatpakBackend

        function onDetailsReady(index, details) {
            var pkg = FlatpakBackend.data(FlatpakBackend.index(index, 0), PackageModel.NameRole)
            var ver = FlatpakBackend.data(FlatpakBackend.index(index, 0), PackageModel.VersionRole)
            var installed = FlatpakBackend.data(FlatpakBackend.index(index, 0), PackageModel.InstalledRole)

            detailsPane.packageName = pkg
            detailsPane.packageVersion = ver
            detailsPane.packageInstalled = installed
            detailsPane.details = details
            detailsPane.packageIndex = index
            detailsPane.open()
        }

        function onConfirmationRequired(action, packageName, message) {
            confirmDialog.action = action
            confirmDialog.packageName = packageName
            confirmDialog.subtitle = message
            confirmDialog.packageIndex = packageList.currentIndex
            confirmDialog.open()
        }

        function onOperationFinished(success, message) {
            page.showPassiveNotification(message, success ? 3000 : 5000)
        }
    }

    Connections {
        target: detailsPane

        function onInstallRequested(index) {
            FlatpakBackend.install(index)
        }

        function onRemoveRequested(index) {
            FlatpakBackend.remove(index)
        }
    }
}
