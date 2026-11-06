/* SPDX-License-Identifier: GPL-2.0-only */
#include "rebobina/hal_sim.h"
#include <assert.h>
#include <stdio.h>

static unsigned done[RB_SIM_TASKS + 1u];
static unsigned completed;
static void task_record(void *ptr)
{
    unsigned value = *(unsigned *)ptr;
    done[completed++] = value;
}
static void test_files(void)
{
    rb_sim_hal sim;
    rb_hal_file h, stale, reader;
    rb_hal *api;
    size_t count;
    unsigned char output[32] = {0};
    static const unsigned char raw[] = {0x43, 0x52, 0x32, 0x00, 0x11, 0xaa, 0x77};
    unsigned i;
    rb_sim_hal_init(&sim);
    api = &sim.api;
    assert(api->ctx == &sim);
    assert(api->open_read(api->ctx, "CAM.CR2", &h) == RB_HAL_NOT_FOUND);
    assert(api->create(api->ctx, "../oops", &h) == RB_HAL_BAD_ARG);
    assert(api->create(api->ctx, "CAM.CR2", &h) == RB_HAL_OK);
    stale = h;
    assert(api->create(api->ctx, "CAM.CR2", &reader) == RB_HAL_EXISTS);
    assert(api->write(api->ctx, h, raw, sizeof(raw), &count) == RB_HAL_OK);
    assert(count == sizeof(raw));
    assert(api->read(api->ctx, h, output, 1, &count) == RB_HAL_BAD_STATE);
    assert(api->close(api->ctx, h) == RB_HAL_OK);
    assert(api->close(api->ctx, h) == RB_HAL_BAD_HANDLE);
    assert(api->open_read(api->ctx, "CAM.CR2", &reader) == RB_HAL_OK);
    assert(api->write(api->ctx, reader, raw, sizeof(raw), &count) == RB_HAL_BAD_STATE);
    assert(api->read(api->ctx, stale, output, sizeof(output), &count) == RB_HAL_BAD_HANDLE);
    rb_sim_hal_short_read_once(&sim, 3);
    assert(api->read(api->ctx, reader, output, sizeof(output), &count) == RB_HAL_OK);
    assert(count == 3);
    assert(api->read(api->ctx, reader, output + 3, sizeof(output) - 3, &count) == RB_HAL_OK);
    assert(count == sizeof(raw) - 3);
    for (i = 0; i < sizeof(raw); ++i) assert(output[i] == raw[i]);
    assert(api->read(api->ctx, reader, output, 2, &count) == RB_HAL_OK && count == 0);
    assert(api->close(api->ctx, reader) == RB_HAL_OK);
    assert(rb_sim_hal_file_size(&sim, "CAM.CR2") == sizeof(raw));
    assert(api->create(api->ctx, "CAM.CR2", &h) == RB_HAL_EXISTS);
}
static void test_faults_and_power_cycle(void)
{
    rb_sim_hal sim;
    rb_hal_file h, old;
    rb_hal *api;
    unsigned char buffer[20] = {0};
    unsigned char output[20] = {0};
    size_t count;
    unsigned i;
    rb_sim_hal_init(&sim);
    api = &sim.api;
    for (i = 0; i < sizeof(buffer); ++i) buffer[i] = (unsigned char)(i + 1);
    rb_sim_hal_inject_fault(&sim, RB_SIM_FAULT_CREATE);
    assert(api->create(api->ctx, "NEW.TXT", &h) == RB_HAL_IO);
    assert(api->create(api->ctx, "NEW.TXT", &h) == RB_HAL_OK);
    rb_sim_hal_inject_fault(&sim, RB_SIM_FAULT_WRITE);
    assert(api->write(api->ctx, h, buffer, sizeof(buffer), &count) == RB_HAL_IO);
    assert(count == 0);
    assert(rb_sim_hal_file_size(&sim, "NEW.TXT") == 0);
    rb_sim_hal_cut_write_once(&sim, 4);
    assert(api->write(api->ctx, h, buffer, sizeof(buffer), &count) == RB_HAL_IO);
    assert(count == 4);
    old = h;
    assert(rb_sim_hal_file_size(&sim, "NEW.TXT") == 4);
    rb_sim_hal_set_media(&sim, 0);
    assert(api->write(api->ctx, h, buffer, 2, &count) == RB_HAL_NO_MEDIA);
    assert(count == 0);
    rb_sim_hal_power_cycle(&sim);
    rb_sim_hal_set_media(&sim, 1);
    assert(api->write(api->ctx, old, buffer, 2, &count) == RB_HAL_BAD_HANDLE);
    rb_sim_hal_inject_fault(&sim, RB_SIM_FAULT_OPEN);
    assert(api->open_read(api->ctx, "NEW.TXT", &h) == RB_HAL_IO);
    assert(api->open_read(api->ctx, "NEW.TXT", &h) == RB_HAL_OK);
    rb_sim_hal_inject_fault(&sim, RB_SIM_FAULT_READ);
    assert(api->read(api->ctx, h, output, sizeof(output), &count) == RB_HAL_IO);
    assert(count == 0);
    assert(api->read(api->ctx, h, output, sizeof(output), &count) == RB_HAL_OK);
    assert(count == 4);
    for (i = 0; i < 4; ++i) assert(output[i] == buffer[i]);
    assert(api->close(api->ctx, h) == RB_HAL_OK);
}
static void test_limits(void)
{
    rb_sim_hal sim;
    rb_hal_file handles[RB_SIM_FILES];
    rb_hal_file h;
    rb_hal *api;
    unsigned i;
    char name[] = "F0.DAT";
    unsigned char data[RB_SIM_FILE_BYTES + 1u] = {0};
    size_t count;
    void *ptr[RB_SIM_ALLOC_SLOTS];
    rb_sim_hal_init(&sim);
    api = &sim.api;
    for (i = 0; i < RB_SIM_FILES; ++i) {
        name[1] = (char)('0' + i);
        assert(api->create(api->ctx, name, &handles[i]) == RB_HAL_OK);
        assert(api->close(api->ctx, handles[i]) == RB_HAL_OK);
    }
    assert(api->create(api->ctx, "EXTRA", &h) == RB_HAL_NO_SPACE);
    assert(api->open_read(api->ctx, "F0.DAT", &h) == RB_HAL_OK);
    assert(api->write(api->ctx, h, data, 2, &count) == RB_HAL_BAD_STATE);
    assert(api->close(api->ctx, h) == RB_HAL_OK);
    /* Espaco de arquivo limitado; escrevendo apenas no modo correto. */
    rb_sim_hal_init(&sim);
    assert(api->create(api->ctx, "FULL", &h) == RB_HAL_OK);
    assert(api->write(api->ctx, h, data, sizeof(data), &count) == RB_HAL_NO_SPACE);
    assert(count == 0);
    assert(api->write(api->ctx, h, data, RB_SIM_FILE_BYTES, &count) == RB_HAL_OK);
    assert(count == RB_SIM_FILE_BYTES);
    assert(api->write(api->ctx, h, data, 1, &count) == RB_HAL_NO_SPACE);
    assert(api->close(api->ctx, h) == RB_HAL_OK);
    rb_sim_hal_inject_fault(&sim, RB_SIM_FAULT_ALLOC);
    assert(api->alloc(api->ctx, 1) == NULL);
    assert(api->alloc(api->ctx, 0) == NULL);
    assert(api->alloc(api->ctx, RB_SIM_ALLOC_BYTES + 1u) == NULL);
    for (i = 0; i < RB_SIM_ALLOC_SLOTS; ++i) {
        ptr[i] = api->alloc(api->ctx, RB_SIM_ALLOC_BYTES);
        assert(ptr[i] != NULL);
    }
    assert(api->alloc(api->ctx, 1) == NULL);
    api->release(api->ctx, ptr[2]);
    assert(api->alloc(api->ctx, 1) == ptr[2]);
    for (i = 0; i < RB_SIM_ALLOC_SLOTS; ++i) api->release(api->ctx, ptr[i]);
}
static void test_tasks(void)
{
    rb_sim_hal sim;
    rb_hal *api;
    unsigned values[RB_SIM_TASKS + 1u];
    unsigned i;
    rb_sim_hal_init(&sim);
    api = &sim.api;
    completed = 0;
    rb_sim_hal_inject_fault(&sim, RB_SIM_FAULT_QUEUE);
    assert(api->enqueue(api->ctx, task_record, values) == RB_HAL_IO);
    assert(api->enqueue(api->ctx, NULL, values) == RB_HAL_BAD_ARG);
    for (i = 0; i < RB_SIM_TASKS; ++i) {
        values[i] = i + 1;
        assert(api->enqueue(api->ctx, task_record, &values[i]) == RB_HAL_OK);
    }
    assert(rb_sim_hal_pending(&sim) == RB_SIM_TASKS);
    assert(api->enqueue(api->ctx, task_record, &values[0]) == RB_HAL_NO_SPACE);
    assert(rb_sim_hal_run_one(&sim));
    assert(completed == 1 && done[0] == 1);
    values[RB_SIM_TASKS] = 99;
    assert(api->enqueue(api->ctx, task_record, &values[RB_SIM_TASKS]) == RB_HAL_OK);
    while (rb_sim_hal_run_one(&sim)) {}
    assert(completed == RB_SIM_TASKS + 1u);
    assert(done[RB_SIM_TASKS] == 99);
    assert(!rb_sim_hal_pending(&sim));
    assert(!rb_sim_hal_run_one(&sim));
    assert(api->enqueue(api->ctx, task_record, values) == RB_HAL_OK);
    rb_sim_hal_power_cycle(&sim);
    assert(rb_sim_hal_pending(&sim) == 0);
}
int main(void)
{
    test_files();
    test_faults_and_power_cycle();
    test_limits();
    test_tasks();
    puts("OK: HAL simulado - arquivos, falhas, memoria, energia e tarefas");
    return 0;
}
