# Deepin Image Viewer

看图是一款外观时尚、性能流畅的图片查看工具。

### 依赖

### 运行时依赖

* [deepin-tool-kit](https://github.com/linuxdeepin/deepin-tool-kit)
* [deepin-shortcut-viewer](https://github.com/linuxdeepin/deepin-shortcut-viewer)
* [deepin-manual](https://github.com/linuxdeepin/deepin-manual)
* libexif
* freeimage
* libraw
* Qt5 (>= 5.6)
  * Qt5-DBus
  * Qt5-Svg
  * Qt5-X11extras
* libimageeditor-dev

## 安装

sudo apt install cmake qtbase5-dev pkg-config libexif-dev libqt5svg5-dev libqt5x11extras5-dev libsqlite3-dev qttools5-dev-tools qttools5-dev libxcb-util0-dev libstartup-notification0-dev libraw-dev libfreeimage-dev libqt5opengl5-dev qtbase5-private-dev qtmultimedia5-dev x11proto-xext-dev libmtdev-dev libegl1-mesa-dev libudev-dev libfontconfig1-dev libfreetype6-dev libglib2.0-dev libxrender-dev libdtkwidget-dev libdtkwidget5-bin libdtkcore5-bin libimageeditor-dev

### 从源代码构建

1. 确保已经安装了所有的编译依赖.
```
$ sudo apt build-dep .
```

2. 构建:
```
$ cd deepin-viewer
$ mkdir Build
$ cd Build
$ qmake ..
$ make
```

3. 安装:
```
$ sudo make install
```
## 使用

执行 `deepin-image-viewer`

## 文档

 - [Development Documentation](https://linuxdeepin.github.io/deepin-image-viewer/)
 - [用户文档](https://wikidev.uniontech.com/index.php?title=%E7%9C%8B%E5%9B%BE)
安装完成后，可执行文件路径为`/usr/bin/deepin-viewer`.

## 帮助

任何使用问题都可以通过以下方式寻求帮助:
* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC Channel](https://webchat.freenode.net/?channels=deepin)
* [Official Forum](https://bbs.deepin.org/)
* [Wiki](https://wiki.deepin.org/)

## 参与贡献

我们鼓励您报告问题并作出更改：

* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) 

## 协议

Deepin Image Viewer 遵循协议 [GPLv3](LICENSE).
