#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

// ============================================================
// ESP32-C3 SuperMini 优化配置
// 硬件配置: MAX98357A功放 + INMP441麦克风 + ST7789屏幕
// ============================================================

// ==================== 音频配置 ====================
// MAX98357A I2S功放 + INMP441 I2S麦克风
// 使用独立I2S通道（Simplex模式）

#define AUDIO_INPUT_SAMPLE_RATE  16000
#define AUDIO_OUTPUT_SAMPLE_RATE 24000

// MAX98357A 功放引脚 (I2S TX - 通道 0)
#define AUDIO_SPKR_I2S_GPIO_BCLK    GPIO_NUM_2   // BCK
#define AUDIO_SPKR_I2S_GPIO_LRCLK   GPIO_NUM_3   // LRC/WS
#define AUDIO_SPKR_I2S_GPIO_DATA    GPIO_NUM_4   // DIN

// INMP441 麦克风引脚 (I2S RX - 通道 1)
#define AUDIO_MIC_I2S_GPIO_SCK      GPIO_NUM_5   // SCK
#define AUDIO_MIC_I2S_GPIO_WS       GPIO_NUM_6   // WS
#define AUDIO_MIC_I2S_GPIO_SD       GPIO_NUM_7   // SD (Data Out)

// ==================== ST7789 LCD 配置 ====================
// SPI接口 240x240 或 240x135

#define DISPLAY_WIDTH           240
#define DISPLAY_HEIGHT          240
#define DISPLAY_SPI_MOSI_PIN    GPIO_NUM_10  // MOSI/SDA
#define DISPLAY_SPI_SCLK_PIN    GPIO_NUM_1   // SCLK/SCL
#define DISPLAY_CS_PIN          GPIO_NUM_0   // CS
#define DISPLAY_DC_PIN          GPIO_NUM_8   // DC
#define DISPLAY_RST_PIN         GPIO_NUM_NC  // RST (可不接)
#define DISPLAY_BL_PIN          GPIO_NUM_NC  // 背光 (直接3.3V常亮)

#define DISPLAY_MIRROR_X        false
#define DISPLAY_MIRROR_Y        false
#define DISPLAY_SWAP_XY         false
#define DISPLAY_INVERT_COLOR    true
#define DISPLAY_OFFSET_X        0
#define DISPLAY_OFFSET_Y        0
#define DISPLAY_SPI_MODE        0
#define DISPLAY_SPI_SCLK_HZ     (40 * 1000 * 1000)  // 40MHz

// ==================== 按键配置 ====================
#define BOOT_BUTTON_GPIO        GPIO_NUM_9   // BOOT按键

// ==================== LED配置 ====================
// 注意: GPIO8用于LCD DC，板载LED不可用
#define BUILTIN_LED_GPIO        GPIO_NUM_NC  // 禁用板载LED

// ==================== 电源管理 ====================
#define ENABLE_POWER_SAVE       true
#define POWER_SAVE_TIMEOUT_SEC  300          // 5分钟无活动进入省电

#endif // _BOARD_CONFIG_H_
