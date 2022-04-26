/*
 * Copyright (C) 2022 UnionTech Technology Co., Ltd.
 *
 * Author:     yeshanshan <yeshanshan@uniontech.com>
 *
 * Maintainer: yeshanshan <yeshanshan@uniontech.com>
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

import QtQuick 2.0
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0

Control {
    id: control
    property string title
    property string description
    property int corners: RoundRectangle.NoneCorner
    property string iconName
    signal clicked()
    property Component action: ActionButton {
        visible: control.iconName
        Layout.alignment: Qt.AlignRight
        icon {
            width: 14
            height: 14
            name: control.iconName
        }

        onClicked: {
            if(showPicLabel.visible){
                showPicLabel.visible=false;
            }else{
                if(!fileControl.isShowToolTip(imageViewer.source,nameedit.text)&& nameedit.text.length>0){
                    var name = nameedit.text
                    //bool返回值判断是否成功
                    if(fileControl.slotFileReName(name,imageViewer.source)){
                        imageViewer.sourcePaths=fileControl.renameOne(imageViewer.sourcePaths,imageViewer.source,fileControl.getNamePath(imageViewer.source,name))
                        imageViewer.source=fileControl.getNamePath(imageViewer.source,name)
                    }
                }
                showPicLabel.visible=true;
            }


        }
    }
    padding: 5
    contentItem: ColumnLayout {
        Label {
            visible: control.title
            text: control.title
            font: DTK.fontManager.t10
        }
        RowLayout {
            LineEdit {
                id: nameedit
                visible:!showPicLabel.visible
                text: fileControl.slotGetFileName(imageViewer.source)
                anchors.fill: nameeditrect
                anchors.topMargin:5
                anchors.leftMargin:10
                font.pixelSize: 16
                focus: true
                selectByMouse: true
                alertText: qsTr("The file already exists, please use another name")
                showAlert: fileControl.isShowToolTip(imageViewer.source,nameedit.text) && nameedit.visible
                height:20
            }
            Label {
                id:showPicLabel
                visible: control.description
                Layout.fillWidth: true
                text: control.description
                font: DTK.fontManager.t8
                elide: Text.ElideMiddle
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true


                    AlertToolTip{
                        id:tip
                        parent: parent
                        visible:parent.focus
                        text: control.description
                    }

                    onMouseXChanged:{

                        if(tip.width<control.width+15){
                            tip.visible=false
                        }else{
                            tip.visible=true
                        }

                    }
                    onExited:{
                        tip.visible=false
                    }
                }
            }
            Loader {
                Layout.leftMargin: 5
                sourceComponent: control.action
            }
        }
    }

    background: RoundRectangle {
        implicitWidth: 66
        implicitHeight: 40
        color:  Qt.rgba(0, 0, 0, 0.05)
        radius: Style.control.radius
        corners: control.corners
    }
}
