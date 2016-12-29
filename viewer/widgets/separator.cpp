#include "separator.h"
#include "utils/baseutils.h"

Separator::Separator(QWidget *parent) : QLabel(parent)
{
    setStyleSheet(utils::base::getFileContent(
                      ":/resources/common/qss/Separator.qss"));
}
