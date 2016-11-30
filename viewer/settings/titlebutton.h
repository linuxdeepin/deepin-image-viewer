#ifndef TITLEBUTTON_H
#define TITLEBUTTON_H

#include <QWidget>

class TitleButton : public QWidget
{
    Q_OBJECT
public:
    enum SettingID{
        SlideshowSetting,
        SlideshowEffect,
        SlideshowTime,

        ShortcutSetting,
        ShortcutView,
        ShortcutAlbum
    };

    explicit TitleButton(SettingID id, bool bigFont, const QString &title, QWidget *parent = 0);

    SettingID id() const;
    void setId(const SettingID &id);

    bool isActived() const;
    void setIsActived(bool isActived);

signals:
    void clicked(SettingID id);

protected:
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

private:
    bool m_bigFont;
    bool m_isActived;
    SettingID m_id;
    QString m_title;
};

#endif // TITLEBUTTON_H
