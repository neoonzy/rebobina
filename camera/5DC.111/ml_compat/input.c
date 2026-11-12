/* SPDX-License-Identifier: GPL-2.0-only */
/* Rebobina adaptation of Magic Lantern 5DC.111 input codes; see input.h. */
#include "input.h"
int rb_5dc_ml_decode_button(uint32_t type, uint32_t param, rb_ui_button *out)
{
    rb_ui_button value;
    if (!out || type != RB_5DC_ML_BUTTON_EVENT_TYPE) return 0;
    switch (param) {
    case RB_5DC_ML_TRASH: value = RB_UI_TRASH; break;
    case RB_5DC_ML_WHEEL_UP:
    case RB_5DC_ML_PRESS_UP: value = RB_UI_UP; break;
    case RB_5DC_ML_WHEEL_DOWN:
    case RB_5DC_ML_PRESS_DOWN: value = RB_UI_DOWN; break;
    case RB_5DC_ML_PRESS_SET: value = RB_UI_SET; break;
    case RB_5DC_ML_MENU: value = RB_UI_BACK; break;
    default: return 0;
    }
    *out = value;
    return 1;
}
