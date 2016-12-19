#ifndef LOADINGICON_H
#define LOADINGICON_H

#include <dpicturesequenceview.h>
#include <QWidget>

DWIDGET_USE_NAMESPACE

class LoadingIcon : public DPictureSequenceView
{
    Q_OBJECT
public:
    explicit LoadingIcon(QWidget *parent = 0);

private:
    void updateIconPath();

    QStringList m_iconPaths;
};

#endif // LOADINGICON_H
