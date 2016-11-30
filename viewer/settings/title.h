#ifndef TITLELINE_H
#define TITLELINE_H

#include <QWidget>
#include <QLabel>

class Title1 : public QWidget
{
    Q_OBJECT
public:
    explicit Title1(const QString &title, QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

private:
    QString m_title;
};

class Title2 : public QLabel
{
    Q_OBJECT
public:
    explicit Title2(const QString &title, QWidget *parent = 0);
};

class Title3 : public QLabel
{
    Q_OBJECT
public:
    explicit Title3(const QString &title, QWidget *parent = 0);
};

#endif // TITLELINE_H
