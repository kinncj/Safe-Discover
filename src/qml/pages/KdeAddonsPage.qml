import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.newstuff as NewStuff

Kirigami.Page {
    id: root

    title: i18n("KDE Add-ons")

    readonly property var categories: [
        { name: i18n("Plasma Widgets"),      knsrc: "plasmoids.knsrc" },
        { name: i18n("Global Themes"),       knsrc: "lookandfeel.knsrc" },
        { name: i18n("Plasma Styles"),       knsrc: "plasma-themes.knsrc" },
        { name: i18n("Color Schemes"),       knsrc: "colorschemes.knsrc" },
        { name: i18n("Icon Themes"),         knsrc: "icons.knsrc" },
        { name: i18n("Cursor Themes"),       knsrc: "xcursor.knsrc" },
        { name: i18n("Window Decorations"),  knsrc: "window-decorations.knsrc" },
        { name: i18n("KWin Effects"),        knsrc: "kwineffect.knsrc" },
        { name: i18n("KWin Scripts"),        knsrc: "kwinscripts.knsrc" },
        { name: i18n("Wallpapers"),          knsrc: "wallpaper.knsrc" },
        { name: i18n("Splash Screens"),      knsrc: "ksplash.knsrc" }
    ]

    header: QQC2.ToolBar {
        contentItem: RowLayout {
            spacing: Kirigami.Units.smallSpacing

            QQC2.Label {
                text: i18n("Category:")
            }

            QQC2.ComboBox {
                id: categoryCombo
                Layout.fillWidth: true
                model: root.categories.map(c => c.name)
                onCurrentIndexChanged: loader.reload()
            }
        }
    }

    Loader {
        id: loader
        anchors.fill: parent
        active: true

        function reload() {
            active = false;
            active = true;
        }

        sourceComponent: NewStuff.Page {
            configFile: root.categories[categoryCombo.currentIndex].knsrc
        }
    }
}
