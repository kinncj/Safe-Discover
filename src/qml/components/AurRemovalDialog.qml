import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import ca.kinncj.SafeDiscover

Kirigami.Dialog {
    id: dialog

    property int packageIndex: -1
    property string packageName: ""
    property int currentStep: 1

    title: i18n("Remove AUR Package")
    preferredWidth: Kirigami.Units.gridUnit * 25

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.largeSpacing

        // Step indicators
        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Repeater {
                model: [i18n("Remove Package"), i18n("Check Orphans"), i18n("Clean Up")]

                delegate: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    Rectangle {
                        width: Kirigami.Units.gridUnit
                        height: width
                        radius: width / 2
                        color: (index + 1) <= dialog.currentStep
                            ? Kirigami.Theme.highlightColor
                            : Kirigami.Theme.disabledTextColor

                        QQC2.Label {
                            anchors.centerIn: parent
                            text: index + 1
                            color: "white"
                            font.pointSize: Kirigami.Theme.smallFont.pointSize
                        }
                    }

                    QQC2.Label {
                        text: modelData
                        font.bold: (index + 1) === dialog.currentStep
                        color: (index + 1) <= dialog.currentStep
                            ? Kirigami.Theme.textColor
                            : Kirigami.Theme.disabledTextColor
                    }

                    Kirigami.Separator {
                        Layout.fillWidth: index < 2
                        visible: index < 2
                    }
                }
            }
        }

        Kirigami.Separator { Layout.fillWidth: true }

        // Step 1: Confirm removal
        ColumnLayout {
            visible: dialog.currentStep === 1
            spacing: Kirigami.Units.smallSpacing

            QQC2.Label {
                text: i18n("Remove %1 and its unneeded dependencies?", dialog.packageName)
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }

            QQC2.Button {
                text: i18n("Remove Package")
                icon.name: "edit-delete"
                onClicked: {
                    AurBackend.confirmRemove(dialog.packageIndex)
                }
            }
        }

        // Step 2: Orphan check results
        ColumnLayout {
            visible: dialog.currentStep === 2
            spacing: Kirigami.Units.smallSpacing

            QQC2.Label {
                text: AurBackend.orphanPackages.length > 0
                    ? i18n("Found %1 orphan package(s):", AurBackend.orphanPackages.length)
                    : i18n("No orphan packages found.")
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }

            QQC2.ScrollView {
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.gridUnit * 6
                visible: AurBackend.orphanPackages.length > 0

                QQC2.TextArea {
                    readOnly: true
                    text: AurBackend.orphanPackages.join("\n")
                    font.family: "monospace"
                }
            }

            RowLayout {
                spacing: Kirigami.Units.smallSpacing

                QQC2.Button {
                    text: i18n("Remove Orphans")
                    icon.name: "edit-delete"
                    visible: AurBackend.orphanPackages.length > 0
                    onClicked: {
                        dialog.currentStep = 3
                        AurBackend.removeOrphans()
                    }
                }

                QQC2.Button {
                    text: i18n("Skip")
                    onClicked: dialog.close()
                }
            }
        }

        // Step 3: Cleanup result
        ColumnLayout {
            visible: dialog.currentStep === 3
            spacing: Kirigami.Units.smallSpacing

            QQC2.Label {
                text: i18n("Cleaning up orphan packages...")
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }

            QQC2.BusyIndicator {
                running: dialog.currentStep === 3
                visible: running
            }
        }
    }

    Connections {
        target: AurBackend

        function onRemovalStepCompleted(step, success, message) {
            if (step === 1 && success) {
                dialog.currentStep = 2
            } else if (step === 3) {
                dialog.close()
            }
        }
    }
}
