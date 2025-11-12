/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "app_sentinel.h"
#include <hal/hal.h>
#include <mooncake.h>
#include <mooncake_log.h>
#include <assets/assets.h>
#include <smooth_lvgl.h>

using namespace mooncake;
using namespace smooth_ui_toolkit::lvgl_cpp;

AppSentinel::AppSentinel()
{
    // 配置 App 名
    setAppInfo().name = "SENTINEL";
    // 配置 App 图标
    setAppInfo().icon = (void*)&icon_sentinel;
    // 配置 App 主题颜色
    static uint32_t theme_color = 0xFF6699;
    setAppInfo().userData       = (void*)&theme_color;
}

// App 被安装时会被调用
void AppSentinel::onCreate()
{
    mclog::tagInfo(getAppInfo().name, "on create");
}

static std::unique_ptr<Button> _button_quit;
static uint32_t _time_count = 0;

// App 被打开时会被调用
// 可以在这里构造 UI，初始化操作等
void AppSentinel::onOpen()
{
    mclog::tagInfo(getAppInfo().name, "on open");

    // Lvgl 操作前要先上锁，避免和 lvgl 线程冲突
    // GetHAL().lvglLock(); 上锁，GetHAL().lvglUnlock(); 解锁
    // 也可以使用 LvglLockGuard 自动上锁和解锁
    LvglLockGuard lock;

    // 创建一个退出按钮
    // 这里使用的是 lvgl 的 cpp 封装
    // 和直接使用 c 方法是一样的, lv_button_create...
    _button_quit = std::make_unique<Button>(lv_screen_active());
    _button_quit->setAlign(LV_ALIGN_CENTER);
    _button_quit->label().setText("QUIT");
    _button_quit->onClick().connect([this]() {
        // 调用 close() 可以关闭 App
        close();
    });
}

// App 运行时会一直被调用，不建议在这里阻塞操作
void AppSentinel::onRunning()
{
    // mclog::tagInfo(getAppInfo().name, "on running");

    // 每隔 1 秒打印一次 "hi"
    if (GetHAL().millis() - _time_count > 1000) {
        mclog::tagInfo(getAppInfo().name, "hi");
        _time_count = GetHAL().millis();
    }
}

// App 被关闭时会被调用
// 可以在这里销毁 UI，释放资源等
void AppSentinel::onClose()
{
    mclog::tagInfo(getAppInfo().name, "on close");

    LvglLockGuard lock;

    // 销毁按钮
    _button_quit.reset();
}
