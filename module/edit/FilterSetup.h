#pragma once
#include <QWidget>

class FiltersPreview;
class FilterSetup : public QWidget
{
    Q_OBJECT
public:
    FilterSetup(QWidget* parent = 0);
    void setImage(const QString& path);
Q_SIGNALS:

private:
    FiltersPreview *m_preview;
};
