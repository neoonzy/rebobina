/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * New Rebobina offline-only validation (2026-11-13).
 * Based on historic Magic Lantern Team GPLv2 5DC.111 research, with credits
 * and source links in docs/5DC-BOOT-CHAIN.md.
 *
 * The old installer changes the persistent bootdisk flag; this code does NOT.
 * It never reads memory-mapped ROM, calls Canon, or writes flash/cards.
 */
#include "boot_chain.h"

rb_5dc_bootflag_state rb_5dc_bootflag_classify(const uint32_t *flags)
{
    if (!flags || flags[0] != UINT32_C(0) ||
        flags[2] != UINT32_MAX)
        return RB_5DC_FLAG_UNRECOGNIZED;
    if (flags[1] == UINT32_C(0)) return RB_5DC_FLAG_OFF_HISTORIC;
    if (flags[1] == UINT32_MAX) return RB_5DC_FLAG_ON_HISTORIC;
    return RB_5DC_FLAG_UNRECOGNIZED;
}

rb_5dc_boot_status rb_5dc_boot_preflight(const rb_5dc_boot_evidence *e)
{
    rb_5dc_bootflag_state flag;
    if (!e) return RB_5DC_BOOT_MISSING_INPUT;
    if (e->model != RB_5DC_BOOT_MODEL)
        return RB_5DC_BOOT_WRONG_MODEL;
    if (e->firmware != RB_5DC_BOOT_FIRMWARE_111)
        return RB_5DC_BOOT_WRONG_FIRMWARE;
    /*
     * Historical installer linker: 0x00800000.
     * Historical ML relocated runtime: 0x00980000.
     * Neither address is independently verified as safe for Rebobina.
     */
    if (e->historical_load_address != RB_5DC_HISTORIC_LOADER_START ||
        e->historical_relocated_address != RB_5DC_HISTORIC_ML_RELOCATED ||
        e->historical_load_address == e->historical_relocated_address)
        return RB_5DC_BOOT_CONFLATED_LOAD_ADDRESSES;
    if (!e->rom_observed)
        return RB_5DC_BOOT_NO_ROM_OBSERVATION;
    if (e->rom_read_prologue != RB_5DC_HISTORIC_ROM_READ_PROLOGUE ||
        e->rom_write_prologue != RB_5DC_HISTORIC_ROM_WRITE_PROLOGUE)
        return RB_5DC_BOOT_ROM_SIGNATURE_MISMATCH;
    flag = rb_5dc_bootflag_classify(e->flags);
    if (flag == RB_5DC_FLAG_UNRECOGNIZED)
        return RB_5DC_BOOT_UNKNOWN_FLAG_LAYOUT;
    /* Never re-run the 2013 toggling installer if already enabled. */
    if (flag == RB_5DC_FLAG_ON_HISTORIC)
        return RB_5DC_BOOT_FLAG_ALREADY_ENABLED;
    if (!e->source_identified)
        return RB_5DC_BOOT_UNVERIFIED_SOURCE;
    if (!e->ram_proven_safe)
        return RB_5DC_BOOT_UNVERIFIED_MEMORY;
    if (!e->canon_handoff_proven)
        return RB_5DC_BOOT_UNVERIFIED_HANDOFF;
    if (!e->recovery_proven)
        return RB_5DC_BOOT_UNVERIFIED_RECOVERY;
    /* STILL only a structural rehearsal; never authorization to run. */
    return RB_5DC_BOOT_RESEARCH_READY_NOT_AUTHORIZED;
}
