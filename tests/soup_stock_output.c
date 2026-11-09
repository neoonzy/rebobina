/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Filtro de filme REAL do Soup Engine, aplicado a uma matriz RGB sintética.
 * Profile .fs lido da revisao upstream fixa; JPEG sai apenas na HAL RAM.
 * Nao processa CR2, nao acessa fotos do usuario nem usa ROM Canon.
 */
#include "rebobina/hal_sim.h"
#include "rebobina/output.h"
#include "se.h"
#include "stock.h"
#include "look.h"
#include "jpeg_enc.h"
#include "jpeg_dec.h"
#include <assert.h>
#include <stdio.h>

enum { W = 8, H = 8, STOCK_MAX = 16384 };
static uint8_t look_arena[65536];

typedef struct look_context {
    rb_output *out;
    se_jpeg *jpeg;
    const uint8_t *input;
    unsigned rows;
    unsigned changed;
    unsigned errors;
} look_context;

static int jpeg_sink(void *ctx, const uint8_t *bytes, size_t n)
{
    look_context *c = ctx;
    if (rb_output_append(c->out, bytes, n) != RB_HAL_OK) {
        ++c->errors;
        return -1;
    }
    return 0;
}
static int emit_row(void *ctx, const uint8_t *rgb, int width)
{
    look_context *c = ctx;
    unsigned i;
    if (width != W || c->rows >= H) return -1;
    for (i = 0; i < W * 3u; ++i)
        if (rgb[i] != c->input[(size_t)c->rows * W * 3u + i]) ++c->changed;
    ++c->rows;
    return se_jpeg_write_rows(c->jpeg, rgb, 1) == SE_OK ? 0 : -1;
}
static size_t read_stock(const char *path, char *buffer, size_t capacity)
{
    FILE *f = fopen(path, "rb");
    size_t n;
    assert(f != NULL);
    n = fread(buffer, 1, capacity, f);
    assert(!ferror(f));
    assert(!feof(f) || n > 0);
    assert(n < capacity); /* entrada de teste truncada nao pode ser aceita */
    assert(fclose(f) == 0);
    buffer[n] = '\0';
    return n;
}
int main(int argc, char **argv)
{
    rb_sim_hal sim;
    rb_hal *hal;
    rb_output out = {0};
    rb_hal_file source, reader;
    se_stock stock;
    se_look look;
    se_look_run *run = NULL;
    se_arena arena;
    se_jpeg encoder;
    se_jdec_info info;
    se_jpeg_exif exif = {"Rebobina","synthetic 5DC host","Soup stock HAL test",
                         "2026:11:06 12:00:00", 0.0f, 0.0f, 0};
    look_context context = {0};
    uint8_t strip[W*8*3];
    uint8_t pixels[W*H*3];
    uint8_t jpeg[RB_SIM_FILE_BYTES] = {0};
    char stock_text[STOCK_MAX];
    size_t stock_bytes, written = 0, size, got, total = 0;
    unsigned x,y;
    assert(argc == 2 || argc == 3);
    stock_bytes = read_stock(argv[1], stock_text, STOCK_MAX-1);
    se_stock_init(&stock);
    assert(se_stock_parse(stock_text, stock_bytes, &stock) == SE_OK);
    assert(stock.version <= SE_STOCK_VERSION);
    se_look_from_stock(&stock, 1, &look);
    (void)se_look_fit(&look, W, sizeof(look_arena)-4096u);
    se_arena_init(&arena, look_arena, sizeof(look_arena));
    assert(se_look_begin(&run, &look, W, &arena) == SE_OK);

    for (y=0; y<H; ++y) for (x=0; x<W; ++x) {
        size_t pos = (y*W+x)*3u;
        pixels[pos]=(uint8_t)(x*30u);
        pixels[pos+1]=(uint8_t)(y*27u);
        pixels[pos+2]=(uint8_t)((x+y)*15u);
    }
    rb_sim_hal_init(&sim);
    hal=&sim.api;
    /* Origem e uma string ficticia, nao um arquivo CR2 valido. */
    assert(hal->create(hal->ctx,"RAW.CR2",&source)==RB_HAL_OK);
    assert(hal->write(hal->ctx,source,"ORIGINAL",8,&written)==RB_HAL_OK);
    assert(written==8);
    assert(hal->close(hal->ctx,source)==RB_HAL_OK);
    assert(rb_output_begin(&out,hal,"STOCK.JPG")==RB_HAL_OK);
    context.out=&out;
    context.jpeg=&encoder;
    context.input=pixels;
    assert(se_jpeg_begin(&encoder,W,H,80,strip,&exif,jpeg_sink,&context)==SE_OK);
    assert(se_look_rows(run,pixels,H,emit_row,&context)==SE_OK);
    assert(se_look_end(run,emit_row,&context)==SE_OK);
    assert(context.rows==H);
    assert(context.changed>0); /* o perfil teve efeito visivel nos bytes */
    assert(se_jpeg_end(&encoder)==SE_OK);
    assert(context.errors==0);
    assert(rb_output_commit(&out)==RB_HAL_OK);
    size=rb_sim_hal_file_size(&sim,"STOCK.JPG");
    assert(size>100 && size<RB_SIM_FILE_BYTES);
    assert(rb_sim_hal_file_size(&sim,"STOCK.JPG.TMP")==0);
    assert(hal->open_read(hal->ctx,"STOCK.JPG",&reader)==RB_HAL_OK);
    while(total<size) {
        assert(hal->read(hal->ctx,reader,jpeg+total,size-total,&got)==RB_HAL_OK);
        assert(got>0);
        total+=got;
    }
    assert(hal->close(hal->ctx,reader)==RB_HAL_OK);
    assert(jpeg[0]==0xff && jpeg[1]==0xd8);
    assert(jpeg[size-2]==0xff && jpeg[size-1]==0xd9);
    assert(se_jpeg_dec_info(jpeg,size,&info)==SE_OK);
    assert(info.width==W && info.height==H);
    assert(rb_sim_hal_file_size(&sim,"RAW.CR2")==8);
    /* Fixtures saem apenas para caminhos EXPLICITOS e nao sobrescrevem arquivos.
     * Usado pelo CI para comparar JPEGs byte a byte com build sanitizado.
     */
    if (argc == 3) {
        FILE *file = fopen(argv[2], "wbx");
        assert(file != NULL);
        assert(fwrite(jpeg, 1, size, file) == size);
        assert(fclose(file) == 0);
    }
    printf("OK: stock real do Soup '%s' aplicado a pixels sinteticos; "
           "%u canais alterados, %lu bytes JPEG em HAL RAM\n",
           stock.name,context.changed,(unsigned long)size);
    return 0;
}
