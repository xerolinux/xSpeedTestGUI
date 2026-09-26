import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.xspeedtest

Window {
    id: win

    readonly property var args: Qt.application.arguments
    readonly property string styleId: args[args.length - 3]
    readonly property string mode: args[args.length - 2]
    readonly property string outDir: args[args.length - 1]
    readonly property int frameMs: 100
    readonly property int frameCount: mode === "settings" ? 1 : 78

    property int taken: 0

    width: stage.width
    height: stage.height
    visible: true
    color: "#12141c"

    SpeedSettings {
        id: settings
        path: outDir + "/settings.conf"
        Component.onCompleted: style = win.styleId
    }

    HistoryModel {
        id: history
        path: outDir + "/history.json"
    }

    SpeedTestRunner {
        id: runner
        history: win.mode === "settings" ? history : null
    }

    Item {
        id: stage

        width: panel.implicitWidth + 40
        height: panel.implicitHeight + 40

        Rectangle {
            anchors.fill: parent
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0; color: "#1b1f3a" }
                GradientStop { position: 0.55; color: "#3a2a5c" }
                GradientStop { position: 1; color: "#12303f" }
            }
        }

        Rectangle {
            anchors.centerIn: parent
            width: panel.implicitWidth
            height: panel.implicitHeight
            radius: Math.max(panel.style.radius, 10) + 4
            color: Qt.alpha(Kirigami.Theme.backgroundColor, settings.glassOpacity + 0.2)
            border.color: Qt.alpha(Kirigami.Theme.textColor, 0.15)

            SpeedPanel {
                id: panel
                anchors.fill: parent
                padding: Kirigami.Units.gridUnit * 1.5
                controlsInset: win.mode === "compact" ? 0 : closeButton.width
                compact: win.mode === "compact"
                settingsOpen: win.mode === "settings"
                runner: runner
                settings: settings
                history: win.mode === "settings" ? history : null
                installer: win.mode === "settings" ? installerObject : null
                settingsTitle: qsTr("App settings")
            }

            QQC2.ToolButton {
                id: closeButton
                visible: win.mode !== "compact"
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: Kirigami.Units.smallSpacing
                icon.name: "window-close-symbolic"
            }
        }
    }

    PlasmoidInstaller {
        id: installerObject
    }

    Timer {
        interval: 300
        running: win.mode !== "settings"
        onTriggered: runner.start()
    }

    Timer {
        interval: win.frameMs
        running: true
        repeat: true
        onTriggered: {
            if (win.taken >= win.frameCount) {
                Qt.quit();
                return;
            }
            const index = win.taken++;
            stage.grabToImage(function (result) {
                result.saveToFile(win.outDir + "/f" + String(index).padStart(3, "0") + ".png");
            });
        }
    }
}
