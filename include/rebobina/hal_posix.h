/* SPDX-License-Identifier: GPL-2.0-only */
/* Linux host apenas: backend da HAL apontado para um diretorio EXPLICITO.
 * Nao usar em Canon, VxWorks ou como prova de atomicidade num CF real.
 */
#ifndef REBOBINA_HAL_POSIX_H
#define REBOBINA_HAL_POSIX_H
#include "rebobina/hal.h"
typedef struct rb_posix_hal {
    rb_hal api;
    int directory_fd;
    int active_fd;
    uint32_t generation;
    unsigned char mode; /* 0 fechado, 1 leitura, 2 escrita */
} rb_posix_hal;

rb_hal_result rb_posix_hal_init(rb_posix_hal *host, const char *directory);
void rb_posix_hal_destroy(rb_posix_hal *host);
#endif
