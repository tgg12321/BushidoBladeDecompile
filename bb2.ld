/*
 * HAND-MAINTAINED — DO NOT regenerate via `make setup` / splat.
 *
 * The rodata-cleanup project (2026-06-09) re-attributed every
 * asm/data/*.rodata*.s block's bytes to C source files. Regenerating this
 * script via splat would re-add `build/asm/data/*.rodata*.o(.rodata)` lines
 * that no longer have backing .s files (link error) AND would conflict with
 * the const declarations now living in src/*.c (multiple-definition).
 *
 * See the long warning block at the top of splat.yaml for recovery
 * procedure if splat re-runs accidentally.
 *
 * Verification: `grep -c 'asm/data/.*\.rodata.*\.o(\.rodata)' bb2.ld`
 * should be 0.
 */
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
        build/src/ings_strings.o(.rodata);
        build/src/main/87A0.o(.rodata);
        build/src/main/9F9C.o(.rodata);
        build/src/main/175A4.o(.rodata);
        build/src/main/17AFC.o(.rodata);
        build/src/main/24BF0.o(.rodata);
        build/src/code6cac_b_rodata_pre.o(.rodata);
        build/src/main/24F08.o(.rodata);
        build/src/main/25788.o(.rodata);
        build/src/main/25C38.o(.rodata);
        build/src/main/26730.o(.rodata);
        build/src/code6cac_b4_post.o(.rodata);
        build/src/code6cac_b5.o(.rodata);
        build/src/code6cac_b5_post.o(.rodata);
        build/src/code6cac_b_rodata_post.o(.rodata);
        build/src/code6cac_c.o(.rodata);
        build/src/code6cac_c0.o(.rodata);
        build/src/code6cac_c_mid.o(.rodata);
        build/src/code6cac_c2.o(.rodata);
        build/src/text1a_pre.o(.rodata);
        build/src/text1a_pre_tu2.o(.rodata);
        build/src/text1a_svc.o(.rodata);
        build/src/text1a_post.o(.rodata);
        build/src/text1a_c.o(.rodata);
        build/src/text1a_c_tu2.o(.rodata);
        build/src/text1a_filepaths.o(.rodata);
        build/src/text1b.o(.rodata);
        build/src/text1a_b_pre_rodata.o(.rodata);
        build/src/text1b_tu1b.o(.rodata);
        build/src/text1a_b_pre_rodata_b.o(.rodata);
        build/src/text1b_tu1c.o(.rodata);
        build/src/text1b_tu1d.o(.rodata);
        build/src/text1b_tu1e.o(.rodata);
        build/src/text1b_b.o(.rodata);
        build/src/text1a_b_mid_rodata.o(.rodata);
        build/src/text1b_b_tu2.o(.rodata);
        build/src/main/psxsdk/libc2/sprintf.o(.rodata);
        build/src/text1a_b_post_rodata.o(.rodata);
        build/src/gpu.o(.rodata);
        build/src/display.o(.rodata);
        build/src/system.o(.rodata);
        build/src/text1a_b_tail_rodata.o(.rodata);
        build/src/ings2.o(.rodata);
        build/src/main.o(.rodata);
        build/src/main/psxsdk/libcomb/comb.o(.rodata);
        main_RODATA_END = .;
        main_RODATA_SIZE = ABSOLUTE(main_RODATA_END - main_RODATA_START);
        main_TEXT_START = .;
        build/src/main/6CF8.o(.text);
        build/src/main/87A0.o(.text);
        build/src/main/9F9C.o(.text);
        build/src/main/175A4.o(.text);
        build/src/main/17AFC.o(.text);
        build/src/main/24BF0.o(.text);
        build/src/main/24F08.o(.text);
        build/src/main/25788.o(.text);
        build/src/main/25C38.o(.text);
        build/src/main/26730.o(.text);
        build/src/code6cac_b4_post.o(.text);
        build/src/code6cac_b5.o(.text);
        build/src/code6cac_b5_post.o(.text);
        build/src/code6cac_c.o(.text);
        build/src/code6cac_c0.o(.text);
        build/src/code6cac_c_mid.o(.text);
        build/src/code6cac_c2.o(.text);
        build/src/text1a_pre.o(.text);
        build/src/text1a_pre_tu2.o(.text);
        build/src/text1a_svc.o(.text);
        build/src/text1a_post.o(.text);
        build/src/text1a_c.o(.text);
        build/src/text1a_c_tu2.o(.text);
        build/src/text1b.o(.text);
        build/src/text1b_tu1b.o(.text);
        build/src/text1b_tu1c.o(.text);
        build/src/text1b_tu1d.o(.text);
        build/src/text1b_tu1e.o(.text);
        build/src/text1b_b.o(.text);
        build/src/text1b_b_tu2.o(.text);
        build/src/main/psxsdk/libc2/sprintf.o(.text);
        build/src/gpu.o(.text);
        build/src/display.o(.text);
        build/src/system.o(.text);
        build/src/ings2.o(.text);
        build/src/main.o(.text);
        build/src/main/psxsdk/libcomb/comb.o(.text);
        build/src/main_post.o(.text);
        main_TEXT_END = .;
        main_TEXT_SIZE = ABSOLUTE(main_TEXT_END - main_TEXT_START);
        main_DATA_START = .;
        build/src/main/87A0.o(.data);
        build/src/main/9F9C.o(.data);
        build/src/main/175A4.o(.data);
        build/src/main/17AFC.o(.data);
        build/src/main/24BF0.o(.data);
        build/src/main/24F08.o(.data);
        build/src/main/25788.o(.data);
        build/src/main/25C38.o(.data);
        build/src/main/26730.o(.data);
        build/src/code6cac_b4_post.o(.data);
        build/src/code6cac_b5.o(.data);
        build/src/code6cac_b5_post.o(.data);
        build/src/code6cac_c.o(.data);
        build/src/code6cac_c0.o(.data);
        build/src/code6cac_c_mid.o(.data);
        build/src/code6cac_c2.o(.data);
        build/src/text1a_pre.o(.data);
        build/src/text1a_pre_tu2.o(.data);
        build/src/text1a_svc.o(.data);
        build/src/text1a_post.o(.data);
        build/src/text1a_c.o(.data);
        build/src/text1a_c_tu2.o(.data);
        build/src/text1b.o(.data);
        build/src/text1b_tu1b.o(.data);
        build/src/text1b_tu1c.o(.data);
        build/src/text1b_tu1d.o(.data);
        build/src/text1b_tu1e.o(.data);
        build/src/text1b_b.o(.data);
        build/src/text1b_b_tu2.o(.data);
        build/src/main/psxsdk/libc2/sprintf.o(.data);
        build/src/gpu.o(.data);
        build/src/display.o(.data);
        build/asm/data/7D920.data.o(.data);
        /* libcd bios.c: Intr (static volatile CD_intr, defined in src/system.c) at 0x800A1494 */
        build/src/system.o(.data);
        build/src/ings2.o(.data);
        build/src/main.o(.data);
        build/src/main/psxsdk/libcomb/comb.o(.data);
        build/src/main_post.o(.data);
        build/asm/data/91C98.data.o(.data);
        build/src/main/6CF8.o(.sdata);
        build/asm/data/938EA.data.o(.data);
        build/src/main/9F9C.o(.sdata);
        build/asm/data/938FE.data.o(.data);
        build/src/main/17AFC.o(.sdata);
        build/asm/data/93950.data.o(.data);
        build/src/main/24F08.o(.sdata);
        build/asm/data/93978.data.o(.data);
        build/src/main/25C38.o(.sdata);
        build/asm/data/939DC.data.o(.data);
        build/src/code6cac_b4_post.o(.sdata);
        build/asm/data/939E6.data.o(.data);
        build/src/code6cac_c0.o(.sdata);
        build/src/code6cac_c_mid.o(.sdata);
        build/asm/data/93A0E.data.o(.data);
        build/src/code6cac_c2.o(.sdata);
        build/src/text1a_pre.o(.sdata);
        build/src/text1a_post.o(.sdata);
        build/asm/data/93A3E.data.o(.data);
        build/src/text1a_c_tu2.o(.sdata);
        build/src/text1b.o(.sdata);
        build/src/text1b_tu1b.o(.sdata);
        build/src/text1b_tu1c.o(.sdata);
        build/asm/data/93AC0.data.o(.data);
        build/src/text1b_tu1d.o(.sdata);
        build/asm/data/93AEA.data.o(.data);
        build/src/text1b_b.o(.sdata);
        build/asm/data/93B08.data.o(.data);
        build/src/main/9F9C.o(.sbss);
        build/asm/data/93B12.data.o(.data);
        build/src/code6cac_c_mid.o(.sbss);
        build/asm/data/93B56.data.o(.data);
        build/src/code6cac_c2.o(.sbss);
        build/src/text1a_pre.o(.sbss);
        build/asm/data/93B7C.data.o(.data);
        build/src/text1a_post.o(.sbss);
        build/asm/data/93B8C.data.o(.data);
        build/src/text1a_c_tu2.o(.sbss);
        build/src/text1b.o(.sbss);
        build/src/text1b_tu1b.o(.sbss);
        build/asm/data/93C1C.data.o(.data);
        build/src/text1b_tu1c.o(.sbss);
        build/asm/data/93D30.data.o(.data);
        build/src/text1b_tu1d.o(.sbss);
        build/asm/data/93DCC.data.o(.data);
        build/src/text1b_b.o(.sbss);
        build/asm/data/93E18.data.o(.data);
        main_DATA_END = .;
        main_DATA_SIZE = ABSOLUTE(main_DATA_END - main_DATA_START);
        main_BSS_START = .;
        build/src/main/87A0.o(.bss);
        build/src/main/9F9C.o(.bss);
        build/src/main/175A4.o(.bss);
        build/src/main/17AFC.o(.bss);
        build/src/main/24BF0.o(.bss);
        build/src/main/24F08.o(.bss);
        build/src/main/25788.o(.bss);
        build/src/main/25C38.o(.bss);
        build/src/main/26730.o(.bss);
        build/src/code6cac_b4_post.o(.bss);
        build/src/code6cac_b5.o(.bss);
        build/src/code6cac_b5_post.o(.bss);
        build/src/code6cac_c.o(.bss);
        build/src/code6cac_c0.o(.bss);
        build/src/code6cac_c_mid.o(.bss);
        build/src/code6cac_c2.o(.bss);
        build/src/text1a_pre.o(.bss);
        build/src/text1a_pre_tu2.o(.bss);
        build/src/text1a_svc.o(.bss);
        build/src/text1a_post.o(.bss);
        build/src/text1a_c.o(.bss);
        build/src/text1a_c_tu2.o(.bss);
        build/src/text1b.o(.bss);
        build/src/text1b_tu1b.o(.bss);
        build/src/text1b_tu1c.o(.bss);
        build/src/text1b_tu1d.o(.bss);
        build/src/text1b_tu1e.o(.bss);
        build/src/text1b_b.o(.bss);
        build/src/text1b_b_tu2.o(.bss);
        build/src/main/psxsdk/libc2/sprintf.o(.bss);
        build/src/gpu.o(.bss);
        build/src/display.o(.bss);
        build/src/system.o(.bss);
        build/src/ings2.o(.bss);
        build/src/main.o(.bss);
        build/src/main/psxsdk/libcomb/comb.o(.bss);
        build/src/main_post.o(.bss);
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
