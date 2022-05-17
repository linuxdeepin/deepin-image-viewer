import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Controls 2.4
import org.deepin.dtk 1.0

Rectangle {

    anchors.top:root.top
//        anchors.topMargin: 10
    width: parent.width
    height: 50
    visible: root.visibility === 5 ? false:true
    color:titlecontrol.ColorSelector.backgroundColor
    gradient: Gradient {
           GradientStop { position: 0.0; color: titlecontrol.ColorSelector.backgroundColor1 }
           GradientStop { position: 1.0; color: titlecontrol.ColorSelector.backgroundColor2 }
       }
    //opacity: 1
    ActionButton {
        anchors.top:titleRect.top
        anchors.topMargin:global.actionMargin
        anchors.left:titleRect.left
        anchors.leftMargin:global.actionMargin
        icon {
            name: "deepin-image-viewer"
            width: 32
            height: 32
        }
    }

    MouseArea { //为窗口添加鼠标事件
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton //只处理鼠标左键
        property point clickPos: "0,0"
        onPressed: { //接收鼠标按下事件
            clickPos  = Qt.point(mouse.x,mouse.y)
            sigTitlePress()
        }
        onPositionChanged: { //鼠标按下后改变位置
            //鼠标偏移量
            var delta = Qt.point(mouse.x-clickPos.x, mouse.y-clickPos.y)

            //如果mainwindow继承自QWidget,用setPos
            root.setX(root.x+delta.x)
            root.setY(root.y+delta.y)
            //               rect1.x = rect1.x + delta.x
            //               rect1.y = rect1.y + delta.y
        }
    }
    TitleBar {
        id :title
        anchors.fill:parent
        aboutDialog: AboutDialog{
            icon:"deepin-image-viewer"
//                productIcon:"deepin-image-viewer"
            width:400
            modality:Qt.NonModal
            version:qsTr(String("Version: %1").arg(Qt.application.version))
            description:qsTr("Image Viewer is an image viewing tool with fashion interface and smooth performance.")
            productName:qsTr("deepin-image-viewer")
            websiteName:DTK.deepinWebsiteName
            websiteLink:DTK.deepinWebsitelLink
            license:qsTr(String("%1 is released under %2").arg(productName).arg("GPLV3"))
        }
    }
}
