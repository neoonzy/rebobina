/* SPDX-License-Identifier: GPL-2.0-only */
/* Plugin Linux host, nao possui qualquer API de camera. */
#include "rebobina/module.h"

static const rb_host_api *g_host;

static rb_status start(const rb_host_api *host)
{
    g_host = host;
    g_host->log("hello", "Plugin independente iniciado.");
    return RB_OK;
}
static void stop(void)
{
    if (g_host) g_host->log("hello", "Plugin independente encerrado.");
    g_host = 0;
}

/* Contrato dinamico SOMENTE para prova Linux host. */
const rb_module rebobina_plugin_v1 = {
    RB_MODULE_ABI_VERSION, "hello-plugin", "Ola via plugin .so", start, stop
};
