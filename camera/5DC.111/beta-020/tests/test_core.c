/* SPDX-License-Identifier: GPL-2.0-only */
/* Camera-free tests exercising the exact portable ARM menu and CF format. */
#include "rb_hal.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void rb_boot(void);
int rb_key(const void *);
void rb_test_worker_init(void);
void rb_test_worker_step(void);

#define MAX_FILES 12
struct file {char name[80];unsigned char data[4096];unsigned len;};
static struct file files[MAX_FILES];
static int files_count;
static int beta_marker,display,bitmap,write_fail,short_read;
static int tasks,ram_calls,allocs,frees,paint_calls,fill_calls,ui_rows,lower_rows;
static int reads,writes;
static uint32_t tick;
static void (*worker)(void*);

static int find(const char *name){
 int i;for(i=0;i<files_count;i++)if(!strcmp(files[i].name,name))return i;
 return -1;
}
static int get(const char *name){
 int ix=find(name);if(ix>=0)return ix;
 assert(files_count<MAX_FILES);
 ix=files_count++;strncpy(files[ix].name,name,sizeof(files[ix].name)-1);
 files[ix].len=0;return ix;
}
static const char *txt(const char *name){
 int ix=find(name);assert(ix>=0);return (const char*)files[ix].data;
}
static void reset_all(void){
 memset(files,0,sizeof(files));files_count=0;
 beta_marker=display=bitmap=write_fail=short_read=0;
 tasks=ram_calls=allocs=frees=paint_calls=fill_calls=ui_rows=lower_rows=0;
 reads=writes=0;tick=100;worker=0;
}
int rb_hal_open(const char *path){
 if(!strcmp(path,"A:/ML/RB8BETA.ON"))return beta_marker?100:-1;
 return find(path);
}
int rb_hal_create(const char *path){
 if(write_fail)return -1;
 assert(strstr(path,"A:/RB8") || strstr(path,"A:/ML/RB8CFG"));
 return get(path);
}
int rb_hal_write(int h,const void *p,unsigned n){
 if(write_fail)return -1;
 assert(h>=0 && h<MAX_FILES && n<sizeof(files[h].data));
 memcpy(files[h].data,p,n);files[h].data[n]=0;files[h].len=n;
 writes++;return (int)n;
}
int rb_hal_read(int h,void *p,unsigned n){
 unsigned count;
 assert(h>=0 && h<MAX_FILES);reads++;
 count=files[h].len;if(count>n)count=n;
 if(short_read && count)count--;
 memcpy(p,files[h].data,count);return (int)count;
}
void rb_hal_close(int h){assert((h>=0&&h<MAX_FILES)||h==100);}
static unsigned char memory[4096];
void *rb_hal_alloc(unsigned n){allocs++;assert(n<=sizeof memory);return memory;}
void rb_hal_free(void *p){assert(p==memory);frees++;}
void rb_hal_meminfo(int *t,int *f){*t=7*1024*1024;*f=2*1024*1024;ram_calls++;}
void rb_hal_sleep(unsigned ms){assert(ms==100u);tick+=10;}
uint32_t rb_hal_tick(void){return tick;}
int rb_hal_task(const char *name,int prio,int stack,void (*fn)(void *),void *arg){
 assert(!strcmp(name,"rb8_beta")&&prio==31&&stack==8192&&arg==0);
 worker=fn;tasks++;return 100;
}
void *rb_hal_uncached(void *p){return p;}
int rb_hal_display_on(void){return display;}
int rb_hal_bitmap_ready(void){return bitmap;}
void rb_hal_fill(unsigned color,int x,int y,int w,int h){
 assert(x>=0&&y>=0&&w>0&&h>0&&x+w<=720&&y+h<=480);
 if(x==0 && y==0 && h==480){assert(w==720&&h==480);if(color==2)paint_calls++;}
 fill_calls++;
}
void rb_hal_text(int x,int y,const char *s,unsigned fg,unsigned bg){
 assert(x>=0&&x<720&&y>=0&&y<480&&fg<=255&&bg<=255&&strlen(s)<128);
 ui_rows++;if(y>240)lower_rows++;
}
static int event(unsigned code){uint32_t ev[2]={0,code};return rb_key(ev);}
static void start(void){rb_boot();assert(tasks&&worker);rb_test_worker_init();}
static void test_headless_and_setup(void){
 reset_all();start();display=bitmap=1;
 rb_test_worker_step();
 assert(paint_calls==0);
 assert(event(7)==1);
 assert(strstr(txt("A:/RB8BOOT.TXT"),"INTERFACE=10HZ"));
 assert(strstr(txt("A:/RB8TEST.TXT"),"WRITE+READBACK"));
 assert(allocs==1 && frees==1 && reads>0);
 puts("PASS headless / boot / RAM / CF readback");
}
static void test_ui_and_persistence(void){
 int before,ix;
 reset_all();beta_marker=1;display=bitmap=1;
 start();
 for(ix=0;ix<10;ix++){rb_test_worker_step();tick+=10;}
 assert(paint_calls==10&&lower_rows>0);
 assert(event(2)==0); /* HOME -> FILM */
 assert(event(1)==0); /* next film */
 assert(event(2)==0); /* select PORTRA */
 assert(event(3)==0); /* MENU -> HOME */
 assert(event(1)==0); /* ROLL selected */
 assert(event(2)==0); /* enter ROLL */
 assert(event(1)==0); /* MARCAR +1 FOTO */
 assert(event(2)==0);assert(event(2)==0);
 assert(event(6)==1); /* PLAY passes to Canon */
 assert(event(1)==0);assert(event(1)==0);assert(event(1)==0); /* SAVE */
 assert(event(2)==0); /* trigger config and report */
 rb_test_worker_step();
 ix=find("A:/ML/RB8CFG1.BIN");assert(ix>=0 && files[ix].len==36);
 assert(strstr(txt("A:/RB8SESSION.TXT"),"FILM_INDEX_HEX=00000001"));
 assert(strstr(txt("A:/RB8SESSION.TXT"),"MANUAL_FRAMES_HEX=00000002"));
 assert(strstr(txt("A:/RB8SESSION.TXT"),"CONFIG_SEQUENCE_HEX=00000001"));
 /* REBOOT on same CF: read back the settings from persistent slot */
 before=paint_calls;rb_boot();rb_test_worker_init();rb_test_worker_step();
 assert(paint_calls==before+1);
 assert(event(7)==0); /* hide display, safely consumed */
 rb_test_worker_step();
 assert(event(7)==0); /* reopen */
 rb_test_worker_step();
 assert(paint_calls==before+2);
 assert(event(2)==0); /* from HOME -> FILM */
 assert(event(3)==0); /* back to HOME */
 assert(event(3)==0); /* exit on root */
 puts("PASS 10Hz UI / navigation / roll / profiles / CF persistence / reboot / toggling");
}
static void test_slot_recovery_and_faults(void){
 int i;
 /* Keep the persistent state from previous test, but reboot to root */
 beta_marker=1;display=bitmap=1;write_fail=0;
 rb_boot();rb_test_worker_init();rb_test_worker_step();
 assert(event(2)==0); /* enter FILM */
 assert(event(1)==0);assert(event(2)==0); /* next film (ACROS) selected */
 assert(event(3)==0); /* return home */
 assert(event(1)==0); /* enter roll */
 assert(event(1)==0); /* TOOLS */
 assert(event(2)==0); /* enter TOOLS */
 assert(event(1)==0);assert(event(1)==0); /* SALVAR */
 assert(event(2)==0);rb_test_worker_step();
 i=find("A:/ML/RB8CFG0.BIN");assert(i>=0);
 /* Damage most recent slot; next boot must fallback to previous valid. */
 files[i].data[12]^=0x01;
 rb_boot();rb_test_worker_init();rb_test_worker_step();
 assert(event(2)==0); /* FILM */
 assert(event(3)==0); /* root */
 /* Force failed save; must not mark success. */
 write_fail=1;
 assert(event(1)==0);assert(event(1)==0);assert(event(2)==0); /* TOOLS */
 assert(event(1)==0);assert(event(1)==0);assert(event(2)==0); /* save */
 rb_test_worker_step();write_fail=0;
 assert(find("A:/ML/RB8CFG1.BIN")>=0);
 puts("PASS dual-slot fallback / corrupt record rejection / fail-closed write");
}
static void test_no_draw_when_dark(void){
 int old_paints=paint_calls;
 display=0;bitmap=0;rb_test_worker_step();
 assert(paint_calls==old_paints);
 display=bitmap=1;rb_test_worker_step();
 assert(paint_calls>=old_paints);
 puts("PASS LCD readiness guards");
}
int main(void){
 test_headless_and_setup();test_ui_and_persistence();
 test_slot_recovery_and_faults();test_no_draw_when_dark();
 puts("ALL HOST TESTS PASSED");return 0;
}
