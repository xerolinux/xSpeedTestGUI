import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

ColumnLayout {
    id: root

    required property var view

    readonly property real gu: Kirigami.Units.gridUnit

    implicitWidth: gu * 14
    spacing: gu * 0.7

    QQC2.Label {
        Layout.fillWidth: true
        Layout.preferredWidth: 1
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
        text: root.view.serverText
        color: root.view.errorState ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.disabledTextColor
    }

    QQC2.Label {
        Layout.alignment: Qt.AlignHCenter
        text: root.view.phaseText.toUpperCase()
        color: Kirigami.Theme.disabledTextColor
        font.letterSpacing: 1
        font.weight: Font.DemiBold
        font.pixelSize: Kirigami.Theme.smallFont.pixelSize
    }

    StyleCanvas {
        Layout.fillWidth: true
        Layout.preferredHeight: root.view.u * 130
        runner: root.view.runner
        style: root.view.style
        animate: root.view.animate
        idleDrift: root.view.idleDrift
    }

    Readout {
        Layout.alignment: Qt.AlignHCenter
        view: root.view
        text: root.view.readout
        unit: root.view.unit
        pixelSize: root.gu * 2.1
    }

    StatChips {
        Layout.fillWidth: true
        view: root.view
    }

    ActionButton {
        view: root.view
    }
}
