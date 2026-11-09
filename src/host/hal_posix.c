/* SPDX-License-Identifier: GPL-2.0-only */
#define _GNU_SOURCE
#include "rebobina/hal_posix.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <linux/fs.h>
#include <unistd.h>

/* Somente basenames ASCII curtos. Nenhum caminho relativo, separador ou symlink. */
static int filename_ok(const char *s)
{
    unsigned i;
    if (!s || !s[0] || s[0] == '.') return 0;
    for (i = 0; i < 47u; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (!c) return i != 0;
        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == '.' || c == '_' || c == '-')) return 0;
    }
    return s[47] == '\0'; /* no maximo 47 caracteres */
}
static rb_hal_result errcode(int e)
{
    if (e == EEXIST) return RB_HAL_EXISTS;
    if (e == ENOENT) return RB_HAL_NOT_FOUND;
    if (e == ENOSPC || e == EDQUOT || e == EFBIG) return RB_HAL_NO_SPACE;
    return RB_HAL_IO;
}
static rb_hal_result open_file(void *arg, const char *name,
                               rb_hal_file *out, int write_mode)
{
    rb_posix_hal *p = (rb_posix_hal *)arg;
    int fd;
    if (!p || !out || !filename_ok(name)) return RB_HAL_BAD_ARG;
    out->slot = 0; out->generation = 0;
    if (p->directory_fd < 0) return RB_HAL_BAD_STATE;
    if (p->active_fd >= 0) return RB_HAL_BAD_STATE;
    fd = openat(p->directory_fd, name,
                O_CLOEXEC | O_NOFOLLOW |
                (write_mode ? O_WRONLY | O_CREAT | O_EXCL : O_RDONLY),
                0600);
    if (fd < 0) return errcode(errno);
    p->active_fd = fd;
    p->mode = write_mode ? 2u : 1u;
    ++p->generation;
    if (!p->generation) ++p->generation;
    out->slot = 1;
    out->generation = p->generation;
    return RB_HAL_OK;
}
static rb_hal_result create_file(void *arg, const char *name, rb_hal_file *out)
{
    return open_file(arg, name, out, 1);
}
static rb_hal_result open_read(void *arg, const char *name, rb_hal_file *out)
{
    return open_file(arg, name, out, 0);
}
static int handle_ok(const rb_posix_hal *p, rb_hal_file h)
{
    return p && p->active_fd >= 0 && p->mode && h.slot == 1 &&
           h.generation && h.generation == p->generation;
}
static rb_hal_result read_file(void *arg, rb_hal_file h,
                               void *buf, size_t len, size_t *count)
{
    rb_posix_hal *p = (rb_posix_hal *)arg;
    ssize_t got;
    if (!p || !count || (len && !buf)) return RB_HAL_BAD_ARG;
    *count = 0;
    if (!handle_ok(p, h)) return RB_HAL_BAD_HANDLE;
    if (p->mode != 1) return RB_HAL_BAD_STATE;
    if (len > 1048576u) len = 1048576u;
    do { got = read(p->active_fd, buf, len); } while (got < 0 && errno == EINTR);
    if (got < 0) return errcode(errno);
    *count = (size_t)got;
    return RB_HAL_OK;
}
static rb_hal_result write_file(void *arg, rb_hal_file h,
                                const void *buf, size_t len, size_t *count)
{
    rb_posix_hal *p = (rb_posix_hal *)arg;
    ssize_t got;
    if (!p || !count || (len && !buf)) return RB_HAL_BAD_ARG;
    *count = 0;
    if (!handle_ok(p, h)) return RB_HAL_BAD_HANDLE;
    if (p->mode != 2) return RB_HAL_BAD_STATE;
    if (len > 1048576u) len = 1048576u;
    do { got = write(p->active_fd, buf, len); } while (got < 0 && errno == EINTR);
    if (got < 0) return errcode(errno);
    *count = (size_t)got;
    return RB_HAL_OK;
}
static rb_hal_result sync_file(void *arg, rb_hal_file h)
{
    rb_posix_hal *p = (rb_posix_hal *)arg;
    if (!handle_ok(p, h)) return RB_HAL_BAD_HANDLE;
    if (p->mode != 2) return RB_HAL_BAD_STATE;
    return fsync(p->active_fd) == 0 ? RB_HAL_OK : errcode(errno);
}
static rb_hal_result close_file(void *arg, rb_hal_file h)
{
    rb_posix_hal *p = (rb_posix_hal *)arg;
    int fd;
    if (!handle_ok(p, h)) return RB_HAL_BAD_HANDLE;
    fd = p->active_fd;
    p->active_fd = -1;
    p->mode = 0;
    ++p->generation;
    /* close() apos EINTR pode ter fechado o descritor. Nao repetir. */
    return close(fd) == 0 ? RB_HAL_OK : RB_HAL_IO;
}
static rb_hal_result rename_exclusive(void *arg, const char *from, const char *to)
{
    rb_posix_hal *p = (rb_posix_hal *)arg;
    if (!p || !filename_ok(from) || !filename_ok(to)) return RB_HAL_BAD_ARG;
    if (p->active_fd >= 0 || p->directory_fd < 0) return RB_HAL_BAD_STATE;
    /* Linux renameat2: RENAME_NOREPLACE, sem fallback que possa sobrescrever. */
    if (syscall(SYS_renameat2, p->directory_fd, from, p->directory_fd, to,
                  RENAME_NOREPLACE) != 0) return errcode(errno);
    /* Se fsync do diretorio falhar apos rename, final pode ja existir. */
    if (fsync(p->directory_fd) != 0) return RB_HAL_IO;
    return RB_HAL_OK;
}
static rb_hal_result remove_file(void *arg, const char *name)
{
    rb_posix_hal *p = (rb_posix_hal *)arg;
    if (!p || !filename_ok(name)) return RB_HAL_BAD_ARG;
    if (p->active_fd >= 0 || p->directory_fd < 0) return RB_HAL_BAD_STATE;
    if (unlinkat(p->directory_fd, name, 0) != 0) return errcode(errno);
    if (fsync(p->directory_fd) != 0) return RB_HAL_IO;
    return RB_HAL_OK;
}
static void *allocate(void *arg, size_t count)
{
    (void)arg;
    return count ? malloc(count) : NULL;
}
static void release(void *arg, void *memory)
{
    (void)arg;
    free(memory);
}
static rb_hal_result enqueue(void *arg, rb_hal_task task, void *param)
{
    (void)arg; (void)task; (void)param;
    return RB_HAL_BAD_STATE; /* Nao implementa agendamento no Linux host */
}
rb_hal_result rb_posix_hal_init(rb_posix_hal *p, const char *directory)
{
    int dirfd;
    rb_posix_hal blank = {0};
    if (!p || !directory || !directory[0]) return RB_HAL_BAD_ARG;
    *p = blank;
    p->directory_fd = -1;
    p->active_fd = -1;
    dirfd = open(directory, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (dirfd < 0) return errcode(errno);
    p->directory_fd = dirfd;
    p->api.ctx = p;
    p->api.create = create_file;
    p->api.open_read = open_read;
    p->api.read = read_file;
    p->api.write = write_file;
    p->api.close = close_file;
    p->api.sync = sync_file;
    p->api.rename_noreplace = rename_exclusive;
    p->api.remove = remove_file;
    p->api.alloc = allocate;
    p->api.release = release;
    p->api.enqueue = enqueue;
    return RB_HAL_OK;
}
void rb_posix_hal_destroy(rb_posix_hal *p)
{
    if (!p) return;
    if (p->active_fd >= 0) (void)close(p->active_fd);
    if (p->directory_fd >= 0) (void)close(p->directory_fd);
    p->active_fd = -1;
    p->directory_fd = -1;
    p->mode = 0;
}
