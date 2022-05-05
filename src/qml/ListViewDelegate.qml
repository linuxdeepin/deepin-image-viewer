import QtQuick 2.1
import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.3

Item {
    id: container
    height: container.ListView.view.height - 40
    width: height
    property int imgRadius: 3
    y: 20
//    scale: bottomthumbnaillistView.currentIndex === index ? 1.2 :1.0

    Rectangle {
        id: enterShader
        height: parent.width + 6
        width: height
        anchors.top: parent.top
        anchors.topMargin: -3

        anchors.left: parent.left
        anchors.leftMargin: -3

        radius: imgRadius * 2
//        color: Material.accent
        color:"#0081FF"
        visible: false
    }

    Image {
        id: img
        width: container.width - 10
        height: container.height - 10
        anchors.centerIn: parent
        smooth: false
        anchors.fill: parent
        source:  fileControl.isSvgImage(sourcePaths[index]) ? mainView.sourcePaths[index] : "image://ThumbnailImage/" + mainView.sourcePaths[index]
        sourceSize.width: 100
        sourceSize.height: 100
        asynchronous: true
        visible: false
        cache: true

        onStatusChanged: {
//            if (img.status === Image.Ready)
//                console.log('Ready')
//             if(img.status === Image.Loading)
//                console.log('Loading')
//             if(img.status === Image.Null)
//                console.log('Null')
             if(img.status === Image.Error){
                img.source = "qrc:/res/picture_damaged_58.svg"
             }
        }
    }
    Rectangle {
        id: mask
        anchors.fill: img
        visible: false
        radius: imgRadius
    }

    OpacityMask {
        anchors.fill: img
        source: img
        maskSource: mask
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true

        onClicked: {
            container.ListView.view.currentIndex = index
            container.forceActiveFocus()
            source = mainView.sourcePaths[index]
            imageViewer.index = index
        }
    }
    states: State {
        name: "active"
        when: bottomthumbnaillistView.currentIndex === index
//        when: container.activeFocus
        PropertyChanges {
            target: container
            scale: 1.2
        }
        PropertyChanges {
            target: enterShader
            visible: true
        }

    }

    transitions: Transition {
        NumberAnimation {
            properties: "scale"
            duration: 100
        }
    }
}
