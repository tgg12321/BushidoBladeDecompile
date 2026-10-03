def rd(p): return open(p, encoding="utf-8", newline="").read()
def wr(p, t): open(p, "w", encoding="utf-8", newline="\n").write(t)
g = rd("include/gte.h")
old = """/* ---- Vector / matrix structs ---------------------------------------- */
typedef struct VECTOR  { s32 vx, vy, vz, pad; } VECTOR;
typedef struct SVECTOR { s16 vx, vy, vz, pad; } SVECTOR;
typedef struct CVECTOR { u8  r,  g,  b,  cd;  } CVECTOR;
typedef struct DVECTOR { s16 vx, vy;          } DVECTOR;
typedef struct MATRIX  { s16 m[3][3]; u16 pad; s32 t[3]; } MATRIX;

"""
assert old in g
g = g.replace(old, "")
g = g.replace('#include "common.h"\n', '#include "common.h"\n#include <psxsdk/libgte.h>\n', 1)
wr("include/gte.h", g)
hdr = """#ifndef PSXSDK_LIBGTE_H
#define PSXSDK_LIBGTE_H

/* PsyQ LIBGTE types and entry points (Sony's libgte.h; SOTN include/psxsdk/libgte.h), spelled as
 * BB2's code uses them (src/main/psxsdk/libgte/). The cop2 instruction macros are in
 * include/gte.h. */

#include "common.h"

typedef struct VECTOR  { s32 vx, vy, vz, pad; } VECTOR;
typedef struct SVECTOR { s16 vx, vy, vz, pad; } SVECTOR;
typedef struct CVECTOR { u8  r,  g,  b,  cd;  } CVECTOR;
typedef struct DVECTOR { s16 vx, vy;          } DVECTOR;
typedef struct MATRIX  { s16 m[3][3]; u16 pad; s32 t[3]; } MATRIX;

extern void InitGeom(void);
extern void SetGeomOffset(s32, s32);
extern void SetGeomScreen(s32);
extern s32 ReadGeomScreen(void);
extern void SetBackColor(s32, s32, s32);
extern void SetFarColor(s32, s32, s32);
extern void ReadSZfifo3(s32 *, s32 *, s32 *);
extern s32 SquareRoot0(s32);
extern s32 SquareRoot12(s32);
extern s32 Square12(s32 *, s32 *);
extern void LoadAverage12(s32 *, s32 *, s32, s32, s32);
extern void *RotMatrix(s16 *, u8 *);
extern void RotMatrixZYX(s16 *, u8 *);
extern void MulMatrix(s32 *, s32 *);
extern void CompMatrix(s32, u8 *, u8 *);
extern void ScaleMatrix(u8 *, s32 *);
extern void ScaleMatrixL(u8 *, u8 *);
extern void ApplyMatrixLV(void *, void *, void *);
extern s32 RotTransPers(SVECTOR *, s32 *, s32 *, s32 *);
extern void RotTransPers3(SVECTOR *, SVECTOR *, SVECTOR *, s32 *, s32 *, s32 *, s32 *, s32 *);
extern s32 RotTransPers4(s16 *, s16 *, s16 *, s16 *, s32 *, s32 *, s32 *, s32 *, s32 *, s32);

#endif /* PSXSDK_LIBGTE_H */
"""
wr("include/psxsdk/libgte.h", hdr)
