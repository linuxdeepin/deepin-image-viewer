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
#include "timelinepanel.h"
#include "contents/timelinebtcontent.h"
#include "application.h"
#include "controller/configsetter.h"
#include "controller/importer.h"
#include "controller/signalmanager.h"
#include "timelineframe.h"
#include "utils/imageutils.h"
#include "widgets/imagebutton.h"
#include "widgets/importframe.h"
#include "widgets/themewidget.h"

#include <QDebug>
#include <QDropEvent>
#include <QLabel>
#include <QMenu>
#include <QMimeData>
#include <QStackedWidget>
#include <QUrl>
#include <QVBoxLayout>

namespace {

const int ICON_MARGIN = 13;
//in order to make the toptoolbarContent align center, add leftMargin 82;
const int MARGIN_DIFF = 82;

}  //namespace

using namespace Dtk::Widget;

TimelinePanel::TimelinePanel(QWidget *parent)
    : ModulePanel(parent)
{
    setAcceptDrops(true);

    initMainStackWidget();
    initStyleSheet();
    initPopupMenu();
    initConnection();
}

bool TimelinePanel::isMainPanel()
{
    return true;
}

QString TimelinePanel::moduleName()
{
    return "TimelinePanel";
}

QWidget *TimelinePanel::toolbarBottomContent()
{
    TimelineBTContent *c = new TimelineBTContent(":/resources/dark/qss/timeline.qss",
                                                 ":/resources/light/qss/timeline.qss");
//    c->setStyleSheet(this->styleSheet());
    connect(m_frame, &TimelineFrame::changeItemSize,
            c, &TimelineBTContent::changeItemSize);
    connect(c, &TimelineBTContent::itemSizeChanged,
            m_frame, &TimelineFrame::setIconSize);
    m_frame->setIconSize(c->iconSize());
    return c;
}

QWidget *TimelinePanel::toolbarTopLeftContent()
{
    ThemeWidget *w = new ThemeWidget(":/resources/dark/qss/timeline.qss",
                                     ":/resources/light/qss/timeline.qss");

    QLabel *label = new QLabel;
    label->setObjectName("TopleftLogo");
    label->setFixedSize(24, 24);
    QHBoxLayout *layout = new QHBoxLayout(w);
    layout->setContentsMargins(ICON_MARGIN, 0, 0, 0);
    layout->addWidget(label);

    return w;
}

QWidget *TimelinePanel::toolbarTopMiddleContent()
{
    ThemeWidget *tTopMiddleContent = new ThemeWidget(":/resources/dark/qss/timeline.qss",
                                                     ":/resources/light/qss/timeline.qss");
    ImageButton *timelineButton = new ImageButton();
    timelineButton->setObjectName("TimelineBtn");

    ImageButton *albumButton = new ImageButton();
    albumButton->setObjectName("AlbumBtn");

    connect(albumButton, &ImageButton::clicked, this, [=] {
        qDebug() << "Change to Album Panel...";
        emit dApp->signalM->gotoAlbumPanel();
    });
    albumButton->setToolTip(tr("Album"));

    QHBoxLayout *layout = new QHBoxLayout(tTopMiddleContent);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addStretch();
    layout->addWidget(timelineButton);
    layout->addWidget(albumButton);
    layout->addStretch();

    return tTopMiddleContent;
}

QWidget *TimelinePanel::extensionPanelContent()
{
    return NULL;
}

void TimelinePanel::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
        QStringList files;
        for (QUrl url : urls) {
            const QString path = url.toLocalFile();
            if (QFileInfo(path).isDir()) {
                // Need popup AlbumCreate dialog
                emit dApp->signalM->importDir(path);
            }
            else {
                files << path;
            }
        }
        if (! files.isEmpty()) {
           Importer::instance()->appendFiles(files);
        }
    }
}

void TimelinePanel::dragEnterEvent(QDragEnterEvent *event)
{
    event->setDropAction(Qt::CopyAction);
    event->accept();
}

void TimelinePanel::showEvent(QShowEvent *e)
{
    ModulePanel::showEvent(e);

    updateMenuContents();
}

void TimelinePanel::initConnection()
{
    connect(Importer::instance(), &Importer::imported, this, [=] (bool success) {
        if (! success) {
            return;
        }
        onImageCountChanged();
    });
    connect(dApp->setter, &ConfigSetter::valueChanged,
            this, &TimelinePanel::updateMenuContents);
    connect(dApp->signalM, &SignalManager::imagesInserted,
            this, &TimelinePanel::onImageCountChanged);
    connect(dApp->signalM, &SignalManager::imagesRemoved,
            this, &TimelinePanel::onImageCountChanged);
    connect(dApp->signalM, &SignalManager::gotoTimelinePanel, this, [=] {
        emit dApp->signalM->gotoPanel(this);
    });
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &TimelinePanel::onThemeChanged);
}

void TimelinePanel::initMainStackWidget()
{
    initImagesFrame();

    m_importFrame = new ImportFrame(this);
    m_importFrame->setButtonText(tr("Add"));
    m_importFrame->setTitle(tr("You can add sync directory or drag and drop  images to timeline"));
    connect(m_importFrame, &ImportFrame::clicked, this, [=] {
        Importer::instance()->showImportDialog();
    });

    m_mainStack = new QStackedWidget;
    m_mainStack->setContentsMargins(0, 0, 0, 0);
    m_mainStack->addWidget(m_importFrame);
//    m_mainStack->addWidget(m_view);
    m_mainStack->addWidget(m_frame);
    //show import frame if no images in database
    m_mainStack->setCurrentIndex(DBManager::instance()->getImgsCount() > 0 ? 1 : 0);

    QLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_mainStack);
}

void TimelinePanel::initImagesFrame()
{
    m_frame = new TimelineFrame;
    connect(m_frame, &TimelineFrame::viewImage,
            this, [=] (const QString &path, const QStringList &paths) {
        SignalManager::ViewInfo vinfo;
        vinfo.lastPanel = this;
        vinfo.path = path;
        vinfo.paths = paths;
        emit dApp->signalM->viewImage(vinfo);
    });

    // NOTE: It's important to use Qt::QueuedConnection to avoid crash
    connect(m_frame, &TimelineFrame::selectIndexChanged,
            this, &TimelinePanel::updateMenuContents, Qt::QueuedConnection);
    connect(m_frame, &TimelineFrame::showMenu, this, [=] {
        updateMenuContents();
        m_menu->popup(QCursor::pos());
    });
}

void TimelinePanel::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    Q_UNUSED(theme)
    initStyleSheet();
    if (this->isVisible()) {
        emit dApp->signalM->updateTopToolbarLeftContent(toolbarTopLeftContent());
        emit dApp->signalM->updateBottomToolbarContent(toolbarBottomContent());
        emit dApp->signalM->updateTopToolbarMiddleContent(toolbarTopMiddleContent());
    }
}
void TimelinePanel::initStyleSheet()
{
    if (dApp->viewerTheme->getCurrentTheme() == ViewerThemeManager::Dark) {
//        setStyleSheet(utils::base::getFileContent(":/resources/dark/qss/timeline.qss"));
    }
    else {
//        setStyleSheet(utils::base::getFileContent(":/resources/light/qss/timeline.qss"));
    }
}

void TimelinePanel::onImageCountChanged()
{
    int count =  DBManager::instance()->getImgsCount();
    m_mainStack->setCurrentIndex(count > 0 ? 1 : 0);
}
