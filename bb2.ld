SECTIONS
{
    __romPos = 0;
    _gp = 0x800A30CC;
    header_ROM_START = __romPos;
    header_VRAM = ADDR(.header);
    .header : AT(header_ROM_START) SUBALIGN(2)
    {
        FILL(0x00000000);
        header_DATA_START = .;
        build/asm/header.o(.data);
        header_DATA_END = .;
        header_DATA_SIZE = ABSOLUTE(header_DATA_END - header_DATA_START);
    }
    __romPos += SIZEOF(.header);
    header_ROM_END = __romPos;
    header_VRAM_END = .;

    main_ROM_START = __romPos;
    main_VRAM = ADDR(.main);
    .main 0x80010000 : AT(main_ROM_START) SUBALIGN(2)
    {
        FILL(0x00000000);
        main_RODATA_START = .;
        build/asm/data/800.rodata_pre.o(.rodata);
        build/src/code6cac.o(.rodata);
        build/asm/data/800.rodata_post.o(.rodata);
        build/src/code6cac_b.o(.rodata);
        build/asm/data/101C.rodata_pre.o(.rodata);
        build/src/code6cac_c.o(.rodata);
        build/asm/data/101C.rodata_post.o(.rodata);
        build/src/code6cac_c2.o(.rodata);
        build/asm/data/101C.rodata_c2_post.o(.rodata);
        build/src/text1a.o(.rodata);
        build/asm/data/101C.rodata_text1a_a.o(.rodata);
        build/src/text1a_b.o(.rodata);
        build/asm/data/101C.rodata_text1a_b.o(.rodata);
        build/src/main.o(.rodata);
        build/asm/data/101C.rodata_main_post.o(.rodata);
        build/src/code6cac_b2.o(.rodata);
        build/src/config.o(.rodata);
        build/src/sound.o(.rodata);
        build/src/text1b.o(.rodata);
        build/src/gpu.o(.rodata);
        build/src/display.o(.rodata);
        build/src/system.o(.rodata);
        build/src/ings2.o(.rodata);
        main_RODATA_END = .;
        main_RODATA_SIZE = ABSOLUTE(main_RODATA_END - main_RODATA_START);
        main_TEXT_START = .;
        build/src/ings.o(.text);
        build/src/code6cac.o(.text);
        build/src/code6cac_b.o(.text);
        build/src/code6cac_b2.o(.text);
        build/src/code6cac_c.o(.text);
        build/src/code6cac_c2.o(.text);
        build/src/config.o(.text);
        build/src/text1a.o(.text);
        build/src/text1a_b.o(.text);
        build/src/sound.o(.text);
        build/src/text1b.o(.text);
        build/src/gpu.o(.text);
        build/src/display.o(.text);
        build/src/system.o(.text);
        build/src/ings2.o(.text);
        build/src/main.o(.text);
        main_TEXT_END = .;
        main_TEXT_SIZE = ABSOLUTE(main_TEXT_END - main_TEXT_START);
        main_DATA_START = .;
        build/asm/data/800.rodata_pre.o(.data);
        build/asm/data/800.rodata_post.o(.data);
        build/src/code6cac.o(.data);
        build/src/code6cac_b.o(.data);
        build/src/code6cac_b2.o(.data);
        build/src/code6cac_c.o(.data);
        build/src/code6cac_c2.o(.data);
        build/src/config.o(.data);
        build/src/text1a.o(.data);
        build/src/text1a_b.o(.data);
        build/src/sound.o(.data);
        build/src/text1b.o(.data);
        build/src/gpu.o(.data);
        build/src/display.o(.data);
        build/src/system.o(.data);
        build/src/ings2.o(.data);
        build/src/main.o(.data);
        build/asm/data/7D920.data.o(.data);
        main_DATA_END = .;
        main_DATA_SIZE = ABSOLUTE(main_DATA_END - main_DATA_START);
        main_BSS_START = .;
        build/asm/data/800.rodata_pre.o(.bss);
        build/asm/data/800.rodata_post.o(.bss);
        build/src/code6cac.o(.bss);
        build/src/code6cac_b.o(.bss);
        build/src/code6cac_b2.o(.bss);
        build/src/code6cac_c.o(.bss);
        build/src/code6cac_c2.o(.bss);
        build/src/config.o(.bss);
        build/src/text1a.o(.bss);
        build/src/text1a_b.o(.bss);
        build/src/sound.o(.bss);
        build/src/text1b.o(.bss);
        build/src/gpu.o(.bss);
        build/src/display.o(.bss);
        build/src/system.o(.bss);
        build/src/ings2.o(.bss);
        build/src/main.o(.bss);
        main_BSS_END = .;
        main_BSS_SIZE = ABSOLUTE(main_BSS_END - main_BSS_START);
    }
    __romPos += SIZEOF(.main);
    __romPos = ALIGN(__romPos, 16);
    . = ALIGN(., 16);
    main_ROM_END = __romPos;
    main_VRAM_END = .;

    /DISCARD/ :
    {
        *(*);
    }
}
