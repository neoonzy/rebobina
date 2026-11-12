/* SPDX-License-Identifier: GPL-2.0-only */
/* Magic Lantern Team platform/5DC.111/gui.h, 2013, GPL v2.
 * Adapted for Rebobina on 2026-11-12. Source:
 * https://github.com/frantony/magic-lantern/blob/a3454ebed3f6adcd165ee9382f451f917d989253/platform/5DC.111/gui.h
 * Historical reports, not yet verified on physical camera. */
#ifndef RB_5DC_ML_INPUT_H
#define RB_5DC_ML_INPUT_H
#include <stdint.h>
#include "rebobina/ui.h"
#define RB_5DC_ML_BUTTON_EVENT_TYPE 0u
#define RB_5DC_ML_WHEEL_UP 0x0u
#define RB_5DC_ML_WHEEL_DOWN 0x1u
#define RB_5DC_ML_PRESS_SET 0x2u
#define RB_5DC_ML_MENU 0x3u
#define RB_5DC_ML_TRASH 0x7u
#define RB_5DC_ML_PRESS_UP 0xFu
#define RB_5DC_ML_PRESS_DOWN 0x16u
int rb_5dc_ml_decode_button(uint32_t type, uint32_t param, rb_ui_button *out);
#endif
