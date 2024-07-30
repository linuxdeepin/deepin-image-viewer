// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes
import org.deepin.dtk 1.0
import org.deepin.image.viewer 1.0 as IV
import "./ImageDelegate"
import "./LiveText"
import "./InformationDialog"

Item {
    id: imageViewer

    // current rotate
    property int currentRotate: 0

    // 记录图像缩放，用于在窗口缩放时，根据前后窗口变化保持图片缩放比例
    property bool enableChangeDisplay: true
    property real lastDisplayScaleWidth: 0
    // Image 类型的对象，空图片、错误图片、消失图片等异常为 null
    property alias targetImage: view.currentImage

    // Note: 对于SVG、动图等特殊类型图片，使用 targeImage 获取的图片 sourceSize 存在差异，
    // 可能为零或导致缩放模糊，调整为使用从文件中读取的原始大小计算。
    // 图片旋转后同样会交换宽度和高度，更新缓存的图片源宽高信息
    property alias targetImageInfo: currentImageInfo
    property bool targetImageReady: (null !== view.currentImage) && (Image.Ready === view.currentImage.status)

    // 退出全屏展示图片
    function escBack() {
        IV.GStatus.showImageInfo = false;
        showNormal();
        showfullAnimation.start();
    }

    function exitLiveText() {
        view.exitLiveText();
    }

    function fitImage() {
        if (targetImageReady) {
            // 按图片原始大小执行缩放
            imageAnimation.scaleAnime(targetImageInfo.width / targetImage.paintedWidth);
        }
    }

    function fitWindow() {
        // 默认状态的图片即适应窗口大小(使用 Image.PreserveAspectFit)
        if (targetImageReady) {
            imageAnimation.scaleAnime(1.0);
        }
    }

    // 窗口拖拽大小变更时保持图片的显示缩放比例
    function keepImageDisplayScale() {
        if (!targetImageReady) {
            return;
        }

        // 当前缩放比例与匹配窗口的图片缩放比例比较，不一致则保持缩放比例
        if (Math.abs(targetImage.scale - 1.0) > Number.EPSILON) {
            if (0 !== lastDisplayScaleWidth) {
                // Note: 拖拽窗口时将保持 scale ，但 paintedWidth / paintedHeight 将变更
                // 因此在此处设置缩放比例时屏蔽重复设置，以保留缩放比例
                enableChangeDisplay = false;
                targetImage.scale = lastDisplayScaleWidth / targetImage.paintedWidth;
                enableChangeDisplay = true;
            } else {
                lastDisplayScaleWidth = targetImage.paintedWidth * targetImage.scale;
            }
        } else {
            // 一致则保持匹配窗口
            fitWindow();
        }
    }

    function recalculateLiveText() {
        if (targetImageReady && IV.Types.DynamicImage !== currentImageInfo.type) {
            exitLiveText();
            startLiveText();
        }
    }

    function rotateImage(angle) {
        if (targetImageReady) {
            IV.GControl.currentRotation += angle;
        }
    }

    // 触发全屏展示图片
    function showPanelFullScreen() {
        view.exitLiveText();
        IV.GStatus.showImageInfo = false;
        showFullScreen();
        view.contentItem.forceActiveFocus();
        showfullAnimation.start();
    }

    function showScaleFloatLabel() {
        // 不存在的图片不弹出缩放提示框
        if (!targetImageReady) {
            return;
        }

        // 图片实际缩放比值 绘制像素宽度 / 图片原始像素宽度
        var readableScale = targetImage.paintedWidth * targetImage.scale / targetImageInfo.width * 100;
        if (readableScale.toFixed(0) > 2000 && readableScale.toFixed(0) <= 3000) {
            floatLabel.displayStr = "2000%";
        } else if (readableScale.toFixed(0) < 2 && readableScale.toFixed(0) >= 0) {
            floatLabel.displayStr = "2%";
        } else if (readableScale.toFixed(0) >= 2 && readableScale.toFixed(0) <= 2000) {
            floatLabel.displayStr = readableScale.toFixed(0) + "%";
        }
        floatLabel.visible = true;
    }

    function startLiveText() {
        view.startLiveTextAnalyze();
    }

    onHeightChanged: keepImageDisplayScale()
    onTargetImageChanged: {
        // 部分多页图 targetImage 变更时不会重复触发 targetImageReady ，在此处进行备用的判断
        if (IV.Types.DynamicImage !== currentImageInfo.type) {
            // 适配窗口
            recalculateLiveText();
        } else {
            exitLiveText();
        }
    }

    // 图片状态变更时触发
    onTargetImageReadyChanged: {
        if (IV.Types.DynamicImage !== currentImageInfo.type) {
            // 适配窗口
            recalculateLiveText();
        }
        showScaleFloatLabel();

        // 重置保留的缩放状态
        lastDisplayScaleWidth = 0;
    }
    onWidthChanged: keepImageDisplayScale()

    // 图像动画：缩放
    ImageAnimation {
        id: imageAnimation

        targetImage: imageViewer.targetImage
    }

    Connections {
        function onPaintedHeightChanged() {
            recalculateLiveText();
        }

        function onPaintedWidthChanged() {
            recalculateLiveText();
        }

        function onScaleChanged() {
            // 图片实际缩放比值 绘制像素宽度 / 图片原始像素宽度
            var readableScale = targetImage.paintedWidth * targetImage.scale / targetImageInfo.width * 100;
            // 缩放限制在 2% ~ 2000% ，变更后再次进入此函数处理
            if (readableScale < 2) {
                targetImage.scale = targetImageInfo.width * 0.02 / targetImage.paintedWidth;
                return;
            } else if (readableScale > 2000) {
                targetImage.scale = targetImageInfo.width * 20 / targetImage.paintedWidth;
                return;
            }

            // 处于保持效果缩放状态时，保留之前的缩放比例
            if (enableChangeDisplay) {
                lastDisplayScaleWidth = targetImage.paintedWidth * targetImage.scale;
                // 显示缩放框
                showScaleFloatLabel();
            }

            // 重新文本识别
            recalculateLiveText();
        }

        function onXChanged() {
            recalculateLiveText();
        }

        function onYChanged() {
            recalculateLiveText();
        }

        enabled: targetImageReady
        ignoreUnknownSignals: true
        target: targetImage
    }

    // 触发切换全屏状态
    Connections {
        function onShowFullScreenChanged() {
            if (window.isFullScreen !== IV.GStatus.showFullScreen) {
                // 关闭详细信息窗口
                IV.GStatus.showImageInfo = false;
                IV.GStatus.showFullScreen ? showPanelFullScreen() : escBack();
            }
        }

        target: IV.GStatus
    }

    PropertyAnimation {
        id: showfullAnimation

        duration: 200
        easing.type: Easing.InExpo
        from: 0
        property: "opacity"
        target: parent.Window.window
        to: 1

        onRunningChanged: {
            IV.GStatus.fullScreenAnimating = running;
            // 动画结束时，重置缩放状态
            if (!running && targetImageReady) {
                // 匹配缩放处理
                if (targetImageInfo.height < targetImage.height) {
                    targetImage.scale = targetImageInfo.width / targetImage.paintedWidth;
                } else {
                    targetImage.scale = 1.0;
                }
            }
        }
    }

    //缩放快捷键
    Shortcut {
        sequence: "Ctrl+="

        onActivated: {
            targetImage.scale = targetImage.scale / 0.9;
        }
    }

    Shortcut {
        sequence: "Ctrl+-"

        onActivated: {
            targetImage.scale = targetImage.scale * 0.9;
        }
    }

    Shortcut {
        sequence: "Up"

        onActivated: {
            targetImage.scale = targetImage.scale / 0.9;
        }
    }

    Shortcut {
        sequence: "Down"

        onActivated: {
            targetImage.scale = targetImage.scale * 0.9;
        }
    }

    Shortcut {
        sequence: "Ctrl+Shift+/"

        onActivated: {
            var screenPos = mapToGlobal(parent.x, parent.y);
            IV.FileControl.showShortcutPanel(screenPos.x + parent.Window.width / 2, screenPos.y + parent.Window.height / 2);
        }
    }

    // 图片滑动视图的上层组件
    Item {
        id: viewBackground

        anchors.fill: parent
    }

    // 图片滑动视图
    ListView {
        id: view

        // 当前展示的 Image 图片对象，空图片、错误图片、消失图片等异常为 undefined
        // 此图片信息用于外部交互缩放、导航窗口等，已标识类型，使用 null !== currentImage 判断
        property Image currentImage: {
            if (view.currentItem) {
                if (view.currentItem.item) {
                    return view.currentItem.item.targetImage;
                }
            }
            return null;
        }

        //live text退出函数
        function exitLiveText() {
            liveTextAnalyzer.breakAnalyze();
            ltw.clearLive();
            liveTextTimer.stop();
        }

        //live text分析函数
        //缩放和切换需要重新执行此函数
        function liveTextAnalyze() {
            console.debug("Live Text analyze start");
            viewBackground.grabToImage(function (result) {
                    //截取当前控件显示
                    liveTextAnalyzer.setImage(result.image); //设置分析图片
                    liveTextAnalyzer.analyze(currentIndex); //执行分析（异步执行，函数会立即返回）
                    // result.saveToFile("/home/user/Desktop/viewer.png") //保存截取的图片，debug用
                });
        }

        //live text执行函数
        function runLiveText(resultCanUse, token) {
            console.debug("function runLiveText", token, currentIndex);
            if (resultCanUse && token == currentIndex) {
                //这里无视警告，就是需要js的==来进行自动类型转换
                console.debug("run live start");
                ltw.drawRect(liveTextAnalyzer.liveBlock());
                ltw.visible = true;
            }
        }

        //live text分析启动控制
        function startLiveTextAnalyze() {
            if (targetImageReady && IV.Types.DynamicImage !== currentImageInfo.type) {
                liveTextTimer.restart();
            }
        }

        // WARNING: 目前 ListView 组件屏蔽输入处理，窗口拖拽依赖底层的 ApplicationWindow
        // 因此不允许 ListView 的区域超过标题栏，图片缩放超过显示区域无妨。
        // 显示图片上下边界距边框 50px (标题栏宽度)，若上下间隔不一致时，进行拖拽、导航定位或需减去(间隔差/2)
        // 在全屏时无上下边框
        anchors.horizontalCenter: parent.horizontalCenter
        boundsBehavior: Flickable.StopAtBounds
        boundsMovement: Flickable.FollowBoundsBehavior
        cacheBuffer: 100
        currentIndex: IV.GControl.currentIndex
        flickDeceleration: 500
        height: window.isFullScreen ? parent.height : (parent.height - (IV.GStatus.titleHeight * 2))
        highlightMoveDuration: 0
        highlightRangeMode: ListView.StrictlyEnforceRange
        interactive: !IV.GStatus.fullScreenAnimating && IV.GStatus.viewInteractive
        model: IV.GControl.globalModel
        orientation: ListView.Horizontal

        // 设置滑动视图的父组件以获取完整的OCR图片信息
        parent: viewBackground
        preferredHighlightBegin: 0
        preferredHighlightEnd: 0
        snapMode: ListView.SnapOneItem
        width: parent.width
        y: window.isFullScreen ? 0 : IV.GStatus.titleHeight

        delegate: Loader {
            id: swipeViewItemLoader

            property alias frameCount: imageInfo.frameCount
            property url imageSource: model.imageUrl

            active: {
                if (ListView.isCurrentItem) {
                    return true;
                }
                if (view.currentIndex - 1 === index || view.currentIndex + 1 === index) {
                    return true;
                }
                return false;
            }
            asynchronous: true
            height: view.height
            visible: active
            width: view.width

            onActiveChanged: {
                if (active && imageInfo.delegateSource) {
                    setSource(imageInfo.delegateSource, {
                            "source": swipeViewItemLoader.imageSource,
                            "type": imageInfo.type
                        });
                }
            }

            // TODO: 这部分组件也应移动到 Component 中，再抽象一层组件，不提前创建
            IV.ImageInfo {
                id: imageInfo

                property url delegateSource
                property bool isCurrentItem: swipeViewItemLoader.ListView.isCurrentItem

                function checkDelegateSource() {
                    if (IV.ImageInfo.Ready !== status && IV.ImageInfo.Error !== status) {
                        return;
                    }
                    if (!imageInfo.exists) {
                        delegateSource = "qrc:/qml/ImageDelegate/NonexistImageDelegate.qml";
                        return;
                    }
                    switch (type) {
                    case IV.Types.NormalImage:
                        delegateSource = "qrc:/qml/ImageDelegate/NormalImageDelegate.qml";
                        return;
                    case IV.Types.DynamicImage:
                        delegateSource = "qrc:/qml/ImageDelegate/DynamicImageDelegate.qml";
                        return;
                    case IV.Types.SvgImage:
                        delegateSource = "qrc:/qml/ImageDelegate/SvgImageDelegate.qml";
                        return;
                    case IV.Types.MultiImage:
                        delegateSource = "qrc:/qml/ImageDelegate/MultiImageDelegate.qml";
                        return;
                    default:
                        // Default is damaged image.
                        delegateSource = "qrc:/qml/ImageDelegate/DamagedImageDelegate.qml";
                        return;
                    }
                }

                // WARNING: 由于 Delegate 组件宽度关联的 view.width ，ListView 会计算 Delegate 大小
                // Loader 在构造时直接设置图片链接会导致数据提前加载，破坏了延迟加载策略
                // 调整机制，不在激活状态的图片信息置为空，在需要加载时设置图片链接
                source: swipeViewItemLoader.active ? swipeViewItemLoader.imageSource : ""

                onDelegateSourceChanged: {
                    if (swipeViewItemLoader.active && delegateSource) {
                        setSource(delegateSource, {
                                "source": swipeViewItemLoader.imageSource,
                                "type": imageInfo.type
                            });
                    }
                }

                // InfoChange 在图片文件变更时触发，此时图片文件路径不变，文件内容被替换、删除
                onInfoChanged: {
                    if (isCurrentItem) {
                        IV.GControl.currentFrameIndex = 0;
                    }
                    checkDelegateSource();
                    var temp = delegateSource;
                    delegateSource = "";
                    delegateSource = temp;
                }
                onIsCurrentItemChanged: checkDelegateSource()
                onStatusChanged: checkDelegateSource()
            }
        }

        Component.onCompleted: {
            liveTextAnalyzer.analyzeFinished.connect(runLiveText);
        }
        onCurrentIndexChanged: {
            // 当通过界面拖拽导致索引变更，需要调整多页图索引范围
            if (view.currentIndex < IV.GControl.currentIndex) {
                IV.GControl.previousImage();
            } else if (view.currentIndex > IV.GControl.currentIndex) {
                IV.GControl.nextImage();
            }
        }
        onMovementEnded: {
            IV.GStatus.viewFlicking = false;
        }
        onMovementStarted: {
            IV.GStatus.viewFlicking = true;
        }

        BusyIndicator {
            anchors.centerIn: parent
            height: 48
            running: visible
            visible: {
                if (view.currentItem.status === Loader.Loading) {
                    return true;
                } else if (view.currentItem.item) {
                    return view.currentItem.item.status === Image.Loading;
                }
                // 非确定状态都为加载，以避免启动时的白屏过长
                return true;
            }
            width: 48
        }

        // 实况文本背景遮罩
        Rectangle {
            id: liveTextBackground

            anchors.centerIn: parent
            color: "#000000" // live text高亮阴影
            opacity: 0.5
            visible: highlightTextButton.checked && highlightTextButton.visible

            onVisibleChanged: {
                var target = view.currentImage;
                if (null !== target) {
                    width = target.paintedWidth * target.scale;
                    height = target.paintedHeight * target.scale;
                }
            }
        }

        //live text分析启动延迟
        Timer {
            id: liveTextTimer

            interval: 1500
            repeat: false
            running: false

            onTriggered: {
                var supportOcr = IV.FileControl.isCanSupportOcr(IV.GControl.currentSource);
                if (supportOcr && targetImageReady && IV.Types.DynamicImage !== currentImageInfo.type) {
                    // 执行条件和OCR按钮使能条件一致
                    view.liveTextAnalyze();
                    running = false;
                }
            }
        }
    }

    IV.ImageInfo {
        id: currentImageInfo

        frameIndex: IV.GControl.currentFrameIndex
        source: IV.GControl.currentSource
    }

    Connections {
        function onViewFlickingChanged() {
            if (IV.GStatus.viewFlicking) {
                view.exitLiveText();
            } else {
                view.startLiveTextAnalyze();
            }
        }

        target: IV.GStatus
    }

    LiveTextWidget {
        //live text主控件
        id: ltw

        anchors.fill: parent
        parent: stackView
    }

    FloatingButton {
        id: highlightTextButton

        property bool isHighlight: false

        checked: isHighlight
        height: 50
        parent: imageViewerArea
        visible: false
        width: 50
        z: parent.z + 100

        // 高亮时不弹出工具栏栏以方便选取
        onCheckedChanged: {
            IV.GStatus.animationBlock = checked;
        }
        onClicked: {
            isHighlight = !isHighlight;
        }
        onVisibleChanged: {
            if (!visible) {
                IV.GStatus.animationBlock = false;
            }
        }

        anchors {
            bottom: parent.bottom
            bottomMargin: thumbnailViewBackGround.height + 20
            right: parent.right
            rightMargin: 100
        }

        DciIcon {
            anchors.centerIn: parent
            height: 45
            name: "icon_recognition_highlight"
            palette: DTK.makeIconPalette(parent.palette)
            width: 45
        }
    }

    //rename窗口
    ReName {
        id: renamedialog

    }

    // 右键菜单
    ViewRightMenu {
        id: rightMenu

        // 拷贝快捷键冲突：选中实况文本时，屏蔽拷贝图片的快捷键
        copyableConfig: !ltw.currentHasSelect

        // 菜单销毁后也需要发送信号，否则可能未正常送达
        Component.onDestruction: {
            IV.GStatus.showRightMenu = false;
        }
        onClosed: {
            IV.GStatus.showRightMenu = false;
            imageViewer.forceActiveFocus();
        }

        Connections {
            function onShowRightMenuChanged() {
                if (IV.GStatus.showRightMenu) {
                    rightMenu.popup(IV.CursorTool.currentCursorPos());
                    rightMenu.focus = true;

                    // 关闭详细信息弹窗
                    IV.GStatus.showImageInfo = false;
                }
            }

            target: IV.GStatus
        }
    }

    // 图片信息窗口
    Loader {
        id: infomationDig

        function show() {
            IV.GStatus.showImageInfo = true;
        }

        active: IV.GStatus.showImageInfo
        asynchronous: true

        // 图片属性信息窗口
        sourceComponent: InformationDialog {
        }
    }

    //导航窗口
    Loader {
        id: naviLoader

        // 导航窗口是否显示
        property bool expectShow: IV.GStatus.enableNavigation && (null !== targetImage) && (targetImage.scale > 1)

        height: 112
        width: 150

        sourceComponent: NavigationWidget {
            targetImage: view.currentImage
            // 默认位置，窗体底部
            y: naviLoader.height + 70

            // 长时间隐藏，请求释放导航窗口
            onRequestRelease: {
                naviLoader.active = false;
            }
        }

        // 仅控制弹出显示导航窗口
        onExpectShowChanged: {
            if (expectShow) {
                active = true;
            }
        }

        anchors {
            bottom: parent.bottom
            bottomMargin: 109
            left: parent.left
            leftMargin: 15
        }
    }
}
