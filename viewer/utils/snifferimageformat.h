#ifndef SNIFFERIMAGEFORMAT_H
#define SNIFFERIMAGEFORMAT_H

#include <QString>

// Sniff image format based on its content.
// Returns an empty string if failed.
QString DetectImageFormat(const QString &filepath);

#endif // SNIFFERIMAGEFORMAT_H
