#pragma once

#include <QObject>

#include "SerialManager.h"

class SettingsManager : public QObject
{
    Q_OBJECT

public:
    explicit SettingsManager(QObject *parent = nullptr);

    void saveSerialSettings(const SerialManager::PortSettings &settings);
    SerialManager::PortSettings loadSerialSettings() const;

private:
    static constexpr const char *kGroupSerial = "serial";
};
