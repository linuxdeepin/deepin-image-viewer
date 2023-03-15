// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import QtQuick.Shapes 1.11
import org.deepin.dtk 1.0
import org.deepin.image.viewer 1.0 as IV

import "./ImageDelegate"
import "./LiveText"

Rectangle {
    // Image 类型的对象，空图片、错误图片、消失图片等异常为 undefined
    property alias targetImage: view.currentImage


    // Indicates the minimum number of zooms
    property int minScaleLevel: 10
    // Indicates the maximum number of zooms
    property int maxScaleLevel: 30
    // current rotate
    property int currentRotate: 0

    // Indicates the current image path
    property var source
    /*: showImg.source*/
    property var sourcePaths

    // 当前源图片宽度
    property int currentSourceWidth: 0
    // 当前源图片高度
    property int currentSourceHeight: 0

    property int index: 0
    property alias swipeIndex: view.currentIndex

    // 界面主动切换时使用，控制滑动视图是否响应界面的拖拽操作
    property bool viewInteractive: true

    property int frameCount: currentIsMultiImage ? fileControl.getImageCount(
                                                       source) : 1
    property int frameIndex: 0
    Connections {
        target: GControl
        onCurrentFrameIndexChanged: {
            frameIndex = GControl.currentFrameIndex
        }
    }

    //是否显示和隐藏导航栏，从配置文件中读取初始配置
    property bool isNavShow: fileControl.isEnableNavigation()

    property double currentScale: 1.0

    property double currentimgX: 0.0

    property double currentimgY: 0.0

//    property double currenImageScale: currentScale / CodeImage.getFitWindowScale(
//                                          source, root.width, root.height) * 100

    property bool isMousePinchArea: true

    property double readWidthHeightRatio: CodeImage.getrealWidthHeightRatio(
                                              imageViewer.source)

    //导航蒙皮位置
    property double m_NavX: 0.0
    property double m_NavY: 0.0

    //用于记录normal状态的宽高
    property int normalWidth: 0
    property int normalHeight: 0

    // 记录放大图片(在qml中像素)和显示窗口像素的比值，用于蒙皮获取准确区域
    property real viewImageWidthRatio: 0
    property real viewImageHeightRatio: 0

    // 标识当前是否处于全屏缩放状态，缩放前后部分控件需重置，例如缩略图栏重新居中设置
    property bool isFullNormalSwitchState: showFulltimer.running
                                           || showfullAnimation.running

    // 记录窗口大小，用于在窗口缩放时，根据前后窗口变化保持图片缩放比例
    property int lastWindowWidth: 0
    property int lastWindowHeight: 0
    // 图片保持适配窗口时的缩放比值，和 currentScale 对应
    property double keepFitWindowScale: 0

    // 图片正在滑动中
//    property bool inFlick: false

    signal sigWheelChange
    signal sigSourceChange

    color: backcontrol.ColorSelector.backgroundColor

    Connections {
        target: root

        onSigTitlePress: {
            infomationDig.hide()
        }

        onWidthChanged: {
            imageViewer.keepImageScale()
        }

        onHeightChanged: {
            imageViewer.keepImageScale()
        }
    }

    // 窗口拖拽大小变更时保持图片的缩放比例
    function keepImageScale() {
        // 当前缩放比例与匹配窗口的图片缩放比例比较，若一致，则保持匹配窗口
        if (Math.abs(imageViewer.keepFitWindowScale - currentScale) > Number.EPSILON) {
            var lastWidth = imageViewer.lastWindowWidth
            var lastHeight = imageViewer.lastWindowHeight

            // 跳过重复判断
            if (lastWidth !== root.width || lastHeight !== root.height) {
                // 首次不执行
                if (lastWidth !== 0 && lastHeight !== 0) {
                    // 获取之前的图片缩放比例
                    var oldImageScale = currentScale / CodeImage.getFitWindowScale(
                                source, lastWidth, lastHeight) * 100
                    // 根据缩放比例反推当前的缩放比例，窗口缩放时界面不同步缩放
                    currentScale = oldImageScale * CodeImage.getFitWindowScale(
                                source, root.width, root.height) / 100
                }

                // 更新记录的窗口大小，用于下次恢复
                imageViewer.lastWindowWidth = root.width
                imageViewer.lastWindowHeight = root.height
            }
        } else {
            fitWindow()
        }
    }

    function showFloatLabel() {
        // 不存在的图片不弹出缩放提示框
//        if (!currentIsExistImage) {
//            return
//        }

        if (currenImageScale.toFixed(0) > 2000 && currenImageScale.toFixed(
                    0) <= 3000) {
            floatLabel.displayStr = "2000%"
        } else if (currenImageScale.toFixed(0) < 2 && currenImageScale.toFixed(
                       0) >= 0) {
            floatLabel.displayStr = "2%"
        } else if (currenImageScale.toFixed(0) >= 2 && currenImageScale.toFixed(
                       0) <= 2000) {
            floatLabel.displayStr = currenImageScale.toFixed(0) + "%"
        }
        floatLabel.visible = CodeImage.imageIsNull(source)
                || currenImageScale.toFixed(0) < 0 || currenImageScale.toFixed(
                    0) > 2000 ? false : true
    }

    function recalculateLiveText() {
        exitLiveText()
        startLiveText()
    }

    function startLiveText() {
        console.debug("function startLiveText()")
        view.startLiveTextAnalyze()
    }

    function exitLiveText() {
        view.exitLiveText()
    }

    function flushNav() {
        if (!isNavShow || currentScale <= 1.0) {
            idNavWidget.visible = false
            return
        }

        if (root.height <= global.minHideHeight
                || root.width <= global.minWidth) {
            idNavWidget.visible = false
        } else {
            console.debug(currentScale)
            idNavWidget.visible = true
        }

        var realWidth = 0
        var realHeight = 0
        if (root.width > root.height * readWidthHeightRatio) {
            realWidth = root.height * readWidthHeightRatio
        } else {
            realWidth = root.width
        }
        if (root.height > root.width / readWidthHeightRatio) {
            realHeight = root.width / readWidthHeightRatio
        } else {
            realHeight = root.height
        }

        viewImageWidthRatio = root.width / (realWidth * currentScale)
        viewImageHeightRatio = root.height / (realHeight * currentScale)

        idNavWidget.setRectPec(currentScale, viewImageWidthRatio,
                               viewImageHeightRatio)
        idNavWidget.setRectLocation(m_NavX, m_NavY)
    }

//    onCurrenImageScaleChanged: {
//        showFloatLabel()
//    }

    onCurrentScaleChanged: {
        // 单独计算图片缩放比，防止属性绑定循环计算，数据异常
        var calcImageScale = currentScale / CodeImage.getFitWindowScale(
                    source, root.width, root.height) * 100
        if (calcImageScale > 2000) {
            currentScale = 20 * CodeImage.getFitWindowScale(source, root.width,
                                                            root.height)
        } else if (calcImageScale < 2 && calcImageScale > 0) {
            currentScale = 0.02 * CodeImage.getFitWindowScale(source, root.width, root.height)
        }

        //刷新导航窗口
        flushNav()

        //重新计算live text
        if (!GStatus.viewFlicking) {
            console.debug("onCurrentScaleChanged")
            recalculateLiveText()
        }
    }

    // 多页图当前图片帧号发生变更，更新当前界面维护的数据信息
    onFrameIndexChanged: {
        // 当前为多页图
        if (currentIsMultiImage) {
            // 设置 fileControl 维护的多页图信息
            fileControl.setCurrentFrameIndex(frameIndex)
            CodeImage.setMultiFrameIndex(frameIndex)
            if (!GStatus.viewFlicking) {
                console.debug("onFrameIndexChanged")
                recalculateLiveText()
            }
        }
    }

    // 图片源发生改变，隐藏导航区域，重置图片缩放比例
    onSourceChanged: {
        // 手动更新图源时，排除空图源影响
        if (source.length === 0) {
            return
        }

        // 多页图索引不在此处进行复位，鼠标点击，按钮切换等不同方式切换显示不同的多页图帧号
        fileControl.slotRotatePixCurrent()
        CodeImage.setReverseHeightWidth(false)

        // 设置图片状态
        fileControl.setCurrentImage(source)
        CodeImage.setMultiFrameIndex(fileControl.isMultiImage(source) ? 0 : -1)
        // 复位图片旋转状态
        imageViewer.currentRotate = 0
        // 复位匹配窗口的缩放比例
        imageViewer.keepFitWindowScale = 0

        // 默认隐藏导航区域
        idNavWidget.visible = false
        // 判断图片大小是否超过了允许显示的展示区域
        if (fileControl.getFitWindowScale(
                    root.width, root.height - titleRect.height * 2) > 1) {
            fitWindow()
        } else {
            fitImage()
        }

        // 设置标题栏
        window.title = fileControl.slotGetFileName(source) + fileControl.slotFileSuffix(source)
        // 显示缩放比例提示框
        showFloatLabel()

        sigSourceChange()

        // 重设工具/菜单栏的隐藏/弹出
        mainView.animationAll()

        // 启动live text分析
        if (!GStatus.viewFlicking) {
            console.debug("onSourceChanged:")
            recalculateLiveText()
        }
    }

    // 部分图片存在加载图片过程，重设图片大小调整到图片加载完成后处理 Image.Ready --> onImageReady()
    function onImageReady() {
        // 复位图片旋转角度
        imageViewer.currentRotate = 0

        // 取得图片的真实大小，部分格式不支持直接获取图片数据，若数据异常，需要从加载缓存中读取
        currentSourceWidth = fileControl.getCurrentImageWidth()
        currentSourceHeight = fileControl.getCurrentImageHeight()
        if ((currentSourceWidth <= 0) || (currentSourceHeight <= 0)) {
            currentSourceWidth = CodeImage.getImageWidth(source)
            currentSourceHeight = CodeImage.getImageHeight(source)
        }

        // 判断图片大小是否超过了允许显示的展示区域
        if (currentSourceHeight > root.height - titleRect.height * 2
                || currentSourceWidth > root.width) {
            fitWindow()
        } else {
            fitImage()
        }
    }

    function fitImage() {
        // 优先采用图片实际加载的数据，若图片未加载完成，采用文件基本信息
        if (CodeImage.getImageWidth(source) <= 0 || CodeImage.getImageHeight(
                    source) <= 0) {
            currentScale = fileControl.getFitWindowScale(root.width,
                                                         root.height)
        } else {
            // 图片数据异常需要从加载完成图片信息中获取
            currentScale = CodeImage.getFitWindowScale(source, root.width,
                                                       root.height)
        }
    }

    function fitWindow() {
        // 调整位置，图片恢复显示到中心
        sigSourceChange()

        // 根据图片大小进行调整，使得对较长图片能顶满看图左右两侧边框
        if (Window.FullScreen == root.visibility) {
            currentScale = 1.0
        } else {
            // 将图片调整在 root.width x enableRootHeight 的区域显示
            var enableRootHeight = (root.height - titleRect.height * 2)
            var imageRatio = fileControl.getCurrentImageHeight(
                        ) / fileControl.getCurrentImageWidth()
            var rootRatio = enableRootHeight / root.width

            // 取得当前图片相对显示宽度
            var curViewImageHeight = root.width * imageRatio
            // 判断高度是否无需调整(即图片高度小于展示区域高度，则无需继续压缩显示区域)
            var useHeight = (curViewImageHeight / rootRatio) <= root.width

            currentScale = useHeight ? 1.0 : (enableRootHeight / root.height)

            // 记录图片适配窗口时的缩放比例
            imageViewer.keepFitWindowScale = currentScale
        }
    }

    function rotateImage(x) {
        // 判断是否为首次进行图片旋转
        var needResetBar = (currentRotate == 0)

        // 更新当前图片的旋转角度
        fileControl.rotateFile(source, x)
        currentRotate = fileControl.currentAngle()
        CodeImage.setReverseHeightWidth(fileControl.isReverseHeightWidth())

        // 判断图片大小是否超过了允许显示的展示区域
        if (fileControl.getFitWindowScale(
                    root.width, root.height - titleRect.height * 2) > 1) {
            fitWindow()
        } else {
            fitImage()
        }

        if (needResetBar) {
            // 重设工具/菜单栏的隐藏/弹出
            mainView.animationAll()
        }

        //重新进行live text分析
        console.debug("rotateImage")
        recalculateLiveText()
    }

    function showPanelFullScreen() {
        normalWidth = root.width
        normalHeight = root.height
        view.exitLiveText()

        showFullScreen()
        view.contentItem.forceActiveFocus()
        showfullAnimation.start()

        //如果是初始界面只全屏
        if (stackView.currentWidgetIndex != 0) {
            stackView.currentWidgetIndex = 1
            currentScale = 1.0
        }
    }

    function escBack() {
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
            currentScale = 1.0 * (normalHeight - titleRect.height * 2) / normalHeight
        }
    }

    PropertyAnimation {
        id: showfullAnimation

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
            !window.isFullScreen ? showPanelFullScreen() : escBack()
        }
    }

    //缩放快捷键
    Shortcut {
        sequence: "Ctrl+="
        onActivated: {
            view.currentDelegate.scale = view.currentDelegate.scale / 0.9
        }
    }

    Shortcut {
        sequence: "Ctrl+-"
        onActivated: {
            view.currentDelegate.scale = view.currentDelegate.scale * 0.9
        }
    }

    Shortcut {
        sequence: "Up"
        onActivated: {
            view.currentDelegate.scale = view.currentDelegate.scale / 0.9
        }
    }

    Shortcut {
        sequence: "Down"
        onActivated: {
            view.currentDelegate.scale = view.currentDelegate.scale * 0.9
        }
    }

    Shortcut {
        sequence: "Ctrl+Shift+/"
        onActivated: {
            var screenPos = mapToGlobal(parent.x, parent.y)
            fileControl.showShortcutPanel(screenPos.x + root.width / 2,
                                          screenPos.y + root.height / 2)
        }
    }

    // 图片滑动视图
    ListView {
        id: view

        // 当前展示的 Image 图片对象，空图片、错误图片、消失图片等异常为 undefined
        // 此图片信息用于外部交互缩放、导航窗口等
        property Image currentImage: view.currentItem.item.targetImage
        property BaseImageDelegate currentDelegate: view.currentItem.item

        anchors.fill: parent
        cacheBuffer: 200
        interactive: !imageViewer.isFullNormalSwitchState
                     && imageViewer.viewInteractive
        preferredHighlightBegin: 0
        preferredHighlightEnd: 0
        highlightRangeMode: ListView.StrictlyEnforceRange
        highlightMoveDuration: 0
        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem
        flickDeceleration: 500
        boundsMovement: Flickable.FollowBoundsBehavior
        boundsBehavior: Flickable.StopAtBounds

        currentIndex: GControl.currentIndex
        model: GControl.globalModel
        delegate: Loader {
            id: swipeViewItemLoader

            property url source: model.imageUrl
            property alias frameCount: imageInfo.frameCount

            active: {
                if (ListView.isCurrentItem) {
                    return true
                }
                if (view.currentIndex - 1 === index
                        || view.currentIndex + 1 === index) {
                    return true
                }
                return false
            }
            visible: active
            asynchronous: true
            width: view.width
            height: view.height

            onActiveChanged: {
                if (active && imageInfo.delegateSource) {
                    setSource(imageInfo.delegateSource, { "source": swipeViewItemLoader.source, "type": imageInfo.type })
                }
            }

            IV.ImageInfo {
                id: imageInfo

                property url delegateSource
                property bool isCurrentItem: swipeViewItemLoader.ListView.isCurrentItem

                function checkDelegateSource() {
                    if (IV.ImageInfo.Ready !== status
                            && IV.ImageInfo.Error !== status) {
                        return
                    }

                    if (!imageInfo.exists) {
                        delegateSource = "qrc:/qml/ImageDelegate/NonexistImageDelegate.qml"
                        return
                    }

                    switch (type) {
                    case IV.Types.NormalImage:
                        delegateSource = "qrc:/qml/ImageDelegate/NormalImageDelegate.qml"
                        return
                    case IV.Types.DynamicImage:
                        delegateSource = "qrc:/qml/ImageDelegate/DynamicImageDelegate.qml"
                        return
                    case IV.Types.SvgImage:
                        delegateSource = "qrc:/qml/ImageDelegate/SvgImageDelegate.qml"
                        return
                    case IV.Types.MultiImage:
                        delegateSource = "qrc:/qml/ImageDelegate/MultiImageDelegate.qml"
                        return
                    default:
                        // Default is damaged image.
                        delegateSource = "qrc:/qml/ImageDelegate/DamagedImageDelegate.qml"
                        return
                    }
                }

                source: swipeViewItemLoader.source

                onDelegateSourceChanged: {
                    if (swipeViewItemLoader.active && delegateSource) {
                        setSource(delegateSource, { "source": swipeViewItemLoader.source, "type": imageInfo.type })
                    }
                }
                onStatusChanged: checkDelegateSource()
                onIsCurrentItemChanged: checkDelegateSource()
            }
        }

        onCurrentIndexChanged: {
            // 当通过界面拖拽导致索引变更，需要调整多页图索引范围
            if (view.currentIndex < GControl.currentIndex) {
                GControl.previousImage()
            } else if (view.currentIndex > GControl.currentIndex) {
                GControl.nextImage()
            }
        }

        onMovementStarted: {
            GStatus.viewFlicking = true
        }

        onMovementEnded: {
            GStatus.viewFlicking = false
        }

        Component.onCompleted: {
            liveTextAnalyzer.analyzeFinished.connect(runLiveText)
        }

        BusyIndicator {
            anchors.centerIn: parent
            width: 48
            height: 48
            running: visible
            visible: {
                if (view.currentItem.status === Loader.Loading) {
                    return true
                } else {
                    return view.currentItem.item.status === Image.Loading
                }
            }
        }

        // 实况文本背景遮罩
        Rectangle {
            id: liveTextBackground

            anchors.centerIn: parent
            color: "#000000" // live text高亮阴影
            opacity: 0.5
            visible: highlightTextButton.checked
                     && highlightTextButton.visible

            onVisibleChanged:  {
                var target = view.currentImage
                if (undefined !== target) {
                    width = target.paintedWidth * target.scale
                    height = target.paintedHeight * target.scale
                }
            }
        }

        //live text分析函数
        //缩放和切换需要重新执行此函数
        function liveTextAnalyze() {
            console.debug("Live Text analyze start")
            view.currentItem.item.grabToImage(function (result) {
                //截取当前控件显示
                liveTextAnalyzer.setImage(result.image) //设置分析图片
                liveTextAnalyzer.analyze(currentIndex) //执行分析（异步执行，函数会立即返回）
                //result.saveToFile("/home/wzyforuos/Desktop/viewer.png") //保存截取的图片，debug用
            })
        }

        //live text执行函数
        function runLiveText(resultCanUse, token) {
            console.debug("function runLiveText", token, currentIndex)
            if (resultCanUse && token == currentIndex) {
                //这里无视警告，就是需要js的==来进行自动类型转换
                console.debug("run live start")
                ltw.drawRect(liveTextAnalyzer.liveBlock())
                ltw.visible = true
            }
        }

        //live text退出函数
        function exitLiveText() {
            console.debug("live exit")
            liveTextAnalyzer.breakAnalyze()
            ltw.clearLive()
            liveTextTimer.stop()
        }

        //live text分析启动控制
        function startLiveTextAnalyze() {
            liveTextTimer.restart()
        }

        //live text分析启动延迟
        Timer {
            id: liveTextTimer

            interval: 1500
            running: false
            repeat: false

            onTriggered: {
                if (fileControl.isCanSupportOcr(GControl.currentSource) && undefined !== targetImage) {
                    // 执行条件和OCR按钮使能条件一致
                    view.liveTextAnalyze()
                    running = false
                }
            }
        }

//        moveDisplaced: Transition {
//            NumberAnimation {
//                properties: "x,y"
//                duration: 100
//            }
//        }

    }

    IV.ImageInfo {
        id: currentImageInfo

        source: GControl.currentSource
    }

    // 图片变更时触发
    onTargetImageChanged: {
        // FIXME
        // 旋转状态
        // 适配窗口，匹配
        // NavigationWidget

        if (targetImage !== undefined) {
            recalculateLiveText()
        } else {
            exitLiveText()
        }
    }

    Connections {
        target: GStatus
        onViewFlickingChanged: {
            if (GStatus.viewFlicking) {
                view.exitLiveText()
            } else {
                view.startLiveTextAnalyze()
            }
        }
    }

    Connections {
        target: undefined === targetImage ? null : targetImage
        enabled: undefined !== targetImage
        ignoreUnknownSignals: true

        onXChanged: recalculateLiveText()
        onYChanged: recalculateLiveText()
        onPaintedWidthChanged: recalculateLiveText()
        onPaintedHeightChanged: recalculateLiveText()
        onScaleChanged: recalculateLiveText()
    }

    LiveTextWidget {
        //live text主控件
        id: ltw

        anchors.fill: imageViewerArea
        parent: imageViewerArea
    }

    FloatingButton {
        id: highlightTextButton

        property bool isHighlight: false

        checked: isHighlight
        width: 50
        height: 50
        visible: false
        parent: window
        anchors {
            right: parent.right
            rightMargin: 100
            bottom: parent.bottom
            bottomMargin: thumbnailViewBackGround.height + 20
        }

        icon {
            width: 45
            height: 45
            name: "icon_recognition_highlight"
        }

        onClicked: {
            isHighlight = !isHighlight
        }
    }

    //rename窗口
    ReName {
        id: renamedialog
    }

    // 右键菜单
    ViewRightMenu {
        id: rightMenu

        onClosed: {
            GStatus.showRightMenu = false
        }

        Connections {
            target: GStatus
            onShowRightMenuChanged: {
                if (GStatus.showRightMenu) {
                    rightMenu.popup(cursorTool.currentCursorPos())
                    rightMenu.focus = true
                }
            }
        }
    }

    // 图片信息窗口
    Loader {
        id: infomationDig

        function show() {
            if (infomationDig.status === Loader.Ready) {
                item.show()
            }
        }

        function hide() {
            if (infomationDig.status === Loader.Ready) {
                item.hide()
            }
        }

        asynchronous: true
        // 图片属性信息窗口
        source: "qrc:/qml/InformationDialog/InformationDialog.qml"
    }

    //导航窗口
    NavigationWidget {
        id: idNavWidget
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 109
        anchors.left: parent.left
        anchors.leftMargin: 15
        visible: false
    }

    onHeightChanged: {
        if (root.height <= global.minHideHeight) {
            idNavWidget.visible = false
        }
    }

    onWidthChanged: {
        if (root.width <= global.minWidth) {
            idNavWidget.visible = false
        }
    }

    // 导航窗口显示配置变更时触发
    onIsNavShowChanged: {
        // 保存设置信息
        fileControl.setEnableNavigation(isNavShow)
    }
}
