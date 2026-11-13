/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * OFFLINE LINK LAYOUT - NEVER INSTALL OR CONVERT TO AUTOEXEC.BIN.
 * 0x00980000..0x00A00000 is a historical Magic Lantern research claim
 * for Canon 5DC.111, NOT a verified free memory region for Rebobina.
 * Source: ML Team platform/5DC.111/Makefile.platform.default, 2013.
 */
ENTRY(rb_stage0_entry)

SECTIONS
{
    . = 0x00980000;
    __rb_stage0_image_start = .;

    .text : {
        KEEP(*(.text.rb_stage0_entry))
        *(.text .text.*)
    }
    . = ALIGN(4);

    .rodata : {
        KEEP(*(.rodata.rb_offline_marker))
        *(.rodata .rodata.*)
    }
    . = ALIGN(4);

    .data : { *(.data .data.*) }
    . = ALIGN(8);

    .bss (NOLOAD) : {
        __rb_stage0_bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __rb_stage0_bss_end = .;
    }
    . = ALIGN(8);

    __rb_stage0_stack_start = .;
    . += 0x1000;
    __rb_stage0_stack_top = .;
    __rb_stage0_image_end = .;

    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.ARM.attributes)
        *(.ARM.exidx*)
        *(.eh_frame*)
    }
}
ASSERT(__rb_stage0_image_end <= 0x00A00000, "OFFLINE stage0 exceeded historical 512 KiB hypothesis");
