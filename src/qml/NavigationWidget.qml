import QtQuick 2.0
import QtQuick.Window 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Styles 1.2
import QtGraphicalEffects 1.0

Rectangle {
    id: idNavigationwidget
    width: 150
    height: 112
    clip: true
    radius: 10

    // 使用浮点，避免精度丢失导致拖拽边界有细微偏差
    property real imgLeft : 0
    property real imgTop: 0
    property real imgRight: 0
    property real imgBottom: 0

    signal changeShowImgPostions(var x, var y)

    // 初始状态以中心向两边延展
    function setRectPec(scale, viewWidthRatio, viewHeightRatio) {
        if(scale <= 1)
            return

        // 需要计算viewport矩形在图片上的映射，再投影到蒙皮上
        // 取得原始图片调整后的大小
        var imgw = 0
        var imgh = 0
        var ratio = idcurrentImg.sourceSize.width / idcurrentImg.sourceSize.height
        if (idcurrentImg.sourceSize.width < idcurrentImg.sourceSize.height) {
            imgw = ratio * idNavigationwidget.height
            imgh = idNavigationwidget.height
        } else {
            imgw = idNavigationwidget.width
            imgh = idNavigationwidget.width / ratio
        }

        // 调整蒙皮大小为图片相较显示区域的大小
        idrectArea.width = viewWidthRatio < 1 ? imgw * viewWidthRatio : imgw
        idrectArea.height = viewHeightRatio < 1 ? imgh * viewHeightRatio: imgh

        idrectArea.x = (idNavigationwidget.width - idrectArea.width) / 2
        idrectArea.y = (idNavigationwidget.height - idrectArea.height) / 2

        // 记录图片显示区域位置信息
        imgLeft = (idNavigationwidget.width - imgw) / 2
        imgTop = (idNavigationwidget.height - imgh) / 2
        imgRight = imgLeft + imgw
        imgBottom = imgTop + imgh
    }

    //计算蒙皮位置
    function setRectLocation(xRatio, yRatio) {
        // 根据可移动区域计算变更的 X,Y值
        var xOffset = xRatio * (imgRight - imgLeft - idrectArea.width)
        idrectArea.x = xOffset + imgLeft
        var yOffset = yRatio * (imgBottom - imgTop - idrectArea.height)
        idrectArea.y = yOffset + imgTop
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

            // 多页图使用不同图像加载类
            source: {
                if (!visible) {
                    return ""
                } else {
                    return imageViewer.currentIsMultiImage
                            ? "image://multiimage/" + imageViewer.source + "#frame_" + imageViewer.frameIndex
                            : "image://viewImage/" + imageViewer.source
                }
            }
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
        // 以图片的范围来限制拖动范围
        drag.minimumX: imgLeft
        drag.maximumX: imgRight - idrectArea.width
        drag.minimumY: imgTop
        drag.maximumY: imgBottom - idrectArea.height

        property bool isPressed: false

        //拖拽与主界面的联动
        onPressed: {
            isPressed = true
            var x = mouseX
            var y = mouseY
            console.info("x: ",x, "y: ", y)
            idrectArea.x = x - idrectArea.width / 2 > 0 ? x - idrectArea.width / 2 : 0
            idrectArea.y = y - idrectArea.height / 2 > 0 ? y - idrectArea.height / 2 : 0

            // 限定鼠标点击的蒙皮在图片内移动
            if (idrectArea.x < imgLeft)
                idrectArea.x = imgLeft
            if (idrectArea.y < imgTop)
                idrectArea.y = imgTop
            if ((idrectArea.x + idrectArea.width) > imgRight)
                idrectArea.x = imgRight - idrectArea.width
            if ((idrectArea.y + idrectArea.height) > imgBottom)
                idrectArea.y = imgBottom - idrectArea.height

            // 根据按键位置更新图片展示区域
            var j = idrectArea.x - imgLeft
            var k = idrectArea.y - imgTop
            // 需要注意蒙皮本身的宽高在计算时应移除
            var x1 = j / (imgRight - imgLeft - idrectArea.width)
            var y1 = k / (imgBottom - imgTop - idrectArea.height)
            //导航拖动与主界面联动,x1和y1即为主界面传入时的比例
            changeShowImgPostions(x1,y1)
        }

        onReleased: {
            isPressed = false
        }

        onPositionChanged: {
            if (isPressed) {
                // 当前蒙皮位置对应比例发送给视图，移除图片和背景矩形边界间的误差
                var x = idrectArea.x - imgLeft
                var y = idrectArea.y - imgTop
                // 左上角相对全图的点的比例,x1和y1即为比例，将此左上角坐标告知大图视图
                // 需要注意蒙皮本身的宽高在计算时应移除
                var x1 = x / (imgRight - imgLeft - idrectArea.width)
                var y1 = y / (imgBottom - imgTop - idrectArea.height)

                // 导航拖动与主界面联动,x1和y1即为主界面传入时的比例
                changeShowImgPostions(x1, y1)
            }
        }
    }
}
