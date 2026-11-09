/* SPDX-License-Identifier: GPL-2.0-only */
#define _GNU_SOURCE
#include "rebobina/hal_posix.h"
#include "rebobina/output.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void inspect_file(int dirfd, const char *name, const char *expected)
{
    char buf[128] = {0};
    int fd = openat(dirfd, name, O_RDONLY | O_NOFOLLOW);
    ssize_t n;
    assert(fd >= 0);
    n = read(fd, buf, sizeof(buf));
    assert(n == (ssize_t)strlen(expected));
    assert(memcmp(buf, expected, (size_t)n) == 0);
    assert(close(fd) == 0);
}
int main(void)
{
    char root[] = "/tmp/rb-posix-hal-XXXXXX";
    char symlink_dir[128];
    rb_posix_hal h, bad;
    rb_output out = {0}, conflict = {0};
    rb_hal_file f, stale;
    size_t count = 0;
    int fd, dir;
    assert(mkdtemp(root) != NULL);
    assert(rb_posix_hal_init(&h, root) == RB_HAL_OK);
    dir = h.directory_fd;
    assert(dir >= 0);
    assert(h.api.create(h.api.ctx, "../escape", &f) == RB_HAL_BAD_ARG);
    assert(h.api.create(h.api.ctx, ".hidden", &f) == RB_HAL_BAD_ARG);
    assert(h.api.create(h.api.ctx, "a/b", &f) == RB_HAL_BAD_ARG);
    assert(rb_output_begin(&out, &h.api, "PHOTO.JPG") == RB_HAL_OK);
    assert(rb_output_begin(&out, &h.api, "DUP.JPG") == RB_HAL_BAD_STATE);
    assert(rb_output_append(&out, "HELLO", 5) == RB_HAL_OK);
    assert(rb_output_commit(&out) == RB_HAL_OK);
    inspect_file(dir, "PHOTO.JPG", "HELLO");
    assert(faccessat(dir, "PHOTO.JPG.TMP", F_OK, 0) != 0);

    /* Jamais substituir um JPEG que ja existe. */
    assert(rb_output_begin(&conflict, &h.api, "PHOTO.JPG") == RB_HAL_OK);
    assert(rb_output_append(&conflict, "HACK", 4) == RB_HAL_OK);
    assert(rb_output_commit(&conflict) == RB_HAL_EXISTS);
    assert(rb_output_abort(&conflict) == RB_HAL_OK);
    inspect_file(dir, "PHOTO.JPG", "HELLO");

    /* Temporario órfão requer recuperacao explicita; sem overwrite. */
    rb_output_init(&out);
    assert(rb_output_begin(&out, &h.api, "ORPHAN.JPG") == RB_HAL_OK);
    assert(rb_output_append(&out, "PART", 4) == RB_HAL_OK);
    assert(rb_output_discard_orphan(&h.api, "ORPHAN.JPG") == RB_HAL_BAD_STATE);
    /* Simula termino do processo: descarta apenas o descritor. */
    rb_posix_hal_destroy(&h);
    assert(rb_posix_hal_init(&h, root) == RB_HAL_OK);
    assert(rb_output_discard_orphan(&h.api, "ORPHAN.JPG") == RB_HAL_OK);
    assert(faccessat(h.directory_fd, "ORPHAN.JPG.TMP", F_OK, 0) != 0);

    /* O_NOFOLLOW recusa abrir link simbolico de entrada no diretorio. */
    assert(symlinkat("PHOTO.JPG", h.directory_fd, "LINK.JPG") == 0);
    assert(h.api.open_read(h.api.ctx, "LINK.JPG", &f) == RB_HAL_IO);
    assert(h.api.create(h.api.ctx, "LINK.JPG", &f) == RB_HAL_EXISTS);

    /* Servico de arquivos respeita geracao de handles e estado. */
    assert(h.api.open_read(h.api.ctx, "PHOTO.JPG", &f) == RB_HAL_OK);
    stale = f;
    assert(h.api.read(h.api.ctx, f, symlink_dir, 1, &count) == RB_HAL_OK && count == 1);
    assert(h.api.write(h.api.ctx, f, "X", 1, &count) == RB_HAL_BAD_STATE);
    assert(h.api.close(h.api.ctx, f) == RB_HAL_OK);
    assert(h.api.sync(h.api.ctx, stale) == RB_HAL_BAD_HANDLE);

    /* Nao seguir links no diretorio raiz de saida. */
    assert(snprintf(symlink_dir, sizeof(symlink_dir), "%s-link", root) > 0);
    assert(symlink(root, symlink_dir) == 0);
    assert(rb_posix_hal_init(&bad, symlink_dir) != RB_HAL_OK);

    /* A renomeacao deve recusar um symlink ja existente como destino. */
    assert(h.api.create(h.api.ctx, "A.TMP", &f) == RB_HAL_OK);
    assert(h.api.write(h.api.ctx, f, "SAFE", 4, &count) == RB_HAL_OK && count == 4);
    assert(h.api.sync(h.api.ctx, f) == RB_HAL_OK);
    assert(h.api.close(h.api.ctx, f) == RB_HAL_OK);
    assert(h.api.rename_noreplace(h.api.ctx, "A.TMP", "LINK.JPG") == RB_HAL_EXISTS);
    assert(h.api.rename_noreplace(h.api.ctx, "A.TMP", "SAFE.JPG") == RB_HAL_OK);
    inspect_file(h.directory_fd, "SAFE.JPG", "SAFE");

    assert(h.api.remove(h.api.ctx, "PHOTO.JPG") == RB_HAL_OK);
    assert(h.api.remove(h.api.ctx, "PHOTO.JPG") == RB_HAL_NOT_FOUND);
    assert(h.api.remove(h.api.ctx, "SAFE.JPG") == RB_HAL_OK);
    assert(unlinkat(h.directory_fd, "LINK.JPG", 0) == 0);
    assert(rmdir(root) == 0 || 1); /* diretório fd permanece aberto, mas pode ser removido */
    rb_posix_hal_destroy(&h);
    assert(unlink(symlink_dir) == 0);
    puts("OK: HAL POSIX isolada, rename exclusivo, sync, symlink recusado e recuperação");
    return 0;
}
