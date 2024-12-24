#include "wifi_board.h"
#include "audio_codecs/cores3_audio_codec.h"
#include "display/no_display.h"
#include "application.h"
#include "button.h"
#include "led.h"
#include "config.h"
#include "iot/thing_manager.h"

#include <esp_log.h>
#include <driver/i2c_master.h>

#define TAG "M5StackCoreS3Board"

class M5StackCoreS3Board : public WifiBoard {
private:
    i2c_master_bus_handle_t i2c_bus_;
    Button boot_button_;

    void InitializeI2c() {
        // Initialize I2C peripheral
        i2c_master_bus_config_t i2c_bus_cfg = {
            .i2c_port = (i2c_port_t)1,
            .sda_io_num = I2C_SDA_PIN,
            .scl_io_num = I2C_SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = {
                .enable_internal_pullup = 1,
            },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &i2c_bus_));
    }

    void I2cDetect() {
        uint8_t address;
        printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");
        for (int i = 0; i < 128; i += 16) {
            printf("%02x: ", i);
            for (int j = 0; j < 16; j++) {
                fflush(stdout);
                address = i + j;
                esp_err_t ret = i2c_master_probe(i2c_bus_, address, pdMS_TO_TICKS(200));
                if (ret == ESP_OK) {
                    printf("%02x ", address);
                } else if (ret == ESP_ERR_TIMEOUT) {
                    printf("UU ");
                } else {
                    printf("-- ");
                }
            }
            printf("\r\n");
        }
    }

    esp_err_t i2c_dev_write_reg_8(i2c_master_dev_handle_t i2c_dev, uint8_t reg, uint8_t data) {
        uint8_t w_buf[2];
        w_buf[0] = reg;
        w_buf[1] = data;
        auto ret = i2c_master_transmit(i2c_dev, w_buf, 2, portMAX_DELAY);
        ESP_ERROR_CHECK(ret);
        return ret;
    }

    uint8_t i2c_dev_read_reg_8(i2c_master_dev_handle_t i2c_dev, uint8_t reg) {
        uint8_t w_buf[1];
        uint8_t r_buf[1];
        w_buf[0] = reg;
        ESP_ERROR_CHECK(i2c_master_transmit_receive(i2c_dev, w_buf, 1, r_buf, 1, portMAX_DELAY));
        return r_buf[0];
    }
    
    i2c_master_dev_handle_t axp2101_handle_;

    void InitializeAxp2101() {
        ESP_LOGI(TAG, "Init AXP2101");

        i2c_device_config_t axp2101_config = {};
        axp2101_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
        axp2101_config.device_address = 0x34;
        axp2101_config.scl_speed_hz = 400000;
        i2c_master_bus_add_device(i2c_bus_, &axp2101_config, &axp2101_handle_);

        uint8_t data = i2c_dev_read_reg_8(axp2101_handle_, 0x90);
        data |= 0b10110100;
        i2c_dev_write_reg_8(axp2101_handle_, 0x90, data);
        i2c_dev_write_reg_8(axp2101_handle_, 0x99, (0b11110 - 5));
        i2c_dev_write_reg_8(axp2101_handle_, 0x97, (0b11110 - 2));
        i2c_dev_write_reg_8(axp2101_handle_, 0x69, 0b00110101);
        i2c_dev_write_reg_8(axp2101_handle_, 0x30, 0b111111);

        i2c_dev_write_reg_8(axp2101_handle_, 0x90, 0xBF);
        i2c_dev_write_reg_8(axp2101_handle_, 0x94, 33 - 5);
        i2c_dev_write_reg_8(axp2101_handle_, 0x95, 33 - 5);
    }

    i2c_master_dev_handle_t aw9523_handle_;

    void aw88298_reset() {
        ESP_LOGI(TAG, "Reset AW88298");
        i2c_dev_write_reg_8(aw9523_handle_, 0x02, 0b00000011);
        vTaskDelay(pdMS_TO_TICKS(10));
        i2c_dev_write_reg_8(aw9523_handle_, 0x02, 0b00000111);
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    void InitializeAw9523() {
        ESP_LOGI(TAG, "Init AW9523");

        i2c_device_config_t aw9523_config = {};
        aw9523_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
        aw9523_config.device_address = 0x58;
        aw9523_config.scl_speed_hz = 400000;
        i2c_master_bus_add_device(i2c_bus_, &aw9523_config, &aw9523_handle_);

        // i2c_dev_write_reg_8(aw9523_handle_, 0x02, 0b00000111);  // P0
        // i2c_dev_write_reg_8(aw9523_handle_, 0x03, 0b10000011);  // P1
        i2c_dev_write_reg_8(aw9523_handle_, 0x02, 0b00000111);  // P0
        i2c_dev_write_reg_8(aw9523_handle_, 0x03, 0b10000011);  // P1
        i2c_dev_write_reg_8(aw9523_handle_, 0x04, 0b00011000);  // CONFIG_P0
        i2c_dev_write_reg_8(aw9523_handle_, 0x05, 0b00001100);  // CONFIG_P1
        i2c_dev_write_reg_8(aw9523_handle_, 0x11, 0b00010000);  // GCR P0 port is Push-Pull mode.
        i2c_dev_write_reg_8(aw9523_handle_, 0x12, 0b11111111);  // LEDMODE_P0
        i2c_dev_write_reg_8(aw9523_handle_, 0x13, 0b11111111);  // LEDMODE_P1

        vTaskDelay(pdMS_TO_TICKS(50));
    }

    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            Application::GetInstance().ToggleChatState();
        });
    }

    // 物联网初始化，添加对 AI 可见设备
    void InitializeIot() {
        auto& thing_manager = iot::ThingManager::GetInstance();
        thing_manager.AddThing(iot::CreateThing("Speaker"));
    }

public:
    M5StackCoreS3Board() : boot_button_(GPIO_NUM_1) {
    // M5StackCoreS3Board() {
        InitializeI2c();
        InitializeAxp2101();
        InitializeAw9523();
        I2cDetect();
        InitializeButtons();
        InitializeIot();
    }

    virtual Led* GetBuiltinLed() override {
        static Led led(GPIO_NUM_NC);
        return &led;
    }

    virtual AudioCodec* GetAudioCodec() override {
        static CoreS3AudioCodec* audio_codec = nullptr;
        if (audio_codec == nullptr) {
            aw88298_reset();
            audio_codec = new CoreS3AudioCodec(i2c_bus_, AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
                AUDIO_I2S_GPIO_MCLK, AUDIO_I2S_GPIO_BCLK, AUDIO_I2S_GPIO_WS, AUDIO_I2S_GPIO_DOUT, AUDIO_I2S_GPIO_DIN,
                AUDIO_CODEC_AW88298_ADDR, AUDIO_CODEC_ES7210_ADDR, AUDIO_INPUT_REFERENCE);
            audio_codec->SetOutputVolume(AUDIO_DEFAULT_OUTPUT_VOLUME);
        }
        return audio_codec;
    }

    virtual Display* GetDisplay() override {
        static NoDisplay display;
        return &display;
    }
};

DECLARE_BOARD(M5StackCoreS3Board);
