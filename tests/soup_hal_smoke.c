/* SPDX-License-Identifier: GPL-2.0-only */
/* Integra contrato HAL Rebobina com se_arena/se_rd REAIS do upstream Soup.
 * Os bytes sao um fixture sintetico, NAO um CR2 ou JPEG verdadeiro.
 */
#include "rebobina/hal_sim.h"
#include "se.h" /* header fixado externamente: jiijii0/soup-engine */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    rb_sim_hal sim;
    rb_hal_file file;
    rb_hal *hal;
    se_arena arena;
    se_rd reader;
    uint8_t raw[8] = {0x49,0x49,0x2a,0x00,0x08,0x00,0x00,0x00};
    uint8_t copy[8] = {0};
    void *space;
    size_t size = 0, count = 0;
    rb_sim_hal_init(&sim);
    hal = &sim.api;
    assert(hal->create(hal->ctx, "FIXTURE.BIN", &file) == RB_HAL_OK);
    assert(hal->write(hal->ctx, file, raw, sizeof(raw), &count) == RB_HAL_OK);
    assert(count == sizeof(raw));
    assert(hal->close(hal->ctx, file) == RB_HAL_OK);
    assert(hal->open_read(hal->ctx, "FIXTURE.BIN", &file) == RB_HAL_OK);
    rb_sim_hal_short_read_once(&sim, 3);
    while (size < sizeof(copy)) {
        assert(hal->read(hal->ctx, file, copy + size,
                         sizeof(copy) - size, &count) == RB_HAL_OK);
        assert(count > 0);
        size += count;
    }
    assert(hal->close(hal->ctx, file) == RB_HAL_OK);
    space = hal->alloc(hal->ctx, 1024);
    assert(space != NULL);
    se_arena_init(&arena, space, 1024);
    {
        uint8_t *scratch = se_arena_alloc(&arena, sizeof(copy), 4);
        size_t i;
        assert(scratch != NULL);
        for (i = 0; i < sizeof(copy); ++i) scratch[i] = copy[i];
        se_rd_init(&reader, scratch, sizeof(copy), 0);
        assert(se_rd_u16(&reader) == 0x4949);
        assert(se_rd_u16(&reader) == 0x002a);
        assert(se_rd_u32(&reader) == 8u);
        assert(reader.err == 0);
        assert(se_rd_u8(&reader) == 0); /* leitura depois de EOF -> erro controlado */
        assert(reader.err != 0);
        assert(se_arena_alloc(&arena, 1024, 8) == NULL);
        se_arena_reset(&arena);
        assert(se_arena_alloc(&arena, 1024, 1) != NULL);
    }
    hal->release(hal->ctx, space);
    assert(se_strerror(SE_E_NOMEM) != NULL);
    puts("OK: Soup se.c + HAL Rebobina com fixture sintetico (SEM CR2/JPEG real)");
    return 0;
}
