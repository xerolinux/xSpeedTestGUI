#include <QClipboard>
#include <QGuiApplication>
#include <QImage>
#include <QtTest>

#include "resultshare.h"

class ResultShareTest : public QObject
{
    Q_OBJECT

private slots:
    void copyImagePutsItOnTheClipboard()
    {
        QImage image(12, 8, QImage::Format_ARGB32);
        image.fill(qRgba(30, 144, 255, 255));

        ResultShare share;
        QVERIFY(share.copyImage(image));

        const QImage back = QGuiApplication::clipboard()->image();
        QVERIFY(!back.isNull());
        QCOMPARE(back.size(), image.size());
        QCOMPARE(back, image);
    }

    void copyingANullImageFails()
    {
        ResultShare share;
        QVERIFY(!share.copyImage(QImage()));
    }
};

QTEST_MAIN(ResultShareTest)
#include "tst_resultshare.moc"
