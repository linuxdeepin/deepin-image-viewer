import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import QtGraphicalEffects 1.0
import org.deepin.dtk 1.0

Item {
    property int currentIndex: 0
    // 用于外部获取当前缩略图栏内容的长度，用于布局
    property alias listContentWidth: bottomthumbnaillistView.contentWidth

    onCurrentIndexChanged: {
        bottomthumbnaillistView.currentIndex = currentIndex
        bottomthumbnaillistView.forceActiveFocus()
    }

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
        // 判断是否为多页图,多页图只进行页面替换
        if (imageViewer.currentIsMultiImage) {
            if (imageViewer.frameIndex != 0) {
                imageViewer.frameIndex--
                return
            }
        }

        if (bottomthumbnaillistView.currentIndex > 0) {
            bottomthumbnaillistView.currentIndex--
            source = mainView.sourcePaths[bottomthumbnaillistView.currentIndex]
            imageViewer.index = currentIndex

            // 向前移动的图像需要特殊判断，若为多页图，调整显示最后一张图
            if (fileControl.isMultiImage(source)) {
                imageViewer.frameIndex = fileControl.getImageCount(source) - 1;
            }
            bottomthumbnaillistView.forceActiveFocus()
        }
    }

    function next (){
        // 判断是否为多页图,多页图只进行页面替换
        if (imageViewer.currentIsMultiImage) {
            if (imageViewer.frameIndex < imageViewer.frameCount - 1) {
                imageViewer.frameIndex++
                return
            }
        }

        if (mainView.sourcePaths.length - 1 > bottomthumbnaillistView.currentIndex) {
            bottomthumbnaillistView.currentIndex++
            source = mainView.sourcePaths[bottomthumbnaillistView.currentIndex]
            imageViewer.index = currentIndex
            bottomthumbnaillistView.forceActiveFocus()
        }
    }

    IconButton {
        id: previousButton
        enabled: currentIndex > 0
                 || imageViewer.frameIndex > 0
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
        enabled: currentIndex < mainView.sourcePaths.length - 1
                 || imageViewer.frameIndex < imageViewer.frameCount - 1
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
            anchors.left: nextButton.right
            anchors.leftMargin:40

            anchors.top: parent.top
            anchors.topMargin: (parent.height - height) / 2
            width:50
            height:50
            icon.name:"icon_11"
            enabled:!CodeImage.imageIsNull(imageViewer.source)
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
            enabled:!CodeImage.imageIsNull(imageViewer.source)
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
            enabled:!CodeImage.imageIsNull(imageViewer.source) && fileControl.isRotatable(imageViewer.source)
            onClicked: {
                imageViewer.rotateImage(-90)

            // 动态刷新导航区域图片内容，同时可在imageviewer的sourceChanged中隐藏导航区域
            // (因导航区域图片source绑定到imageviewer的source属性)
                imageViewer.source = ""
                imageViewer.source = mainView.sourcePaths[bottomthumbnaillistView.currentIndex]
            }
            ToolTip.delay: 500
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Rotate")
        }

    ListView {
        id: bottomthumbnaillistView

        // 使用范围模式，允许高亮缩略图在preferredHighlightBegin~End的范围外，使缩略图填充空白区域
        highlightRangeMode: ListView.ApplyRange
        highlightFollowsCurrentItem: true

        preferredHighlightBegin: width / 2 - 25
        preferredHighlightEnd: width / 2 + 25

        anchors.left: rotateButton.right
        anchors.leftMargin: 10

        anchors.right: ocrButton.left
        anchors.rightMargin: 10

        anchors.top: parent.top
        anchors.topMargin: -5

        height: parent.height + 10
        width: parent.width - deleteButton.width - 60

        clip: true
        spacing: 10
        focus: true

        //滑动联动主视图
        onCurrentIndexChanged: {
            mainView.currentIndex = currentIndex
            source = mainView.sourcePaths[currentIndex]
            if (currentItem) {
                currentItem.forceActiveFocus()
            }

            // 特殊处理，防止默认显示首个缩略图时采用Center的策略会被遮挡部分
            if (0 == currentIndex) {
                positionViewAtBeginning()
            } else {
                // 尽可能将高亮缩略图显示在列表中
                positionViewAtIndex(currentIndex, ListView.Center)
            }
        }

        Connections {
            target: imageViewer
            onSwipeIndexChanged: {
                var imageSwipeIndex = imageViewer.swipeIndex
                if (currentIndex - imageSwipeIndex == 1) {
                    // 向前切换当通过拖动等方式时，调整多页图索引为最后一张图片
                    var curSource = sourcePaths[imageSwipeIndex]
                    if (fileControl.isMultiImage(curSource)) {
                        imageViewer.frameIndex = fileControl.getImageCount(curSource) - 1
                    }
                } else {
                    // 其它情况均设置为首张图片
                    imageViewer.frameIndex = 0
                }

                bottomthumbnaillistView.currentIndex = imageSwipeIndex
                currentIndex= imageSwipeIndex
                bottomthumbnaillistView.forceActiveFocus()
            }
        }

        orientation: Qt.Horizontal

        cacheBuffer: 400
        model: mainView.sourcePaths.length
        delegate: ListViewDelegate {
        }

         // 添加两组空的表头表尾用于占位，防止在边界的高亮缩略图被遮挡
        header: Rectangle {
            width: 10
        }

        footer: Rectangle {
            width: 10
        }
        
        Behavior on x {
            NumberAnimation {
                duration: 50
                easing.type: Easing.OutQuint
            }
        }

        Behavior on y {
            NumberAnimation {
                duration: 50
                easing.type: Easing.OutQuint
            }
        }
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
        enabled: fileControl.isCanSupportOcr(source) && !CodeImage.imageIsNull(source)
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
