/* SPDX-License-Identifier: GPL-2.0-only */
/* Endereco do payload legado Soup, NAO um endereco de boot Canon. */
ENTRY(rb_payload_header)
MEMORY {
  payload (rx) : ORIGIN = 0x009D0000, LENGTH = 128K
}
SECTIONS {
  .text : {
    KEEP(*(.header))
    *(.text*)
    *(.rodata*)
    . = ALIGN(4);
  } > payload
  .data : {
    *(.data*)
    . = ALIGN(4);
  } > payload
  .bss (NOLOAD) : {
    *(.bss*)
    *(COMMON)
  } > payload
  /DISCARD/ : { *(.ARM.exidx*) *(.comment) *(.note*) *(.ARM.attributes) }
  ASSERT(SIZEOF(.bss) == 0, "Esta prova nao pode usar .bss")
  ASSERT(SIZEOF(.data) == 0, "Esta prova nao pode usar .data")
}
