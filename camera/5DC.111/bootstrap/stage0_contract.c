/* SPDX-License-Identifier: GPL-2.0-only */
/* New Rebobina code informed by Magic Lantern Team 5DC.111 research. */
#include "stage0_contract.h"

rb_stage0_result rb_stage0_check(const rb_stage0_context *context)
{
    if (!context) return RB_STAGE0_NO_CONTEXT;
    if (context->magic != RB_STAGE0_MAGIC) return RB_STAGE0_BAD_MAGIC;
    if (context->platform != RB_STAGE0_PLATFORM_5DC)
        return RB_STAGE0_BAD_PLATFORM;
    if (context->firmware != RB_STAGE0_FIRMWARE_111)
        return RB_STAGE0_BAD_FIRMWARE;
    if (context->historical_start != RB_STAGE0_HISTORICAL_START ||
        context->historical_limit != RB_STAGE0_HISTORICAL_LIMIT ||
        context->historical_start >= context->historical_limit)
        return RB_STAGE0_BAD_LAYOUT;
    return RB_STAGE0_RESEARCH_MATCH_ONLY;
}
