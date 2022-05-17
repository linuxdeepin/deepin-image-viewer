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
        if(fileControl.isCheckOnly()){
            setX(screen.width / 2 - width / 2);
            setY(screen.height / 2 - height / 2);
        }
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
        property Palette backgroundColor1: Palette {
            normal: Qt.rgba(255/255, 255/255, 255/255, 0.6)
            normalDark:Qt.rgba(26/255, 26/255, 26/255, 0.6)
        }
        property Palette backgroundColor2: Palette {
            normal: Qt.rgba(255/255, 255/255, 255/255, 0.02)
            normalDark:Qt.rgba(26/255, 26/255, 26/255, 0.02)
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

    ViewTopTitle{
         id:titleRect
    }


}



