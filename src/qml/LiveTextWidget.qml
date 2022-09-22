import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Controls 2.4
//import QtQuick.Controls 1.4
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0

import "LiveBlockController.js" as BlockLoader

Item {
    id: root
    visible: false

    //广播信号
    //退出Live Text模式
    signal liveExit()

    //清除选择效果，参数为发出此信号的index，-1表示全部清除且只有root可以发出
    signal clearSelect(int pressesIndex)

    //鼠标在block中点击，此时要初始化所有block的选择能力，鼠标按下的X坐标，鼠标按下的Y坐标
    signal mousePressedInBlock(int pressX, int pressY)

    //鼠标在整个Live区域中移动，此时鼠标必定是保持press状态，参数为当前鼠标的X坐标，当前鼠标的Y坐标
    signal mouseMoveInLive(int moveX, int moveY)

    //鼠标在block中释放，此时要解除所有block的选择状态
    signal mouseReleasedInBlock()

    //block汇总
    property var blockArray : new Array

    //目前暂时只能处理对齐的矩形框
    //blocks: QList<QList<float>>
    function drawRect(blocks) {
        for(var id = 0;id !== blocks.length;id++) {
            BlockLoader.createBlockObjects(root)
            var rectDetail = BlockLoader.block
            rectDetail.index = id
            rectDetail.width = blocks[id][2] - blocks[id][0]
            rectDetail.height = blocks[id][5] - blocks[id][3]
            rectDetail.x = blocks[id][0]
            rectDetail.y = blocks[id][1]
            rectDetail.setCharLocations(liveTextAnalyzer.charBox(id))
            //控件销毁
            liveExit.connect(rectDetail.destroy)
            //清除选择
            clearSelect.connect(rectDetail.clearSelect)
            //框选相关
            //广播信号发出
            mousePressedInBlock.connect(rectDetail.mousePressed)
            mouseMoveInLive.connect(rectDetail.mouseAxisChanged)
            mouseReleasedInBlock.connect(rectDetail.mouseReleased)
            //触发广播信号
            rectDetail.blockPressed.connect(mousePressedInBlock)
            rectDetail.blockMouseMoved.connect(mouseMoveInLive)
            rectDetail.blockMouseReleased.connect(mouseReleasedInBlock)
            //弹出菜单
            rectDetail.blockMenu.connect(doPopMenu)
            //保存
            blockArray.push(rectDetail)
        }
    }

    function doTextCopy() {
        var currentData
        var result = ""
        for(var i = 0;i != blockArray.length;++i) {
            currentData = blockArray[i].getSelectedText()
            if(currentData.length !== 0) {
                result = result + currentData + "\n"
            }
        }
        fileControl.copyText(result)
    }

    function doTextSelectAll() {
        for(var i = 0;i != blockArray.length;++i) {
            blockArray[i].selectAll()
        }
    }

    function doPopMenu() {
        liveMenu.popup()
    }

    function clearLive() {
        root.visible = false
        liveExit()
        blockArray = []
    }

    function haveSelect() {
        var have = false
        for(var i = 0;i != blockArray.length;++i) {
            if(blockArray[i].getSelectedTextCount() !== 0) {
                have = true
                break
            }
        }
        return have
    }

    Rectangle {
        id: highlightTextRect
        width: 75
        height: 75
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20

        property bool isHighlight: false

        Button {
            id: highlightTextButton
            text: qsTr("Highlight\nText")
            anchors.fill: parent

            onClicked: {
                for(var i = 0;i != blockArray.length;++i) {
                    blockArray[i].enableHighLight(!parent.isHighlight)
                }
                parent.isHighlight = !parent.isHighlight
            }
        }
    }

    Menu {
        id: liveMenu

        MenuItem {
            id: copyItem
            text: qsTr("Copy (Ctrl+C)")
            onTriggered: {
                doTextCopy()
            }

            Shortcut {
                sequence: "Ctrl+C"
                enabled: parent.enabled
                onActivated: {
                    doTextCopy()
                }
            }
        }

        MenuItem {
            text: qsTr("Select all (Ctrl+A)")
            onTriggered: {
                doTextSelectAll()
            }

            Shortcut {
                sequence: "Ctrl+A"
                onActivated: {
                    doTextSelectAll()
                }
            }
        }

        onVisibleChanged: {
            if(visible == true) {
                copyItem.enabled = haveSelect()
            }
        }
    }
}
