import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Window 2.10
import QtQuick.Controls 2.4
import org.deepin.dtk 1.0 as D
import org.deepin.dtk 1.0

DialogWindow {
    id: renamedialog
    modality: Qt.WindowModal
    flags: Qt.Window | Qt.WindowCloseButtonHint
    title: " "
    visible: false
//    minimumWidth: 380
//    maximumWidth: 380
//    minimumHeight: 178
//    maximumHeight: 178

    minimumWidth: 400
    maximumWidth: 400
    minimumHeight: 220
    maximumHeight: 220

    width: 400
    height: 220
//    opacity: 1

    icon :"deepin-image-viewer"
//    color: "#E5DEDA"

//    D.DWindow.enabled: true

//    TitleBar {
//        Rectangle {
//            anchors.top: parent.top
//            anchors.topMargin: 0

//        }
//        width: parent.width
//        height:50
//    }

    function getFileName(name)  {
        nameedit.text = name
    }
    function getFileSuffix(suffix) {
        filesuffix.text = suffix
    }
    Text {
        id: renametitle
        width: parent.width
        height:48
        anchors.left: parent.left
        anchors.top:parent.top
        font.pixelSize: 16
        //        text: qsTr("Input a new name")
        text: qsTr("请输入图片名称")
        verticalAlignment: Text.AlignBottom
        horizontalAlignment: Text.AlignHCenter
    }
    Rectangle {
        id: nameeditrect
        width:320
        height: 35
        color: "transparent"
//        border.color: "grey"
        anchors.left: parent.left
        anchors.top: renametitle.bottom
        anchors.leftMargin:10
        anchors.topMargin:14
//        anchors.bottomMargin:10
        LineEdit {
            id: nameedit
//            text: "test"
            anchors.fill: nameeditrect
            anchors.topMargin:5
            anchors.leftMargin:10
            font.pixelSize: 16
            focus: true
            selectByMouse: true
            alertText: qsTr("文件名已被占用,请使用其他名称")
            showAlert: fileControl.isShowToolTip(source,nameedit.text)
        }
    }

    Text {
        id: filesuffix
//        height: 35
        font.pixelSize: 16
        text: ".jpg"
        anchors.right: parent.right
        anchors.top: renametitle.bottom
        anchors.rightMargin:10
        anchors.topMargin:20
    }

    Button {
        id: cancelbtn
//        text: qsTr("Cancel")
        text: qsTr("取消")
        width: 169
        height: 33
        font.pixelSize: 16
        anchors.left: parent.left
        anchors.top: nameeditrect.bottom
        anchors.leftMargin:10
        anchors.topMargin:25
        onClicked:{
            renamedialog.visible = false
        }

    }

    RecommandButton {
        id: enterbtn
//        text: qsTr("Confirm")
        text: qsTr("确定")
        enabled:!fileControl.isShowToolTip(source,nameedit.text)&& nameedit.text.length>0
        width: 169
        height: 33
        font.pixelSize: 16
        anchors.right: parent.right
        anchors.top: nameeditrect.bottom
        anchors.rightMargin:10
        anchors.topMargin:25
        font.capitalization: Font.MixedCase

        onClicked:{
            var name = nameedit.text
            //bool返回值判断是否成功

            if(fileControl.slotFileReName(name,source)){
                sourcePaths=fileControl.renameOne(sourcePaths,source,fileControl.getNamePath(source,name))
                source=fileControl.getNamePath(source,name)

            }
            renamedialog.visible = false
        }
        palette {
                button: "#25CD00"
        }
    }
    onVisibleChanged: {
        console.log(width)
        setX(root.x  + root.width/2 - width / 2);
        setY(root.y  + root.height/2 - height / 2);
    }
}
