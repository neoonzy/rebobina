/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * 5DC.111 boot-chain research contract (no IO, no memory mapped reads).
 * Inspired by Magic Lantern Team 2013 installer/5DC.111 and platform/5DC.111,
 * GPLv2. Original sources and restrictions: docs/5DC-BOOT-CHAIN.md.
 * Newly implemented for Rebobina 2026-11-13.
 *
 * All outcomes are OFFLINE RESEARCH RESULTS. No result authorizes boot.
 */
#ifndef RB_5DC_BOOT_CHAIN_H
#define RB_5DC_BOOT_CHAIN_H
#include <stdint.h>

#define RB_5DC_BOOT_MODEL UINT32_C(0x35444331) /* 5DC1 */
#define RB_5DC_BOOT_FIRMWARE_111 UINT32_C(0x00010101)
#define RB_5DC_HISTORIC_LOADER_START UINT32_C(0x00800000)
#define RB_5DC_HISTORIC_ML_RELOCATED UINT32_C(0x00980000)
#define RB_5DC_HISTORIC_ROM_READ_PROLOGUE UINT32_C(0xE52DE004)
#define RB_5DC_HISTORIC_ROM_WRITE_PROLOGUE UINT32_C(0xE92D41F0)

typedef enum rb_5dc_boot_status {
    RB_5DC_BOOT_MISSING_INPUT = 1,
    RB_5DC_BOOT_WRONG_MODEL = 2,
    RB_5DC_BOOT_WRONG_FIRMWARE = 3,
    RB_5DC_BOOT_CONFLATED_LOAD_ADDRESSES = 4,
    RB_5DC_BOOT_NO_ROM_OBSERVATION = 5,
    RB_5DC_BOOT_ROM_SIGNATURE_MISMATCH = 6,
    RB_5DC_BOOT_UNKNOWN_FLAG_LAYOUT = 7,
    RB_5DC_BOOT_FLAG_ALREADY_ENABLED = 8,
    RB_5DC_BOOT_UNVERIFIED_SOURCE = 9,
    RB_5DC_BOOT_UNVERIFIED_MEMORY = 10,
    RB_5DC_BOOT_UNVERIFIED_HANDOFF = 11,
    RB_5DC_BOOT_UNVERIFIED_RECOVERY = 12,
    RB_5DC_BOOT_RESEARCH_READY_NOT_AUTHORIZED = 13
} rb_5dc_boot_status;

/*
 * Filled by a FUTURE separately validated read-only diagnostics adapter.
 * Tests use synthetic values; neither this struct nor its source provides
 * read/flash/boot APIs. Values copied from history are not measurements.
 */
typedef struct rb_5dc_boot_evidence {
    uint32_t model;
    uint32_t firmware;
    uint32_t historical_load_address;
    uint32_t historical_relocated_address;
    uint32_t rom_read_prologue;
    uint32_t rom_write_prologue;
    uint32_t flags[3];
    unsigned rom_observed;
    unsigned source_identified;
    unsigned ram_proven_safe;
    unsigned canon_handoff_proven;
    unsigned recovery_proven;
} rb_5dc_boot_evidence;

typedef enum rb_5dc_bootflag_state {
    RB_5DC_FLAG_UNRECOGNIZED = 0,
    RB_5DC_FLAG_OFF_HISTORIC = 1,
    RB_5DC_FLAG_ON_HISTORIC = 2
} rb_5dc_bootflag_state;

/* Only classification. Never toggles flags or returns modified values. */
rb_5dc_bootflag_state rb_5dc_bootflag_classify(const uint32_t *flags);

/* Fail closed. Even with all flags true, NOT a physical test authorization. */
rb_5dc_boot_status rb_5dc_boot_preflight(const rb_5dc_boot_evidence *evidence);

#endif
