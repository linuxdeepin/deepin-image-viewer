TEMPLATE  = subdirs

SUBDIRS  += \
    dbase \
    dutil \
    dwidget \
    examples \

dutil.depends = dbase
dwidget.depends = dutil
