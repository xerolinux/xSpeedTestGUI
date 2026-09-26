import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item {
    id: root

    required property SpeedSettings settings
    required property HistoryModel history
    property PlasmoidInstaller installer: null

    property bool justInstalled: false

    property string pendingStyle: settings.style
    property int pendingUnit: settings.speedUnit
    property string pendingBackend: settings.backend
    property bool pendingAutoStart: settings.autoStart
    property bool pendingShowHistory: settings.showHistory
    property bool pendingAnimate: settings.animate
    property real pendingOpacity: settings.glassOpacity
    property int pendingBlur: blur.strength

    readonly property bool dirty: pendingStyle !== settings.style
        || pendingUnit !== settings.speedUnit
        || pendingBackend !== settings.backend
        || pendingAutoStart !== settings.autoStart
        || pendingShowHistory !== settings.showHistory
        || pendingAnimate !== settings.animate
        || Math.abs(pendingOpacity - settings.glassOpacity) > 0.0001
        || pendingBlur !== blur.strength

    implicitWidth: Kirigami.Units.gridUnit * 27
    implicitHeight: layout.implicitHeight

    SystemBlur {
        id: blur
    }

    function apply() {
        settings.style = pendingStyle;
        settings.speedUnit = pendingUnit;
        settings.backend = pendingBackend;
        settings.autoStart = pendingAutoStart;
        settings.showHistory = pendingShowHistory;
        settings.animate = pendingAnimate;
        settings.glassOpacity = pendingOpacity;
        if (pendingBlur !== blur.strength)
            blur.apply(pendingBlur);
    }

    function installPlasmoid() {
        installer.install();
        justInstalled = installer.errorString.length === 0;
    }

    ColumnLayout {
        id: layout
        anchors.fill: parent
        spacing: Kirigami.Units.largeSpacing

        Kirigami.FormLayout {
            Layout.fillWidth: true

            QQC2.ComboBox {
                id: styleBox
                Kirigami.FormData.label: qsTr("Style")
                textRole: "name"
                valueRole: "id"
                model: Styles.list
                onActivated: root.pendingStyle = currentValue
                Binding on currentIndex {
                    value: (styleBox.count, styleBox.indexOfValue(root.pendingStyle))
                    restoreMode: Binding.RestoreNone
                }
            }

            QQC2.ComboBox {
                id: unit
                Kirigami.FormData.label: qsTr("Speed unit")
                textRole: "text"
                valueRole: "value"
                model: [
                    {text: qsTr("Megabits per second (Mbps)"), value: SpeedSettings.Mbps},
                    {text: qsTr("Kilobits per second (kbps)"), value: SpeedSettings.Kbps},
                    {text: qsTr("Megabytes per second (MBps)"), value: SpeedSettings.MBps},
                    {text: qsTr("Kilobytes per second (KBps)"), value: SpeedSettings.KBps}
                ]
                onActivated: root.pendingUnit = currentValue
                Binding on currentIndex {
                    value: (unit.count, unit.indexOfValue(root.pendingUnit))
                    restoreMode: Binding.RestoreNone
                }
            }

            QQC2.ComboBox {
                id: backend
                Kirigami.FormData.label: qsTr("Test server")
                textRole: "text"
                valueRole: "value"
                model: [
                    {text: qsTr("Automatic (closest and fastest)"), value: "auto"},
                    {text: qsTr("speedtest-cli servers only"), value: "speedtest"},
                    {text: qsTr("Cloudflare only"), value: "cloudflare"}
                ]
                onActivated: root.pendingBackend = currentValue
                Binding on currentIndex {
                    value: (backend.count, backend.indexOfValue(root.pendingBackend))
                    restoreMode: Binding.RestoreNone
                }
            }

            QQC2.Switch {
                Kirigami.FormData.label: qsTr("Test on launch")
                onToggled: root.pendingAutoStart = checked
                Binding on checked {
                    value: root.pendingAutoStart
                    restoreMode: Binding.RestoreNone
                }
            }

            QQC2.Switch {
                Kirigami.FormData.label: qsTr("Recent results")
                onToggled: root.pendingShowHistory = checked
                Binding on checked {
                    value: root.pendingShowHistory
                    restoreMode: Binding.RestoreNone
                }
            }

            QQC2.Switch {
                Kirigami.FormData.label: qsTr("Animation")
                onToggled: root.pendingAnimate = checked
                Binding on checked {
                    value: root.pendingAnimate
                    restoreMode: Binding.RestoreNone
                }
            }

            RowLayout {
                Kirigami.FormData.label: qsTr("Window opacity")

                QQC2.Slider {
                    from: 0.1
                    to: 1
                    stepSize: 0.01
                    onMoved: root.pendingOpacity = value
                    Binding on value {
                        value: root.pendingOpacity
                        restoreMode: Binding.RestoreNone
                    }
                }
                QQC2.Label {
                    Layout.minimumWidth: Kirigami.Units.gridUnit * 2.5
                    text: Math.round(root.pendingOpacity * 100) + "%"
                }
            }

            RowLayout {
                Kirigami.FormData.label: qsTr("Blur strength")
                enabled: blur.available

                QQC2.Slider {
                    from: 1
                    to: 15
                    stepSize: 1
                    onMoved: root.pendingBlur = Math.round(value)
                    Binding on value {
                        value: root.pendingBlur
                        restoreMode: Binding.RestoreNone
                    }
                }
                QQC2.Label {
                    Layout.minimumWidth: Kirigami.Units.gridUnit * 2.5
                    text: root.pendingBlur
                }
            }

            QQC2.Button {
                Kirigami.FormData.label: qsTr("Results")
                text: qsTr("Clear history")
                enabled: root.history.count > 0
                onClicked: root.history.clear()
            }

            RowLayout {
                visible: root.installer !== null
                Kirigami.FormData.label: qsTr("Plasma widget")

                QQC2.Label {
                    text: !root.installer ? "" : root.installer.outdated ? qsTr("Update available") : root.installer.userInstalled ? qsTr("Installed") : root.installer.installed ? qsTr("Installed by the system") : qsTr("Not installed")
                    color: root.installer && root.installer.outdated ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
                }
                QQC2.Button {
                    visible: root.installer && !root.installer.installed
                    text: qsTr("Install Plasmoid")
                    onClicked: root.installPlasmoid()
                }
                QQC2.Button {
                    visible: root.installer && root.installer.outdated
                    text: qsTr("Update Plasmoid")
                    onClicked: root.installer.update()
                }
            }
        }

        QQC2.Label {
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            visible: root.installer && root.installer.outdated
            wrapMode: Text.Wrap
            text: qsTr("Updating installs the new widget and restarts the Plasma shell. Your panels and desktop reload for a few seconds.")
        }

        QQC2.Label {
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            visible: blur.errorString.length > 0
            wrapMode: Text.Wrap
            color: Kirigami.Theme.negativeTextColor
            text: blur.errorString
        }

        QQC2.Label {
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            visible: root.installer && root.installer.errorString.length > 0
            wrapMode: Text.Wrap
            color: Kirigami.Theme.negativeTextColor
            text: root.installer ? root.installer.errorString : ""
        }

        QQC2.Label {
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            visible: root.justInstalled && root.installer && root.installer.userInstalled
            wrapMode: Text.Wrap
            text: qsTr("Plasmoid installed. Add xSpeedTest from Add Widgets on your desktop or panel.")
        }

        QQC2.Button {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: Kirigami.Units.gridUnit * 8
            highlighted: root.dirty
            enabled: root.dirty
            text: qsTr("Apply")
            onClicked: root.apply()
        }
    }
}
