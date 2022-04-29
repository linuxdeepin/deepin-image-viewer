import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Window 2.11
import QtGraphicalEffects 1.0

import org.deepin.dtk 1.0

Item {
    property alias source: imageViewer.source
    property alias sourcePaths: imageViewer.sourcePaths
    property alias currentIndex: imageViewer.swipeIndex

    signal closeFullThumbnail

    property bool isEnterCurrentView:true

    //    anchors.fill: rootItem

    function setThumbnailCurrentIndex(index) {
        thumbnailListView.currentIndex = index
    }

    //左右按钮隐藏动画
    NumberAnimation {
        id :hideLeftButtonAnimation
        target: floatLeftButton
        from: floatLeftButton.x
        to: -50
        property: "x"
        duration: 200
        easing.type: Easing.InOutQuad
    }

    NumberAnimation {
        id :showLeftButtonAnimation
        target: floatLeftButton
        from: floatLeftButton.x
        to: 20
        property: "x"
        duration: 200
        easing.type: Easing.InOutQuad
    }

    NumberAnimation {
        id :hideRightButtonAnimation
        target: floatRightButton
        from: floatRightButton.x
        to: parent.width
        property: "x"
        duration: 200
        easing.type: Easing.InOutQuad
    }

    NumberAnimation {
        id :showRightButtonAnimation
        target: floatRightButton
        from: floatRightButton.x
        to:parent.width-70
        property: "x"
        duration: 200
        easing.type: Easing.InOutQuad
    }

    //工具栏动画和标题栏动画
    NumberAnimation {
        id :hideBottomAnimation
        target: thumbnailViewBackGround
        from: thumbnailViewBackGround.y
        to: root.height
        property: "y"
        duration: 200
        easing.type: Easing.InOutQuad
    }
    NumberAnimation {
        id :hideTopTitleAnimation
        target: titleRect
        from: titleRect.y
        to: -50
        property: "y"
        duration: 200
        easing.type: Easing.InOutQuad
    }

    NumberAnimation {
        id :showBottomAnimation
        target: thumbnailViewBackGround
        from: thumbnailViewBackGround.y
        to:  root.height-global.showBottomY
        property: "y"
        duration: 200
        easing.type: Easing.InOutQuad
    }

    NumberAnimation {
        id :showTopTitleAnimation
        target: titleRect
        from: titleRect.y
        to: 0
        property: "y"
        duration: 200
        easing.type: Easing.InOutQuad
    }

    //判断工具栏和标题栏的显示隐藏
    function animationAll(){
        //        hideBottomAnimation.stop()
        //        hideTopTitleAnimation.stop()
        //        showRightButtonAnimation.stop()
        //        hideRightButtonAnimation.stop()
        if(root.visibility == Window.FullScreen ){
            if(imageViewerArea.mouseY > height-100){
                showBottomAnimation.start()
            }else{
                hideBottomAnimation.start()
                hideTopTitleAnimation.start()
            }
        }else if(currentWidgetIndex != 0 &&
                 ((root.height<=global.minHideHeight || root.width<=global.minWidth)&&(imageViewerArea.mouseY <= height-100) &&imageViewerArea.mouseY >= titleRect.height )){
            hideBottomAnimation.start()
            hideTopTitleAnimation.start()
        }else if(imageViewerArea.mouseY > height-100 || imageViewerArea.mouseY<titleRect.height ||
                 (imageViewer.currentScale <= 1.0*(root.height-titleRect.height*2)/root.height)){
            showBottomAnimation.start()
            showTopTitleAnimation.start()
        }else{
            hideBottomAnimation.start()
            hideTopTitleAnimation.start()
        }

        if(imageViewerArea.mouseX>=root.width-100 && imageViewerArea.mouseX<=root.width && isEnterCurrentView){
            showLeftButtonAnimation.start()
            showRightButtonAnimation.start()
        }else if(imageViewerArea.mouseX<=100 && imageViewerArea.mouseX>=0 && isEnterCurrentView){
            showLeftButtonAnimation.start()
            showRightButtonAnimation.start()
        }else {
            hideLeftButtonAnimation.start()
            hideRightButtonAnimation.start()
        }
    }

    //判断工具栏和标题栏的显示隐藏
    function changeSizeMoveAll(){
        showBottomAnimation.stop()
        showTopTitleAnimation.stop()
        hideBottomAnimation.stop()
        hideTopTitleAnimation.stop()
        showRightButtonAnimation.stop()
        hideRightButtonAnimation.stop()
        if(root.visibility==Window.FullScreen ){
            if(imageViewerArea.mouseY > height-100){
                thumbnailViewBackGround.y=root.height-global.showBottomY
            }else{
                thumbnailViewBackGround.y=root.height
                titleRect.y=-50
            }
        }else if(currentWidgetIndex != 0 &&
                 ((root.height<=global.minHideHeight || root.width<=global.minWidth)&&(imageViewerArea.mouseY <= height-100) &&imageViewerArea.mouseY >= titleRect.height )){
            thumbnailViewBackGround.y=root.height
            titleRect.y=-50
        }else if(imageViewerArea.mouseY > height-100 || imageViewerArea.mouseY<titleRect.height ||
                 (imageViewer.currentScale <= 1.0*(root.height-titleRect.height*2)/root.height)){
            thumbnailViewBackGround.y=root.height-global.showBottomY
            titleRect.y=0
        }else{
            thumbnailViewBackGround.y=root.height
            titleRect.y=-50
        }

        if(imageViewerArea.mouseX<=100 && imageViewerArea.mouseX<=root.width && isEnterCurrentView){
            floatLeftButton.x=20
            floatRightButton.x=parent.width-70
        }else if(imageViewerArea.mouseX>=root.width-100 && imageViewerArea.mouseX>=0 && isEnterCurrentView){
            floatLeftButton.x=20
            floatRightButton.x=parent.width-70
        }else{
            floatLeftButton.x=-50
            floatRightButton.x=parent.width
        }
    }

    function slotShowFullScreen(){
        thumbnailViewBackGround.y=Screen.height
        floatRightButton.x=Screen.width

    }
    function slotMaxWindow(){
        thumbnailViewBackGround.y=Screen.height
        floatRightButton.x=Screen.width
    }

    onHeightChanged: {
        changeSizeMoveAll()
    }

    onWidthChanged: {
        changeSizeMoveAll()
    }

    ImageViewer {
        id: imageViewer
        anchors.fill: parent
    }
    Connections {
        target: imageViewer
        onSigWheelChange :{
            animationAll()
        }
    }


    FloatingButton {
        id:floatLeftButton
        visible: mainView.sourcePaths.length>1 && currentIndex>0
        enabled: currentIndex>0? true:false
        checked: false
        anchors.top: parent.top
        anchors.topMargin: global.titleHeight+(parent.height-global.titleHeight-global.showBottomY)/2
        icon.name : "icon_previous"
        width: 50
        height: 50
        onClicked: {
            thumbnailListView.previous();
        }
        Component.onCompleted: {
            animationAll()
        }
    }
    FloatingButton {
        id:floatRightButton
        checked: false
        visible: mainView.sourcePaths.length>1 && currentIndex<mainView.sourcePaths.length-1
        enabled: currentIndex<mainView.sourcePaths.length-1 ? true:false
        anchors.top: parent.top
        anchors.topMargin: global.titleHeight+(parent.height-global.titleHeight-global.showBottomY)/2
        width: 50
        height: 50
        icon.name:"icon_next"
        onClicked: {
            thumbnailListView.next();
        }
        Component.onCompleted: {
            animationAll()
        }
    }


    MouseArea{
        anchors.fill: imageViewer
        id:imageViewerArea
        acceptedButtons: Qt.LeftButton
        hoverEnabled: true
        onMouseYChanged: {
            animationAll()
            mouse.accepted = false;
        }

        onEntered: {
            isEnterCurrentView = true
            animationAll()
        }

        onExited: {
            isEnterCurrentView = false
            animationAll()
        }

    }

    FloatingPanel {
        id: thumbnailViewBackGround

        width: parent.width - 30 < 500+sourcePaths.length*50 ? parent.width - 30 : 500+sourcePaths.length*50
        height: 70

        anchors.right: parent.right
        anchors.rightMargin: (parent.width-width)/2

        Component.onCompleted: {

            animationAll()
        }
    }

    ThumbnailListView {
        id: thumbnailListView
        anchors.fill: thumbnailViewBackGround

        //         property int currentIndex: 0
    }

    //浮动提示框
    FloatingNotice {
        id: floatLabel
        visible: false
        anchors.bottom: thumbnailViewBackGround.top
        anchors.bottomMargin: global.floatMargin
        anchors.left: parent.left
        anchors.leftMargin: parent.width / 2 - 50
        opacity: 0.7

        Timer {
            interval: 1500
            running: parent.visible
            repeat: false
            onTriggered: {
                parent.visible = false
            }
        }
    }

}
