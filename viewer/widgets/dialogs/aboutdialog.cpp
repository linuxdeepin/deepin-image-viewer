#include "aboutdialog.h"

namespace {

const QString WINDOW_ICON = "";
const QString PRODUCT_ICON = ":/dialogs/images/resources/images/deepin-image-viewer.png";
const QString VERSION = "1.2";

}  // namespace

AboutDialog::AboutDialog()
    : DAboutDialog()
{
    setProductIcon(QIcon(PRODUCT_ICON));
    setProductName(tr("Deepin Image Viewer"));
    setVersion(tr("Version:") + VERSION);
    setDescription(tr("Deepin Image Viewer is a fashion & smooth image manager.") +
                   "\n" +
                   tr("It is featured with image management, image viewing and basic image editing."));
    setLicense(tr("Deepin Image Viewer is released under GPL v3."));

    connect(this, SIGNAL(closed()), this, SLOT(deleteLater()));
}
