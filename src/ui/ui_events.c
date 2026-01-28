// Custom UI event handlers.
#include "ui.h"

#ifdef __cplusplus
extern "C" {
#endif

void ui_adjust_temp_offset(float delta);
void ui_adjust_humid_offset(float delta);

#ifdef __cplusplus
}
#endif

void ui_event_Screen1(lv_event_t * e)
{
    static lv_point_t swipe_start;
    static bool swipe_tracking = false;
    lv_dir_t dir = LV_DIR_NONE;
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_GESTURE) {
        dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    } else if (code == LV_EVENT_PRESSED) {
        lv_indev_t * indev = lv_indev_get_act();
        if (indev) {
            lv_indev_get_point(indev, &swipe_start);
            swipe_tracking = true;
        }
        return;
    } else if (code == LV_EVENT_RELEASED && swipe_tracking) {
        lv_point_t swipe_end;
        lv_indev_t * indev = lv_indev_get_act();
        swipe_tracking = false;
        if (!indev) {
            return;
        }
        lv_indev_get_point(indev, &swipe_end);

        int16_t dx = swipe_end.x - swipe_start.x;
        int16_t dy = swipe_end.y - swipe_start.y;
        if (LV_ABS(dx) >= 30 && LV_ABS(dx) > LV_ABS(dy)) {
            dir = (dx > 0) ? LV_DIR_RIGHT : LV_DIR_LEFT;
        }
    } else {
        return;
    }

    if (dir == LV_DIR_LEFT) {
        lv_scr_load_anim(ui_Screen2, LV_SCR_LOAD_ANIM_MOVE_LEFT, 200, 0, false);
    }
}

void ui_event_Screen2(lv_event_t * e)
{
    static lv_point_t swipe_start;
    static bool swipe_tracking = false;
    lv_dir_t dir = LV_DIR_NONE;
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_GESTURE) {
        dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    } else if (code == LV_EVENT_PRESSED) {
        lv_indev_t * indev = lv_indev_get_act();
        if (indev) {
            lv_indev_get_point(indev, &swipe_start);
            swipe_tracking = true;
        }
        return;
    } else if (code == LV_EVENT_RELEASED && swipe_tracking) {
        lv_point_t swipe_end;
        lv_indev_t * indev = lv_indev_get_act();
        swipe_tracking = false;
        if (!indev) {
            return;
        }
        lv_indev_get_point(indev, &swipe_end);

        int16_t dx = swipe_end.x - swipe_start.x;
        int16_t dy = swipe_end.y - swipe_start.y;
        if (LV_ABS(dx) >= 30 && LV_ABS(dx) > LV_ABS(dy)) {
            dir = (dx > 0) ? LV_DIR_RIGHT : LV_DIR_LEFT;
        }
    } else {
        return;
    }

    if (dir == LV_DIR_RIGHT) {
        lv_scr_load_anim(ui_Screen1, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 200, 0, false);
    }
}

void ui_event_TempOffsetMinus(lv_event_t * e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        ui_adjust_temp_offset(-0.1f);
    }
}

void ui_event_TempOffsetPlus(lv_event_t * e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        ui_adjust_temp_offset(0.1f);
    }
}

void ui_event_HumidOffsetMinus(lv_event_t * e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        ui_adjust_humid_offset(-0.1f);
    }
}

void ui_event_HumidOffsetPlus(lv_event_t * e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        ui_adjust_humid_offset(0.1f);
    }
}
