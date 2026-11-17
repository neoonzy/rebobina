/* SPDX-License-Identifier: GPL-2.0-only */
/* Deterministic HAL fake; never talks to a camera or physical CF. */
#include "rb_hal.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static char boot[4096],check[4096],events[4096],final[4096];
static unsigned boot_len,check_len,event_len,final_len;
static unsigned fails,ui_enabled,display_enabled,draws,alloc_count,free_count;
static unsigned sleeps,run_count;
static char drawn[160];
static void (*worker)(void *);
static unsigned char heap[4096];
static uint32_t ticks=0x99u;
int rb_hal_create(const char *path){
 if(fails&1u)return -1;
 if(!strcmp(path,"A:/RB5BOOT.TXT")) return 1;
 if(!strcmp(path,"A:/RB5TEST.TXT")) return 2;
 if(!strcmp(path,"A:/RB5EVENT.TXT"))return 3;
 if(!strcmp(path,"A:/RB5FINAL.TXT"))return 4;
 assert(0&&"invalid write path");return -1;
}
int rb_hal_open(const char *p){
 if(!strcmp(p,"A:/ML/RB5UI.ON")) return ui_enabled?5:-1;
 if(!strcmp(p,"A:/RB5BOOT.TXT"))return boot_len?1:-1;
 assert(0&&"invalid open path");return -1;
}
int rb_hal_read(int h,void *b,unsigned n){assert(h==1&&n>boot_len);memcpy(b,boot,boot_len);return (int)boot_len;}
int rb_hal_write(int h,const void *src,unsigned n){
 char *p=0; unsigned *len=0;
 if(fails&2u)return (int)n-1;
 if(h==1){p=boot;len=&boot_len;} else if(h==2){p=check;len=&check_len;}
 else if(h==3){p=events;len=&event_len;} else if(h==4){p=final;len=&final_len;}
 assert(p&&n<4096); memcpy(p,src,n);p[n]=0;*len=n;return (int)n;
}
void rb_hal_close(int h){assert(h>=1 && h<=5);}
void *rb_hal_alloc(unsigned n){alloc_count++;if(fails&4u)return 0;assert(n==256u||n==4096u);return heap;}
void rb_hal_free(void *p){assert(p==heap);free_count++;}
void rb_hal_meminfo(int *total,int *free_bytes){*total=64*1024*1024;*free_bytes=2*1024*1024;}
void rb_hal_sleep(unsigned ms){assert(ms==20u||ms==1200u||ms==500u);sleeps++;}
uint32_t rb_hal_tick(void){return ticks++;}
int rb_hal_task(const char *name,int priority,int size,void (*entry)(void *),void *arg){
 assert(!strcmp(name,"rb5_diag")&&priority==31&&size==8192&&arg==0);
 if(fails&8u) return -1;
 worker=entry;return 420;
}
void *rb_hal_uncached(void *ptr){return ptr;}
int rb_hal_display_on(void){return (int)display_enabled;}
void rb_hal_text(int x,int y,const char *s){
 assert(x==12&&y>=17&&y<=203);assert(strlen(s)<95);draws++;
 if(strstr(s,"REBOBINA"))strncpy(drawn,s,sizeof(drawn)-1u);
}
void rb_boot(void);void rb_worker(void *);int rb_key(const void *);
static void reset(void){
 memset(boot,0,sizeof boot);memset(check,0,sizeof check);
 memset(events,0,sizeof events);memset(final,0,sizeof final);
 boot_len=check_len=event_len=final_len=0;
 fails=ui_enabled=display_enabled=draws=alloc_count=free_count=sleeps=run_count=0;
 worker=0;ticks=0x99u;drawn[0]=0;
}
static void inject(uint32_t param){uint32_t ev[2]={0,param};assert(rb_key(ev)==1);}
static void test_full_safe(void){
 unsigned i;
 reset();rb_boot();assert(worker&&boot_len);
 assert(strstr(boot,"REBOBINA 0.1.0 DEV / BOOT"));
 assert(strstr(boot,"MEM_256_HEX=00000001"));
 assert(strstr(boot,"MEM_4096_HEX=00000001"));
 for(i=0;i<9;i++)inject((uint32_t[]){3,11,9,4,11,9,2,2,7}[i]);
 worker(0);
 assert(check_len&&event_len&&final_len);
 assert(strstr(check,"READBACK=PASS"));
 assert(strstr(check,"FREE_RAM_HEX=00200000"));
 assert(strstr(check,"UI_MODE=SAFE_HEADLESS"));
 assert(strstr(events,"TOTAL_EVENTS_HEX=00000009"));
 assert(strstr(events,"EV_00000000_TYPE=00000000_PARAM=00000003"));
 assert(strstr(events,"EV_00000008_TYPE=00000000_PARAM=00000007"));
 assert(strstr(final,"UI_DRAW_CALLS_HEX=00000000"));
 assert(draws==0&&alloc_count==2&&free_count==2);
 printf("PASS: real camera events, memory, tasks, filesystem, readback, headless\n");
}
static void test_ui_and_truncation(void){
 unsigned i;
 reset();ui_enabled=1;display_enabled=1;rb_boot();
 inject(1);inject(2); inject(0); /* page: forward, forward, backward -> 1 */
 for(i=0;i<70;i++)inject(4);
 worker(0);
 assert(check_len&&event_len&&final_len);
 assert(strstr(check,"UI_MODE=ENABLED_BY_MARKER"));
 assert(strstr(events,"TOTAL_EVENTS_HEX=00000049"));
 assert(strstr(events,"CAPTURED_EVENTS_HEX=00000030"));
 assert(strstr(events,"DROPPED_EVENTS_HEX=00000019"));
 assert(strstr(final,"UI_PAGE_HEX=00000001"));
 assert(strstr(final,"UI_DRAW_CALLS_HEX=00000026"));
 assert(draws>100&&strstr(drawn,"REBOBINA"));
 printf("PASS: 5-page UI, navigation, 48-event cap, overflow accounting\n");
}
static void test_faults(void){
 reset();fails=4u|8u;rb_boot();
 assert(!worker);assert(strstr(boot,"MEM_256_HEX=00000000"));
 assert(strstr(boot,"WORKER_ID_HEX=FFFFFFFF"));
 reset();fails=1u;rb_boot();assert(!boot_len);assert(worker);
 worker(0);assert(!check_len&&!event_len&&!final_len);
 reset();fails=2u;rb_boot();assert(!boot_len);assert(worker);
 worker(0);assert(!check_len&&!event_len&&!final_len);
 printf("PASS: allocation/task failures, create/short-write failures\n");
}
int main(void){test_full_safe();test_ui_and_truncation();test_faults();puts("ALL HOST TESTS PASSED");return 0;}
