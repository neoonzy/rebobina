/* SPDX-License-Identifier: GPL-2.0-only */
#include "rebobina/ui.h"
#include <assert.h>
#include <stdio.h>

typedef struct mock_screen {
    unsigned begin_calls;
    unsigned row_calls;
    unsigned end_calls;
    unsigned selected_rows;
} mock_screen;

static void begin(void *ctx) { ((mock_screen *)ctx)->begin_calls++; }
static void row(void *ctx, unsigned row, const char *text, int selected)
{
    mock_screen *screen = ctx;
    assert(text != NULL);
    assert(row < RB_UI_ITEMS + 1u);
    screen->row_calls++;
    if (selected) screen->selected_rows++;
}
static void end(void *ctx) { ((mock_screen *)ctx)->end_calls++; }

int main(void)
{
    rb_ui ui;
    mock_screen screen = {0, 0, 0, 0};
    rb_display display = {&screen, begin, row, end};
    rb_ui_init(&ui);
    assert(ui.opened == 0 && ui.selected == 0);
    assert(rb_ui_event(&ui, RB_UI_SET) == RB_UI_NONE);
    assert(rb_ui_event(&ui, RB_UI_UP) == RB_UI_NONE);
    rb_ui_render(&ui, &display);
    assert(screen.row_calls == 1 && screen.selected_rows == 0);
    rb_ui_event(&ui, RB_UI_TRASH);
    assert(ui.opened == 1);
    assert(rb_ui_event(&ui, RB_UI_UP) == RB_UI_NONE);
    assert(ui.selected == RB_UI_ITEMS - 1);
    rb_ui_event(&ui, RB_UI_DOWN);
    assert(ui.selected == 0);
    rb_ui_event(&ui, RB_UI_DOWN);
    rb_ui_event(&ui, RB_UI_DOWN);
    assert(ui.selected == 2);
    assert(rb_ui_event(&ui, RB_UI_SET) == RB_UI_MODULES);
    rb_ui_render(&ui, &display);
    assert(screen.row_calls == RB_UI_ITEMS + 2u);
    assert(screen.selected_rows == 1);
    rb_ui_event(&ui, RB_UI_BACK);
    assert(ui.opened == 0);
    rb_ui_event(&ui, RB_UI_TRASH);
    rb_ui_event(&ui, RB_UI_TRASH);
    assert(ui.opened == 0);
    assert(rb_ui_item_name(2) != NULL);
    assert(rb_ui_item_name(RB_UI_ITEMS) == NULL);
    assert(rb_ui_event(NULL, RB_UI_SET) == RB_UI_NONE);
    rb_ui_init(NULL);
    rb_ui_render(NULL, &display);
    rb_ui_render(&ui, NULL);
    assert(screen.begin_calls == screen.end_calls);
    puts("OK: menu e eventos em simulacao host");
    return 0;
}
