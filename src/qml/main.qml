import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Controls 2.4
import org.deepin.dtk 1.0 as D
import org.deepin.dtk 1.0
ApplicationWindow {
    //    flags:Qt.FramelessWindowHint


    GlobalVar{
        id: global
    }
    signal sigTitlePress
    // 设置 dtk 风格窗口
    D.DWindow.enabled: true
    id: root
    title: ""

    visible: true
    minimumHeight:global.minHeight
    minimumWidth:global.minWidth

    width: fileControl.getlastWidth()
    height: fileControl.getlastHeight()

    flags:Qt.Window |Qt.WindowMinMaxButtonsHint |Qt.WindowCloseButtonHint|Qt.WindowTitleHint
    Component.onCompleted: {
        setX(screen.width / 2 - width / 2);
        setY(screen.height / 2 - height / 2);
    }

    onWindowStateChanged: {
        global.sigWindowStateChange()
    }

    onWidthChanged: {
        if(root.visibility!=Window.FullScreen && root.visibility !=Window.Maximized){
            fileControl.setSettingWidth(width)
        }

    }
    onHeightChanged: {
        if(root.visibility!=Window.FullScreen &&root.visibility!=Window.Maximized){
            fileControl.setSettingHeight(height)
        }
    }
    //关闭的时候保存信息
    onClosing: {
        fileControl.saveSetting()
    }

//        header:tt
    Control {
        id: titlecontrol
        hoverEnabled: true // 开启 Hover 属性
        property Palette backgroundColor: Palette {
            normal: "white"
            normalDark:"#262626"
        }
    }
    Control {
        id: backcontrol
        hoverEnabled: true // 开启 Hover 属性
        property Palette backgroundColor: Palette {
            normal: "#F8F8F8"
            normalDark:"#000000"
        }
    }
    Rectangle {
        id: rect

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
        color:titlecontrol.ColorSelector.backgroundColor
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


}



