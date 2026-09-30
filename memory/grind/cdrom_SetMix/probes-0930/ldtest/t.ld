SECTIONS {
  . = 0x80010000;
  .text : { *(.text) }
  .bss : { *(.bss) *(COMMON) }
  /DISCARD/ : { *(*) }
}
foo = 0x800A3718;
