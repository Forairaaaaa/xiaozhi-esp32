/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "view.h"
#include <mooncake_log.h>
#include <smooth_ui_toolkit.h>
#include <smooth_lvgl.h>
#include <assets/assets.h>
#include <src/core/lv_obj.h>

using namespace view;
using namespace smooth_ui_toolkit::lvgl_cpp;

static std::string _tag             = "LauncherView";
static int _last_clicked_icon_pos_x = -1;

LauncherView::~LauncherView()
{
    _icon_panels.clear();
    _panel.reset();
}

void LauncherView::init(std::vector<mooncake::AppProps_t> appPorps)
{
    mclog::tagInfo(_tag, "init");

    // Create panel
    _panel = std::make_unique<Container>(lv_screen_active());
    _panel->setAlign(LV_ALIGN_CENTER);
    _panel->setSize(320, 240);
    _panel->setRadius(0);
    _panel->setBorderWidth(0);
    _panel->setScrollbarMode(LV_SCROLLBAR_MODE_OFF);
    _panel->setBgColor(lv_color_hex(0xFF6699));
    lv_obj_set_scroll_snap_x(_panel->get(), LV_SCROLL_SNAP_CENTER);

    // Create icons
    int icon_x   = 0;
    int icon_y   = 0;
    int icon_gap = 320;
    for (const auto& props : appPorps) {
        // mclog::tagInfo(_tag, "name: {}, id: {}", props.info.name, props.appID);

        // Icon panel
        _icon_panels.push_back(std::make_unique<Container>(_panel->get()));
        _icon_panels.back()->setAlign(LV_ALIGN_CENTER);
        _icon_panels.back()->setSize(190, 160);
        _icon_panels.back()->setPos(icon_x, icon_y);
        _icon_panels.back()->setBorderWidth(0);
        _icon_panels.back()->removeFlag(LV_OBJ_FLAG_SCROLLABLE);
        _icon_panels.back()->setBgOpa(0);

        // Icon click callback
        auto app_id = props.appID;
        auto pos_x  = icon_x;
        _icon_panels.back()->onClick().connect([&, app_id, pos_x]() {
            _clicked_app_id          = app_id;
            _last_clicked_icon_pos_x = pos_x;
        });

        // Icon label
        _labels.push_back(std::make_unique<Label>(_panel->get()));
        _labels.back()->setTextColor(lv_color_hex(0x000000));
        _labels.back()->setTextFont(&MontserratSemiBold26);
        _labels.back()->setAlign(LV_ALIGN_CENTER);
        _labels.back()->setPos(icon_x, icon_y - 99);
        _labels.back()->setText(props.info.name);

        // Icon image
        if (props.info.icon != nullptr) {
            _icon_images.push_back(std::make_unique<Image>(_icon_panels.back()->get()));
            _icon_images.back()->setSrc(props.info.icon);
            _icon_images.back()->setAlign(LV_ALIGN_CENTER);
        }

        icon_x += icon_gap;
    }

    // Capture icon_gap and total icon count for indicator callbacks
    int total_icons = appPorps.size();

    // Create left and right indicators
    auto scroll_to_nearby_icon = [&, icon_gap, total_icons](int direction) {
        auto current_scroll_x = _panel->getScrollX();
        int current_index     = (current_scroll_x + icon_gap / 2) / icon_gap;
        int target_index      = current_index + direction;

        if (target_index >= 0 && target_index < total_icons) {
            int target_x        = target_index * icon_gap;
            int scroll_distance = target_x - current_scroll_x;
            _panel->scrollBy(-scroll_distance, 0, LV_ANIM_ON);
        }
    };

    _lr_indicators.push_back(std::make_unique<Image>(_panel->get()));
    _lr_indicators.back()->setSrc(&icon_indicator_left);
    _lr_indicators.back()->align(LV_ALIGN_CENTER, -127, 0);
    _lr_indicators.back()->addFlag(LV_OBJ_FLAG_FLOATING);
    _lr_indicators.back()->addFlag(LV_OBJ_FLAG_CLICKABLE);
    _lr_indicators.back()->onClick().connect([scroll_to_nearby_icon]() { scroll_to_nearby_icon(-1); });

    _lr_indicators.push_back(std::make_unique<Image>(_panel->get()));
    _lr_indicators.back()->setSrc(&icon_indicator_right);
    _lr_indicators.back()->align(LV_ALIGN_CENTER, 127, 0);
    _lr_indicators.back()->addFlag(LV_OBJ_FLAG_FLOATING);
    _lr_indicators.back()->addFlag(LV_OBJ_FLAG_CLICKABLE);
    _lr_indicators.back()->onClick().connect([scroll_to_nearby_icon]() { scroll_to_nearby_icon(1); });

    // Navigate to last clicked icon
    if (_last_clicked_icon_pos_x != -1) {
        // mclog::tagInfo(_tag, "navigate to last clicked icon, pos x: {}", _last_clicked_icon_pos_x);
        _panel->scrollBy(-_last_clicked_icon_pos_x, 0, LV_ANIM_OFF);
        _last_clicked_icon_pos_x = -1;
    }
}

void LauncherView::update()
{
    if (_clicked_app_id != -1) {
        if (onAppClicked) {
            onAppClicked(_clicked_app_id);
        }
        _clicked_app_id = -1;
    }
}
