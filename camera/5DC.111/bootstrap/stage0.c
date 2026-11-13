/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Rebobina freestanding startup *demonstrator*.
 * New code (2026-11-13). Historic 5DC.111 memory concept:
 * Magic Lantern Team, GPLv2, documented in docs/ML-5DC-REUSO.md.
 * NO camera boot, ROM calls, card access or Canon firmware handoff.
 */
#include "stage0_contract.h"
#include "boot_chain.h"
#include "rebobina/module.h"
#include "rebobina/ui.h"

extern unsigned char __rb_stage0_bss_start[];
extern unsigned char __rb_stage0_bss_end[];

static rb_registry rb_stage0_registry;
static rb_ui rb_stage0_ui;
volatile uint32_t rb_stage0_last_result;
volatile uint32_t rb_stage0_boot_preflight_result;

/* Retained in ELF to prevent accidental confusion with a runnable image. */
__attribute__((used, section(".rodata.rb_offline_marker")))
const char rb_stage0_warning[] =
    "REBOBINA 5DC STAGE0 OFFLINE ONLY - NOT CAMERA BOOTABLE";

void rb_stage0_prepare(void)
{
    volatile unsigned char *current = __rb_stage0_bss_start;
    const unsigned char *const end = __rb_stage0_bss_end;
    static const rb_stage0_context historical_fixture = {
        RB_STAGE0_MAGIC, RB_STAGE0_PLATFORM_5DC, RB_STAGE0_FIRMWARE_111,
        RB_STAGE0_HISTORICAL_START, RB_STAGE0_HISTORICAL_LIMIT
    };

    /*
     * Static source facts alone are NOT ROM measurements. This evidence is
     * intentionally incomplete and must always fail closed on real boot.
     */
    static const rb_5dc_boot_evidence unobserved_historical_claim = {
        .model = RB_5DC_BOOT_MODEL,
        .firmware = RB_5DC_BOOT_FIRMWARE_111,
        .historical_load_address = RB_5DC_HISTORIC_LOADER_START,
        .historical_relocated_address = RB_5DC_HISTORIC_ML_RELOCATED,
        .rom_observed = 0
    };

    /* Validate freestanding BSS initialization at link time, not on Canon. */
    while (current < end) *current++ = 0;

    rb_registry_init(&rb_stage0_registry);
    rb_ui_init(&rb_stage0_ui);
    rb_stage0_last_result = (uint32_t)rb_stage0_check(&historical_fixture);
    rb_stage0_boot_preflight_result = (uint32_t)rb_5dc_boot_preflight(
        &unobserved_historical_claim);
    /* Must remain RB_5DC_BOOT_NO_ROM_OBSERVATION; no physical handoff. */

    /* Never invoke firmware_entry, Magic Lantern or any camera service. */
}
