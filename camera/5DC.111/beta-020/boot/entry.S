/* SPDX-License-Identifier: GPL-2.0-only */
/* Soup Engine's existing 5DC.111 loader: FILM, entry +4, event hook +0x44.
 * No Canon boot modification; this replaces only SE.BIN on a TEST CF.
 */
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
