#pragma once

#include <QByteArray>
#include <QString>

class LogFormatter
{
public:
    static QString toHex(const QByteArray &data);
    static QString toAscii(const QByteArray &data);
    static QString timestampNow();
    static QString buildLogLine(const QString &direction,
                                const QByteArray &data,
                                bool showHex,
                                bool showAscii);
};
