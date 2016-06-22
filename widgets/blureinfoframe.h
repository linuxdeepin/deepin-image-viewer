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
class BlureInfoFrame : public BlureFrame
{
    Q_OBJECT
public:
    explicit BlureInfoFrame(QWidget *parent, QWidget *source);
    void setTopContent(QWidget *w);
    void addInfoPair(const QString &title, const QString &value);
    void close();

signals:
    void closed();

private:
    QVBoxLayout *m_topLayout;
    QVBoxLayout *m_bottomLayout;
    QFormLayout *m_infoLayout;
};

#endif // BLUREINFOFRAME_H
