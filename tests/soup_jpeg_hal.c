/* SPDX-License-Identifier: GPL-2.0-only */
/* JPEG 8x8 sintetico pelo encoder REAL Soup, armazenado na HAL RAM Rebobina.
 * Nao acessa CR2, ROM Canon, CF ou cameras. Usar somente para testes.
 */
#include "rebobina/hal_sim.h"
#include "se.h"
#include "jpeg_enc.h"
#include "jpeg_dec.h"
#include <assert.h>
#include <stdio.h>

typedef struct sink_context {
    rb_hal *hal;
    rb_hal_file file;
    int failures;
} sink_context;

static int jpeg_sink(void *ctx, const uint8_t *data, size_t length)
{
    sink_context *s = ctx;
    size_t written = 0;
    rb_hal_result status = s->hal->write(s->hal->ctx, s->file,
                                         data, length, &written);
    if (status != RB_HAL_OK || written != length) {
        s->failures++;
        return -1;
    }
    return 0;
}

static void make_pixels(uint8_t *pixels)
{
    unsigned x, y;
    for (y = 0; y < 8; ++y)
        for (x = 0; x < 8; ++x) {
            unsigned o = (y * 8u + x) * 3u;
            pixels[o] = (uint8_t)(x * 31u);
            pixels[o + 1u] = (uint8_t)(y * 31u);
            pixels[o + 2u] = (uint8_t)(x * 17u + y * 13u);
        }
}

static se_err encode(sink_context *ctx)
{
    se_jpeg j;
    uint8_t strip[8u * 8u * 3u];
    uint8_t pixels[8u * 8u * 3u];
    se_jpeg_exif exif = {
        "Rebobina", "EOS 5D Classic (synthetic host)",
        "Soup Engine / Rebobina HAL smoke", "2026:11:06 12:00:00",
        0.0f, 0.0f, 0
    };
    se_err rc;
    make_pixels(pixels);
    rc = se_jpeg_begin(&j, 8, 8, 80, strip, &exif, jpeg_sink, ctx);
    if (rc != SE_OK) return rc;
    rc = se_jpeg_write_rows(&j, pixels, 8);
    if (rc != SE_OK) return rc;
    return se_jpeg_end(&j);
}

int main(void)
{
    rb_sim_hal sim;
    rb_hal_file src, dst, rd;
    rb_hal *hal;
    sink_context sink;
    se_jdec_info info;
    uint8_t jpeg_bytes[RB_SIM_FILE_BYTES] = {0};
    const uint8_t original[] = {'R','A','W','0'};
    size_t total = 0, chunk = 0, size;
    rb_sim_hal_init(&sim);
    hal = &sim.api;
    assert(hal->create(hal->ctx, "ORIGINAL.CR2", &src) == RB_HAL_OK);
    assert(hal->write(hal->ctx, src, original, sizeof(original), &chunk) == RB_HAL_OK);
    assert(hal->close(hal->ctx, src) == RB_HAL_OK);
    assert(hal->create(hal->ctx, "PREVIEW.JPG", &dst) == RB_HAL_OK);
    sink.hal = hal;
    sink.file = dst;
    sink.failures = 0;
    assert(encode(&sink) == SE_OK);
    assert(sink.failures == 0);
    size = rb_sim_hal_file_size(&sim, "PREVIEW.JPG");
    assert(size > 100 && size < RB_SIM_FILE_BYTES);
    assert(hal->close(hal->ctx, dst) == RB_HAL_OK);
    assert(hal->open_read(hal->ctx, "PREVIEW.JPG", &rd) == RB_HAL_OK);
    while (total < size) {
        assert(hal->read(hal->ctx, rd, jpeg_bytes + total, size - total, &chunk) == RB_HAL_OK);
        assert(chunk > 0);
        total += chunk;
    }
    assert(hal->close(hal->ctx, rd) == RB_HAL_OK);
    assert(jpeg_bytes[0] == 0xff && jpeg_bytes[1] == 0xd8);
    assert(jpeg_bytes[total-2] == 0xff && jpeg_bytes[total-1] == 0xd9);
    assert(se_jpeg_dec_info(jpeg_bytes, total, &info) == SE_OK);
    assert(info.width == 8 && info.height == 8);
    assert(rb_sim_hal_file_size(&sim, "ORIGINAL.CR2") == sizeof(original));

    /* Falha de gravação depois de bytes parciais, sem sobrescrever original. */
    assert(hal->create(hal->ctx, "BROKEN.JPG", &dst) == RB_HAL_OK);
    sink.file = dst;
    sink.failures = 0;
    rb_sim_hal_cut_write_once(&sim, 12);
    assert(encode(&sink) == SE_E_IO);
    assert(sink.failures > 0);
    assert(rb_sim_hal_file_size(&sim, "BROKEN.JPG") == 12);
    rb_sim_hal_power_cycle(&sim);
    assert(rb_sim_hal_file_size(&sim, "ORIGINAL.CR2") == sizeof(original));
    assert(hal->create(hal->ctx, "ORIGINAL.CR2", &src) == RB_HAL_EXISTS);
    puts("OK: JPEG 8x8 real do Soup gravado em HAL RAM, SOI/EOI e cabecalho validados");
    puts("OK: escrita JPEG parcial falhou e arquivo original ficou intocado");
    return 0;
}
