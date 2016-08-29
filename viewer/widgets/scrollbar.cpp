#include "scrollbar.h"
#include "utils/baseutils.h"

ScrollBar::ScrollBar(QWidget *parent)
    : DScrollBar(parent)
{
    setStyleSheet(utils::base::getFileContent(
                      ":/qss/resources/qss/ScrollBar.qss"));
}
