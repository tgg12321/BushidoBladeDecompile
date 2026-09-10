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
