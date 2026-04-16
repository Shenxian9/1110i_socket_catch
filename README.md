# NokiaFBusSniffer

NokiaFBusSniffer 是一个基于 **Qt 6 + Qt Widgets + Qt SerialPort** 的 Windows 串口抓包/调试工具（第一阶段：单串口调试模式），用于研究 Nokia 1110i + CA-45 数据线场景下的串口通信行为。

## 功能概览

- 串口扫描与连接管理（含波特率/数据位/校验位/停止位/流控配置）
- RX/TX 实时日志显示（毫秒级时间戳、HEX/ASCII 双视图）
- 手动发送（HEX 或文本）
- 一键发送 `0x55` 同步序列（长度可配）
- 日志保存/清空
- 串口错误处理（断连、打开失败提示）
- 预设按钮：
  - Nokia FBUS 115200 8N1
  - Legacy 9600 8N1

## 在 Qt Creator 中构建与运行（Windows）

1. 安装 Qt 6（建议包含 `Qt SerialPort` 组件）和 MSVC 编译工具链。
2. 打开 Qt Creator，选择 **File -> Open File or Project...**。
3. 选择本项目根目录下的 `CMakeLists.txt`。
4. 在 Kit 选择页面选择一个 Qt 6 + MSVC 的 Kit（例如 *Desktop Qt 6.x MSVC2022 64bit*）。
5. 点击 **Configure Project**。
6. 配置完成后，点击左下角绿色三角按钮 **Run** 即可启动。

> 如果提示缺少 SerialPort 模块，请回到 Qt Maintenance Tool 为当前 Qt 版本补装 `Qt SerialPort`。

## 后续扩展预留

代码结构已预留后续扩展方向（第一版未实现）：

- 双串口桥接/转发模式
- 自动分帧显示
- 协议字段着色
- CSV 导出
- 关键字过滤
- FBus 帧校验辅助解析
