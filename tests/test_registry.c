/* SPDX-License-Identifier: GPL-2.0-only */
#include "rebobina/module.h"
#include <assert.h>
#include <stdio.h>

static int started, stopped, logged;

static void test_log(const char *id, const char *message)
{
    if (id && message) logged++;
}
static rb_status good_start(const rb_host_api *host)
{
    started++;
    host->log("good", "started");
    return RB_OK;
}
static void good_stop(void) { stopped++; }
static rb_status bad_start(const rb_host_api *host)
{
    (void)host;
    return RB_ERR_START;
}

int main(void)
{
    rb_registry r;
    rb_host_api host = {RB_MODULE_ABI_VERSION, test_log};
    const rb_module good = {RB_MODULE_ABI_VERSION, "good", "Good", good_start, good_stop};
    const rb_module duplicate = {RB_MODULE_ABI_VERSION, "good", "Duplicado", good_start, good_stop};
    const rb_module wrong_abi = {999, "other", "Other", good_start, good_stop};
    const rb_module bad = {RB_MODULE_ABI_VERSION, "bad", "Bad", bad_start, good_stop};

    assert(rb_registry_add(NULL, &good) == RB_ERR_ARGUMENT);
    rb_registry_init(&r);
    assert(r.count == 0 && r.started == 0);
    assert(rb_registry_add(&r, &good) == RB_OK);
    assert(rb_registry_add(&r, &duplicate) == RB_ERR_DUPLICATE);
    assert(rb_registry_add(&r, &wrong_abi) == RB_ERR_ABI);
    assert(rb_registry_add(&r, &bad) == RB_OK);
    assert(rb_registry_start(&r, &host) == RB_ERR_START);
    assert(started == 1 && stopped == 1);
    assert(r.started == 0 && logged == 1);
    rb_registry_stop(&r);
    assert(stopped == 1);

    rb_registry_init(&r);
    assert(rb_registry_add(&r, &good) == RB_OK);
    assert(rb_registry_start(&r, &host) == RB_OK);
    assert(rb_registry_start(&r, &host) == RB_ERR_STATE);
    assert(rb_registry_add(&r, &bad) == RB_ERR_STATE);
    rb_registry_stop(&r);
    rb_registry_stop(&r);
    assert(started == 2 && stopped == 2);

    puts("OK: registro e ciclo de vida");
    return 0;
}
