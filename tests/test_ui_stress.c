/* SPDX-License-Identifier: GPL-2.0-only */
/* Teste deterministico de estados e eventos da UI, somente host. */
#include "rebobina/ui.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    rb_ui ui;
    uint32_t seed = 0xC0FFEEu;
    unsigned i;
    rb_ui_init(&ui);
    for (i = 0; i < 100000u; ++i) {
        seed = seed * 1664525u + 1013904223u;
        /* Inclui eventos invalidos 5 e 6 para garantir que nao ha efeito. */
        rb_ui_event(&ui, (rb_ui_button)((seed >> 16) % 7u));
        assert(ui.opened == 0 || ui.opened == 1);
        assert(ui.selected < RB_UI_ITEMS);
        assert(rb_ui_item_name(ui.selected) != NULL);
    }
    puts("OK: 100000 eventos de menu sem estado invalido");
    return 0;
}
