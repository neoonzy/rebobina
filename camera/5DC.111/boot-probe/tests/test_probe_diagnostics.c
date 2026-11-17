/* SPDX-License-Identifier: GPL-2.0-only */
/* Host fault injection: no camera ROM, MMIO, flash writes or CF. */
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int create_result;
static int write_result;
static unsigned close_calls;
static unsigned led_calls;
static unsigned flash_count[2];
static unsigned flash_duration[2];
static unsigned sleeps;

int rb_test_create(const char *path)
{
    assert(strcmp(path, "A:/REBOBINA.TXT") == 0);
    return create_result;
}
int rb_test_write(int handle, const void *bytes, unsigned size)
{
    assert(handle == create_result);
    assert(bytes != NULL);
    assert(size > 60 && size < 250);
    assert(strstr((const char *)bytes, "BOOT PROBE 0.0.2") != NULL);
    return write_result < 0 ? write_result : (write_result == 0 ? (int)size : write_result);
}
void rb_test_close(int handle)
{
    assert(handle == create_result);
    ++close_calls;
}
void rb_test_led(unsigned flashes, unsigned duration_ms)
{
    assert(led_calls < 2);
    flash_count[led_calls] = flashes;
    flash_duration[led_calls] = duration_ms;
    ++led_calls;
}
void rb_test_sleep(unsigned duration_ms)
{
    assert(duration_ms == 700u);
    ++sleeps;
}
void rb_boot(void);
int rb_key(const void *event);
static void check(int handle, int written, unsigned expected_second, unsigned duration,
                  unsigned closes)
{
    create_result = handle;
    write_result = written;
    close_calls = led_calls = sleeps = 0;
    rb_boot();
    assert(sleeps == 1);
    assert(led_calls == 2);
    assert(flash_count[0] == 1 && flash_duration[0] == 170);
    assert(flash_count[1] == expected_second && flash_duration[1] == duration);
    assert(close_calls == closes);
    assert(rb_key(NULL) == 1);
}
int main(void)
{
    check(-1, 0, 2, 180, 0); /* Create failed */
    check(23, -1, 3, 180, 1); /* Write failed */
    check(23, 1, 3, 180, 1);  /* Short write */
    check(23, 0, 1, 900, 1);  /* Full write */
    puts("OK: entry, create failure, write failure, full write diagnostics");
    return 0;
}
