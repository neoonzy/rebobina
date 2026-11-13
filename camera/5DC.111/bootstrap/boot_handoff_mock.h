/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * HOST SIMULATION ONLY: fake Rebobina work -> fake Canon continuation.
 * Not a hardware ABI, not a Canon firmware handoff, never link into stage0.
 */
#ifndef RB_5DC_HANDOFF_MOCK_H
#define RB_5DC_HANDOFF_MOCK_H
#include "boot_chain.h"

typedef int (*rb_5dc_mock_callback)(void *ctx);

typedef struct rb_5dc_handoff_mock_result {
    rb_5dc_boot_status preflight;
    unsigned mock_module_attempts;
    unsigned mock_canon_continuations;
    int mock_module_exit;
    int mock_canon_exit;
} rb_5dc_handoff_mock_result;

/* Returns 0 if the simulated sequence ran; -1 if callbacks/report absent.
 * A nonzero module result NEVER prevents the mock Canon continuation.
 * This cannot establish actual fallback on the camera.
 */
int rb_5dc_handoff_mock_run(const rb_5dc_boot_evidence *evidence,
                           rb_5dc_mock_callback module,
                           rb_5dc_mock_callback canon,
                           void *context,
                           rb_5dc_handoff_mock_result *report);
#endif
