/* SPDX-License-Identifier: GPL-2.0-only */
/* Contrato portavel experimental; NENHUM endereco ROM ou IO Canon. */
#ifndef REBOBINA_HAL_H
#define REBOBINA_HAL_H
#include <stddef.h>
#include <stdint.h>

typedef enum rb_hal_result {
    RB_HAL_OK = 0,
    RB_HAL_BAD_ARG,
    RB_HAL_NO_MEDIA,
    RB_HAL_NOT_FOUND,
    RB_HAL_EXISTS,
    RB_HAL_NO_SPACE,
    RB_HAL_IO,
    RB_HAL_BAD_HANDLE,
    RB_HAL_BAD_STATE
} rb_hal_result;

/* Nao e um descritor Canon. Handles antigos expiram ao fechar/abrir. */
typedef struct rb_hal_file {
    uint32_t slot;
    uint32_t generation;
} rb_hal_file;

typedef void (*rb_hal_task)(void *arg);

typedef struct rb_hal {
    void *ctx;
    rb_hal_result (*create)(void *ctx, const char *name, rb_hal_file *out);
    rb_hal_result (*open_read)(void *ctx, const char *name, rb_hal_file *out);
    rb_hal_result (*read)(void *ctx, rb_hal_file handle,
                          void *buffer, size_t length, size_t *count);
    rb_hal_result (*write)(void *ctx, rb_hal_file handle,
                           const void *buffer, size_t length, size_t *count);
    rb_hal_result (*close)(void *ctx, rb_hal_file handle);
    void *(*alloc)(void *ctx, size_t size);
    void (*release)(void *ctx, void *ptr);
    rb_hal_result (*enqueue)(void *ctx, rb_hal_task fn, void *arg);
} rb_hal;
#endif
