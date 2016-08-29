#ifndef DEFAULTIMAGEVIEWER_H
#define DEFAULTIMAGEVIEWER_H

#include <QObject>

namespace service {
    //check the current image viewer is deepin-image-viewer.
    bool isDefaultImageViewer();
    //Register/unregister as the default image viewer.
    bool setDefaultImageViewer(bool isDefault);
}

#endif // DEFAULTIMAGEVIEWER_H
