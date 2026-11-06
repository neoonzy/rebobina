/* SPDX-License-Identifier: GPL-2.0-only */
/* Simulacao em RAM com falhas deterministicas. Nenhum acesso ao disco fisico. */
#include "rebobina/hal_sim.h"

static int same(const char *a, const char *b)
{
    while (*a && *b) {
        if (*a != *b) return 0;
        ++a; ++b;
    }
    return *a == *b;
}
static int safe_name(const char *name)
{
    size_t i;
    if (!name || !name[0]) return 0;
    for (i = 0; i < RB_SIM_FILENAME; ++i) {
        unsigned char c = (unsigned char)name[i];
        if (!c) return i > 0 && i < RB_SIM_FILENAME;
        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-'))
            return 0;
    }
    return 0;
}
static rb_sim_file *find(rb_sim_hal *sim, const char *name)
{
    unsigned i;
    for (i = 0; i < RB_SIM_FILES; ++i)
        if (sim->files[i].exists && same(sim->files[i].name, name))
            return &sim->files[i];
    return 0;
}
static int fault(rb_sim_hal *sim, rb_sim_fault target)
{
    if (sim->fail_once != target) return 0;
    sim->fail_once = RB_SIM_FAULT_NONE;
    return 1;
}
static rb_hal_result create(void *ctx, const char *name, rb_hal_file *out)
{
    rb_sim_hal *sim = ctx;
    unsigned i, j;
    rb_sim_file *f;
    if (!sim || !out || !safe_name(name)) return RB_HAL_BAD_ARG;
    out->slot = 0;
    out->generation = 0;
    if (!sim->media_present) return RB_HAL_NO_MEDIA;
    if (fault(sim, RB_SIM_FAULT_CREATE)) return RB_HAL_IO;
    if (find(sim, name)) return RB_HAL_EXISTS; /* Nunca sobrescrever um original */
    for (i = 0; i < RB_SIM_FILES; ++i) if (!sim->files[i].exists) break;
    if (i == RB_SIM_FILES) return RB_HAL_NO_SPACE;
    f = &sim->files[i];
    for (j = 0; j < RB_SIM_FILENAME; ++j) {
        f->name[j] = name[j];
        if (!name[j]) break;
    }
    f->size = 0;
    f->cursor = 0;
    f->exists = 1;
    f->mode = 2;
    ++f->generation;
    if (!f->generation) ++f->generation;
    out->slot = i + 1u;
    out->generation = f->generation;
    return RB_HAL_OK;
}
static rb_hal_result open_read(void *ctx, const char *name, rb_hal_file *out)
{
    rb_sim_hal *sim = ctx;
    rb_sim_file *f;
    if (!sim || !out || !safe_name(name)) return RB_HAL_BAD_ARG;
    out->slot = 0;
    out->generation = 0;
    if (!sim->media_present) return RB_HAL_NO_MEDIA;
    if (fault(sim, RB_SIM_FAULT_OPEN)) return RB_HAL_IO;
    f = find(sim, name);
    if (!f) return RB_HAL_NOT_FOUND;
    if (f->mode) return RB_HAL_BAD_STATE;
    f->mode = 1;
    f->cursor = 0;
    ++f->generation;
    if (!f->generation) ++f->generation;
    out->slot = (unsigned)(f - sim->files) + 1u;
    out->generation = f->generation;
    return RB_HAL_OK;
}
static rb_sim_file *checked(rb_sim_hal *sim, rb_hal_file handle)
{
    rb_sim_file *f;
    if (!sim || handle.slot == 0 || handle.slot > RB_SIM_FILES) return 0;
    f = &sim->files[handle.slot - 1u];
    if (!f->exists || !f->mode || !handle.generation ||
        handle.generation != f->generation)
        return 0;
    return f;
}
static rb_hal_result read_file(void *ctx, rb_hal_file handle,
                               void *buffer, size_t length, size_t *count)
{
    rb_sim_hal *sim = ctx;
    rb_sim_file *f;
    size_t i, n;
    if (!sim || !count || (length && !buffer)) return RB_HAL_BAD_ARG;
    *count = 0;
    if (!sim->media_present) return RB_HAL_NO_MEDIA;
    f = checked(sim, handle);
    if (!f) return RB_HAL_BAD_HANDLE;
    if (f->mode != 1) return RB_HAL_BAD_STATE;
    if (fault(sim, RB_SIM_FAULT_READ)) return RB_HAL_IO;
    n = f->size - f->cursor;
    if (n > length) n = length;
    if (sim->max_next_read && n > sim->max_next_read) n = sim->max_next_read;
    sim->max_next_read = 0;
    for (i = 0; i < n; ++i) ((unsigned char *)buffer)[i] = f->bytes[f->cursor+i];
    f->cursor += n;
    *count = n; /* zero com status OK = EOF */
    return RB_HAL_OK;
}
static rb_hal_result write_file(void *ctx, rb_hal_file handle,
                                const void *buffer, size_t length, size_t *count)
{
    rb_sim_hal *sim = ctx;
    rb_sim_file *f;
    size_t i, n;
    int interrupted;
    if (!sim || !count || (length && !buffer)) return RB_HAL_BAD_ARG;
    *count = 0;
    if (!sim->media_present) return RB_HAL_NO_MEDIA;
    f = checked(sim, handle);
    if (!f) return RB_HAL_BAD_HANDLE;
    if (f->mode != 2) return RB_HAL_BAD_STATE;
    if (fault(sim, RB_SIM_FAULT_WRITE)) return RB_HAL_IO;
    if (length > RB_SIM_FILE_BYTES - f->cursor) return RB_HAL_NO_SPACE;
    interrupted = sim->cut_write_pending;
    n = length;
    if (interrupted && n > sim->cut_next_write) n = sim->cut_next_write;
    if (sim->max_next_write && n > sim->max_next_write) n = sim->max_next_write;
    sim->max_next_write = 0;
    sim->cut_write_pending = 0;
    for (i = 0; i < n; ++i) f->bytes[f->cursor+i] = ((const unsigned char *)buffer)[i];
    f->cursor += n;
    if (f->cursor > f->size) f->size = f->cursor;
    *count = n;
    return interrupted ? RB_HAL_IO : RB_HAL_OK; /* fail mesmo apos gravacao parcial */
}
static rb_hal_result close_file(void *ctx, rb_hal_file handle)
{
    rb_sim_hal *sim = ctx;
    rb_sim_file *f = checked(sim, handle);
    if (!f) return RB_HAL_BAD_HANDLE;
    f->mode = 0;
    return RB_HAL_OK; /* Fechamento local liberado mesmo apos remocao de midia. */
}
static rb_hal_result sync_file(void *ctx, rb_hal_file handle)
{
    rb_sim_hal *sim = ctx;
    rb_sim_file *f;
    if (!sim) return RB_HAL_BAD_ARG;
    if (!sim->media_present) return RB_HAL_NO_MEDIA;
    f = checked(sim, handle);
    if (!f) return RB_HAL_BAD_HANDLE;
    if (f->mode != 2) return RB_HAL_BAD_STATE;
    if (fault(sim, RB_SIM_FAULT_SYNC)) return RB_HAL_IO;
    /* RAM: nao e um fsync real nem uma garantia de durabilidade. */
    return RB_HAL_OK;
}
static rb_hal_result rename_noreplace(void *ctx, const char *old, const char *name)
{
    rb_sim_hal *sim = ctx;
    rb_sim_file *f;
    unsigned i;
    if (!sim || !safe_name(old) || !safe_name(name) || same(old, name))
        return RB_HAL_BAD_ARG;
    if (!sim->media_present) return RB_HAL_NO_MEDIA;
    if (fault(sim, RB_SIM_FAULT_RENAME)) return RB_HAL_IO;
    f = find(sim, old);
    if (!f) return RB_HAL_NOT_FOUND;
    if (f->mode) return RB_HAL_BAD_STATE;
    if (find(sim, name)) return RB_HAL_EXISTS;
    for (i = 0; i < RB_SIM_FILENAME; ++i) {
        f->name[i] = name[i];
        if (!name[i]) break;
    }
    return RB_HAL_OK;
}
static rb_hal_result remove_file(void *ctx, const char *name)
{
    rb_sim_hal *sim = ctx;
    rb_sim_file *f;
    if (!sim || !safe_name(name)) return RB_HAL_BAD_ARG;
    if (!sim->media_present) return RB_HAL_NO_MEDIA;
    if (fault(sim, RB_SIM_FAULT_REMOVE)) return RB_HAL_IO;
    f = find(sim, name);
    if (!f) return RB_HAL_NOT_FOUND;
    if (f->mode) return RB_HAL_BAD_STATE;
    f->exists = 0;
    f->name[0] = '\0';
    f->size = 0;
    f->cursor = 0;
    ++f->generation;
    if (!f->generation) ++f->generation;
    return RB_HAL_OK;
}
static void *allocate(void *ctx, size_t bytes)
{
    rb_sim_hal *sim = ctx;
    unsigned i;
    if (!sim || !bytes || bytes > RB_SIM_ALLOC_BYTES) return 0;
    if (fault(sim, RB_SIM_FAULT_ALLOC)) return 0;
    for (i = 0; i < RB_SIM_ALLOC_SLOTS; ++i)
        if (!sim->busy[i]) {
            sim->busy[i] = 1;
            return sim->heap[i];
        }
    return 0;
}
static void release(void *ctx, void *ptr)
{
    rb_sim_hal *sim = ctx;
    unsigned i, j;
    if (!sim || !ptr) return;
    for (i = 0; i < RB_SIM_ALLOC_SLOTS; ++i)
        if (sim->heap[i] == ptr && sim->busy[i]) {
            for (j = 0; j < RB_SIM_ALLOC_BYTES; ++j) sim->heap[i][j] = 0;
            sim->busy[i] = 0;
            return;
        }
}
static rb_hal_result enqueue(void *ctx, rb_hal_task fn, void *arg)
{
    rb_sim_hal *sim = ctx;
    unsigned index;
    if (!sim || !fn) return RB_HAL_BAD_ARG;
    if (fault(sim, RB_SIM_FAULT_QUEUE)) return RB_HAL_IO;
    if (sim->count == RB_SIM_TASKS) return RB_HAL_NO_SPACE;
    index = (sim->head + sim->count) % RB_SIM_TASKS;
    sim->tasks[index].fn = fn;
    sim->tasks[index].arg = arg;
    ++sim->count;
    return RB_HAL_OK;
}

void rb_sim_hal_init(rb_sim_hal *sim)
{
    unsigned char *bytes;
    size_t i;
    if (!sim) return;
    bytes = (unsigned char *)sim;
    for (i = 0; i < sizeof(*sim); ++i) bytes[i] = 0;
    sim->media_present = 1;
    sim->api.ctx = sim;
    sim->api.create = create;
    sim->api.open_read = open_read;
    sim->api.read = read_file;
    sim->api.write = write_file;
    sim->api.close = close_file;
    sim->api.sync = sync_file;
    sim->api.rename_noreplace = rename_noreplace;
    sim->api.remove = remove_file;
    sim->api.alloc = allocate;
    sim->api.release = release;
    sim->api.enqueue = enqueue;
}
void rb_sim_hal_inject_fault(rb_sim_hal *sim, rb_sim_fault fault_type)
{
    if (sim && fault_type >= RB_SIM_FAULT_NONE && fault_type <= RB_SIM_FAULT_REMOVE)
        sim->fail_once = fault_type;
}
void rb_sim_hal_short_read_once(rb_sim_hal *sim, size_t max_bytes)
{
    if (sim) sim->max_next_read = max_bytes;
}
void rb_sim_hal_short_write_once(rb_sim_hal *sim, size_t max_bytes)
{
    if (sim) sim->max_next_write = max_bytes;
}
void rb_sim_hal_cut_write_once(rb_sim_hal *sim, size_t max_bytes)
{
    if (!sim) return;
    sim->cut_next_write = max_bytes;
    sim->cut_write_pending = 1;
}
void rb_sim_hal_set_media(rb_sim_hal *sim, int present)
{
    if (sim) sim->media_present = present != 0;
}
void rb_sim_hal_power_cycle(rb_sim_hal *sim)
{
    unsigned i;
    if (!sim) return;
    for (i = 0; i < RB_SIM_FILES; ++i) {
        sim->files[i].mode = 0;
        ++sim->files[i].generation;
    }
    sim->head = 0;
    sim->count = 0;
    sim->max_next_read = 0;
    sim->max_next_write = 0;
    sim->cut_write_pending = 0;
    sim->fail_once = RB_SIM_FAULT_NONE;
    for (i = 0; i < RB_SIM_ALLOC_SLOTS; ++i) sim->busy[i] = 0;
}
int rb_sim_hal_run_one(rb_sim_hal *sim)
{
    rb_sim_work task;
    if (!sim || !sim->count) return 0;
    task = sim->tasks[sim->head];
    sim->head = (sim->head + 1u) % RB_SIM_TASKS;
    --sim->count;
    task.fn(task.arg);
    return 1;
}
unsigned rb_sim_hal_pending(const rb_sim_hal *sim)
{
    return sim ? sim->count : 0;
}
size_t rb_sim_hal_file_size(const rb_sim_hal *sim, const char *name)
{
    rb_sim_file *f;
    if (!sim || !safe_name(name)) return 0;
    f = find((rb_sim_hal *)sim, name);
    return f ? f->size : 0;
}
