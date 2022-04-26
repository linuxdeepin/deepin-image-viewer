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

    RowLayout {
        id: leftLayout
        visible: mainView.sourcePaths.length>1

        anchors.left: parent.left
        anchors.leftMargin: 15

        anchors.top: parent.top
        anchors.topMargin: (parent.height - height) / 2

        //        ThumbnailButton {
        //            icon.source: "qrc:/res/dcc_back_36px.svg"
        //            onClickedLeft: closeFullThumbnail()
        //        }
        IconButton {

            icon {
                width: 32
                height: 32
                name: "go-previous"
            }
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
            ToolTip.text: qsTr("Next")

        }
        IconButton {
            icon {
                width: 32
                height: 32
                name: "go-next"
            }
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
            ToolTip.text: qsTr("Previous")
        }
    }
    RowLayout {
        id: fitButtonLayout

        anchors.left: mainView.sourcePaths.length>1 ?leftLayout.right:parent.left
        anchors.leftMargin:mainView.sourcePaths.length>1 ? 40 : 15

        anchors.top: parent.top
        anchors.topMargin: (parent.height - height) / 2
        ThumbnailButton {
            icon.source: "qrc:/res/dcc_11_36px.svg"

            onClickedLeft: {
                imageViewer.fitImage()
            }
            ToolTip.delay: 500
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Original size")
        }
        ThumbnailButton {
            icon.source: "qrc:/res/dcc_fit_36px.svg"
            onClickedLeft: {
                imageViewer.fitWindow()
            }
            ToolTip.delay: 500
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Fit to window")
        }

        ThumbnailButton {
            icon.source: "qrc:/res/dcc_left_36px.svg"
            onClickedLeft: {
                imageViewer.rotateImage(-90)

//                bottomthumbnaillistView.currentItem.rotation=bottomthumbnaillistView.currentItem.rotation-90
            }
            ToolTip.delay: 500
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Rotate")
        }
//        ThumbnailButton {
//            icon.source: "qrc:/res/dcc_right_36px.svg"
//            onClickedLeft: {
//                imageViewer.rotateImage(90)
//            }
//        }
    }

    ListView {
        id: bottomthumbnaillistView
        highlightRangeMode: ListView.StrictlyEnforceRange
        highlightFollowsCurrentItem: true

        preferredHighlightBegin: width/2 - 25
        preferredHighlightEnd: width/2 + 25

        anchors.left: fitButtonLayout.right
        anchors.leftMargin: 15

        anchors.right: ocrButton.left
        anchors.rightMargin: 15

        anchors.top: parent.top

        height: parent.height
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

    ThumbnailButton {
        id :ocrButton
        anchors.right: deleteButton.left
        anchors.rightMargin: 15

        anchors.top: parent.top
        anchors.topMargin: (parent.height - height) / 2

        icon.source: "qrc:/res/dcc_ocr_36px.svg"
        onClickedLeft: {
            fileControl.ocrImage(source)
        }
        ToolTip.delay: 500
        ToolTip.timeout: 5000
        ToolTip.visible: hovered
        ToolTip.text: qsTr("Extract text")
    }

    ThumbnailButton {
        id: deleteButton

        anchors.right: parent.right
        anchors.rightMargin: 15

        anchors.top: parent.top
        anchors.topMargin: (parent.height - height) / 2

        icon.source: "qrc:/res/dcc_delete_36px.svg"
        icon.color: enabled ? "red" :"ffffff"
        onClickedLeft: {
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
