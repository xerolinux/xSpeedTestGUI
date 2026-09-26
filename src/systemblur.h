#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>

class SystemBlur : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int strength READ strength NOTIFY strengthChanged)
    Q_PROPERTY(bool available READ available NOTIFY strengthChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)

public:
    static constexpr int MinStrength = 1;
    static constexpr int MaxStrength = 15;
    static constexpr int DefaultStrength = 8;

    explicit SystemBlur(QObject *parent = nullptr);

    int strength() const { return m_strength; }
    bool available() const { return m_available; }
    QString errorString() const { return m_error; }

    Q_INVOKABLE void apply(int strength);

signals:
    void strengthChanged();
    void errorStringChanged();

private:
    void setError(const QString &message);

    int m_strength = DefaultStrength;
    bool m_available = false;
    QString m_error;
};
