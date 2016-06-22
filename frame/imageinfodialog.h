#ifndef IMAGEINFODIALOG_H
#define IMAGEINFODIALOG_H

#include "widgets/blureinfoframe.h"

class QLabel;
class QFormLayout;
class ImageInfoDialog : public BlureInfoFrame
{
    Q_OBJECT
public:
    explicit ImageInfoDialog(QWidget *parent, QWidget *source);
    void setPath(const QString &path);

private:
    QLabel *m_thumbnail;
};

#endif // IMAGEINFODIALOG_H
