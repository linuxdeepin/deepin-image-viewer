// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEEDITCONTROLLER_H
#define IMAGEEDITCONTROLLER_H

#include <QImage>
#include <QObject>
#include <QQuickImageProvider>
#include <QReadWriteLock>
#include <QRectF>
#include <QUrl>
#include <QVariantList>
#include <QVector>

class ImageEditController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ active NOTIFY activeChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY historyChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY historyChanged)
    Q_PROPERTY(bool modified READ modified NOTIFY historyChanged)
    Q_PROPERTY(int revision READ revision NOTIFY revisionChanged)

public:
    explicit ImageEditController(QObject *parent = nullptr);

    bool active() const;
    bool canRedo() const;
    bool canUndo() const;
    bool modified() const;
    int revision() const;
    QImage image() const;

    Q_INVOKABLE bool beginEdit(const QUrl &source, int frameIndex = 0);
    Q_INVOKABLE bool canEdit(const QUrl &source) const;
    Q_INVOKABLE void discard();
    Q_INVOKABLE bool isEditing(const QUrl &source, int frameIndex) const;
    Q_INVOKABLE bool applyEffect(const QString &effect, const QRectF &normalizedRect, int strength);
    Q_INVOKABLE bool crop(const QRectF &normalizedRect);
    Q_INVOKABLE void commitHistory();
    Q_INVOKABLE bool redo();
    Q_INVOKABLE bool undo();
    Q_INVOKABLE QUrl defaultSaveUrl() const;
    Q_INVOKABLE bool isOriginalUrl(const QUrl &destination) const;
    Q_INVOKABLE bool saveComposite(const QUrl &destination, const QVariantList &annotations);
    Q_INVOKABLE void markSaved();

Q_SIGNALS:
    void activeChanged();
    void historyChanged();
    void saveFailed(const QString &message);
    void revisionChanged();

private:
    QRect pixelRect(const QRectF &normalizedRect) const;
    void applyBoxBlur(const QRect &rect, int radius);
    void applyMosaic(const QRect &rect, int blockSize);
    void applyGraffiti(const QRect &rect, int spacing);

    mutable QReadWriteLock m_lock;
    QImage m_image;
    QUrl m_source;
    int m_frameIndex = 0;
    int m_revision = 0;
    QVector<QImage> m_history;
    int m_historyIndex = -1;
    int m_savedHistoryIndex = 0;
};

class EditedImageProvider : public QQuickImageProvider
{
public:
    explicit EditedImageProvider(ImageEditController *controller);
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    ImageEditController *m_controller = nullptr;
};

#endif // IMAGEEDITCONTROLLER_H
