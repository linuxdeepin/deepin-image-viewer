#pragma once
#include <QWidget>

class FiltersPreview;
class FilterSetup : public QWidget
{
    Q_OBJECT
public:
    FilterSetup(QWidget* parent = 0);
    void setImage(const QString& path);
    QString imagePath() const;
Q_SIGNALS:
    void filterIdChanged(int);
    void filterIndensityChanged(qreal);

private:
    FiltersPreview *m_preview;
    QString m_path;
};
