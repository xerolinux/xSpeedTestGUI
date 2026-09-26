import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.xspeedtest

Window {
    id: win

    readonly property real cornerRadius: Math.max(panel.style.radius, 10) + 4

    width: panel.implicitWidth
    height: panel.implicitHeight
    visible: true
    title: "xSpeedTest"
    color: "transparent"
    flags: Qt.Window | Qt.FramelessWindowHint

    WindowBlur {
        window: win
        radius: win.cornerRadius
    }

    SpeedSettings {
        id: settings
        profile: "app"
    }

    HistoryModel {
        id: history
    }

    SpeedTestRunner {
        id: runner
        history: history
        backend: settings.backend
    }

    PlasmoidInstaller {
        id: installer
    }

    Component.onCompleted: {
        if (settings.autoStart)
            runner.start();
    }

    onActiveChanged: if (active) installer.refresh()

    Shortcut {
        sequence: "Escape"
        onActivated: Qt.quit()
    }

    Rectangle {
        anchors.fill: parent
        radius: win.cornerRadius
        color: Qt.alpha(Kirigami.Theme.backgroundColor, settings.glassOpacity)
        border.color: Qt.alpha(Kirigami.Theme.textColor, 0.15)

        DragHandler {
            target: null
            onActiveChanged: if (active) win.startSystemMove()
        }

        SpeedPanel {
            id: panel
            anchors.fill: parent
            padding: Kirigami.Units.gridUnit * 1.5
            controlsInset: closeButton.width
            runner: runner
            settings: settings
            history: history
            installer: installer
            settingsTitle: qsTr("App settings")
        }

        QQC2.ToolButton {
            id: closeButton
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: Kirigami.Units.smallSpacing
            icon.name: "window-close-symbolic"
            onClicked: Qt.quit()
        }
    }
}
