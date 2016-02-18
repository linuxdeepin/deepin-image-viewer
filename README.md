# Deepin Viewer

Deepin Viewer is Deepin Desktop Environment Image Viewer.

## Dependencies

### Build dependencies

* qmake (>= 5.3)
* [libdui](https://github.com/linuxdeepin/libdui) (developer package)

### Runtime dependencies

* [libdui](https://github.com/linuxdeepin/libdui)
* Qt5 (>= 5.3)
  * Qt5-DBus
  * Qt5-Svg
  * Qt5-X11extras

## Installation

### Build from source code

1. Make sure you have installed all dependencies.

2. Build:
```
$ cd deepin-viewer
$ mkdir Build
$ cd Build
$ qmake ..
$ make
```

3. Install:
```
$ sudo make install
```

When install complete, the executable binary file is placed into `/usr/bin/deepin-viewer`.

## Getting help

Any usage issues can ask for help via
* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC Channel](https://webchat.freenode.net/?channels=deepin)
* [Official Forum](https://bbs.deepin.org/)
* [Wiki](http://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes
* [Contribution guide for users](http://wiki.deepin.org/index.php?title=Contribution_Guidelines_for_Users)
* [Contribution guide for developers](http://wiki.deepin.org/index.php?title=Contribution_Guidelines_for_Developers)

## License

DDE Dock is licensed under [GPLv3](https://github.com/linuxdeepin/developer-center/wiki/LICENSE).
