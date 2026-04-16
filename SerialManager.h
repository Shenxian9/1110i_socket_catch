#pragma once

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>

class SerialManager : public QObject
{
    Q_OBJECT

public:
    explicit SerialManager(QObject *parent = nullptr);

    struct PortSettings {
        QString portName;
        qint32 baudRate = QSerialPort::Baud115200;
        QSerialPort::DataBits dataBits = QSerialPort::Data8;
        QSerialPort::Parity parity = QSerialPort::NoParity;
        QSerialPort::StopBits stopBits = QSerialPort::OneStop;
        QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl;
    };

    QList<QSerialPortInfo> availablePorts() const;

    bool openPort(const PortSettings &settings, QString *errorText = nullptr);
    void closePort();
    bool isOpen() const;

    bool sendBytes(const QByteArray &payload, QString *errorText = nullptr);

    // 扩展预留：后续可实现双串口桥接（当前版本返回 false）。
    bool startBridgeMode(const QString &leftPortName,
                         const QString &rightPortName,
                         QString *errorText = nullptr);
    void stopBridgeMode();

signals:
    void dataReceived(const QByteArray &data);
    void dataSent(const QByteArray &data);
    void errorHappened(const QString &errorText);
    void connectionStateChanged(bool connected, const QString &portName);

private slots:
    void onReadyRead();
    void onErrorOccurred(QSerialPort::SerialPortError error);

private:
    QSerialPort m_port;
};
