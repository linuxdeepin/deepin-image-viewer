#pragma once

#include <QObject>
#include <QImage>
#include <QQuickImageProvider>

namespace DeepinOCRPlugin {
    class DeepinOCRDriver;
}

class LiveTextAnalyzer : public QObject, public QQuickImageProvider
{
    Q_OBJECT
public:
    explicit LiveTextAnalyzer(QObject *parent = nullptr);
    Q_INVOKABLE void setImage(const QImage &image);

    Q_INVOKABLE QVariant liveBlock() const;
    Q_INVOKABLE QVariant charBox(int blockIndex) const;
    Q_INVOKABLE QString textResult(int blockIndex, int startIndex, int len) const;

signals:
    void analyzeFinished(bool resultCanUse);

public slots:
    Q_INVOKABLE void analyze();
    Q_INVOKABLE void breakAnalyze();

protected:
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    DeepinOCRPlugin::DeepinOCRDriver *ocrDriver;
    QImage imageCache;
};
