import QtQuick 2.11

Item {
    property var imgPaths
    property var imgCurrentPath
    property int imgCurrentIndex:0
    property int minHeight:300
    property int minWidth:628
    property int minHideHeight:428
    property int floatMargin:60
    property int titleHeight:50
    property int showBottomY: 80
    property int actionMargin: 9//应用图标距离顶栏

    signal sigWindowStateChange()
}
