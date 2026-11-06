/* SPDX-License-Identifier: GPL-2.0-only */
#include "rebobina/output.h"

static int allowed(unsigned char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-';
}
static int output_names(const char *src, char *final, char *temp)
{
    size_t i;
    if (!src) return 0;
    for (i = 0; i < RB_OUTPUT_NAME_BYTES; ++i) {
        unsigned char c = (unsigned char)src[i];
        if (!c) break;
        if (!allowed(c)) return 0;
        /* Reserva 4 bytes para ".TMP" e 1 para terminador. */
        if (i >= RB_OUTPUT_NAME_BYTES - 5u) return 0;
        final[i] = (char)c;
        temp[i] = (char)c;
    }
    if (!i || i >= RB_OUTPUT_NAME_BYTES - 4u) return 0;
    final[i] = '\0';
    temp[i] = '.';
    temp[i + 1u] = 'T';
    temp[i + 2u] = 'M';
    temp[i + 3u] = 'P';
    temp[i + 4u] = '\0';
    return 1;
}
static int valid_hal(const rb_hal *hal)
{
    return hal && hal->ctx && hal->create && hal->write && hal->close &&
           hal->sync && hal->rename_noreplace && hal->remove;
}
rb_hal_result rb_output_begin(rb_output *out, rb_hal *hal, const char *final_name)
{
    rb_hal_result rc;
    rb_output initial = {0};
    if (!out || !valid_hal(hal)) return RB_HAL_BAD_ARG;
    /* Antes de zerar o objeto, quem chamou deve ter encerrado tentativa anterior. */
    *out = initial;
    if (!output_names(final_name, out->final_name, out->temp_name))
        return RB_HAL_BAD_ARG;
    out->hal = hal;
    rc = hal->create(hal->ctx, out->temp_name, &out->handle);
    if (rc != RB_HAL_OK) return rc;
    out->staged = 1;
    out->open = 1;
    return RB_HAL_OK;
}
rb_hal_result rb_output_append(rb_output *out, const void *bytes, size_t n)
{
    size_t offset = 0;
    if (!out || !out->hal || !out->staged || !out->open ||
        out->failed || out->published) return RB_HAL_BAD_STATE;
    if (n && !bytes) return RB_HAL_BAD_ARG;
    while (offset < n) {
        size_t done = 0;
        rb_hal_result rc = out->hal->write(out->hal->ctx, out->handle,
                                            (const unsigned char *)bytes + offset,
                                            n - offset, &done);
        if (done > n - offset || rc != RB_HAL_OK || !done) {
            out->failed = 1; /* Nunca reutilizar arquivo parcialmente gravado. */
            return rc == RB_HAL_OK ? RB_HAL_IO : rc;
        }
        offset += done;
    }
    return RB_HAL_OK;
}
rb_hal_result rb_output_commit(rb_output *out)
{
    rb_hal_result rc;
    if (!out || !out->hal || !out->staged || !out->open ||
        out->failed || out->published) return RB_HAL_BAD_STATE;
    rc = out->hal->sync(out->hal->ctx, out->handle);
    if (rc != RB_HAL_OK) {
        out->failed = 1;
        return rc;
    }
    rc = out->hal->close(out->hal->ctx, out->handle);
    if (rc != RB_HAL_OK) {
        out->failed = 1;
        return rc;
    }
    out->open = 0;
    rc = out->hal->rename_noreplace(out->hal->ctx,
                                    out->temp_name, out->final_name);
    if (rc != RB_HAL_OK) {
        out->failed = 1;
        return rc;
    }
    out->published = 1;
    out->staged = 0;
    return RB_HAL_OK;
}
rb_hal_result rb_output_abort(rb_output *out)
{
    rb_hal_result rc, closed = RB_HAL_OK;
    if (!out || !out->hal || !out->staged || out->published)
        return RB_HAL_BAD_STATE;
    if (out->open) {
        closed = out->hal->close(out->hal->ctx, out->handle);
        if (closed == RB_HAL_OK || closed == RB_HAL_BAD_HANDLE)
            out->open = 0;
    }
    if (out->open) return closed; /* Nao apagar arquivo ainda aberto. */
    rc = out->hal->remove(out->hal->ctx, out->temp_name);
    if (rc == RB_HAL_OK || rc == RB_HAL_NOT_FOUND) {
        out->staged = 0;
        out->failed = 0;
        return RB_HAL_OK;
    }
    return rc;
}
rb_hal_result rb_output_discard_orphan(rb_hal *hal, const char *name)
{
    char final[RB_OUTPUT_NAME_BYTES] = {0};
    char temp[RB_OUTPUT_NAME_BYTES] = {0};
    rb_hal_result rc;
    if (!valid_hal(hal) || !output_names(name, final, temp))
        return RB_HAL_BAD_ARG;
    rc = hal->remove(hal->ctx, temp);
    return rc == RB_HAL_NOT_FOUND ? RB_HAL_OK : rc;
}
