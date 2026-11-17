/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Rebobina Boot Probe 0.0.2 — Canon EOS 5D Classic 1.1.1.
 * Camera ROM addresses and LED register from GPLv2 research:
 * Magic Lantern Team, platform/5DC.111/consts.h, stubs.S (2013)
 * jiijii0 / Soup Engine, Canon-EOS-5D/src/ml/payload/cam.h
 *
 * Experimental, no bootflags, no Canon firmware update or image changes.
 * No BSS or non-constant globals. Called by the existing Soup Engine loader
 * after ML initialization; must always return to the loader.
 *
 * Blue LED indication at entry and after file I/O:
 *   one short flash, then a pause: rb_boot reached (the only entry signal)
 *   two short flashes: FIO_CreateFile failed
 *   three short flashes: FIO_WriteFile reported short write / error
 *   one long flash: full write reported (not verified on disk)
 * This is a bounded diagnostic, not a hardware safety certification.
 */
#include <stdint.h>
#include <stddef.h>

#define RB_5DC_FIO_CREATE 0xFFAE5B2Cu
#define RB_5DC_FIO_WRITE  0xFFAE5B6Cu
#define RB_5DC_FIO_CLOSE  0xFFAE5B78u
#define RB_5DC_MSLEEP     0xFFB187F4u
#define RB_5DC_BLUE_LED  0xC02200F0u
#define RB_5DC_LED_ON    0x46u
#define RB_5DC_LED_OFF   0x44u

#ifdef RB_PROBE_HOST_TEST
/* Host-only mocks; no hardcoded camera addresses accessed in tests. */
extern int rb_test_create(const char *path);
extern int rb_test_write(int handle, const void *data, unsigned n);
extern void rb_test_close(int handle);
extern void rb_test_led(unsigned flashes, unsigned duration_ms);
extern void rb_test_sleep(unsigned duration_ms);
#define rb_create rb_test_create
#define rb_write rb_test_write
#define rb_close rb_test_close
#define rb_flash rb_test_led
#define rb_sleep rb_test_sleep
#else
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
static void rb_sleep(unsigned ms)
{
    ((void (*)(unsigned))(uintptr_t)RB_5DC_MSLEEP)(ms);
}
static void rb_flash(unsigned flashes, unsigned duration_ms)
{
    volatile uint32_t *const led = (volatile uint32_t *)(uintptr_t)RB_5DC_BLUE_LED;
    unsigned i;
    for (i = 0; i < flashes; ++i) {
        *led = RB_5DC_LED_ON;
        rb_sleep(duration_ms);
        *led = RB_5DC_LED_OFF;
        if (i + 1u < flashes) rb_sleep(250u);
    }
}
#endif

void rb_boot(void)
{
    static const char path[] = "A:/REBOBINA.TXT";
    static const char message[] =
        "REBOBINA BOOT PROBE 0.0.2\n"
        "EOS 5D Classic 1.1.1 / VxWorks\n"
        "Carregado via AUTOEXEC.BIN Soup Engine.\n"
        "Diagnostico: LED azul, sem menu.\n";
    int handle;
    int written;
    unsigned expected = (unsigned)(sizeof(message) - 1u);

    rb_flash(1u, 170u); /* Reached the payload entry at all. */
    rb_sleep(700u);      /* Separate entry and result observations. */
    handle = rb_create(path);
    if (handle == -1) {
        rb_flash(2u, 180u);
        return;
    }
    written = rb_write(handle, message, expected);
    rb_close(handle);
    if (written != (int)expected) {
        rb_flash(3u, 180u);
        return;
    }
    rb_flash(1u, 900u); /* API reported success, not a verified on-card file. */
}

/* All GUI events pass through to the Canon/Magic Lantern handler. */
int rb_key(const void *event)
{
    (void)event;
    return 1;
}
