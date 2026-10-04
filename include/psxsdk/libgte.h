#ifndef PSXSDK_LIBGTE_H
#define PSXSDK_LIBGTE_H

/* PsyQ LIBGTE types and entry points (Sony's libgte.h; SOTN include/psxsdk/libgte.h), spelled as
 * BB2's code already spells them: the C definition in src/main/psxsdk/libgte/ where there is one
 * (SetBackColor, SetFarColor), otherwise the callers' declarations (most LIBGTE modules are
 * hand-written asm). The cop2 instruction macros are in include/gte.h. */

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
extern s32 ratan2(s32, s32);
extern s32 Square12(s32 *, s32 *);
extern void LoadAverage12(s32 *, s32 *, s32, s32, s32);
extern void *RotMatrix(s16 *, u8 *);
extern void RotMatrixZYX(s16 *, u8 *);
extern MATRIX *RotMatrixX(s32, MATRIX *);
extern MATRIX *RotMatrixY(s32, MATRIX *);
extern MATRIX *RotMatrixZ(s32, MATRIX *);
extern MATRIX *MulMatrix0(MATRIX *, MATRIX *, MATRIX *);
extern void MulMatrix(s32 *, s32 *);
extern MATRIX *CompMatrix(MATRIX *, MATRIX *, MATRIX *);
extern void ScaleMatrix(u8 *, s32 *);
extern MATRIX *ScaleMatrixL(MATRIX *, VECTOR *);
extern void ApplyMatrixLV(void *, void *, void *);
extern VECTOR *ApplyRotMatrix(SVECTOR *, VECTOR *);
extern VECTOR *ApplyRotMatrixLV(VECTOR *, VECTOR *);
extern void SetRotMatrix(MATRIX *);
extern void SetTransMatrix(MATRIX *);
extern void RotTrans(SVECTOR *, VECTOR *, s32 *);
extern s32 RotTransPers(SVECTOR *, s32 *, s32 *, s32 *);
extern void RotTransPers3(SVECTOR *, SVECTOR *, SVECTOR *, s32 *, s32 *, s32 *, s32 *, s32 *);
extern s32 RotTransPers4(s16 *, s16 *, s16 *, s16 *, s32 *, s32 *, s32 *, s32 *, s32 *, s32);

#endif /* PSXSDK_LIBGTE_H */
