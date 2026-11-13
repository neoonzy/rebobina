/* SPDX-License-Identifier: GPL-2.0-only */
/* Synthetic research fixtures, never actual Canon firmware memory. */
#include "boot_chain.h"
#include <assert.h>
#include <stdio.h>

static rb_5dc_boot_evidence synthetic_full_evidence(void)
{
    rb_5dc_boot_evidence e = {0};
    e.model = RB_5DC_BOOT_MODEL;
    e.firmware = RB_5DC_BOOT_FIRMWARE_111;
    e.historical_load_address = RB_5DC_HISTORIC_LOADER_START;
    e.historical_relocated_address = RB_5DC_HISTORIC_ML_RELOCATED;
    e.rom_read_prologue = RB_5DC_HISTORIC_ROM_READ_PROLOGUE;
    e.rom_write_prologue = RB_5DC_HISTORIC_ROM_WRITE_PROLOGUE;
    e.flags[0] = 0;
    e.flags[1] = 0;
    e.flags[2] = UINT32_MAX;
    e.rom_observed = 1; /* test fixture, NOT real observed firmware */
    e.source_identified = 1;
    e.ram_proven_safe = 1;
    e.canon_handoff_proven = 1;
    e.recovery_proven = 1;
    return e;
}
int main(void)
{
    rb_5dc_boot_evidence e = synthetic_full_evidence();
    const uint32_t wrong_flags[3] = {1, 0, UINT32_MAX};
    assert(rb_5dc_boot_preflight(NULL) == RB_5DC_BOOT_MISSING_INPUT);
    assert(rb_5dc_bootflag_classify(NULL) == RB_5DC_FLAG_UNRECOGNIZED);
    assert(rb_5dc_bootflag_classify(wrong_flags) == RB_5DC_FLAG_UNRECOGNIZED);
    assert(rb_5dc_bootflag_classify(e.flags) == RB_5DC_FLAG_OFF_HISTORIC);
    /* Even totally fabricated "complete" evidence cannot authorize boot. */
    assert(rb_5dc_boot_preflight(&e) == RB_5DC_BOOT_RESEARCH_READY_NOT_AUTHORIZED);
    e.model = 0x35443232u;
    assert(rb_5dc_boot_preflight(&e) == RB_5DC_BOOT_WRONG_MODEL);
    e = synthetic_full_evidence();
    e.firmware = 0x00010100u;
    assert(rb_5dc_boot_preflight(&e) == RB_5DC_BOOT_WRONG_FIRMWARE);
    e = synthetic_full_evidence();
    e.historical_load_address = RB_5DC_HISTORIC_ML_RELOCATED;
    assert(rb_5dc_boot_preflight(&e) == RB_5DC_BOOT_CONFLATED_LOAD_ADDRESSES);
    e = synthetic_full_evidence();
    e.historical_relocated_address = RB_5DC_HISTORIC_LOADER_START;
    assert(rb_5dc_boot_preflight(&e) == RB_5DC_BOOT_CONFLATED_LOAD_ADDRESSES);
    e = synthetic_full_evidence();
    e.rom_observed = 0;
    assert(rb_5dc_boot_preflight(&e) == RB_5DC_BOOT_NO_ROM_OBSERVATION);
    e = synthetic_full_evidence();
    e.rom_write_prologue ^= 1u;
    assert(rb_5dc_boot_preflight(&e) == RB_5DC_BOOT_ROM_SIGNATURE_MISMATCH);
    e = synthetic_full_evidence();
    e.flags[0] = 1;
    assert(rb_5dc_boot_preflight(&e) == RB_5DC_BOOT_UNKNOWN_FLAG_LAYOUT);
    e = synthetic_full_evidence();
    e.flags[1] = UINT32_MAX;
    assert(rb_5dc_bootflag_classify(e.flags) == RB_5DC_FLAG_ON_HISTORIC);
    assert(rb_5dc_boot_preflight(&e) == RB_5DC_BOOT_FLAG_ALREADY_ENABLED);
    e = synthetic_full_evidence();
    e.source_identified = 0;
    assert(rb_5dc_boot_preflight(&e) == RB_5DC_BOOT_UNVERIFIED_SOURCE);
    e = synthetic_full_evidence();
    e.ram_proven_safe = 0;
    assert(rb_5dc_boot_preflight(&e) == RB_5DC_BOOT_UNVERIFIED_MEMORY);
    e = synthetic_full_evidence();
    e.canon_handoff_proven = 0;
    assert(rb_5dc_boot_preflight(&e) == RB_5DC_BOOT_UNVERIFIED_HANDOFF);
    e = synthetic_full_evidence();
    e.recovery_proven = 0;
    assert(rb_5dc_boot_preflight(&e) == RB_5DC_BOOT_UNVERIFIED_RECOVERY);
    puts("OK: preflight 5DC.111 fail-closed (OFFLINE ONLY, no boot authorization)");
    return 0;
}
