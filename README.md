# ESP32-C3 SuperMini 小智AI固件

基于 [xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) 官方固件优化，专为 ESP32-C3 SuperMini 开发板设计。

## 硬件配置

| 模块 | 型号 | 说明 |
|------|------|------|
| 主控 | ESP32-C3 SuperMini | 4MB Flash / 400KB SRAM |
| 功放 | MAX98357A | I2S数字功放 |
| 麦克风 | INMP441 | I2S数字麦克风 |
| 屏幕 | ST7789 | 240x240 SPI LCD |

## 接线图

### MAX98357A 功放
```
MAX98357A    ESP32-C3
---------    --------
VIN    -->   3.3V
GND    -->   GND
BCLK   -->   GPIO2
LRC    -->   GPIO3
DIN    -->   GPIO4
GAIN   -->   不接(默认9dB) 或 GND(12dB)
SD     -->   不接(默认启用)
```

### INMP441 麦克风
```
INMP441      ESP32-C3
-------      --------
VDD    -->   3.3V
GND    -->   GND
SCK    -->   GPIO5
WS     -->   GPIO6
SD     -->   GPIO7
L/R    -->   GND (左声道)
```

### ST7789 屏幕
```
ST7789       ESP32-C3
------       --------
VCC    -->   3.3V
GND    -->   GND
SCL    -->   GPIO1
SDA    -->   GPIO10
DC     -->   GPIO8
CS     -->   GPIO0
RST    -->   3.3V (或不接)
BLK    -->   3.3V (常亮)
```

## 安装部署

### 方法一：Flash Download Tool 烧录（推荐新手）

#### 步骤1：准备工作

1. **下载固件**：访问 [Releases](https://github.com/dakeqi/xiaozhi-esp32c3-supermini/releases) 下载最新固件压缩包
2. **下载烧录工具**：[ESP Flash Download Tool](https://www.espressif.com/zh-hans/support/download/other-tools)
3. **安装驱动**：ESP32-C3 SuperMini 使用内置 USB-Serial，一般免驱。如无法识别请安装 [CH343驱动](http://www.wch.cn/downloads/CH343SER_EXE.html)

#### 步骤2：连接设备

1. 用 USB Type-C 线连接 ESP32-C3 SuperMini 到电脑
2. 打开设备管理器，记录 COM 端口号（如 COM3）
3. 如果未识别，按住 BOOT 键再插入 USB

#### 步骤3：配置烧录工具

1. 解压并运行 `flash_download_tool_x.x.x.exe`
2. 选择芯片类型：
   - **ChipType**: ESP32-C3
   - **WorkMode**: Develop
   - **LoadMode**: USB
3. 点击 OK 进入主界面

#### 步骤4：添加固件文件

勾选并配置以下3个文件（从固件压缩包解压）：

| 文件名 | 烧录地址 |
|--------|----------|
| `bootloader.bin` | `0x0` |
| `partition-table.bin` | `0x8000` |
| `xiaozhi.bin` | `` |

**配置参数**：
- **SPI SPEED**: 40MHz
- **SPI MODE**: DIO
- **COM**: 选择你的端口（如 COM3）
- **BAUD**: 921600（速度快）或 115200（更稳定）

#### 步骤5：开始烧录

1. 点击 **ERASE** 清除Flash（首次烧录建议执行）
2. 等待擦除完成
3. 点击 **START** 开始烧录
4. 等待进度条完成，显示 "FINISH" 表示成功
5. 按 RST 键或重新插拔 USB 重启设备

#### 烧录失败排查

| 问题 | 解决方案 |
|------|----------|
| 无法连接 | 按住BOOT键再点START |
| 下载超时 | 降低波特率到115200 |
| 校验失败 | 重新下载固件文件 |
| 端口被占用 | 关闭串口监视器 |

### 方法二：源码编译

#### 环境准备

```bash
# 1. 安装 ESP-IDF v5.4+
git clone -b v5.4 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32c3
source export.sh

# 2. 克隆官方仓库
git clone --recursive https://github.com/78/xiaozhi-esp32.git
cd xiaozhi-esp32

# 3. 下载板卡配置
git clone https://github.com/dakeqi/xiaozhi-esp32c3-supermini.git board-config

# 4. 复制板卡文件
mkdir -p main/boards/esp32c3-supermini
cp board-config/config.h main/boards/esp32c3-supermini/
cp board-config/esp32c3_supermini_board.cc main/boards/esp32c3-supermini/
```

#### 编译烧录

```bash
# 设置目标芯片
idf.py set-target esp32c3

# 配置（选择 ESP32-C3 SuperMini）
idf.py menuconfig
# Xiaozhi Assistant -> Board Type -> ESP32-C3 SuperMini

# 编译
idf.py build

# 烧录
idf.py -p COM3 flash

# 查看日志
idf.py -p COM3 monitor
```

## 使用配置

### 首次配网

1. 上电后设备进入配网模式，屏幕显示配网二维码
2. 微信扫码或使用小智APP配网
3. 输入WiFi密码完成配网

### 语音唤醒

默认唤醒词：**"你好小智"**

说出唤醒词后，设备进入对话模式，可以：
- 问答对话
- 播放音乐
- 智能家居控制
- 更多功能...

### 按键操作

| 操作 | 功能 |
|------|------|
| 短按 BOOT | 手动唤醒/停止对话 |
| 长按 BOOT 5秒 | 重置WiFi配置 |

## 调试指南

### 串口日志

```bash
# Windows
idf.py -p COM3 monitor

# Linux/Mac
idf.py -p /dev/ttyUSB0 monitor
```

### 常见问题

#### 1. 没有声音
- 检查 MAX98357A 接线是否正确
- 确认 BCLK/LRC/DIN 三根线都已连接
- 检查扬声器是否正常（4Ω-8Ω）

#### 2. 麦克风无法录音
- 检查 INMP441 的 L/R 引脚是否接 GND
- 确认 SCK/WS/SD 接线正确
- 检查 VDD 是否为 3.3V

#### 3. 屏幕不亮/显示异常
- 确认 SPI 接线正确
- 检查 DC 引脚是否接 GPIO8
- 尝试调整 `DISPLAY_INVERT_COLOR` 配置

#### 4. 无法唤醒
- 确保环境安静，靠近麦克风说话
- 标准普通话发音："你-好-小-智"
- 检查串口日志是否有识别信息

#### 5. WiFi连接失败
- 确认WiFi为2.4GHz（不支持5GHz）
- 检查密码是否正确
- 长按BOOT 5秒重新配网

### 引脚占用总览

| GPIO | 功能 | 模块 |
|------|------|------|
| 0 | CS | ST7789 |
| 1 | SCLK | ST7789 |
| 2 | BCLK | MAX98357A |
| 3 | LRC | MAX98357A |
| 4 | DIN | MAX98357A |
| 5 | SCK | INMP441 |
| 6 | WS | INMP441 |
| 7 | SD | INMP441 |
| 8 | DC | ST7789 |
| 9 | BOOT | 按键 |
| 10 | MOSI | ST7789 |

## 相关链接

- [小智官方固件](https://github.com/78/xiaozhi-esp32)
- [小智官方服务](https://xiaozhi.me)
- [ESP32-C3 SuperMini 资料](https://www.cnblogs.com/wuqiyang/p/18932737)

## License

MIT License
