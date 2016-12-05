#ifndef SHORTCUTFRAME_H
#define SHORTCUTFRAME_H

#include <QWidget>

class QVBoxLayout;
class ShortcutFrame : public QWidget
{
    Q_OBJECT
public:
    explicit ShortcutFrame(QWidget *parent);

signals:
    void resetAll();

private:
    void checkShortcut(bool force = false);
    void resetShortcut();
    void initViewShortcut();
    void initAlbumShortcut();
    void initResetButton();
    const QString trLabel(const char *str);
    QMap<QString, QString> albumValues();
    QMap<QString, QString> viewValues();

private:
    QVBoxLayout *m_layout;
};

#endif // SHORTCUTFRAME_H
