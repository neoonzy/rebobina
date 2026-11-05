/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Prova de carregamento dinamico SOMENTE EM LINUX HOST.
 * dlopen() executa codigo nativo SEM sandbox. Nunca apontar para arquivo nao confiavel.
 */
#include "rebobina/module.h"
#include <dlfcn.h>
#include <stdio.h>

static void host_log(const char *id, const char *message)
{
    printf("[%s] %s\n", id, message);
}

int main(int argc, char **argv)
{
    void *handle;
    const rb_module *module;
    rb_registry registry;
    rb_host_api api = {RB_MODULE_ABI_VERSION, host_log};
    rb_status status;
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <modulo-host-confiavel.so>\n", argv[0]);
        return 2;
    }
    handle = dlopen(argv[1], RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        fprintf(stderr, "dlopen falhou: %s\n", dlerror());
        return 3;
    }
    module = (const rb_module *)dlsym(handle, "rebobina_plugin_v1");
    if (!module) {
        fprintf(stderr, "Modulo sem simbolo rebobina_plugin_v1\n");
        dlclose(handle);
        return 4;
    }
    rb_registry_init(&registry);
    status = rb_registry_add(&registry, module);
    if (status != RB_OK) {
        fprintf(stderr, "Modulo rejeitado: codigo %d (ABI ou contrato incompatível)\n", status);
        dlclose(handle);
        return 5;
    }
    status = rb_registry_start(&registry, &api);
    if (status != RB_OK) {
        fprintf(stderr, "Falha ao iniciar modulo: codigo %d\n", status);
        rb_registry_stop(&registry);
        dlclose(handle);
        return 6;
    }
    printf("Modulo carregado no HOST: %s\n", module->name);
    rb_registry_stop(&registry);
    dlclose(handle);
    return 0;
}
