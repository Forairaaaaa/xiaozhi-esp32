/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "view.h"
#include <mooncake_log.h>
#include <smooth_ui_toolkit.h>
#include <smooth_lvgl.h>

using namespace view;
using namespace smooth_ui_toolkit::lvgl_cpp;

static std::string _tag             = "LauncherView";
static int _last_clicked_icon_pos_x = -1;

LauncherView::~LauncherView()
{
    _icons.clear();
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
    _panel->setBgColor(lv_color_hex(0xF6F6F6));
    lv_obj_set_scroll_snap_x(_panel->get(), LV_SCROLL_SNAP_CENTER);

    // Create icons
    int icon_x = 0;
    int icon_y = -10;
    for (const auto& props : appPorps) {
        // mclog::tagInfo(_tag, "name: {}, id: {}", props.info.name, props.appID);

        // Icon panel
        _icons.push_back(std::make_unique<Container>(_panel->get()));
        _icons.back()->setAlign(LV_ALIGN_CENTER);
        _icons.back()->setSize(160, 160);
        _icons.back()->setPos(icon_x, icon_y);
        _icons.back()->setRadius(12);

        // Icon click callback
        auto app_id = props.appID;
        auto pos_x  = icon_x;
        _icons.back()->onClick().connect([&, app_id, pos_x]() {
            _clicked_app_id          = app_id;
            _last_clicked_icon_pos_x = pos_x;
        });

        // Icon label
        _labels.push_back(std::make_unique<Label>(_panel->get()));
        // _labels.back()->setTextFont(&lv_font_montserrat_16);
        _labels.back()->setAlign(LV_ALIGN_CENTER);
        _labels.back()->setPos(icon_x, icon_y + 100);
        _labels.back()->setText(props.info.name);

        // Icon image
        if (props.info.icon != nullptr) {
            _icon_images.push_back(std::make_unique<Image>(_icons.back()->get()));
            _icon_images.back()->setSrc(props.info.icon);
            _icon_images.back()->setAlign(LV_ALIGN_CENTER);
        }

        icon_x += 196;
    }

    // Navigate to last clicked icon
    if (_last_clicked_icon_pos_x != -1) {
        // mclog::tagInfo(_tag, "navigate to last clicked icon, pos x: {}", _last_clicked_icon_pos_x);
        _panel->scrollTo(_last_clicked_icon_pos_x, 0, LV_ANIM_OFF);
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
