import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.SearchField {
    id: searchField

    signal searchRequested(string query)

    placeholderText: i18n("Search packages...")
    autoAccept: false

    onAccepted: {
        if (text.length >= 2) {
            searchRequested(text)
        }
    }

    Keys.onReturnPressed: {
        if (text.length >= 2) {
            searchRequested(text)
        }
    }
}
