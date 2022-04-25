import QtQuick 2.11

import QtQuick.Controls 2.4
import org.deepin.dtk 1.0 as D
import org.deepin.dtk 1.0
ApplicationWindow {
    //    flags:Qt.FramelessWindowHint


    GlobalVar{
        id: global
    }

    // 设置 dtk 风格窗口
    D.DWindow.enabled: true
    id: root
    title: ""

    visible: true
    minimumHeight:330
    minimumWidth:628
    width: 800
    height: 600
    //    color: "transparent"
    flags:Qt.Window |Qt.WindowMinMaxButtonsHint |Qt.WindowCloseButtonHint|Qt.WindowTitleHint
    Component.onCompleted: {
        setX(screen.width / 2 - width / 2);
        setY(screen.height / 2 - height / 2);
    }

    onWindowStateChanged: {
        global.sigWindowStateChange()
    }



//        header:tt

    Rectangle {
        id: rect
//        color:"#F8F8F8"
        width: root.width;
        height: root.height;
        color: "black"
        anchors.centerIn: parent

        MainStack{

            anchors.fill: parent
//            interactive: false
        }
    }

    Rectangle {
        id:titleRect
        anchors.top:root.top
//        anchors.topMargin: 10
        width: parent.width
        height: 50
        visible: root.visibility === 5 ? false:true
        color:"white"
        opacity: 1
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
        }
    }


}



