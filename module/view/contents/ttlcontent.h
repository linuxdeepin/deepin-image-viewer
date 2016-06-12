#ifndef TTLCONTENT_H
#define TTLCONTENT_H

#include <QWidget>

class TTLContent : public QWidget
{
    Q_OBJECT
public:
    enum ImageSource {
        FromAlbum,
        FromTimeline,
        FromFileManager
    };

    explicit TTLContent(ImageSource source, QWidget *parent = 0);

signals:
    void clicked(ImageSource source);
    void backToMain();
};

#endif // TTLCONTENT_H
