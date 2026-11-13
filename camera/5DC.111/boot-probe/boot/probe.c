/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Rebobina - prova experimental de vida na EOS 5D Classic 1.1.1.
 * ABI/endereco de ROM baseados no Soup Engine 5DC (jiijii0, GPLv2).
 * Nao instala, nao altera boot flags, firmware Canon, LCD nem fotos RAW.
 *
 * Escrita observavel: SOMENTE A:/REBOBINA.TXT na inicializacao.
 * NAO VALIDADO NA CAMERA REAL. USE APENAS EQUIPAMENTO E CF DE TESTE.
 */
#include <stdint.h>
#include <stddef.h>
#define RB_5DC_FIO_CREATE 0xFFAE5B2Cu
#define RB_5DC_FIO_WRITE  0xFFAE5B6Cu
#define RB_5DC_FIO_CLOSE  0xFFAE5B78u

static int rb_create(const char *path)
{
    return ((int (*)(const char *))(uintptr_t)RB_5DC_FIO_CREATE)(path);
}
static int rb_write(int handle, const void *data, unsigned n)
{
    return ((int (*)(int, const void *, unsigned))(uintptr_t)RB_5DC_FIO_WRITE)(handle, data, n);
}
static void rb_close(int handle)
{
    ((void (*)(int))(uintptr_t)RB_5DC_FIO_CLOSE)(handle);
}

void rb_boot(void)
{
    static const char path[] = "A:/REBOBINA.TXT";
    static const char message[] =
        "REBOBINA BOOT PROBE 0.0.1\n"
        "EOS 5D Classic 1.1.1 / VxWorks\n"
        "Executado por loader legado Soup Engine.\n"
        "SEM interface e SEM garantia de compatibilidade.\n";
    int handle = rb_create(path);
    if (handle == -1) return;
    (void)rb_write(handle, message, (unsigned)(sizeof(message) - 1u));
    rb_close(handle);
}

/* A GUI deve continuar com controle Canon/ML; nenhum IO no callback. */
int rb_key(const void *event)
{
    (void)event;
    return 1;
}
