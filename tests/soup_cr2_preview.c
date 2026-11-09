/* SPDX-License-Identifier: GPL-2.0-only */
/* Fixture CR2 sintetico: IFD0 com JPEG real do Soup, mas SEM RAW de camera.
 * Testa limites TIFF, offsets truncados, dimensoes e decoder upstream.
 */
#include "rebobina/cr2_host.h"
#include "se.h"
#include "jpeg_enc.h"
#include "jpeg_dec.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

enum { W=8, H=8, PREVIEW_OFFSET=128, CAPACITY=4096 };
typedef struct fixture {
    unsigned char container[CAPACITY];
    unsigned char rgb[W*H*3];
    unsigned char strip[W*8*3];
    se_jpeg encoder;
    unsigned char jpeg[2048];
    size_t used;
} fixture;
typedef struct sink {
    fixture *f;
} sink;

static void put16(unsigned char *b, unsigned off, unsigned val)
{
    b[off] = (unsigned char)val;
    b[off+1] = (unsigned char)(val >> 8);
}
static void put32(unsigned char *b, unsigned off, unsigned val)
{
    b[off] = (unsigned char)val;
    b[off+1] = (unsigned char)(val >> 8);
    b[off+2] = (unsigned char)(val >> 16);
    b[off+3] = (unsigned char)(val >> 24);
}
static void entry(unsigned char *b, unsigned n, unsigned tag, unsigned value)
{
    unsigned base = 18u + 12u*n;
    put16(b,base,tag);
    put16(b,base+2u,4); /* TIFF LONG */
    put32(b,base+4u,1);
    put32(b,base+8u,value);
}
static int jpeg_write(void *arg, const unsigned char *bytes, size_t n)
{
    fixture *f = (fixture *)arg;
    if (n > sizeof(f->jpeg)-f->used) return -1;
    memcpy(f->jpeg+f->used,bytes,n);
    f->used+=n;
    return 0;
}
static void build(fixture *f)
{
    unsigned x,y;
    se_jpeg_exif exif = {"Rebobina","synthetic CR2 preview",
                         "Fixture, not a Canon photo","2026:11:08 12:00:00",
                         0,0,0};
    memset(f,0,sizeof(*f));
    for(y=0;y<H;++y) for(x=0;x<W;++x){
        unsigned o=(y*W+x)*3u;
        f->rgb[o]=(unsigned char)(x*30u);
        f->rgb[o+1]=(unsigned char)(y*29u);
        f->rgb[o+2]=(unsigned char)((x+y)*14u);
    }
    assert(se_jpeg_begin(&f->encoder,W,H,82,f->strip,&exif,jpeg_write,f)==SE_OK);
    assert(se_jpeg_write_rows(&f->encoder,f->rgb,H)==SE_OK);
    assert(se_jpeg_end(&f->encoder)==SE_OK);
    assert(f->used > 100u);
    assert(PREVIEW_OFFSET+f->used <= CAPACITY);
    f->container[0]='I';f->container[1]='I';
    f->container[2]=42;f->container[3]=0;
    put32(f->container,4,16); /* IFD0 */
    f->container[8]='C';f->container[9]='R';
    f->container[10]=2;f->container[11]=0;
    put16(f->container,16,5);
    entry(f->container,0,0x0100,W);
    entry(f->container,1,0x0101,H);
    entry(f->container,2,0x0103,6);
    entry(f->container,3,0x0111,PREVIEW_OFFSET);
    entry(f->container,4,0x0117,(unsigned)f->used);
    memcpy(f->container+PREVIEW_OFFSET,f->jpeg,f->used);
}
typedef struct decoded {
    size_t rows;
    unsigned long sum;
} decoded;
static int on_rgb(void *arg, const unsigned char *rgb, int nrows, int width)
{
    decoded *d = (decoded *)arg;
    int i;
    if(nrows<1||width!=W||d->rows+(size_t)nrows>H) return -1;
    for(i=0;i<nrows*width*3;++i) d->sum+=rgb[i];
    d->rows+=(size_t)nrows;
    return 0;
}
int main(int argc, char **argv)
{
    static fixture f; /* evita buffer grande na pilha em hardware futuro */
    static unsigned char memory[65536];
    rb_cr2_host_preview p={0};
    se_arena arena;
    se_jdec_info info={0};
    decoded got={0}, direct={0};
    size_t len;
    build(&f);
    len=PREVIEW_OFFSET+f.used;
    assert(rb_cr2_host_find_preview(f.container,len,&p)==0);
    assert(p.offset==PREVIEW_OFFSET && p.bytes==f.used && p.width==W && p.height==H);
    se_arena_init(&arena,memory,sizeof(memory));
    assert(se_jpeg_decode_scaled(f.container+p.offset,p.bytes,1,&arena,
                                  on_rgb,&got,&info)==SE_OK);
    assert(got.rows==H && info.width==W && info.height==H);
    se_arena_reset(&arena);
    assert(se_jpeg_decode_scaled(f.jpeg,f.used,1,&arena,
                                  on_rgb,&direct,&info)==SE_OK);
    assert(got.sum==direct.sum && got.rows==direct.rows);

    /* Offset+bytes precisa caber no arquivo real, nao apenas ter SOI. */
    put32(f.container,18+12*4+8,(unsigned)f.used+400u);
    assert(rb_cr2_host_find_preview(f.container,len,&p)!=0);
    put32(f.container,18+12*4+8,(unsigned)f.used);

    f.container[0]='M'; assert(rb_cr2_host_find_preview(f.container,len,&p)!=0);
    f.container[0]='I';
    f.container[8]='X'; assert(rb_cr2_host_find_preview(f.container,len,&p)!=0);
    f.container[8]='C';
    put32(f.container,18+12*2+8,7); /* compressao nao JPEG */
    assert(rb_cr2_host_find_preview(f.container,len,&p)!=0);
    put32(f.container,18+12*2+8,6);
    f.container[PREVIEW_OFFSET]=0;
    assert(rb_cr2_host_find_preview(f.container,len,&p)!=0);
    f.container[PREVIEW_OFFSET]=0xff;
    put32(f.container,18+12*0+8,9999); /* dimensoes nao coincidem */
    assert(rb_cr2_host_find_preview(f.container,len,&p)!=0);
    put32(f.container,18+12*0+8,W);
    assert(rb_cr2_host_find_preview(f.container,PREVIEW_OFFSET+1,&p)!=0);
    assert(rb_cr2_host_find_preview(f.container,0,&p)!=0);
    assert(rb_cr2_host_find_preview(NULL,len,&p)!=0);
    assert(rb_cr2_host_find_preview(f.container,len,NULL)!=0);
    assert(rb_cr2_host_find_preview(f.container,len,&p)==0);
    if(argc>2) {
        fputs("Uso (somente testes): soup-cr2-preview [arquivo_sintetico.CR2]\\n", stderr);
        return 2;
    }
    if(argc==2) {
        FILE *target = fopen(argv[1],"wbx"); /* criacao exclusiva, nunca sobrescreve */
        if(!target) return 3;
        if(fwrite(f.container,1,len,target)!=len) {
            (void)fclose(target);
            return 4;
        }
        if(fclose(target)!=0) return 5;
        puts("Fixture CR2 sintetico escrito exclusivamente para testes (SEM RAW real)");
    }
    puts("OK: extracao e decode de preview JPEG em CR2 sintetico; offsets TIFF, SOI, dimensoes e truncamento");
    return 0;
}
