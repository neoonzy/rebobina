/* SPDX-License-Identifier: GPL-2.0-only */
/* API EXPERIMENTAL: esta versão roda apenas no host, não na câmera. */
#ifndef REBOBINA_MODULE_H
#define REBOBINA_MODULE_H

#include <stddef.h>
#include <stdint.h>

#define RB_MODULE_ABI_VERSION 1u
#define RB_MAX_MODULES 16u

typedef enum rb_status {
    RB_OK = 0,
    RB_ERR_ARGUMENT,
    RB_ERR_ABI,
    RB_ERR_DUPLICATE,
    RB_ERR_CAPACITY,
    RB_ERR_STATE,
    RB_ERR_START
} rb_status;

typedef struct rb_host_api {
    uint32_t abi_version;
    void (*log)(const char *module_id, const char *message);
} rb_host_api;

typedef struct rb_module {
    uint32_t abi_version;
    const char *id;
    const char *name;
    rb_status (*start)(const rb_host_api *host);
    void (*stop)(void);
} rb_module;

typedef struct rb_registry {
    const rb_module *items[RB_MAX_MODULES];
    size_t count;
    size_t started;
} rb_registry;

void rb_registry_init(rb_registry *registry);
rb_status rb_registry_add(rb_registry *registry, const rb_module *module);
rb_status rb_registry_start(rb_registry *registry, const rb_host_api *host);
void rb_registry_stop(rb_registry *registry);

#endif
