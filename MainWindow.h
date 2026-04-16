#pragma once

#include <QByteArray>
#include <QMainWindow>

#include "SerialManager.h"

class QComboBox;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QCheckBox;
class QRadioButton;
class QSpinBox;

class SettingsManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void refreshPorts();
    void openPort();
    void closePort();

    void applyPresetFbus();
    void applyPresetLegacy();

    void sendManualData();
    void sendSync55();

    void handleReceived(const QByteArray &data);
    void handleSent(const QByteArray &data);
    void handleSerialError(const QString &errorText);
    void handleConnectionState(bool connected, const QString &portName);

    void saveLogToFile();
    void clearLog();
    void refreshLogView();

private:
    enum class EntryType {
        Data,
        Info,
        Error,
    };

    struct LogEntry {
        QString timestamp;
        EntryType type = EntryType::Data;
        QString direction;
        QByteArray data;
        QString text;
    };

    void buildUi();
    void wireSignals();
    void loadSettings();
    void saveSettings();

    SerialManager::PortSettings currentSettingsFromUi() const;
    void applySettingsToUi(const SerialManager::PortSettings &settings);

    static bool parseHexInput(const QString &input, QByteArray *output, QString *errorText);

    void appendDataLog(const QString &direction, const QByteArray &data);
    void appendInfoLog(const QString &message);
    void appendErrorLog(const QString &message);

    QString renderEntry(const LogEntry &entry) const;

    SerialManager m_serial;
    SettingsManager *m_settings = nullptr;
    QList<LogEntry> m_entries;

    QComboBox *m_portCombo = nullptr;
    QComboBox *m_baudCombo = nullptr;
    QComboBox *m_dataBitsCombo = nullptr;
    QComboBox *m_parityCombo = nullptr;
    QComboBox *m_stopBitsCombo = nullptr;
    QComboBox *m_flowControlCombo = nullptr;

    QPushButton *m_refreshPortsBtn = nullptr;
    QPushButton *m_openBtn = nullptr;
    QPushButton *m_closeBtn = nullptr;
    QLabel *m_statusLabel = nullptr;

    QPushButton *m_presetFbusBtn = nullptr;
    QPushButton *m_presetLegacyBtn = nullptr;

    QPlainTextEdit *m_logEdit = nullptr;
    QCheckBox *m_showHexCheck = nullptr;
    QCheckBox *m_showAsciiCheck = nullptr;
    QCheckBox *m_autoScrollCheck = nullptr;

    QPlainTextEdit *m_sendEdit = nullptr;
    QRadioButton *m_sendHexRadio = nullptr;
    QRadioButton *m_sendTextRadio = nullptr;
    QCheckBox *m_logTxCheck = nullptr;

    QPushButton *m_sendBtn = nullptr;
    QPushButton *m_sendSyncBtn = nullptr;
    QSpinBox *m_syncCountSpin = nullptr;

    QPushButton *m_saveLogBtn = nullptr;
    QPushButton *m_clearLogBtn = nullptr;
};
