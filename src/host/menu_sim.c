/* SPDX-License-Identifier: GPL-2.0-only */
/* Simulador de INTERFACE em terminal, NAO de boot, hardware ou ROM Canon. */
#include "rebobina/ui.h"
#include <stdio.h>

static void screen_begin(void *context)
{
    (void)context;
    puts("---- TELA SIMULADA ----");
}
static void screen_row(void *context, unsigned row, const char *s, int selected)
{
    (void)context;
    printf("%u %s %s\n", row, selected ? ">" : " ", s);
}
static void screen_end(void *context)
{
    (void)context;
    puts("-----------------------");
}

int main(int argc, char **argv)
{
    rb_ui ui;
    rb_display display = {0, screen_begin, screen_row, screen_end};
    const char *steps = argc > 1 ? argv[1] : "tddsbt";
    unsigned i;
    rb_ui_init(&ui);
    rb_ui_render(&ui, &display);
    for (i = 0; steps[i] != '\0'; ++i) {
        rb_ui_button b;
        rb_ui_action act;
        switch (steps[i]) {
        case 't': b = RB_UI_TRASH; break;
        case 'u': b = RB_UI_UP; break;
        case 'd': b = RB_UI_DOWN; break;
        case 's': b = RB_UI_SET; break;
        case 'b': b = RB_UI_BACK; break;
        default:
            fprintf(stderr, "Entrada invalida: '%c' (use t,u,d,s,b)\n", steps[i]);
            return 2;
        }
        act = rb_ui_event(&ui, b);
        if (act == RB_UI_MODULES)
            puts("[sim] Modulos: Soup Engine ainda NAO integrado.");
        else if (act != RB_UI_NONE)
            printf("[sim] Acao %d: demonstração sem acesso à câmera.\n", (int)act);
        rb_ui_render(&ui, &display);
    }
    return 0;
}
