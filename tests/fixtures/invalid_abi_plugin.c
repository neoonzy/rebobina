/* SPDX-License-Identifier: GPL-2.0-only */
/* Fixture de teste negativo, nao empacotar para camera. */
#include "rebobina/module.h"
static rb_status start(const rb_host_api *host)
{
    (void)host;
    return RB_OK;
}
static void stop(void) {}
const rb_module rebobina_plugin_v1 = {
    99u, "abi-invalid", "ABI invalida", start, stop
};
