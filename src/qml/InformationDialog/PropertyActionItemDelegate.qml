// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import org.deepin.dtk 1.0
import org.deepin.dtk.style 1.0 as DS
import org.deepin.image.viewer 1.0 as IV

Control {
    id: control

    property Component action: ActionButton {
        Layout.alignment: Qt.AlignRight
        visible: showPicLabel.visible

        onClicked: {
            dealShowPicLabelClick();
        }

        icon {
            height: 14
            name: control.iconName
            width: 14
        }
    }
    property int corners: RoundRectangle.NoneCorner
    property string description
    property int descriptionWidth: control.width - leftPadding - rightPadding - 20
    property string iconName
    property Palette infoTextColor: Palette {
        normal: Qt.rgba(0, 0, 0, 1)
        normalDark: Qt.rgba(1, 1, 1, 1)
    }
    property Palette sectionTextColor: Palette {
        normal: Qt.rgba(0, 0, 0, 0.6)
        normalDark: Qt.rgba(1, 1, 1, 0.6)
    }
    property string title

    // 允许组件接收焦点（用于转移焦点）
    focus: true

    signal clicked

    function dealShowPicLabelClick() {
        if (showPicLabel.visible) {
            showPicLabel.visible = false;
            // 每次显示编辑框时显示为图片名称
            nameedit.text = IV.FileControl.slotGetFileName(IV.GControl.currentSource);
            nameedit.forceActiveFocus();
        } else {
            var name = nameedit.text;
            if (!IV.FileControl.isShowToolTip(IV.GControl.currentSource, name) && name.length > 0) {
                IV.FileControl.slotFileReName(name, IV.GControl.currentSource);
            }
            showPicLabel.visible = true;
        }
    }

    // 复位当前属性编辑组件，关闭编辑框（不保存）
    function reset() {
        showPicLabel.visible = true;
    }

    // 提交更改并关闭编辑框（保存文件名）
    function commitAndClose() {
        if (!showPicLabel.visible) {
            var name = nameedit.text;
            if (!IV.FileControl.isShowToolTip(IV.GControl.currentSource, name) && name.length > 0) {
                IV.FileControl.slotFileReName(name, IV.GControl.currentSource);
            }
            showPicLabel.visible = true;
        }
    }

    // 检查是否处于编辑状态
    function isEditing() {
        return !showPicLabel.visible;
    }

    bottomPadding: 4
    leftPadding: 10
    rightPadding: 10
    topPadding: 3

    background: RoundRectangle {
        color: Qt.rgba(0, 0, 0, 0.05)
        corners: control.corners
        implicitHeight: 40
        implicitWidth: 66
        radius: Style.control.radius
    }
    contentItem: ColumnLayout {
        spacing: 0

        ElideLabel {
            Layout.fillWidth: true
            // 系数微调整以满足默认字号标签均显示的效果
            Layout.minimumWidth: descriptionWidth + 5
            color: control.ColorSelector.sectionTextColor
            font: DTK.fontManager.t10
            sourceText: control.title
            tipsColor: control.palette.toolTipText
            width: descriptionWidth
        }

        RowLayout {
            LineEdit {
                id: nameedit

                Layout.fillWidth: true
                alertText: qsTr("The file already exists, please use another name")
                color: control.ColorSelector.infoTextColor
                focus: true
                font.pixelSize: 16
                height: 20
                selectByMouse: true
                showAlert: IV.FileControl.isShowToolTip(IV.GControl.currentSource, nameedit.text) && nameedit.visible
                text: IV.FileControl.slotGetFileName(IV.GControl.currentSource)
                visible: !showPicLabel.visible

                // 限制输入特殊字符
                validator: RegularExpressionValidator {
                    regularExpression: /^[^ \\.\\\\/\':\\*\\?\"<>|%&][^\\\\/\':\\*\\?\"<>|%&]*/
                }

                Keys.onPressed: {
                    if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                        dealShowPicLabelClick();
                    }
                    if (event.key === Qt.Key_Escape) {
                        reset();
                    }
                }

                // 焦点变化时处理 - 失去焦点时保存文件名
                onActiveFocusChanged: {
                    // 失去焦点且输入框可见时，保存文件名
                    if (!activeFocus && visible && !showPicLabel.visible) {
                        // 延迟执行，避免与其他焦点事件冲突
                        commitAndCloseTimer.restart();
                    }
                }

                // 失去焦点时保存文件名（备用方案）
                onEditingFinished: {
                    // 只有当输入框可见时才处理（避免在 reset 时触发）
                    if (visible && !showPicLabel.visible) {
                        dealShowPicLabelClick();
                    }
                }

                Timer {
                    id: commitAndCloseTimer
                    interval: 10
                    onTriggered: {
                        if (!nameedit.activeFocus && nameedit.visible && !showPicLabel.visible) {
                            dealShowPicLabelClick();
                        }
                    }
                }

                anchors {
                    leftMargin: 10
                    topMargin: 5
                }
            }

            ElideLabel {
                id: showPicLabel

                Layout.fillWidth: true
                color: control.ColorSelector.infoTextColor
                sourceText: control.description
                tipsColor: control.palette.toolTipText
                width: descriptionWidth
            }

            Loader {
                Layout.leftMargin: 5
                sourceComponent: control.action
            }
        }
    }
}
