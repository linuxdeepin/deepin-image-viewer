// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Window 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0 as D
import org.deepin.dtk 1.0
//DialogWindow {

//}

DialogWindow {

    width: 280
    property int leftX: 20
    property int topY: 70
    x: root.x+root.width - width - leftX
    y: root.y + topY
    minimumWidth: 280
    maximumWidth: 280
    minimumHeight: contentHeight4.height+60
    maximumHeight: contentHeight4.height+60

    visible: false

    property var filePath
    property string fileName: fileControl.slotGetFileNameSuffix(filePath)

    header: DialogTitleBar {
        enableInWindowBlendBlur: true
        content: Loader {
            sourceComponent: Label {
                anchors.centerIn: parent
                textFormat: Text.PlainText
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font: DTK.fontManager.t8
                text: title
                elide: Text.ElideMiddle
            }
            property string title: fileName
        }
    }

    ColumnLayout {
        id: contentHeight4
        width: 260
        anchors {
            horizontalCenter: parent.horizontalCenter
            margins: 10
        }

        PropertyItem {
            title: qsTr("Basic info")
            ColumnLayout {
                spacing: 1
                PropertyActionItemDelegate {
                    id: fileNameProp
                    Layout.fillWidth: true
                    title: qsTr("File name")
                    description: fileName
                    iconName: "action_edit"
                    onClicked: {

                    }
                    corners: RoundRectangle.TopCorner
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 1
                    PropertyItemDelegate {
                        title: qsTr("Size")
                        description: fileControl.slotGetInfo("FileSize",filePath)
                        corners: RoundRectangle.BottomLeftCorner
                    }
                    PropertyItemDelegate {
                        title: qsTr("Dimensions")
                        description: fileControl.slotGetInfo("Dimension",filePath)
                        Layout.fillWidth: true
                    }
                    PropertyItemDelegate {
                        title: qsTr("Type")
                        description: fileControl.slotFileSuffix(filePath,false)
                        corners: RoundRectangle.BottomRightCorner
                    }
                }
            }
            ColumnLayout {
                spacing: 1
                PropertyActionItemDelegate {
                    Layout.fillWidth: true
                    title: qsTr("Date captured")
                    description: fileControl.slotGetInfo("DateTimeOriginal",filePath)
                    corners: RoundRectangle.TopCorner
                }

                PropertyActionItemDelegate {
                    Layout.fillWidth: true
                    title: qsTr("Date modified")
                    description: fileControl.slotGetInfo("DateTimeDigitized",filePath)
                    corners: RoundRectangle.BottomCorner
                }
            }
        }
        PropertyItem {
            title: qsTr("Details")
            ColumnLayout {
                spacing: 1
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 1
                    PropertyItemDelegate {
                        contrlImplicitWidth:66
                        title: qsTr("Aperture")
                        description: fileControl.slotGetInfo("ApertureValue",filePath)
                        corners: RoundRectangle.TopLeftCorner
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth:106
                        title: qsTr("Exposure program")
                        description: fileControl.slotGetInfo("ExposureProgram",filePath)
                        Layout.fillWidth: true
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth:86
                        title: qsTr("Focal length")
                        description: fileControl.slotGetInfo("FocalLength",filePath)
                        corners: RoundRectangle.TopRightCorner
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 1
                    PropertyItemDelegate {
                        contrlImplicitWidth: 66
                        title: qsTr("ISO")
                        description: fileControl.slotGetInfo("ISOSpeedRatings",filePath)

                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth: 106
                        title: qsTr("Exposure mode")
                        description: fileControl.slotGetInfo("ExposureMode",filePath)
                        Layout.fillWidth: true
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth: 86
                        title: qsTr("Exposure time")
                        description: fileControl.slotGetInfo("ExposureTime",filePath)
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 1
                    PropertyItemDelegate {
                        contrlImplicitWidth:66
                        title: qsTr("Flash")
                        description: fileControl.slotGetInfo("Flash",filePath)                
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth:106
                        title: qsTr("Flash compensation")
                        description: fileControl.slotGetInfo("FlashExposureComp",filePath)
                        Layout.fillWidth: true
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth:86
                        title: qsTr("Max aperture")
                        description: fileControl.slotGetInfo("MaxApertureValue",filePath)
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 1
                    PropertyItemDelegate {
                        contrlImplicitWidth:66
                        title: qsTr("Colorspace")
                        description: fileControl.slotGetInfo("ColorSpace",filePath)
                        corners: RoundRectangle.BottomLeftCorner
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth:106
                        title: qsTr("Metering mode")
                        description: fileControl.slotGetInfo("MeteringMode",filePath)
                        Layout.fillWidth: true
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth:86
                        title: qsTr("White balance")
                        description: fileControl.slotGetInfo("WhiteBalance",filePath)
                        corners: RoundRectangle.BottomRightCorner
                    }
                }
            }

            PropertyItemDelegate {
                contrlImplicitWidth:240
                title: qsTr("Device model")
                description: fileControl.slotGetInfo("Model",filePath)
                corners: RoundRectangle.AllCorner
            }
            PropertyItemDelegate {
                contrlImplicitWidth:240
                title: qsTr("Lens model")
                description: fileControl.slotGetInfo("LensType",filePath)
                corners: RoundRectangle.AllCorner
            }

            // 默认隐藏"详细"菜单，再初始布局完成后隐藏
            Component.onCompleted: {
                showProperty = false
            }
        }
    }

    onVisibleChanged: {
        setX(root.x + root.width / 2 - width / 2)
        setY(root.y + root.height / 2 - height / 2)
    }

    // 窗口关闭时复位组件状态
    onClosing: {
        fileNameProp.reset();
    }
    // 图片变更时复位组件状态(切换时关闭重命名框)
    onFileNameChanged: {
        fileNameProp.reset();
    }
}
