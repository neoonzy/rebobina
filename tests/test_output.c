/* SPDX-License-Identifier: GPL-2.0-only */
/* Testes host somente RAM: commit exclusivo, rollback, energia e erro de IO. */
#include "rebobina/output.h"
#include "rebobina/hal_sim.h"
#include <assert.h>
#include <stdio.h>

static int readable(rb_hal *api, const char *name)
{
    rb_hal_file h;
    rb_hal_result rc = api->open_read(api->ctx, name, &h);
    if (rc == RB_HAL_NOT_FOUND) return 0;
    assert(rc == RB_HAL_OK);
    assert(api->close(api->ctx, h) == RB_HAL_OK);
    return 1;
}
static void seed(rb_hal *api, const char *name, const char *value, size_t length)
{
    rb_hal_file h;
    size_t n = 0;
    assert(api->create(api->ctx, name, &h) == RB_HAL_OK);
    assert(api->write(api->ctx, h, value, length, &n) == RB_HAL_OK);
    assert(n == length);
    assert(api->close(api->ctx, h) == RB_HAL_OK);
}
static void test_commit_and_exclusive(void)
{
    rb_sim_hal sim;
    rb_hal *api;
    rb_output out = {0}, collision = {0};
    rb_hal_file f;
    size_t n = 0;
    char buf[8] = {0};
    rb_sim_hal_init(&sim);
    api = &sim.api;
    seed(api, "ORIG.CR2", "RAW!", 4);
    assert(rb_output_begin(&out, api, "FILM.JPG") == RB_HAL_OK);
    assert(rb_output_begin(&out, api, "DUP.JPG") == RB_HAL_BAD_STATE);
    assert(readable(api, "FILM.JPG") == 0);
    assert(rb_output_append(&out, "SOUP", 4) == RB_HAL_OK);
    assert(rb_output_append(&out, NULL, 0) == RB_HAL_OK);
    assert(rb_output_commit(&out) == RB_HAL_OK);
    assert(out.published && !out.staged && !out.open);
    assert(rb_output_commit(&out) == RB_HAL_BAD_STATE);
    assert(rb_output_abort(&out) == RB_HAL_BAD_STATE);
    assert(!readable(api, "FILM.JPG.TMP"));
    assert(readable(api, "FILM.JPG"));
    assert(rb_sim_hal_file_size(&sim, "FILM.JPG") == 4);
    assert(api->open_read(api->ctx, "FILM.JPG", &f) == RB_HAL_OK);
    assert(api->read(api->ctx, f, buf, 4, &n) == RB_HAL_OK && n == 4);
    assert(buf[0]=='S' && buf[1]=='O' && buf[2]=='U' && buf[3]=='P');
    assert(api->close(api->ctx, f) == RB_HAL_OK);
    assert(rb_sim_hal_file_size(&sim, "ORIG.CR2") == 4);
    /* Sucesso parcial de write deve ser completado pelo loop do output. */
    assert(rb_output_begin(&collision, api, "SHORT.JPG") == RB_HAL_OK);
    rb_sim_hal_short_write_once(&sim, 2);
    assert(rb_output_append(&collision, "123456", 6) == RB_HAL_OK);
    assert(rb_output_commit(&collision) == RB_HAL_OK);
    assert(rb_sim_hal_file_size(&sim, "SHORT.JPG") == 6);
    assert(rb_output_begin(&collision, api, "FILM.JPG") == RB_HAL_OK);
    assert(rb_output_append(&collision, "NEW!", 4) == RB_HAL_OK);
    assert(rb_output_commit(&collision) == RB_HAL_EXISTS);
    assert(rb_output_abort(&collision) == RB_HAL_OK);
    assert(rb_sim_hal_file_size(&sim, "FILM.JPG") == 4);
    assert(!readable(api, "FILM.JPG.TMP"));
}
static void test_write_and_sync_faults(void)
{
    rb_sim_hal sim;
    rb_hal *api;
    rb_output out = {0};
    rb_sim_hal_init(&sim);
    api = &sim.api;
    assert(rb_output_begin(&out, api, "F1.JPG") == RB_HAL_OK);
    rb_sim_hal_inject_fault(&sim, RB_SIM_FAULT_WRITE);
    assert(rb_output_append(&out, "123456", 6) == RB_HAL_IO);
    assert(rb_output_commit(&out) == RB_HAL_BAD_STATE);
    assert(rb_output_abort(&out) == RB_HAL_OK);
    assert(!readable(api, "F1.JPG.TMP"));
    assert(!readable(api, "F1.JPG"));

    assert(rb_output_begin(&out, api, "F2.JPG") == RB_HAL_OK);
    rb_sim_hal_cut_write_once(&sim, 3);
    assert(rb_output_append(&out, "123456", 6) == RB_HAL_IO);
    assert(rb_sim_hal_file_size(&sim, "F2.JPG.TMP") == 3);
    assert(rb_output_abort(&out) == RB_HAL_OK);
    assert(!readable(api, "F2.JPG.TMP"));

    assert(rb_output_begin(&out, api, "F3.JPG") == RB_HAL_OK);
    assert(rb_output_append(&out, "123456", 6) == RB_HAL_OK);
    rb_sim_hal_inject_fault(&sim, RB_SIM_FAULT_SYNC);
    assert(rb_output_commit(&out) == RB_HAL_IO);
    assert(rb_output_commit(&out) == RB_HAL_BAD_STATE);
    assert(rb_output_abort(&out) == RB_HAL_OK);
    assert(!readable(api, "F3.JPG"));

    assert(rb_output_begin(&out, api, "F4.JPG") == RB_HAL_OK);
    assert(rb_output_append(&out, "123456", 6) == RB_HAL_OK);
    rb_sim_hal_inject_fault(&sim, RB_SIM_FAULT_RENAME);
    assert(rb_output_commit(&out) == RB_HAL_IO);
    assert(readable(api, "F4.JPG.TMP"));
    assert(!readable(api, "F4.JPG"));
    rb_sim_hal_inject_fault(&sim, RB_SIM_FAULT_REMOVE);
    assert(rb_output_abort(&out) == RB_HAL_IO);
    assert(readable(api, "F4.JPG.TMP"));
    assert(rb_output_discard_orphan(api, "F4.JPG") == RB_HAL_OK);
    assert(rb_output_discard_orphan(api, "F4.JPG") == RB_HAL_OK);
    assert(!readable(api, "F4.JPG.TMP"));
}
static void test_power_cut_orphans(void)
{
    rb_sim_hal sim;
    rb_output out;
    rb_hal *api;
    rb_sim_hal_init(&sim);
    api = &sim.api;
    seed(api, "RAW.CR2", "ABC", 3);
    assert(rb_output_begin(&out, api, "OUT.JPG") == RB_HAL_OK);
    assert(rb_output_append(&out, "1234", 4) == RB_HAL_OK);
    assert(rb_output_discard_orphan(api, "OUT.JPG") == RB_HAL_BAD_STATE);
    rb_sim_hal_set_media(&sim, 0);
    assert(rb_output_commit(&out) == RB_HAL_NO_MEDIA);
    assert(rb_output_abort(&out) == RB_HAL_NO_MEDIA);
    rb_sim_hal_power_cycle(&sim);
    rb_sim_hal_set_media(&sim, 1);
    assert(readable(api, "OUT.JPG.TMP"));
    assert(!readable(api, "OUT.JPG"));
    assert(rb_sim_hal_file_size(&sim, "RAW.CR2") == 3);
    assert(rb_output_discard_orphan(api, "OUT.JPG") == RB_HAL_OK);
    assert(!readable(api, "OUT.JPG.TMP"));
    /* Power cycle invalidou contexto RAM da transacao: inicializar nova sessao. */
    rb_output_init(&out);
    assert(rb_output_begin(&out, api, "OUT.JPG") == RB_HAL_OK);
    assert(rb_output_append(&out, "GOOD", 4) == RB_HAL_OK);
    assert(rb_output_commit(&out) == RB_HAL_OK);
    assert(readable(api, "OUT.JPG"));
    assert(rb_sim_hal_file_size(&sim, "RAW.CR2") == 3);
}
static void test_validation_and_limits(void)
{
    rb_sim_hal sim;
    rb_hal *api;
    rb_output out;
    rb_hal_file src, old, dst;
    size_t n = 0;
    rb_sim_hal_init(&sim);
    api = &sim.api;
    assert(rb_output_begin(NULL, api, "X.JPG") == RB_HAL_BAD_ARG);
    assert(rb_output_begin(&out, NULL, "X.JPG") == RB_HAL_BAD_ARG);
    assert(rb_output_begin(&out, api, "../X.JPG") == RB_HAL_BAD_ARG);
    assert(rb_output_begin(&out, api, "") == RB_HAL_BAD_ARG);
    assert(rb_output_begin(&out, api, "12345678901234567890123456789012345678901234.JPG") == RB_HAL_BAD_ARG);
    assert(rb_output_begin(&out, api, "ALPHA.JPG") == RB_HAL_OK);
    assert(rb_output_append(&out, NULL, 5) == RB_HAL_BAD_ARG);
    assert(rb_output_append(&out, "good", 4) == RB_HAL_OK);
    assert(api->rename_noreplace(api->ctx, "ALPHA.JPG.TMP", "DEST.JPG") == RB_HAL_BAD_STATE);
    assert(api->remove(api->ctx, "ALPHA.JPG.TMP") == RB_HAL_BAD_STATE);
    assert(rb_output_abort(&out) == RB_HAL_OK);

    assert(api->create(api->ctx, "OLD.JPG", &src) == RB_HAL_OK);
    old = src;
    assert(api->write(api->ctx, src, "old", 3, &n) == RB_HAL_OK && n == 3);
    assert(api->sync(api->ctx, src) == RB_HAL_OK);
    assert(api->close(api->ctx, src) == RB_HAL_OK);
    assert(api->sync(api->ctx, old) == RB_HAL_BAD_HANDLE);
    seed(api, "DEST.JPG", "dest", 4);
    assert(api->rename_noreplace(api->ctx, "OLD.JPG", "DEST.JPG") == RB_HAL_EXISTS);
    assert(rb_sim_hal_file_size(&sim, "DEST.JPG") == 4);
    assert(api->remove(api->ctx, "DEST.JPG") == RB_HAL_OK);
    assert(api->rename_noreplace(api->ctx, "OLD.JPG", "DEST.JPG") == RB_HAL_OK);
    assert(api->open_read(api->ctx, "OLD.JPG", &dst) == RB_HAL_NOT_FOUND);
    assert(rb_sim_hal_file_size(&sim, "DEST.JPG") == 3);
}
int main(void)
{
    test_commit_and_exclusive();
    test_write_and_sync_faults();
    test_power_cut_orphans();
    test_validation_and_limits();
    puts("OK: saida em duas fases, rename exclusivo, sync, falhas, recuperacao RAM");
    return 0;
}
