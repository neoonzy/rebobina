/* SPDX-License-Identifier: GPL-2.0-only */
/* Demonstra apenas o contrato de módulos no computador. */
#include "rebobina/module.h"
#include <stdio.h>

extern const rb_module rb_hello_module;

static void print_log(const char *id, const char *message)
{
    printf("[%s] %s\n", id, message);
}

int main(void)
{
    rb_registry registry;
    rb_host_api host = {RB_MODULE_ABI_VERSION, print_log};

    puts("Rebobina / protótipo host (não instalar na câmera)");
    rb_registry_init(&registry);
    if (rb_registry_add(&registry, &rb_hello_module) != RB_OK) return 1;
    if (rb_registry_start(&registry, &host) != RB_OK) return 2;
    rb_registry_stop(&registry);
    return 0;
}
