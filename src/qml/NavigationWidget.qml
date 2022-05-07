import QtQuick 2.0
import QtQuick.Window 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Styles 1.2
import QtGraphicalEffects 1.0

Rectangle {
    id: idNavigationwidget
    width: 150
    height: 102
    clip: true
    radius: 10

    signal changeShowImgPostions(var x, var y)

    //初始状态以中心向两边延展
    function setRectPec(a) {
        if(a < 1)
            return
        var w = 150 / a
        var h = 102 / a
        idrectArea.width = w
        idrectArea.height = h
        idrectArea.x = 75 - w / 2
        idrectArea.y = 51 - h / 2
    }

    //计算蒙皮位置
    function setRectLocation(x, y) {
        var x1 = idNavigationwidget.width * x
        var y1 = idNavigationwidget.height * y
        x1 -= idrectArea.width
        y1 -= idrectArea.height
        if(x1 < 0 || y1 < 0)
            return
        idrectArea.x = x1
        idrectArea.y = y1
    }

    //背景图片绘制区域
    Rectangle {
        id: idImgRect
        anchors.fill: parent
        radius: 10

        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: Rectangle {
                width: idImgRect.width
                height: idImgRect.height
                radius: idImgRect.radius
            }
        }

        Image {
            id: idcurrentImg
            fillMode: Image.PreserveAspectFit
            cache: false
            width: parent.width
            height: parent.height
            asynchronous: true
            source: "image://viewImage/"+imageViewer.source
        }
    }
    //test 前端获取后端加载到的图像数据，放开以下代码在缩放时会有弹窗显示后端加载的图像
    /*Window {
        id : rrr
        visible: false
        Image {
            id: aaa
            anchors.fill: parent
        }
    }
    Connections {
        target: CodeImage
        onCallQmlRefeshImg:
        {
            aaa.source = ""
            aaa.source = "image://ThumbnailImage"
            rrr.show()
        }
    }*/

    //退出按钮
    ToolButton {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 3
        anchors.topMargin: 3
        width: 22
        height: 22

        Image {
            source: "qrc:/res/close_hover.svg"
            anchors.fill: parent
        }

        background: Rectangle {
            radius: 50
        }
        onClicked: {
            idNavigationwidget.visible = false
            isNavShow = false
        }
        z: 100
    }

    //显示范围蒙皮
    Rectangle {
        id: idrectArea
        opacity: 0.4
        color: "black"
    }

    //允许拖动范围
    MouseArea {
        anchors.fill: parent
        drag.target: idrectArea
        drag.axis: Drag.XAndYAxis//设置拖拽的方式
        //TODO 应以图片的范围来限制拖动范围
        drag.minimumX: 0
        drag.maximumX: parent.width - idrectArea.width
        drag.minimumY: 0
        drag.maximumY: parent.height - idrectArea.height

        property bool isPressed: false

        //拖拽与主界面的联动
        onPressed: {
            isPressed = true
            var x = mouseX
            var y = mouseY
            idrectArea.x = x - idrectArea.width / 2 > 0 ? x - idrectArea.width / 2 : 0
            idrectArea.y = y - idrectArea.height / 2 > 0 ? y - idrectArea.height / 2 : 0


            var j = idrectArea.x + idrectArea.width
            var k = idrectArea.y + idrectArea.height

            var x1 = j / idNavigationwidget.width
            var y1 = k / idNavigationwidget.height
            //导航拖动与主界面联动,x1和y1即为主界面传入时的比例
            changeShowImgPostions(x1,y1)
        }

        onReleased: {
            isPressed = false
        }

        onPositionChanged: {
            if (isPressed) {
                //当前蒙皮位置对应比例发送给视图
                var x = idrectArea.x + idrectArea.width
                var y = idrectArea.y + idrectArea.height
                //左上角相对全图的点的比例,x1和y1即为比例，将此左上角坐标告知大图视图
                var x1 = x / idNavigationwidget.width
                var y1 = y / idNavigationwidget.height
                //导航拖动与主界面联动,x1和y1即为主界面传入时的比例
                changeShowImgPostions(x1,y1)
            }
        }
    }
}
