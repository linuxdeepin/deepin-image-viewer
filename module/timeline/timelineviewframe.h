#ifndef TIMELINEVIEWFRAME_H
#define TIMELINEVIEWFRAME_H

#include "controller/databasemanager.h"
#include "widgets/thumbnaillistview.h"
#include <QLabel>
#include <QWidget>
#include <QFrame>
#include <QListView>
#include <QStandardItem>
#include <QVBoxLayout>

class SignalManager;
class TimelineViewFrame : public QFrame
{
    Q_OBJECT
public:
    explicit TimelineViewFrame(const QString &timeline, QWidget *parent);
    ~TimelineViewFrame();
    void insertItem(const DatabaseManager::ImageInfo &info);
    bool removeItem(const QString &name);
    void clearSelection() const;
    void updateThumbnail(const QString &name);
    void setMultiSelection(bool multiple);
    bool isMultiSelection() const;
    bool posInSelected(const QPoint &pos);

    QMap<QString, QString> selectedImages() const;
    QString timeline() const;
    bool isEmpty() const;
    int hOffset() const;

    void setIconSize(const QSize &iconSize);
    void setTickable(bool v);

signals:
    void singleClicked(QMouseEvent *e);
    void clicked(const QModelIndex &index);
    void viewImage(const QString &path, const QStringList &paths);

protected:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;

private:
    void initListView();

private:
    QString m_timeline;
    ThumbnailListView *m_view;
    DatabaseManager *m_dbManager;
    SignalManager *m_sManager;
    QLabel *m_title;
    QLabel *m_separator;
};

#endif // TIMELINEVIEWFRAME_H
