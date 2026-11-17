/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * REBOBINA 0.0.3 — unified camera diagnostic, Canon EOS 5D Classic 1.1.1.
 * Load ONLY as SE.BIN under the already working Soup Engine 5DC loader.
 * Does not change bootflags, Canon ROM, camera settings, AUTOEXEC.BIN,
 * photographs, or any file other than A:/RB3BOOT.TXT and A:/RB3LATE.TXT.
 *
 * References: jiijii0/soup-engine Canon-EOS-5D/src/ml/payload/cam.h,
 * Magic Lantern Team platform/5DC.111 (GPLv2 research/credit).
 * No upstream binary or proprietary Canon ROM distributed here.
 */
#include <stdint.h>
#include <stddef.h>

#define RB_VERSION "0.0.3"
#define RB_ECOUNT 12u
#define RB_BUF_CAP 512u
#define RB_WAIT_MS 8000u

#ifndef RB_PROBE_HOST_TEST
#define ROM_CREATE 0xFFAE5B2Cu
#define ROM_WRITE  0xFFAE5B6Cu
#define ROM_CLOSE  0xFFAE5B78u
#define ROM_OPEN   0xFFAE5B1Cu
#define ROM_READ   0xFFAE5B54u
#define ROM_ALLOC  0xFFB2F664u
#define ROM_FREE   0xFFB2F680u
#define ROM_SLEEP  0xFFB187F4u
#define ROM_TICK   0xFFB58DE4u
#define ROM_TASK   0xFFB18B40u
#define ML_BMP_PRINTF 0x0098110Cu
#define DISPLAY_IS_ON 0x0000FCA0u
#define LED_MMIO 0xC02200F0u
static int rb_create(const char *p) { return ((int (*)(const char *))(uintptr_t)ROM_CREATE)(p); }
static int rb_write(int h, const void *b, unsigned n) { return ((int (*)(int,const void *,unsigned))(uintptr_t)ROM_WRITE)(h,b,n); }
static int rb_open(const char *p) { return ((int (*)(const char *,int))(uintptr_t)ROM_OPEN)(p,0); }
static int rb_read(int h, void *b, unsigned n) { return ((int (*)(int,void *,unsigned))(uintptr_t)ROM_READ)(h,b,n); }
static void rb_close(int h) { ((void (*)(int))(uintptr_t)ROM_CLOSE)(h); }
static void *rb_alloc(unsigned n) { return ((void *(*)(unsigned))(uintptr_t)ROM_ALLOC)(n); }
static void rb_free(void *p) { ((void (*)(void *))(uintptr_t)ROM_FREE)(p); }
static void rb_sleep(unsigned ms) { ((void (*)(unsigned))(uintptr_t)ROM_SLEEP)(ms); }
static uint32_t rb_tick(void) { return ((uint32_t (*)(void))(uintptr_t)ROM_TICK)(); }
static int rb_task(const char *name, int prio, int stack, void (*fn)(void *), void *arg)
{ return ((int (*)(const char *,int,int,void (*)(void *),void *))(uintptr_t)ROM_TASK)(name,prio,stack,fn,arg); }
static void *rb_read_alias(void *p) { return (void *)((uintptr_t)p | (uintptr_t)0x10000000u); }
static int rb_display_on(void) { return *(volatile uint32_t *)(uintptr_t)DISPLAY_IS_ON == 1u; }
static void rb_screen(const char *s) {
    ((void (*)(uint32_t,int,int,const char *,...))(uintptr_t)ML_BMP_PRINTF)(0x00030201u,16,24,"%s",s);
}
static void rb_led(void) {
    volatile uint32_t *led=(volatile uint32_t *)(uintptr_t)LED_MMIO;
    *led=0x46u; rb_sleep(90u); *led=0x44u;
}
#else
extern int rb_test_create(const char *p);
extern int rb_test_write(int h,const void *b,unsigned n);
extern int rb_test_open(const char *p);
extern int rb_test_read(int h,void *b,unsigned n);
extern void rb_test_close(int h);
extern void *rb_test_alloc(unsigned n);
extern void rb_test_free(void *p);
extern void rb_test_sleep(unsigned ms);
extern uint32_t rb_test_tick(void);
extern int rb_test_task(const char *name,int prio,int stack,void (*fn)(void *),void *arg);
extern int rb_test_display_on(void);
extern void rb_test_screen(const char *s);
extern void rb_test_led(void);
#define rb_create rb_test_create
#define rb_write rb_test_write
#define rb_open rb_test_open
#define rb_read rb_test_read
#define rb_close rb_test_close
#define rb_alloc rb_test_alloc
#define rb_free rb_test_free
#define rb_sleep rb_test_sleep
#define rb_tick rb_test_tick
#define rb_task rb_test_task
#define rb_display_on rb_test_display_on
#define rb_screen rb_test_screen
#define rb_led rb_test_led
static void *rb_read_alias(void *p) { return p; }
#endif

/* Forced initialized .data: the Soup loader copies the image but does NOT
 * clear .bss. These static buffers survive asynchronous FIO_WriteFile DMA.
 */
static char rb_boot_buf[RB_BUF_CAP] = { 'B' };
static char rb_late_buf[RB_BUF_CAP] = { 'L' };
static char rb_read_buf[RB_BUF_CAP] = { 'R' };
static volatile struct rb_event_state {
    uint32_t magic;
    uint32_t count;
    uint32_t first[RB_ECOUNT];
} rb_events = { 0x52423345u, 0xFFFFFFFFu, { 0xFFFFFFFFu } };
static volatile uint32_t rb_boot_length = 0xFFFFFFFFu;

static unsigned rb_append(char *buf, unsigned n, const char *s)
{
    while (*s && n + 1u < RB_BUF_CAP) buf[n++] = *s++;
    buf[n] = 0;
    return n;
}
static unsigned rb_hex(char *buf, unsigned n, uint32_t v)
{
    static const char digits[] = "0123456789ABCDEF";
    unsigned i;
    for (i = 0; i < 8u && n + 1u < RB_BUF_CAP; ++i)
        buf[n++] = digits[(v >> (28u - 4u*i)) & 15u];
    buf[n] = 0;
    return n;
}
static int rb_exists(const char *path)
{
    int h=rb_open(path);
    if (h == -1) return 0;
    rb_close(h);
    return 1;
}
static int rb_store(const char *path, const char *buf, unsigned size)
{
    int h=rb_create(path);
    int w;
    if (h == -1) return -1;
    w=rb_write(h,buf,size);
    rb_close(h);
    return w == (int)size ? 1 : 0;
}
static int rb_memtest(void)
{
    volatile unsigned char *p = (volatile unsigned char *)rb_alloc(256u);
    unsigned i;
    int ok=1;
    if (!p) return 0;
    for (i=0; i<256u; ++i) p[i]=(unsigned char)((i*29u+7u)&255u);
    for (i=0; i<256u; ++i) if (p[i] != (unsigned char)((i*29u+7u)&255u)) ok=0;
    rb_free((void *)p);
    return ok;
}
static int rb_readback(void)
{
    unsigned i;
    unsigned length=rb_boot_length;
    int h,n;
    volatile unsigned char *p;
    if (!length || length >= RB_BUF_CAP) return -2;
    h=rb_open("A:/RB3BOOT.TXT");
    if (h == -1) return -1;
    /* 5DC VxWorks FIO uses DMA; access the read destination uncached.
     * Do NOT touch rb_read_buf through its cached alias after this call.
     */
    p=(volatile unsigned char *)rb_read_alias(rb_read_buf);
    n=rb_read(h,(void *)p,length+1u);
    rb_close(h);
    if (n != (int)length) return -3;
    for (i=0;i<length;++i) if (p[i] != (unsigned char)rb_boot_buf[i]) return -4;
    return 1;
}

/* Worker is independent of Canon's GUI task. No filesystem writes in rb_key.
 * Scheduled once, then returns. All results are optional diagnostics.
 */
void rb_worker(void *unused)
{
    unsigned n=0;
    unsigned i, count;
    int status, screen=0;
    (void)unused;
    rb_sleep(RB_WAIT_MS);
    status=rb_readback();
    n=rb_append(rb_late_buf,n,"REBOBINA DIAGNOSTICO 0.0.3 / PASSIVE KEY TEST\n");
    n=rb_append(rb_late_buf,n,"READBACK=");
    n=rb_append(rb_late_buf,n,status == 1 ? "PASS\n" : "FAIL_");
    if(status!=1){ n=rb_hex(rb_late_buf,n,(uint32_t)status); n=rb_append(rb_late_buf,n,"\n"); }
    count=rb_events.count;
    n=rb_append(rb_late_buf,n,"GUI_EVENTS_HEX=");
    n=rb_hex(rb_late_buf,n,count); n=rb_append(rb_late_buf,n,"\n");
    for(i=0;i<RB_ECOUNT && i<count;++i){
        n=rb_append(rb_late_buf,n,"RAW_EVENT_");
        n=rb_hex(rb_late_buf,n,i);
        n=rb_append(rb_late_buf,n,"=");
        n=rb_hex(rb_late_buf,n,rb_events.first[i]);
        n=rb_append(rb_late_buf,n,"\n");
    }
    /* Experimental screen hook: explicit opt-in marker + powered display. */
    if(rb_exists("A:/ML/RB3LCD.ON")) {
        if(rb_display_on()) { rb_screen("REBOBINA 0.0.3"); screen=1; }
        else screen=2;
    }
    n=rb_append(rb_late_buf,n,"LCD=");
    n=rb_append(rb_late_buf,n,screen==1?"CALL_ATTEMPTED\n":screen==2?"DISPLAY_OFF\n":"OPT_IN_DISABLED\n");
    (void)rb_store("A:/RB3LATE.TXT", rb_late_buf,n);
    if(rb_exists("A:/ML/RB3LED.ON")) rb_led();
}

void rb_boot(void)
{
    unsigned n=0;
    uint32_t tick0,tick1;
    int mem,task_result,io_result;
    rb_events.count=0u;
    tick0=rb_tick();
    rb_sleep(20u);
    tick1=rb_tick();
    mem=rb_memtest();
    task_result=rb_task("rb_diag",31,8192,rb_worker,0);
    n=rb_append(rb_boot_buf,n,"REBOBINA BOOT DIAGNOSTICO 0.0.3\n");
    n=rb_append(rb_boot_buf,n,"MODEL=EOS_5D_CLASSIC\nFIRMWARE_TARGET=1.1.1\n");
    n=rb_append(rb_boot_buf,n,"LOADER=SOUP_LEGACY_5DC\n");
    n=rb_append(rb_boot_buf,n,"TICK_BEFORE="); n=rb_hex(rb_boot_buf,n,tick0);
    n=rb_append(rb_boot_buf,n,"\nTICK_AFTER="); n=rb_hex(rb_boot_buf,n,tick1);
    n=rb_append(rb_boot_buf,n,"\nMEM_256="); n=rb_append(rb_boot_buf,n,mem?"PASS":"FAIL");
    n=rb_append(rb_boot_buf,n,"\nWORKER_ID_HEX="); n=rb_hex(rb_boot_buf,n,(uint32_t)task_result);
    n=rb_append(rb_boot_buf,n,"\nEVENT_HOOK=PASS_THROUGH\n");
    rb_boot_length=n;
    io_result=rb_store("A:/RB3BOOT.TXT",rb_boot_buf,n);
    /* Never rewrite rb_boot_buf after FIO_WriteFile: possible asynchronous DMA. */
    (void)io_result;
    if(rb_exists("A:/ML/RB3LED.ON")) rb_led();
}

/* Called by the Soup GUI hook. Observational only, constant time, always
 * returns 1 to let Magic Lantern/Canon process the original event.
 */
int rb_key(const void *event)
{
    uint32_t n=rb_events.count;
    if(event && ((uintptr_t)event & 3u)==0u && rb_events.magic==0x52423345u && n!=0xFFFFFFFFu && n < 0xFFFFFFFEu){
        if(n<RB_ECOUNT) rb_events.first[n] = *(const volatile uint32_t *)event;
        rb_events.count=n+1u;
    }
    return 1;
}
