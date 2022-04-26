import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Dialogs 1.3
import Qt.labs.folderlistmodel 2.11
import org.deepin.dtk 1.0


Item {
    id: openwidget
    property string file
    //    anchors.fill: parent

    Rectangle{
        color:"#F8F8F8"
        anchors.fill: parent
        anchors.centerIn: openwidget
        Image {
            id: openWidgetImage
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            fillMode: Image.Pad
            source: "qrc:/res/icon_import_photo.svg"

            width:300
        }

        RecommandButton{

            id: openFileBtn
            font.capitalization: Font.MixedCase
            text: qsTr("Open Image")

            onClicked: fileDialog.open()
            width: 300
            height: 35
            anchors.top:openWidgetImage.bottom
            anchors.topMargin:10

            anchors.left : openWidgetImage.left
            anchors.leftMargin: 0

        }
    }
    Shortcut {
        sequence: "Ctrl+O"
        onActivated:{
            if(stackView.currentWidgetIndex!= 2){
                fileDialog.open()
            }
        }
    }
    FileDialog {
        id: fileDialog
        title: qsTr("Select pictures")
        folder: shortcuts.pictures
        selectMultiple: true
        nameFilters: ["Image files (*.jpg *.png *.bmp *gif)"]
        onAccepted: {
            mainView.sourcePaths = fileControl.getDirImagePath(fileDialog.fileUrls[0]);
            mainView.source = fileDialog.fileUrls[0]
            mainView.currentIndex=mainView.sourcePaths.indexOf(mainView.source)
            if(mainView.sourcePaths.length >0){

                mainView.setThumbnailCurrentIndex(mainView.sourcePaths.indexOf(mainView.source))
                console.log( "test",mainView.source)
                stackView.currentWidgetIndex= 1
            }

        }
    }

    FolderListModel
    {
        id: foldermodel
        folder: "file://" + platform.picturesLocation()
        showDirs: false
        showDotAndDotDot: false
        nameFilters: ["*.dng", "*.nef", "*.bmp", "*.gif", "*.ico", "*.jpeg", "*.jpg", "*.pbm", "*.pgm","*.png",  "*.pnm", "*.ppm",
            "*.svg", "*.tga", "*.tif", "*.tiff", "*.wbmp", "*.webp", "*.xbm", "*.xpm", "*.gif"]
        sortField: FolderListModel.Type
        showOnlyReadable: true
        sortReversed: false

        onCountChanged: {
            //           if(!root.run){
            //              root.fileMonitor()
            //           }
        }
    }
    Component.onCompleted: {

        var tempPath =fileControl.parseCommandlineGetPath("x");

        mainView.sourcePaths = fileControl.getDirImagePath(tempPath);
        mainView.source=tempPath
        if(mainView.sourcePaths.length >0){
            mainView.setThumbnailCurrentIndex(mainView.sourcePaths.indexOf(mainView.source))

            stackView.currentWidgetIndex= 1
        }
    }

}
