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
    property bool settingsOpen: false
    property string settingsTitle: qsTr("Settings")
    property real padding: Kirigami.Units.gridUnit * 1.5
    property real controlsInset: 0

    readonly property var style: view.style

    implicitWidth: (settingsOpen ? page.implicitWidth : view.implicitWidth) + padding * 2
    implicitHeight: column.implicitHeight + padding * 2

    ColumnLayout {
        id: column
        anchors.fill: parent
        anchors.margins: root.padding
        spacing: Kirigami.Units.largeSpacing

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.smallSpacing - root.padding
            Layout.rightMargin: Kirigami.Units.smallSpacing - root.padding + root.controlsInset
            spacing: 0

            Kirigami.Heading {
                visible: root.settingsOpen
                text: root.settingsTitle
                level: 2
            }
            Item {
                Layout.fillWidth: true
            }
            QQC2.ToolButton {
                checkable: true
                checked: root.settingsOpen
                icon.name: "configure-symbolic"
                onClicked: root.settingsOpen = checked
                QQC2.ToolTip.text: qsTr("Settings")
                QQC2.ToolTip.visible: hovered
            }
        }

        SpeedView {
            id: view
            Layout.fillWidth: true
            visible: !root.settingsOpen
            runner: root.runner
            settings: root.settings
            history: root.history
            compact: root.compact
            animate: root.animate && !root.settingsOpen
            idleDrift: root.idleDrift
        }

        SettingsPage {
            id: page
            Layout.fillWidth: true
            visible: root.settingsOpen
            settings: root.settings
            history: root.history
            installer: root.installer
        }
    }
}
