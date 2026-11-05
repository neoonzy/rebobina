/* SPDX-License-Identifier: GPL-2.0-only */
#include "rebobina/module.h"
/* Sem libc: compilável em ambiente ARM freestanding. */
static int rb_id_equal(const char *a, const char *b)
{
    while (*a && *b) {
        if (*a != *b) return 0;
        ++a;
        ++b;
    }
    return *a == *b;
}

void rb_registry_init(rb_registry *registry)
{
    if (!registry) return;
    {
        size_t i;
        registry->count = 0;
        registry->started = 0;
        for (i = 0; i < RB_MAX_MODULES; ++i)
            registry->items[i] = 0;
    }
}

rb_status rb_registry_add(rb_registry *registry, const rb_module *module)
{
    size_t i;
    if (!registry || !module || !module->id || !module->name ||
        !module->id[0] || !module->name[0] || !module->start || !module->stop)
        return RB_ERR_ARGUMENT;
    if (module->abi_version != RB_MODULE_ABI_VERSION)
        return RB_ERR_ABI;
    if (registry->started != 0) return RB_ERR_STATE;
    for (i = 0; i < registry->count; ++i)
        if (rb_id_equal(registry->items[i]->id, module->id))
            return RB_ERR_DUPLICATE;
    if (registry->count >= RB_MAX_MODULES) return RB_ERR_CAPACITY;
    registry->items[registry->count++] = module;
    return RB_OK;
}

rb_status rb_registry_start(rb_registry *registry, const rb_host_api *host)
{
    size_t i;
    if (!registry || !host || host->abi_version != RB_MODULE_ABI_VERSION ||
        !host->log) return RB_ERR_ARGUMENT;
    if (registry->started != 0) return RB_ERR_STATE;
    for (i = 0; i < registry->count; ++i) {
        rb_status rc = registry->items[i]->start(host);
        if (rc != RB_OK) {
            rb_registry_stop(registry);
            return RB_ERR_START;
        }
        registry->started++;
    }
    return RB_OK;
}

void rb_registry_stop(rb_registry *registry)
{
    if (!registry) return;
    while (registry->started > 0) {
        registry->started--;
        registry->items[registry->started]->stop();
    }
}
