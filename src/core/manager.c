/* SPDX-License-Identifier: GPL-2.0-only */
#include "rebobina/manager.h"

void rb_manager_init(rb_manager *manager)
{
    size_t i;
    if (!manager) return;
    rb_registry_init(&manager->registry);
    for (i = 0; i < RB_MAX_MODULES; ++i)
        manager->active[i] = 0;
}

rb_status rb_manager_add(rb_manager *manager, const rb_module *module)
{
    size_t i;
    if (!manager) return RB_ERR_ARGUMENT;
    for (i = 0; i < manager->registry.count; ++i)
        if (manager->active[i]) return RB_ERR_STATE;
    return rb_registry_add(&manager->registry, module);
}

int rb_manager_is_active(const rb_manager *manager, size_t index)
{
    if (!manager || index >= manager->registry.count) return 0;
    return manager->active[index] != 0;
}

rb_status rb_manager_enable(rb_manager *manager, size_t index, const rb_host_api *host)
{
    rb_status status;
    if (!manager || !host || index >= manager->registry.count ||
        host->abi_version != RB_MODULE_ABI_VERSION || !host->log)
        return RB_ERR_ARGUMENT;
    if (manager->active[index]) return RB_ERR_STATE;
    status = manager->registry.items[index]->start(host);
    if (status != RB_OK) return RB_ERR_START;
    manager->active[index] = 1;
    return RB_OK;
}

rb_status rb_manager_disable(rb_manager *manager, size_t index)
{
    if (!manager || index >= manager->registry.count)
        return RB_ERR_ARGUMENT;
    if (!manager->active[index]) return RB_ERR_STATE;
    manager->active[index] = 0;
    manager->registry.items[index]->stop();
    return RB_OK;
}

void rb_manager_stop_all(rb_manager *manager)
{
    size_t i;
    if (!manager) return;
    for (i = manager->registry.count; i > 0; --i)
        if (manager->active[i - 1u])
            (void)rb_manager_disable(manager, i - 1u);
}
