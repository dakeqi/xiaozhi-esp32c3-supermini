#include "wifi_board.h"
#include "audio_codecs/no_audio_codec.h"
#include "display/spi_lcd_display.h"
#include "application.h"
#include "button.h"
#include "led/single_led.h"
#include "mcp_server.h"
#include "settings.h"
#include "config.h"
#include "power_save_timer.h"
#include "press_to_talk_mcp_tool.h"

#include <wifi_station.h>
#include <esp_log.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>

#define TAG "ESP32C3SuperMiniBoard"

/**
 * ESP32-C3 SuperMini 开发板配置
 * 
 * 硬件配置:
 * - MAX98357A I2S功放
 * - INMP441 I2S麦克风
 * - ST7789 SPI LCD (240x240)
 * - 4MB Flash
 * - 单按键交互
 */
class Esp32C3SuperMiniBoard : public WifiBoard {
private:
    esp_lcd_panel_io_handle_t panel_io_ = nullptr;
    esp_lcd_panel_handle_t panel_ = nullptr;
    Display* display_ = nullptr;
    Button boot_button_;
    PowerSaveTimer* power_save_timer_ = nullptr;
    PressToTalkMcpTool* press_to_talk_tool_ = nullptr;

    void InitializePowerSaveTimer() {
#if ENABLE_POWER_SAVE
        power_save_timer_ = new PowerSaveTimer(60, POWER_SAVE_TIMEOUT_SEC);
        power_save_timer_->OnEnterSleepMode([this]() {
            ESP_LOGI(TAG, "进入省电模式");
            if (display_) {
                GetDisplay()->SetPowerSaveMode(true);
            }
        });
        power_save_timer_->OnExitSleepMode([this]() {
            ESP_LOGI(TAG, "退出省电模式");
            if (display_) {
                GetDisplay()->SetPowerSaveMode(false);
            }
        });
        power_save_timer_->SetEnabled(true);
#endif
    }

    void InitializeSpi() {
        spi_bus_config_t buscfg = {};
        buscfg.mosi_io_num = DISPLAY_SPI_MOSI_PIN;
        buscfg.miso_io_num = GPIO_NUM_NC;
        buscfg.sclk_io_num = DISPLAY_SPI_SCLK_PIN;
        buscfg.quadwp_io_num = GPIO_NUM_NC;
        buscfg.quadhd_io_num = GPIO_NUM_NC;
        buscfg.max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t);
        ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
        ESP_LOGI(TAG, "SPI总线初始化完成");
    }

    void InitializeSt7789Display() {
        // LCD控制IO初始化
        esp_lcd_panel_io_spi_config_t io_config = {};
        io_config.cs_gpio_num = DISPLAY_CS_PIN;
        io_config.dc_gpio_num = DISPLAY_DC_PIN;
        io_config.spi_mode = DISPLAY_SPI_MODE;
        io_config.pclk_hz = DISPLAY_SPI_SCLK_HZ;
        io_config.trans_queue_depth = 10;
        io_config.lcd_cmd_bits = 8;
        io_config.lcd_param_bits = 8;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &io_config, &panel_io_));

        // 初始化ST7789驱动
        ESP_LOGI(TAG, "安装ST7789驱动");
        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = DISPLAY_RST_PIN;
        panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
        panel_config.bits_per_pixel = 16;
        ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(panel_io_, &panel_config, &panel_));
        
        ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_));
        ESP_ERROR_CHECK(esp_lcd_panel_init(panel_));
        ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_, DISPLAY_INVERT_COLOR));
        ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_, DISPLAY_SWAP_XY));
        ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y));
        ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_, true));

        display_ = new SpiLcdDisplay(panel_io_, panel_,
                                     DISPLAY_WIDTH, DISPLAY_HEIGHT, 
                                     DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, 
                                     DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY);
        ESP_LOGI(TAG, "ST7789显示屏初始化完成 (%dx%d)", DISPLAY_WIDTH, DISPLAY_HEIGHT);
    }

    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting && 
                !WifiStation::GetInstance().IsConnected()) {
                ResetWifiConfiguration();
            }
            if (!press_to_talk_tool_ || !press_to_talk_tool_->IsPressToTalkEnabled()) {
                app.ToggleChatState();
            }
        });

        boot_button_.OnPressDown([this]() {
            if (power_save_timer_) {
                power_save_timer_->WakeUp();
            }
            if (press_to_talk_tool_ && press_to_talk_tool_->IsPressToTalkEnabled()) {
                Application::GetInstance().StartListening();
            }
        });

        boot_button_.OnPressUp([this]() {
            if (press_to_talk_tool_ && press_to_talk_tool_->IsPressToTalkEnabled()) {
                Application::GetInstance().StopListening();
            }
        });
    }

    void InitializeTools() {
        press_to_talk_tool_ = new PressToTalkMcpTool();
        press_to_talk_tool_->Initialize();
    }

public:
    Esp32C3SuperMiniBoard() : boot_button_(BOOT_BUTTON_GPIO) {
        ESP_LOGI(TAG, "========================================");
        ESP_LOGI(TAG, "小智AI - ESP32-C3 SuperMini");
        ESP_LOGI(TAG, "MAX98357A + INMP441 + ST7789");
        ESP_LOGI(TAG, "========================================");

        InitializeSpi();
        InitializeSt7789Display();
        InitializeButtons();
        InitializePowerSaveTimer();
        InitializeTools();

        ESP_LOGI(TAG, "板卡初始化完成");
    }

    virtual Led* GetLed() override {
        // 板载LED被LCD DC占用，返回空实现
        static SingleLed led(GPIO_NUM_NC);
        return &led;
    }

    virtual Display* GetDisplay() override {
        return display_;
    }

    virtual AudioCodec* GetAudioCodec() override {
        // 使用NoAudioCodecSimplex: MAX98357A + INMP441
        // 独立I2S通道，扬声器和麦克风分开
        static NoAudioCodecSimplex audio_codec(
            AUDIO_INPUT_SAMPLE_RATE,        // 麦克风采样率
            AUDIO_OUTPUT_SAMPLE_RATE,       // 扬声器采样率
            AUDIO_SPKR_I2S_GPIO_BCLK,       // 扬声器 BCLK
            AUDIO_SPKR_I2S_GPIO_LRCLK,      // 扬声器 LRC
            AUDIO_SPKR_I2S_GPIO_DATA,       // 扬声器 DIN
            AUDIO_MIC_I2S_GPIO_SCK,         // 麦克风 SCK
            AUDIO_MIC_I2S_GPIO_WS,          // 麦克风 WS
            AUDIO_MIC_I2S_GPIO_SD           // 麦克风 SD
        );
        return &audio_codec;
    }

    virtual void SetPowerSaveMode(bool enabled) override {
        if (!enabled && power_save_timer_) {
            power_save_timer_->WakeUp();
        }
        WifiBoard::SetPowerSaveMode(enabled);
    }
};

DECLARE_BOARD(Esp32C3SuperMiniBoard);
