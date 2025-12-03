#include "wifi_board.h"
#include "codecs/no_audio_codec.h"
#include "display/lcd_display.h"
#include "application.h"
#include "button.h"
#include "led/single_led.h"
#include "mcp_server.h"
#include "config.h"

#include <wifi_station.h>
#include <esp_log.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <driver/spi_common.h>
#include <driver/gpio.h>

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
    Button boot_button_;
    LcdDisplay* display_ = nullptr;
    bool lamp_power_ = false;  // 灯/继电器状态

    void InitializeSpi() {
        spi_bus_config_t buscfg = {};
        buscfg.mosi_io_num = DISPLAY_MOSI_PIN;
        buscfg.miso_io_num = GPIO_NUM_NC;
        buscfg.sclk_io_num = DISPLAY_CLK_PIN;
        buscfg.quadwp_io_num = GPIO_NUM_NC;
        buscfg.quadhd_io_num = GPIO_NUM_NC;
        buscfg.max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t);
        ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    }

    void InitializeLcdDisplay() {
        esp_lcd_panel_io_handle_t panel_io = nullptr;
        esp_lcd_panel_handle_t panel = nullptr;

        ESP_LOGD(TAG, "Install panel IO");
        esp_lcd_panel_io_spi_config_t io_config = {};
        io_config.cs_gpio_num = DISPLAY_CS_PIN;
        io_config.dc_gpio_num = DISPLAY_DC_PIN;
        io_config.spi_mode = DISPLAY_SPI_MODE;
        io_config.pclk_hz = 40 * 1000 * 1000;
        io_config.trans_queue_depth = 10;
        io_config.lcd_cmd_bits = 8;
        io_config.lcd_param_bits = 8;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &io_config, &panel_io));

        ESP_LOGD(TAG, "Install LCD driver");
        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = DISPLAY_RST_PIN;
        panel_config.rgb_ele_order = DISPLAY_RGB_ORDER;
        panel_config.bits_per_pixel = 16;
        ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(panel_io, &panel_config, &panel));

        esp_lcd_panel_reset(panel);
        esp_lcd_panel_init(panel);
        esp_lcd_panel_invert_color(panel, DISPLAY_INVERT_COLOR);
        esp_lcd_panel_swap_xy(panel, DISPLAY_SWAP_XY);
        esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);
        esp_lcd_panel_disp_on_off(panel, true);

        display_ = new SpiLcdDisplay(panel_io, panel,
                                    DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, 
                                    DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY);
    }

    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting && !WifiStation::GetInstance().IsConnected()) {
                ResetWifiConfiguration();
            }
            app.ToggleChatState();
        });
    }

    // 初始化灯/继电器控制（语音命令）
    void InitializeLampControl() {
        // 配置GPIO8为输出（和LED共用）
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << BUILTIN_LED_GPIO),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&io_conf);
        gpio_set_level(BUILTIN_LED_GPIO, 0);

        auto& mcp_server = McpServer::GetInstance();
        
        // 注册语音命令：获取灯状态
        mcp_server.AddTool("self.lamp.get_state", "获取灯的开关状态", 
            PropertyList(), [this](const PropertyList& properties) -> ReturnValue {
            return lamp_power_ ? "{\"power\": true}" : "{\"power\": false}";
        });

        // 注册语音命令：开灯
        mcp_server.AddTool("self.lamp.turn_on", "打开灯", 
            PropertyList(), [this](const PropertyList& properties) -> ReturnValue {
            lamp_power_ = true;
            gpio_set_level(BUILTIN_LED_GPIO, 1);
            ESP_LOGI(TAG, "Lamp turned ON");
            return true;
        });

        // 注册语音命令：关灯
        mcp_server.AddTool("self.lamp.turn_off", "关闭灯", 
            PropertyList(), [this](const PropertyList& properties) -> ReturnValue {
            lamp_power_ = false;
            gpio_set_level(BUILTIN_LED_GPIO, 0);
            ESP_LOGI(TAG, "Lamp turned OFF");
            return true;
        });

        ESP_LOGI(TAG, "Lamp control initialized on GPIO%d", BUILTIN_LED_GPIO);
    }

public:
    Esp32C3SuperMiniBoard() : boot_button_(BOOT_BUTTON_GPIO) {
        InitializeSpi();
        InitializeLcdDisplay();
        InitializeButtons();
        InitializeLampControl();  // 初始化灯控制
    }

    virtual Led* GetLed() override {
        // 返回nullptr，因为GPIO8用于灯/继电器控制
        return nullptr;
    }

    virtual Display* GetDisplay() override {
        return display_;
    }

    virtual AudioCodec* GetAudioCodec() override {
        static NoAudioCodecSimplex audio_codec(
            AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_SPK_GPIO_BCLK, AUDIO_I2S_SPK_GPIO_LRCK, AUDIO_I2S_SPK_GPIO_DOUT,
            AUDIO_I2S_MIC_GPIO_SCK, AUDIO_I2S_MIC_GPIO_WS, AUDIO_I2S_MIC_GPIO_DIN);
        return &audio_codec;
    }
};

DECLARE_BOARD(Esp32C3SuperMiniBoard);
