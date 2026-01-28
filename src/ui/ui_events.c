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

typedef struct {
    uint32_t press_start;
    bool triggered;
} ui_long_press_state_t;

static ui_long_press_state_t screen1_long_press = {0};
static ui_long_press_state_t screen2_long_press = {0};

static void ui_handle_long_press(lv_event_t * e,
                                 ui_long_press_state_t * state,
                                 lv_obj_t * target_screen,
                                 lv_scr_load_anim_t anim)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_PRESSED) {
        state->press_start = lv_tick_get();
        state->triggered = false;
    } else if (code == LV_EVENT_PRESSING) {
        if (!state->triggered && lv_tick_elaps(state->press_start) >= 3000) {
            state->triggered = true;
            lv_scr_load_anim(target_screen, anim, 200, 0, false);
        }
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        state->press_start = 0;
        state->triggered = false;
    }
}

void ui_event_GotoScreen2(lv_event_t * e)
{
    ui_handle_long_press(e, &screen1_long_press, ui_Screen2, LV_SCR_LOAD_ANIM_MOVE_LEFT);
}

void ui_event_GotoScreen1(lv_event_t * e)
{
    ui_handle_long_press(e, &screen2_long_press, ui_Screen1, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
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
