/* SPDX-License-Identifier: GPL-2.0-only */
/* REBOBINA 0.3.0-beta: Canon EOS 5D Classic 1.1.1 / patched Soup loader.
 * Own interface and persistent roll/profile settings; no ROM/bootflag writes.
 * Soup look and CR2 processing exist on HOST ONLY, not in this payload.
 */
#include "rb_hal.h"
#include <stdint.h>
#include <stddef.h>

#define RB_MAGIC 0x52423242u
#define RB_CONFIG_MAGIC 0x52424346u
#define RB_CONFIG_VERSION 1u
#define RB_EVENT_CAP 64u
#define RB_FILE_CAP 3072u
#define RB_MENU_ITEMS 6u
#define RB_FILMS 13u
#define RB_TICK_MS 100u /* maximum 10 complete display refreshes / second */
#define RB_ROLL_ON 1u
#define RB_DIAG_ON 2u
#define RB_FLAG_AUTO_SAVE 1u
#define RB_FLAG_DEBUG 2u
#define RB_FLAG_START_OPEN 4u
#define RB_REQ_SAVE 1u
#define RB_REQ_REPORT 2u
#define RB_REQ_RAM 4u
#define RB_REQ_CF 8u
#define RB_PAGE_HOME 0u
#define RB_PAGE_FILM 1u
#define RB_PAGE_ROLL 2u
#define RB_PAGE_TOOLS 3u
#define RB_PAGE_MODULES 4u
#define RB_PAGE_SETTINGS 5u
#define RB_PAGE_ABOUT 6u
#define RB_STATUS_IDLE 0u
#define RB_STATUS_SAVED 1u
#define RB_STATUS_SAVE_FAIL 2u
#define RB_STATUS_LOADED 3u
#define RB_STATUS_NO_CONFIG 4u
#define RB_STATUS_RAM_OK 5u
#define RB_STATUS_RAM_FAIL 6u
#define RB_STATUS_CF_OK 7u
#define RB_STATUS_CF_FAIL 8u
#define RB_STATUS_HOST_ONLY 9u
#define RB_STATUS_PENDING 10u

/* Fixed size binary config; in-camera write is to Rebobina-owned files only.
 * Two alternating slots; at least one previous valid slot survives failure.
 * All fields are uint32_t on ARM. CRC-like FNV-1a checks accidental damage.
 */
struct rb_config {
    uint32_t magic, version, sequence, film, roll, frames, modules, flags, sum;
};

typedef struct rb_evt { uint32_t type, param; } rb_evt;
static volatile struct rb_state {
    uint32_t magic, ready, beta_enabled, menu_want, menu_active, page, selected;
    uint32_t film, roll, frames, modules, flags, dirty, requests;
    uint32_t config_seq, saved_slot, config_status;
    uint32_t event_total, event_lost, last_key, redraws, fullscreen_fills;
    uint32_t tick_boot, total_ram, free_ram, test_ram, test_cf;
    uint32_t status, last_input_tick, last_save_tick, ui_shown;
    rb_evt events[RB_EVENT_CAP];
} g = { .magic=RB_MAGIC, .film=0u, .roll=1u, .modules=RB_ROLL_ON|RB_DIAG_ON,
        .flags=RB_FLAG_AUTO_SAVE|RB_FLAG_START_OPEN, .last_key=0xffffffffu,
        .events={{0xffffffffu,0xffffffffu}} };

/* Initialized .data only. The Soup loader never zeroes .bss. */
static char boot_log[RB_FILE_CAP]={'B'};
static char session_log[RB_FILE_CAP]={'S'};
static char io_log[128]={'I'};
static char read_log[128]={'R'};
static char screen_line[128]={'T'};
static struct rb_config cfg_to_write[2] = {{.magic=RB_CONFIG_MAGIC},{.magic=RB_CONFIG_MAGIC}};
static uint32_t cfg_read_raw[2][12] = {{0xFFFFFFFFu},{0xFFFFFFFFu}};
static volatile uint32_t worker_cycle=0u;

static const char *const film_names[RB_FILMS]={
 "SOUP 800","PORTRA","ACROS","CINE / ETERNA",
 "ASTIA","BLEACH BYPASS","CLASSIC CHROME","CLASSIC NEG",
 "GOLD","NOSTALGIC NEG","PROVIA","REALA ACE","VELVIA"
};
static const char *const home_names[RB_MENU_ITEMS]={
 "FILM / PERFIS","ROLL / CADERNO","FERRAMENTAS","MODULOS","PREFERENCIAS","SOBRE"
};
static const char *const roll_names[5]={
 "NOVO ROLO","MARCAR +1 FOTO","DESFAZER -1 FOTO","ZERAR CONTADOR","SALVAR CADERNO"
};
static const char *const tool_names[5]={
 "TESTAR MEMORIA","TESTAR CF","SALVAR PREFERENCIAS","EXPORTAR RELATORIO","VER DIAGNOSTICOS"
};
static const char *const module_names[4]={
 "CADERNO DE ROLOS","DIAGNOSTICOS","SOUP / NO COMPUTADOR","CR2 / NO COMPUTADOR"
};
static const char *const settings_names[5]={
 "SALVAR AUTOMATICAMENTE","MOSTRAR DADOS TECNICOS","ABRIR COM LCD","SALVAR AGORA","RESTAURAR PADRAO"
};

static unsigned app(char *dst,unsigned len,unsigned cap,const char *s){
 while(*s && len+1u<cap) dst[len++]=*s++;
 dst[len]=0; return len;
}
static unsigned hexd(char *dst,unsigned n,unsigned cap,uint32_t v){
 static const char hex[]="0123456789ABCDEF";
 unsigned k;
 for(k=0;k<8u && n+1u<cap;k++)dst[n++]=hex[(v>>(28u-4u*k))&15u];
 dst[n]=0;return n;
}
static uint32_t divide10(uint32_t v,unsigned *rem){
 /* ARM946E-S has no hardware DIV; no libgcc in a freestanding payload. */
 uint32_t q=0u,r=0u;unsigned b;
 for(b=32u;b>0u;b--){
  r=(r<<1)|((v>>(b-1u))&1u);
  if(r>=10u){r-=10u;q|=1u<<(b-1u);}
 }
 *rem=(unsigned)r;return q;
}
static unsigned decd(char *dst,unsigned n,unsigned cap,uint32_t v){
 char nums[11];unsigned size=0u,rem;
 do{v=divide10(v,&rem);nums[size++]=(char)('0'+rem);}while(v && size<10u);
 while(size && n+1u<cap)dst[n++]=nums[--size];
 dst[n]=0;return n;
}
static unsigned field(char *dst,unsigned n,unsigned cap,const char *key,uint32_t v){
 n=app(dst,n,cap,key);n=hexd(dst,n,cap,v);return app(dst,n,cap,"\n");
}
static int exists(const char *name){int h=rb_hal_open(name);if(h<0)return 0;rb_hal_close(h);return 1;}
static int write_file(const char *name,const void *data,unsigned len){
 int h=rb_hal_create(name),written;
 if(h<0)return 0;
 written=rb_hal_write(h,data,len);rb_hal_close(h);
 return written==(int)len;
}
static uint32_t sum_words(const struct rb_config *c){
 uint32_t hash=2166136261u;const uint32_t *p=(const uint32_t*)c;
 unsigned i,b;
 for(i=0;i<8u;i++)for(b=0;b<4u;b++){
  hash^=(p[i]>>(b*8u))&255u;
  hash*=16777619u;
 }
 return hash;
}
static int valid_config(const struct rb_config *c){
 return c->magic==RB_CONFIG_MAGIC && c->version==RB_CONFIG_VERSION &&
   c->film<RB_FILMS && c->roll>=1u && c->roll<=9999u && c->frames<=36u &&
   !(c->modules & ~(RB_ROLL_ON|RB_DIAG_ON)) &&
   !(c->flags & ~(RB_FLAG_AUTO_SAVE|RB_FLAG_DEBUG|RB_FLAG_START_OPEN)) &&
   c->sum==sum_words(c);
}
static const char *cfg_path(unsigned slot){
 return slot?"A:/ML/RB9CFG1.BIN":"A:/ML/RB9CFG0.BIN";
}
static const char *legacy_cfg_path(unsigned slot){
 return slot?"A:/ML/RB8CFG1.BIN":"A:/ML/RB8CFG0.BIN";
}
static int read_config(unsigned slot,struct rb_config *out,int legacy){
 int h=rb_hal_open(legacy?legacy_cfg_path(slot):cfg_path(slot)),len;
 volatile uint8_t *src;uint8_t *dst;unsigned i;
 if(h<0)return 0;
 src=(volatile uint8_t*)rb_hal_uncached(cfg_read_raw[slot]);
 len=rb_hal_read(h,(void*)src,sizeof(struct rb_config)+1u);
 rb_hal_close(h);
 if(len!=(int)sizeof(struct rb_config))return 0;
 dst=(uint8_t*)out;
 for(i=0;i<sizeof(struct rb_config);i++)dst[i]=src[i];
 return valid_config(out);
}
static void load_config(void){
 struct rb_config a,b;
 int ok0=read_config(0u,&a,0),ok1=read_config(1u,&b,0);
 int from_legacy=0;
 const struct rb_config *selected=0;
 /* Import from 0.2 but NEVER overwrite its RB8CFG files: rollback
  * to the older binary must remain possible, even after index 4..12. */
 if(!ok0 && !ok1){
  ok0=read_config(0u,&a,1);
  ok1=read_config(1u,&b,1);
  from_legacy=1;
 }
 if(ok0 && ok1)selected=((int32_t)(b.sequence-a.sequence)>0)?&b:&a;
 else if(ok0)selected=&a;else if(ok1)selected=&b;
 if(selected){
  g.film=selected->film;g.roll=selected->roll;g.frames=selected->frames;
  g.modules=selected->modules;g.flags=selected->flags;
  g.config_seq=from_legacy?0u:selected->sequence;
  g.saved_slot=from_legacy?0xffffffffu:((selected==&a)?0u:1u);
  g.config_status=1u;g.status=RB_STATUS_LOADED;
 }else{g.config_seq=0u;g.saved_slot=0xffffffffu;g.config_status=0u;g.status=RB_STATUS_NO_CONFIG;}
 g.menu_want=!!(g.flags&RB_FLAG_START_OPEN);
}
static int save_config(void){
 unsigned slot=(g.config_seq+1u)&1u;
 struct rb_config *c=&cfg_to_write[slot];
 c->magic=RB_CONFIG_MAGIC;c->version=RB_CONFIG_VERSION;
 c->sequence=g.config_seq+1u;c->film=g.film;c->roll=g.roll;
 c->frames=g.frames;c->modules=g.modules;c->flags=g.flags;
 c->sum=sum_words(c);
 if(!write_file(cfg_path(slot),c,sizeof(*c))){g.status=RB_STATUS_SAVE_FAIL;return 0;}
 g.config_seq=c->sequence;g.saved_slot=slot;g.config_status=1u;
 g.dirty=0u;g.last_save_tick=rb_hal_tick();g.status=RB_STATUS_SAVED;
 return 1;
}
static void changed(void){g.dirty=1u;g.status=RB_STATUS_PENDING;g.last_input_tick=rb_hal_tick();}
static void defaults(void){
 g.film=0u;g.roll=1u;g.frames=0u;
 g.modules=RB_ROLL_ON|RB_DIAG_ON;g.flags=RB_FLAG_AUTO_SAVE|RB_FLAG_START_OPEN;
 changed();
}
static uint32_t memtest(unsigned bytes){
 volatile uint8_t *buf=(volatile uint8_t*)rb_hal_alloc(bytes);unsigned i;int ok=1;
 if(!buf)return 0u;
 for(i=0;i<bytes;i++)buf[i]=(uint8_t)((i*29u+7u)&255u);
 for(i=0;i<bytes;i++)if(buf[i]!=(uint8_t)((i*29u+7u)&255u))ok=0;
 rb_hal_free((void*)buf);
 return (uint32_t)ok;
}
static int test_card(void){
 unsigned n=0u,i;volatile uint8_t *read_back;
 int h,readbytes;
 n=app(io_log,n,sizeof(io_log),"REBOBINA 0.3.0 BETA / CF TEST\n");
 n=app(io_log,n,sizeof(io_log),"WRITE+READBACK+VERIFY\n");
 if(!write_file("A:/RB9TEST.TXT",io_log,n))return 0;
 h=rb_hal_open("A:/RB9TEST.TXT");if(h<0)return 0;
 read_back=(volatile uint8_t*)rb_hal_uncached(read_log);
 readbytes=rb_hal_read(h,(void*)read_back,n+1u);
 rb_hal_close(h);
 if(readbytes!=(int)n)return 0;
 for(i=0;i<n;i++)if(read_back[i]!=(uint8_t)io_log[i])return 0;
 return 1;
}
static unsigned write_session(char *dst){
 unsigned n=0u;
 n=app(dst,n,RB_FILE_CAP,"REBOBINA 0.3.0 BETA / SESSION\n");
 n=field(dst,n,RB_FILE_CAP,"FILM_INDEX_HEX=",g.film);
 n=field(dst,n,RB_FILE_CAP,"ROLL_HEX=",g.roll);
 n=field(dst,n,RB_FILE_CAP,"MANUAL_FRAMES_HEX=",g.frames);
 n=field(dst,n,RB_FILE_CAP,"MODULES_HEX=",g.modules);
 n=field(dst,n,RB_FILE_CAP,"SETTINGS_HEX=",g.flags);
 n=field(dst,n,RB_FILE_CAP,"CONFIG_SEQUENCE_HEX=",g.config_seq);
 n=field(dst,n,RB_FILE_CAP,"CONFIG_STATUS_HEX=",g.config_status);
 n=field(dst,n,RB_FILE_CAP,"GUI_EVENTS_HEX=",g.event_total);
 n=field(dst,n,RB_FILE_CAP,"EVENTS_DROPPED_HEX=",g.event_lost);
 n=field(dst,n,RB_FILE_CAP,"LAST_KEY_HEX=",g.last_key);
 n=field(dst,n,RB_FILE_CAP,"FRAME_DRAWS_HEX=",g.redraws);
 n=field(dst,n,RB_FILE_CAP,"BACKGROUND_FILLS_HEX=",g.fullscreen_fills);
 n=field(dst,n,RB_FILE_CAP,"TOTAL_RAM_HEX=",g.total_ram);
 n=field(dst,n,RB_FILE_CAP,"FREE_RAM_HEX=",g.free_ram);
 n=field(dst,n,RB_FILE_CAP,"RAM_TEST_HEX=",g.test_ram);
 n=field(dst,n,RB_FILE_CAP,"CF_TEST_HEX=",g.test_cf);
 n=field(dst,n,RB_FILE_CAP,"TICK_HEX=",rb_hal_tick());
 n=app(dst,n,RB_FILE_CAP,"NOTE=FILM_PRESET_IS_CONFIGURATION_ONLY\n");
 n=app(dst,n,RB_FILE_CAP,"NOTE=ROLL_FRAMES_ARE_MANUAL\n");
 return n;
}

/* All GUI hook handling is bounded, allocation-free, file-free and
 * fall-through for Canon shutter/exposure events and unknown commands. */
static unsigned length_of_page(uint32_t page){
 switch(page){
 case RB_PAGE_HOME:return RB_MENU_ITEMS;
 case RB_PAGE_FILM:return RB_FILMS;
 case RB_PAGE_ROLL:return 5u;
 case RB_PAGE_TOOLS:return 5u;
 case RB_PAGE_MODULES:return 4u;
 case RB_PAGE_SETTINGS:return 5u;
 default:return 1u;
 }
}
static void enter(uint32_t page){
 g.page=page;
 g.selected=(page==RB_PAGE_FILM)?g.film:0u;
}
static void chosen(void){
 unsigned sel=g.selected;
 switch(g.page){
 case RB_PAGE_HOME: enter(sel+1u);break;
 case RB_PAGE_FILM:
  if(g.film!=sel){g.film=sel;changed();}
  break;
 case RB_PAGE_ROLL:
  if(!(g.modules&RB_ROLL_ON))break;
  if(sel==0u){if(g.roll<9999u){g.roll++;g.frames=0;changed();}}
  else if(sel==1u){if(g.frames<36u){g.frames++;changed();}}
  else if(sel==2u){if(g.frames>0u){g.frames--;changed();}}
  else if(sel==3u){g.frames=0u;changed();}
  else if(sel==4u)g.requests|=RB_REQ_SAVE|RB_REQ_REPORT;
  break;
 case RB_PAGE_TOOLS:
  if(sel==0u)g.requests|=RB_REQ_RAM;
  else if(sel==1u)g.requests|=RB_REQ_CF;
  else if(sel==2u)g.requests|=RB_REQ_SAVE;
  else if(sel==3u)g.requests|=RB_REQ_REPORT;
  else if(sel==4u)g.flags^=RB_FLAG_DEBUG;
  break;
 case RB_PAGE_MODULES:
  if(sel==0u){g.modules^=RB_ROLL_ON;changed();}
  else if(sel==1u){g.modules^=RB_DIAG_ON;changed();}
  else g.status=RB_STATUS_HOST_ONLY;
  break;
 case RB_PAGE_SETTINGS:
  if(sel==0u){g.flags^=RB_FLAG_AUTO_SAVE;changed();}
  else if(sel==1u){g.flags^=RB_FLAG_DEBUG;changed();}
  else if(sel==2u){g.flags^=RB_FLAG_START_OPEN;changed();}
  else if(sel==3u)g.requests|=RB_REQ_SAVE;
  else if(sel==4u)defaults();
  break;
 default:break;
 }
}
int rb_key(const void *event){
 const volatile uint32_t *e;
 uint32_t type,param,index;
 if(!event || ((uintptr_t)event&3u) || g.magic!=RB_MAGIC || !g.ready)return 1;
 e=(const volatile uint32_t*)event;type=e[0];param=e[1];
 index=g.event_total;
 if(index<RB_EVENT_CAP){g.events[index].type=type;g.events[index].param=param;}
 else if(index<0xfffffffeu)g.event_lost++;
 if(index<0xfffffffeu)g.event_total=index+1u;
 g.last_key=param;
 if(type!=0u)return 1;
 /* Do not leak TRASH to the legacy ML menu while LCD is still dark. */
 if(param==7u && g.beta_enabled){
  g.menu_want=(!g.menu_want);g.last_input_tick=rb_hal_tick();
  return 0;
 }
 if(!g.ui_shown || !g.menu_active)return 1;
 if(param==3u){
  if(g.page==RB_PAGE_HOME)g.menu_want=0u;
  else enter(RB_PAGE_HOME);
  g.last_input_tick=rb_hal_tick();return 0;
 }
 if(param==4u){enter(RB_PAGE_ABOUT);g.last_input_tick=rb_hal_tick();return 0;}
 if(param==0u || param==15u || param==19u){
  unsigned n=length_of_page(g.page);
  g.selected=g.selected==0u?n-1u:g.selected-1u;
  g.last_input_tick=rb_hal_tick();return 0;
 }
 if(param==1u || param==18u || param==22u){
  unsigned n=length_of_page(g.page);
  g.selected=g.selected+1u;if(g.selected>=n)g.selected=0u;
  g.last_input_tick=rb_hal_tick();return 0;
 }
 if(param==2u){chosen();g.last_input_tick=rb_hal_tick();return 0;}
 return 1; /* Never consume shutter, playback, zoom or unknown events. */
}

static const char *page_name(unsigned p){
 switch(p){
 case RB_PAGE_HOME:return "INICIO";
 case RB_PAGE_FILM:return "FILM";
 case RB_PAGE_ROLL:return "ROLL";
 case RB_PAGE_TOOLS:return "FERRAMENTAS";
 case RB_PAGE_MODULES:return "MODULOS";
 case RB_PAGE_SETTINGS:return "PREFERENCIAS";
 default:return "SOBRE";
 }
}
static const char *status_name(unsigned s){
 switch(s){
 case RB_STATUS_SAVED:return "SALVO NO CF";
 case RB_STATUS_SAVE_FAIL:return "ERRO: NAO SALVOU";
 case RB_STATUS_LOADED:return "CONFIG CARREGADA";
 case RB_STATUS_NO_CONFIG:return "NOVA CONFIGURACAO";
 case RB_STATUS_RAM_OK:return "MEMORIA: OK";
 case RB_STATUS_RAM_FAIL:return "MEMORIA: FALHOU";
 case RB_STATUS_CF_OK:return "CF: LEITURA OK";
 case RB_STATUS_CF_FAIL:return "CF: FALHA";
 case RB_STATUS_HOST_ONLY:return "RECURSO SO NO HOST";
 case RB_STATUS_PENDING:return "ALTERACOES PENDENTES";
 default:return "PRONTO";
 }
}
static void text_ui(int x,int y,const char *s,unsigned fg,unsigned bg){rb_hal_text(x,y,s,fg,bg);}
static void text_num(int x,int y,const char *prefix,uint32_t n){
 unsigned len=app(screen_line,0u,sizeof(screen_line),prefix);
 (void)decd(screen_line,len,sizeof(screen_line),n);
 text_ui(x,y,screen_line,1u,2u);
}
static void text_hex(int x,int y,const char *prefix,uint32_t n){
 unsigned len=app(screen_line,0u,sizeof(screen_line),prefix);
 (void)hexd(screen_line,len,sizeof(screen_line),n);
 text_ui(x,y,screen_line,1u,2u);
}
static void row_mark(unsigned k,const char *s,int selected){
 int y=126+(int)k*49;
 if(selected)rb_hal_fill(5u,18,y-7,684,44);
 text_ui(35,y,selected?">":" ",selected?2u:1u,selected?5u:2u);
 text_ui(64,y,s,selected?2u:1u,selected?5u:2u);
}
static void row(unsigned k,const char *s){row_mark(k,s,g.selected==k);}
static void draw(void){
 unsigned i,accent=5u;
 if(g.page==RB_PAGE_FILM)accent=15u;
 else if(g.page==RB_PAGE_ROLL)accent=6u;
 else if(g.page==RB_PAGE_MODULES)accent=8u;
 rb_hal_fill(2u,0,0,720,480);
 rb_hal_fill(accent,0,0,720,14);
 text_ui(24,27,"REBOBINA",1u,2u);
 text_ui(24,64,page_name(g.page),accent,2u);
 text_ui(430,27,"BETA 0.3",1u,2u);
 rb_hal_fill(accent,22,102,676,5);
 switch(g.page){
 case RB_PAGE_HOME:
  for(i=0;i<RB_MENU_ITEMS;i++)row(i,home_names[i]);
  break;
 case RB_PAGE_FILM:{
  unsigned first=g.selected>=3u?g.selected-3u:0u;
  if(first>RB_FILMS-4u)first=RB_FILMS-4u;
  for(i=0u;i<4u;i++)row_mark(i,film_names[first+i],g.selected==first+i);
  text_ui(26,341,"PERFIS PARA REVELACAO NO PC",15u,2u);
  text_num(26,385,"ATIVO: ",g.film+1u);
  text_num(400,385,"DE 13: ",g.selected+1u);
  break;
 }
 case RB_PAGE_ROLL:
  for(i=0;i<5u;i++)row(i,roll_names[i]);
  text_num(28,385,"ROLO: ",g.roll);
  text_num(320,385,"FOTOS: ",g.frames);
  break;
 case RB_PAGE_TOOLS:
  for(i=0;i<5u;i++)row(i,tool_names[i]);
  text_num(28,384,"RAM LIVRE KB: ",g.free_ram/1024u);
  break;
 case RB_PAGE_MODULES:
  for(i=0;i<4u;i++)row(i,module_names[i]);
  text_ui(26,340,(g.modules&RB_ROLL_ON)?"ROLL: LIGADO":"ROLL: DESLIGADO",6u,2u);
  text_ui(26,378,(g.modules&RB_DIAG_ON)?"DIAG: LIGADO":"DIAG: DESLIGADO",6u,2u);
  break;
 case RB_PAGE_SETTINGS:
  for(i=0;i<5u;i++)row(i,settings_names[i]);
  text_ui(26,381,(g.flags&RB_FLAG_AUTO_SAVE)?"AUTO-SAVE: LIGADO":"AUTO-SAVE: DESLIGADO",6u,2u);
  break;
 case RB_PAGE_ABOUT:
  text_ui(25,132,"CAMERA: CANON EOS 5D CLASSIC",1u,2u);
  text_ui(25,183,"REBOBINA / SOUP COMPAT",1u,2u);
  text_ui(25,234,"BETA: MENU / ROLOS / CF / MODULOS",1u,2u);
  text_ui(25,285,"FILTROS CR2: DISPONIVEIS NO HOST",15u,2u);
  text_ui(25,336,"SEM ALTERACAO DA ROM OU BOOTFLAGS",1u,2u);
  break;
 }
 text_ui(22,400,status_name(g.status),6u,2u);
 rb_hal_fill(5u,15,426,690,3);
 text_ui(20,437,"SET: ABRIR   MENU: VOLTAR   LIXEIRA: OCULTAR",1u,2u);
 if(g.flags&RB_FLAG_DEBUG)text_hex(350,388,"EV:",g.event_total);
 g.redraws++;
}

static void report_boot(void){
 unsigned n=0u;
 n=app(boot_log,n,RB_FILE_CAP,"REBOBINA 0.3.0 BETA / BOOT\n");
 n=app(boot_log,n,RB_FILE_CAP,"TARGET=5DC.111 / SOUP LEGACY LOADER\n");
 n=field(boot_log,n,RB_FILE_CAP,"TICK_BOOT_HEX=",g.tick_boot);
 n=field(boot_log,n,RB_FILE_CAP,"WORKER_START_HEX=",g.ready);
 n=field(boot_log,n,RB_FILE_CAP,"UI_MARKER_HEX=",g.beta_enabled);
 n=field(boot_log,n,RB_FILE_CAP,"RAM_TEST_HEX=",g.test_ram);
 n=field(boot_log,n,RB_FILE_CAP,"CF_TEST_HEX=",g.test_cf);
 n=app(boot_log,n,RB_FILE_CAP,"INTERFACE=10HZ LCD 720x480\n");
 n=app(boot_log,n,RB_FILE_CAP,"SOUP_PHOTO_PROCESSING=HOST_ONLY\n");
 (void)write_file("A:/RB9BOOT.TXT",boot_log,n);
}

static void worker_init(void){
 int total,freebytes;
 load_config();
 g.test_ram=memtest(4096u);
 g.test_cf=(uint32_t)test_card();
 rb_hal_meminfo(&total,&freebytes);
 g.total_ram=(uint32_t)total;g.free_ram=(uint32_t)freebytes;
 g.ready=1u;
 report_boot();
}

static void worker_step(void){
 uint32_t req; int total,freebytes;
 {
  /* Never paint if Canon has torn down the bitmap or LCD is dark. */
  g.ui_shown=(uint32_t)(g.beta_enabled&&rb_hal_display_on()&&rb_hal_bitmap_ready());
  if(!g.ui_shown)g.menu_active=0u;
  else if(g.menu_want){g.menu_active=1u;draw();g.fullscreen_fills++;}
  else if(g.menu_active){
    /* Transition to transparent overlay; no Canon property writes. */
    rb_hal_fill(0u,0,0,720,480);
    g.menu_active=0u;
  }
  /* File operations are not done from the GUI task. */
  req=g.requests;
  if(req){g.requests=0u;
    if(req&RB_REQ_RAM){g.test_ram=memtest(4096u);g.status=g.test_ram?RB_STATUS_RAM_OK:RB_STATUS_RAM_FAIL;}
    if(req&RB_REQ_CF){g.test_cf=(uint32_t)test_card();g.status=g.test_cf?RB_STATUS_CF_OK:RB_STATUS_CF_FAIL;}
    if(req&RB_REQ_SAVE)(void)save_config();
    if(req&RB_REQ_REPORT){unsigned n=write_session(session_log);
      (void)write_file("A:/RB9SESSION.TXT",session_log,n);
    }
  }
  if(((worker_cycle++&7u)==0u) && (g.modules&RB_DIAG_ON)){
    rb_hal_meminfo(&total,&freebytes);
    g.total_ram=(uint32_t)total;g.free_ram=(uint32_t)freebytes;
  }
  /* Automatic save is idle-debounced; at most once per 3 seconds. */
  if(g.dirty && (g.flags&RB_FLAG_AUTO_SAVE)){
    uint32_t now=rb_hal_tick();
    if((uint32_t)(now-g.last_input_tick)>=300u &&
       (uint32_t)(now-g.last_save_tick)>=300u) (void)save_config();
  }
 }
}

#ifdef RB_BETA_HOST_TEST
void rb_test_worker_init(void){worker_init();}
void rb_test_worker_step(void){worker_step();}
#endif
void rb_worker(void *arg){
 (void)arg;
 worker_init();
 for(;;){
  /* Tick-paced loop: wait only for the remaining part of a 100ms period.
   * An unconditional 100ms after drawing would run substantially slower
   * than 10 Hz when rendering requires meaningful CPU time. */
  uint32_t begin=rb_hal_tick();
  uint32_t elapsed;
  worker_step();
  elapsed=rb_hal_tick()-begin;
  if(elapsed<10u)rb_hal_sleep((10u-elapsed)*10u);
  else rb_hal_sleep(1u); /* no busy loop when camera is busy */
 }
}

void rb_boot(void){
 int tid;
 g.magic=RB_MAGIC;g.ready=0u;g.beta_enabled=(uint32_t)exists("A:/ML/RB9BETA.ON");
 g.menu_active=0u;g.menu_want=g.beta_enabled;
 g.page=RB_PAGE_HOME;g.selected=0u;
 g.film=0u;g.roll=1u;g.frames=0u;
 g.modules=RB_ROLL_ON|RB_DIAG_ON;g.flags=RB_FLAG_AUTO_SAVE|RB_FLAG_START_OPEN;
 g.dirty=0u;g.requests=0u;g.event_total=0u;g.event_lost=0u;
 g.config_seq=0u;g.config_status=0u;g.saved_slot=0xffffffffu;
 g.last_key=0xffffffffu;g.redraws=0u;g.fullscreen_fills=0u;
 g.total_ram=0u;g.free_ram=0u;g.test_ram=0u;g.test_cf=0u;
 g.status=RB_STATUS_IDLE;g.ui_shown=0u;
 g.tick_boot=rb_hal_tick();g.last_input_tick=g.tick_boot;g.last_save_tick=g.tick_boot-300u;
 worker_cycle=0u;
 /* Fail-closed when task allocation fails: no GUI interception. */
 tid=rb_hal_task("rb9_beta",31,8192,rb_worker,0);
 if(tid<0)g.ready=0u;
}
