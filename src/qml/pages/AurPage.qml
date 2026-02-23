import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import ca.kinncj.SafeDiscover

Kirigami.ScrollablePage {
    id: page

    property alias detailsPane: detailsPane

    title: i18n("AUR Packages")
    icon.name: "package-available"

    header: ColumnLayout {
        spacing: 0

        PackageSearchBar {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            onSearchRequested: (query) => AurBackend.search(query)
        }

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: AurBackend.errorMessage.length > 0
            text: AurBackend.errorMessage
            type: Kirigami.MessageType.Error
            showCloseButton: true
        }

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: !ToolChecker.konsoleAvailable
            text: i18n("Konsole is not available. AUR package builds require a terminal emulator.")
            type: Kirigami.MessageType.Warning
        }
    }

    PackageListView {
        id: packageList
        model: AurBackend
        onPackageClicked: (index) => AurBackend.fetchDetails(index)
    }

    PackageDetailsPane {
        id: detailsPane
    }

    ActionConfirmDialog {
        id: confirmDialog
        onConfirmed: (action, index) => {
            if (action === "install") {
                AurBackend.confirmInstall(index)
            } else {
                removalDialog.packageIndex = index
                removalDialog.packageName = confirmDialog.packageName
                removalDialog.currentStep = 1
                removalDialog.open()
            }
        }
    }

    AurRemovalDialog {
        id: removalDialog
    }

    footer: OutputLogPanel {
        id: logPanel
    }

    Connections {
        target: AurBackend

        function onDetailsReady(index, details) {
            var pkg = AurBackend.data(AurBackend.index(index, 0), PackageModel.NameRole)
            var ver = AurBackend.data(AurBackend.index(index, 0), PackageModel.VersionRole)
            var installed = AurBackend.data(AurBackend.index(index, 0), PackageModel.InstalledRole)

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
            AurBackend.install(index)
        }

        function onRemoveRequested(index) {
            AurBackend.remove(index)
        }
    }

    Connections {
        target: packageList

        function onInstallRequested(index) {
            AurBackend.install(index)
        }

        function onRemoveRequested(index) {
            AurBackend.remove(index)
        }
    }
}
