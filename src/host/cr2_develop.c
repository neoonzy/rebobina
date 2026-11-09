/* SPDX-License-Identifier: GPL-2.0-only */
/* Rebobina preview developer (Linux HOST): CR2 -> JPEG embarcado -> Soup look
 * -> NOVO JPEG, sem tocar no CR2. Nao decodifica RAW mosaico do sensor.
 */
#include "rebobina/cr2_host.h"
#include "rebobina/hal_posix.h"
#include "rebobina/output.h"
#include "se.h"
#include "cr2.h"
#include "stock.h"
#include "look.h"
#include "jpeg_dec.h"
#include "jpeg_enc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define RB_MAX_CR2 (64u * 1024u * 1024u)
#define RB_MAX_STOCK (16u * 1024u)
#define RB_ARENA_BYTES (32u * 1024u * 1024u)

typedef struct preview_context {
    se_look_run *run;
    se_jpeg *jpeg;
    rb_output *out;
    int width;
    unsigned rows_written;
    int failed;
} preview_context;

static int sink_jpeg(void *arg, const uint8_t *buf, size_t n)
{
    preview_context *c = (preview_context *)arg;
    if (rb_output_append(c->out, buf, n) != RB_HAL_OK) {
        c->failed = 1;
        return -1;
    }
    return 0;
}
static int emit_rgb(void *arg, const uint8_t *rgb, int width)
{
    preview_context *c = (preview_context *)arg;
    if (width != c->width ||
        se_jpeg_write_rows(c->jpeg, rgb, 1) != SE_OK) {
        c->failed = 1;
        return -1;
    }
    ++c->rows_written;
    return 0;
}
static int decoded_rows(void *arg, const uint8_t *rgb, int nrows, int width)
{
    preview_context *c = (preview_context *)arg;
    if (width != c->width ||
        se_look_rows(c->run, rgb, nrows, emit_rgb, c) != SE_OK) {
        c->failed = 1;
        return -1;
    }
    return 0;
}
/* Aloca a partir de arquivo de origem SOMENTE LEITURA e tamanho limitado.
 * A leitura inteira acontece no host. A camera nao tem memoria para isso.
 */
static uint8_t *load_file(const char *path, size_t max_bytes, size_t *size)
{
    FILE *fp;
    long n;
    uint8_t *bytes;
    if (!path || !size) return NULL;
    fp = fopen(path, "rb");
    if (!fp) return NULL;
    if (fseek(fp, 0, SEEK_END) != 0 ||
        (n = ftell(fp)) <= 0 || (unsigned long)n > max_bytes ||
        fseek(fp, 0, SEEK_SET) != 0) {
        (void)fclose(fp);
        return NULL;
    }
    bytes = (uint8_t *)malloc((size_t)n + 1u);
    if (!bytes) {
        (void)fclose(fp);
        return NULL;
    }
    if (fread(bytes, 1, (size_t)n, fp) != (size_t)n ||
        fclose(fp) != 0) {
        free(bytes);
        return NULL;
    }
    bytes[n] = 0;
    *size = (size_t)n;
    return bytes;
}
static int run(const char *input, const char *stock_path, const char *directory,
               const char *name, int scale)
{
    uint8_t *raw = NULL, *stock_data = NULL, *memory = NULL;
    rb_cr2_host_preview preview = {0};
    rb_posix_hal posix;
    rb_output output = {0};
    se_arena arena;
    se_stock stock;
    se_look look;
    se_cr2_info rawinfo = {0};
    se_look_run *lookrun = NULL;
    se_jpeg *encoder;
    se_jpeg_exif exif;
    se_jdec_info jpeg_info = {0};
    preview_context context = {0};
    uint8_t *strip;
    size_t raw_size=0, stock_size=0;
    int ow, oh, committed = 0, hal_ready = 0;
    se_err rc;
    memset(&posix, 0, sizeof(posix));
    raw = load_file(input, RB_MAX_CR2, &raw_size);
    if (!raw) { fputs("Erro: CR2 nao lido ou maior que 64 MiB.\n",stderr); goto done; }
    if (rb_cr2_host_find_preview(raw, raw_size, &preview) != 0) {
        fputs("Erro: preview JPEG ausente ou estrutura CR2 invalida.\n", stderr);
        goto done;
    }
    stock_data = load_file(stock_path, RB_MAX_STOCK, &stock_size);
    if (!stock_data) { fputs("Erro: arquivo .fs invalido ou >16 KiB.\n",stderr); goto done; }
    se_stock_init(&stock);
    if (se_stock_parse((const char *)stock_data, stock_size, &stock) != SE_OK) {
        fputs("Erro: nao foi possivel interpretar o perfil .fs.\n", stderr);
        goto done;
    }
    se_jpeg_scaled_size(preview.width, preview.height, scale, &ow, &oh);
    if (ow <= 0 || oh <= 0 || ow > 12000 || oh > 12000) goto done;
    memory = (uint8_t *)malloc(RB_ARENA_BYTES);
    if (!memory) { fputs("Erro: memoria insuficiente.\n", stderr); goto done; }
    se_arena_init(&arena, memory, RB_ARENA_BYTES);
    encoder = SE_NEW(&arena, se_jpeg, 1);
    if (!encoder) goto done;
    se_look_from_stock(&stock, scale, &look);
    (void)se_look_fit(&look, ow, SE_LOOK_BUDGET);
    rc = se_look_begin(&lookrun, &look, ow, &arena);
    if (rc != SE_OK) { fputs("Erro: sem memoria para efeito de filme.\n",stderr); goto done; }
    strip = SE_NEW(&arena, uint8_t, (size_t)8u * (size_t)ow * 3u);
    if (!strip) goto done;

    if (se_cr2_parse(raw, raw_size, &rawinfo) != SE_OK)
        memset(&rawinfo, 0, sizeof(rawinfo));
    memset(&exif, 0, sizeof(exif));
    exif.make = rawinfo.make[0] ? rawinfo.make : "Canon";
    exif.model = rawinfo.model[0] ? rawinfo.model : "Canon EOS 5D";
    exif.software = stock.name;
    exif.datetime = rawinfo.datetime;
    exif.exposure_time = rawinfo.exposure_time;
    exif.aperture = rawinfo.aperture;
    exif.iso = rawinfo.iso;

    if (rb_posix_hal_init(&posix, directory) != RB_HAL_OK) {
        fputs("Erro: diretorio de saida invalido ou simbolico.\n", stderr);
        goto done;
    }
    hal_ready = 1;
    rc = (se_err)rb_output_begin(&output, &posix.api, name);
    if (rc != RB_HAL_OK) {
        fputs("Erro: nao foi possivel criar temporario exclusivo (.TMP).\n", stderr);
        goto done;
    }
    context.run = lookrun;
    context.jpeg = encoder;
    context.out = &output;
    context.width = ow;
    if (se_jpeg_begin(encoder, ow, oh, 88, strip, &exif,
                      sink_jpeg, &context) != SE_OK ||
        se_jpeg_decode_scaled(raw + preview.offset, preview.bytes, scale,
                              &arena, decoded_rows, &context, &jpeg_info) != SE_OK ||
        se_look_end(lookrun, emit_rgb, &context) != SE_OK ||
        context.failed || context.rows_written != (unsigned)oh ||
        se_jpeg_end(encoder) != SE_OK) {
        fputs("Erro: processamento interrompido; JPEG nao publicado.\n", stderr);
        goto done;
    }
    if (rb_output_commit(&output) != RB_HAL_OK) {
        fputs("Erro: nao foi possivel publicar JPEG sem sobrescrever destino.\n", stderr);
        goto done;
    }
    committed = 1;
    printf("OK: preview %dx%d, perfil '%s', JPEG %dx%d salvo como '%s'.\n",
           preview.width, preview.height, stock.name, ow, oh, name);
done:
    if (hal_ready) {
        if (output.staged && !output.published) {
            if (rb_output_abort(&output) != RB_HAL_OK)
                fputs("AVISO: temporario .TMP pode ter permanecido; nao apague sem inspecao.\n",
                      stderr);
        }
        rb_posix_hal_destroy(&posix);
    }
    free(memory);
    free(stock_data);
    free(raw);
    return committed ? 0 : 1;
}
int main(int argc, char **argv)
{
    int scale = 2;
    if (argc != 5 && argc != 6) {
        fputs("Uso: rebobina-cr2-develop <entrada.CR2> <perfil.fs> "
              "<diretorio-existente> <saida.JPG> [escala:1|2]\n", stderr);
        return 2;
    }
    if (argc == 6) {
        if (strcmp(argv[5], "1") == 0) scale = 1;
        else if (strcmp(argv[5], "2") == 0) scale = 2;
        else { fputs("Escala permitida: 1 ou 2.\n", stderr); return 2; }
    }
    return run(argv[1],argv[2],argv[3],argv[4],scale);
}
