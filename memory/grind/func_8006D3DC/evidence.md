# Evidence bank — func_8006D3DC

## s1 (2026-09-14, recon) — MATCHED, sandbox 0, verify-oracle ok:true

OBJECT MODEL: D_800A374C — MATCHES. The flagged declaration `extern s32 D_800A374C;`
(m2c_context.h, census g_dma_buf_base) is the shape the target wants at this call site.
Target bytes 8006D544-8006D554: `lui $a0,%hi(D_800A374C); lw $a0,%lo(D_800A374C)($a0);
addiu $a0,$a0,0x28; jal AddPrim`. Written in C as `AddPrim(D_800A374C + 0x28, arg0[7])`,
i.e. the symbol's VALUE is an s32 holding an OT base and 0x28 is a byte offset added to
it — exactly the spelling three in-file siblings already ship on main
(src/text1b.c:5840 func_8006BB68 `AddPrim(D_800A374C + 0x28, arg0[7])`,
src/text1b.c:6096 area func_8006DD94, src/text1b.c:5621 `AddPrim(D_800A374C + 0x4C, ...)`).
No pointer-typed or struct-typed re-declaration is needed; measured score 0 with the
declaration exactly as flagged, so the declaration is not a residual source here.

### Function identity
6-entry menu-highlight draw loop. `arg0` is the 8-word GameObj-ish context used by the
whole text1b draw family (arg0[1] = data block, arg0[5] = texture/prim chain head,
arg0[7] = the OT cursor). Per entry it fills a 0x2C-byte descriptor, hands its address to
func_8007352C, sets the draw mode from func_8006E480, and appends to the OT at
D_800A374C + 0x28. Entry 0 is the "no colour" title row; entry D_800A3528+1 is the
selected row and gets the rsin() pulse shade; entry D_800A3528+4 (when not one of rows
1/2/3) gets 0x80; everything else gets 0x40 with semi-transparency on. Epilogue is the
standard `func_80069898(obj, rect{0xDA,0x25,0xCB,1}, 0x11)`.

### Sibling map (this is what made the session cheap)
func_8006DD94 (src/text1b.c:6019+, matched on main) is a near-twin: identical descriptor
type, identical rsin pulse expression, identical SetDrawMode/AddPrim/`arg0[7] += 0xC`
body, identical rect+func_80069898 epilogue. Deltas: 6 iterations vs 3;
`q = *(s32 **)(arg0[1] + 0x38)` indexed `q[i]` vs `+0x3C` indexed `q[i+8]`; no trailing
func_8006D808 call; the arm structure has four arms (0 / selected / +4 / default) instead
of three. func_8006BB68 (src/text1b.c:5818) is the second-order sibling and supplies the
pre-loop descriptor-init idiom.

### Frame arithmetic — NO oversized-locals carve-out is needed here
Target frame 0x70 with $s0-$s5,$ra saved at sp+0x50..0x68 (7 words -> ALIGN8(28) = 0x20)
and a 0x18 outgoing-args area (SetDrawMode's 5th arg stores at sp+0x10), so the locals
region is 0x70 - 0x20 - 0x18 = 0x38 = 56 bytes. The fully-written set is exactly 56:
descriptor 0x2C at sp+0x18..0x43, then u16 rect[4] at sp+0x48..0x4F (the rect is
8-aligned, so sp+0x44..0x47 is natural padding, not a dead pad). This is the opposite of
the twin func_8006DD94, which needed a 0x34 descriptor and carries an OVERSIZED-LOCALS
FAKE: do NOT import that carve-out here, the plain declaration is correct.

### The three spelling facts that closed the last 27 points
1. `0x40` must live in a callee-saved register across the loop (target `addiu $s2,$zero,
   0x40` at 8006D400, read by the `sb $s2` triple at 8006D4D8). A bare literal does not
   get there: it lands in a QImode pseudo inside a conditional arm
   (tmp/grind/func_8006D3DC/dumps/text1b.loop.8006D3DC insn 170,
   `(set (reg:QI 122) (const_int 64))`) and loop.c leaves it in the loop. Contrast the
   constant 1, which is an SImode pseudo (it feeds both a QI `sb has_color` and an SI
   `sw semi`) and IS hoisted by loop.c to the preheader (insn 323, before code_label 37).
2. The holder must be `u8`, not `s32`. With `s32 dim` the value reaches a register but
   the chained byte assignment needs a QImode truncation, emitting `move v0,s5` before
   the sb triple (score 22). With `u8 dim` the sb's read the holder directly (score 10).
3. The pre-loop `s.has_color = 1;` is a real target store (`addiu $v0,$zero,0x1` at
   8006D420 + `sb $v0,0x40($sp)` at 8006D428), not an artefact. Its absence costs those
   two instructions AND reorders the prologue (score 10 -> 6 when added).
Final residual after those three was pure prologue scheduling: the tie-break order of the
three independent `sw $sN` / `init $sN` pairs follows source order of the initialisers,
and the target's order is semi(s5), i(s1), dim(s2).

### Measured probe ladder (all on HEAD chassis, sandbox --disable all, no FAKE present)
  A  first full body, bare 0x40 literal, no pre-loop has_color init            27
  B  + `s32 dim = 0x40;` declared before the loop                              22
  C  + holder retyped to `u8 dim = 0x40;`                                      10
  D  + `s.has_color = 1;` added to the pre-loop descriptor-init block           6
  E  + declaration order semi, dim, i                                           4
  F  declaration order semi, i, dim via `for (i = 0, dim = 0x40; ...)`          0
  G  declaration order semi, i, dim via `s16 i = 0;` + `for (; i < 6; i++)`     0
G is the shipped form (F and G are byte-identical; G avoids the comma operator).
Full build with G in place: SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa, verify-oracle
reports ok:true / build_matches:true.
