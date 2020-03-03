/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "shortcutframe.h"
#include <QDebug>
#include <QFormLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include "../title.h"
#include "application.h"
#include "controller/configsetter.h"
#include "shortcuteditor.h"

namespace {

const QString SHORTCUTVIEW = "SHORTCUTVIEW";
const QString SHORTCUTALBUM = "SHORTCUTALBUM";

struct ShortcutKey {
    QString key;
    const char *name;
};

static ShortcutKey ShortcutViewKeys[] = {
    {"View", QT_TRANSLATE_NOOP("ShortcutKey", "View")},
    {"Fullscreen", QT_TRANSLATE_NOOP("ShortcutKey", "Fullscreen")},
    {"Slide show", QT_TRANSLATE_NOOP("ShortcutKey", "Slide show")},
    {"End show", QT_TRANSLATE_NOOP("ShortcutKey", "End show")},
    {"Print", QT_TRANSLATE_NOOP("ShortcutKey", "Print")},
    {"Copy", QT_TRANSLATE_NOOP("ShortcutKey", "Copy")},
    {"Throw to trash", QT_TRANSLATE_NOOP("ShortcutKey", "Delete")},
    {"Remove from album", QT_TRANSLATE_NOOP("ShortcutKey", "Remove from album")},
    {"Favorite", QT_TRANSLATE_NOOP("ShortcutKey", "Favorite")},
    {"Unfavorite", QT_TRANSLATE_NOOP("ShortcutKey", "Unfavorite")},
    {"Rotate clockwise", QT_TRANSLATE_NOOP("ShortcutKey", "Rotate clockwise")},
    {"Rotate counterclockwise", QT_TRANSLATE_NOOP("ShortcutKey", "Rotate counterclockwise")},
    {"Set as wallpaper", QT_TRANSLATE_NOOP("ShortcutKey", "Set as wallpaper")},
    {"Display in file manager", QT_TRANSLATE_NOOP("ShortcutKey", "Display in file manager")},
    {"Image info", QT_TRANSLATE_NOOP("ShortcutKey", "Image info")},
    {"", ""}};

static ShortcutKey ShortcutAlbumKeys[] = {
    {"New album", QT_TRANSLATE_NOOP("ShortcutKey", "New album")},
    {"Rename", QT_TRANSLATE_NOOP("ShortcutKey", "Rename")},
    {"Delete", QT_TRANSLATE_NOOP("ShortcutKey", "Delete")},
    {"", ""}};

const QStringList InvisibleKeys = QStringList() << "View"
                                                << "Fullscreen"
                                                << "Quit slideshow"
                                                << "Copy"
                                                << "Throw to trash"
                                                << "Delete";

}  // namespace

ShortcutFrame::ShortcutFrame(QWidget *parent)
    : QFrame(parent)
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

    for (ShortcutKey *i = ShortcutViewKeys; !i->key.isEmpty(); i++) {
        if (InvisibleKeys.contains(i->key)) {
            continue;
        }
        ShortcutEditor *se = new ShortcutEditor(SHORTCUTVIEW, i->key);
        connect(this, &ShortcutFrame::resetAll, se, &ShortcutEditor::forceUpdate);
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

    for (ShortcutKey *i = ShortcutAlbumKeys; !i->key.isEmpty(); i++) {
        if (InvisibleKeys.contains(i->key)) {
            continue;
        }
        ShortcutEditor *se = new ShortcutEditor(SHORTCUTALBUM, i->key);
        connect(this, &ShortcutFrame::resetAll, se, &ShortcutEditor::forceUpdate);
        fl->addRow(new Title3(trLabel(i->name) + ":"), se);
    }

    m_layout->addLayout(layout);
}

void ShortcutFrame::initResetButton()
{
    QPushButton *resetBTN = new QPushButton(tr("Restore Defaults"));
    resetBTN->setFixedSize(310, 36);
    connect(resetBTN, &QPushButton::clicked, this, &ShortcutFrame::resetShortcut);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setAlignment(Qt::AlignHCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(resetBTN);

    m_layout->addSpacing(74);
    m_layout->addLayout(layout);
    m_layout->addSpacing(80);
}

QMap<QString, QString> ShortcutFrame::viewValues()
{
    QMap<QString, QString> vs;
    vs.insert("View", "Return");
    vs.insert("Fullscreen", "F11");
    vs.insert("Slide show", "F5");
    vs.insert("End show", "ESC");
    vs.insert("Print", "Ctrl + P");
    vs.insert("Copy", "Ctrl + C");
    vs.insert("Throw to trash", "Delete");
    vs.insert("Remove from album", "Shift + Delete");
    vs.insert("Favorite", "Ctrl + K");
    vs.insert("Unfavorite", "Ctrl + Shift + K");
    vs.insert("Rotate clockwise", "Ctrl + R");
    vs.insert("Rotate counterclockwise", "Ctrl + Shift + R");
    vs.insert("Set as wallpaper", "Ctrl + F9");
    vs.insert("Display in file manager", "Ctrl + D");
    vs.insert("Image info", "Alt + Enter");

    return vs;
}

QMap<QString, QString> ShortcutFrame::albumValues()
{
    QMap<QString, QString> vs;
    vs.insert("New album", "Ctrl + Shift + N");
    vs.insert("Rename", "F2");
    vs.insert("Delete", "Delete");

    return vs;
}

const QString ShortcutFrame::trLabel(const char *str)
{
    return qApp->translate("ShortcutKey", str);
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
