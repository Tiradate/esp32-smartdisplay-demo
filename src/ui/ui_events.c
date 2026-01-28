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

void ui_event_GotoScreen2(lv_event_t * e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_scr_load_anim(ui_Screen2, LV_SCR_LOAD_ANIM_MOVE_LEFT, 200, 0, false);
    }
}

void ui_event_GotoScreen1(lv_event_t * e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
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
