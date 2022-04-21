/*
 * Copyright (C) 2021 ~ 2022 UnionTech Technology Co., Ltd.
 *
 * Author:     Chen Bin <chenbin@uniontech.com>
 *
 * Maintainer: Chen Bin <chenbin@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.11
import org.deepin.dtk.impl 1.0 as D
import org.deepin.dtk.style 1.0 as DS

ToolTip {
    id: control
    property Item target

    x: 0
    topPadding: DS.Style.alertToolTip.verticalPadding
    bottomPadding: DS.Style.alertToolTip.verticalPadding
    leftPadding: DS.Style.alertToolTip.horizontalPadding
    rightPadding: DS.Style.alertToolTip.horizontalPadding
    implicitWidth: DS.Style.control.implicitWidth(control)
    implicitHeight: DS.Style.control.implicitHeight(control)
    margins: 0
    closePolicy: Popup.NoAutoClose

    background: Item {
        BoxShadow {
            anchors.fill: _background
            shadowBlur: 20
            shadowOffsetY: 6
            shadowColor: Qt.rgba(0, 0, 0, 0.2)
            cornerRadius: _background.radius
        }

        Rectangle {
            property D.Palette backgroundColor: DS.Style.alertToolTip.background
            property D.Palette borderColor: DS.Style.control.border
            id: _background
            anchors.fill: parent
            color: D.ColorSelector.backgroundColor
            border.color: D.ColorSelector.borderColor
            radius: DS.Style.control.radius
        }
    }

    contentItem: Text {
        property D.Palette textColor: DS.Style.alertToolTip.text
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        text: control.text
        color: D.ColorSelector.textColor
    }

    enter: Transition {
        NumberAnimation { properties: "opacity"; from: 0.0; to: 1.0; duration: 200 }
        NumberAnimation { properties: "y"; from: control.target.height; to: control.target.height + DS.Style.control.spacing; duration: 200 }
    }

    exit: Transition {
        NumberAnimation { properties: "opacity"; from: 1.0; to: 0.0 }
        NumberAnimation { properties: "y"; from: control.target.height + DS.Style.control.spacing ; to: control.target.height }
    }

    BoxShadow {
        property D.Palette dropShadowColor: DS.Style.alertToolTip.dropShadow
        y: - height * (0.75) - control.topMargin - control.topPadding
        width: DS.Style.alertToolTip.connectorWidth
        height: DS.Style.alertToolTip.connectorHeight
        shadowBlur: 4
        shadowOffsetY: 2
        shadowColor: D.ColorSelector.dropShadowColor
        cornerRadius: _background.radius

        Rectangle {
            anchors.fill: parent
            color: _background.color
            border.color: _background.D.ColorSelector.borderColor
            border.width: 1
        }
    }
}
