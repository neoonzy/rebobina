/* SPDX-License-Identifier: GPL-2.0-only */
/* Pure HOST simulation, no camera memory, ROM calls, or card boot. */
#include "boot_handoff_mock.h"

int rb_5dc_handoff_mock_run(const rb_5dc_boot_evidence *e,
                           rb_5dc_mock_callback module,
                           rb_5dc_mock_callback canon,
                           void *ctx,
                           rb_5dc_handoff_mock_result *report)
{
    rb_5dc_handoff_mock_result result = {0};
    if (!canon || !report) return -1;

    result.preflight = rb_5dc_boot_preflight(e);
    /*
     * Only a synthetic "complete" fixture may attempt simulated work.
     * NOT a physical approval, since no evidence is authenticated here.
     */
    if (result.preflight == RB_5DC_BOOT_RESEARCH_READY_NOT_AUTHORIZED && module) {
        result.mock_module_attempts = 1;
        result.mock_module_exit = module(ctx);
    }

    /* Even blocked/failed simulated work cannot skip the Canon mock. */
    result.mock_canon_continuations = 1;
    result.mock_canon_exit = canon(ctx);
    *report = result;
    return 0;
}
