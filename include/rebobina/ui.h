/* SPDX-License-Identifier: GPL-2.0-only */
/* Modelo de menu portavel. Nenhuma chamada de hardware. */
#ifndef RB_UI_H
#define RB_UI_H
#include "rebobina/display.h"

#define RB_UI_ITEMS 4u

typedef enum rb_ui_button {
    RB_UI_TRASH = 0,
    RB_UI_UP,
    RB_UI_DOWN,
    RB_UI_SET,
    RB_UI_BACK
} rb_ui_button;

typedef enum rb_ui_action {
    RB_UI_NONE = 0,
    RB_UI_HOME,
    RB_UI_TOOLS,
    RB_UI_MODULES,
    RB_UI_ABOUT
} rb_ui_action;

typedef struct rb_ui {
    unsigned opened;
    unsigned selected;
} rb_ui;

void rb_ui_init(rb_ui *ui);
rb_ui_action rb_ui_event(rb_ui *ui, rb_ui_button button);
void rb_ui_render(const rb_ui *ui, const rb_display *display);
const char *rb_ui_item_name(unsigned index);

#endif
