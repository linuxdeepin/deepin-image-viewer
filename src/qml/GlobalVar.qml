import QtQuick 2.11

Item {
    property var imgPaths
    property var imgCurrentPath
    property int imgCurrentIndex:0
    property int minHeight:300
    property int minWidth:628
    property int minHideHeight:425      //调整窗口高度小于425px时，隐藏工具栏和标题栏
    property int floatMargin:65
    property int titleHeight:50
    property int showBottomY: 80
    property int actionMargin: 9        //应用图标距离顶栏

    property int rightMenuItemHeight: 32//右键菜单item的高度

    signal sigWindowStateChange()

    property bool animationBlock: false
}
