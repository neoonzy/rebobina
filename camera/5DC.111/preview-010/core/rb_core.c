/* SPDX-License-Identifier: GPL-2.0-only */
/* REBOBINA 0.1.0-dev: integrated proof-of-concept for EOS 5D Classic 1.1.1.
 * Do not change Canon ROM, bootflags, AUTOEXEC.BIN, DCIM or camera settings.
 * Input is never consumed; LCD overlay only with A:/ML/RB5UI.ON and Canon
 * display already powered on; no MMIO palette/display/LED writes.
 * Reuses 5DC Soup Engine loader ABI: FILM +4 boot / +0x44 GUI event.
 */
#include "rb_hal.h"
#include <stdint.h>
#include <stddef.h>

#define RB_EVENTS 48u
#define RB_CAP 4096u
#define RB_SAMPLE_COUNT 76u /* ~38 seconds, one physical session */
#define RB_MAGIC 0x52423545u
#define RB_GUI_EVENT 0u

struct rb_event { uint32_t type, param; };
static volatile struct rb_state {
    uint32_t magic, count, lost, page, last_param, ui_enabled;
    uint32_t display_seen, draws;
    struct rb_event events[RB_EVENTS];
} state = { RB_MAGIC, 0xffffffffu, 0, 0, 0xffffffffu, 0, 0, 0, {{0xffffffffu,0xffffffffu}} };

/* Allocate initialized .data: the Soup legacy loader does not clear .bss.
 * Separate, never-reused write buffers protect FIO's asynchronous DMA path.
 */
static char boot_buf[RB_CAP] = { 'B' };
static char check_buf[RB_CAP] = { 'C' };
static char events_buf[RB_CAP] = { 'E' };
static char final_buf[RB_CAP] = { 'F' };
static char read_buf[RB_CAP] = { 'R' };
static char ui_line[96] = { 'U' };
static volatile unsigned boot_len = 0xffffffffu;
static volatile uint32_t boot_write_ok = 0xffffffffu;
static volatile uint32_t check_write_ok = 0xffffffffu;
static volatile uint32_t event_write_ok = 0xffffffffu;
static volatile uint32_t mem_256_ok = 0xffffffffu;
static volatile uint32_t mem_4096_ok = 0xffffffffu;
static volatile uint32_t task_id = 0xffffffffu;

static unsigned append(char *s, unsigned n, const char *v, unsigned cap)
{
    while (*v && n + 1u < cap) s[n++]=*v++;
    if (n < cap) s[n]=0;
    return n;
}
static unsigned hex8(char *s, unsigned n, uint32_t v, unsigned cap)
{
    static const char digits[]="0123456789ABCDEF";
    unsigned i;
    for (i=0; i<8u && n+1u<cap; ++i) s[n++]=digits[(v>>(28u-4u*i))&15u];
    if (n<cap) s[n]=0;
    return n;
}
static unsigned field_hex(char *s,unsigned n,const char *name,uint32_t value,unsigned cap)
{
    n=append(s,n,name,cap);
    n=hex8(s,n,value,cap);
    return append(s,n,"\n",cap);
}
static int exists(const char *path)
{
    int h=rb_hal_open(path);
    if(h == -1) return 0;
    rb_hal_close(h);
    return 1;
}
static int store(const char *path, const char *buf, unsigned n)
{
    int h=rb_hal_create(path), r;
    if(h == -1) return 0;
    r=rb_hal_write(h,buf,n);
    rb_hal_close(h);
    return r == (int)n;
}
static int memtest(unsigned bytes)
{
    volatile unsigned char *p=(volatile unsigned char *)rb_hal_alloc(bytes);
    unsigned i;
    int ok=1;
    if(!p) return 0;
    for(i=0;i<bytes;i++) p[i]=(unsigned char)((i*29u+7u)&255u);
    for(i=0;i<bytes;i++) if(p[i]!=(unsigned char)((i*29u+7u)&255u)) ok=0;
    rb_hal_free((void *)p);
    return ok;
}
static int readback(void)
{
    unsigned i, length=boot_len;
    int h,r;
    volatile unsigned char *p;
    if(!length || length>=RB_CAP) return -2;
    h=rb_hal_open("A:/RB5BOOT.TXT");
    if(h == -1) return -1;
    /* DMA writes to physical memory, not coherent with a cached reader. */
    p=(volatile unsigned char *)rb_hal_uncached(read_buf);
    r=rb_hal_read(h,(void *)p,length+1u);
    rb_hal_close(h);
    if(r!=(int)length) return -3;
    for(i=0;i<length;i++) if(p[i]!=(unsigned char)boot_buf[i]) return -4;
    return 1;
}

/* Passively capture the first 48 raw GUI events; never block Canon/ML.
 * The second word is event->param according to the original 5DC gui.h.
 * Update count last; worker only reads slots at indices < count.
 */
int rb_key(const void *ev)
{
    if(ev && !((uintptr_t)ev & 3u) && state.magic==RB_MAGIC){
        uint32_t n=state.count;
        if(n!=0xffffffffu && n<0xfffffffeu){
            const volatile uint32_t *p=(const volatile uint32_t *)ev;
            uint32_t type=p[0], param=p[1];
            if(n<RB_EVENTS){
                state.events[n].type=type;
                state.events[n].param=param;
            }else{
                state.lost++;
            }
            state.last_param=param;
            if(type==RB_GUI_EVENT && state.ui_enabled){
                /* Navigation is OBSERVATIONAL: the original Canon action
                 * still happens because the hook always returns 1. */
                if(param==0u) state.page=(state.page+4u)%5u;
                if(param==1u || param==2u) state.page=(state.page+1u)%5u;
            }
            state.count=n+1u;
        }
    }
    return 1;
}

static const char *button_name(uint32_t p)
{
    switch(p){
    case 0x00u: return "WHEEL_UP";
    case 0x01u: return "WHEEL_DOWN";
    case 0x02u: return "SET";
    case 0x03u: return "MENU";
    case 0x04u: return "INFO";
    case 0x05u: return "JUMP";
    case 0x06u: return "PLAY";
    case 0x07u: return "TRASH";
    case 0x08u: return "ZOOMIN?";
    case 0x09u: return "ZOOMIN_UP?";
    case 0x0Au: return "ZOOMOUT?";
    case 0x0Bu: return "ZOOMOUT_UP?";
    case 0x0Fu: return "UP";
    case 0x12u: return "RIGHT";
    case 0x13u: return "LEFT";
    case 0x16u: return "DOWN";
    default: return "OTHER";
    }
}

static void line_hex(int y, const char *label, uint32_t value)
{
    unsigned n=append(ui_line,0u,label,sizeof(ui_line));
    (void)hex8(ui_line,n,value,sizeof(ui_line));
    rb_hal_text(12,y,ui_line);
}
static void render(void)
{
    uint32_t page=state.page%5u;
    rb_hal_text(12,17,"REBOBINA 0.1.0 DEV");
    if(page==0u){
        rb_hal_text(12,48,"[ STATUS ]");
        rb_hal_text(12,75,"5D CLASSIC / VxWorks");
        rb_hal_text(12,103,"BOOT + FILE I/O + TASK");
        rb_hal_text(12,131,"INPUT / MEMORY / LCD");
        line_hex(159,"EVENTS: ",state.count);
    } else if(page==1u){
        rb_hal_text(12,48,"[ MEMORY ]");
        rb_hal_text(12,75,mem_256_ok?"256 BYTES : PASS":"256 BYTES : FAIL");
        rb_hal_text(12,103,mem_4096_ok?"4096 BYTES: PASS":"4096 BYTES: FAIL");
        line_hex(131,"TASK ID: ",task_id);
    } else if(page==2u){
        rb_hal_text(12,48,"[ INPUT ]");
        line_hex(75,"EVENTS: ",state.count);
        line_hex(103,"LAST: 0x",state.last_param);
        rb_hal_text(12,131,button_name(state.last_param));
        rb_hal_text(12,159,"ALL KEYS PASS THROUGH");
    } else if(page==3u){
        rb_hal_text(12,48,"[ STORAGE ]");
        rb_hal_text(12,75,boot_write_ok?"BOOT FILE: PASS":"BOOT FILE: FAIL");
        rb_hal_text(12,103,check_write_ok?"TEST FILE: PASS":"TEST FILE: FAIL");
        rb_hal_text(12,131,"READBACK IN RB5TEST.TXT");
    } else {
        rb_hal_text(12,48,"[ ABOUT ]");
        rb_hal_text(12,75,"REBOBINA / GPLV2");
        rb_hal_text(12,103,"SOUP LOADER / MAGIC LANTERN");
        rb_hal_text(12,131,"EXPERIMENTAL 5DC.111");
        rb_hal_text(12,159,"NO ROM / BOOTFLAG WRITES");
    }
    rb_hal_text(12,203,"SET OR WHEEL: NEXT PAGE");
    state.draws++;
}

/* A single worker does the entire integrated test. It never writes files
 * while inside the GUI hook, never touches photos or camera configuration.
 */
void rb_worker(void *arg)
{
    unsigned n=0u, i, count;
    uint32_t start_tick=rb_hal_tick(), end_tick=0u;
    int read_ok, total=0, freebytes=0;
    (void)arg;
    rb_hal_sleep(1200u);
    read_ok=readback();
    rb_hal_meminfo(&total,&freebytes);
    n=append(check_buf,n,"REBOBINA 0.1.0 DEV / INTEGRATED TEST\n",RB_CAP);
    n=append(check_buf,n,read_ok==1?"READBACK=PASS\n":"READBACK=FAIL\n",RB_CAP);
    n=field_hex(check_buf,n,"READBACK_STATUS_HEX=",(uint32_t)read_ok,RB_CAP);
    n=field_hex(check_buf,n,"MEM_256=",mem_256_ok,RB_CAP);
    n=field_hex(check_buf,n,"MEM_4096=",mem_4096_ok,RB_CAP);
    n=field_hex(check_buf,n,"TOTAL_RAM_HEX=",(uint32_t)total,RB_CAP);
    n=field_hex(check_buf,n,"FREE_RAM_HEX=",(uint32_t)freebytes,RB_CAP);
    n=field_hex(check_buf,n,"TICK_START_HEX=",start_tick,RB_CAP);
    n=append(check_buf,n,"UI_MODE=",RB_CAP);
    n=append(check_buf,n,state.ui_enabled?"ENABLED_BY_MARKER\n":"SAFE_HEADLESS\n",RB_CAP);
    check_write_ok=(uint32_t)store("A:/RB5TEST.TXT",check_buf,n);
    /* 38 seconds. One draw at most every 1000ms; no auto panel power-on. */
    for(i=0;i<RB_SAMPLE_COUNT;i++){
        if(state.ui_enabled && rb_hal_display_on()){
            state.display_seen++;
            if((i & 1u)==0u) render();
        }
        rb_hal_sleep(500u);
    }
    count=state.count;
    if(count>RB_EVENTS) count=RB_EVENTS;
    n=append(events_buf,0u,"REBOBINA 0.1.0 DEV / GUI EVENTS\n",RB_CAP);
    n=field_hex(events_buf,n,"TOTAL_EVENTS_HEX=",state.count,RB_CAP);
    n=field_hex(events_buf,n,"CAPTURED_EVENTS_HEX=",count,RB_CAP);
    n=field_hex(events_buf,n,"DROPPED_EVENTS_HEX=",state.lost,RB_CAP);
    for(i=0u;i<count;i++){
        n=append(events_buf,n,"EV_",RB_CAP);
        n=hex8(events_buf,n,i,RB_CAP);
        n=append(events_buf,n,"_TYPE=",RB_CAP);
        n=hex8(events_buf,n,state.events[i].type,RB_CAP);
        n=append(events_buf,n,"_PARAM=",RB_CAP);
        n=hex8(events_buf,n,state.events[i].param,RB_CAP);
        n=append(events_buf,n,"\n",RB_CAP);
    }
    event_write_ok=(uint32_t)store("A:/RB5EVENT.TXT",events_buf,n);
    end_tick=rb_hal_tick();
    n=append(final_buf,0u,"REBOBINA 0.1.0 DEV / FINAL REPORT\n",RB_CAP);
    n=field_hex(final_buf,n,"TICK_END_HEX=",end_tick,RB_CAP);
    n=field_hex(final_buf,n,"EVENTS_TOTAL_HEX=",state.count,RB_CAP);
    n=field_hex(final_buf,n,"LAST_PARAM_HEX=",state.last_param,RB_CAP);
    n=field_hex(final_buf,n,"UI_PAGE_HEX=",state.page,RB_CAP);
    n=field_hex(final_buf,n,"UI_DRAW_CALLS_HEX=",state.draws,RB_CAP);
    n=field_hex(final_buf,n,"DISPLAY_ON_SAMPLES_HEX=",state.display_seen,RB_CAP);
    n=field_hex(final_buf,n,"BOOT_WRITE_OK=",boot_write_ok,RB_CAP);
    n=field_hex(final_buf,n,"TEST_WRITE_OK=",check_write_ok,RB_CAP);
    n=field_hex(final_buf,n,"EVENT_WRITE_OK=",event_write_ok,RB_CAP);
    n=append(final_buf,n,"NOTE=UI_DRAW_CALLS_IS_NOT_VISUAL_CONFIRMATION\n",RB_CAP);
    (void)store("A:/RB5FINAL.TXT",final_buf,n);
}

void rb_boot(void)
{
    uint32_t tick0,tick1;
    unsigned n=0u;
    int tid;
    state.magic=RB_MAGIC;
    state.count=0u;
    state.lost=0u;
    state.page=0u;
    state.last_param=0xffffffffu;
    state.draws=0u;
    state.display_seen=0u;
    state.ui_enabled=(uint32_t)exists("A:/ML/RB5UI.ON");
    tick0=rb_hal_tick();
    rb_hal_sleep(20u);
    tick1=rb_hal_tick();
    mem_256_ok=(uint32_t)memtest(256u);
    mem_4096_ok=(uint32_t)memtest(4096u);
    tid=rb_hal_task("rb5_diag",31,8192,rb_worker,0);
    task_id=(uint32_t)tid;
    n=append(boot_buf,n,"REBOBINA 0.1.0 DEV / BOOT\n",RB_CAP);
    n=append(boot_buf,n,"TARGET=5DC.111 / SOUP LEGACY LOADER\n",RB_CAP);
    n=field_hex(boot_buf,n,"TICK_BEFORE_HEX=",tick0,RB_CAP);
    n=field_hex(boot_buf,n,"TICK_AFTER_HEX=",tick1,RB_CAP);
    n=field_hex(boot_buf,n,"MEM_256_HEX=",mem_256_ok,RB_CAP);
    n=field_hex(boot_buf,n,"MEM_4096_HEX=",mem_4096_ok,RB_CAP);
    n=field_hex(boot_buf,n,"WORKER_ID_HEX=",task_id,RB_CAP);
    n=field_hex(boot_buf,n,"UI_ENABLED_HEX=",state.ui_enabled,RB_CAP);
    n=append(boot_buf,n,"EVENT_HOOK=PASS_THROUGH\n",RB_CAP);
    boot_len=n;
    boot_write_ok=(uint32_t)store("A:/RB5BOOT.TXT",boot_buf,n);
}
