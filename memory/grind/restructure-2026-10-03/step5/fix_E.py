import sys
def sub(p, old, new):
    t = open(p, encoding="utf-8", newline="").read()
    assert t.count(old) == 1, (p, old)
    open(p, "w", encoding="utf-8", newline="\n").write(t.replace(old, new))
part = sys.argv[1]
if part == "E1":
    sub("include/psxsdk/libgte.h",
        "/* PsyQ LIBGTE types and entry points (Sony's libgte.h; SOTN include/psxsdk/libgte.h), spelled as\n * BB2's code uses them (src/main/psxsdk/libgte/). The cop2 instruction macros are in\n * include/gte.h. */",
        "/* PsyQ LIBGTE types and entry points (Sony's libgte.h; SOTN include/psxsdk/libgte.h), spelled as\n * BB2's code already spells them: the C definition in src/main/psxsdk/libgte/ where there is one\n * (SetBackColor, SetFarColor), otherwise the callers' declarations (most LIBGTE modules are\n * hand-written asm). The cop2 instruction macros are in include/gte.h. */")
    t = "src/main/17AFC.c"
    s = open(t, encoding="utf-8", newline="").read()
    assert s.count("include/gte.h:88-89") == 2
    open(t, "w", encoding="utf-8", newline="\n").write(s.replace("include/gte.h:88-89", "include/gte.h:82-83"))
else:
    sub("include/psxsdk/libgpu.h",
        "/* PsyQ LIBGPU types and entry points (Sony's libgpu.h; SOTN include/psxsdk/libgpu.h), spelled as\n * BB2's code uses them (the module definitions in src/main/psxsdk/libgpu/). */",
        "/* PsyQ LIBGPU types and entry points (Sony's libgpu.h; SOTN include/psxsdk/libgpu.h), spelled as\n * BB2's code already spells them: the types as the game and library code declared them, the\n * prototypes as the C definitions in src/main/psxsdk/libgpu/ and every caller's declaration\n * agree. */")
    sub("include/psxsdk/libgpu.h",
        "/* PsyQ LIBGPU.H POLY_FT3 (0x20 bytes) -- the only libgpu primitive with a\n * u16 clut at +0xE, u16 tpage at +0x16 and v0/v1/v2 at +0xD/+0x15/+0x1D;\n * the caller (func_80043454) walks a 0x20-stride primitive array. */",
        "/* PsyQ LIBGPU.H POLY_FT3 (0x20 bytes), a flat textured triangle: u16 clut at +0xE, u16 tpage\n * at +0x16, v0/v1/v2 at +0xD/+0x15/+0x1D. */")
    sub("include/psxsdk/libgpu.h",
        "/* PsyQ LIBGPU.H POLY_FT4 (0x28 bytes) -- u16 clut at +0xE, u16 tpage at\n * +0x16, v0/v1/v2/v3 at +0xD/+0x15/+0x1D/+0x25; the 0x28-stride quad\n * sibling of func_80043BD0 (POLY_FT3). */",
        "/* PsyQ LIBGPU.H POLY_FT4 (0x28 bytes), a flat textured quad: u16 clut at +0xE, u16 tpage at\n * +0x16, v0/v1/v2/v3 at +0xD/+0x15/+0x1D/+0x25. */")
    sub("include/psxsdk/libgpu.h",
        "/* PsyQ LIBGPU.H POLY_GT3 (0x28 bytes) -- u16 clut at +0xE, u16 tpage at\n * +0x1A, v0/v1/v2 at +0xD/+0x19/+0x25; the caller (func_80043454) walks a\n * 0x28-stride primitive array (asm/funcs/func_80043454.s:182,198). */",
        "/* PsyQ LIBGPU.H POLY_GT3 (0x28 bytes), a gouraud textured triangle: u16 clut at +0xE, u16 tpage\n * at +0x1A, v0/v1/v2 at +0xD/+0x19/+0x25. */")
    sub("include/psxsdk/libgpu.h",
        "/* PsyQ LIBGPU.H POLY_GT4 (0x34 bytes) -- u16 clut at +0xE, u16 tpage at\n * +0x1A, v0/v1/v2/v3 at +0xD/+0x19/+0x25/+0x31; the 0x34-stride quad\n * sibling of func_80043D34 (POLY_GT3). */",
        "/* PsyQ LIBGPU.H POLY_GT4 (0x34 bytes), a gouraud textured quad: u16 clut at +0xE, u16 tpage at\n * +0x1A, v0/v1/v2/v3 at +0xD/+0x19/+0x25/+0x31. */")
    sub("include/psxsdk/libgpu.h",
        "/* LIBGPU's debug-print hook (Sony's libgpu.h declares it). */",
        "/* LIBGPU's debug-print hook (a .data word; the LIBGPU modules call through it). */")
    sub("src/main/51268.c", "/* PsyQ LIBGPU.H POLY_FT4 (0x28 bytes), same layout as text1a_c.c's. */\n", "")
    sub("src/main/psxsdk/libgpu/sys.c",
        "/* GpuQueueItem and `extern volatile GpuQueueItem _que[64];` are declared in include/gpu.h. */",
        "/* GpuQueueItem and `extern volatile GpuQueueItem _que[64];` are declared at the top of this file. */")
    sub("include/psxsdk/libc.h",
        "/* PsyQ C library entry points (Sony's libc.h / ctype.h / setjmp.h; SOTN include/psxsdk/libc.h),\n * spelled as BB2's code uses them (the LIBC2 module definitions in src/main/psxsdk/libc2/). */",
        "/* PsyQ C library entry points (Sony's libc.h / ctype.h / setjmp.h; SOTN include/psxsdk/libc.h),\n * spelled as BB2's code already spells them: the LIBC2 C definitions in src/main/psxsdk/libc2/,\n * and for setjmp (an asm module) its caller's declaration. */")
