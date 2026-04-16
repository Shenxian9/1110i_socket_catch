#include "LogFormatter.h"

#include <QDateTime>

QString LogFormatter::toHex(const QByteArray &data)
{
    if (data.isEmpty()) {
        return "";
    }

    return data.toHex(' ').toUpper();
}

QString LogFormatter::toAscii(const QByteArray &data)
{
    QString ascii;
    ascii.reserve(data.size());

    for (unsigned char byte : data) {
        if (byte >= 32 && byte <= 126) {
            ascii.append(QChar::fromLatin1(static_cast<char>(byte)));
        } else {
            ascii.append('.');
        }
    }

    return ascii;
}

QString LogFormatter::timestampNow()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
}

QString LogFormatter::buildLogLine(const QString &direction,
                                   const QByteArray &data,
                                   bool showHex,
                                   bool showAscii)
{
    QStringList fields;
    fields << QString("[%1] %2").arg(timestampNow(), direction);

    if (showHex) {
        fields << QString("HEX: %1").arg(toHex(data));
    }

    if (showAscii) {
        fields << QString("ASCII: %1").arg(toAscii(data));
    }

    return fields.join(" | ");
}
