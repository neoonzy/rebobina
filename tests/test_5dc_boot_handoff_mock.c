/* SPDX-License-Identifier: GPL-2.0-only */
#include "boot_handoff_mock.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

typedef struct mock_ctx {
    unsigned module_calls;
    unsigned canon_calls;
    int module_exit;
    int canon_exit;
} mock_ctx;
static int mock_module(void *ptr)
{
    mock_ctx *ctx = ptr;
    ctx->module_calls++;
    return ctx->module_exit;
}
static int mock_canon(void *ptr)
{
    mock_ctx *ctx = ptr;
    ctx->canon_calls++;
    return ctx->canon_exit;
}
static rb_5dc_boot_evidence full_fixture(void)
{
    rb_5dc_boot_evidence e = {0};
    e.model = RB_5DC_BOOT_MODEL;
    e.firmware = RB_5DC_BOOT_FIRMWARE_111;
    e.historical_load_address = RB_5DC_HISTORIC_LOADER_START;
    e.historical_relocated_address = RB_5DC_HISTORIC_ML_RELOCATED;
    e.rom_read_prologue = RB_5DC_HISTORIC_ROM_READ_PROLOGUE;
    e.rom_write_prologue = RB_5DC_HISTORIC_ROM_WRITE_PROLOGUE;
    e.flags[2] = UINT32_MAX;
    e.rom_observed = 1; /* deliberately fake for HOST */
    e.source_identified = 1;
    e.ram_proven_safe = 1;
    e.canon_handoff_proven = 1;
    e.recovery_proven = 1;
    return e;
}
static void try_case(rb_5dc_boot_evidence *e,
                     rb_5dc_boot_status expected, unsigned expected_mod_calls,
                     int failure)
{
    rb_5dc_handoff_mock_result result;
    mock_ctx ctx = {0};
    ctx.module_exit = failure;
    ctx.canon_exit = 7;
    assert(rb_5dc_handoff_mock_run(e, mock_module, mock_canon, &ctx, &result) == 0);
    assert(result.preflight == expected);
    assert(ctx.module_calls == expected_mod_calls);
    assert(result.mock_module_attempts == expected_mod_calls);
    assert(ctx.canon_calls == 1);
    assert(result.mock_canon_continuations == 1);
    assert(result.mock_canon_exit == 7);
    assert(result.mock_module_exit == (expected_mod_calls ? failure : 0));
}
int main(void)
{
    rb_5dc_boot_evidence e = full_fixture();
    rb_5dc_handoff_mock_result unchanged = {0};
    mock_ctx ctx = {0};

    assert(rb_5dc_handoff_mock_run(&e, mock_module, NULL, &ctx, &unchanged) == -1);
    assert(ctx.module_calls == 0 && ctx.canon_calls == 0);
    assert(rb_5dc_handoff_mock_run(&e, mock_module, mock_canon, &ctx, NULL) == -1);
    assert(ctx.module_calls == 0 && ctx.canon_calls == 0);
    try_case(&e, RB_5DC_BOOT_RESEARCH_READY_NOT_AUTHORIZED, 1, 0);
    try_case(&e, RB_5DC_BOOT_RESEARCH_READY_NOT_AUTHORIZED, 1, -123);
    e.rom_observed = 0;
    try_case(&e, RB_5DC_BOOT_NO_ROM_OBSERVATION, 0, 0);
    e = full_fixture();
    e.source_identified = 0;
    try_case(&e, RB_5DC_BOOT_UNVERIFIED_SOURCE, 0, 0);
    e = full_fixture();
    e.ram_proven_safe = 0;
    try_case(&e, RB_5DC_BOOT_UNVERIFIED_MEMORY, 0, 0);
    e = full_fixture();
    e.canon_handoff_proven = 0;
    try_case(&e, RB_5DC_BOOT_UNVERIFIED_HANDOFF, 0, 0);
    e = full_fixture();
    e.recovery_proven = 0;
    try_case(&e, RB_5DC_BOOT_UNVERIFIED_RECOVERY, 0, 0);
    e = full_fixture();
    e.flags[1] = UINT32_MAX;
    try_case(&e, RB_5DC_BOOT_FLAG_ALREADY_ENABLED, 0, 0);
    e = full_fixture();
    e.model = 0x35443232u;
    try_case(&e, RB_5DC_BOOT_WRONG_MODEL, 0, 0);
    try_case(NULL, RB_5DC_BOOT_MISSING_INPUT, 0, 0);
    puts("OK: simulated handoff always reaches Canon MOCK; hardware NOT tested");
    return 0;
}
