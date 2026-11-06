/* SPDX-License-Identifier: GPL-2.0-only */
/* Saida em duas fases: arquivo temporario, sync, close, rename exclusivo.
 * Contrato experimental e testes host. NAO garante atomicidade no CF real.
 */
#ifndef REBOBINA_OUTPUT_H
#define REBOBINA_OUTPUT_H
#include "rebobina/hal.h"

#define RB_OUTPUT_NAME_BYTES 48u

typedef struct rb_output {
    rb_hal *hal;
    rb_hal_file handle;
    char final_name[RB_OUTPUT_NAME_BYTES];
    char temp_name[RB_OUTPUT_NAME_BYTES];
    unsigned char staged;
    unsigned char open;
    unsigned char failed;
    unsigned char published;
} rb_output;

/* Inicializar antes do primeiro begin; nao reiniciar com escrita pendente. */
void rb_output_init(rb_output *output);
/* Nao sobrescreve um arquivo final existente. Usar nomes curtos ASCII. */
rb_hal_result rb_output_begin(rb_output *output, rb_hal *hal, const char *final_name);
rb_hal_result rb_output_append(rb_output *output, const void *bytes, size_t n);
rb_hal_result rb_output_commit(rb_output *output);
/* Em caso de falha: fecha e apaga APENAS o temporario, se houver midia. */
rb_hal_result rb_output_abort(rb_output *output);
/* Acao explicita de recuperacao apos power cycle: remove apenas o .TMP. */
rb_hal_result rb_output_discard_orphan(rb_hal *hal, const char *final_name);
#endif
