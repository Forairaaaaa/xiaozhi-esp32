/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "hal.h"
#include <memory>
#include <mooncake_log.h>

static std::unique_ptr<Hal> _hal_instance;
static const std::string _tag = "HAL";

Hal& GetHAL()
{
    if (!_hal_instance) {
        mclog::tagInfo(_tag, "creating hal instance");
        _hal_instance = std::make_unique<Hal>();
    }
    return *_hal_instance.get();
}

void Hal::init()
{
    mclog::tagInfo(_tag, "init");

    xiaozhi_board_init();
}

/* -------------------------------------------------------------------------- */
/*                                   System                                   */
/* -------------------------------------------------------------------------- */
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>

void Hal::delay(std::uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

std::uint32_t Hal::millis()
{
    return esp_timer_get_time() / 1000;
}

void Hal::feedTheDog()
{
    vTaskDelay(1);
}

/* -------------------------------------------------------------------------- */
/*                                   Xiaozhi                                  */
/* -------------------------------------------------------------------------- */
#include <esp_log.h>
#include <esp_err.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <esp_event.h>
#include <board.h>

void Hal::xiaozhi_board_init()
{
    mclog::tagInfo(_tag, "xiaozhi board init");

    // Initialize the default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize NVS flash for WiFi configuration
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        mclog::tagWarn(_tag, "erasing NVS flash to fix corruption");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Init board
    auto& board = Board::GetInstance();
}

/* -------------------------------------------------------------------------- */
/*                                   Display                                  */
/* -------------------------------------------------------------------------- */
#include <board.h>
#include <display.h>
#include <boards/m5stack-stack-chan/stack_chan_display.h>

void Hal::lvglLock()
{
    auto display = static_cast<StackChanLcdDisplay*>(Board::GetInstance().GetDisplay());
    display->LvglLock();
}

void Hal::lvglUnlock()
{
    auto display = static_cast<StackChanLcdDisplay*>(Board::GetInstance().GetDisplay());
    display->LvglUnlock();
}
