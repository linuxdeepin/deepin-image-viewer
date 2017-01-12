#include "volumemonitor.h"

#include "utils/baseutils.h"
#include <fcntl.h>
#include <unistd.h>
#include <QFile>
#include <QSet>
#include <QTextStream>
#include <QDebug>

namespace {

const char mountFile[] = "/proc/mounts";

QSet<QString> parseMountFile()
{
    using namespace utils::base;
    QString fileContent = getFileContent(mountFile);
    return QSet<QString>::fromList(fileContent.split("\n"));
}

QString getMountPoint(const QString& record)
{
    const QStringList items = record.split(" ");
    if (items.length() > 4) {
        return items.at(1);
    } else {
        return "";
    }
}

}  // namespace

VolumeMonitor *VolumeMonitor::m_monitor = NULL;
VolumeMonitor *VolumeMonitor::instance()
{
    if (! m_monitor) {
        m_monitor = new VolumeMonitor;
        m_monitor->start();
    }

    return m_monitor;
}

VolumeMonitor::VolumeMonitor(QObject *parent)
    : QObject(parent)
{
    this->setObjectName("VolumeMonitor");
}

bool VolumeMonitor::start()
{
    //get the set of mounted device's info;
    m_fileContentSet = parseMountFile();

    m_fileKde = open(mountFile, O_RDONLY);
    if (m_fileKde == -1) {
        qWarning() << "open /proc/mounts failed!";
        return false;
    }

    m_socketNotifier = new QSocketNotifier(m_fileKde,
                                           QSocketNotifier::Write, this);
    m_socketNotifier->setEnabled(true);
    connect(m_socketNotifier, &QSocketNotifier::activated,
            this, &VolumeMonitor::onFileChanged);
    return true;
}

bool VolumeMonitor::isRunning()
{
    if (m_fileKde!= -1 && m_socketNotifier) {
        return true;
    }
    else {
        return false;
    }
}

bool VolumeMonitor::stop()
{
    if (this->isRunning()) {
        close(m_fileKde);
        m_fileKde = -1;
        delete m_socketNotifier;
        m_socketNotifier = nullptr;
        return true;
    }
    else {
        return false;
    }
}
void VolumeMonitor::onFileChanged()
{
    QSet <QString> changedItemSet = parseMountFile();
    for(const QString& item: changedItemSet - m_fileContentSet) {
        emit this->deviceAdded(getMountPoint(item));
    }
    for(const QString& item: m_fileContentSet - changedItemSet) {
        emit this->deviceRemoved(getMountPoint(item));
    }

    m_fileContentSet = changedItemSet;
}

VolumeMonitor::~VolumeMonitor()
{
    this->stop();
}
