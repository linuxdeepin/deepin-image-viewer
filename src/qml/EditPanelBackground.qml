// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Effects
import Qt5Compat.GraphicalEffects
import org.deepin.dtk 1.0 as DTK

Item {
    id: root

    required property Item blurSource
    required property Item target
    property color borderColor
    property color shadowColor
    property color tintColor
    property real radius: 18
    readonly property int blurPadding: 32
    readonly property point sourceOrigin: {
        // Keep this binding sensitive to geometry used internally by mapToItem().
        target.x;
        target.y;
        target.width;
        target.height;
        blurSource.x;
        blurSource.y;
        return target.mapToItem(blurSource, -blurPadding, -blurPadding);
    }

    DTK.BoxShadow {
        anchors.fill: root
        cornerRadius: root.radius
        hollow: true
        shadowBlur: 20
        shadowColor: root.shadowColor
        shadowOffsetY: 6
    }

    Item {
        id: panelSurface

        anchors.fill: parent
        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: Rectangle {
                height: panelSurface.height
                radius: root.radius
                width: panelSurface.width
            }
        }

        ShaderEffectSource {
            id: capturedImage

            height: root.height + root.blurPadding * 2
            hideSource: false
            live: true
            recursive: false
            smooth: true
            sourceItem: root.blurSource
            sourceRect: Qt.rect(root.sourceOrigin.x, root.sourceOrigin.y, width, height)
            visible: false
            width: root.width + root.blurPadding * 2
            x: -root.blurPadding
            y: -root.blurPadding
        }

        MultiEffect {
            anchors.fill: capturedImage
            autoPaddingEnabled: false
            blur: 0.6
            blurEnabled: true
            blurMax: root.blurPadding
            source: capturedImage
        }

        Rectangle {
            anchors.fill: parent
            color: root.tintColor
        }
    }

    Rectangle {
        anchors.fill: parent
        border.color: root.borderColor
        border.width: 1
        color: "transparent"
        radius: root.radius
    }
}
