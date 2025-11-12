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

namespace view {

class LauncherView {
public:
    ~LauncherView();

    std::function<void(int appID)> onAppClicked;

    void init(std::vector<mooncake::AppProps_t> appPorps);
    void update();

private:
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _panel;
    std::vector<std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container>> _icon_panels;
    std::vector<std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Image>> _icon_images;
    std::vector<std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Label>> _icon_labels;
    std::vector<std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container>> _lr_indicator_panels;
    std::vector<std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Image>> _lr_indicators_images;

    int _clicked_app_id = -1;

    void update_lr_indicator_edge_fade(int scroll_x);
};

}  // namespace view
