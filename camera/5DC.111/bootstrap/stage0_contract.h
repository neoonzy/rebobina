/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Independent Rebobina 5DC.111 stage-0 structural contract.
 * Research derived from Magic Lantern Team historical 5DC.111 (GPLv2),
 * see docs/ML-5DC-REUSO.md. New code: Rebobina contributors (2026-11-13).
 *
 * IMPORTANT: NO value returned by this contract permits a physical boot.
 */
#ifndef RB_5DC_STAGE0_CONTRACT_H
#define RB_5DC_STAGE0_CONTRACT_H

#include <stdint.h>

#define RB_STAGE0_MAGIC UINT32_C(0x52423030)        /* "RB00" */
#define RB_STAGE0_PLATFORM_5DC UINT32_C(0x35444331) /* "5DC1" */
#define RB_STAGE0_FIRMWARE_111 UINT32_C(0x00010101)
#define RB_STAGE0_HISTORICAL_START UINT32_C(0x00980000)
#define RB_STAGE0_HISTORICAL_LIMIT UINT32_C(0x00A00000)

typedef struct rb_stage0_context {
    uint32_t magic;
    uint32_t platform;
    uint32_t firmware;
    uint32_t historical_start;
    uint32_t historical_limit;
} rb_stage0_context;

typedef enum rb_stage0_result {
    RB_STAGE0_NO_CONTEXT = 1,
    RB_STAGE0_BAD_MAGIC = 2,
    RB_STAGE0_BAD_PLATFORM = 3,
    RB_STAGE0_BAD_FIRMWARE = 4,
    RB_STAGE0_BAD_LAYOUT = 5,
    RB_STAGE0_RESEARCH_MATCH_ONLY = 6
} rb_stage0_result;

/* Pure function. "RESEARCH_MATCH_ONLY" is NOT authorization to boot. */
rb_stage0_result rb_stage0_check(const rb_stage0_context *context);

#endif
