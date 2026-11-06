/* SPDX-License-Identifier: GPL-2.0-only */
/* Mock deterministico totalmente em RAM. NAO emula CF, DMA, RTOS ou Canon. */
#ifndef REBOBINA_HAL_SIM_H
#define REBOBINA_HAL_SIM_H
#include "rebobina/hal.h"

#define RB_SIM_FILES 4u
#define RB_SIM_FILE_BYTES 4096u
#define RB_SIM_FILENAME 48u
#define RB_SIM_TASKS 8u
#define RB_SIM_ALLOC_SLOTS 8u
#define RB_SIM_ALLOC_BYTES 1024u

typedef enum rb_sim_fault {
    RB_SIM_FAULT_NONE = 0,
    RB_SIM_FAULT_CREATE,
    RB_SIM_FAULT_OPEN,
    RB_SIM_FAULT_READ,
    RB_SIM_FAULT_WRITE,
    RB_SIM_FAULT_ALLOC,
    RB_SIM_FAULT_QUEUE
} rb_sim_fault;

typedef struct rb_sim_file {
    char name[RB_SIM_FILENAME];
    unsigned char bytes[RB_SIM_FILE_BYTES];
    size_t size;
    size_t cursor;
    uint32_t generation;
    unsigned char exists;
    unsigned char mode; /* 0: fechado, 1: leitura, 2: escrita */
} rb_sim_file;

typedef struct rb_sim_work {
    rb_hal_task fn;
    void *arg;
} rb_sim_work;

typedef struct rb_sim_hal {
    rb_hal api;
    rb_sim_file files[RB_SIM_FILES];
    rb_sim_work tasks[RB_SIM_TASKS];
    unsigned head;
    unsigned count;
    _Alignas(16) unsigned char heap[RB_SIM_ALLOC_SLOTS][RB_SIM_ALLOC_BYTES];
    unsigned char busy[RB_SIM_ALLOC_SLOTS];
    unsigned char media_present;
    rb_sim_fault fail_once;
    size_t max_next_read;  /* 0 = sem limite de leitura */
    size_t cut_next_write; /* usado somente se cut_write_pending=1 */
    unsigned char cut_write_pending;
} rb_sim_hal;

void rb_sim_hal_init(rb_sim_hal *sim);
void rb_sim_hal_inject_fault(rb_sim_hal *sim, rb_sim_fault fault);
void rb_sim_hal_short_read_once(rb_sim_hal *sim, size_t max_bytes);
void rb_sim_hal_cut_write_once(rb_sim_hal *sim, size_t max_bytes);
void rb_sim_hal_set_media(rb_sim_hal *sim, int present);
/* Fecha handles e descarta tarefas, preserva bytes parciais no armazenamento simulado. */
void rb_sim_hal_power_cycle(rb_sim_hal *sim);
int rb_sim_hal_run_one(rb_sim_hal *sim);
unsigned rb_sim_hal_pending(const rb_sim_hal *sim);
size_t rb_sim_hal_file_size(const rb_sim_hal *sim, const char *name);

#endif
