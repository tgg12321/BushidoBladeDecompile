/* func_8006D3DC — MATCHED (session s1, 2026-09-14): sandbox --disable all = 0,
 * full build SHA1 = 62efab4f73f992798c43e8c730aa43baa10bb4fa (verify-oracle ok:true).
 *
 * Derivation: near-twin of func_8006DD94 (same file) and func_8006BB68 — same
 * 0x2C descriptor handed to func_8007352C, same rsin() pulse colour, same
 * SetDrawMode/AddPrim(D_800A374C + 0x28) tail, same func_80069898(obj, rect, 0x11)
 * epilogue.  Differences from the twin: 6 iterations (not 3), q = *(arg0[1]+0x38)
 * indexed q[i] (not q[i+8]), no func_8006D808 tail call, and NO oversized-locals
 * carve-out is needed here — the plain fully-written locals set
 * (EnvA 0x2C at sp+0x18 .. sp+0x43, 8-aligned u16 rect[4] at sp+0x48) yields
 * vars = 0x38, + 0x18 outgoing args + 0x20 gp-save = frame 0x70 = the target frame.
 *
 * Every construct below materialises in the TARGET bytes — nothing is a coercion
 * carrier, so no /* FAKE */ annotation applies:
 *   - `u8 dim = 0x40;`      -> target 8006D400 `addiu $s2, $zero, 0x40`, read by
 *                              the three `sb $s2, 0x41/0x42/0x43($sp)` at 8006D4D8.
 *                              u8 (not s32) is load-bearing: an s32 holder forces
 *                              a QI copy `move v0,s2` before the sb triple (probe B,
 *                              score 22).  A bare 0x40 literal is never hoisted out
 *                              of the loop by loop.c (the const lands in a QImode
 *                              pseudo, dump text1b.loop insn 170) — probe A, score 27.
 *   - `s.has_color = 1;` in the pre-loop descriptor-init block
 *                           -> target 8006D420 `addiu $v0,$zero,0x1` +
 *                              8006D428 `sb $v0, 0x40($sp)`.  The sibling
 *                              func_8006BB68 initialises the same slot
 *                              (S69E18.byte28, offset 0x28) in its own pre-loop
 *                              init block, so this is the original init shape.
 *                              Dropping it costs 2 insns + a prologue reorder
 *                              (probe C, score 10).
 *   - declaration order `s32 semi = 0; s16 i = 0; u8 dim = 0x40;` with
 *     `for (; i < 6; i++)`  -> fixes the first scheduler pass's tie-break order of
 *                              the three independent `sw $sN / init $sN` pairs to
 *                              s5(semi), s1(i), s2(dim) as in the target prologue.
 *                              `u8 dim` before `semi` = order dim,semi,i (score 10);
 *                              semi,dim,i (score 4); semi,i,dim (score 0).
 */
extern s16 D_800A3528;
extern s32 D_800A374C;
extern s32 func_8006E480(s32, s32);
extern s32 func_8007352C(s32);
/* EnvA: the 0x2C-byte draw descriptor func_8007352C consumes.  Same field
   layout as EnvB (func_8006DD94) and S69E18 (func_80069E18); this call site
   declares only the fields through col_b, which is what the target frame
   (locals 0x18..0x4F = EnvA 0x2C + 8-aligned u16 rect[4]) accounts for. */
typedef struct EnvA {
    s32 *header;
    s8  *table;
    s32  out;
    s32  pad0C;
    s32  semi;
    u32  ot_idx;
    s32  x;
    s32  y;
    s32  pad20, pad24;
    u8   has_color;
    u8   col_r;
    u8   col_g;
    u8   col_b;
} EnvA;
void func_8006D3DC(s32 *arg0) {
    EnvA s;
    u16 rect[4];
    s32 *q;
    s32 c;
    s32 hdr;
    s32 semi = 0;
    s16 i = 0;
    u8 dim = 0x40;

    s.ot_idx = 0xA;
    q = *(s32 **)(arg0[1] + 0x38);
    s.x = 0;
    s.has_color = 1;

    for (; i < 6; i++) {
        s.has_color = 1;
        if (i == 0) {
            s.y = 0;
            s.has_color = 0;
            s.semi = 0;
        } else if (i == D_800A3528 + 1) {
            s.y = *(s16 *)(D_800A34FC + 0xE);
            c = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;
            s.col_r = s.col_g = s.col_b = c;
            s.semi = 0;
        } else if (i != 1 && i != 2 && i != 3 && i == D_800A3528 + 4) {
            s.col_r = s.col_g = s.col_b = 0x80;
            s.y = 0;
            s.semi = 0;
        } else {
            s.col_r = s.col_g = s.col_b = dim;
            s.y = 0;
            s.semi = 1;
        }
        hdr = q[i];
        s.header = (s32 *)hdr;
        s.table = (s8 *)(hdr + 0xC);
        s.out = arg0[5];
        arg0[5] = func_8007352C((s32)&s);
        SetDrawMode(arg0[7], 1, 0, func_8006E480((s32)s.header, semi), 0);
        AddPrim(D_800A374C + 0x28, arg0[7]);
        arg0[7] += 0xC;
    }

    rect[0] = 0xDA;
