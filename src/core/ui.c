/* SPDX-License-Identifier: GPL-2.0-only */
#include "rebobina/ui.h"

static const char *const rb_menu_names[RB_UI_ITEMS] = {
    "Inicio", "Ferramentas", "Modulos", "Sobre"
};

void rb_ui_init(rb_ui *ui)
{
    if (!ui) return;
    ui->opened = 0;
    ui->selected = 0;
}

const char *rb_ui_item_name(unsigned index)
{
    if (index >= RB_UI_ITEMS) return 0;
    return rb_menu_names[index];
}

rb_ui_action rb_ui_event(rb_ui *ui, rb_ui_button button)
{
    if (!ui) return RB_UI_NONE;
    if (button == RB_UI_TRASH) {
        ui->opened = !ui->opened;
        return RB_UI_NONE;
    }
    if (!ui->opened) return RB_UI_NONE;
    switch (button) {
    case RB_UI_UP:
        ui->selected = (ui->selected + RB_UI_ITEMS - 1u) % RB_UI_ITEMS;
        break;
    case RB_UI_DOWN:
        ui->selected = (ui->selected + 1u) % RB_UI_ITEMS;
        break;
    case RB_UI_SET:
        return (rb_ui_action)(ui->selected + 1u);
    case RB_UI_BACK:
        ui->opened = 0;
        break;
    default:
        break;
    }
    return RB_UI_NONE;
}

void rb_ui_render(const rb_ui *ui, const rb_display *display)
{
    unsigned i;
    if (!ui || !display || !display->row) return;
    if (display->begin) display->begin(display->context);
    if (!ui->opened) {
        display->row(display->context, 0, "Rebobina: TRASH abre o menu", 0);
    } else {
        display->row(display->context, 0, "REBOBINA / SIMULADOR HOST", 0);
        for (i = 0; i < RB_UI_ITEMS; ++i)
            display->row(display->context, i + 1u,
                         rb_menu_names[i], i == ui->selected);
    }
    if (display->end) display->end(display->context);
}
