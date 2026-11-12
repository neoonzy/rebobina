/* SPDX-License-Identifier: GPL-2.0-only */
/* Host-only translation test; NO camera event dispatch or ROM access. */
#include "input.h"
#include <assert.h>
#include <stdio.h>
static void maps(unsigned code, rb_ui_button expected)
{
    rb_ui_button output = RB_UI_BACK;
    assert(rb_5dc_ml_decode_button(0, code, &output) == 1);
    assert(output == expected);
}
int main(void)
{
    rb_ui_button output = RB_UI_BACK;
    rb_ui ui;
    maps(RB_5DC_ML_TRASH, RB_UI_TRASH);
    maps(RB_5DC_ML_PRESS_SET, RB_UI_SET);
    maps(RB_5DC_ML_MENU, RB_UI_BACK);
    maps(RB_5DC_ML_WHEEL_UP, RB_UI_UP);
    maps(RB_5DC_ML_WHEEL_DOWN, RB_UI_DOWN);
    maps(RB_5DC_ML_PRESS_UP, RB_UI_UP);
    maps(RB_5DC_ML_PRESS_DOWN, RB_UI_DOWN);
    assert(!rb_5dc_ml_decode_button(1, RB_5DC_ML_TRASH, &output));
    assert(output == RB_UI_BACK);
    assert(!rb_5dc_ml_decode_button(0, 0x8u, &output));
    assert(output == RB_UI_BACK);
    assert(!rb_5dc_ml_decode_button(0, RB_5DC_ML_TRASH, NULL));
    rb_ui_init(&ui);
    assert(rb_5dc_ml_decode_button(0, RB_5DC_ML_TRASH, &output));
    rb_ui_event(&ui, output);
    assert(ui.opened == 1);
    assert(rb_5dc_ml_decode_button(0, RB_5DC_ML_WHEEL_DOWN, &output));
    rb_ui_event(&ui, output);
    assert(ui.selected == 1);
    assert(rb_5dc_ml_decode_button(0, RB_5DC_ML_PRESS_SET, &output));
    assert(rb_ui_event(&ui, output) == RB_UI_TOOLS);
    assert(rb_5dc_ml_decode_button(0, RB_5DC_ML_MENU, &output));
    rb_ui_event(&ui, output);
    assert(ui.opened == 0);
    puts("OK: Magic Lantern 5DC input to Rebobina UI in host");
    return 0;
}
