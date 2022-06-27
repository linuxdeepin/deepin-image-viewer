import QtQuick 2.11
import QtQuick.Controls 2.4
import org.deepin.dtk 1.0

Rectangle {

    property int indexImg
    property var images:[]
    property int modelCount
    property bool autoRun: false

    signal backtrack()
    color: "#000000"
    Timer {
        id: timer
        interval: 3000
        running: autoRun
        repeat: true
        property bool reserve: false

        onTriggered: {
            sliderMainShow.indexImg++
            if (sliderMainShow.indexImg > sliderMainShow.images.length-1) {
                sliderMainShow.indexImg=0
            }
        }
    }

    function restart() {
        fadeInOutImage.restart()
    }

    function outSliderShow(){
        showNormal()
        sliderMainShow.autoRun = false
        sliderMainShow.backtrack()
        if(stackView.currentWidgetIndex===2){
            mainView.currentIndex=sliderMainShow.indexImg
        }
        stackView.currentWidgetIndex=1
    }

    SFadeInOut {
        id: fadeInOutImage
        anchors.fill: parent
        imageSource: images[indexImg]
        width: parent.width
        height: parent.width
    }

    onBacktrack: {
        sliderArea.cursorShape= "ArrowCursor"
    }
    MouseArea{
        anchors.fill: parent
        id:sliderArea
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        cursorShape: "BlankCursor"
        onClicked: {
            console.log("right menu");
            if (mouse.button === Qt.RightButton) {
                sliderMenu.popup()
            }
        }
        onDoubleClicked:{
            console.log("onDoubleClicked");
            outSliderShow()
        }

        onCursorShapeChanged: {
            sliderCursorTimer.start()
        }
        Timer {
            id :sliderCursorTimer
            interval: 3000  //设置定时器定时时间为500ms,默认1000ms
            running: true  //是否开启定时，默认是false，当为true的时候，进入此界面就开始定时
            repeat: true   //是否重复定时,默认为false
            onTriggered:  sliderArea.cursorShape= "BlankCursor"
        }

        hoverEnabled: true
        onMouseXChanged: {
            sliderArea.cursorShape= "ArrowCursor"
        }
        onMouseYChanged: {
            sliderArea.cursorShape= "ArrowCursor"
            if(mouseY > height-100){
                showSliderAnimation.start()
            }else{
//                sliderFloatPanel.visible=false
                hideSliderAnimation.start()
            }
        }
        NumberAnimation {
            id :hideSliderAnimation
            target: sliderFloatPanel
            from: sliderFloatPanel.y
            to: screen.height
            property: "y"
            duration: 200
            easing.type: Easing.InOutQuad
        }

        NumberAnimation {
            id :showSliderAnimation
            target: sliderFloatPanel
            from: sliderFloatPanel.y
            to: screen.height-80
            property: "y"
            duration: 200
            easing.type: Easing.InOutQuad
        }

        FloatingPanel {
            id: sliderFloatPanel

            width: 232
            height: 70

            Component.onCompleted: {
                sliderFloatPanel.x=(screen.width-width)/2
                sliderFloatPanel.y=screen.height-80
            }

            IconButton {
                id:sliderPrevious
                icon.name : "icon_previous"
                width:50
                height:50
                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.top: parent.top
                anchors.topMargin: parent.height/2-height/2
                onClicked: {
                    sliderMainShow.indexImg--
                    if (sliderMainShow.indexImg < 0) {
                        sliderMainShow.indexImg=images.length-1
                    }
                    autoRun=false
                }

                ToolTip.delay: 500
                ToolTip.timeout: 5000
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Previous")
            }
            IconButton{
                id:sliderPause
                icon.name : autoRun ? "icon_suspend": "icon_play_start"
                width:50
                height:50
                anchors.left: sliderPrevious.right
                anchors.leftMargin: 10
                anchors.top: parent.top
                anchors.topMargin: parent.height/2-height/2
                onClicked: {
                    autoRun=!autoRun
                }
                ToolTip.delay: 500
                ToolTip.timeout: 5000
                ToolTip.visible: hovered
                ToolTip.text: autoRun ? qsTr("Pause") : qsTr("Play")
            }
            IconButton{
                id:sliderNext
                icon.name : "icon_next"
                width:50
                height:50
                anchors.left: sliderPause.right
                anchors.leftMargin: 10
                anchors.top: parent.top
                anchors.topMargin: parent.height/2-height/2
                onClicked: {
                    console.log("next")
                    sliderMainShow.indexImg++
                    if (sliderMainShow.indexImg > sliderMainShow.images.length-1) {
                        sliderMainShow.indexImg=0
                    }
                    autoRun=false
                }
                ToolTip.delay: 500
                ToolTip.timeout: 5000
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Next")
            }

            ActionButton {
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.top: parent.top
                anchors.topMargin: parent.height/2-height/2
                icon.name :"entry_clear"
                width:24
                height:24
                onClicked: {
                    outSliderShow()
                }
                ToolTip.delay: 500
                ToolTip.timeout: 5000
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Exit")
            }
            
        }
    }

    Menu {
        x: 250; y: 600
        id: sliderMenu

        MenuItem {
            text:autoRun ?qsTr("Pause") : qsTr("Play")
            onTriggered: {
                autoRun=!autoRun
            }

            // 添加处理快捷键，播放幻灯片时暂停/播放
            Shortcut {
                id: pauseShortCut
                sequence: "Space"
                onActivated: {
                    autoRun = !autoRun
                }
            }
        }

        MenuItem {
            text:qsTr("Exit")
            onTriggered: {
                outSliderShow()
            }
        }
    }

    onVisibleChanged: {
        // 进行幻灯片播放时允许响应空格快捷键处理暂停/播放
        pauseShortCut.enabled = visible
    }
}
