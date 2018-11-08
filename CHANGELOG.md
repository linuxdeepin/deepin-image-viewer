<a name="1.3.1"></a>
## 1.3.1 (2018-11-08)


#### Bug Fixes

*   empty command can start a new window ([577ec38b](https://github.com/linuxdeepin/deepin-image-viewer/commit/577ec38b2b4378113c44e2286b2ca668dc57c11e))
*   shortcut preview info is empty ([239c9c9a](https://github.com/linuxdeepin/deepin-image-viewer/commit/239c9c9a8de61f5d0f3cbf5a05d7b009a4c68f38))
*   shortcut is invalid ([74e17db2](https://github.com/linuxdeepin/deepin-image-viewer/commit/74e17db2fe4d869bb7d78697ad9666b58553a2ef))



<a name="1.3.0"></a>
## 1.3.0 (2018-10-24)


#### Features

*   add logo on topbar ([f2f70f5f](https://github.com/linuxdeepin/deepin-image-viewer/commit/f2f70f5f7c4ee4cf966bd3c21edeccbf0e41b716))

#### Bug Fixes

*   crash when press "Delete" ([8c67acc9](https://github.com/linuxdeepin/deepin-image-viewer/commit/8c67acc99a5d55fe39a0523aae71216475678c68))
*   remove dtk old interface. ([59ec8eb6](https://github.com/linuxdeepin/deepin-image-viewer/commit/59ec8eb67982b0862071639042fd15d32e60aaac))



<a name="1.2.23"></a>
### 1.2.23 (2018-07-20)


#### Bug Fixes

*   memory leak ([d680e97c](https://github.com/linuxdeepin/deepin-image-viewer/commit/d680e97c63df617b34bdda0dd3aaa3d2bb347caa))



<a name="1.2.22"></a>
## 1.2.22 (2018-05-24)


#### Bug Fixes

*   don't restore cursor. ([6f81631e](6f81631e))
* **flatpak:**  default save path error in flatpak environment ([699b4ae6](699b4ae6))



<a name="1.2.21"></a>
## 1.2.21 (2018-05-15)

*   Fix image is empty


<a name="1.2.20"></a>
## 1.2.20 (2018-05-14)


#### Bug Fixes

*   can't print image in landscape mode ([a63841a3](a63841a3))



<a name="1.2.19"></a>
### 1.2.19 (2018-04-11)


#### Features

* **print_dialog:**  add image settings option. ([53a8c772](53a8c772))



<a name="1.2.18"></a>
### 1.2.18 (2018-03-22)


#### Features

*   set wallpaper with dbus in flatpak ([30b00248](30b00248))

#### Bug Fixes

*   favorite menu and title states not synced ([2b16ceaa](2b16ceaa))

<a name="1.2.17"></a>
## 1.2.17 (2018-03-15)


#### Features

*   add notice for tif ([60e5a2da](60e5a2da))
*   add manual id ([65408f0b](65408f0b))
*   imageutils cutSquareImage support hidpi ([67fcbbb7](67fcbbb7))

#### Bug Fixes

*   toast hide failed ([9955ccfb](9955ccfb))
*   cleared item can be shown again ([afbc2b45](afbc2b45))
*   remove stays-on-top hint from scan dialog ([7c559016](7c559016))
*   build failed on static build ([c917a434](c917a434))
*   set titlebar background transparent ([c9dc6da9](c9dc6da9))
*   set pixmap device pixel ratio to device pixel ratio of window ([c792a3d5](c792a3d5))
*   remove seek operation ([b2019875](b2019875))
*   dxcb workaround ([c9cc715a](c9cc715a))
*   Adapt lintian ([dcad7f8a](dcad7f8a))
*   hide image on press super+d ([ca79cd62](ca79cd62))
*   set wallpaper by dde dbus ([b6f9a64c](b6f9a64c))
*   mainwindow button now showing on dark mode ([af0ff636](af0ff636))
*   Replace dbus-send with QDBusInterface ([86599aa4](86599aa4))
*   skip de check for flatpak ([3655aa3c](3655aa3c))
*   fix install path ([efefd53e](efefd53e))
*   setting wallpaper(convert the image's format). ([646d32c7](646d32c7))
* **hidpi:**
  *  top-left mainwindow icon not scaled ([54c971f8](54c971f8))
  *  some widgets are not showing clear ([4ba95264](4ba95264))
  *  some parts are not shown very clear ([4910a8af](4910a8af))
* **slideshow:**  cursor not showing with menu ([bd1e3499](bd1e3499))
* **timeline:**
  *  menu content changed after showing ([6b306252](6b306252))
  *  timeline date not showing ([3116dd53](3116dd53))
