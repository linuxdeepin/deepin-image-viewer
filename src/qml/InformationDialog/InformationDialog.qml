// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.deepin.dtk 1.0
import org.deepin.image.viewer 1.0 as IV

DialogWindow {
    id: dialog

    Accessible.name: fileName
    Accessible.role: Accessible.Dialog

    property string fileName: IV.FileControl.slotGetFileNameSuffix(filePath)
    property url filePath: IV.GControl.currentSource
    property int leftX: 20
    property int propFullWidth: 260
    property int propLeftWidth: 66
    property int propMidWidth: 106
    property int propRightWidth: 86
    property int topY: 70

    flags: Qt.Dialog | Qt.WindowCloseButtonHint | Qt.MSWindowsFixedSizeDialogHint
    maximumWidth: 280
    minimumWidth: 280
    visible: false
    width: 280

    header: DialogTitleBar {
        property string title: fileName

        enableInWindowBlendBlur: false

        content: Loader {
            sourceComponent: Label {
                anchors.centerIn: parent
                elide: Text.ElideMiddle
                font: DTK.fontManager.t6
                horizontalAlignment: Text.AlignHCenter
                text: title
                textFormat: Text.PlainText
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    Component.onCompleted: {
        setX(window.x + window.width - width - leftX);
        setY(window.y + topY);
        show();
    }

    // 窗口关闭时复位组件状态
    onClosing: {
        fileNameProp.reset();
        IV.GStatus.showImageInfo = false;
    }

    // 图片变更时复位组件状态(切换时关闭重命名框)
    onFileNameChanged: {
        fileNameProp.reset();
    }

    ColumnLayout {
        id: contentHeight

        width: 260

        anchors {
            horizontalCenter: parent.horizontalCenter
            margins: 10
        }

        PropertyItem {
            title: qsTr("Basic info")
            Accessible.name: "BasicInfo"
            Accessible.role: Accessible.ListItem

            ColumnLayout {
                spacing: 1

                PropertyActionItemDelegate {
                    id: fileNameProp

                    Layout.fillWidth: true
                    corners: RoundRectangle.TopCorner
                    description: fileName
                    iconName: "action_edit"
                    implicitWidth: propFullWidth
                    title: qsTr("File name")
                    Accessible.name: "FileNameProp"
                    Accessible.role: Accessible.ListItem
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 1

                    PropertyItemDelegate {
                        contrlImplicitWidth: propLeftWidth
                        corners: RoundRectangle.BottomLeftCorner
                        description: IV.FileControl.slotGetInfo("FileSize", filePath)
                        title: qsTr("Size")
                        Accessible.name: "InfoSize"
                        Accessible.role: Accessible.ListItem
                    }

                    PropertyItemDelegate {
                        Layout.fillWidth: true
                        contrlImplicitWidth: propMidWidth
                        description: {
                            var originalDim = IV.FileControl.slotGetInfo("OriginalDimension", filePath);
                            if (originalDim && originalDim !== "-") {
                                return originalDim;
                            } else {
                                return imageInfo.width + "x" + imageInfo.height;
                            }
                        }
                        title: qsTr("Dimensions")
                        Accessible.name: "Dimensions"
                        Accessible.role: Accessible.ListItem
                    }

                    PropertyItemDelegate {
                        contrlImplicitWidth: propRightWidth
                        corners: RoundRectangle.BottomRightCorner
                        description: IV.FileControl.slotFileSuffix(filePath, false)
                        title: qsTr("Type")
                        Accessible.name: "InfoType"
                        Accessible.role: Accessible.ListItem
                    }
                }
            }

            ColumnLayout {
                spacing: 1

                PropertyItemDelegate {
                    Layout.fillWidth: true
                    corners: RoundRectangle.TopCorner
                    description: IV.FileControl.slotGetInfo("DateTimeOriginal", filePath)
                    title: qsTr("Date captured")
                    Accessible.name: "DateCaptured"
                    Accessible.role: Accessible.ListItem
                }

                PropertyItemDelegate {
                    Layout.fillWidth: true
                    corners: RoundRectangle.BottomCorner
                    description: IV.FileControl.slotGetInfo("DateTimeDigitized", filePath)
                    title: qsTr("Date modified")
                    Accessible.name: "DateModified"
                    Accessible.role: Accessible.ListItem
                }
            }
        }

        PropertyItem {
            id: detailInfoItem
            Accessible.name: "DetailInfoItem"
            Accessible.role: Accessible.ListItem

            // 详细信息默认不显示，会影响自动布局效果，因此目前设置为固定布局
            showProperty: false
            title: qsTr("Details")

            GridLayout {
                Layout.fillWidth: true
                columnSpacing: 1
                columns: 3
                rowSpacing: 1
                rows: 4

                PropertyItemDelegate {
                    contrlImplicitWidth: propLeftWidth
                    corners: RoundRectangle.TopLeftCorner
                    description: IV.FileControl.slotGetInfo("ApertureValue", filePath)
                    title: qsTr("Aperture")
                    Accessible.name: "Aperture"
                    Accessible.role: Accessible.ListItem
                }

                PropertyItemDelegate {
                    Layout.fillWidth: true
                    Layout.minimumWidth: propMidWidth
                    contrlImplicitWidth: propMidWidth
                    description: IV.FileControl.slotGetInfo("ExposureProgram", filePath)
                    title: qsTr("Exposure program")
                    Accessible.name: "ExposureProgram"
                    Accessible.role: Accessible.ListItem
                }

                PropertyItemDelegate {
                    contrlImplicitWidth: propRightWidth
                    corners: RoundRectangle.TopRightCorner
                    description: IV.FileControl.slotGetInfo("FocalLength", filePath)
                    title: qsTr("Focal length")
                    Accessible.name: "FocalLength"
                    Accessible.role: Accessible.ListItem
                }

                PropertyItemDelegate {
                    contrlImplicitWidth: propLeftWidth
                    description: IV.FileControl.slotGetInfo("ISOSpeedRatings", filePath)
                    title: qsTr("ISO")
                    Accessible.name: "Iso"
                    Accessible.role: Accessible.ListItem
                }

                PropertyItemDelegate {
                    Layout.fillWidth: true
                    contrlImplicitWidth: propMidWidth
                    description: IV.FileControl.slotGetInfo("ExposureMode", filePath)
                    title: qsTr("Exposure mode")
                    Accessible.name: "ExposureMode"
                    Accessible.role: Accessible.ListItem
                }

                PropertyItemDelegate {
                    contrlImplicitWidth: propRightWidth
                    description: IV.FileControl.slotGetInfo("ExposureTime", filePath)
                    title: qsTr("Exposure time")
                    Accessible.name: "ExposureTime"
                    Accessible.role: Accessible.ListItem
                }

                PropertyItemDelegate {
                    contrlImplicitWidth: propLeftWidth
                    description: IV.FileControl.slotGetInfo("Flash", filePath)
                    title: qsTr("Flash")
                    Accessible.name: "Flash"
                    Accessible.role: Accessible.ListItem
                }

                PropertyItemDelegate {
                    Layout.fillWidth: true
                    contrlImplicitWidth: propMidWidth
                    description: IV.FileControl.slotGetInfo("FlashExposureComp", filePath)
                    title: qsTr("Flash compensation")
                    Accessible.name: "FlashCompensation"
                    Accessible.role: Accessible.ListItem
                }

                PropertyItemDelegate {
                    contrlImplicitWidth: propRightWidth
                    description: IV.FileControl.slotGetInfo("MaxApertureValue", filePath)
                    title: qsTr("Max aperture")
                    Accessible.name: "MaxAperture"
                    Accessible.role: Accessible.ListItem
                }

                PropertyItemDelegate {
                    contrlImplicitWidth: propLeftWidth
                    corners: RoundRectangle.BottomLeftCorner
                    description: IV.FileControl.slotGetInfo("ColorSpace", filePath)
                    title: qsTr("Colorspace")
                    Accessible.name: "Colorspace"
                    Accessible.role: Accessible.ListItem
                }

                PropertyItemDelegate {
                    Layout.fillWidth: true
                    contrlImplicitWidth: propMidWidth
                    description: IV.FileControl.slotGetInfo("MeteringMode", filePath)
                    title: qsTr("Metering mode")
                    Accessible.name: "MeteringMode"
                    Accessible.role: Accessible.ListItem
                }

                PropertyItemDelegate {
                    contrlImplicitWidth: propRightWidth
                    corners: RoundRectangle.BottomRightCorner
                    description: IV.FileControl.slotGetInfo("WhiteBalance", filePath)
                    title: qsTr("White balance")
                    Accessible.name: "WhiteBalance"
                    Accessible.role: Accessible.ListItem
                }
            }

            ColumnLayout {
                spacing: 1

                PropertyItemDelegate {
                    contrlImplicitWidth: propFullWidth
                    corners: RoundRectangle.AllCorner
                    description: IV.FileControl.slotGetInfo("Model", filePath)
                    title: qsTr("Device model")
                    Accessible.name: "DeviceModel"
                    Accessible.role: Accessible.ListItem
                }

                PropertyItemDelegate {
                    contrlImplicitWidth: propFullWidth
                    corners: RoundRectangle.AllCorner
                    description: IV.FileControl.slotGetInfo("LensType", filePath)
                    title: qsTr("Lens model")
                    Accessible.name: "LensModel"
                    Accessible.role: Accessible.ListItem
                }
            }
        }

        // 隐藏占位组件
        Item {
            id: footer

            Layout.preferredHeight: detailInfoItem.showProperty ? 5 : 10
            width: 10
        }
    }

    // DialogWindow 无法直接包含 ImageInfo
    Item {
        IV.ImageInfo {
            id: imageInfo

            frameIndex: IV.GControl.currentFrameIndex
            source: IV.GControl.currentSource
        }
    }
}
