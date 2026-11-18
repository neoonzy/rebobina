/* SPDX-License-Identifier: GPL-2.0-only */
/* Soup's payload load region, NOT the Canon firmware boot address. */
ENTRY(rb_payload_header)
MEMORY { payload (rwx) : ORIGIN = 0x009D0000, LENGTH = 128K }
SECTIONS {
  .text : { KEEP(*(.header)) *(.text*) *(.rodata*) . = ALIGN(4); } > payload
  .data : { *(.data*) . = ALIGN(4); } > payload
  .bss (NOLOAD) : { *(.bss*) *(COMMON) } > payload
  /DISCARD/ : { *(.ARM.exidx*) *(.comment) *(.note*) *(.ARM.attributes) }
  ASSERT(SIZEOF(.bss) == 0, "No .bss: the legacy loader does not initialize it")
  ASSERT(__payload_end <= ORIGIN(payload) + LENGTH(payload), "Payload too large")
  __payload_end = .;
}
