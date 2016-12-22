#include "scrollbar.h"
#include "utils/baseutils.h"
#include <QDebug>

ScrollBar::ScrollBar(QWidget *parent)
    : DScrollBar(parent)
{
    setStyleSheet(utils::base::getFileContent(
                      ":/resources/common/qss/ScrollBar.qss"));
}
