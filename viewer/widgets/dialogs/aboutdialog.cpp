/*
 * Copyright (C) 2016 ~ 2017 Deepin Technology Co., Ltd.
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
#include "aboutdialog.h"

namespace {

const QString WINDOW_ICON = "";
const QString PRODUCT_ICON = ":/dialogs/images/resources/images/deepin-image-viewer.png";
const QString VERSION = "1.2";

}  // namespace

AboutDialog::AboutDialog()
    : DAboutDialog()
{
    setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint);
    setModal(true);
    setProductIcon(QIcon(PRODUCT_ICON));
    setProductName(tr("Deepin Image Viewer"));
    setVersion(tr("Version:") + VERSION);
    //FIXME: acknowledgementLink is empty!
    setAcknowledgementLink("https://www.deepin.org/acknowledgments/deepin-image-viewer/");
    setDescription(tr("Deepin Image Viewer is a fashion & smooth image manager.") +
                   "\n" +
                   tr("It is featured with image management, image viewing and basic image editing."));

    connect(this, SIGNAL(closed()), this, SLOT(deleteLater()));
}
