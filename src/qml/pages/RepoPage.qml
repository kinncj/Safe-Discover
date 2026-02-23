import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import ca.kinncj.SafeDiscover

Kirigami.ScrollablePage {
    id: page

    property alias detailsPane: detailsPane

    title: i18n("Pacman Packages")
    icon.name: "package-x-generic"

    header: ColumnLayout {
        spacing: 0

        PackageSearchBar {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            onSearchRequested: (query) => PacmanBackend.search(query)
        }

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: PacmanBackend.errorMessage.length > 0
            text: PacmanBackend.errorMessage
            type: Kirigami.MessageType.Error
            showCloseButton: true
        }

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: CommandRunner.pacmanLocked
            text: i18n("Pacman database is locked. Operations are blocked.")
            type: Kirigami.MessageType.Warning
        }
    }

    PackageListView {
        id: packageList
        model: PacmanBackend
        onPackageClicked: (index) => PacmanBackend.fetchDetails(index)
    }

    PackageDetailsPane {
        id: detailsPane
    }

    ActionConfirmDialog {
        id: confirmDialog
        onConfirmed: (action, index) => {
            if (action === "install") {
                PacmanBackend.confirmInstall(index)
            } else {
                PacmanBackend.confirmRemove(index)
            }
        }
    }

    footer: OutputLogPanel {
        id: logPanel
    }

    Connections {
        target: PacmanBackend

        function onDetailsReady(index, details) {
            var pkg = PacmanBackend.data(PacmanBackend.index(index, 0), PackageModel.NameRole)
            var ver = PacmanBackend.data(PacmanBackend.index(index, 0), PackageModel.VersionRole)
            var installed = PacmanBackend.data(PacmanBackend.index(index, 0), PackageModel.InstalledRole)

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
            PacmanBackend.install(index)
        }

        function onRemoveRequested(index) {
            PacmanBackend.remove(index)
        }
    }

    Connections {
        target: packageList

        function onInstallRequested(index) {
            PacmanBackend.install(index)
        }

        function onRemoveRequested(index) {
            PacmanBackend.remove(index)
        }
    }
}
