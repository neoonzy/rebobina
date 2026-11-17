/* SPDX-License-Identifier: GPL-2.0-only */
/* Canon EOS 5D Classic 1.1.1 / VxWorks ROM ABI, from GPLv2 research of
 * Magic Lantern Team and jiijii0 / Soup Engine (Canon-EOS-5D/src/ml/payload/cam.h).
 * Not applicable to other cameras or firmware revisions.
 */
#include "rb_hal.h"
#include <stdint.h>
#define ROM_CREATE       0xFFAE5B2Cu
#define ROM_OPEN         0xFFAE5B1Cu
#define ROM_READ         0xFFAE5B54u
#define ROM_WRITE        0xFFAE5B6Cu
#define ROM_CLOSE        0xFFAE5B78u
#define ROM_ALLOC        0xFFB2F664u
#define ROM_FREE         0xFFB2F680u
#define ROM_MEMINFO      0xFFB2F750u
#define ROM_SLEEP        0xFFB187F4u
#define ROM_TICK         0xFFB58DE4u
#define ROM_TASK         0xFFB18B40u
#define ML_BMP_PRINTF    0x0098110Cu
#define DISPLAY_IS_ON    0x0000FCA0u
#define UNCACHED_ALIAS   0x10000000u
int rb_hal_create(const char *p) { return ((int (*)(const char *))(uintptr_t)ROM_CREATE)(p); }
int rb_hal_open(const char *p) { return ((int (*)(const char *,int))(uintptr_t)ROM_OPEN)(p,0); }
int rb_hal_read(int h,void *b,unsigned n) { return ((int (*)(int,void *,unsigned))(uintptr_t)ROM_READ)(h,b,n); }
int rb_hal_write(int h,const void *b,unsigned n) { return ((int (*)(int,const void *,unsigned))(uintptr_t)ROM_WRITE)(h,b,n); }
void rb_hal_close(int h) { ((void (*)(int))(uintptr_t)ROM_CLOSE)(h); }
void *rb_hal_alloc(unsigned n) { return ((void *(*)(unsigned))(uintptr_t)ROM_ALLOC)(n); }
void rb_hal_free(void *p) { ((void (*)(void *))(uintptr_t)ROM_FREE)(p); }
void rb_hal_meminfo(int *total,int *free_bytes) {
  ((void (*)(int *,int *))(uintptr_t)ROM_MEMINFO)(total,free_bytes);
}
void rb_hal_sleep(unsigned ms) { ((void (*)(unsigned))(uintptr_t)ROM_SLEEP)(ms); }
uint32_t rb_hal_tick(void) { return ((uint32_t (*)(void))(uintptr_t)ROM_TICK)(); }
int rb_hal_task(const char *name,int prio,int stack,void (*fn)(void *),void *arg) {
  return ((int (*)(const char *,int,int,void (*)(void *),void *))(uintptr_t)ROM_TASK)(name,prio,stack,fn,arg);
}
void *rb_hal_uncached(void *p) { return (void *)((uintptr_t)p | UNCACHED_ALIAS); }
int rb_hal_display_on(void) { return *(volatile const uint32_t *)(uintptr_t)DISPLAY_IS_ON == 1u; }
void rb_hal_text(int x,int y,const char *s) {
  /* Uses relocated Magic Lantern bitmap font, does not modify registers or
   * force Canon's LCD to turn on. ONLY called with RB5UI.ON marker present. */
  ((void (*)(uint32_t,int,int,const char *,...))(uintptr_t)ML_BMP_PRINTF)(
    0x00030201u,x,y,"%s",s);
}
