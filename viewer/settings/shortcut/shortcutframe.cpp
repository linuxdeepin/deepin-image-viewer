#include "shortcutframe.h"
#include "shortcuteditor.h"
#include "application.h"
#include "controller/configsetter.h"
#include "../title.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QDebug>

namespace {

const QString SHORTCUTVIEW = "SHORTCUTVIEW";
const QString SHORTCUTALBUM = "SHORTCUTALBUM";

struct ShortcutKey {
    QString key;
    const char *name;
};

static ShortcutKey ShortcutViewKeys[] = {
    {"View",                        QT_TRANSLATE_NOOP("ShortcutKey", "View")},
    {"Fullscreen",                  QT_TRANSLATE_NOOP("ShortcutKey", "Fullscreen")},
    {"Start slideshow",             QT_TRANSLATE_NOOP("ShortcutKey", "Start slideshow")},
//    {"Quit slideshow",              QT_TRANSLATE_NOOP("ShortcutKey", "Quit slideshow")},
    {"Print",                       QT_TRANSLATE_NOOP("ShortcutKey", "Print")},
    {"Copy",                        QT_TRANSLATE_NOOP("ShortcutKey", "Copy")},
    {"Throw to trash",              QT_TRANSLATE_NOOP("ShortcutKey", "Throw to trash")},
    {"Remove from album",           QT_TRANSLATE_NOOP("ShortcutKey", "Remove from album")},
    {"Add to my favorite",          QT_TRANSLATE_NOOP("ShortcutKey", "Add to my favorite")},
    {"Unfavorite",                  QT_TRANSLATE_NOOP("ShortcutKey", "Unfavorite")},
    {"Rotate clockwise",            QT_TRANSLATE_NOOP("ShortcutKey", "Rotate clockwise")},
    {"Rotate counterclockwise",     QT_TRANSLATE_NOOP("ShortcutKey", "Rotate counterclockwise")},
    {"Set as wallpaper",            QT_TRANSLATE_NOOP("ShortcutKey", "Set as wallpaper")},
    {"Display in file manager",     QT_TRANSLATE_NOOP("ShortcutKey", "Display in file manager")},
    {"Image info",                  QT_TRANSLATE_NOOP("ShortcutKey", "Image info")},
    {"", ""}
};

static ShortcutKey ShortcutAlbumKeys[] = {
    {"New album",                   QT_TRANSLATE_NOOP("ShortcutKey", "New album")},
    {"Rename",                      QT_TRANSLATE_NOOP("ShortcutKey", "Rename")},
    {"Delete",                      QT_TRANSLATE_NOOP("ShortcutKey", "Delete")},
    {"", ""}
};

}

ShortcutFrame::ShortcutFrame(QWidget *parent)
    :QFrame(parent)
{
    checkShortcut();

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 20, 0, 0);
    m_layout->setSpacing(0);

    Title1 *tl = new Title1(tr("Shortcuts"));
    m_layout->addWidget(tl);
    initViewShortcut();
    initAlbumShortcut();
    initResetButton();
}

void ShortcutFrame::initViewShortcut()
{
    Title2 *t2 = new Title2(tr("View Picture"));
    m_layout->addSpacing(10);
    m_layout->addWidget(t2);
    m_layout->addSpacing(10);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 96, 0);
    QFormLayout *fl = new QFormLayout;
    fl->setVerticalSpacing(10);
    fl->setHorizontalSpacing(40);
    fl->setContentsMargins(0, 0, 0, 0);
    fl->setLabelAlignment(Qt::AlignLeft);
//    layout->addStretch();
    layout->addSpacing(37);
    layout->addLayout(fl);

    for (ShortcutKey *i = ShortcutViewKeys; ! i->key.isEmpty(); i ++) {
        ShortcutEditor * se = new ShortcutEditor(SHORTCUTVIEW, i->key);
        connect(this, &ShortcutFrame::resetAll,
                se, &ShortcutEditor::forceUpdate);
        fl->addRow(new Title3(trLabel(i->name) + ":"), se);
    }

    m_layout->addLayout(layout);
}

void ShortcutFrame::initAlbumShortcut()
{
    Title2 *t2 = new Title2(tr("Album"));
    m_layout->addSpacing(10);
    m_layout->addWidget(t2);
    m_layout->addSpacing(10);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 96, 0);
    QFormLayout *fl = new QFormLayout;
    fl->setVerticalSpacing(10);
    fl->setHorizontalSpacing(100);
    fl->setContentsMargins(0, 0, 0, 0);
    fl->setLabelAlignment(Qt::AlignLeft);
//    layout->addStretch();
    layout->addSpacing(37);
    layout->addLayout(fl);

    for (ShortcutKey *i = ShortcutAlbumKeys; ! i->key.isEmpty(); i ++) {
        ShortcutEditor * se = new ShortcutEditor(SHORTCUTALBUM, i->key);
        connect(this, &ShortcutFrame::resetAll,
                se, &ShortcutEditor::forceUpdate);
        fl->addRow(new Title3(trLabel(i->name) + ":"), se);
    }

    m_layout->addLayout(layout);
}

void ShortcutFrame::initResetButton()
{
    QPushButton *resetBTN = new QPushButton(tr("Restore to default"));
    resetBTN->setFixedSize(310, 36);
    connect(resetBTN, &QPushButton::clicked,
            this, &ShortcutFrame::resetShortcut);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setAlignment(Qt::AlignHCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(resetBTN);

    m_layout->addSpacing(74);
    m_layout->addLayout(layout);
    m_layout->addSpacing(80);
}

const QString ShortcutFrame::trLabel(const char *str)
{
    return qApp->translate("ShortcutKey", str);
}

QMap<QString, QString> ShortcutFrame::viewValues()
{
    QMap<QString, QString> vs;
    vs.insert("View", "Return");
    vs.insert("Fullscreen", "F11");
    vs.insert("Start slideshow", "F5");
//    vs.insert("Quit slideshow", "ESC");
    vs.insert("Print", "Ctrl+P");
    vs.insert("Copy", "Ctrl+C");
    vs.insert("Throw to trash", "Delete");
    vs.insert("Remove from album", "Shift+Delete");
    vs.insert("Add to my favorite", "Ctrl+K");
    vs.insert("Unfavorite", "Ctrl+Shift+K");
    vs.insert("Rotate clockwise", "Ctrl+R");
    vs.insert("Rotate counterclockwise", "Ctrl+Shift+R");
    vs.insert("Set as wallpaper", "Ctrl+F8");
    vs.insert("Display in file manager", "Ctrl+D");
    vs.insert("Image info", "Alt+Return");

    return vs;
}

QMap<QString, QString> ShortcutFrame::albumValues()
{
    QMap<QString, QString> vs;
    vs.insert("New album", "Ctrl+Shift+N");
    vs.insert("Rename", "F2");
    vs.insert("Delete", "Delete");

    return vs;
}

void ShortcutFrame::checkShortcut(bool force)
{
    auto vvs = viewValues();
    for (QString key : vvs.keys()) {
        if (force || dApp->setter->value(SHORTCUTVIEW, key).isNull()) {
            dApp->setter->setValue(SHORTCUTVIEW, key, vvs[key]);
        }
    }

    auto avs = albumValues();
    for (QString key : avs.keys()) {
        if (force || dApp->setter->value(SHORTCUTALBUM, key).isNull()) {
            dApp->setter->setValue(SHORTCUTALBUM, key, avs[key]);
        }
    }
}

void ShortcutFrame::resetShortcut()
{
    checkShortcut(true);
    emit resetAll();
}
