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

//            anchors.right: parent.right
//            anchors.rightMargin: (parent.width-width)/2
//            anchors.bottom: parent.bottom
//            anchors.bottomMargin: 10

            radius:15

            Component.onCompleted: {
                sliderFloatPanel.x=(screen.width-width)/2
                sliderFloatPanel.y=screen.height-80
            }

            ThumbnailButton {
                id:sliderPrevious
                icon.source:  "qrc:/res/dcc_previous_36px.svg"
                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.top: parent.top
                anchors.topMargin: parent.height/2-height/2
                onClickedLeft: {
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
            ThumbnailButton{
                id:sliderPause
                icon.source: autoRun ? "qrc:/res/dcc_suspend_normal_36px.svg": "qrc:/res/dcc_play_normal_36px.svg"
                anchors.left: sliderPrevious.right
                anchors.leftMargin: 10
                anchors.top: parent.top
                anchors.topMargin: parent.height/2-height/2
                onClickedLeft: {
                    autoRun=!autoRun
                }
                ToolTip.delay: 500
                ToolTip.timeout: 5000
                ToolTip.visible: hovered
                ToolTip.text: autoRun ? qsTr("Pause") : qsTr("Play")
            }
            ThumbnailButton{
                id:sliderNext
                icon.source:   "qrc:/res/dcc_next_36px.svg"
                anchors.left: sliderPause.right
                anchors.leftMargin: 10
                anchors.top: parent.top
                anchors.topMargin: parent.height/2-height/2
                onClickedLeft: {
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
        //        model: ObjectModelProxy {
        //            id: proxyModel
        //            property string filterText
        //            filterAcceptsItem: function(item) {
        //                return item.text.includes(filterText)
        //            }
        //            sourceModel: option_menu.contentModel
        //        }
        MenuItem {
            text:autoRun ?qsTr("Pause") : qsTr("Play")
            onTriggered: {
                autoRun=!autoRun
            }
        }

        MenuItem {
            text:qsTr("Exit")
            onTriggered: {
                outSliderShow()
            }
        }
    }
}
