import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Controls 2.4
import org.deepin.dtk 1.0

ApplicationWindow {
    //    flags:Qt.FramelessWindowHint


    GlobalVar{
        id: global
    }
    signal sigTitlePress
    // 设置 dtk 风格窗口
    DWindow.enabled: true
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

    onClosing: {
        fileControl.saveSetting() //保存信息
        fileControl.terminateShortcutPanelProcess() //结束快捷键面板进程
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




}



