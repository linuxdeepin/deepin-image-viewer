import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import QtGraphicalEffects 1.0
import org.deepin.dtk 1.0

Item {
    property int currentIndex: 0
    //      property alias currentIndex :bottomthumbnaillistView.currentIndex

    onCurrentIndexChanged: {
        bottomthumbnaillistView.currentIndex = currentIndex
        bottomthumbnaillistView.forceActiveFocus()
    }

    //    Timer{
    //        interval: 200
    //        running: true
    //        repeat: true
    //        onTriggered: {
    //           bottomthumbnaillistView.forceActiveFocus()
    //        }
    //    }
    function rotateImage( x ){
        bottomthumbnaillistView.currentItem.rotation=bottomthumbnaillistView.currentItem.rotation +x
    }

    function deleteCurrentImage(){

        if (mainView.sourcePaths.length - 1 > bottomthumbnaillistView.currentIndex) {
            var tempPathIndex=bottomthumbnaillistView.currentIndex
            var tmpPath=source
            //需要保存临时变量，重置后赋值
            imageViewer.sourcePaths = fileControl.removeList(sourcePaths,tempPathIndex)
            imageViewer.swipeIndex=tempPathIndex
            fileControl.deleteImagePath(tmpPath)
        }else if(mainView.sourcePaths.length - 1 == 0){
            stackView.currentWidgetIndex=0
            root.title=""
            fileControl.deleteImagePath(imageViewer.sourcePaths[0])
            imageViewer.sourcePaths=fileControl.removeList(sourcePaths,0)

        }else{
            bottomthumbnaillistView.currentIndex--
            imageViewer.source = imageViewer.sourcePaths[bottomthumbnaillistView.currentIndex]
            fileControl.deleteImagePath(sourcePaths[bottomthumbnaillistView.currentIndex+1])
            imageViewer.sourcePaths = fileControl.removeList(imageViewer.sourcePaths,bottomthumbnaillistView.currentIndex+1)
        }
    }

    function previous (){
        if (bottomthumbnaillistView.currentIndex > 0) {
            bottomthumbnaillistView.currentIndex--
            source = mainView.sourcePaths[bottomthumbnaillistView.currentIndex]
            imageViewer.index = currentIndex
            bottomthumbnaillistView.forceActiveFocus()
        }
    }

    function next (){
        if (mainView.sourcePaths.length - 1 > bottomthumbnaillistView.currentIndex) {
            bottomthumbnaillistView.currentIndex++
            source = mainView.sourcePaths[bottomthumbnaillistView.currentIndex]
            imageViewer.index = currentIndex
            bottomthumbnaillistView.forceActiveFocus()
        }
    }

    IconButton {
        id: previousButton
        visible: mainView.sourcePaths.length>1
        enabled: currentIndex>0? true:false

        anchors.left: parent.left
        anchors.leftMargin: 15

        anchors.top: parent.top
        anchors.topMargin: (parent.height - height) / 2

        width:50
        height:50
        icon.name: "icon_previous"
        onClicked: {
            previous();
        }

        Shortcut{
            sequence: "Left"
            onActivated: previous();
        }

        ToolTip.delay: 500
        ToolTip.timeout: 5000
        ToolTip.visible: hovered
        ToolTip.text: qsTr("Previous")

    }
    IconButton {
        id: nextButton
        visible: mainView.sourcePaths.length>1
        enabled: currentIndex<mainView.sourcePaths.length-1? true:false
        anchors.left: previousButton.right
        anchors.leftMargin: 10

        anchors.top: parent.top
        anchors.topMargin: (parent.height - height) / 2

        width:50
        height:50
        icon.name:"icon_next"
        onClicked: {
            next();
        }
        Shortcut{
            sequence: "Right"
            onActivated: next();
        }
        ToolTip.delay: 500
        ToolTip.timeout: 5000
        ToolTip.visible: hovered
        ToolTip.text: qsTr("Next")
    }



        IconButton {
            id: fitImageButton
            anchors.left: mainView.sourcePaths.length>1 ?nextButton.right:parent.left
            anchors.leftMargin:mainView.sourcePaths.length>1 ? 40 : 15

            anchors.top: parent.top
            anchors.topMargin: (parent.height - height) / 2
            width:50
            height:50
            icon.name:"icon_11"

            onClicked: {
                imageViewer.fitImage()
            }
            ToolTip.delay: 500
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Original size")
        }
        IconButton {
            id: fitWindowButton
            anchors.left: fitImageButton.right
            anchors.leftMargin:10

            anchors.top: parent.top
            anchors.topMargin: (parent.height - height) / 2

            width:50
            height:50
            icon.name:"icon_self-adaption"
            onClicked: {
                imageViewer.fitWindow()
            }
            ToolTip.delay: 500
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Fit to window")
        }

        IconButton {
            id: rotateButton
            anchors.left: fitWindowButton.right
            anchors.leftMargin:10

            anchors.top: parent.top
            anchors.topMargin: (parent.height - height) / 2

            width:50
            height:50
            icon.name:"icon_rotate"
            onClicked: {
                imageViewer.rotateImage(-90)

//                bottomthumbnaillistView.currentItem.rotation=bottomthumbnaillistView.currentItem.rotation-90
            }
            ToolTip.delay: 500
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Rotate")
        }


    ListView {
        id: bottomthumbnaillistView
        highlightRangeMode: ListView.StrictlyEnforceRange
        highlightFollowsCurrentItem: true

        preferredHighlightBegin: width/2 - 25
        preferredHighlightEnd: width/2 + 25

        anchors.left: rotateButton.right
        anchors.leftMargin: 15

        anchors.right: ocrButton.left
        anchors.rightMargin: 15

        anchors.top: parent.top
        anchors.topMargin: -5

        height: parent.height+10
        width: parent.width - deleteButton.width - 60

        clip: true
        spacing: 10
        focus: true

        //滑动联动主视图
        onCurrentIndexChanged: {
            mainView.currentIndex=currentIndex

            source = mainView.sourcePaths[currentIndex]
            if(currentItem){
                currentItem.forceActiveFocus()
            }
            positionViewAtIndex(currentIndex,ListView.Center)
        }

        Connections {
            target: imageViewer
            onSwipeIndexChanged: {
                bottomthumbnaillistView.currentIndex = imageViewer.swipeIndex
                currentIndex= imageViewer.swipeIndex
                bottomthumbnaillistView.forceActiveFocus()
            }
        }

        orientation: Qt.Horizontal

        cacheBuffer: 200
        model: mainView.sourcePaths.length
        delegate: ListViewDelegate {
        }

        Behavior on y {
            NumberAnimation {
                duration: 50
                easing.type: Easing.OutQuint
            }
        }

        Keys.enabled: true
        Keys.onPressed: {
            switch (event.key) {
            case Qt.Key_Left:
                if (currentIndex > 0) {
                    currentIndex--
                    source = mainView.sourcePaths[currentIndex]
                }
                break
            case Qt.Key_Right:
                if (mainView.sourcePaths.length - 1 > currentIndex) {
                    currentIndex++
                    source = mainView.sourcePaths[currentIndex]
                }
                break
            }
            event.accepted = true
        }

        //        Component.onCompleted: {
        //            console.log("123456")
        //            bottomthumbnaillistView.currentIndex = 0
        //            bottomthumbnaillistView.forceActiveFocus();
        //        }
    }

    IconButton {
        id :ocrButton
        width:50
        height:50
        icon.name:"icon_character_recognition"
        anchors.right: deleteButton.left
        anchors.rightMargin: 15

        anchors.top: parent.top
        anchors.topMargin: (parent.height - height) / 2

        onClicked: {
            fileControl.ocrImage(source)
        }
        ToolTip.delay: 500
        ToolTip.timeout: 5000
        ToolTip.visible: hovered
        ToolTip.text: qsTr("Extract text")
    }

    IconButton {
        id: deleteButton
        width:50
        height:50
        icon.name: "icon_delete"

        anchors.right: parent.right
        anchors.rightMargin: 15

        anchors.top: parent.top
        anchors.topMargin: (parent.height - height) / 2

        icon.source: "qrc:/res/dcc_delete_36px.svg"
        icon.color: enabled ? "red" :"ffffff"
        onClicked: {
            deleteCurrentImage()
        }
        //        visible: fileControl.isCanDelete(source) ? true :false

        enabled: fileControl.isCanDelete(source) ? true :false

        ToolTip.delay: 500
        ToolTip.timeout: 5000
        ToolTip.visible: hovered
        ToolTip.text: qsTr("Delete")

    }
}
