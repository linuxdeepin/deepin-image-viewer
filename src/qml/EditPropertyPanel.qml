// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import org.deepin.dtk 1.0 as DTK

DTK.Control {
    id: propertyPanel

    property string currentTool: "pen"
    property string blurMode: "gaussian"
    property color currentColor: "#f82a2a"
    property int effectStrength: 15
    property string textMode: "plain"
    property int thickness: 2
    readonly property color themeTextColor: Qt.rgba(0, 0, 0, 0.7)

    signal blurModeSelected(string mode)
    signal colorSelected(color value)
    signal effectStrengthSelected(int value)
    signal textModeSelected(string mode)
    signal thicknessSelected(int value)

    readonly property bool isTextTool: currentTool === "text"
    readonly property bool isEffectTool: currentTool === "blur"

    onBlurModeChanged: {
        effectStrength = blurMode === "gaussian" ? 15 : 16;
        effectStrengthSelected(effectStrength);
    }

    Accessible.name: qsTr("Tool properties")
    Accessible.role: Accessible.Pane
    implicitHeight: 56
    implicitWidth: isEffectTool ? 367 : isTextTool ? 265 : 333
    padding: 0

    background: Rectangle {
        border.color: Qt.rgba(0, 0, 0, 0.04)
        border.width: 1
        color: Qt.rgba(0.969, 0.969, 0.969, 0.92)
        radius: 18
    }

    component ModeButton: DTK.ToolButton {
        id: modeButton

        required property string iconPath
        property bool selected: false

        display: AbstractButton.IconOnly
        height: 36
        icon.color: selected ? "white" : propertyPanel.themeTextColor
        icon.height: 20
        icon.name: iconPath
        icon.width: 20
        padding: 8
        width: 36

        background: Rectangle {
            color: modeButton.selected ? modeButton.palette.highlight
                  : modeButton.down ? Qt.rgba(propertyPanel.themeTextColor.r,
                                              propertyPanel.themeTextColor.g,
                                              propertyPanel.themeTextColor.b, 0.18)
                  : modeButton.hovered ? Qt.rgba(propertyPanel.themeTextColor.r,
                                                 propertyPanel.themeTextColor.g,
                                                 propertyPanel.themeTextColor.b, 0.1)
                  : "transparent"
            radius: 8
        }
    }

    component ThicknessDot: Item {
        height: 36
        width: 36

        Rectangle {
            anchors.centerIn: parent
            color: propertyPanel.currentColor
            height: 2 + (propertyPanel.thickness - 1) * 12 / 49
            radius: height / 2
            width: height
        }
    }

    component EffectIcon: DTK.ToolButton {
        required property int iconSize

        display: AbstractButton.IconOnly
        height: 36
        icon.color: propertyPanel.themeTextColor
        icon.height: iconSize
        icon.name: propertyPanel.blurMode === "mosaic" ? "edit_mosaic"
                  : propertyPanel.blurMode === "graffiti" ? "edit_graffiti"
                  : "edit_blur"
        icon.width: iconSize
        padding: 0
        width: iconSize
    }

    component Separator: Item {
        height: 36
        width: 13

        Rectangle {
            anchors.centerIn: parent
            color: Qt.rgba(propertyPanel.themeTextColor.r,
                           propertyPanel.themeTextColor.g,
                           propertyPanel.themeTextColor.b, 0.12)
            height: 32
            width: 1
        }
    }

    component ColorPalette: Item {
        height: 36
        width: 136

        Grid {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            columns: 6
            columnSpacing: 6
            rowSpacing: 6

            Repeater {
                model: ["#000000", "#7d7d7d", "#ffffff", "#f82a2a", "#ff8100", "#fff100",
                        "#99e338", "#006d06", "#00b7c3", "#0089f7", "#7600ac", "#003781"]

                delegate: Rectangle {
                    id: swatch
                    required property string modelData

                    border.color: modelData === "#ffffff" ? Qt.rgba(0, 0, 0, 0.18) : "transparent"
                    border.width: 1
                    color: modelData
                    height: 14
                    radius: 7
                    width: 14

                    Rectangle {
                        anchors.centerIn: parent
                        border.color: "white"
                        border.width: 1
                        color: "transparent"
                        height: 18
                        radius: 9
                        visible: propertyPanel.currentColor.toString().toLowerCase() === swatch.modelData
                        width: 18
                    }

                    TapHandler {
                        onTapped: {
                            propertyPanel.currentColor = swatch.modelData;
                            propertyPanel.colorSelected(swatch.modelData);
                        }
                    }
                }
            }
        }
    }

    contentItem: Item {
        Row {
            id: textControls
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.top: parent.top
            anchors.topMargin: 10
            spacing: 8
            visible: propertyPanel.isTextTool

            ModeButton {
                iconPath: "edit_text"
                selected: propertyPanel.textMode === "plain"
                DTK.ToolTip.text: qsTr("Plain Text")
                onClicked: {
                    propertyPanel.textMode = "plain";
                    propertyPanel.textModeSelected("plain");
                }
            }
            ModeButton {
                iconPath: "edit_number"
                selected: propertyPanel.textMode === "number"
                DTK.ToolTip.text: qsTr("Numbered Step")
                onClicked: {
                    propertyPanel.textMode = "number";
                    propertyPanel.textModeSelected("number");
                }
            }
            Separator { }
            ColorPalette { }
        }

        Row {
            id: effectControls
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.top: parent.top
            anchors.topMargin: 10
            spacing: 8
            visible: propertyPanel.isEffectTool

            ModeButton { iconPath: "edit_blur"; selected: propertyPanel.blurMode === "gaussian"; DTK.ToolTip.text: qsTr("Gaussian Blur"); onClicked: { propertyPanel.blurMode = "gaussian"; propertyPanel.blurModeSelected("gaussian"); } }
            ModeButton { iconPath: "edit_mosaic"; selected: propertyPanel.blurMode === "mosaic"; DTK.ToolTip.text: qsTr("Mosaic"); onClicked: { propertyPanel.blurMode = "mosaic"; propertyPanel.blurModeSelected("mosaic"); } }
            ModeButton { iconPath: "edit_graffiti"; selected: propertyPanel.blurMode === "graffiti"; DTK.ToolTip.text: qsTr("Graffiti"); onClicked: { propertyPanel.blurMode = "graffiti"; propertyPanel.blurModeSelected("graffiti"); } }
            Separator { }
            EffectIcon { iconSize: 12 }
            DTK.Slider {
                id: effectSlider

                from: propertyPanel.blurMode === "gaussian" ? 5 : 8
                height: 36
                stepSize: 1
                to: propertyPanel.blurMode === "gaussian" ? 30 : 32
                value: propertyPanel.effectStrength
                width: 120
                background: Rectangle {
                    color: Qt.rgba(propertyPanel.themeTextColor.r,
                                   propertyPanel.themeTextColor.g,
                                   propertyPanel.themeTextColor.b, 0.2)
                    height: 4
                    radius: 2
                    width: effectSlider.availableWidth
                    x: effectSlider.leftPadding
                    y: effectSlider.topPadding + effectSlider.availableHeight / 2 - height / 2

                    Rectangle {
                        color: propertyPanel.palette.highlight
                        height: parent.height
                        radius: parent.radius
                        width: effectSlider.visualPosition * parent.width
                    }
                }
                handle: Rectangle {
                    border.color: Qt.rgba(0, 0, 0, 0.08)
                    border.width: 1
                    color: propertyPanel.palette.highlight
                    height: 16
                    radius: 8
                    width: 16
                    x: effectSlider.leftPadding + effectSlider.visualPosition * (effectSlider.availableWidth - width)
                    y: effectSlider.topPadding + effectSlider.availableHeight / 2 - height / 2
                }
                onMoved: {
                    propertyPanel.effectStrength = Math.round(value);
                    propertyPanel.effectStrengthSelected(propertyPanel.effectStrength);
                }
            }
            EffectIcon { iconSize: 24 }
        }

        Row {
            id: strokeControls
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.top: parent.top
            anchors.topMargin: 10
            spacing: 8
            visible: !propertyPanel.isTextTool && !propertyPanel.isEffectTool

            ThicknessDot { }
            DTK.Slider {
                id: strokeSlider

                from: 1
                height: 36
                stepSize: 1
                to: 50
                value: propertyPanel.thickness
                width: 120
                background: Rectangle {
                    color: Qt.rgba(propertyPanel.themeTextColor.r,
                                   propertyPanel.themeTextColor.g,
                                   propertyPanel.themeTextColor.b, 0.2)
                    height: 4
                    radius: 2
                    width: strokeSlider.availableWidth
                    x: strokeSlider.leftPadding
                    y: strokeSlider.topPadding + strokeSlider.availableHeight / 2 - height / 2

                    Rectangle {
                        color: propertyPanel.palette.highlight
                        height: parent.height
                        radius: parent.radius
                        width: strokeSlider.visualPosition * parent.width
                    }
                }
                handle: Rectangle {
                    border.color: Qt.rgba(0, 0, 0, 0.08)
                    border.width: 1
                    color: propertyPanel.palette.highlight
                    height: 16
                    radius: 8
                    width: 16
                    x: strokeSlider.leftPadding + strokeSlider.visualPosition * (strokeSlider.availableWidth - width)
                    y: strokeSlider.topPadding + strokeSlider.availableHeight / 2 - height / 2
                }
                onMoved: {
                    propertyPanel.thickness = Math.round(value);
                    propertyPanel.thicknessSelected(propertyPanel.thickness);
                }
            }
            Separator { }
            ColorPalette { }
        }
    }

    ColorDialog {
        id: colorDialog

        selectedColor: propertyPanel.currentColor
        title: qsTr("Color")

        onAccepted: {
            propertyPanel.currentColor = selectedColor;
            propertyPanel.colorSelected(selectedColor);
        }
    }
}
