#ifndef SHORTCUTFRAME_H
#define SHORTCUTFRAME_H

#include <QFrame>

class QVBoxLayout;
class ShortcutFrame : public QFrame
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

    QMap<QString, QString> viewValues();
    QMap<QString, QString> albumValues();
private:
    QVBoxLayout *m_layout;
};

#endif // SHORTCUTFRAME_H
