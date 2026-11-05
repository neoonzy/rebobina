/* SPDX-License-Identifier: GPL-2.0-only */
#include "rebobina/module.h"

static const rb_host_api *g_host;

static rb_status hello_start(const rb_host_api *host)
{
    g_host = host;
    host->log("hello", "Olá! Módulo carregado no computador.");
    return RB_OK;
}

static void hello_stop(void)
{
    if (g_host) g_host->log("hello", "Módulo encerrado.");
    g_host = 0;
}

const rb_module rb_hello_module = {
    RB_MODULE_ABI_VERSION, "hello", "Olá mundo", hello_start, hello_stop
};
