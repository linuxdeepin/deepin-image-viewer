#ifndef BLUREINFOFRAME_H
#define BLUREINFOFRAME_H

#include "blureframe.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>

/*!
 * \brief The BlureInfoFrame class
 * Use for PopupImageInfo and PopupAlbumInfo
 */
class BlureInfoFrame : public BlurFrame
{
    Q_OBJECT
public:
    explicit BlureInfoFrame(QWidget *parent);
    void setTopContent(QWidget *w);
    void addInfoPair(const QString &title, const QString &value);
    void close();

signals:
    void closed();

private:
    int m_leftMax = 0;
    int m_rightMax = 0;
    QFrame *m_infoFrame;
    QVBoxLayout *m_topLayout;
    QVBoxLayout *m_bottomLayout;
    QFormLayout *m_infoLayout;
};

#endif // BLUREINFOFRAME_H
