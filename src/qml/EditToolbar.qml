// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import org.deepin.dtk 1.0 as DTK
import org.deepin.image.viewer 1.0 as IV

DTK.Control {
    id: editToolbar

    property string currentTool: "pen"
    property string blurMode: "gaussian"
    property string textMode: "plain"
    property bool canRedo: false
    property bool canUndo: false
    readonly property color themeTextColor: Qt.rgba(0, 0, 0, 0.7)

    signal closeRequested
    signal redoRequested
    signal rotateRequested
    signal saveRequested
    signal toolSelected(string tool)
    signal undoRequested

    function selectTool(tool) {
        currentTool = tool;
        toolSelected(tool);
    }

    Accessible.name: qsTr("Edit toolbar")
    Accessible.role: Accessible.ToolBar
    implicitHeight: 56
    implicitWidth: 514
    padding: 0

    background: Rectangle {
        border.color: Qt.rgba(0, 0, 0, 0.04)
        border.width: 1
        color: Qt.rgba(0.969, 0.969, 0.969, 0.92)
        radius: 18
    }

    component ToolbarButton: DTK.ToolButton {
        id: toolbarButton

        required property string iconPath
        property bool selected: false

        Accessible.name: DTK.ToolTip.text
        Accessible.role: Accessible.Button
        display: AbstractButton.IconOnly
        height: 36
        icon.color: selected ? "white" : editToolbar.themeTextColor
        icon.height: 20
        icon.name: iconPath
        icon.width: 20
        padding: 8
        width: 36

        background: Rectangle {
            color: toolbarButton.selected ? toolbarButton.palette.highlight
                  : toolbarButton.down ? Qt.rgba(editToolbar.themeTextColor.r,
                                                 editToolbar.themeTextColor.g,
                                                 editToolbar.themeTextColor.b, 0.18)
                  : toolbarButton.hovered ? Qt.rgba(editToolbar.themeTextColor.r,
                                                    editToolbar.themeTextColor.g,
                                                    editToolbar.themeTextColor.b, 0.1)
                  : "transparent"
            radius: 8
        }

        DTK.ToolTip.delay: 500
        DTK.ToolTip.timeout: 5000
        DTK.ToolTip.visible: hovered
    }

    component ToolSelectButton: ToolbarButton {
        required property string tool

        selected: editToolbar.currentTool === tool
        onClicked: editToolbar.selectTool(tool)
    }

    component Separator: Item {
        height: 36
        width: 13

        Rectangle {
            anchors.centerIn: parent
            color: Qt.rgba(editToolbar.themeTextColor.r,
                           editToolbar.themeTextColor.g,
                           editToolbar.themeTextColor.b, 0.12)
            height: 32
            width: 1
        }
    }

    contentItem: Row {
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
        spacing: 0

        ToolSelectButton { tool: "pen"; iconPath: "edit_pen"; DTK.ToolTip.text: qsTr("Pen") + " (P)" }
        ToolSelectButton { tool: "line"; iconPath: "edit_line"; DTK.ToolTip.text: qsTr("Line") + " (L)" }
        ToolSelectButton { tool: "arrow"; iconPath: "edit_arrow"; DTK.ToolTip.text: qsTr("Arrow") + " (A)" }
        ToolSelectButton { tool: "rect"; iconPath: "edit_rect"; DTK.ToolTip.text: qsTr("Rect") + " (R)" }
        ToolSelectButton { tool: "ellipse"; iconPath: "edit_ellipse"; DTK.ToolTip.text: qsTr("Ellipse") + " (C)" }
        ToolSelectButton { tool: "text"; iconPath: editToolbar.textMode === "number" ? "edit_number" : "edit_text"; DTK.ToolTip.text: qsTr("Text") + " (T)" }
        ToolSelectButton {
            tool: "blur"
            iconPath: editToolbar.blurMode === "mosaic" ? "edit_mosaic"
                    : editToolbar.blurMode === "graffiti" ? "edit_graffiti"
                    : "edit_blur"
            DTK.ToolTip.text: qsTr("Effects") + " (B)"
        }

        Separator { }
        ToolbarButton { iconPath: "edit_rotate"; DTK.ToolTip.text: qsTr("Rotate"); onClicked: editToolbar.rotateRequested() }
        ToolSelectButton { tool: "crop"; iconPath: "edit_crop"; DTK.ToolTip.text: qsTr("Crop") + " (X)" }
        ToolbarButton { enabled: editToolbar.canUndo; iconPath: "edit_undo"; DTK.ToolTip.text: qsTr("Undo") + " (Ctrl+Z)"; onClicked: editToolbar.undoRequested() }
        ToolbarButton { enabled: editToolbar.canRedo; iconPath: "edit_redo"; DTK.ToolTip.text: qsTr("Redo") + " (Ctrl+Y)"; onClicked: editToolbar.redoRequested() }

        Separator { }
        ToolbarButton { iconPath: "edit_save"; DTK.ToolTip.text: qsTr("Save Copy"); onClicked: editToolbar.saveRequested() }
        ToolbarButton { iconPath: "edit_close"; DTK.ToolTip.text: qsTr("Close Edit"); onClicked: editToolbar.closeRequested() }
    }

    Shortcut { enabled: IV.GStatus.editMode; sequence: "P"; onActivated: selectTool("pen") }
    Shortcut { enabled: IV.GStatus.editMode; sequence: "L"; onActivated: selectTool("line") }
    Shortcut { enabled: IV.GStatus.editMode; sequence: "A"; onActivated: selectTool("arrow") }
    Shortcut { enabled: IV.GStatus.editMode; sequence: "R"; onActivated: selectTool("rect") }
    Shortcut { enabled: IV.GStatus.editMode; sequence: "C"; onActivated: selectTool("ellipse") }
    Shortcut { enabled: IV.GStatus.editMode; sequence: "T"; onActivated: selectTool("text") }
    Shortcut {
        enabled: IV.GStatus.editMode && editToolbar.currentTool === "text"
        sequence: "N"
        onActivated: {
            editToolbar.textMode = "number";
            editToolbar.selectTool("text");
        }
    }
    Shortcut { enabled: IV.GStatus.editMode; sequence: "B"; onActivated: selectTool("blur") }
    Shortcut { enabled: IV.GStatus.editMode; sequence: "X"; onActivated: selectTool("crop") }
    Shortcut { enabled: IV.GStatus.editMode && editToolbar.canUndo; sequences: [StandardKey.Undo]; onActivated: editToolbar.undoRequested() }
    Shortcut { enabled: IV.GStatus.editMode && editToolbar.canRedo; sequences: [StandardKey.Redo]; onActivated: editToolbar.redoRequested() }
}
