/* SPDX-License-Identifier: GPL-2.0-only */
/* Verificacao defensiva de JPEG embutido em CR2 via motor upstream Soup.
 * NAO valida/descomprime o RAW nem autentica o modelo de camera.
 */
#ifndef REBOBINA_CR2_HOST_H
#define REBOBINA_CR2_HOST_H

#include <stddef.h>
#include <stdint.h>

typedef struct rb_cr2_host_preview {
    uint32_t offset;
    uint32_t bytes;
    int width;
    int height;
} rb_cr2_host_preview;

/* 0 em sucesso; -1 se header, dimensoes, strip ou JPEG forem invalidos.
 * Exige arquivo completo em buffer; o tamanho e validado ANTES de acessar
 * a faixa de JPEG. Nao abrir arquivos nem escrever no disco.
 */
int rb_cr2_host_find_preview(const void *data, size_t length,
                            rb_cr2_host_preview *out);
#endif
