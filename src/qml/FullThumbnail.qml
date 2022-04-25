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

    //    anchors.fill: rootItem

    function setThumbnailCurrentIndex(index) {
        thumbnailListView.currentIndex = index
        console.log(index)
    }
    function changeTitleBottomY( ){
        if(root.height<=global.minHideHeight ||root.width<=global.minWidth){
            hideBottomAnimation.start()
            hideTopTitleAnimation.start()
        }
        else if(imageViewerArea.mouseY > height-100 || imageViewerArea.mouseY<titleRect.height || imageViewer.currentScale <= 1.0*(root.height-titleRect.height*2)/root.height){
            showBottomAnimation.start()
            showTopTitleAnimation.start()
        }else{
            hideBottomAnimation.start()
            hideTopTitleAnimation.start()
        }


    }
    onHeightChanged: {
        changeTitleBottomY()
    }
    onWidthChanged: {
        changeTitleBottomY()
    }

    ImageViewer {
        id: imageViewer
        anchors.fill: parent
    }
    Connections {
        target: imageViewer
        onSigWheelChange :{
            changeTitleBottomY()
        }
    }


    MouseArea{
        anchors.fill: imageViewer
        id:imageViewerArea
        acceptedButtons: Qt.LeftButton
        //        enabled: false
        hoverEnabled: true
        onMouseYChanged: {
            changeTitleBottomY()
             mouse.accepted = false;
        }

        onEntered: {
            changeTitleBottomY()
        }

        onExited: {
            changeTitleBottomY()
        }

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
            to: root.height-90
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
    }

    FloatingPanel {
        id: thumbnailViewBackGround

        width: parent.width - 30 < 500+sourcePaths.length*50 ? parent.width - 30 : 500+sourcePaths.length*50
        height: 80

        anchors.right: parent.right
        anchors.rightMargin: (parent.width-width)/2

        radius:15
        //        backgroundColor: Palette{
        //            normal : "#F0F0F0"
        //        }
        Component.onCompleted: {
            changeTitleBottomY()
        }
    }

    ThumbnailListView {
        id: thumbnailListView
        anchors.fill: thumbnailViewBackGround

        //         property int currentIndex: 0
    }



}
