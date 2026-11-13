/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Rebobina independent ARM946E-S entry *for offline ELF linking only*.
 * Inspired by Magic Lantern Team historical 5DC.111 startup research:
 * https://github.com/frantony/magic-lantern/tree/a3454ebed3f6adcd165ee9382f451f917d989253/platform/5DC.111
 * This code is NEW (2026-11-13), not copied from local_entry.S.
 *
 * No Canon memory writes, ROM calls, loader signature or boot flag changes.
 * This entry has NO firmware handoff. If invoked on real hardware it would
 * park indefinitely. NEVER CONVERT OR INSTALL THIS ELF ON THE CAMERA.
 */
.syntax unified
.cpu arm946e-s
.arm
.section .text.rb_stage0_entry,"ax",%progbits
.align 2
.global rb_stage0_entry
.type rb_stage0_entry,%function
rb_stage0_entry:
    ldr sp, =__rb_stage0_stack_top
    bl rb_stage0_prepare
.Loffline_stop:
    b .Loffline_stop
.size rb_stage0_entry, .-rb_stage0_entry
