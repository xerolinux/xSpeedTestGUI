#include "resultshare.h"

#include <QGuiApplication>
#include <QClipboard>

ResultShare::ResultShare(QObject *parent)
    : QObject(parent)
{
}

bool ResultShare::copyImage(const QImage &image)
{
    if (image.isNull())
        return false;
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (!clipboard)
        return false;
    clipboard->setImage(image);
    return true;
}
