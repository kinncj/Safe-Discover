import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import ca.kinncj.SafeDiscover

Kirigami.ScrollablePage {
    id: page

    title: i18n("Settings")
    icon.name: "configure"

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing

        FormCard.FormHeader {
            title: i18n("Execution Mode")
        }

        FormCard.FormCard {
            FormCard.FormComboBoxDelegate {
                id: pacmanExecMode
                text: i18n("Pacman")
                description: i18n("How to execute pacman operations")
                model: [i18n("Embedded"), i18n("Terminal")]
                currentIndex: Config.pacmanExecMode
                onCurrentIndexChanged: Config.pacmanExecMode = currentIndex
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormComboBoxDelegate {
                id: aurExecMode
                text: i18n("AUR (paru)")
                description: i18n("How to execute AUR operations")
                model: [i18n("Embedded"), i18n("Terminal")]
                currentIndex: Config.aurExecMode
                onCurrentIndexChanged: Config.aurExecMode = currentIndex
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormComboBoxDelegate {
                id: flatpakExecMode
                text: i18n("Flatpak")
                description: i18n("How to execute Flatpak operations")
                model: [i18n("Embedded"), i18n("Terminal")]
                currentIndex: Config.flatpakExecMode
                onCurrentIndexChanged: Config.flatpakExecMode = currentIndex
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormComboBoxDelegate {
                id: firmwareExecMode
                text: i18n("Firmware")
                description: i18n("How to execute firmware operations")
                model: [i18n("Embedded"), i18n("Terminal")]
                currentIndex: Config.firmwareExecMode
                onCurrentIndexChanged: Config.firmwareExecMode = currentIndex
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormSwitchDelegate {
                id: fallbackToggle
                text: i18n("Fallback to Terminal")
                description: i18n("Open a terminal window if embedded execution fails")
                checked: Config.fallbackToTerminal
                onCheckedChanged: Config.fallbackToTerminal = checked
            }
        }

        FormCard.FormHeader {
            title: i18n("Flatpak")
        }

        FormCard.FormCard {
            FormCard.FormTextFieldDelegate {
                id: defaultRemote
                label: i18n("Default Remote")
                text: Config.defaultRemote
                onTextChanged: Config.defaultRemote = text
            }
        }

        FormCard.FormHeader {
            title: i18n("Search")
        }

        FormCard.FormCard {
            FormCard.FormSpinBoxDelegate {
                id: searchLimit
                label: i18n("Maximum Search Results")
                value: Config.searchResultLimit
                from: 10
                to: 500
                onValueChanged: Config.searchResultLimit = value
            }
        }

        FormCard.FormHeader {
            title: i18n("Logging")
        }

        FormCard.FormCard {
            FormCard.FormSwitchDelegate {
                id: persistLogsToggle
                text: i18n("Persist Logs")
                description: i18n("Save session logs to ~/.local/state/safe-discover/logs/")
                checked: Config.persistLogs
                onCheckedChanged: {
                    Config.persistLogs = checked
                    LogManager.persistLogs = checked
                }
            }
        }

        FormCard.FormHeader {
            title: i18n("Available Tools")
        }

        FormCard.FormCard {
            FormCard.FormTextDelegate {
                text: i18n("paru (AUR)")
                description: ToolChecker.paruAvailable ? i18n("Available") : i18n("Not found")
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextDelegate {
                text: i18n("flatpak")
                description: ToolChecker.flatpakAvailable ? i18n("Available") : i18n("Not found")
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextDelegate {
                text: i18n("fwupdmgr")
                description: ToolChecker.fwupdmgrAvailable ? i18n("Available") : i18n("Not found")
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextDelegate {
                text: i18n("konsole")
                description: ToolChecker.konsoleAvailable ? i18n("Available") : i18n("Not found")
            }
        }
    }
}
