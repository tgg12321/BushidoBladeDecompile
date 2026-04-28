/* gte.h -- PsyQ-style GTE coprocessor macros for GCC 2.7.2.
 *
 * The PSX Geometry Transformation Engine (GTE / COP2) has no C-level
 * representation, so any function that uses it must wrap the cop2 ops in
 * inline asm. This header is the minimum surface area: macros that emit
 * one cop2 op each, leaving all surrounding control flow / arithmetic in
 * real C.
 *
 * Macro families:
 *   gte_LDxx     -- mtc2 register transfer into the GTE
 *   gte_STxx     -- mfc2 register transfer out of the GTE
 *   gte_LDV?     -- load short-vector from memory (lwc2)
 *   gte_<OP>     -- the cop2 ops themselves (rtps, rtpt, nclip, ...)
 *   gte_SetXxx   -- short helpers to set common control regs
 *
 * Reference: libgte / libpsyq SDK 3.5 inline macros. The opcode/control
 * field layout is defined in include/gte_macros.inc on the asm side.
 */
#ifndef GTE_H
#define GTE_H

#include "common.h"

/* ---- Vector / matrix structs ---------------------------------------- */
typedef struct VECTOR  { s32 vx, vy, vz, pad; } VECTOR;
typedef struct SVECTOR { s16 vx, vy, vz, pad; } SVECTOR;
typedef struct CVECTOR { u8  r,  g,  b,  cd;  } CVECTOR;
typedef struct DVECTOR { s16 vx, vy;          } DVECTOR;
typedef struct MATRIX  { s16 m[3][3]; u16 pad; s32 t[3]; } MATRIX;

/* ---- COP2 vector/matrix register loads (lwc2) ----------------------- */
#define gte_ldv0(p)  __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" :: "r"(p))
#define gte_ldv1(p)  __asm__ volatile ("lwc2 $2, 0(%0)\n\tlwc2 $3, 4(%0)" :: "r"(p))
#define gte_ldv2(p)  __asm__ volatile ("lwc2 $4, 0(%0)\n\tlwc2 $5, 4(%0)" :: "r"(p))
#define gte_ldv3(v0, v1, v2) do { gte_ldv0(v0); gte_ldv1(v1); gte_ldv2(v2); } while (0)

#define gte_ldrgb(p) __asm__ volatile ("lwc2 $6, 0(%0)" :: "r"(p))

/* ---- COP2 result stores (swc2 / mfc2) ------------------------------- */
#define gte_stsxy(p)  __asm__ volatile ("swc2 $14, 0(%0)" :: "r"(p))
#define gte_stsxy3(p0, p1, p2) do { \
    __asm__ volatile ("swc2 $12, 0(%0)" :: "r"(p0)); \
    __asm__ volatile ("swc2 $13, 0(%0)" :: "r"(p1)); \
    __asm__ volatile ("swc2 $14, 0(%0)" :: "r"(p2)); \
} while (0)
#define gte_stsz(p)   __asm__ volatile ("swc2 $19, 0(%0)" :: "r"(p))

#define gte_stmac0(out) __asm__ volatile ("mfc2 %0, $24" : "=r"(out))
#define gte_stmac1(out) __asm__ volatile ("mfc2 %0, $25" : "=r"(out))
#define gte_stmac2(out) __asm__ volatile ("mfc2 %0, $26" : "=r"(out))
#define gte_stmac3(out) __asm__ volatile ("mfc2 %0, $27" : "=r"(out))

/* ---- The cop2 ops themselves ---------------------------------------- *
 * Encodings come from gte_macros.inc / Sony GTE documentation. They emit
 * a single 32-bit cop2 instruction via .word so GNU as accepts them
 * uniformly. Each macro has no operands: the GTE state must already be
 * populated via gte_LDxx. Read results out with gte_STxx.                */

#define gte_rtps()   __asm__ volatile (".word 0x4A180001")  /* perspective xform single   */
#define gte_rtpt()   __asm__ volatile (".word 0x4A280030")  /* perspective xform triple   */
#define gte_nclip()  __asm__ volatile (".word 0x4A400006")  /* normal clip                */
#define gte_avsz3()  __asm__ volatile (".word 0x4A48002D")  /* avg Z over 3 vertices      */
#define gte_avsz4()  __asm__ volatile (".word 0x4A58002E")  /* avg Z over 4 vertices      */
#define gte_dpcs()   __asm__ volatile (".word 0x4A780010")  /* depth cue single           */
#define gte_dpct()   __asm__ volatile (".word 0x4AF8002A")  /* depth cue triple           */
#define gte_intpl()  __asm__ volatile (".word 0x4A980011")  /* interpolation              */
#define gte_sqr()    __asm__ volatile (".word 0x4AA00428")  /* square                     */
#define gte_ncs()    __asm__ volatile (".word 0x4AC8041E")  /* normal color               */
#define gte_nct()    __asm__ volatile (".word 0x4AD80420")  /* normal color triple        */
#define gte_ncds()   __asm__ volatile (".word 0x4AE80413")  /* normal color depth single  */
#define gte_ncdt()   __asm__ volatile (".word 0x4AF80416")  /* normal color depth triple  */
#define gte_nccs()   __asm__ volatile (".word 0x4B08041B")  /* normal color color single  */
#define gte_ncct()   __asm__ volatile (".word 0x4B18043F")  /* normal color color triple  */
#define gte_cdp()    __asm__ volatile (".word 0x4B280414")  /* color depth queue          */
#define gte_cc()     __asm__ volatile (".word 0x4B38041C")  /* color col                  */
#define gte_dpcl()   __asm__ volatile (".word 0x4A680029")  /* depth cue color light      */

#endif /* GTE_H */
