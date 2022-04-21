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
    x:root.x+root.width-width-leftX
    y:root.y+topY
    minimumWidth: 280
    maximumWidth: 280
    minimumHeight: contentHeight4.height+60
    maximumHeight: contentHeight4.height+60

    visible: false

    property string fileName: fileControl.slotGetFileNameSuffix(source)

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
//        Image {
//            source: "qrc:/assets/popup/nointeractive.svg"
//        }
        PropertyItem {
            title: "基本信息"
            ColumnLayout {
                spacing: 1
                PropertyActionItemDelegate {
                    Layout.fillWidth: true
                    title: "文件名"
                    description: fileName
                    iconName: "action_edit"
                    onClicked: {

                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 1
                    PropertyItemDelegate {
                        title: "大小"
                        description: fileControl.slotGetInfo("FileSize",source)
                        corners: RoundRectangle.BottomLeftCorner
                    }
                    PropertyItemDelegate {
                        title: "分辨率"
                        description: fileControl.slotGetInfo("Dimension",source)
                        Layout.fillWidth: true
                    }
                    PropertyItemDelegate {
                        title: "格式"
                        description: fileControl.slotFileSuffix(source,false)
                        corners: RoundRectangle.BottomRightCorner
                    }
                }
            }
            ColumnLayout {
                spacing: 1
                PropertyActionItemDelegate {
                    Layout.fillWidth: true
                    title: "拍摄日期"
                    description: fileControl.slotGetInfo("DateTimeOriginal",source)
                    corners: RoundRectangle.TopCorner
                }

                PropertyActionItemDelegate {
                    Layout.fillWidth: true
                    title: "修改日期"
                    description: fileControl.slotGetInfo("DateTimeDigitized",source)
                    corners: RoundRectangle.BottomCorner
                }
            }
        }
        PropertyItem {
            title: "详细信息"
            ColumnLayout {
                spacing: 1
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 1
                    PropertyItemDelegate {
                        contrlImplicitWidth:66
                        title: "光圈"
                        description: fileControl.slotGetInfo("ApertureValue",source)
                        corners: RoundRectangle.TopLeftCorner
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth:106
                        title: "曝光程序"
                        description: fileControl.slotGetInfo("ExposureProgram",source)
                        Layout.fillWidth: true
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth:86
                        title: "焦距"
                        description: fileControl.slotGetInfo("FocalLength",source)
                        corners: RoundRectangle.TopRightCorner
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 1
                    PropertyItemDelegate {
                        contrlImplicitWidth:66
                        title: "ISO感光度"
                        description: fileControl.slotGetInfo("ISOSpeedRatings",source)

                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth:106
                        title: "曝光模式"
                        description: fileControl.slotGetInfo("ExposureMode",source)
                        Layout.fillWidth: true
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth:86
                        title: "曝光时间"
                        description: fileControl.slotGetInfo("ExposureTime",source)
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 1
                    PropertyItemDelegate {
                        contrlImplicitWidth:66
                        title: "闪光灯"
                        description: fileControl.slotGetInfo("Flash",source)                
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth:106
                        title: "闪光灯补偿"
                        description: fileControl.slotGetInfo("FlashExposureComp",source)
                        Layout.fillWidth: true
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth:86
                        title: "最大光圈值"
                        description: fileControl.slotGetInfo("MaxApertureValue",source)
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 1
                    PropertyItemDelegate {
                        contrlImplicitWidth:66
                        title: "颜色空间"
                        description: fileControl.slotGetInfo("ColorSpace",source)
                        corners: RoundRectangle.BottomLeftCorner
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth:106
                        title: "曝光模式"
                        description: fileControl.slotGetInfo("MeteringMode",source)
                        Layout.fillWidth: true
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth:86
                        title: "白平衡"
                        description: fileControl.slotGetInfo("WhiteBalance",source)
                        corners: RoundRectangle.BottomRightCorner
                    }
                }
            }

            PropertyItemDelegate {
                title: "设备型号"
                description: fileControl.slotGetInfo("Model",source)
                corners: RoundRectangle.AllCorner
            }
            PropertyItemDelegate {
                title: "镜头型号"
                description: fileControl.slotGetInfo("LensType",source)
                corners: RoundRectangle.AllCorner
            }
        }
    }
    onVisibleChanged: {
        setX(root.x+root.width-width-leftX)
        setY(root.y+topY)
    }
}
