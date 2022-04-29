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
    property bool ctrlPressed: false

    //是否显示和隐藏导航栏
    property bool  isNavShow : true

    property double  currentScale : 1.0

    property double  currentimgX : 0.0

    property double  currentimgY : 0.0

    property double  currenImageScale : currentScale / CodeImage.getFitWindowScale(source,root.width, root.height) * 100

    property bool isMousePinchArea: true

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
        floatLabel.visible = CodeImage.imageIsNull(source) ? false : true
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
            currentScale = 1.0

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
                    width: parent.width
                    height: parent.height

                    clip: true
                    color: backcontrol.ColorSelector.backgroundColor

                    Image {
                        id: showImg

                        fillMode: Image.PreserveAspectFit
                        width: parent.width
                        height: parent.height
                        source:  !fileControl.isDynamicImage(sourcePaths[index]) ? "image://viewImage/"+sourcePaths[index] : ""
                        visible: !fileControl.isDynamicImage(sourcePaths[index]) && !CodeImage.imageIsNull(sourcePaths[index])
                        asynchronous: true

                        cache: false
                        clip: true
                        scale: currentScale
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

                    //动态图才会显示
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
                        drag.target: showAnimatedImg.visible ? showAnimatedImg : showImg
                        enabled : isMousePinchArea
                        function setImgPostions(x, y)
                        {
                            currentimgX = msArea.drag.maximumX - x * (msArea.drag.maximumX - msArea.drag.minimumX)
                            currentimgY = msArea.drag.maximumY - y * (msArea.drag.maximumY - msArea.drag.minimumY)
                            if (showAnimatedImg.visible) {
                                showAnimatedImg.x = currentimgX
                                showAnimatedImg.y = currentimgY
                            } else {
                                showImg.x = currentimgX
                                showImg.y = currentimgY
                            }
                        }

                        Connections {
                            target: idNavWidget
                            onChangeShowImgPostions : {
                                msArea.setImgPostions(x, y)
                            }
                        }


                        function changeRectXY()
                        {
                            if (currentScale <= 1.0) {
                                drag.minimumX = 0
                                drag.minimumY = 0
                                drag.maximumX = 0
                                drag.maximumY = 0
                                showAnimatedImg.x = 0;
                                showAnimatedImg.y = 0;
                                showImg.x = 0;
                                showImg.y = 0;
                            } else if (showAnimatedImg.visible) {
                                drag.minimumX = (showAnimatedImg.width * showAnimatedImg.scale > parent.width) ? (parent.width - showAnimatedImg.width - showAnimatedImg.width * (showAnimatedImg.scale - 1) / 2) : showAnimatedImg.width * (showAnimatedImg.scale - 1) / 2
                                drag.minimumY = (showAnimatedImg.height * showAnimatedImg.scale > parent.height) ? (parent.height - showAnimatedImg.height - showAnimatedImg.height * (showAnimatedImg.scale - 1) / 2) : showAnimatedImg.height * (showAnimatedImg.scale - 1) / 2
                                drag.maximumX = (showAnimatedImg.width * showAnimatedImg.scale > parent.width) ? showAnimatedImg.width * (showAnimatedImg.scale - 1) / 2 : (parent.width - showAnimatedImg.width * showAnimatedImg.scale) + showAnimatedImg.width * (showAnimatedImg.scale - 1) / 2
                                drag.maximumY = (showAnimatedImg.height * showAnimatedImg.scale > parent.height) ? showAnimatedImg.height * (showAnimatedImg.scale - 1) / 2 : (parent.height - showAnimatedImg.height * showAnimatedImg.scale) + showAnimatedImg.height * (showAnimatedImg.scale - 1) / 2
                            } else {
                                drag.minimumX = (showImg.width * showImg.scale > parent.width) ? (parent.width - showImg.width - showImg.width * (showImg.scale - 1) / 2) : showImg.width * (showImg.scale - 1) / 2
                                drag.minimumY = (showImg.height * showImg.scale > parent.height) ? (parent.height - showImg.height - showImg.height * (showImg.scale - 1) / 2) : showImg.height * (showImg.scale - 1) / 2
                                drag.maximumX = (showImg.width * showImg.scale > parent.width) ? showImg.width * (showImg.scale - 1) / 2 : (parent.width - showImg.width * showImg.scale) + showImg.width * (showImg.scale - 1) / 2
                                drag.maximumY = (showImg.height * showImg.scale > parent.height) ? showImg.height * (showImg.scale - 1) / 2 : (parent.height - showImg.height * showImg.scale) + showImg.height * (showImg.scale - 1) / 2
                            }
                            if (showAnimatedImg.x >= drag.maximumX) {
                                showAnimatedImg.x = drag.maximumX
                            }
                            if (showAnimatedImg.y >= drag.maximumY) {
                                showAnimatedImg.y = drag.maximumY
                            }
                            if (showImg.x >= drag.maximumX) {
                                showImg.x = drag.maximumX
                            }
                            if (showImg.y >= drag.maximumY) {
                                showImg.y = drag.maximumY
                            }
                        }
                        //                        onClicked: {
                        //                            console.log("right menu");
                        //                            if (mouse.button === Qt.RightButton) {
                        //                                option_menu.popup(parent)
                        //                            }
                        //                        }
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
                            infomationDig.hide()
                             showFulltimer.start()
                        }

                        onWheel: {
                            var datla = wheel.angleDelta.y / 120
                            if (ctrlPressed)
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
                                    idNavWidget.visible = true
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

        onWidthChanged: {
            //            if (view.width > 0) {
            //                if (view.width > root.width
            //                        || view.height > root.height) {
            //                    fitWindow()
            //                } else {
            //                    fitImage()
            //                }
            //            }

            fitWindow()

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


}


