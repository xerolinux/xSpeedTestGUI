#pragma once

#include <QImage>
#include <QObject>
#include <QtQml/qqmlregistration.h>

class ResultShare : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ResultShare(QObject *parent = nullptr);

    Q_INVOKABLE bool copyImage(const QImage &image);
};
