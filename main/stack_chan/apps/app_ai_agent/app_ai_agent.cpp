/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "app_ai_agent.h"
#include <hal/hal.h>
#include <mooncake.h>
#include <mooncake_log.h>
#include <assets/assets.h>
#include <smooth_lvgl.h>

using namespace mooncake;
using namespace smooth_ui_toolkit::lvgl_cpp;

AppAiAgent::AppAiAgent()
{
    // 配置 App 名
    setAppInfo().name = "AI.AGENT";
    // 配置 App 图标
    setAppInfo().icon = (void*)&icon_ai_agent;
    // 配置 App 主题颜色
    static uint32_t theme_color = 0x33CC99;
    setAppInfo().userData       = (void*)&theme_color;
}

// App 被安装时会被调用
void AppAiAgent::onCreate()
{
    mclog::tagInfo(getAppInfo().name, "on create");
}

// App 被打开时会被调用
// 可以在这里构造 UI，初始化操作等
void AppAiAgent::onOpen()
{
    mclog::tagInfo(getAppInfo().name, "on open");

    GetHAL().startXiaozhi();
}

// App 运行时会一直被调用，不建议在这里阻塞操作
void AppAiAgent::onRunning()
{
}

// App 被关闭时会被调用
// 可以在这里销毁 UI，释放资源等
void AppAiAgent::onClose()
{
    mclog::tagInfo(getAppInfo().name, "on close");
}
