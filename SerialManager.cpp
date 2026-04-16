#include "SerialManager.h"

SerialManager::SerialManager(QObject *parent)
    : QObject(parent)
{
    connect(&m_port, &QSerialPort::readyRead, this, &SerialManager::onReadyRead);
    connect(&m_port,
            &QSerialPort::errorOccurred,
            this,
            &SerialManager::onErrorOccurred);
}

QList<QSerialPortInfo> SerialManager::availablePorts() const
{
    return QSerialPortInfo::availablePorts();
}

bool SerialManager::openPort(const PortSettings &settings, QString *errorText)
{
    if (m_port.isOpen()) {
        m_port.close();
    }

    m_port.setPortName(settings.portName);
    m_port.setBaudRate(settings.baudRate);
    m_port.setDataBits(settings.dataBits);
    m_port.setParity(settings.parity);
    m_port.setStopBits(settings.stopBits);
    m_port.setFlowControl(settings.flowControl);

    if (!m_port.open(QIODevice::ReadWrite)) {
        const QString reason = m_port.errorString();
        if (errorText) {
            *errorText = reason;
        }
        emit errorHappened(QString("打开串口失败: %1").arg(reason));
        emit connectionStateChanged(false, settings.portName);
        return false;
    }

    emit connectionStateChanged(true, settings.portName);
    return true;
}

void SerialManager::closePort()
{
    const QString name = m_port.portName();
    if (m_port.isOpen()) {
        m_port.close();
    }
    emit connectionStateChanged(false, name);
}

bool SerialManager::isOpen() const
{
    return m_port.isOpen();
}

bool SerialManager::sendBytes(const QByteArray &payload, QString *errorText)
{
    if (!m_port.isOpen()) {
        const QString reason = QStringLiteral("串口尚未打开");
        if (errorText) {
            *errorText = reason;
        }
        emit errorHappened(reason);
        return false;
    }

    const qint64 written = m_port.write(payload);
    if (written == -1) {
        const QString reason = m_port.errorString();
        if (errorText) {
            *errorText = reason;
        }
        emit errorHappened(QString("发送失败: %1").arg(reason));
        return false;
    }

    if (written != payload.size()) {
        const QString reason = QString("发送不完整: %1 / %2 字节").arg(written).arg(payload.size());
        if (errorText) {
            *errorText = reason;
        }
        emit errorHappened(reason);
        return false;
    }

    emit dataSent(payload);
    return true;
}

bool SerialManager::startBridgeMode(const QString &leftPortName,
                                    const QString &rightPortName,
                                    QString *errorText)
{
    Q_UNUSED(leftPortName);
    Q_UNUSED(rightPortName);

    const QString reason = QStringLiteral("桥接模式尚未实现（预留接口）");
    if (errorText) {
        *errorText = reason;
    }
    emit errorHappened(reason);
    return false;
}

void SerialManager::stopBridgeMode()
{
    // 预留接口：后续桥接模式实现时在此停止双串口转发。
}

void SerialManager::onReadyRead()
{
    const QByteArray incoming = m_port.readAll();
    if (!incoming.isEmpty()) {
        emit dataReceived(incoming);
    }
}

void SerialManager::onErrorOccurred(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError) {
        return;
    }

    const QString reason = m_port.errorString();
    emit errorHappened(QString("串口错误: %1").arg(reason));

    if (error == QSerialPort::ResourceError || error == QSerialPort::DeviceNotFoundError) {
        closePort();
    }
}
