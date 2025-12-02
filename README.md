# ESP32-C3 SuperMini 小智AI固件

## 板卡简介

ESP32-C3 SuperMini 是一款超小尺寸（22.52mm x 18mm）的开发板，适合嵌入式AI语音交互场景。

### 硬件特性
- **芯片**: ESP32-C3FN4 (RISC-V 32位单核，160MHz)
- **Flash**: 4MB
- **RAM**: 400KB SRAM
- **尺寸**: 22.52mm × 18mm (超小型)
- **USB**: Type-C (USB Serial/JTAG)
- **WiFi**: 2.4GHz 802.11 b/g/n
- **蓝牙**: BLE 5.0
- **GPIO**: 13个可用IO口
- **板载LED**: GPIO8

## 引脚配置

### 默认引脚分配

| 功能 | GPIO | 说明 |
|------|------|------|
| 板载LED | GPIO8 | 状态指示 |
| BOOT按键 | GPIO9 | 多功能按键 |
| I2S WS | GPIO4 | 音频字时钟 |
| I2S BCK | GPIO5 | 音频位时钟 |
| I2S DIN | GPIO6 | 麦克风数据输入 |
| I2S DOUT | GPIO7 | 扬声器数据输出 |
| 功放使能 | GPIO10 | 功放PA控制 |
| WS2812 LED | GPIO2 | RGB LED（可选）|
| I2C SDA | GPIO8 | OLED显示（复用）|
| I2C SCL | GPIO9 | OLED显示（复用）|
| UART TX | GPIO21 | 扩展串口 |
| UART RX | GPIO20 | 扩展串口 |
| ADC | GPIO0 | 电池电压检测 |

## 功能特性

### ✅ 已实现
- WiFi连接和配网
- **离线语音唤醒**（“你好小智”，使用WN9S小型模型）
- 板载LED状态指示
- 按键交互（单击、长按、按住说话）
- 电源管理（自动休眠）
- I2C OLED显示支持（可选）
- WS2812 RGB LED支持（可选）
- MCP协议设备控制

### ⚠️ 受限功能
- **OTA升级**: 不支持（4MB只够single factory分区）
- **主题/表情**: 受限（assets仅有~1.5MB）

## 接线建议

### 最小系统（仅WiFi语音）
```
ESP32-C3 SuperMini
├── USB Type-C → 供电 + 调试
└── 板载LED (GPIO8) → 状态指示
```

### 标准配置（带音频）
```
ESP32-C3 + MAX98357A功放 + SPH0645麦克风
├── I2S WS (GPIO4)   → LRC
├── I2S BCK (GPIO5)  → BCLK
├── I2S DOUT (GPIO7) → DIN (功放)
├── I2S DIN (GPIO6)  → DOUT (麦克风)
└── PA使能 (GPIO10)  → SD (功放)
```

### 完整配置（音频+显示）
```
添加SSD1306 OLED (128x64, I2C)
├── SDA (GPIO8) → SDA
├── SCL (GPIO9) → SCL
├── VCC → 3.3V
└── GND → GND

注意: OLED与LED复用GPIO8，需断开LED或使用其他IO
```

## 编译和烧录

### 1. 环境准备
```bash
# 安装ESP-IDF 5.4+
# 克隆仓库
git clone https://github.com/78/xiaozhi-esp32.git
cd xiaozhi-esp32
```

### 2. 选择目标板
```bash
# 设置编译目标为ESP32-C3 SuperMini
idf.py set-target esp32c3
```

### 3. 配置板卡
```bash
idf.py menuconfig
# 选择: Board Options → esp32c3-supermini
```

### 4. 编译
```bash
idf.py build -DBOARD=esp32c3-supermini
```

### 5. 烧录
```bash
# 自动检测端口并烧录
idf.py flash

# 或指定端口
idf.py -p COM3 flash  # Windows
idf.py -p /dev/ttyACM0 flash  # Linux
```

### 6. 查看日志
```bash
idf.py monitor
```

## 使用说明

### 按键操作
- **单击**: 切换聊天状态 / 重置WiFi（未连接时）
- **按住**: 按键说话模式（需在后台启用PTT）
- **长按**: 进入配网模式

### LED指示
- **快速闪烁**: 启动中
- **慢速闪烁**: WiFi连接中  
- **常亮**: 已连接，待机
- **呼吸灯**: 就绪状态
- **闪烁**: 正在处理

### 省电模式
- 默认5分钟无活动自动进入省电
- 按任意键唤醒
- 可在`config.h`中调整超时时间

## 配置优化

### 针对4MB Flash优化
```c
// sdkconfig中已优化:
CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y
CONFIG_LWIP_MAX_SOCKETS=8
CONFIG_ESP_WIFI_STATIC_RX_BUFFER_NUM=4
```

### 低功耗优化
```c
CONFIG_PM_ENABLE=y
CONFIG_FREERTOS_USE_TICKLESS_IDLE=y
ENABLE_POWER_SAVE=true
POWER_SAVE_TIMEOUT_SEC=300
```

## 常见问题

**Q: 烧录后无反应？**
A: 检查USB驱动，SuperMini使用USB Serial/JTAG，可能需要安装CH343驱动

**Q: 无法连接WiFi？**
A: 单击BOOT键切换状态，长按进入配网模式

**Q: 没有声音？**
A: SuperMini需要外接I2S音频模块，请检查接线和音频codec配置

**Q: OLED不显示？**
A: 确认I2C地址为0x3C，SDA/SCL接线正确，注意GPIO8与LED冲突

## 硬件购买

- 淘宝搜索: ESP32-C3 SuperMini
- 价格: ¥8-15 (单片)
- 推荐配套:
  - MAX98357A I2S功放模块
  - SPH0645 I2S麦克风  
  - SSD1306 OLED 128x64

## 技术支持

- GitHub Issues: https://github.com/78/xiaozhi-esp32/issues
- QQ群: 1011329060
- 文档: https://ccnphfhqs21z.feishu.cn/wiki/F5krwD16viZoF0kKkvDcrZNYnhb
