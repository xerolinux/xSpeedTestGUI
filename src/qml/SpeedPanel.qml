import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item {
    id: root

    required property SpeedTestRunner runner
    required property SpeedSettings settings
    required property HistoryModel history
    property PlasmoidInstaller installer: null
    property bool compact: false
    property bool animate: true
    property bool idleDrift: true
    property string page: "main"
    property string settingsTitle: qsTr("Settings")
    property real padding: Kirigami.Units.gridUnit * 1.5
    property real controlsInset: 0

    readonly property var style: view.style

    function toggle(name) {
        page = page === name ? "main" : name;
    }

    implicitWidth: Math.max(view.implicitWidth, settingsPage.implicitWidth, historyPage.implicitWidth) + padding * 2
    implicitHeight: header.implicitHeight + column.spacing + Math.max(view.implicitHeight, settingsPage.implicitHeight, historyPage.implicitHeight) + padding * 2

    ColumnLayout {
        id: column
        anchors.fill: parent
        anchors.margins: root.padding
        spacing: Kirigami.Units.largeSpacing

        RowLayout {
            id: header
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.smallSpacing - root.padding
            Layout.rightMargin: Kirigami.Units.smallSpacing - root.padding + root.controlsInset
            spacing: 0

            Kirigami.Heading {
                visible: root.page !== "main"
                text: root.page === "history" ? qsTr("Test History") : root.settingsTitle
                level: 2
            }
            Item {
                Layout.fillWidth: true
            }
            QQC2.ToolButton {
                checkable: true
                checked: root.page === "history"
                icon.name: "view-history"
                onClicked: root.toggle("history")
                QQC2.ToolTip.text: qsTr("Test History")
                QQC2.ToolTip.visible: hovered
            }
            QQC2.ToolButton {
                checkable: true
                checked: root.page === "settings"
                icon.name: "configure-symbolic"
                onClicked: root.toggle("settings")
                QQC2.ToolTip.text: qsTr("Settings")
                QQC2.ToolTip.visible: hovered
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            implicitWidth: Math.max(view.implicitWidth, settingsPage.implicitWidth, historyPage.implicitWidth)
            implicitHeight: Math.max(view.implicitHeight, settingsPage.implicitHeight, historyPage.implicitHeight)

            SpeedView {
                id: view
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width
                height: implicitHeight
                visible: root.page === "main"
                runner: root.runner
                settings: root.settings
                compact: root.compact
                animate: root.animate && root.page === "main"
                idleDrift: root.idleDrift
            }

            HistoryPage {
                id: historyPage
                anchors.fill: parent
                visible: root.page === "history"
                settings: root.settings
                history: root.history
            }

            SettingsPage {
                id: settingsPage
                anchors.fill: parent
                visible: root.page === "settings"
                settings: root.settings
                installer: root.installer
                showOpacity: !root.compact
            }
        }
    }
}
