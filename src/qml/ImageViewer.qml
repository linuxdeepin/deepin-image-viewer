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

    property int currentSourceWidth :0;

    property int currentSourceHeight :0;

    property int index: 0
    property alias swipeIndex: view.currentIndex

    //是否显示和隐藏导航栏
    property bool  isNavShow : true

    property double  currentScale : 1.0

    property double  currentimgX : 0.0

    property double  currentimgY : 0.0

    property double  currenImageScale : currentScale / fileControl.getFitWindowScale(root.width,root.height) * 100

    property bool isMousePinchArea:true

    signal sigWheelChange
    signal sigImageShowFullScreen
    signal sigImageShowNormal
    signal sigSourceChange
    color: "#F8F8F8"

    onCurrenImageScaleChanged: {
        floatLabel.displayStr = currenImageScale.toFixed(0) + "%"
        floatLabel.visible = true
    }

    onCurrentScaleChanged: {
        idNavWidget.setRectPec(currentScale)
    }

    onSourceChanged: {
        fileControl.slotRotatePixCurrent();

        console.log("source:",mainView.source)
        fileControl.setCurrentImage(source)

        idNavWidget.visible=false
//        if(fileControl.getFitWindowScale(root.width,root.height-100)<1.0){
//            fitImage()
//        }else{
//            fitWindow()
//        }
        fitWindow()


        root.title=fileControl.slotGetFileName(source)+fileControl.slotFileSuffix(source)

        floatLabel.displayStr = (currentScale / fileControl.getFitWindowScale(root.width,root.height) * 100).toFixed(0) + "%"
        floatLabel.visible = true

        sigSourceChange();

        mainView.animationAll()

    }

    function fitImage() {
        currentScale=fileControl.getFitWindowScale(root.width,root.height);
    }

    function fitWindow() {
        currentScale =root.visibility==Window.FullScreen ? 1.0 : 1.0 *(root.height-titleRect.height*2)/root.height
    }
    function rotateImage(x) {
        //        rotation = currentRotate + x
        view.currentItem.rotation=view.currentItem.rotation+x
        thumbnailListView.rotateImage(x)
        fileControl.rotateFile(source,x)
    }
    function deleteItem (item, list) {
        // 先遍历list里面的每一个元素，对比item与每个元素的id是否相等，再利用splice的方法删除
        for (var key in fileList) {
            if (list[key].id === item) {
                list.splice(key, 1)
            }
        }
    }
    function startSliderShow(){
        if(sourcePaths.length>0){
            showFullScreen()
            sliderMainShow.images=sourcePaths
            sliderMainShow.modelCount=sourcePaths.length
            sliderMainShow.autoRun = true
            sliderMainShow.indexImg = view.currentIndex
            stackView.currentWidgetIndex=2
        }

    }
    function showPanelFullScreen(){
        showFullScreen()
        stackView.currentWidgetIndex=1
        currentScale=1.0
        sigImageShowFullScreen()
    }

    function escBack() {
        showNormal()
        sliderMainShow.autoRun = false
        sliderMainShow.backtrack()
        if(stackView.currentWidgetIndex===2) {
            mainView.currentIndex=sliderMainShow.indexImg
        }

        stackView.currentWidgetIndex=1
        currentScale=1.0

        sigImageShowNormal()
    }
    //缩放快捷键
    Shortcut {
        sequence: "Ctrl+="
        onActivated:{
           currentScale =  currentScale /0.9
        }
    }

    Shortcut {
        sequence: "Ctrl+-"
        onActivated:{
           currentScale =  currentScale *0.9
        }
    }

    Shortcut {
        sequence: "Up"
        onActivated:{
           currentScale =  currentScale /0.9
        }
    }


    Shortcut {
        sequence: "Down"
        onActivated:{
           currentScale =  currentScale *0.9
        }
    }

    MouseArea {
        anchors.fill: parent

        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: {
            console.log("right menu");
            if (mouse.button === Qt.RightButton) {
                option_menu.popup()
            }
        }
//        onWheel: {
//            var datla = wheel.angleDelta.y / 120
//            if (datla > 0) {
//                view.currentItem.scale = view.currentItem.scale/0.9
//            } else {
//                view.currentItem.scale = view.currentItem.scale * 0.9
//            }


//            if (view.currentItem.scale * 100 < 100) {
//                idNavWidget.visible = false
//            } else {
//                idNavWidget.visible = true
//                //TODO 依据配置文件设置再来控制显影，这里先默认根据图片比例控制
//            }
//            idNavWidget.setRectPec(view.currentItem.scale)
//            //            CodeImage.m_path = source
//            //            CodeImage.loadThumbnail(source)
//        }
    }
    Menu {
        x: 250; y: 600
        id: option_menu
        //        model: ObjectModelProxy {
        //            id: proxyModel
        //            property string filterText
        //            filterAcceptsItem: function(item) {
        //                return item.text.includes(filterText)
        //            }
        //            sourceModel: option_menu.contentModel
        //        }
        maxVisibleItems: 20
        MenuItem {
            id : right_fullscreen
            text:root.visibility!=Window.FullScreen ? qsTr("Fullscreen") : qsTr("Exit fullscreen")
            //            shortcut: "F11"
            onTriggered: root.visibility!=Window.FullScreen ?imageViewer.showPanelFullScreen() :showNormal()
            Shortcut {
                sequence: root.visibility!=Window.FullScreen ?"F11":"Esc"
                onActivated: root.visibility!=Window.FullScreen ?imageViewer.showPanelFullScreen() : imageViewer.escBack()
            }
        }


                MenuItem {
                    text: qsTr("Print")
                    onTriggered: {
                    fileControl.showPrintDialog(mainView.source)
                    }
                    Shortcut {
                        sequence: "Ctrl+P"
                        onActivated:  fileControl.showPrintDialog(mainView.source)
                    }
                }

        MenuItem {
            text: qsTr("Extract text")
            onTriggered: {
                fileControl.ocrImage(source)
            }
            Shortcut {
                sequence: "Alt+O"
                onActivated:  fileControl.ocrImage(source)
            }
        }

        MenuItem {
            text:qsTr( "Slide show")
            onTriggered: {startSliderShow()}
            Shortcut {
                sequence: "F5"
                onActivated:{startSliderShow()
                }
            }
        }


        MenuSeparator { }
        MenuItem {
            text:qsTr( "Copy")
            onTriggered: {
                fileControl.copyImage(source)
            }
            Shortcut {
                sequence: "Ctrl+C"
                onActivated:{
                    fileControl.copyImage(source)
                }
            }
        }

        MenuItem {
            text: qsTr("Rename")
            onTriggered: {
                var x = parent.mapToGlobal(0,0).x + parent.width/2 - 190
                var y = parent.mapToGlobal(0,0).y + parent.height/2 - 89
                renamedialog.setX(x)
                renamedialog.setY(y)
                renamedialog.getFileName(fileControl.slotGetFileName(source))
                renamedialog.getFileSuffix(fileControl.slotFileSuffix(source))
                renamedialog.show()
            }
            Shortcut {
                sequence: "F2"
                onActivated:{
                    var x = parent.mapToGlobal(0,0).x + parent.width/2 - 190
                    var y = parent.mapToGlobal(0,0).y + parent.height/2 - 89
                    renamedialog.setX(x)
                    renamedialog.setY(y)
                    renamedialog.getFileName(fileControl.slotGetFileName(source))
                    renamedialog.getFileSuffix(fileControl.slotFileSuffix(source))
                    renamedialog.show()
                }
            }
        }

        MenuItem {
            text: qsTr("Delete")
            onTriggered: {
                thumbnailListView.deleteCurrentImage()
            }
            Shortcut {
                sequence: "Delete"
                onActivated:{
                    thumbnailListView.deleteCurrentImage()
                }
            }
        }

        MenuSeparator { }

        MenuItem {
            text: qsTr("Rotate clockwise")
            onTriggered: {
                imageViewer.rotateImage(90)
            }
            Shortcut {
                sequence: "Ctrl+R"
                onActivated:{
                    imageViewer.rotateImage(90)
                }
            }
        }

        MenuItem {
            text: qsTr("Rotate counterclockwise")
            onTriggered: {
                imageViewer.rotateImage(-90)
            }
            Shortcut {
                sequence: "Ctrl+Shift+R"
                onActivated:{
                    imageViewer.rotateImage(-90)
                }
            }
        }

        MenuItem {
            id :showNavigation

            text: !isNavShow ?qsTr("Show navigation window"): qsTr("Hide navigation window")
            onTriggered: {
                if(isNavShow){
                    isNavShow=false
                    idNavWidget.visible = false
                }else{
                    isNavShow=true
                    idNavWidget.visible = true
                }
                idNavWidget.setRectPec(view.currentItem.scale)
            }
        }

        MenuItem {
            text: qsTr("Set as wallpaper")
            onTriggered: {
                fileControl.setWallpaper(source)
            }
            Shortcut {
                sequence: "Ctrl+F"
                onActivated:{
                    fileControl.setWallpaper(source)
                }
            }
        }

        MenuItem {
            text: qsTr("Display in file manager")
            onTriggered: {
                fileControl.displayinFileManager(source)
            }
            Shortcut {
                sequence: "Alt+D"
                onActivated:{
                    fileControl.displayinFileManager(source)
                }
            }
        }

        MenuItem {
            text:qsTr( "Image info")
            onTriggered: {
                infomationDig.show()
            }
            Shortcut {
                sequence: "Ctrl+I"
                onActivated:{
                    infomationDig.show()
                }
            }
        }
    }

    SwipeView {
        id: view
        currentIndex: sourcePaths.indexOf(source)
        width: parent.width
        height: parent.height
        //        anchors.centerIn: parent

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
                    color: "#F8F8F8"

                    Image {
                        id: showImg

                        fillMode:Image.PreserveAspectFit
                        width: parent.width
                        height: parent.height
                        source: sourcePaths[index]
                        asynchronous: true
                        cache: true
                        clip: true
                        scale: currentScale
                        smooth: true

                        onSourceChanged: {
                            //                            view.currentItem.rotation=0
                        }
                        onCacheChanged: {
                            view.currentItem.rotation=0
                        }

                        onStatusChanged: {
                            msArea.changeRectXY()
                            if(fileControl.isDynamicImage(sourcePaths[index])){
                                visible=false
                            }else{
                                visible=true
                            }

                            if (showImg.status === Image.Ready)
                                console.log('Ready')
                            if(showImg.status === Image.Loading)
                                console.log('Loading')
                            if(showImg.status === Image.Null)
                                console.log('Null')
                            if(showImg.status === Image.Error){
                                source="qrc:/res/picture damaged_light.svg"
                            }
                        }
                    }

                    //动态图才会显示
                    AnimatedImage {
                        id: showAnimatedImg

                        fillMode:Image.PreserveAspectFit
                        width: parent.width
                        height: parent.height
                        //                        source: sourcePaths[index]
                        asynchronous: true
                        cache: false
                        clip: true
                        scale: currentScale
                        smooth: true

                        onStatusChanged: {
                            msArea.changeRectXY()
                            if(!fileControl.isDynamicImage(sourcePaths[index])){
                                visible=false
                            }else{
                                visible=true;
                                source=sourcePaths[index]
                            }

                            if (showImg.status === Image.Ready)
                                console.log('Ready')
                            if(showImg.status === Image.Loading)
                                console.log('Loading')
                            if(showImg.status === Image.Null)
                                console.log('Null')
                            if(showImg.status === Image.Error){
                                source="qrc:/res/picture damaged_light.svg"
                            }
                        }
                    }

                    PinchArea {
                        enabled: isMousePinchArea
                        anchors.fill: showAnimatedImg.visible ? showAnimatedImg: showImg

                        onPinchStarted: {
                            pinch.accepted = true
                        }

                        onPinchUpdated: {
                            if(pinch.scale < 5 && pinch.scale > 0.2){
                                currentScale = pinch.scale;
                            }
                        }

                        onPinchFinished: {
                            if(pinch.scale < 5 && pinch.scale > 0.2){
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
                        drag.target:showAnimatedImg.visible ? showAnimatedImg: showImg
                        enabled: isMousePinchArea
                        function setImgPostions(x, y) {
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
                            onChangeShowImgPostions :{
                                msArea.setImgPostions(x,y)
                            }
                        }


                        function changeRectXY() {
                            if(currentScale<=1.0){
                                drag.minimumX=0
                                drag.minimumY=0
                                drag.maximumX=0
                                drag.maximumY=0
                                showAnimatedImg.x=0;
                                showAnimatedImg.y=0;
                                showImg.x=0;
                                showImg.y=0;
                            }else if (showAnimatedImg.visible){
                                drag.minimumX = (showAnimatedImg.width * showAnimatedImg.scale > parent.width) ? (parent.width - showAnimatedImg.width - showAnimatedImg.width * (showAnimatedImg.scale - 1) / 2) : showAnimatedImg.width * (showAnimatedImg.scale - 1) / 2
                                drag.minimumY = (showAnimatedImg.height * showAnimatedImg.scale > parent.height) ? (parent.height - showAnimatedImg.height - showAnimatedImg.height * (showAnimatedImg.scale - 1) / 2) : showAnimatedImg.height * (showAnimatedImg.scale - 1) / 2
                                drag.maximumX = (showAnimatedImg.width * showAnimatedImg.scale > parent.width) ? showAnimatedImg.width * (showAnimatedImg.scale - 1) / 2 : (parent.width - showAnimatedImg.width * showAnimatedImg.scale) + showAnimatedImg.width * (showAnimatedImg.scale - 1) / 2
                                drag.maximumY = (showAnimatedImg.height * showAnimatedImg.scale > parent.height) ? showAnimatedImg.height * (showAnimatedImg.scale - 1) / 2 : (parent.height - showAnimatedImg.height * showAnimatedImg.scale) + showAnimatedImg.height * (showAnimatedImg.scale - 1) / 2
                            }else{
                                drag.minimumX = (showImg.width * showImg.scale > parent.width) ? (parent.width - showImg.width - showImg.width * (showImg.scale - 1) / 2) : showImg.width * (showImg.scale - 1) / 2
                                drag.minimumY = (showImg.height * showImg.scale > parent.height) ? (parent.height - showImg.height - showImg.height * (showImg.scale - 1) / 2) : showImg.height * (showImg.scale - 1) / 2
                                drag.maximumX = (showImg.width * showImg.scale > parent.width) ? showImg.width * (showImg.scale - 1) / 2 : (parent.width - showImg.width * showImg.scale) + showImg.width * (showImg.scale - 1) / 2
                                drag.maximumY = (showImg.height * showImg.scale > parent.height) ? showImg.height * (showImg.scale - 1) / 2 : (parent.height - showImg.height * showImg.scale) + showImg.height * (showImg.scale - 1) / 2
                            }
                            if (showAnimatedImg.x >= drag.maximumX ) {
                                showAnimatedImg.x=drag.maximumX
                            }
                            if (showAnimatedImg.y >=drag.maximumY  ) {
                                showAnimatedImg.y=drag.maximumY
                            }
                            if ( showImg.x>=drag.maximumX ) {
                                showImg.x=drag.maximumX
                            }
                            if ( showImg.y >=drag.maximumY ) {
                                showImg.y=drag.maximumY
                            }
                        }

                        onPressed: {
                            infomationDig.hide()
//                            changeRectXY()
                        }

                        onMouseXChanged: {
                            changeRectXY()
                            if (showAnimatedImg.visible) {
                                currentimgX = showAnimatedImg.x
                            } else {
                                currentimgX = showImg.x
                            }
                            //以整个图片中心为平面原点，currentimgX，currentimgY为当前视口右下角相对于整个图片的坐标，以此计算导航窗口蒙皮和位置
                            //计算相对位置
                            var x1 = (drag.maximumX - currentimgX) / (drag.maximumX - drag.minimumX)
                            var y1 = (drag.maximumY - currentimgY) / (drag.maximumY - drag.minimumY)

                            idNavWidget.setRectLocation(x1, y1)
                        }

                        onMouseYChanged: {
                            changeRectXY()
                            if (showAnimatedImg.visible) {
                                currentimgY = showAnimatedImg.y
                            } else {
                                currentimgY = showImg.y
                            }
                            var x1 = (drag.maximumX - currentimgX) / (drag.maximumX - drag.minimumX)
                            var y1 = (drag.maximumY - currentimgY) / (drag.maximumY - drag.minimumY)

                            idNavWidget.setRectLocation(x1, y1)
                        }

                        onDoubleClicked: {
                            infomationDig.hide()
                            root.visibility!=Window.FullScreen ?showFullScreen() : imageViewer.escBack()
                        }

                        onWheel: {

                            var datla = wheel.angleDelta.y / 120
                            if (datla > 0)
                                currentScale=currentScale/0.9
                            else
                                currentScale=currentScale * 0.9
                            if (currentScale * 100 < 100) {
                                idNavWidget.visible = false
                            } else if(isNavShow) {
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

        onWidthChanged: {
            if (view.width > 0) {
                if (view.width > root.width
                        || view.height > root.height) {
                    fitWindow()
                } else {
                    fitImage()
                }
            }
        }
        onCurrentItemChanged: {
            currentRotate=0
//            source=sourcePaths[view.currentIndex]
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
        anchors.bottomMargin:109
        anchors.left: parent.left
        anchors.leftMargin:15
        visible: false
    }


}
