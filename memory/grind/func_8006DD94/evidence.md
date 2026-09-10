# Evidence bank — func_8006DD94

## s1 (recon, 2026-09-10) — chassis: HEAD, honest floor at dispatch 117 (no C body at all)

OBJECT MODEL: D_800A374C — **MATCHES**. Declared `extern s32 D_800A374C;` (m2c_context.h,
and again at file scope inside src/text1b.c at 1571/2818/3141/6344). The target reads it
with `lui %hi / lw %lo` and adds 0x28 before passing it to AddPrim
(asm/funcs/func_8006DD94.s, 8006DEBC..8006DECC) — exactly the shape the COMPLETED-C
sibling func_8006BB68 uses (`AddPrim(D_800A374C + 0x28, arg0[7]);`). sdata_exclude.txt:69
already pins `func_8006DD94: D_800A374C` so maspsx keeps it out of the $gp window: no
declaration change was needed and none was tried. Measured: with that declaration in
place the whole function reaches sandbox score 0 this session, so this symbol's object
model is confirmed byte-exact, not merely plausible. No other global the function touches
was flagged; the gp-rel reads (D_800A352C, D_800A3514, D_800A34FC) use the existing
file-scope `extern s32` declarations unchanged and are byte-exact.

## Structure (fully recovered this session)

The function is a 3-iteration render loop over `q = *(s32 **)(arg0[1] + 0x3C)`: per
iteration it fills a texture/env descriptor on the stack from `q[i + 8]`, calls
func_8007352C (the EnvA consumer at src/text1b.c:6695+), then SetDrawMode + AddPrim,
advancing arg0[5] (the in-tex chain) and arg0[7] (the OT pointer, +0xC per iteration).
After the loop it calls `func_8006D808(&arg0[5], &arg0[7], q, s.arg2 /*=0xA*/, -1)` and
finishes with `func_80069898((GameObj *)arg0, rect, 0x11)` over a 4-halfword rect
{0xF5, 0x25, 0x96, 1} stored in the order [2],[0],[1],[3] — the same store order as the
sibling. The loop counter `i` is HImode (`s16`): the target sign-extends it at every use
(`sll 16 / sra 16` at the compare against `D_800A352C + 1`, `sll 16 / sra 14` for the
`q[i + 8]` index).

The per-iteration colour write is a chained assignment `s.col_r = s.col_g = s.col_b = v;`
— GCC evaluates the innermost assignment first, which reproduces the target's
`sb ..,0x43 / sb ..,0x42 / sb ..,0x41` store order exactly. The colour fields must be
**u8**, not s8: with s8 the constant 0x80 folds to -128 and cc1 emits `li s4,-128` where
the target has `li s4,128` (measured: score 24 -> 23 on that one change).

## The stack descriptor is 0x34 bytes, not 0x2C — and the typedef must be function-local

Frame equation (mips.c compute_frame_size, MIPS_STACK_ALIGN=8):
`frame = ALIGN8(vars) + ALIGN8(args) + ALIGN8(gp_regs)`. The target's descriptor sits at
sp+0x18 and its rect at sp+0x50, so vars = 0x50-0x18+8 = 64. With the shared `S69E18`
typedef (0x2C, src/text1b.c:5557) the struct ends at 0x44, the rect lands at sp+0x48 and
cc1 prints `# vars= 56, regs= 6/0, args= 24` — score 23. Adding two trailing pad words
(size 0x34) moves the rect to sp+0x50, cc1 prints `# vars= 64` (the target's number), and
the score fell 23 -> 8.

**Do NOT widen S69E18 itself.** Measured this session: widening the shared typedef
regressed COMPLETED-C func_8006BB68 from score 0 to score 17. The 0x34 layout is declared
as a function-local `EnvB` typedef immediately above func_8006DD94; with that split both
functions measure 0.

## The last 8 insns: the loop-invariant zero that lives in $s5

The target saves a SEVENTH callee-saved register, sets `addu $s5,$zero,$zero` in the
prologue right next to `i = 0`, and inside the loop passes it as func_8006E480's second
argument (`jal func_8006E480` / delay slot `addu $a1,$s5,$zero`). That one register is
worth all 8 remaining insns: it raises ALIGN8(gp_regs) from 24 to 32, so the frame is 120
instead of 112 and $ra sits at 0x70 instead of 0x6C. Diff = addiu sp x2, sw/lw ra offset,
the missing sw/lw/addu of s5, and the a1 move.

A literal `0` cannot produce it: GCC expands a constant argument straight into the hard
register a1, and loop.c's `move_movables` only hoists loop-invariant SETs whose
destination is a pseudo — there is nothing to hoist and no pseudo for local-alloc to place
in a callee-saved register. Measured spellings (cc1's own `.frame` line read directly —
the instrument from the phantom-frame-slots note — plus the a1 setup insn out of cc1's
asm):

| spelling of func_8006E480's 2nd argument | frame / gp_regs | a1 setup | sandbox |
|---|---|---|---|
| literal `0` | 112 / 6 | `move a1,zero` | 8 |
| `s32 semi; semi = 0;` before the loop | 112 / 6 | `move a1,zero` (cse folded it) | 8 |
| `s32 semi = 0;` declared inside the loop body | 112 / 6 | `move a1,$20` | not scored |
| `s16 semi; semi = 0;` before the loop | **120 / 7** | not a plain move | not scored |
| `u8 semi; semi = 0;` before the loop | **120 / 7** | not a plain move | not scored |
| `s32 *clut; clut = NULL;` passed as `(s32)clut` | **120 / 7** | `move a1,$21` ($s5) | **0** |

So the zero has to reach the call through a narrow-typed or pointer-typed local: an `s32`
zero is constant-folded by cse (which runs before loop.c), a pointer-typed zero is not, so
its `(set pseudo 0)` survives into loop.c, is hoisted out of the loop, and local-alloc
gives the now-loop-spanning pseudo a callee-saved register.

A variant that also measures 0 is a block-scope redeclaration of the callee as
`extern s32 func_8006E480(s32, s32 *);` so the NULL passes with no cast — but that
conflicts with the file-scope `extern s32 func_8006E480(s32, s32);` at src/text1b.c:5626,
so the cast form is the one left in src/.

## s2 (recon, 2026-09-10) — chassis: HEAD + s1 candidate applied; floor 8 ordinary-C → **0 ordinary-C**

**CORRECTION OF s1 — the load-bearing row in s1's table is wrong.** s1 recorded
`s32 semi; semi = 0;` before the loop as "frame 112 / gp_regs 6 / `move a1,zero` (cse
folded it) / score 8", and built its whole ruling-request on the inference that only a
narrow-mode or pointer-mode pseudo can survive cse into loop.c. Re-measured this session
on BOTH chassis, four ways:

| spelling of func_8006E480's 2nd argument | chassis | sandbox |
|---|---|---|
| literal `0`, no named local at all | s1 candidate chassis (s1) | 8 |
| literal `0`, no named local at all | EnvA-named chassis (s2, v15_nolocal) | 8 |
| `s32 semi; semi = 0;` before the loop, read once (the argument) | s1 candidate chassis (s2, v16) | **0** |
| `s32 semi = 0;` declaration initializer, read once | s1 candidate chassis (s2, v17) | **0** |
| `s32 semi; semi = 0;`, read once | EnvA-named chassis (s2, v14) | **0** |
| `s32 semi = 0;`, read twice (`s.semi = semi;` + the argument) | EnvA-named chassis (s2, final_init) | **0** |

So the mode of the local was never the mechanism. The mechanism is only: **a named local
rather than the literal at the call site.** A constant argument is expanded straight into
the hard register `$a1` (`move a1,zero`), so no pseudo is live across the loop, no 7th
callee-saved register is allocated, and the frame is 112 instead of the target's 120. Any
local — `s32`, `u8`, `s16`, `s32 *` — gives the pseudo, and RTL dump `.cse`
(tmp/grind/func_8006DD94/dumps/text1b.cse, sliced to
tmp/grind/func_8006DD94/s1/cse.txt) shows exactly this on the two-read form: `(insn 11
(set (reg/v:SI 77) (const_int 0)))` survives cse in the loop preheader, cse folds the
MEMORY use (`insn 27`, the `s.semi = semi;` store, becomes a `const_int 0` store) but the
in-loop argument read keeps reg 77 live across the loop, and the allocator seats a
loop-spanning call-crossing pseudo in a call-saved register ($s5). `.loop` shows the set
already sits in the preheader, so no LICM hoist is involved — s1's `move_movables` story
is not the mechanism either.

**Ordinary-C floor is 0.** The pointer-clothed `clut` construct that s1 filed a
ruling-request over is UNNECESSARY. src/text1b.c now carries the ordinary-C body and:
`sandbox func_8006DD94 --disable all` = **score 0** (117/117, rules_dropped 0), and
`verify-oracle` = **build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle**,
both this session.

**SEMANTICS: the zero is the semi-transparency mode, and it is truthful in both uses.**
Reading func_8007352C's own decompiled body (src/text1b.c:6710-6805) gives the descriptor
its real field names — it is the `EnvA` struct, and offset 0x10 is `semi`, which
func_8007352C passes to `SetSemiTrans((s32)sp, env->semi)` (src/text1b.c:6790). Reading
func_8006E480's body (src/text1b.c:6106-6110): `(a0[0] & 0xFE1F) + (a0[1] << 7) + a1` is
a getTPage word — the 0xFE1F mask clears bits 5-8, `a0[1] << 7` refills bits 7-8 (the
colour-depth `tp` field) and the second argument refills bits 5-6, which is the PS1 GPU's
**abr (semi-transparency) field**. So one local named `semi` holding 0 feeding both
`s.semi` and func_8006E480's second argument is the same physical quantity in both places:
semi-transparency mode 0 (opaque). The local is not dead, not a pad, has a truthful name
and a truthful type, and is read twice.

**Local naming adopted from EnvA.** The s1 candidate's placeholder names (p0/p1/in_tex/
zero10/arg2/width/zero1C) are replaced by EnvA's (header/table/out/semi/ot_idx/x/y) plus
the two trailing pad words that make the 0x34 size. Byte-neutral (still score 0); the
0x34 widening still MUST stay a function-local typedef (s1: widening the shared EnvA
regressed COMPLETED-C func_8006BB68 from 0 to 17).

**Dead axes measured this session (all banked in rejected/):** narrow-typed holders
(u8 1, u16 1, s8 2, s16 2 — the frame is right but the argument needs an extension insn
the target lacks); passing the struct member itself (`s.semi`) instead of a local (9 —
&s escapes to func_8007352C so the member reloads every iteration); a literal
null-pointer constant cast at the call site with no local (`(s32)(s32 *)0`, 8 — folded
like any constant); the same named zero given a THIRD, in-loop read (`s.y = semi;` in
the else branch, 1).

## s3 (recon, 2026-09-10; dispatched as "session 1" after the s2 layer-1 FAIL) — chassis: HEAD (INCLUDE_ASM, floor 117) + the s2 candidate re-applied

OBJECT MODEL: D_800A374C — **MATCHES**, unchanged from s1. The existing `extern s32
D_800A374C;` plus the `sdata_exclude.txt:69` row (`func_8006DD94: D_800A374C`) gives the
target's `lui %hi / lw %lo` + `addiu 0x28` shape; the body that measures sandbox 0 this
session uses that declaration untouched, so the symbol's object model is byte-confirmed,
not merely plausible. No other global the function touches is flagged (D_800A352C,
D_800A3514, D_800A34FC all read gp-relative through their pre-existing file-scope
`extern s32` declarations and are byte-exact).

### The ENTIRE remaining residual is one 8-byte stack displacement

Re-measured from the s2 body with the two trailing `EnvB` words deleted (i.e. the plain
0x2C descriptor + a separate `u16 rect[4]`, which is what layer-1 would have accepted):

| body | cc1 `.frame` | descriptor at | rect at | sandbox |
|---|---|---|---|---|
| 0x2C descriptor + separate `u16 rect[4]` | `vars= 56, regs= 7/0, args= 24` (frame 112) | sp+0x18 | sp+0x48 | **21** |
| s2's 0x34 descriptor (two TRAILING words — BANNED) + separate rect | `vars= 64` (frame 120) | sp+0x18 | sp+0x50 | 0 |
| merged frame-block struct, rect a member at struct 0x38 | `vars= 64` (frame 120) | sp+0x18 | sp+0x50 | **0** |

The target (asm/funcs/func_8006DD94.s) writes the descriptor at sp+0x18..0x43 and the rect
at sp+0x50..0x57, leaving sp+0x44..0x4F untouched. GCC 2.7.2 8-aligns every stack slot, so
a 0x2C descriptor followed by a separate rect puts the rect at sp+0x48 — 8 bytes short. All
21 differing insns are that displacement plus its knock-on `sw/lw` offsets; nothing else in
the body is wrong. (Instrument: cc1's own `.frame` comment, per [[phantom-frame-slots-gcc272]];
harness `tmp/grind/func_8006DD94/s1/sweep.py`, which patches the body, runs the project's
exact `cpp | cc1`, and prints `vars=` plus the measured sp offsets of `&descriptor` and
`&rect`.)

### No scalar spelling reserves those bytes — measured, not inferred

[[phantom-frame-slots-gcc272]] says GCC 2.7.2 can reserve locals bytes for ordinary LIVE
locals no instruction touches, and names a minimal trigger (two HImode locals feeding an
HImode bitwise expression). Swept on this chassis, every candidate DECLARED BETWEEN the
descriptor and the rect (so that any phantom slot would land in the hole):

| spelling | rect lands at | vars |
|---|---|---|
| base (nothing between) | sp+0x48 | 56 |
| all scalars (`i`,`q`,`c`,`hdr`,`semi`) moved before the rect | sp+0x48 | 56 |
| `s16 aa, bb;` + `s.x = (aa & ~bb) & 1;` (the memory note's own trigger) | sp+0x48 | 56 |
| `s16 cc;` + `s.x = cc & 1;` | sp+0x48 | 56 |
| `s64 acc;` + a 64-bit multiply | sp+0x48 | 56 |
| `s16 i;` hoisted above the rect | sp+0x48 | 56 |
| `u16 uv[4];` written four times | **sp+0x50** | **64** |
| `s32 t[2];` written and read | **sp+0x50** | **64** |
| `s32 t[3];` written and read | sp+0x58 | 72 |

Conclusion: on THIS function only a declared 8-byte AGGREGATE moves the rect to sp+0x50,
and a written one materialises stores at sp+0x44..0x4B that the target does not contain
(the `u16 uv[4]` row emits `sh` at 72/74/76/78). GCC 2.7.2 assigns stack slots to
top-of-function decls in declaration order before any statement is expanded, so an
`assign_stack_temp` — which is allocated during statement expansion — can only land ABOVE
the rect and can never fill this hole. The phantom-live-local escape hatch is measured
CLOSED here.

### The merged frame-block struct (the s3 form) — bytes proven

Modelling the function's whole stack-locals block as ONE struct puts the three unknown
words BETWEEN two used members instead of at the end: descriptor fields at struct
0x00..0x2B (the address `&s.header` = sp+0x18 is what goes to func_8007352C, bit-identical
to s2's `&s`), three unwritten words `sp44/sp48/sp4C` at struct 0x2C..0x37, and the screen
rect as a real member `u16 rect[4]` at struct 0x38 => sp+0x50. Measured this session with
that exact body in src/text1b.c: `sandbox func_8006DD94 --disable all` = **score 0**
(117/117, rules_dropped 0) and `verify-oracle` = **build_sha1
62efab4f73f992798c43e8c730aa43baa10bb4fa, build_matches true**. Two filler words instead of
three puts the rect member at struct 0x34 => sp+0x4C and scores 5 (banked in rejected/) —
the frame is already right there, which isolates the last 5 insns to the rect's own
address+stores.

### In-file precedent for unwritten INTERIOR members in already-accepted C

- `S_69AE4`, src/text1b.c:5424-5426 — the stack-block struct of func_80069AE4, which is
  COMPLETED-C and off the queue. Its members are named for their sp offsets; sp24, sp38 and
  sp3C are never written by func_80069AE4 (verified against asm/funcs/func_80069AE4.s: the
  descriptor-relative writes are 0x0,0x4,0x8,0x10,0x14,0x18,0x1C,0x28 only) and are followed
  by the written member sp40. Its descriptor is passed as `&s.sp18` (src/text1b.c:5498) —
  the same "address of the first member of a frame-block struct" idiom this body uses.
- `EnvA`, src/text1b.c:6654-6669 — the descriptor type of COMPLETED-C func_8007352C, which
  ships pad0C/pad20/pad24: interior members no caller writes.

### The hole is a recurring family feature, not a one-off

Census of every asm function that calls func_8007352C with a stack descriptor
(`tmp/grind/func_8006DD94/s1/descensus.py`): after the descriptor's last written byte at
+0x2B, func_8006DD94 and func_8006F97C both jump to +0x38 (a 12-byte hole), func_80069F80
and func_8006A1A0 jump to +0x40 (20 bytes), func_80070188 jumps to +0x50 (36 bytes). All of
those are still INCLUDE_ASM, so none is usable as decompiled precedent — but the pattern
says whatever source idiom reserves the hole is shared across this whole render-function
family, so a ruling here generalises to at least four more queue items.
