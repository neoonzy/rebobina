/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef RB_HAL_H
#define RB_HAL_H
#include <stdint.h>

/* Experimental interface restricted to 5DC.111, matched to working Soup port. */
int rb_hal_create(const char *path);
int rb_hal_open(const char *path);
int rb_hal_read(int handle, void *dst, unsigned count);
int rb_hal_write(int handle, const void *src, unsigned count);
void rb_hal_close(int handle);
void *rb_hal_alloc(unsigned size);
void rb_hal_free(void *ptr);
void rb_hal_meminfo(int *total, int *free_bytes);
void rb_hal_sleep(unsigned ms);
uint32_t rb_hal_tick(void);
int rb_hal_task(const char *name, int priority, int stack_size, void (*entry)(void *), void *arg);
void *rb_hal_uncached(void *ptr);
int rb_hal_display_on(void);
void rb_hal_text(int x, int y, const char *string);
#endif
