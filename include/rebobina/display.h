/* SPDX-License-Identifier: GPL-2.0-only */
/* Abstracao de tela, sem enderecos Canon: prototipo host. */
#ifndef RB_DISPLAY_H
#define RB_DISPLAY_H
typedef struct rb_display {
    void *context;
    void (*begin)(void *context);
    void (*row)(void *context, unsigned row, const char *text, int selected);
    void (*end)(void *context);
} rb_display;
#endif
