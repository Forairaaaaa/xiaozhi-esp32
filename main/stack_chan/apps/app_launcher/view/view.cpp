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
#include <functional>
#include <cstdint>
#include <vector>

using namespace view;
using namespace smooth_ui_toolkit;
using namespace smooth_ui_toolkit::lvgl_cpp;

/* -------------------------------------------------------------------------- */
/*                    Dynamic backgroud color handle class                    */
/* -------------------------------------------------------------------------- */
class DynamicBgColor {
public:
    std::function<void(const uint32_t& bgColor)> onBgColorChanged;

    void init(const std::vector<uint32_t>& stepColors, uint32_t stepGap)
    {
        _step_colors = stepColors;
        _step_gap    = stepGap;

        _bg_color.duration = 0.3;
        _bg_color.begin();

        jumpTo(0);
    }

    void jumpTo(int index)
    {
        if (index < 0 || index >= _step_colors.size()) {
            return;
        }
        _bg_color.teleport(_step_colors[index]);
        _current_index = index;

        if (onBgColorChanged) {
            onBgColorChanged(_step_colors[index]);
        }
    }

    void update(int scrollValue)
    {
        _last_index = _current_index;

        // Update current index
        _current_index = (scrollValue + _step_gap / 2) / _step_gap;
        if (_current_index < 0) {
            _current_index = 0;
        }
        if (_current_index >= _step_colors.size()) {
            _current_index = _step_colors.size() - 1;
        }

        // If index changed
        if (_last_index != _current_index) {
            // mclog::tagInfo(_tag, "index changed from {} to {}", _last_index, _current_index);
            _bg_color = _step_colors[_current_index];
        }

        // Update background color
        _bg_color.update();
        if (!_bg_color.done()) {
            if (onBgColorChanged) {
                onBgColorChanged(_bg_color.toHex());
            }
        }
    }

private:
    std::vector<uint32_t> _step_colors;
    int _current_index = 0;
    int _last_index    = 0;
    uint32_t _step_gap = 0;
    color::AnimateRgb_t _bg_color;
};

static std::string _tag             = "LauncherView";
static int _last_clicked_icon_pos_x = -1;
static std::unique_ptr<DynamicBgColor> _dynamic_bg_color;
static constexpr int _icon_gap = 320;

LauncherView::~LauncherView()
{
    _icon_panels.clear();
    _panel.reset();
}

void LauncherView::init(std::vector<mooncake::AppProps_t> appPorps)
{
    mclog::tagInfo(_tag, "init");

    /* ---------------------------------- Panel --------------------------------- */
    _panel = std::make_unique<Container>(lv_screen_active());
    _panel->setAlign(LV_ALIGN_CENTER);
    _panel->setSize(320, 240);
    _panel->setRadius(0);
    _panel->setBorderWidth(0);
    _panel->setScrollbarMode(LV_SCROLLBAR_MODE_OFF);
    _panel->setBgColor(lv_color_hex(0x33CC99));
    lv_obj_set_scroll_snap_x(_panel->get(), LV_SCROLL_SNAP_CENTER);

    /* ---------------------------------- Icons --------------------------------- */
    int icon_x = 0;
    int icon_y = 0;
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

        icon_x += _icon_gap;
    }

    /* ------------------------------ LR indicators ----------------------------- */
    // Scroll to nearby icon handler
    int total_icons            = appPorps.size();
    auto scroll_to_nearby_icon = [&, total_icons](int direction) {
        auto current_scroll_x = _panel->getScrollX();
        int current_index     = (current_scroll_x + _icon_gap / 2) / _icon_gap;
        int target_index      = current_index + direction;

        if (target_index >= 0 && target_index < total_icons) {
            int target_x        = target_index * _icon_gap;
            int scroll_distance = target_x - current_scroll_x;
            _panel->scrollBy(-scroll_distance, 0, LV_ANIM_ON);
        }
    };

    // Go left indicator
    _lr_indicator_panels.push_back(std::make_unique<Container>(_panel->get()));
    _lr_indicator_panels.back()->setAlign(LV_ALIGN_CENTER);
    _lr_indicator_panels.back()->setSize(52, 160);
    _lr_indicator_panels.back()->setPos(-134, 0);
    _lr_indicator_panels.back()->setBorderWidth(0);
    _lr_indicator_panels.back()->addFlag(LV_OBJ_FLAG_FLOATING);
    _lr_indicator_panels.back()->removeFlag(LV_OBJ_FLAG_SCROLLABLE);
    _lr_indicator_panels.back()->setBgOpa(0);
    _lr_indicator_panels.back()->onClick().connect([scroll_to_nearby_icon]() { scroll_to_nearby_icon(-1); });

    _lr_indicators_images.push_back(std::make_unique<Image>(_lr_indicator_panels.back()->get()));
    _lr_indicators_images.back()->setSrc(&icon_indicator_left);
    _lr_indicators_images.back()->align(LV_ALIGN_CENTER, 0, 0);

    // Go right indicator
    _lr_indicator_panels.push_back(std::make_unique<Container>(_panel->get()));
    _lr_indicator_panels.back()->setAlign(LV_ALIGN_CENTER);
    _lr_indicator_panels.back()->setSize(52, 160);
    _lr_indicator_panels.back()->setPos(134, 0);
    _lr_indicator_panels.back()->setBorderWidth(0);
    _lr_indicator_panels.back()->addFlag(LV_OBJ_FLAG_FLOATING);
    _lr_indicator_panels.back()->removeFlag(LV_OBJ_FLAG_SCROLLABLE);
    _lr_indicator_panels.back()->setBgOpa(0);
    _lr_indicator_panels.back()->onClick().connect([scroll_to_nearby_icon]() { scroll_to_nearby_icon(1); });

    _lr_indicators_images.push_back(std::make_unique<Image>(_lr_indicator_panels.back()->get()));
    _lr_indicators_images.back()->setSrc(&icon_indicator_right);
    _lr_indicators_images.back()->align(LV_ALIGN_CENTER, 0, 0);

    /* ---------------------------- Dynamic bg color ---------------------------- */
    _dynamic_bg_color = std::make_unique<DynamicBgColor>();

    std::vector<uint32_t> step_colors;
    step_colors.resize(appPorps.size());
    for (size_t i = 0; i < appPorps.size(); i++) {
        uint32_t color = 0xDADADA;
        if (appPorps[i].info.userData != nullptr) {
            color = *(uint32_t*)appPorps[i].info.userData;
        }
        step_colors[i] = color;
    }

    _dynamic_bg_color->onBgColorChanged = [&](const uint32_t& bgColor) {
        // mclog::tagInfo(_tag, "bg color changed to {:06X}", bgColor);
        _panel->setBgColor(lv_color_hex(bgColor));
    };

    _dynamic_bg_color->init(step_colors, _icon_gap);

    /* ----------------------------- History restore ---------------------------- */
    if (_last_clicked_icon_pos_x != -1) {
        // mclog::tagInfo(_tag, "navigate to last clicked icon, pos x: {}", _last_clicked_icon_pos_x);
        _panel->scrollBy(-_last_clicked_icon_pos_x, 0, LV_ANIM_OFF);
        _dynamic_bg_color->jumpTo(_last_clicked_icon_pos_x / _icon_gap);
        update_lr_indicator_edge_fade(_last_clicked_icon_pos_x);
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

    int scroll_x = _panel->getScrollX();

    // Update bg color
    _dynamic_bg_color->update(scroll_x);
    update_lr_indicator_edge_fade(scroll_x);
}

void LauncherView::update_lr_indicator_edge_fade(int scroll_x)
{
    if (scroll_x <= 0 + _icon_gap / 2) {
        _lr_indicator_panels[0]->setOpa(100);
    } else {
        _lr_indicator_panels[0]->setOpa(255);
    }
    if (scroll_x >= _icon_gap * ((int)_icon_panels.size() - 1) - _icon_gap / 2) {
        _lr_indicator_panels[1]->setOpa(100);
    } else {
        _lr_indicator_panels[1]->setOpa(255);
    }
}
