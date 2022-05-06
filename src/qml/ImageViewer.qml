import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Controls 2.4
//import QtQuick.Controls 1.4
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0

Rectangle {

    // Indicates the minimum number of zooms
    property int minScaleLevel: 10
    // Indicates the maximum number of zooms
    property int maxScaleLevel: 30
    // Indicates the current scaleLevel of zooms
    property int currentScaleLevel: 0
    //current rotate
    property int currentRotate: 0

    // Indicates the current image path
    property var source
    /*: showImg.source*/
    property var sourcePaths

    property int currentSourceWidth : 0;

    property int currentSourceHeight : 0;

    property int index: 0
    property alias swipeIndex: view.currentIndex

    //是否显示和隐藏导航栏
    property bool  isNavShow : true

    property double  currentScale : 1.0

    property double  currentimgX : 0.0

    property double  currentimgY : 0.0

    property double  currenImageScale : currentScale / CodeImage.getFitWindowScale(source,root.width, root.height) * 100

    property bool isMousePinchArea: true

    property double readWidthHeightRatio: CodeImage.getrealWidthHeightRatio(imageViewer.source)
    //导航蒙皮位置
    property double  m_NavX : 0.0
    property double  m_NavY : 0.0

    signal sigWheelChange
    signal sigImageShowFullScreen
    signal sigImageShowNormal
    signal sigSourceChange

    //    color: "#F8F8F8"

    color: backcontrol.ColorSelector.backgroundColor
    ViewRightMenu {
        id: option_menu
    }
    Connections {
        target: root
        onSigTitlePress: {
            infomationDig.hide()
        }
    }

    function showFloatLabel() {
        console.info("scale value:", currenImageScale.toFixed(0))
        if(currenImageScale.toFixed(0) > 2000 && currenImageScale.toFixed(0) <= 3000){
            floatLabel.displayStr = "2000%"
        }else if(currenImageScale.toFixed(0)<2 && currenImageScale.toFixed(0) >=0 ){
            floatLabel.displayStr = "2%"
        }else if(currenImageScale.toFixed(0) >=2 && currenImageScale.toFixed(0) <= 2000 ){
            floatLabel.displayStr = currenImageScale.toFixed(0) + "%"
        }
        floatLabel.visible = CodeImage.imageIsNull(source)||currenImageScale.toFixed(0)<0 ||currenImageScale.toFixed(0)>2000 ? false : true
    }

    onCurrenImageScaleChanged: {
        showFloatLabel()
    }

    onCurrentScaleChanged: {
        idNavWidget.setRectPec(currentScale)

        if(currenImageScale>2000){
            currentScale = 20 * CodeImage.getFitWindowScale(source,root.width, root.height)
        } else if(currenImageScale<2 &&currenImageScale>0){
            currentScale = 0.02 * CodeImage.getFitWindowScale(source,root.width, root.height)
        }
    }


    onSourceChanged: {

        fileControl.slotRotatePixCurrent();

        console.log("source:", mainView.source)
        fileControl.setCurrentImage(source)

        idNavWidget.visible = false
//        if(fileControl.getFitWindowScale(root.width,root.height-100)<1.0){
//            fitImage()
//        }else{
//            fitWindow()
//        }
        fitWindow()

        root.title = fileControl.slotGetFileName(source) + fileControl.slotFileSuffix(source)

        showFloatLabel()

        sigSourceChange();

        mainView.animationAll()

    }


    function fitImage()
    {
        currentScale = CodeImage.getFitWindowScale(source,root.width, root.height);
    }

    function fitWindow()
    {
        currentScale = root.visibility == Window.FullScreen ? 1.0 : 1.0 * (root.height - titleRect.height * 2) / root.height
    }
    function rotateImage(x)
    {
        //        rotation = currentRotate + x
        view.currentItem.rotation = view.currentItem.rotation + x
        thumbnailListView.rotateImage(x)
        fileControl.rotateFile(source, x)
    }
    function deleteItem(item, list)
    {
        // 先遍历list里面的每一个元素，对比item与每个元素的id是否相等，再利用splice的方法删除
        for (var key in fileList) {
            if (list[key].id === item) {
                list.splice(key, 1)
            }
        }
    }
    function startSliderShow()
    {
        if (sourcePaths.length > 0) {
            showFullScreen()
            sliderMainShow.images = sourcePaths
            sliderMainShow.modelCount = sourcePaths.length
            sliderMainShow.autoRun = true
            sliderMainShow.indexImg = view.currentIndex
            stackView.currentWidgetIndex = 2
        }

    }

    PropertyAnimation {
        id :showfullAnimation
        target: root
        from: 0
        to: 1
        property: "opacity"
        duration: 200
        easing.type: Easing.InExpo
    }
    Timer {
        id: showFulltimer
        interval: 200
        running: false
        repeat: false

        onTriggered: {
            root.visibility != Window.FullScreen ? showPanelFullScreen() : imageViewer.escBack()
        }
    }
    function showPanelFullScreen()
    {
        showFullScreen()
        view.contentItem.forceActiveFocus()
        showfullAnimation.start()

        //如果是初始界面只全屏
        if (stackView.currentWidgetIndex != 0) {
            stackView.currentWidgetIndex = 1
            currentScale = 1.0
            sigImageShowFullScreen()
        }
    }

    function escBack()
    {
        showNormal()
        showfullAnimation.start()
        //如果是初始界面只正常大小
        if (stackView.currentWidgetIndex != 0) {
            sliderMainShow.autoRun = false
            sliderMainShow.backtrack()
            if (stackView.currentWidgetIndex == 2) {
                mainView.currentIndex = sliderMainShow.indexImg
            }

            stackView.currentWidgetIndex = 1

            sigImageShowNormal()
        }
    }
    //缩放快捷键
    Shortcut {
        sequence: "Ctrl+="
        onActivated: {
            currentScale =  currentScale / 0.9
        }
    }

    Shortcut {
        sequence: "Ctrl+-"
        onActivated: {
            currentScale =  currentScale * 0.9
        }
    }

    Shortcut {
        sequence: "Up"
        onActivated: {
            currentScale =  currentScale / 0.9
        }
    }


    Shortcut {
        sequence: "Down"
        onActivated: {
            currentScale =  currentScale * 0.9
        }
    }



    SwipeView {
        id: view
//        interactive:false
        currentIndex: sourcePaths.indexOf(source)
        width: parent.width
        height: parent.height
        //        anchors.centerIn: parent

        //初始打开和点击缩略图切换都不会再有滑动效果
        Component.onCompleted:{
            contentItem.highlightMoveDuration = 0       //将移动时间设为0
        }


        clip: true
        //onCurrentIndexChanged: {
        //    view.currentItem.rotation=0
        ////    view.currentItem.source=
        //}

        Repeater {
            model: sourcePaths.length

            Loader {

                active: SwipeView.isCurrentItem || SwipeView.isNextItem
                        || SwipeView.isPreviousItem
                sourceComponent: Rectangle {

                    id: flickableL
                    width: CodeImage.getImageWidth(sourcePaths[index])
                    height: CodeImage.getImageHeight(sourcePaths[index])

                    clip: true
                    color: backcontrol.ColorSelector.backgroundColor

                    //normal image
                    Image {
                        id: showImg

                        fillMode: Image.PreserveAspectFit
                        width: parent.width
                        height: parent.height
                        source:  fileControl.isNormalStaticImage(sourcePaths[index]) ? "image://viewImage/"+sourcePaths[index] : ""
                        visible: fileControl.isNormalStaticImage(sourcePaths[index]) && !CodeImage.imageIsNull(sourcePaths[index])
                        asynchronous: true

                        cache: false
                        clip: true
                        scale: currentScale
                        mipmap: true
                        smooth: true

                        onStatusChanged: {
                            msArea.changeRectXY()

                            if (showImg.status === Image.Ready)
                                console.log('Ready')
                            if (showImg.status === Image.Loading)
                                console.log('Loading')
                            if (showImg.status === Image.Null)
                                console.info('Null')
                        }
                    }

                    //svg image
                    Image {
                        id: showSvgImg

                        fillMode: Image.PreserveAspectFit
                        width: parent.width
                        height: parent.height
                        source:  fileControl.isSvgImage(sourcePaths[index]) ? sourcePaths[index] : ""
                        visible: fileControl.isSvgImage(sourcePaths[index]) && !CodeImage.imageIsNull(sourcePaths[index])
                        asynchronous: true
                        sourceSize: Qt.size(width,height)
                        cache: false
                        clip: true
                        scale: currentScale
                        smooth: true
                        mipmap: true


                        onStatusChanged: {
                            msArea.changeRectXY()
                        }
                    }

                    //dynamic image
                    AnimatedImage {
                        id: showAnimatedImg

                        fillMode: Image.PreserveAspectFit
                        width: parent.width
                        height: parent.height
                        source:  fileControl.isDynamicImage(sourcePaths[index]) ? sourcePaths[index] : ""
                        visible: fileControl.isDynamicImage(sourcePaths[index]) && !CodeImage.imageIsNull(sourcePaths[index])
                        asynchronous: true
                        cache: false
                        clip: true
                        scale: currentScale
                        smooth: true

                        onStatusChanged: {
                            msArea.changeRectXY()
                        }
                    }

                    ActionButton {
                        id: damageIcon
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        icon {
                            name: "photo_breach"
                            width: 151
                            height: 151
                        }
                        visible: CodeImage.imageIsNull(sourcePaths[index])

                    }
                    BusyIndicator {
                        running: true
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        width:48
                        height:48
                        visible: showImg.status === Image.Loading && !CodeImage.imageIsNull(sourcePaths[index])
                    }


                    Connections {
                        target: root
                        onSigTitlePress: {
                            infomationDig.hide()
                            msArea.forceActiveFocus()
                        }
                    }

                    PinchArea {
                        enabled: isMousePinchArea
                        anchors.fill: showAnimatedImg.visible ? showAnimatedImg : showImg

                        onPinchStarted : {
                            pinch.accepted = true
                        }

                        onPinchUpdated: {
                            if (pinch.scale < 5 && pinch.scale > 0.2)
                            {
                                currentScale = pinch.scale;
                            }
                        }

                        onPinchFinished: {
                            if (pinch.scale < 5 && pinch.scale > 0.2)
                            {
                                currentScale = pinch.scale;
                            }
                        }
                        MultiPointTouchArea{
                            anchors.fill: parent
                            minimumTouchPoints: 1
                            maximumTouchPoints: 3
                        }
                    }
                    MouseArea {
                        id: msArea
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        drag.target: showAnimatedImg.visible ? showAnimatedImg : showImg.visible ?  showImg : showSvgImg
                        enabled : isMousePinchArea
                        function setImgPostions(x, y)
                        {
                            currentimgX = msArea.drag.maximumX - x * (msArea.drag.maximumX - msArea.drag.minimumX)
                            currentimgY = msArea.drag.maximumY - y * (msArea.drag.maximumY - msArea.drag.minimumY)
                            if (showAnimatedImg.visible) {
                                showAnimatedImg.x = currentimgX
                                showAnimatedImg.y = currentimgY
                            } else if(showImg.visible) {
                                showImg.x = currentimgX
                                showImg.y = currentimgY
                            }else if(showSvgImg.visible){
                                showSvgImg.x = currentimgX
                                showSvgImg.y = currentimgY
                            }
                        }

                        Connections {
                            target: idNavWidget
                            onChangeShowImgPostions : {
                                msArea.setImgPostions(x, y)
                            }
                        }


                        property int realWidth : 0
                        property int realHeight : 0
                        function changeRectXY()
                        {
                            readWidthHeightRatio = CodeImage.getrealWidthHeightRatio(imageViewer.source)
                            realWidth = 0;
                            realHeight = 0;
                            if (currentScale <= 1.0) {
                                drag.minimumX = 0
                                drag.minimumY = 0
                                drag.maximumX = 0
                                drag.maximumY = 0
                                showAnimatedImg.x = 0;
                                showAnimatedImg.y = 0;
                                showImg.x = 0;
                                showImg.y = 0;
                                showSvgImg.x = 0;
                                showSvgImg.y = 0;
                            } else {
                                if (showAnimatedImg.visible) {
                                    if (showAnimatedImg.width > showAnimatedImg.height * readWidthHeightRatio){
                                        realWidth = showAnimatedImg.height * readWidthHeightRatio
                                    }else{
                                        realWidth = showAnimatedImg.width
                                    }
                                    if(showAnimatedImg.height > showAnimatedImg.width / readWidthHeightRatio){
                                        realHeight = showAnimatedImg.width / readWidthHeightRatio
                                    }else{
                                        realHeight = showAnimatedImg.height
                                    }
                                } else if (showImg.visible) {
                                    if(showImg.width > showImg.height * readWidthHeightRatio){
                                        realWidth = showImg.height * readWidthHeightRatio
                                    }else{
                                        realWidth = showImg.width
                                    }
                                    if(showImg.height > showImg.width / readWidthHeightRatio){
                                        realHeight = showImg.width / readWidthHeightRatio
                                    }else{
                                        realHeight = showImg.height
                                    }

                                } else if (showSvgImg.visible) {
                                    if(showSvgImg.width > showSvgImg.height * readWidthHeightRatio){
                                        realWidth = showSvgImg.height * readWidthHeightRatio
                                    }else{
                                        realWidth = showSvgImg.width
                                    }
                                    if(showSvgImg.height > showSvgImg.width / readWidthHeightRatio){
                                        realHeight = showSvgImg.width / readWidthHeightRatio
                                    }else{
                                        realHeight = showSvgImg.height
                                    }
                                }

                                drag.minimumX = - realWidth * (currentScale-1)/2
                                drag.maximumX =  realWidth * (currentScale-1)/2
                                drag.minimumY = - realHeight * (currentScale-1)/2
                                drag.maximumY =  realHeight * (currentScale-1)/2
                                if (realHeight * currentScale >root.height) {
                                    drag.maximumY = ( realHeight * currentScale - root.height )/2
                                    drag.minimumY = - drag.maximumY
                                } else {
                                    drag.maximumY = 0
                                    drag.minimumY = 0
                                }

                                if (realWidth * currentScale > root.width) {
                                    drag.maximumX = ( realWidth * currentScale - root.width )/2
                                    drag.minimumX = - drag.maximumX
                                } else {
                                    drag.maximumX = 0
                                    drag.minimumX = 0
                                }
                            }

                            if (showAnimatedImg.x >= drag.maximumX) {
                                showAnimatedImg.x = drag.maximumX
                            }
                            if (showAnimatedImg.x <= drag.minimumX) {
                                showAnimatedImg.x = drag.minimumX
                            }
                            if (showAnimatedImg.y >= drag.maximumY) {
                                showAnimatedImg.y = drag.maximumY
                            }
                            if (showAnimatedImg.y <= drag.minimumY) {
                                showAnimatedImg.y = drag.minimumY
                            }


                            if (showImg.x >= drag.maximumX) {
                                showImg.x = drag.maximumX
                            }
                            if (showImg.x <= drag.minimumX) {
                                showImg.x = drag.minimumX
                            }
                            if (showImg.y >= drag.maximumY) {
                                showImg.y = drag.maximumY
                            }
                            if (showImg.y <= drag.minimumY) {
                                showImg.y = drag.minimumY
                            }

                            if (showSvgImg.x >= drag.maximumX) {
                                showSvgImg.x = drag.maximumX
                            }
                            if (showSvgImg.x <= drag.minimumX) {
                                showSvgImg.x = drag.minimumX
                            }
                            if (showSvgImg.y >= drag.maximumY) {
                                showSvgImg.y = drag.maximumY
                            }
                            if (showSvgImg.y <= drag.minimumY) {
                                showSvgImg.y = drag.minimumY
                            }

                        }
                        Connections {
                            target: imageViewer
                            onSigSourceChange : {
                                //图元位置归位
                                showImg.x = 0
                                showImg.y = 0
                                showAnimatedImg.x = 0
                                showAnimatedImg.y = 0
                                showSvgImg.x = 0
                                showSvgImg.y = 0
                            }
                        }

                        onPressed: {
                            infomationDig.hide()
                            //                            changeRectXY()
                            console.log("608")
                            if (mouse.button === Qt.RightButton)
                            {
                                option_menu.popup()
                            }
                        }

                        onMouseXChanged: {
                            changeRectXY()
                            if (showAnimatedImg.visible)
                            {
                                currentimgX = showAnimatedImg.x
                            } else
                            {
                                currentimgX = showImg.x
                            }
                            //以整个图片中心为平面原点，currentimgX，currentimgY为当前视口右下角相对于整个图片的坐标，以此计算导航窗口蒙皮和位置
                            //计算相对位置
                            m_NavX = (drag.maximumX - currentimgX) / (drag.maximumX - drag.minimumX)
                            m_NavY = (drag.maximumY - currentimgY) / (drag.maximumY - drag.minimumY)

                            idNavWidget.setRectLocation(m_NavX, m_NavY)
                        }

                        onMouseYChanged: {
                            changeRectXY()
                            if (showAnimatedImg.visible)
                            {
                                currentimgY = showAnimatedImg.y
                            } else
                            {
                                currentimgY = showImg.y
                            }
                            m_NavX = (drag.maximumX - currentimgX) / (drag.maximumX - drag.minimumX)
                            m_NavY = (drag.maximumY - currentimgY) / (drag.maximumY - drag.minimumY)

                            idNavWidget.setRectLocation(m_NavX, m_NavY)
                        }

                        onDoubleClicked: {
                            if (!thumbnailListView.contains(msArea.mapToItem(thumbnailListView, mouse.x, mouse.y))) {
                                infomationDig.hide()
                                showFulltimer.start()
                            }
                        }

                        onWheel: {
                            var datla = wheel.angleDelta.y / 120
                            if (global.ctrlPressed)
                                datla > 0 ? thumbnailListView.previous() : thumbnailListView.next()
                            else {
                                if (datla > 0)
                                    currentScale = currentScale / 0.9
                                else
                                    currentScale = currentScale * 0.9

                                if (currentScale * 100 < 100)
                                {
                                    idNavWidget.visible = false
                                } else if (isNavShow)
                                {
                                    if(root.height<=global.minHideHeight || root.width<=global.minWidth){
                                        idNavWidget.visible=false
                                    }else{
                                        idNavWidget.visible=true
                                    }
                                }
                                changeRectXY()

                                sigWheelChange()

                                /*
                                缩放计算规则：val对应的是showImg.width和showImg.height
                                缩小：(即showImg.scale < 1)
                                    min:初始值+val*(1-showImg.scale) / 2
                                    max:初始值-val*(1-showImg.scale) / 2
                                放大：(即showImg.scale > 1)
                                    min:初始值-val*(showImg.scale-1) / 2
                                    max:初始值+val*(showImg.scale-1) / 2
                                */
                            }
                        }
                    }
                }
            }
        }

        onCurrentItemChanged: {
            currentRotate = 0
        }


    }

    //rename窗口
    ReName {
        id: renamedialog
    }
    //info的窗口
    InfomationDialog {

        id: infomationDig

    }



    //导航窗口
    NavigationWidget {
        id : idNavWidget
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 109
        anchors.left: parent.left
        anchors.leftMargin: 15
        visible: false
    }
    onHeightChanged: {
        if(root.height<=global.minHideHeight ){
            idNavWidget.visible=false
        }
        fitWindow()
    }

    onWidthChanged: {
        if( root.width<=global.minWidth){
            idNavWidget.visible=false
        }
        fitWindow()
    }


}


