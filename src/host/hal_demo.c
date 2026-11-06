/* SPDX-License-Identifier: GPL-2.0-only */
/* Exemplo apenas em RAM. NAO processa CR2 ou cria JPEG real. */
#include "rebobina/hal_sim.h"
#include <stdio.h>

static void work(void *arg) { ++*(unsigned *)arg; }
int main(void)
{
    rb_sim_hal sim;
    rb_hal_file f;
    size_t n = 0;
    unsigned completed = 0;
    static const unsigned char demo[] = {'R','B','S','I','M','0','1'};
    rb_sim_hal_init(&sim);
    if (sim.api.create(sim.api.ctx, "SAMPLE.BIN", &f) != RB_HAL_OK) return 1;
    if (sim.api.write(sim.api.ctx, f, demo, sizeof(demo), &n) != RB_HAL_OK ||
        n != sizeof(demo)) return 2;
    if (sim.api.close(sim.api.ctx, f) != RB_HAL_OK) return 3;
    if (sim.api.enqueue(sim.api.ctx, work, &completed) != RB_HAL_OK) return 4;
    (void)rb_sim_hal_run_one(&sim);
    printf("Rebobina HAL host: %lu bytes em RAM, %u tarefa executada. "
           "NAO e um arquivo Canon ou processamento Soup.\n",
           (unsigned long)rb_sim_hal_file_size(&sim, "SAMPLE.BIN"), completed);
    return completed == 1 ? 0 : 5;
}
