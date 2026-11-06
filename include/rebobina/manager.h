/* SPDX-License-Identifier: GPL-2.0-only */
/* Gerenciador de ciclo de vida, host primeiro; nao e carregador ARM. */
#ifndef RB_MANAGER_H
#define RB_MANAGER_H

#include "rebobina/module.h"

typedef struct rb_manager {
    rb_registry registry;
    unsigned char active[RB_MAX_MODULES];
} rb_manager;

void rb_manager_init(rb_manager *manager);
rb_status rb_manager_add(rb_manager *manager, const rb_module *module);
rb_status rb_manager_enable(rb_manager *manager, size_t index, const rb_host_api *host);
rb_status rb_manager_disable(rb_manager *manager, size_t index);
int rb_manager_is_active(const rb_manager *manager, size_t index);
void rb_manager_stop_all(rb_manager *manager);

#endif
