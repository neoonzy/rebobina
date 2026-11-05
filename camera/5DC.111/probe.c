/* SPDX-License-Identifier: GPL-2.0-only */
/* OBJETO ARM APENAS: nao possui entry point, enderecos ROM nem boot Canon. */
#include "rebobina/module.h"
#include "rebobina/ui.h"
#include <stdint.h>

/* Simbolo utilizado somente para inspecao da toolchain ARM. */
uint32_t rb_5dc_compilation_probe(void)
{
    return RB_MODULE_ABI_VERSION + RB_UI_ITEMS;
}
