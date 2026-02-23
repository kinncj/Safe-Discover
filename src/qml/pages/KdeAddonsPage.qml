import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.newstuff as NewStuff

Kirigami.ScrollablePage {
    id: root

    title: i18n("KDE Add-ons")
    icon.name: "preferences-desktop-plasma"

    readonly property var categories: [
        { name: i18n("Plasma Widgets"),      knsrc: "plasmoids.knsrc",              icon: "plasma" },
        { name: i18n("Global Themes"),       knsrc: "lookandfeel.knsrc",            icon: "preferences-desktop-theme-global" },
        { name: i18n("Plasma Styles"),       knsrc: "plasma-themes.knsrc",          icon: "preferences-desktop-plasma-theme" },
        { name: i18n("Color Schemes"),       knsrc: "colorschemes.knsrc",           icon: "preferences-desktop-color" },
        { name: i18n("Icon Themes"),         knsrc: "icons.knsrc",                  icon: "preferences-desktop-icons" },
        { name: i18n("Cursor Themes"),       knsrc: "xcursor.knsrc",                icon: "preferences-cursors" },
        { name: i18n("Window Decorations"),  knsrc: "window-decorations.knsrc",     icon: "preferences-system-windows-actions" },
        { name: i18n("KWin Effects"),        knsrc: "kwineffect.knsrc",             icon: "preferences-desktop-effects" },
        { name: i18n("KWin Scripts"),        knsrc: "kwinscripts.knsrc",            icon: "preferences-desktop-scripts" },
        { name: i18n("Wallpapers"),          knsrc: "wallpaper.knsrc",              icon: "preferences-desktop-wallpaper" },
        { name: i18n("Splash Screens"),      knsrc: "ksplash.knsrc",               icon: "preferences-splash" }
    ]

    NewStuff.Dialog {
        id: newStuffDialog
        configFile: ""
    }

    GridLayout {
        columns: root.width > Kirigami.Units.gridUnit * 30 ? 3 : 2
        rowSpacing: Kirigami.Units.largeSpacing
        columnSpacing: Kirigami.Units.largeSpacing

        Repeater {
            model: root.categories

            delegate: QQC2.ItemDelegate {
                Layout.fillWidth: true

                contentItem: RowLayout {
                    spacing: Kirigami.Units.largeSpacing

                    Kirigami.Icon {
                        source: modelData.icon
                        implicitWidth: Kirigami.Units.iconSizes.medium
                        implicitHeight: Kirigami.Units.iconSizes.medium
                    }

                    QQC2.Label {
                        text: modelData.name
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }

                    Kirigami.Icon {
                        source: "go-next"
                        implicitWidth: Kirigami.Units.iconSizes.small
                        implicitHeight: Kirigami.Units.iconSizes.small
                        color: Kirigami.Theme.disabledTextColor
                    }
                }

                onClicked: {
                    newStuffDialog.configFile = modelData.knsrc
                    newStuffDialog.open()
                }
            }
        }
    }
}
