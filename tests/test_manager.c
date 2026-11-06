/* SPDX-License-Identifier: GPL-2.0-only */
#include "rebobina/manager.h"
#include <assert.h>
#include <stdio.h>

static unsigned starts[3], stops[3];
static unsigned order[3], order_size;
static rb_status start_a(const rb_host_api *host)
{
    (void)host;
    ++starts[0];
    return RB_OK;
}
static rb_status start_b(const rb_host_api *host)
{
    (void)host;
    ++starts[1];
    return RB_OK;
}
static rb_status start_fail(const rb_host_api *host)
{
    (void)host;
    ++starts[2];
    return RB_ERR_START;
}
static void stop_a(void) { ++stops[0]; order[order_size++] = 0; }
static void stop_b(void) { ++stops[1]; order[order_size++] = 1; }
static void stop_fail(void) { ++stops[2]; }
static void quiet_log(const char *id, const char *message)
{
    (void)id;
    (void)message;
}

int main(void)
{
    rb_manager manager;
    rb_host_api host = {RB_MODULE_ABI_VERSION, quiet_log};
    rb_host_api invalid = {9u, quiet_log};
    rb_module a = {RB_MODULE_ABI_VERSION, "a", "A", start_a, stop_a};
    rb_module b = {RB_MODULE_ABI_VERSION, "b", "B", start_b, stop_b};
    rb_module failed = {RB_MODULE_ABI_VERSION, "fail", "Fail", start_fail, stop_fail};
    rb_manager_init(&manager);
    assert(rb_manager_add(&manager, &a) == RB_OK);
    assert(rb_manager_add(&manager, &b) == RB_OK);
    assert(rb_manager_add(&manager, &failed) == RB_OK);
    assert(rb_manager_add(&manager, &a) == RB_ERR_DUPLICATE);
    assert(rb_manager_enable(&manager, 0, &invalid) == RB_ERR_ARGUMENT);
    assert(rb_manager_enable(&manager, 3, &host) == RB_ERR_ARGUMENT);
    assert(rb_manager_enable(&manager, 0, &host) == RB_OK);
    assert(rb_manager_enable(&manager, 0, &host) == RB_ERR_STATE);
    assert(rb_manager_is_active(&manager, 0));
    assert(rb_manager_add(&manager, &a) == RB_ERR_STATE);
    assert(rb_manager_enable(&manager, 1, &host) == RB_OK);
    assert(rb_manager_enable(&manager, 2, &host) == RB_ERR_START);
    assert(!rb_manager_is_active(&manager, 2));
    rb_manager_stop_all(&manager);
    rb_manager_stop_all(&manager);
    assert(!rb_manager_is_active(&manager, 0));
    assert(!rb_manager_is_active(&manager, 1));
    assert(starts[0] == 1 && starts[1] == 1 && starts[2] == 1);
    assert(stops[0] == 1 && stops[1] == 1 && stops[2] == 0);
    assert(order_size == 2 && order[0] == 1 && order[1] == 0);
    assert(rb_manager_disable(&manager, 0) == RB_ERR_STATE);
    assert(rb_manager_add(&manager, NULL) == RB_ERR_ARGUMENT);
    assert(rb_manager_enable(NULL, 0, &host) == RB_ERR_ARGUMENT);
    assert(rb_manager_disable(NULL, 0) == RB_ERR_ARGUMENT);
    assert(!rb_manager_is_active(NULL, 0));
    rb_manager_init(NULL);
    rb_manager_stop_all(NULL);
    puts("OK: manager de modulos (host), inicio, falha e desligamento");
    return 0;
}
