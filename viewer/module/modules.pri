!isEmpty(FULL_FUNCTIONALITY) {
    include (timeline/timeline.pri)
    include (album/album.pri)
}

include (view/view.pri)
#include (edit/edit.pri)
include (slideshow/slideshow.pri)

HEADERS += \
    $$PWD/modulepanel.h
