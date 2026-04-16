#include "MainWindow.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QFileDialog>
#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSaveFile>
#include <QSpinBox>
#include <QScrollBar>
#include <QTextStream>
#include <QVBoxLayout>

#include "LogFormatter.h"
#include "SettingsManager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_settings(new SettingsManager(this))
{
    buildUi();
    wireSignals();

    refreshPorts();
    loadSettings();
    refreshLogView();
}

MainWindow::~MainWindow()
{
    saveSettings();
    m_serial.closePort();
}

void MainWindow::buildUi()
{
    setWindowTitle("NokiaFBusSniffer - Serial Packet Debug Tool");
    resize(1200, 800);

    auto *central = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);

    auto *configGroup = new QGroupBox("串口配置", central);
    auto *configLayout = new QGridLayout(configGroup);

    m_portCombo = new QComboBox(configGroup);
    m_baudCombo = new QComboBox(configGroup);
    m_dataBitsCombo = new QComboBox(configGroup);
    m_parityCombo = new QComboBox(configGroup);
    m_stopBitsCombo = new QComboBox(configGroup);
    m_flowControlCombo = new QComboBox(configGroup);

    m_baudCombo->addItem("9600", QSerialPort::Baud9600);
    m_baudCombo->addItem("115200", QSerialPort::Baud115200);
    m_baudCombo->addItem("19200", QSerialPort::Baud19200);
    m_baudCombo->addItem("38400", QSerialPort::Baud38400);
    m_baudCombo->addItem("57600", QSerialPort::Baud57600);

    m_dataBitsCombo->addItem("5", QSerialPort::Data5);
    m_dataBitsCombo->addItem("6", QSerialPort::Data6);
    m_dataBitsCombo->addItem("7", QSerialPort::Data7);
    m_dataBitsCombo->addItem("8", QSerialPort::Data8);

    m_parityCombo->addItem("None", QSerialPort::NoParity);
    m_parityCombo->addItem("Even", QSerialPort::EvenParity);
    m_parityCombo->addItem("Odd", QSerialPort::OddParity);
    m_parityCombo->addItem("Mark", QSerialPort::MarkParity);
    m_parityCombo->addItem("Space", QSerialPort::SpaceParity);

    m_stopBitsCombo->addItem("1", QSerialPort::OneStop);
    m_stopBitsCombo->addItem("1.5", QSerialPort::OneAndHalfStop);
    m_stopBitsCombo->addItem("2", QSerialPort::TwoStop);

    m_flowControlCombo->addItem("None", QSerialPort::NoFlowControl);
    m_flowControlCombo->addItem("RTS/CTS", QSerialPort::HardwareControl);
    m_flowControlCombo->addItem("XON/XOFF", QSerialPort::SoftwareControl);

    m_refreshPortsBtn = new QPushButton("刷新串口", configGroup);
    m_openBtn = new QPushButton("打开串口", configGroup);
    m_closeBtn = new QPushButton("关闭串口", configGroup);
    m_statusLabel = new QLabel("状态: 未连接", configGroup);

    configLayout->addWidget(new QLabel("串口", configGroup), 0, 0);
    configLayout->addWidget(m_portCombo, 0, 1);
    configLayout->addWidget(new QLabel("波特率", configGroup), 0, 2);
    configLayout->addWidget(m_baudCombo, 0, 3);
    configLayout->addWidget(new QLabel("数据位", configGroup), 1, 0);
    configLayout->addWidget(m_dataBitsCombo, 1, 1);
    configLayout->addWidget(new QLabel("校验位", configGroup), 1, 2);
    configLayout->addWidget(m_parityCombo, 1, 3);
    configLayout->addWidget(new QLabel("停止位", configGroup), 2, 0);
    configLayout->addWidget(m_stopBitsCombo, 2, 1);
    configLayout->addWidget(new QLabel("流控", configGroup), 2, 2);
    configLayout->addWidget(m_flowControlCombo, 2, 3);

    auto *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(m_refreshPortsBtn);
    btnLayout->addWidget(m_openBtn);
    btnLayout->addWidget(m_closeBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(m_statusLabel);
    configLayout->addLayout(btnLayout, 3, 0, 1, 4);

    auto *presetGroup = new QGroupBox("研究预设", central);
    auto *presetLayout = new QHBoxLayout(presetGroup);
    m_presetFbusBtn = new QPushButton("Nokia FBUS 115200 8N1", presetGroup);
    m_presetLegacyBtn = new QPushButton("Legacy 9600 8N1", presetGroup);
    presetLayout->addWidget(m_presetFbusBtn);
    presetLayout->addWidget(m_presetLegacyBtn);
    presetLayout->addStretch();

    auto *logGroup = new QGroupBox("日志", central);
    auto *logLayout = new QVBoxLayout(logGroup);

    auto *logOptLayout = new QHBoxLayout();
    m_showHexCheck = new QCheckBox("显示 HEX", logGroup);
    m_showAsciiCheck = new QCheckBox("显示 ASCII", logGroup);
    m_autoScrollCheck = new QCheckBox("自动滚动", logGroup);
    m_showHexCheck->setChecked(true);
    m_showAsciiCheck->setChecked(true);
    m_autoScrollCheck->setChecked(true);

    logOptLayout->addWidget(m_showHexCheck);
    logOptLayout->addWidget(m_showAsciiCheck);
    logOptLayout->addWidget(m_autoScrollCheck);
    logOptLayout->addStretch();

    m_logEdit = new QPlainTextEdit(logGroup);
    m_logEdit->setReadOnly(true);

    logLayout->addLayout(logOptLayout);
    logLayout->addWidget(m_logEdit);

    auto *sendGroup = new QGroupBox("发送", central);
    auto *sendLayout = new QVBoxLayout(sendGroup);

    m_sendEdit = new QPlainTextEdit(sendGroup);
    m_sendEdit->setPlaceholderText("输入 HEX 或文本数据...");
    m_sendEdit->setPlainText("55 55 55 55 55 55 55 55");

    auto *sendModeLayout = new QHBoxLayout();
    m_sendHexRadio = new QRadioButton("HEX 模式", sendGroup);
    m_sendTextRadio = new QRadioButton("文本模式", sendGroup);
    m_sendHexRadio->setChecked(true);
    m_logTxCheck = new QCheckBox("发送后记录 TX", sendGroup);
    m_logTxCheck->setChecked(true);

    sendModeLayout->addWidget(m_sendHexRadio);
    sendModeLayout->addWidget(m_sendTextRadio);
    sendModeLayout->addWidget(m_logTxCheck);
    sendModeLayout->addStretch();

    auto *actionLayout = new QHBoxLayout();
    m_sendBtn = new QPushButton("发送", sendGroup);
    m_sendSyncBtn = new QPushButton("发送 0x55 同步序列", sendGroup);
    m_syncCountSpin = new QSpinBox(sendGroup);
    m_syncCountSpin->setRange(1, 4096);
    m_syncCountSpin->setValue(64);
    m_saveLogBtn = new QPushButton("保存日志", sendGroup);
    m_clearLogBtn = new QPushButton("清空日志", sendGroup);

    actionLayout->addWidget(m_sendBtn);
    actionLayout->addSpacing(16);
    actionLayout->addWidget(m_sendSyncBtn);
    actionLayout->addWidget(new QLabel("长度", sendGroup));
    actionLayout->addWidget(m_syncCountSpin);
    actionLayout->addStretch();
    actionLayout->addWidget(m_saveLogBtn);
    actionLayout->addWidget(m_clearLogBtn);

    sendLayout->addWidget(m_sendEdit);
    sendLayout->addLayout(sendModeLayout);
    sendLayout->addLayout(actionLayout);

    mainLayout->addWidget(configGroup);
    mainLayout->addWidget(presetGroup);
    mainLayout->addWidget(logGroup, 1);
    mainLayout->addWidget(sendGroup);

    setCentralWidget(central);
}

void MainWindow::wireSignals()
{
    connect(m_refreshPortsBtn, &QPushButton::clicked, this, &MainWindow::refreshPorts);
    connect(m_openBtn, &QPushButton::clicked, this, &MainWindow::openPort);
    connect(m_closeBtn, &QPushButton::clicked, this, &MainWindow::closePort);

    connect(m_presetFbusBtn, &QPushButton::clicked, this, &MainWindow::applyPresetFbus);
    connect(m_presetLegacyBtn, &QPushButton::clicked, this, &MainWindow::applyPresetLegacy);

    connect(m_sendBtn, &QPushButton::clicked, this, &MainWindow::sendManualData);
    connect(m_sendSyncBtn, &QPushButton::clicked, this, &MainWindow::sendSync55);

    connect(m_saveLogBtn, &QPushButton::clicked, this, &MainWindow::saveLogToFile);
    connect(m_clearLogBtn, &QPushButton::clicked, this, &MainWindow::clearLog);

    connect(m_showHexCheck, &QCheckBox::toggled, this, &MainWindow::refreshLogView);
    connect(m_showAsciiCheck, &QCheckBox::toggled, this, &MainWindow::refreshLogView);

    connect(&m_serial, &SerialManager::dataReceived, this, &MainWindow::handleReceived);
    connect(&m_serial, &SerialManager::dataSent, this, &MainWindow::handleSent);
    connect(&m_serial, &SerialManager::errorHappened, this, &MainWindow::handleSerialError);
    connect(&m_serial,
            &SerialManager::connectionStateChanged,
            this,
            &MainWindow::handleConnectionState);
}

void MainWindow::refreshPorts()
{
    const QString current = m_portCombo->currentText();
    m_portCombo->clear();

    const QList<QSerialPortInfo> ports = m_serial.availablePorts();
    for (const QSerialPortInfo &info : ports) {
        const QString label = QString("%1 (%2)").arg(info.portName(), info.description());
        m_portCombo->addItem(label, info.portName());
    }

    if (!current.isEmpty()) {
        for (int i = 0; i < m_portCombo->count(); ++i) {
            if (m_portCombo->itemData(i).toString() == current || m_portCombo->itemText(i).contains(current)) {
                m_portCombo->setCurrentIndex(i);
                break;
            }
        }
    }

    appendInfoLog(QString("已刷新串口列表，共 %1 个串口").arg(ports.size()));
}

void MainWindow::openPort()
{
    const SerialManager::PortSettings settings = currentSettingsFromUi();
    if (settings.portName.isEmpty()) {
        QMessageBox::warning(this, "提示", "请选择串口后再打开。");
        return;
    }

    QString errorText;
    if (!m_serial.openPort(settings, &errorText)) {
        QMessageBox::critical(this, "打开串口失败", errorText);
        return;
    }

    saveSettings();
    appendInfoLog(QString("串口已打开: %1").arg(settings.portName));
}

void MainWindow::closePort()
{
    if (!m_serial.isOpen()) {
        return;
    }

    m_serial.closePort();
    appendInfoLog("串口已关闭");
}

void MainWindow::applyPresetFbus()
{
    m_baudCombo->setCurrentIndex(m_baudCombo->findData(QSerialPort::Baud115200));
    m_dataBitsCombo->setCurrentIndex(m_dataBitsCombo->findData(QSerialPort::Data8));
    m_parityCombo->setCurrentIndex(m_parityCombo->findData(QSerialPort::NoParity));
    m_stopBitsCombo->setCurrentIndex(m_stopBitsCombo->findData(QSerialPort::OneStop));
    m_flowControlCombo->setCurrentIndex(m_flowControlCombo->findData(QSerialPort::NoFlowControl));
    appendInfoLog("已应用预设: Nokia FBUS 115200 8N1");
}

void MainWindow::applyPresetLegacy()
{
    m_baudCombo->setCurrentIndex(m_baudCombo->findData(QSerialPort::Baud9600));
    m_dataBitsCombo->setCurrentIndex(m_dataBitsCombo->findData(QSerialPort::Data8));
    m_parityCombo->setCurrentIndex(m_parityCombo->findData(QSerialPort::NoParity));
    m_stopBitsCombo->setCurrentIndex(m_stopBitsCombo->findData(QSerialPort::OneStop));
    m_flowControlCombo->setCurrentIndex(m_flowControlCombo->findData(QSerialPort::NoFlowControl));
    appendInfoLog("已应用预设: Legacy 9600 8N1");
}

void MainWindow::sendManualData()
{
    const QString input = m_sendEdit->toPlainText().trimmed();
    if (input.isEmpty()) {
        QMessageBox::warning(this, "提示", "发送内容为空。");
        return;
    }

    QByteArray payload;
    if (m_sendHexRadio->isChecked()) {
        QString error;
        if (!parseHexInput(input, &payload, &error)) {
            QMessageBox::critical(this, "HEX 输入错误", error);
            return;
        }
    } else {
        payload = input.toUtf8();
    }

    QString errorText;
    if (!m_serial.sendBytes(payload, &errorText)) {
        QMessageBox::critical(this, "发送失败", errorText);
    }
}

void MainWindow::sendSync55()
{
    const int count = m_syncCountSpin->value();
    QByteArray payload(count, static_cast<char>(0x55));

    QString errorText;
    if (!m_serial.sendBytes(payload, &errorText)) {
        QMessageBox::critical(this, "发送失败", errorText);
        return;
    }

    appendInfoLog(QString("已发送 0x55 同步序列，长度 %1 字节").arg(count));
}

void MainWindow::handleReceived(const QByteArray &data)
{
    appendDataLog("RX", data);
}

void MainWindow::handleSent(const QByteArray &data)
{
    if (m_logTxCheck->isChecked()) {
        appendDataLog("TX", data);
    }
}

void MainWindow::handleSerialError(const QString &errorText)
{
    appendErrorLog(errorText);
}

void MainWindow::handleConnectionState(bool connected, const QString &portName)
{
    if (connected) {
        m_statusLabel->setText(QString("状态: 已连接 %1").arg(portName));
    } else {
        m_statusLabel->setText("状态: 未连接");
    }
}

void MainWindow::saveLogToFile()
{
    const QString path = QFileDialog::getSaveFileName(this,
                                                      "保存日志",
                                                      "NokiaFBusSniffer.log",
                                                      "Text Files (*.txt *.log);;All Files (*)");
    if (path.isEmpty()) {
        return;
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "保存失败", QString("无法写入文件: %1").arg(file.errorString()));
        return;
    }

    QTextStream stream(&file);
    for (const LogEntry &entry : m_entries) {
        stream << renderEntry(entry) << Qt::endl;
    }

    if (!file.commit()) {
        QMessageBox::critical(this, "保存失败", QString("提交文件失败: %1").arg(file.errorString()));
        return;
    }

    appendInfoLog(QString("日志已保存到: %1").arg(path));
}

void MainWindow::clearLog()
{
    m_entries.clear();
    refreshLogView();
    appendInfoLog("日志已清空");
}

void MainWindow::refreshLogView()
{
    QStringList lines;
    lines.reserve(m_entries.size());

    for (const LogEntry &entry : m_entries) {
        lines << renderEntry(entry);
    }

    m_logEdit->setPlainText(lines.join('\n'));

    if (m_autoScrollCheck->isChecked()) {
        auto *bar = m_logEdit->verticalScrollBar();
        bar->setValue(bar->maximum());
    }
}

void MainWindow::loadSettings()
{
    applySettingsToUi(m_settings->loadSerialSettings());
}

void MainWindow::saveSettings()
{
    m_settings->saveSerialSettings(currentSettingsFromUi());
}

SerialManager::PortSettings MainWindow::currentSettingsFromUi() const
{
    SerialManager::PortSettings settings;
    settings.portName = m_portCombo->currentData().toString();
    settings.baudRate = m_baudCombo->currentData().toInt();
    settings.dataBits = static_cast<QSerialPort::DataBits>(m_dataBitsCombo->currentData().toInt());
    settings.parity = static_cast<QSerialPort::Parity>(m_parityCombo->currentData().toInt());
    settings.stopBits = static_cast<QSerialPort::StopBits>(m_stopBitsCombo->currentData().toInt());
    settings.flowControl = static_cast<QSerialPort::FlowControl>(m_flowControlCombo->currentData().toInt());
    return settings;
}

void MainWindow::applySettingsToUi(const SerialManager::PortSettings &settings)
{
    if (!settings.portName.isEmpty()) {
        for (int i = 0; i < m_portCombo->count(); ++i) {
            if (m_portCombo->itemData(i).toString() == settings.portName) {
                m_portCombo->setCurrentIndex(i);
                break;
            }
        }
    }

    m_baudCombo->setCurrentIndex(m_baudCombo->findData(settings.baudRate));
    m_dataBitsCombo->setCurrentIndex(m_dataBitsCombo->findData(settings.dataBits));
    m_parityCombo->setCurrentIndex(m_parityCombo->findData(settings.parity));
    m_stopBitsCombo->setCurrentIndex(m_stopBitsCombo->findData(settings.stopBits));
    m_flowControlCombo->setCurrentIndex(m_flowControlCombo->findData(settings.flowControl));
}

bool MainWindow::parseHexInput(const QString &input, QByteArray *output, QString *errorText)
{
    if (!output || !errorText) {
        return false;
    }

    const QStringList parts = input.simplified().split(' ');
    QByteArray result;
    result.reserve(parts.size());

    for (const QString &part : parts) {
        bool ok = false;
        const int value = part.toInt(&ok, 16);
        if (!ok || part.size() > 2 || value < 0x00 || value > 0xFF) {
            *errorText = QString("非法 HEX 字节: '%1'，请输入形如 '55 1E 00' 的内容。")
                             .arg(part);
            return false;
        }
        result.append(static_cast<char>(value));
    }

    *output = result;
    return true;
}

void MainWindow::appendDataLog(const QString &direction, const QByteArray &data)
{
    LogEntry entry;
    entry.timestamp = LogFormatter::timestampNow();
    entry.type = EntryType::Data;
    entry.direction = direction;
    entry.data = data;
    m_entries.append(entry);
    refreshLogView();
}

void MainWindow::appendInfoLog(const QString &message)
{
    LogEntry entry;
    entry.timestamp = LogFormatter::timestampNow();
    entry.type = EntryType::Info;
    entry.text = message;
    m_entries.append(entry);
    refreshLogView();
}

void MainWindow::appendErrorLog(const QString &message)
{
    LogEntry entry;
    entry.timestamp = LogFormatter::timestampNow();
    entry.type = EntryType::Error;
    entry.text = message;
    m_entries.append(entry);
    refreshLogView();
}

QString MainWindow::renderEntry(const LogEntry &entry) const
{
    switch (entry.type) {
    case EntryType::Data: {
        return QString("[%1] %2 | HEX: %3 | ASCII: %4")
            .arg(entry.timestamp,
                 entry.direction,
                 m_showHexCheck->isChecked() ? LogFormatter::toHex(entry.data) : "(hidden)",
                 m_showAsciiCheck->isChecked() ? LogFormatter::toAscii(entry.data) : "(hidden)");
    }
    case EntryType::Info:
        return QString("[%1] INFO | %2").arg(entry.timestamp, entry.text);
    case EntryType::Error:
        return QString("[%1] ERROR | %2").arg(entry.timestamp, entry.text);
    }

    return {};
}
