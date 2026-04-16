#include "SettingsManager.h"

#include <QSettings>

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
{
}

void SettingsManager::saveSerialSettings(const SerialManager::PortSettings &settings)
{
    QSettings storage;
    storage.beginGroup(kGroupSerial);
    storage.setValue("portName", settings.portName);
    storage.setValue("baudRate", settings.baudRate);
    storage.setValue("dataBits", static_cast<int>(settings.dataBits));
    storage.setValue("parity", static_cast<int>(settings.parity));
    storage.setValue("stopBits", static_cast<int>(settings.stopBits));
    storage.setValue("flowControl", static_cast<int>(settings.flowControl));
    storage.endGroup();
}

SerialManager::PortSettings SettingsManager::loadSerialSettings() const
{
    QSettings storage;
    storage.beginGroup(kGroupSerial);

    SerialManager::PortSettings settings;
    settings.portName = storage.value("portName", "").toString();
    settings.baudRate = storage.value("baudRate", QSerialPort::Baud115200).toInt();
    settings.dataBits = static_cast<QSerialPort::DataBits>(
        storage.value("dataBits", static_cast<int>(QSerialPort::Data8)).toInt());
    settings.parity = static_cast<QSerialPort::Parity>(
        storage.value("parity", static_cast<int>(QSerialPort::NoParity)).toInt());
    settings.stopBits = static_cast<QSerialPort::StopBits>(
        storage.value("stopBits", static_cast<int>(QSerialPort::OneStop)).toInt());
    settings.flowControl = static_cast<QSerialPort::FlowControl>(
        storage.value("flowControl", static_cast<int>(QSerialPort::NoFlowControl)).toInt());

    storage.endGroup();
    return settings;
}
