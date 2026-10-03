import re
def rd(p): return open(p, encoding="utf-8", newline="").read()
def wr(p, t): open(p, "w", encoding="utf-8", newline="\n").write(t)
g = rd("include/gpu.h")
def cut(text, start, end_incl):
    i = text.index(start); j = text.index(end_incl, i) + len(end_incl)
    return text[:i], text[i:j], text[j:]
# blocks to move (verbatim, with their leading comments)
otag_start = "/* PS1 ordering-table / primitive tag word"
pre, otag, g = cut(g, otag_start, "} OTag;\n")
g = pre + g
pre, gprintf, g = cut(g, "/* Named globals */\nextern void (*GPU_printf)();\n", "extern void (*GPU_printf)();\n")
g = pre + g
pre, devtab, g = cut(g, "/* PsyQ libgpu device table", "extern GpuDevTable *g_gpu_dev_table;\n")
g = pre + g
pre, envtypes, g = cut(g, "/* PsyQ LIBGPU.H environment types (public header layouts). */", "} DISPENV; /* 0x14 */\n")
g = pre + g
pre, ctx, g = cut(g, "/* libgpu SYS state block:", "extern GpuCtx g_gpu_ctx;\n")
g = pre + g
pre, que, g = cut(g, "/* PsyQ libgpu packet queue", "extern volatile GpuQueueItem _que[64];\n")
g = pre + g
pre, drenv, g = cut(g, "/* PsyQ libgpu sys.c DR_ENV packet buffer", "extern GpuDrEnv D_800F1858;\n")
g = pre + g
pre, drmove, g = cut(g, "/* PsyQ DR_MOVE: DMA tag word, then five GPU command words. */", "typedef struct { u32 tag; u32 code[5]; } DR_MOVE;\n")
g = pre + g
pre, sdm, g = cut(g, "extern void SetDrawMove(DR_MOVE *, RECT *, u32, u32);\n", "extern void SetDrawMove(DR_MOVE *, RECT *, u32, u32);\n")
g = pre + g
g = g.replace('#include "common.h"\n', '#include "common.h"\n#include <psxsdk/libgpu.h>\n', 1)
g = re.sub(r"\n{3,}", "\n\n", g)
wr("include/gpu.h", g)
open("tmp/s5/gpu_blocks.txt", "w", encoding="utf-8", newline="\n").write(
    "\n=====OTAG\n" + otag + "\n=====DEVTAB\n" + devtab + "\n=====ENV\n" + envtypes + "\n=====CTX\n" + ctx +
    "\n=====QUE\n" + que + "\n=====DRENV\n" + drenv + "\n=====DRMOVE\n" + drmove)
hdr = """#ifndef PSXSDK_LIBGPU_H
#define PSXSDK_LIBGPU_H

/* PsyQ LIBGPU types and entry points (Sony's libgpu.h; SOTN include/psxsdk/libgpu.h), spelled as
 * BB2's code uses them (the module definitions in src/main/psxsdk/libgpu/). */

#include "common.h"

""" + otag + "\n" + envtypes + "\n" + drmove + """
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
    u8 r3, g3, b3, pad3;
    s16 x3, y3;
} POLY_G4;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad1;
} POLY_FT3;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad1;
    s16 x3, y3;
    u8 u3, v3;
    u16 pad2;
} POLY_FT4;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    u8 r1, g1, b1, p1;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    u8 r2, g2, b2, p2;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad2;
} POLY_GT3;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    u8 r1, g1, b1, p1;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    u8 r2, g2, b2, p2;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad2;
    u8 r3, g3, b3, p3;
    s16 x3, y3;
    u8 u3, v3;
    u16 pad3;
} POLY_GT4;

/* LIBGPU's debug-print hook (Sony's libgpu.h declares it). */
extern void (*GPU_printf)();

extern void SetDispMask(s32);
extern void DrawSync(s32);
extern void DrawOTag(u32 *);
extern DRAWENV *PutDrawEnv(DRAWENV *);
extern u32 GetTPage(s32, s32, s32, s32);
extern void SetPolyF4(u8 *);
""" + sdm + """
#endif /* PSXSDK_LIBGPU_H */
"""
hdr = re.sub(r"\n{3,}", "\n\n", hdr)
wr("include/psxsdk/libgpu.h", hdr)
# libgpu/sys.c-only internals go into sys.c after its includes
s = rd("src/main/psxsdk/libgpu/sys.c")
block = "\n" + devtab + "\n" + ctx + "\n" + que + "\n" + drenv
anchor = '#include "psx.h"\n'
assert s.count(anchor) == 1
s = s.replace(anchor, anchor + block, 1)
s = s.replace('#include "gpu.h"\n', '#include <psxsdk/libgpu.h>\n', 1)
wr("src/main/psxsdk/libgpu/sys.c", s)
p = rd("src/main/psxsdk/libgpu/prim.c")
p = p.replace('#include "gpu.h"\n', '#include <psxsdk/libgpu.h>\n', 1)
wr("src/main/psxsdk/libgpu/prim.c", p)
print(g)
