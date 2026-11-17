/* SPDX-License-Identifier: GPL-2.0-only */
/* Protocolo do loader Soup 5DC (5DC.111): FILM, +4 (boot) e +0x44 (GUI).
 * Nao e um AUTOEXEC.BIN nem um bootstrap independente do Magic Lantern. */
.syntax unified
.arm
.section .header, "ax", %progbits
.global rb_payload_header
rb_payload_header:
    .word 0x4D4C4946
    b rb_boot
    .space 0x3C, 0
    b rb_key
    .space 0xB8, 0
