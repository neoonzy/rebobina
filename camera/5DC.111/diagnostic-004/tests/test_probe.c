/* SPDX-License-Identifier: GPL-2.0-only */
/* Host simulation only. No camera memory or boot code is run here. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static char boot_bytes[1024],late_bytes[1024];
static unsigned boot_n,late_n;
static unsigned alloc_calls, free_calls, sleep_calls, screen_calls, led_calls;
static unsigned shortwrite, memfail, marker_lcd, marker_led, display_on;
static int task_result=37, fail_create;
static void (*worker)(void *);
static unsigned char heap[256];
static uint32_t ticks=0x70;

int rb_test_create(const char *p) {
    if (fail_create) return -1;
    if (!strcmp(p,"A:/RB4BOOT.TXT")) return 9;
    if (!strcmp(p,"A:/RB4LATE.TXT")) return 10;
    assert(!"unexpected create path"); return -1;
}
int rb_test_write(int h,const void *b,unsigned n) {
    if (shortwrite) return (int)n-1;
    assert(n<1024);
    if (h==9){memcpy(boot_bytes,b,n); boot_bytes[n]=0; boot_n=n;}
    else if(h==10){memcpy(late_bytes,b,n); late_bytes[n]=0; late_n=n;}
    else assert(!"wrong handle");
    return (int)n;
}
int rb_test_open(const char *p) {
    if(!strcmp(p,"A:/RB4BOOT.TXT")) return boot_n ? 9 : -1;
    if(!strcmp(p,"A:/ML/RB4LCD.ON")) return marker_lcd ? 11 : -1;
    if(!strcmp(p,"A:/ML/RB4LED.ON")) return marker_led ? 12 : -1;
    assert(!"unexpected open path"); return -1;
}
int rb_test_read(int h,void *b,unsigned n) {
    assert(h==9); assert(n>boot_n); memcpy(b,boot_bytes,boot_n); return (int)boot_n;
}
void rb_test_close(int h) {assert(h==9||h==10||h==11||h==12);}
void *rb_test_alloc(unsigned n) { assert(n==256); alloc_calls++; return memfail?0:heap; }
void rb_test_free(void *p) {assert(p==heap); free_calls++;}
void rb_test_sleep(unsigned n) {assert(n==20||n==15000||n==90); sleep_calls++;}
uint32_t rb_test_tick(void) {return ticks++;}
int rb_test_task(const char *name,int prio,int stack,void (*fn)(void *),void *arg) {
    assert(!strcmp(name,"rb_diag") && prio==31 && stack==8192 && arg==0);
    if(task_result>=0) worker=fn;
    return task_result;
}
int rb_test_display_on(void) {return display_on;}
void rb_test_screen(const char *s) {assert(!strcmp(s,"REBOBINA 0.0.4")); screen_calls++;}
void rb_test_led(void) {led_calls++;}
void rb_boot(void);
void rb_worker(void *);
int rb_key(const void *);

static void reset(void) {
    memset(boot_bytes,0,sizeof(boot_bytes)); memset(late_bytes,0,sizeof(late_bytes));
    boot_n=late_n=alloc_calls=free_calls=sleep_calls=screen_calls=led_calls=0;
    shortwrite=memfail=marker_lcd=marker_led=display_on=fail_create=0;
    task_result=37; worker=0; ticks=0x70;
}
static void test_full(void) {
    uint32_t a[2]={0,0xAABBCCDD}, b[2]={0,0xDEADBEEF};
    reset(); rb_boot();
    assert(boot_n>120 && strstr(boot_bytes,"MEM_256=PASS"));
    assert(strstr(boot_bytes,"WORKER_ID_HEX=00000025"));
    assert(alloc_calls==1 && free_calls==1 && worker);
    assert(rb_key(a)==1 && rb_key(b)==1 && rb_key(NULL)==1);
    worker(0);
    assert(late_n>100 && strstr(late_bytes,"READBACK=PASS"));
    assert(strstr(late_bytes,"GUI_EVENTS_HEX=00000002"));
    assert(strstr(late_bytes,"RAW_EVENT_00000000=00000000"));
    assert(strstr(late_bytes,"EVENT_PARAM_00000000=AABBCCDD"));
    assert(strstr(late_bytes,"EVENT_PARAM_00000001=DEADBEEF"));
    assert(screen_calls==0 && led_calls==0);
    printf("OK: full boot, memory, tick, worker, readback, passive keys\n");
}
static void test_opt_ins(void) {
    reset(); marker_lcd=marker_led=display_on=1;
    rb_boot(); assert(led_calls==1); assert(worker);
    worker(0); assert(led_calls==2 && screen_calls==1);
    assert(strstr(late_bytes,"LCD=CALL_ATTEMPTED"));
    printf("OK: explicit optical opt-ins\n");
}
static void test_errors(void) {
    reset(); memfail=1; task_result=-1;
    rb_boot(); assert(strstr(boot_bytes,"MEM_256=FAIL"));
    assert(strstr(boot_bytes,"WORKER_ID_HEX=FFFFFFFF"));
    assert(worker==0 && free_calls==0);
    reset(); fail_create=1; rb_boot(); assert(boot_n==0);
    rb_worker(0); assert(late_n==0);
    reset(); shortwrite=1; rb_boot(); assert(boot_n==0);
    rb_worker(0); assert(late_n==0);
    printf("OK: allocation, worker and output fault paths\n");
}
int main(void) {
    test_full();test_opt_ins();test_errors();
    puts("ALL HOST DIAGNOSTICS PASSED (not a camera test)");
    return 0;
}
