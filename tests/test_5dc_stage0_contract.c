/* SPDX-License-Identifier: GPL-2.0-only */
#include "stage0_contract.h"
#include <assert.h>
#include <stdio.h>

int main(void)
{
    rb_stage0_context ctx = {
        RB_STAGE0_MAGIC, RB_STAGE0_PLATFORM_5DC, RB_STAGE0_FIRMWARE_111,
        RB_STAGE0_HISTORICAL_START, RB_STAGE0_HISTORICAL_LIMIT
    };

    assert(rb_stage0_check(NULL) == RB_STAGE0_NO_CONTEXT);
    assert(rb_stage0_check(&ctx) == RB_STAGE0_RESEARCH_MATCH_ONLY);
    ctx.magic = 0;
    assert(rb_stage0_check(&ctx) == RB_STAGE0_BAD_MAGIC);
    ctx.magic = RB_STAGE0_MAGIC;
    ctx.platform = 0x35443232u; /* not the original 5D */
    assert(rb_stage0_check(&ctx) == RB_STAGE0_BAD_PLATFORM);
    ctx.platform = RB_STAGE0_PLATFORM_5DC;
    ctx.firmware = 0x00010100u;
    assert(rb_stage0_check(&ctx) == RB_STAGE0_BAD_FIRMWARE);
    ctx.firmware = RB_STAGE0_FIRMWARE_111;
    ctx.historical_start = 0x00981000u;
    assert(rb_stage0_check(&ctx) == RB_STAGE0_BAD_LAYOUT);
    ctx.historical_start = RB_STAGE0_HISTORICAL_START;
    ctx.historical_limit = RB_STAGE0_HISTORICAL_START;
    assert(rb_stage0_check(&ctx) == RB_STAGE0_BAD_LAYOUT);

    puts("OK: stage0 structural research contract (OFFLINE, not boot authorization)");
    return 0;
}
