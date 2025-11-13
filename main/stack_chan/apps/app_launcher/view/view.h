/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <mooncake.h>
#include <smooth_ui_toolkit.h>
#include <smooth_lvgl.h>
#include <functional>
#include <vector>
#include <memory>
#include "animation/animate_value/animate_value.h"

namespace view {

class LauncherView {
public:
    ~LauncherView();

    enum State_t {
        STATE_STARTUP,
        STATE_NORMAL,
    };

    std::function<void(int appID)> onAppClicked;

    void init(std::vector<mooncake::AppProps_t> appPorps);
    void update();

private:
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _panel;
    std::vector<std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container>> _icon_panels;
    std::vector<std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Image>> _icon_images;
    std::vector<std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container>> _lr_indicator_panels;
    std::vector<std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Image>> _lr_indicators_images;

    std::unique_ptr<smooth_ui_toolkit::AnimateVector2> _startup_anim;

    int _clicked_app_id = -1;
    State_t _state      = STATE_STARTUP;

    void handle_state_startup();
    void handle_state_normal();
    void update_lr_indicator_edge_fade(int scroll_x);
};

}  // namespace view
