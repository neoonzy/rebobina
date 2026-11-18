/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef RB_HAL_H
#define RB_HAL_H
#include <stdint.h>
/* Firmware-specific functions; target Canon EOS 5D Classic 1.1.1 only. */
int rb_hal_create(const char *path);
int rb_hal_open(const char *path);
int rb_hal_read(int h,void *dst,unsigned bytes);
int rb_hal_write(int h,const void *src,unsigned bytes);
void rb_hal_close(int h);
void *rb_hal_alloc(unsigned n);
void rb_hal_free(void *p);
void rb_hal_meminfo(int *total,int *freebytes);
void rb_hal_sleep(unsigned ms);
uint32_t rb_hal_tick(void);
int rb_hal_task(const char *name,int priority,int stack_size,void (*fn)(void*),void *arg);
void *rb_hal_uncached(void *p);
int rb_hal_display_on(void);
int rb_hal_bitmap_ready(void);
void rb_hal_fill(unsigned color,int x,int y,int w,int h);
void rb_hal_text(int x,int y,const char *text,unsigned fg,unsigned bg);
#endif
