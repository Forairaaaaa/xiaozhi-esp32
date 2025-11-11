/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <mooncake.h>
#include <cstdint>

/**
 * @brief 派生 App
 *
 */
class AppDummy : public mooncake::AppAbility {
public:
    AppDummy();

    // 重写生命周期回调
    void onCreate() override;
    void onOpen() override;
    void onRunning() override;
    void onClose() override;

private:
    uint32_t _time_count = 0;
};
