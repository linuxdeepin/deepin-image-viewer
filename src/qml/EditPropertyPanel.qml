// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import org.deepin.dtk 1.0 as DTK
import org.deepin.image.viewer 1.0 as IV

DTK.Control {
    id: propertyPanel

    // uos-design: allow-literal-color Annotation colors are image content values, not UI theme colors.
    property string currentTool: ""
    property string blurMode: "gaussian"
    property color currentColor: "#f82a2a"
    property int effectStrength: 15
    readonly property var effectSteps: blurMode === "gaussian" ? [5, 15, 30] : [8, 16, 32]
    property string textMode: "plain"
    property int thickness: 5
    readonly property int effectEndpointIconSize: 20
    readonly property color themeTextColor: palette.windowText

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
    implicitWidth: isEffectTool ? 381 : isTextTool ? 255 : 339
    padding: 0

    // uos-design: allow-manual-blur-overlay See EditToolbar.qml. Keep both edit
    // surfaces on the same live, theme-aware translucent treatment.
    background: Item {
        DTK.BoxShadow {
            anchors.fill: panelBackground
            cornerRadius: panelBackground.radius
            hollow: true
            shadowBlur: 20
            shadowColor: Qt.rgba(propertyPanel.palette.shadow.r,
                                 propertyPanel.palette.shadow.g,
                                 propertyPanel.palette.shadow.b, 0.24)
            shadowOffsetY: 6
        }

        Rectangle {
            id: panelBackground

            anchors.fill: parent
            border.color: Qt.rgba(propertyPanel.themeTextColor.r,
                                  propertyPanel.themeTextColor.g,
                                  propertyPanel.themeTextColor.b, 0.12)
            border.width: 1
            color: Qt.rgba(propertyPanel.palette.window.r,
                           propertyPanel.palette.window.g,
                           propertyPanel.palette.window.b, 0.72)
            radius: 18
        }
    }

    component ModeButton: DTK.ToolButton {
        id: modeButton

        required property string iconPath
        property bool selected: false

        Accessible.name: DTK.ToolTip.text
        Accessible.role: Accessible.Button
        checkable: true
        display: AbstractButton.IconOnly
        checked: selected
        height: 36
        icon.height: 20
        icon.name: iconPath
        icon.width: 20
        padding: 8
        width: 36

        DTK.ToolTip.delay: 500
        DTK.ToolTip.timeout: 5000
        DTK.ToolTip.visible: hovered
    }

    component ThicknessDot: Item {
        height: 36
        width: 36

        Accessible.name: qsTr("Thickness")
        Accessible.role: Accessible.Indicator

        Rectangle {
            anchors.centerIn: parent
            color: propertyPanel.themeTextColor
            height: 2 + (propertyPanel.thickness - 1) * 12 / 9
            radius: height / 2
            width: height
        }
    }

    component EffectIcon: DTK.ToolButton {
        required property int iconSize
        required property string iconName

        display: AbstractButton.IconOnly
        height: 36
        icon.height: iconSize
        icon.name: iconName
        icon.width: iconSize
        padding: 0
        width: 36
        Accessible.name: "EditPropertyPanel_ToolButton"
    }

    component SliderTrack: Rectangle {
        required property var slider

        color: Qt.rgba(propertyPanel.themeTextColor.r,
                       propertyPanel.themeTextColor.g,
                       propertyPanel.themeTextColor.b, 0.2)
        height: 4
        radius: 2
        width: slider.availableWidth
        x: slider.leftPadding
        y: slider.topPadding + slider.availableHeight / 2 - height / 2

        Rectangle {
            color: propertyPanel.palette.highlight
            height: parent.height
            radius: parent.radius
            width: parent.slider.visualPosition * parent.width
        }
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

        Accessible.name: qsTr("Color")
        Accessible.role: Accessible.Grouping

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
                    readonly property bool selected: propertyPanel.currentColor.toString().toLowerCase() === modelData

                    border.color: modelData === "#ffffff" ? Qt.rgba(0, 0, 0, 0.18) : "transparent"
                    border.width: 1
                    color: modelData
                    height: 14
                    radius: 7
                    width: 14

                    Rectangle {
                        anchors.centerIn: parent
                        border.color: propertyPanel.palette.highlight
                        border.width: 2
                        color: "transparent"
                        height: 18
                        opacity: swatch.selected ? 1 : swatchHover.hovered ? 0.55 : 0
                        radius: 9
                        width: 18

                        Behavior on opacity {
                            NumberAnimation { duration: IV.GStatus.animationDefaultDuration }
                        }
                    }

                    HoverHandler { id: swatchHover }

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
            EffectIcon {
                iconName: propertyPanel.blurMode === "gaussian" ? "edit_blur_weak_endpoint"
                        : propertyPanel.blurMode === "mosaic" ? "edit_mosaic"
                        : "edit_graffiti"
                iconSize: propertyPanel.effectEndpointIconSize / 2
            }
            DTK.Slider {
                id: effectSlider

                from: 0
                height: 36
                stepSize: 1
                to: 2
                value: Math.max(0, propertyPanel.effectSteps.indexOf(propertyPanel.effectStrength))
                width: 120
                background: SliderTrack { slider: effectSlider }
                Binding { target: effectSlider.handle; property: "width"; value: 16 }
                Binding { target: effectSlider.handle; property: "height"; value: 16 }
                Binding { target: effectSlider.handle; property: "x"; value: effectSlider.leftPadding + effectSlider.visualPosition * (effectSlider.availableWidth - effectSlider.handle.width) }
                Binding { target: effectSlider.handle; property: "y"; value: effectSlider.topPadding + effectSlider.availableHeight / 2 - effectSlider.handle.height / 2 }
                onMoved: {
                    propertyPanel.effectStrength = propertyPanel.effectSteps[Math.round(value)];
                    propertyPanel.effectStrengthSelected(propertyPanel.effectStrength);
                }
            }
            EffectIcon {
                iconName: propertyPanel.blurMode === "mosaic" ? "edit_mosaic"
                        : propertyPanel.blurMode === "graffiti" ? "edit_graffiti"
                        : "edit_blur"
                iconSize: propertyPanel.effectEndpointIconSize
            }
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
                to: 10
                value: propertyPanel.thickness
                width: 120
                background: SliderTrack { slider: strokeSlider }
                Binding { target: strokeSlider.handle; property: "width"; value: 16 }
                Binding { target: strokeSlider.handle; property: "height"; value: 16 }
                Binding { target: strokeSlider.handle; property: "x"; value: strokeSlider.leftPadding + strokeSlider.visualPosition * (strokeSlider.availableWidth - strokeSlider.handle.width) }
                Binding { target: strokeSlider.handle; property: "y"; value: strokeSlider.topPadding + strokeSlider.availableHeight / 2 - strokeSlider.handle.height / 2 }
                onMoved: {
                    propertyPanel.thickness = Math.round(value);
                    propertyPanel.thicknessSelected(propertyPanel.thickness);
                }
            }
            Separator { }
            ColorPalette { }
        }
    }
}
