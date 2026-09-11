# Evidence bank — func_8003DE14

## s1 (2026-09-10, recon) — baseline established, chassis re-measured

**OBJECT MODEL:** the dispatch brief carried NO `DATA MODEL` section for this
function, and that is correct: `func_8003DE14` touches **zero globals**. Its entire
data surface is (a) the `s16 *rect` parameter (a 4-halfword GPU RECT: x, y, w, h),
(b) the `s32 count` parameter, (c) three stack locals (`u16 src_buf[0x200]` at
sp+0x10, `u16 dst_buf[0x200]` at sp+0x410, `u8 color_info[0x20]` at sp+0x810), and
(d) four calls (`DrawSync`, `StoreImage`, `LoadImage`, `func_80052BE4`). There is no
`%hi/%lo` or `$gp`-relative reference anywhere in `asm/funcs/func_8003DE14.s`, so no
symbol has a census row, header declaration, or aggregate-merge signal to flag.
Per-symbol verdict table: **(no flagged symbols — the function is global-free.)**
One declaration MISMATCH does exist and is recorded as MISMATCH-unmeasured:
`include/code6cac.h:506` declares `extern void func_8003DE14(s32 *, s32);` while the
target's own body indexes the first argument as `s16[4]` (`lh $s7,2($s0)`,
`lhu $v0,0x6($s0)`, `sh` stores at +0/+2/+4/+6). cc1 emits
"conflicting types for func_8003DE14" when the C body is present. It is
MISMATCH-unmeasured because `include/code6cac.h` is OUTSIDE this function's candidate
scope (`tools/grinder/scope_allow.txt` has no entry for func_8003DE14), and because
`s32 *` vs `s16 *` is codegen-neutral for the pointer itself — the two call sites
(`func_8003E0E0`, `func_8003E120`) both pass `s16 buf[4]` and already byte-match at
HEAD. If a future session needs the header corrected it is an integration handoff,
not a code lever.

## Chassis + floor

- HEAD as committed carries `INCLUDE_ASM("asm/funcs", func_8003DE14);` — sandbox
  scores **173** (`no_c_body: true`). That is the "no C at all" number, not a floor.
- `canonical func_8003DE14` -> verdict **C**, `hand_coded_tier: LOW`, 173 target insns.
  No S1/S2/S6 hand-asm signal. Ordinary pure-C target.
- `memory/grind/func_8003DE14/migration_pin.json` recorded floor **119** on the
  pre-migration (pre `-mel` / pre `-msoft-float`) chassis, banked body in
  `retired-chassis-2026-08/body.c`. That body carries FIVE `register ... asm("sN")`
  pins and one `__asm__ volatile("" : "=r"(count) : "0"(count))` scheduling barrier —
  all cheats, all stripped by the sandbox, which is why 119 was so high.
- **De-cheating that body (pins deleted, barrier deleted, nothing else changed)
  measures 66 on the current chassis.** The 119 pin was a stale-chassis artifact;
  the honest floor was never 119 on this toolchain. `target_insns == build_insns ==
  173` at that point, i.e. the control-flow skeleton, the frame layout (0x858 with
  src_buf/dst_buf/color_info at +0x10/+0x410/+0x810) and the call sequence are
  already exactly right. The whole residual is register allocation + one hoist.
- **Best form this session: 57** (`memory/grind/func_8003DE14/candidate.c`).

## Target's register map (read off asm/funcs/func_8003DE14.s)

`s0`=rect, `s2`=count-1 (the `addiu $s2,$s2,-1` right after the first DrawSync — so
the C-level `count` is already decremented before the outer loop), `s1`=i,
`s7`=saved_y, `s5`=color_info[0] (r), `s4`=color_info[1] (g), `s3`=color_info[2] (b),
`s6`=target_color, `fp`=0x1000 (the blend denominator constant), `a3`=src cursor,
`a2`=dst cursor, `t4`=j, `t3`=factor, `t5`=complement (0x1000-factor).

## The mult/mflo "gap" is NOT a diff (dead end, do not re-derive)

`asm/funcs/func_8003DE14.s` shows 179 instructions including three 2-nop pads between
`mflo` and the next `mult`; the engine reports `target_insns: 173`. That is not a
missing pass — `objdump` elides runs of identical zero words as `...`, and the build
reproduces those pads anyway (GNU `as` in reorder mode inserts the mult/mflo hazard
nops). `multu_pad_funcs.txt` contains ZERO active entries project-wide and
func_8003DE14 does **not** need one. Do not spend a session on multu_pad.

## Pass attribution (dumps read, not guessed) — `tmp/grind/func_8003DE14/dumps/`

`pwsh tools/grinder/dump.ps1 func_8003DE14` with the s1 best form applied. The `.loop`
dump prints move_movables' own decisions verbatim:

    ;; Function func_8003DE14
    Loop from 145 to 303: 57 real insns.          <- the INNER (per-pixel) loop
    Insn 174: regno 117 (life 1), savings 1  moved to 383
    Insn 226: regno 131 (life 1), savings 1 not desirable
    Insn 235: regno 135 (life 1), savings 1 not desirable
    Insn 244: regno 139 (life 1), savings 1 not desirable
    ...
    Loop from 104 to 373: 98 real insns.          <- the OUTER (per-step) loop
    Insn 383: regno 117 (life 77), savings 1 halved since already moved  moved to 384

Identified from `.cse` / `.jump` (same insn numbers):

- insn 226/235/244 = `mult reg76/77/78, reg108` = `r*factor`, `g*factor`, `b*factor`.
  These are loop-invariant and GCC declined to hoist them ("not desirable"). **The
  target also computes all three inside the inner loop** (`mult $s5,$t3`,
  `mult $s4,$t3`, `mult $s3,$t3` at 8003DFA0 / 8003DFC0 / 8003DFE0). So this half of
  the hoist behaviour is already correct and must be preserved by anything we change.
- insn 174 = `(set (reg:HI 117) (subreg:HI (reg/v:SI 79) 0))` = the HImode truncation
  of `target_color` for `*dst = target_color` in the last-step arm. It was hoisted out
  of both loops and then coalesced away; harmless.

**The gate is `loop.c:1631`:**
`if (already_moved[regno] || (threshold * savings * m->lifetime) >= insn_count || ...)`
with `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` (`loop.c:532`) and
`m->savings = n_times_used[regno]` (`loop.c:793`),
`m->lifetime = uid_luid[regno_last_uid] - uid_luid[regno_first_uid]` (`loop.c:791`).

Calibration derived from the dump itself: the inner loop has `insn_count = 57`, and
every `life 1, savings 1` movable in it is reported **"not desirable"**. Therefore
`threshold * 1 * 1 < 57`, i.e. **`threshold <= 56` on this chassis** for a call-free
loop. (This is the `-msoft-float` threshold regime — see the `softfloat-adoption`
memory: hard float doubled this number.) Two load-bearing consequences:

1. A once-set, once-used loop-invariant inside a >= 57-real-insn call-free loop is
   ALREADY left inline by loop.c on this chassis. No anti-LICM construct is needed
   for that shape here — which retires the premise the retired-chassis body was
   built on (its `__asm__` barrier was defeating a hoist that no longer happens).
2. `insn 174` was moved despite identical life/savings, so it went through the
   `already_moved[regno]` / `m->forces` clause of `loop.c:1631`, not the arithmetic
   one. Any future "why did THIS one hoist" question must check that clause first.

## Where the 57 actually lives (side-by-side diff vs `build/src/code6cac_c2.o`)

Reproduce with `python3 tmp/grind/func_8003DE14/s1/sd2.py` (uses `objdump -dz` on both
the sandbox `.o` and the reference `.o`; branch-target text differences it prints are
false positives — the engine masks those).

1. **`count - 1` placement.** Target computes `addiu $v0,$s2,-1` at the inner-loop
   preheader (8003DF34) AND again in the back-branch delay slot (8003E048) — i.e. the
   comparand is recomputed every inner iteration, with the preheader copy being the
   reorg.c delay-slot peel (see `reorg-peel-is-not-a-source-statement`). Our build
   computes it once and keeps it live in a register across the whole inner loop.
2. **`src` / `dst` cursor registers are swapped**: target `a3`=src / `a2`=dst, build
   `a2`=src / `a3`=dst. Swapping the C declaration order (`dst` before `src`) does NOT
   flip them (measured: still 57).
3. **`target_color` OR-association + one extra insn.** Target:
   `v0=(g&0xF8)<<2; v0|=(-0x8000); a0=(r>>3)|v0; s6=a0|((b&0xF8)<<7)`.
   Build folds the constant leftwards: `v1=(r>>3)|(-0x8000); a0=v1|((g&0xF8)<<2); ...`
   and then needs an extra `move s6,a0` (this is the whole `build_insns 174` vs
   `target_insns 173` delta). Note the constant is `(s32)-0x8000` (materialised with
   `addiu $v1,$zero,-32768`), NOT `0x8000` — spelling it `0x8000` would emit `ori`.
4. **r/g/b live in the wrong callee-saves**: target `s5/s4/s3` + `s6`=target_color;
   build `s6/s5/s4` + `s3`=target_color. One-register rotation, almost certainly a
   cascade of (1)/(3) rather than an independent problem.
5. **Blend-body scheduling is structurally identical** (same mult/mflo interleave,
   same `sra 15 / sra 10 / sra 5` + mask sequence); only the scratch register names
   and two mflo destinations differ.

## Forms measured this session (all on HEAD chassis 2026-09-10)

| form | description | score |
|---|---|---|
| HEAD | `INCLUDE_ASM` (no C body) | 173 |
| v1 | de-cheated banked body; `if (i == count - 1)` written inline | **66** |
| v2 | v1 + borrow the existing local `total` for `count - 1` | **57** |
| v3 | v1 + fresh multi-set `tmp` (div numerator, then `count - 1`) | 58 |
| v4 | v2 + named intermediate `gc` for `((g&0xF8)<<2)|-0x8000` | 62 |
| v5 | v2 + `dst` declared before `src` | 57 |

Sources: `tmp/grind/func_8003DE14/s1/body_v1.c` .. `body_v5.c`; v1/v4/v5 also banked
under `memory/grind/func_8003DE14/rejected/`.

- [s1] OBJECT MODEL: the dispatch brief carried no DATA MODEL section and that is correct — func_8003DE14 touches ZERO globals. Its data surface is the `s16 *rect` param (4-halfword GPU RECT), the `s32 count` param, three stack locals (u16 src_buf[0x200] at sp+0x10, u16 dst_buf[0x200] at sp+0x410, u8 color_info[0x20] at sp+0x810) and four calls (DrawSync, StoreImage, LoadImage, func_80052BE4). asm/funcs/func_8003DE14.s contains no %hi/%lo and no $gp-relative reference, so no symbol carries a census row or aggregate-merge signal. Per-symbol verdicts: (no flagged symbols). One MISMATCH-unmeasured: include/code6cac.h:506 declares `extern void func_8003DE14(s32 *, s32);` while the target body indexes arg0 as s16[4] (lh $s7,2($s0); lhu $v0,6($s0); sh at +0/+2/+4/+6), so cc1 warns 'conflicting types'. Unmeasured because include/code6cac.h is outside this function's candidate scope (no entry in tools/grinder/scope_allow.txt) and because s32* vs s16* is codegen-neutral for the pointer itself — both call sites (func_8003E0E0, func_8003E120) pass s16 buf[4] and already byte-match at HEAD. Correcting it would be an integration handoff, not a code lever.

- [s1] Chassis re-measurement: HEAD carries INCLUDE_ASM and scores 173 (no_c_body). `canonical` returns verdict C, hand_coded_tier LOW, no S1/S2/S6 signal — ordinary pure-C target, distance is size not hand-asm evidence.

- [s1] The 119 floor in memory/grind/func_8003DE14/migration_pin.json is STALE and must never be quoted again: de-cheating the banked body (five register-asm pins + one __asm__ barrier removed, nothing else changed) measures 66 on the current -mel/-msoft-float chassis, and further pure-C work reached 57.

- [s1] At 66 and at 57 the build has exactly the right instruction count relative to the reference (173/173 at v1, 174/173 at v2) — the frame (0x858), the local layout, the control-flow skeleton and the four-call sequence are all already correct. The entire residual is register allocation plus one extra copy.

- [s1] Target register map read off asm/funcs/func_8003DE14.s: s0=rect, s2=count-1 (decremented right after the first DrawSync), s1=i, s7=saved_y, s5=color_info[0] (r), s4=color_info[1] (g), s3=color_info[2] (b), s6=target_color, fp=0x1000 blend denominator, a3=src cursor, a2=dst cursor, t4=j, t3=factor, t5=complement.

- [s1] loop.c threshold calibration for THIS chassis, derived from the function's own .loop diagnostics rather than from the formula: the inner (per-pixel) loop is `Loop from 145 to 303: 57 real insns` and every `life 1, savings 1` movable in it is reported `not desirable`, so threshold <= 56 at loop.c:1631 for a call-free loop under -msoft-float. Practical consequence: a once-set once-used loop-invariant inside this inner loop is ALREADY left inline, so no anti-LICM construct is needed for that shape — which retires the premise the retired-chassis body's __asm__ barrier was built on.

- [s1] insn 174 (regno 117) WAS moved out of both loops despite the same `life 1, savings 1`, so it took the `already_moved[regno]` / `m->forces` clause of loop.c:1631 rather than the arithmetic clause. Identified from the .cse/.jump dumps as `(set (reg:HI 117) (subreg:HI (reg/v:SI 79) 0))` — the HImode truncation of target_color for `*dst = target_color` in the last-step arm; it is coalesced away later and is harmless.

- [s1] The three 2-nop mult/mflo pads visible in asm/funcs/func_8003DE14.s are NOT a diff: both the sandbox object and build/src/code6cac_c2.o contain them (GNU as reorder mode inserts the hazard nops), and the 179-vs-173 count gap was objdump's `...` elision of identical zero words. multu_pad_funcs.txt has zero active entries project-wide; this function needs no pipeline scope grant.

- [s1] Remaining diff, item 1 (the +1 instruction): the target computes target_color as v0=(g&0xF8)<<2; v0|=(s32)-0x8000; a0=(r>>3)|v0; s6=a0|((b&0xF8)<<7). The build folds the constant leftwards (v1=(r>>3)|(s32)-0x8000 first) and then needs a trailing `move s6,a0`. Note the constant is genuinely `(s32)-0x8000` (materialised via `addiu $v1,$zero,-32768`), not 0x8000 — spelling it 0x8000 would emit `ori`.

- [s1] Remaining diff, item 2: the target recomputes `addiu $v0,$s2,-1` every inner iteration (preheader copy at 8003DF34 plus the back-branch delay-slot copy at 8003E048 — the reorg.c delay-slot peel, cf. the reorg-peel-is-not-a-source-statement memory). The best form keeps the comparand live in t6 across the inner loop instead.

- [s1] Remaining diff, item 3: r/g/b sit in the wrong callee-saves (target s5/s4/s3 with s6=target_color; build s6/s5/s4 with s3=target_color) — a one-register rotation that is most likely a cascade of the two items above.

- [s1] Reusable tooling written this session: tmp/grind/func_8003DE14/s1/sd2.py prints an aligned side-by-side of the sandbox object against build/src/code6cac_c2.o using objdump -dz (elision off). Its branch-target text differences are false positives — engine/score.py masks control-flow targets.

- [s1] The best form (57) is NOT submittable: it contains `total = count - 1;`, a borrow of an existing local that is dead after its guard test. That is the variable-reuse family (.claude/rules/defeat-licm-hoist-var-reuse.md, gated by staged-value-reused-variable.md) and needs a /* FAKE */ annotation, a quoted scope sentence, a precedent citation and a demonstrated lever-exhaustion ladder. Frontier item F2 exists to determine whether it is needed at all.


## s2 (2026-09-10, structural) - floor 57 -> 52, insn count now EXACT, borrow retired

**The headline: `candidate.c` no longer contains any sanctioned-family construct.**
s1's standing constraint ("the 57 form needs the `total = count - 1;` borrow and
therefore a FAKE family claim") is retired. The comparand stays inline for a purely
mechanical reason that costs nothing at the C level, and the resulting form scores
BETTER than the borrow (52 vs 57) with `build_insns == target_insns == 173` (the
borrow form was 174 - it carried the extra `move s6,a0`).

### The loop.c:1631 threshold is a knife edge, and `threshold -= 3` is the lever

Calibration, all read off `.loop` diagnostics for this function (never guessed):

| form | inner loop "real insns" | first movable | count-1 movable (insn 147) |
|---|---|---|---|
| s1 v2 (borrow, 57) | 57 | insn 174 regno 117 moved | not a loop movable (set outside) |
| v1 (inline, 66) | 58 | **insn 147 regno 115 moved to 382** | hoisted out of inner AND outer loop |
| s2 x1 (complement moved inside, 59) | 59 | insn 144 regno 114 (complement, life 50) moved to 382 | **"not desirable" - stays inline** |

`threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` (`loop.c:532`); the
inner loop is call-free. From v1: `threshold * 1 * 1 >= 58` was TRUE for insn 147 and
FALSE for every later `life 1, savings 1` movable in the same loop, and the only thing
that changes between them is `threshold -= 3` executed after each successful move
(`loop.c:1904`). Therefore **`threshold` is 58 or 59 on this chassis** (58 <= threshold
<= 59, i.e. n_non_fixed_regs is 28 and the real value is 58). That is a one-insn margin
against `insn_count`, which is why this function looked like it needed a coercion: the
comparand hoists only because it is the FIRST movable in a 58-insn loop.

**The fix is ordinary C: declare `complement` inside the inner do-body.**
`s32 complement = blend_base - factor;` is loop-invariant, so loop.c hoists it right
back to the inner-loop preheader - which is exactly where the target keeps it
(`subu $t5,$fp,$t3` at 8003DF30, immediately before the reorg-peeled
`addiu $v0,$s2,-1` at 8003DF34). But it is now the first movable in the list (insn 144,
life 50, savings 1), so it consumes the first move, `threshold` drops to 55, and insn
147 (`count - 1`, life 1, savings 1) fails `55 >= 59` and is left inline at the top of
the loop body - which is the target's shape (the 8003DF34 copy is reorg's duplicate of
the loop-top insn, not a preheader computation; cf. the
`reorg-peel-is-not-a-source-statement` memory). Moving the declaration also pushes the
inner loop to 59 real insns, so the margin is no longer knife-edge either.

Consequence for the NEXT session: there is no outstanding family question on this
function. Everything measured this session is ordinary C.

### Second structural win: target_color as one flat expression

`target_color = ((u32)r >> 3) | (((g & 0xF8) << 2) | (s32)-0x8000) | ((b & 0xF8) << 7);`
(single statement, no `|=` accumulation) scores **52**. With the `|=` accumulation
spelling on the same chassis it is 59. The flat form also fixes the callee-save
rotation that s1 recorded as diff item 4: r/g/b now land in the target's `s5/s4/s3`
and `target_color` in `s6`, and they no longer appear in the side-by-side diff at all.
So s1's F3 ("the r/g/b rotation is a downstream cascade") is CONFIRMED - it fell out
for free.

### Forms measured this session (all HEAD chassis 2026-09-10, `sandbox --disable all`)

On the s1 borrow chassis (base = s1 candidate, 57, build_insns 174) - every OR
re-association is worse and NONE of them removed the extra `move`:

| form | target_color spelling | score |
|---|---|---|
| base | s1 candidate (accumulated with `\|=`) | 57 |
| w1 | flat single expression | 59 |
| w2 | blue-first flat | 66 |
| w3 | green-or-const statement, then red, then blue | 63 |
| w4 | four-step accumulation, green first | 64 |
| w5 | explicit left-nested parens | 59 |

On the new structural chassis (`complement` inside the inner loop):

| form | change vs x1 | score | build_insns |
|---|---|---|---|
| x1 | v1 + complement inside inner loop | 59 | 173 |
| x5 | x1 + `s32 last = count - 1;` block-local | 59 | 173 |
| **y1** | **x1 + flat single-expression target_color** | **52** | **173** |
| y3 | x1 + three-statement target-shaped OR | 58 | 173 |
| y4 | x1 + four-step accumulation, green first | 58 | 173 |
| y6 | x1 + `target_color` declared before r/g/b | 59 | 173 |
| y7 | x1 + two-statement OR | 60 | 173 |
| z1 | y1 + `dst` declared before `src` | 52 | 173 |
| z2 | y1 + `j` declared first in the outer body | 52 | 173 |
| z4 | y1 + green-or-const as the leading term | 53 | 173 |
| z5 | y1 + blue before green | 72 | 173 |
| q1 | y1 + named `gc` intermediate | 55 | 173 |
| q2 | y1 + right-associated OR chain | 68 | 173 |

Sources: `tmp/grind/func_8003DE14/s2/body_*.c`; the interesting losers are banked under
`memory/grind/func_8003DE14/rejected/`.

### What the remaining 52 is (side-by-side vs `build/src/code6cac_c2.o`, y1 applied)

Reproduce with `python3 tmp/grind/func_8003DE14/s1/sd2.py` (Bash tool, not PowerShell -
it shells out to `wsl`, which is not on PATH inside tools/wsl.sh when that is invoked
from PowerShell). Both objects are 179 instructions.

1. **OR-chain association (still open, now the leading item).** Target computes the
   GREEN term first and ORs the constant into it:
   `srl a0,s5,3 / andi v0,s4,0xF8 / sll v0,v0,2 / or v0,v0,v1 / or a0,a0,v0 /
   andi v0,s3,0xF8 / sll v0,v0,7 / or s6,a0,v0`.
   The build folds the constant into the RED term instead:
   `li v0,-32768 / srl v1,s5,3 / or v1,v1,v0 / ... / or s6,v1,v0`.
   Every source-level regrouping tried (seven spellings, table above) either keeps the
   constant on the red term or costs more elsewhere. fold-const.c reassociates the
   constant operand out of the parenthesised green group; the parentheses are not
   honoured. This now needs a `.combine`/`.cse` read, not another spelling guess.
2. **`src`/`dst` cursors still swapped**: target `a3`=src / `a2`=dst, build `a2`=src /
   `a3`=dst. Declaration order is inert (z1 = 52, same as y1) - third independent
   confirmation of s1's K4.
3. **`j`/`complement` swapped in the same way**: target `t4`=j / `t5`=complement,
   build `t5`=j / `t4`=complement. Note this is the SAME relation as (2): in both pairs
   the build gives the lower-numbered temp to the wrong member. One allocation
   decision, two visible symptoms - attack it with `tools/ra_solver`, not spellings.
4. **Two local ordering differences in the blend arm** (the `mflo`/`srl` interleave
   around the blue channel; the `andi 0x7C00` sits after the final `or` chain in the
   build and before it in the target) plus one prologue pair (`addiu a1,sp,16` vs
   `move s1,zero`). Scratch-register names only, no count difference.

- [s2] [s2] New honest floor 52/173 (was 57), and for the first time build_insns == target_insns == 173 - s1's 57 form carried an extra `move s6,a0` (174). Best form: memory/grind/func_8003DE14/candidate.c, also tmp/grind/func_8003DE14/s2/body_y1.c.

- [s2] [s2] candidate.c is now ORDINARY C END TO END. s1's standing constraint (the 57 form needed `total = count - 1;`, a borrow of a dead local = the variable-reuse family, hence a /* FAKE */ annotation + scope quote + precedent + lever-exhaustion) is WITHDRAWN: the borrow is gone, the replacement is strictly better, and there is no open sanctioned-family question on this function.

- [s2] [s2] loop.c threshold pinned by measurement, not formula: the inner loop is call-free, so loop.c:532 gives `threshold = 2 * (1 + n_non_fixed_regs)`, and the dumps bound it to 58 <= threshold <= 59 (a life-1/savings-1 movable IS moved at insn_count 58 but every later one in the same loop is 'not desirable', and the only thing that changes is `threshold -= 3` at loop.c:1904). So n_non_fixed_regs == 28 and threshold == 58 under -msoft-float. This supersedes s1's looser 'threshold <= 56' calibration, which was read off a 57-insn loop and did not account for the -= 3 decay.

- [s2] [s2] The general lever this exposes, reusable on any function with a knife-edge LICM hoist: you do not have to defeat loop.c's movable test on the insn you care about - you can instead give it a CHEAPER neighbour earlier in the loop. Each successful move costs `threshold -= 3` (loop.c:1904, and again at loop.c:1719), so one extra moved invariant ahead of yours in the movables list can flip yours from 'moved' to 'not desirable'. Declaring an existing loop-invariant local inside the loop body where it is used is ordinary, human C and is enough to do it.

- [s2] [s2] Mechanism note for the 8003DF34 / 8003E048 pair in the target: the `addiu $v0,$s2,-1` at the inner-loop preheader is reorg's delay-slot duplicate of the LOOP-TOP insn (cf. the reorg-peel-is-not-a-source-statement memory), not a hoisted invariant. The source truth is that `count - 1` is computed inside the inner loop every iteration - which is what the x1/y1 forms now produce.

- [s2] [s2] target_color spelling matters a lot and the winner is the flat one: `target_color = ((u32)r >> 3) | (((g & 0xF8) << 2) | (s32)-0x8000) | ((b & 0xF8) << 7);` = 52, while `|=` accumulation of the same terms = 59, a named `gc` intermediate = 55, the three-statement target-shaped spelling = 58, right-associated = 68, blue-before-green = 72. On the s1 borrow chassis the same flat form was 59 (worse than its 57) - spelling verdicts for this expression are chassis-relative and must be re-measured after any structural change.

- [s2] [s2] Residual 52 decomposes into exactly four items (side-by-side diff, both objects 179 instructions, tmp/grind/func_8003DE14/s2/sidediff_y1.txt): (1) the OR chain still folds `-0x8000` into the RED term instead of the green one; (2) src/dst cursors swapped (target a3=src/a2=dst); (3) j/complement swapped (target t4=j/t5=complement) - the SAME relation as (2), one allocation decision with two symptoms; (4) two local ordering differences in the blend arm plus one prologue pair, scratch names only, no count delta.

- [s2] [s2] Tooling note: tmp/grind/func_8003DE14/s1/sd2.py must be run from the Bash tool (it shells out to `wsl` directly). `bash tools/wsl.sh ...` invoked from the PowerShell tool fails with 'wsl: command not found'. The batch measurement driver written this session is tmp/grind/func_8003DE14/s2/measure.ps1 (applies body_<form>.c via tmp/grind/func_8003DE14/s1/apply.py with Windows python - safe because apply.py uses newline='' - then runs the sandbox and prints score/build_insns/target_insns); run it as `& tmp/grind/func_8003DE14/s2/measure.ps1 -Forms a,b,c` for one turn per batch.

## s3 (2026-09-10, structural) — floor 52 -> 31; OR chain, i/count and src/dst all closed

Three independent residual items from s2's list were closed this session, each
with a measured ordinary-C lever, and each mechanism was read out of a dump or a
solver model rather than guessed.

### 1. The OR-chain constant: fold-const.c's `associate:` block, and how to aim it

s2's live frontier said the constant absorption "is decided in the TREE before
RTL". That is CONFIRMED by the `.rtl` dump on the y1 (52) chassis: insn 86/87
already read `(set (reg 94) (const_int -32768))` / `(set (reg 93) (ior (reg 92
= red) (reg 94)))`, i.e. the constant is on the RED term before any RTL pass
runs. But the frontier's conclusion — that no remaining regrouping can move it —
is WRONG, and the rule that decides it is short enough to aim:

`fold-const.c:3685 associate:` calls `split_tree` (fold-const.c:882) on arg0 and
then arg1. For `X | (VAR|CON)` (s2's y1 spelling) split_tree(arg0) fails,
split_tree(arg1) succeeds, and the code at fold-const.c:3778 rebuilds
`(ARG0 | CON) | VAR` — the constant migrates ONTO THE OTHER TERM. For
`(VAR|CON) | ARG1` split_tree(arg0) succeeds and fold-const.c:3729 rebuilds
`VAR | (ARG1 | CON)` — again the constant migrates onto the other term.

So the rule is: **whichever term you write the constant next to in C, fold moves
it to the OTHER term.** The target wants `-0x8000` on the GREEN term, therefore
the C must attach it to the RED term:

    target_color = (((u32)r >> 3) | (s32)-0x8000) | ((g & 0xF8) << 2) | ((b & 0xF8) << 7);

That form (`a1`) reaches `.rtl` as `red | (green | -0x8000)` — the target's tree
— and emits `li $3,-32768 / lbu $21,$20,$19 / srl $4,$21,3 / andi $2,$20,0xf8 /
sll $2,$2,2 / or $2,$2,$3 / or $4,$4,$2`, byte-exact against 8003DEA0-8003DEC0
including register names. This retires s2's item (1) and explains all seven
earlier spellings: y1 (52) attaches K to green -> lands on red; z4 (53) attaches
K to green and leads with it -> `green | (red|K)`; q2 right-associated -> same
family. All consistent with the one rule above.

**In isolation `a1` scores 60, WORSE than y1's 52** — it is a strict improvement
that the score hides, because fixing the OR chain flipped the `i`/`count`
callee-save assignment (see 2). Banked as
`rejected/a1-or-chain-exact-but-i-count-callee-saves-swapped-60.c` for the
record; it is the base every later s3 form is built on. Score is not monotone in
correctness here: the sidediff, not the number, told us the OR item was closed.

### 2. `i`/`count` in $s1/$s2 — a 1% allocno-priority margin, moved by statement placement

`tools/ra_solver/extract.py` + `simulate.py` reproduce this function's global
allocation 25/25 exactly. On the a1 chassis pseudo 73 (`count`) had
refs=13/livelen=119 -> priority 3277 and pseudo 74 (`i`) refs=12/livelen=111 ->
priority 3243, so `count` was allocated first and took $s1. Target wants `i` in
$s1. `inverse.py global --goal '{"73": 18, "74": 17}'` returned REACHABLE with 24
one-atom vectors, the cheapest being `live_shrink` on `i` by as little as 2
LUIDs (36*10000/109 = 3302 > 3277).

The ordinary-C lever for that is where the `i = 0;` STATEMENT sits. Moving it
from its s2 position (early, between the second `DrawSync` and the rect fixup)
to anywhere later — measured at four different sites: immediately before
`if (count > 0)`, before `r = color_info[0];`, before `saved_y = rect[1];`,
before `func_80052BE4(color_info)` — all score **43** and all fix both the
$s1/$s2 assignment AND the `move sN,zero` slot in the prologue. 52 -> 43.

### 3. `src`/`dst` in $a3/$a2 — a reg_n_refs gap closed by NOT sharing the tail

On the 43 chassis `inverse.py --goal '{"108": 7, "109": 6}'` (src -> $a3,
dst -> $a2) was REACHABLE at one atom: `refs_down` on src by 6 or `refs_up` on
dst by 6 (src refs=32/pri 27118, dst refs=26/pri 17931 — dst must overtake).
The C lever that lifts `dst`'s references is to stop routing an arm through the
shared `advance_dst: dst++;` tail and write the increment in the arm:

    if (pixel == 0) { *dst++ = pixel; src++; goto loop_check; }

Doing this in the `i == count - 1` zero-pixel arm ONLY (`d4`) scores **31** and
the a2/a3 pair disappears from the diff (re-extracted model confirms 108 -> $a3,
109 -> $a2). Doing it in more arms over-merges: two arms (`d5`) or all three
(`d2`) score 35 with build_insns 171 (jump2 cross-jumps two instructions away);
the blend-arm-only variant `d3` is 37/172; the blend-zero-arm-only variant `d6`
is 33/172. `d4` is the least-merged form that still moves the allocation, and it
is one instruction short (172 vs 173).

### Forms measured this session (HEAD chassis 2026-09-10, `sandbox --disable all`)

| form | change | score | build_insns |
|---|---|---|---|
| base | s2 candidate (y1) | 52 | 173 |
| a1 | red-or-constant-first target_color | 60 | 173 |
| a3 | base + `b_shift` masked in its own statement | 53 | 173 |
| a2 | a1 + a3 | 61 | 173 |
| **b1** | **a1 + `i = 0;` moved late (before `if (count > 0)`)** | **43** | 173 |
| b2/b3/b4 | a1 + `i = 0;` moved to three other late sites | 43 | 173 |
| c1 | b1 + `j` declared inside `if (total > 0)` | 44 | 173 |
| c2 | b1 + `src`/`dst`/`j` all declared inside the `if` | 46 | 173 |
| c3 | b1 + `j` declared before `factor` | 43 | 173 |
| d1 | b1 + `b_shift` masked in its own statement | 44 | 173 |
| d2 | b1 + inline `*dst++` in all three arms | 35 | 171 |
| d3 | b1 + inline `*dst++` in the blend-full arm only | 37 | 172 |
| **d4** | **b1 + inline `*dst++` in the last-frame zero arm only** | **31** | 172 |
| d5 | b1 + inline `*dst++` in both zero arms | 35 | 171 |
| d6 | b1 + inline `*dst++` in the blend zero arm only | 33 | 172 |
| d7 | d4 + `j` declared inside `if (total > 0)` | 33 | 172 |
| d8 | d4 + `complement = 0x1000 - factor` (literal, no `blend_base`) | 31 | 172 |

Bodies: `tmp/grind/func_8003DE14/s3/body_*.c`; side-by-side diffs
`sidediff_a1.txt`, `sidediff_b1.txt`, `sidediff_d2.txt`, `sidediff_d4.txt`.

### What the remaining 31 is (sidediff_d4.txt, target 179 / build 178 objdump lines)

1. **One instruction short.** The target keeps `sh v0,0(a2) / j .L8003E024 /
   addiu a3,a3,2` in the `i == count - 1` zero arm and a separate shared
   `addiu a2,a2,2`; our d4 emits one `j` and lets jump2 merge one instruction
   more than the target does. This is the price of the (3) lever, and it is the
   first thing to attack next: a shape that lifts `dst`'s refs WITHOUT giving
   jump2 a second identical tail.
2. **`j`/`complement` still swapped** (build $t5=j/$t4=complement, target
   $t4=j/$t5=complement). Re-extracted on the d4 chassis: pseudo 115 (`j`)
   refs=11/livelen=58/pri 5689, pseudo 116 (`complement`) refs=11/livelen=53/
   pri 6226. `inverse.py --goal '{"115": 12, "116": 13}'` -> REACHABLE, minimal
   ONE atom, 9 vectors: `live_shrink` 115 by 8, `refs_down` 116 by 1,
   `live_extend` 116 by 8, or `refs_up` 115 by 2.
3. **Blend-arm emission order** — the `mflo`/`srl` interleave on the blue
   channel and the final `or`/`andi 0x7C00` pair. No count delta; a sched.c
   item.

### Ledger lines

- [s3] New honest floor **31/173** (was 52). Best form: `memory/grind/func_8003DE14/candidate.c` = `tmp/grind/func_8003DE14/s3/body_d4.c`. Still ordinary C end to end — no FAKE construct, no sanctioned-family claim.
- [s3] **The fold-const aiming rule (reusable project-wide):** in GCC 2.7.2, for an associative operator with a constant operand, `fold`'s `associate:` block (fold-const.c:3685, via `split_tree` at fold-const.c:882) moves the constant OFF the term you wrote it next to and ONTO the other operand. To make the constant land on term X in the emitted code, write it next to term Y in the C. Parentheses do not protect the grouping; this is decided in the tree, before RTL.
- [s3] **Score is not monotone in per-item correctness on this function.** Fixing the OR chain (a1) raised the score 52 -> 60 because it flipped an unrelated 1%-margin allocno priority; adding the `i = 0;` move on top took it to 43. Judge items by the sidediff, and re-test a "worse" spelling as a BASE before rejecting it.
- [s3] **`i = 0;` placement is a live-range lever.** `i` and `count` sat 1% apart in global.c allocno priority (3243 vs 3277); moving the `i = 0;` statement later by ~15 LUIDs shrinks `i`'s live range enough to reverse the order. Four different late placements all score 43, so the lever is "late", not any particular site.
- [s3] **Arm-local `*dst++` is a reg_n_refs lever, and it is dose-dependent.** Routing ONE arm around the shared `advance_dst: dst++;` tail lifts `dst`'s references enough to swap the src/dst cursor allocation (target $a3=src/$a2=dst); routing two or three arms costs 2 instructions to jump2 cross-jumping. Only the one-arm dose (d4) is both allocation-correct and within 1 instruction.
- [s3] **`tools/ra_solver` reproduces this function 25/25** (extract.py + simulate.py on the d4 chassis, dispositions 25/25 match). Every RA claim in this ledger from s3 on is model-backed, and the two remaining goals it was asked about both came back REACHABLE at one atom — this function has no FORECLOSED allocation item.

- [s3] Honest floor is now 31/173 (was 52), build_insns 172 vs target 173. Best form: memory/grind/func_8003DE14/candidate.c = tmp/grind/func_8003DE14/s3/body_d4.c. Ordinary C end to end — no /* FAKE */ construct, no sanctioned-family claim, no borrowed local.

- [s3] REUSABLE PROJECT-WIDE: in GCC 2.7.2, fold's associate: block (fold-const.c:3685 via split_tree at fold-const.c:882) moves a constant operand OFF the term it is written next to and ONTO the other operand of an associative chain. To make a constant land on term X in the emitted code, write it next to term Y. Parentheses do not protect the grouping — the rewrite happens in the tree, before RTL.

- [s3] Score is not monotone in per-item correctness on this function: fixing the OR chain alone raised the score 52 -> 60 (it flipped a 1%-margin allocno priority elsewhere), and only with the i = 0 move on top did it fall to 43. Items must be judged by the side-by-side diff, and a 'worse' spelling re-tested as a BASE before it is rejected.

- [s3] `i = 0;` placement is a live-range lever: i and count sat 1% apart in global.c allocno priority (3243 vs 3277), and moving the statement later by ~15 LUIDs reverses the order. Four different late placements all score 43, so the lever is 'late', not any particular site.

- [s3] Arm-local `*dst++` is a reg_n_refs lever and it is dose-dependent: routing ONE arm around the shared `advance_dst: dst++;` tail swaps the src/dst cursors into the target's $a3/$a2, while two or three arms hand jump2 a second identical tail and cost 2 instructions (171 vs 173).

- [s3] tools/ra_solver reproduces this function's global allocation exactly (extract.py + simulate.py, dispositions 25/25 on the d4 chassis), so every RA claim from s3 on is model-backed. Both goals it was asked about — i/count and src/dst — came back REACHABLE at one atom and both were then closed in ordinary C; the remaining j/complement goal is also REACHABLE at one atom. No allocation item on this function is FORECLOSED.

- [s3] The residual 31 is exactly three items (tmp/grind/func_8003DE14/s3/sidediff_d4.txt): (1) one instruction short — jump2 merges the last-frame zero arm's tail one instruction further than the target, the price of the dst-refs lever; (2) j/complement still swapped (build $t5=j/$t4=complement vs target $t4=j/$t5=complement); (3) blend-arm emission order (the mflo/srl interleave on the blue channel and the final or/andi 0x7C00 pair), a sched.c item with no count delta.

- [s3] s2's item (4) 'prologue pair (addiu a1,sp,16 vs move s1,zero)' and its items (1) and (2) are all gone from the diff; the whole prologue through the color computation is now byte-exact.


## s4 (permuter) — the 173rd instruction is found; the residual is now ONE reference-count threshold

Chassis at dispatch: HEAD 2026-09-10 (post -mel, post -msoft-float). candidate.c
(the s3 "d4" form) re-measured at the top of the session: **score 31,
build_insns 172, target_insns 173** — the ledger floor reproduces exactly.

### The permuter workspaces (this session's mandated modality)

Two full-TU workspaces were built from scratch (there was no prior permuter
workspace for this function, so the 2026-09-01 chassis rule does not bite):

  * `tmp/perm_3DE14_s4a` — the d4 chassis (score 31). base.c is the WHOLE
    preprocessed `src/code6cac_c2.c`; compile.sh reproduces the Makefile
    pipeline verbatim (cc1 `-O2 -G0 -funsigned-char -mcpu=3000 -mips1
    -mno-abicalls -fno-builtin -w -mel -msoft-float` | prologue_fix | maspsx
    with the full flag set incl. `--prefill-label-funcs` | multu_pad) and then
    sed-extracts the `.ent func_8003DE14` .. `.end func_8003DE14` region before
    assembling. Permuter base score 360. 27,747 iterations, 985 s, best find 300.
  * `tmp/perm_3DE14_s4b` — the f1 chassis (see below). Permuter base score 300.
    7,009 iterations, 447 s, best find 290.

Both were harvested with `--stop`; no campaign outlived the session.

### The d4 campaign's one real datum: it is a dst reference-count problem

The best find on the d4 chassis (`output-300-1`, 300 vs base 360) respells the
zero-pixel arm of the `i == count - 1` branch as `dst = dst++; *dst = pixel;`.
That is undefined behaviour with an unsequenced self-assignment, and under this
cc1 it leaves `dst` UNCHANGED, so the arm stops advancing the destination cursor
— the function computes a different image. Measured 30/174 and rejected on
SEMANTICS (`rejected/e5-...`), not on policy.

Its value is the mechanism it points at: the extra reference to `dst` is what
buys the point. Five semantics-preserving respellings of the same reference
count were then measured by hand on the d4 chassis:

| form | spelling of the zero-pixel arm (or colour arm) | score / insns |
|---|---|---|
| e1 | `*dst = pixel; dst = dst + 1;` | 31 / 172 |
| e2 | `dst[0] = pixel; dst++;` | 31 / 172 |
| e4 | e1 + colour arm also split into store-then-increment | 31 / 172 |
| e6 | colour arm split only | 31 / 172 |
| e3 | `dst = dst + 1; dst[-1] = pixel;` | 31 / **174** |

**Finding: C-level "extra references" that GCC folds back into a single
post-increment RTL insn do NOT raise `reg_n_refs`.** `REG_N_REFS` is counted by
flow.c over RTL, long after the tree has canonicalised `*p = v; p = p + 1;`
into `*p++ = v;`, so e1/e2/e4/e6 are literally the same insn stream as
candidate.c. Only e3, which forces a distinct address form, changes anything —
and it changes the wrong thing (one instruction too many).

### The session's real result: f1 recovers the 173rd instruction

Reading the TARGET's own disassembly around the inner-loop arms (rather than
inferring from the score) settles what s3's d4 sweep got backwards:

```
 0x12c  lhu   v0,0(a3)          ; i == count-1 : pixel = *src
 0x134  bnez  v0,0x148
 0x13c  sh    v0,0(a2)          ; ZERO-PIXEL ARM: store, NO dst++
 0x140  j     0x210             ;   -> advance_dst
 0x144  addiu a3,a3,2           ;   src++ in the delay slot
 0x148  sh    s6,0(a2)          ; COLOUR ARM: store
 0x14c  addiu a2,a2,2           ;   dst++ INLINE
 0x150  j     0x214             ;   -> loop_check
 0x154  addiu a3,a3,2
 0x16c  sh    t0,0(a2)          ; blend zero-pixel arm -> 0x210, src++ in slot
 0x20c  sh    v0,0(a2)          ; blend main path, falls through
 0x210  addiu a2,a2,2           ; advance_dst:
 0x214  lh    v0,4(s0)          ; loop_check:
```

So the arm that carries an inline `dst++` is the **target-colour** arm, and the
zero-pixel arm routes through the shared `advance_dst` tail. s3's d4 put the
inline increment on the zero-pixel arm as WELL, which gave jump2 two identical
`sh / addiu a2 / j / addiu a3` tails to cross-jump — that merge is what ate the
173rd instruction and is why every d-series form topped out at 172.

Form **f1** = candidate.c with exactly one hunk changed:

```c
    if (pixel == 0) {
-       *dst++ = pixel;
+       *dst   = pixel;
        src++;
-       goto loop_check;
+       goto advance_dst;
    }
```

Measured **43 / 173**. The instruction count is now EXACT and a side-by-side
against the target shows the arm block is structurally identical — every
remaining line in that region differs only by which of `$a2`/`$a3` is used.
Banked as `memory/grind/func_8003DE14/chassis_f1_structure_exact_43.c`.

f1's whole residual, from the s4 side-by-side (`tmp/grind/func_8003DE14/s4/sbs.sh`):
  (a) `src`/`dst` swapped — we allocate 108(src)->$a2, 109(dst)->$a3; the target
      has 108->$a3, 109->$a2. This one swap accounts for the entire 43-vs-31
      delta and also drags the v0/v1/a0/a1 naming through the blend arm and the
      loop-bottom `lh v0,4(s0) / lh v1,6(s0) / mult v0,v1` reload.
  (b) `j`/`complement` still $t5/$t4 instead of the target's $t4/$t5 (unchanged
      since s3).
  (c) the blend arm's mflo/srl interleave (unchanged since s3, H-s3-4).

### ra_solver re-extracted on the f1 chassis (the model is exact here)

`tools/ra_solver/extract.py func_8003DE14 code6cac_c2` + `simulate.py`:
sort order MATCH, **dispositions 25/25**.

  * pseudo 108 (`src`): refs 32, priority 27118 -> $a2 (reg 6)
  * pseudo 109 (`dst`): refs 26, priority 17931 -> $a3 (reg 7)
  * pseudo 115 (`j`): refs 11, livelen 57, priority 5789 -> $t5
  * pseudo 116 (`complement`): refs 11, livelen 52, priority 6346 -> $t4

`inverse.py global <model> --goal '{"108": 7, "109": 6}'` -> minimal solution
size **1 atom, 8 distinct vectors**, and every one of them is a reference count:
`refs_down 108: 32 -> 26 (or lower)` or `refs_up 109: 26 -> 32..38`. There is NO
live-length, birth-order or preference vector at size 1. The reason is a
`floor_log2` threshold in global.c's priority formula
(`floor_log2(refs) * refs * 10000 / livelen`): the two live lengths are within
one LUID of each other (~59 vs ~58), but floor_log2(32) = 5 while
floor_log2(26) = 4, so src gets a 25% priority bonus purely from crossing 32.

`inverse.py global <model> --goal '{"115": 12, "116": 13}'` -> minimal solution
size 1, 9 vectors: `live_shrink 115 by 8` (measured dead, H-s3-5),
`refs_down 116 by 1..5`, `live_extend 116 by 8`, `refs_up 115 by 2`.

### The refs_down-on-src direction is measured dead in its two natural spellings

The only ordinary-C way to remove ~6 weighted references from `src` is to stop
writing four separate `src++`:

  * **g1** — `u16 pixel = *src++;` hoisted to the top of the inner do-body, all
    four arm-local `src++` deleted: **61 / 162**. Eleven instructions vanish:
    with one increment the three `j <tail>` arms have nothing for reorg.c to put
    in their delay slots.
  * **g2** — only the `*src` READ hoisted, the four `src++` kept: **48 / 166**.
    The single shared load lets jump2 merge arm tails; seven instructions lost.

Both destroy the very arm structure f1 exists to reproduce, so the refs_down
direction cannot be bought without giving back the 173rd instruction.

### f1-chassis permuter campaign

7,009 iterations found nothing below the base 300 except `output-290-1`, whose
only change is `extern void DrawSync(s32);` -> `extern long long DrawSync(s32);`
in the TU's declarations. A DImode return changes the call's clobber set and
therefore the pressure at the call, which is why it moves the permuter score by
10. It is (i) outside this session's surface (the declaration lives in the
shared includes, not in func_8003DE14), (ii) a prototype contradiction on a
Sony library function whose real signature returns `int`, and (iii) not measured
against the honest objdump metric. Banked as a LEAD for a future session that
wants to test whether the target's TU really did see a wider DrawSync
prototype — under the prototype-contradiction norm
([[sotn-prototype-struct-precedent-2026-08-10]]: bytes decide the declaration)
that is a legitimate question, but it needs its own evidence, not a permuter
score delta.

- [s4] candidate.c (s3 d4 form) re-measured at dispatch on HEAD: score 31, build_insns 172, target_insns 173 — the ledger floor reproduces exactly on the current chassis.

- [s4] The target's inner-loop arm block (assembled from asm/funcs/func_8003DE14.s) is: 0x13c zero-pixel arm `sh v0,0(a2) / j 0x210 / addiu a3,a3,2` (no dst increment); 0x148 colour arm `sh s6,0(a2) / addiu a2,a2,2 / j 0x214 / addiu a3,a3,2`; 0x16c blend zero-pixel arm `sh t0,0(a2) / j 0x210 / addiu a3,a3,2`; 0x20c blend main falls through; 0x210 = advance_dst (`addiu a2,a2,2`); 0x214 = loop_check.

- [s4] Form f1 (zero-pixel arm of the i==count-1 branch respelled `*dst = pixel; src++; goto advance_dst;`) measures 43 / 173 — the FIRST form in this grind with the target's instruction count. Banked as memory/grind/func_8003DE14/chassis_f1_structure_exact_43.c.

- [s4] f1's complete residual: (a) src/dst swapped ($a2/$a3), which also drags the v0/v1/a0/a1 naming in the blend arm and the loop-bottom `lh v0,4(s0) / lh v1,6(s0) / mult v0,v1` reload; (b) the j/complement $t4/$t5 pair; (c) the blend arm's mflo/srl interleave. Items (b) and (c) are unchanged from s3.

- [s4] ra_solver on the f1 chassis: sort order MATCH, dispositions 25/25. 108 (src) refs 32 / pri 27118 -> $a2; 109 (dst) refs 26 / pri 17931 -> $a3; 115 (j) refs 11 / livelen 57 / pri 5789 -> $t5; 116 (complement) refs 11 / livelen 52 / pri 6346 -> $t4.

- [s4] inverse.py global --goal '{"108": 7, "109": 6}' on the f1 model: minimal solution size 1, EIGHT vectors, all reference-count (refs_down 108 to <=26, or refs_up 109 to >=32). No live-length or birth-order atom exists at size 1 — the two live lengths are ~59 and ~58, so the entire priority gap is the floor_log2 step from 26 (=4) to 32 (=5).

- [s4] C-level reference splits that the tree folds are INERT for reg_n_refs: `*dst = pixel; dst = dst + 1;`, `dst[0] = pixel; dst++;` and the same split applied to the colour arm all reproduce candidate.c's 31 / 172 exactly. Only a spelling that survives to RTL as a distinct address form changes anything (`dst = dst + 1; dst[-1] = pixel;` = 31 / 174).

- [s4] Hoisting src's read and/or increment out of the arms (g1 = 61/162, g2 = 48/166) is measured dead: the target's four delay-slot `addiu a3,a3,2` exist only because there are four separate src++ statements for reorg.c to fill from.

- [s4] Two permuter campaigns totalling 34,756 iterations produced no semantics-preserving in-function improvement. Both were harvested with --stop; nothing outlived the session.

- [s4] A reusable full-TU permuter workspace for this function now exists (tmp/perm_3DE14_s4a / _s4b): base.c is the whole preprocessed src/code6cac_c2.c and compile.sh reproduces the Makefile pipeline verbatim including -mel/-msoft-float and --prefill-label-funcs, then sed-extracts the .ent/.end region. A later session can re-seed it by re-running cpp over an edited src.

- [s4] src/code6cac_c2.c was restored to its committed INCLUDE_ASM state before the session ended; the only tracked changes are the ledger files under memory/grind/func_8003DE14/.


## s5 (enumerate, 2026-09-10)

Systematic spelling sweep per the owner's 2026-09-08 ruling. Four regions,
4,488 distinct spellings, every one scored with `sandbox --disable all`.

| sweep | chassis | region | axes | N | best | baseline |
|---|---|---|---|---|---|---|
| enum1 | f1_srcup (43/173) | inner-loop blend block | naming x decl order | 1800 | **42 / 173** | 43 |
| enum2 | h1 (42/173) | outer-loop decl block total/src/dst/factor/j | decl order | 120 | 42 (= base) | 42 |
| enum3 | d4 (31/172) | inner-loop blend block | naming x decl order | 1800 | 31 (= base) | 31 |
| enum4 | h1 (42/173) | inner-loop blend block | naming x decl order x operand swaps | 768 | 42 (= base) | 42 |

Score histograms are in hypotheses.md [s5]; the raw per-variant results are
tmp/grind/func_8003DE14/s5/enum{1,2,3,4}_results.json and the variant bodies
are in the sibling enum{1,2,3,4}/ directories.

What this buys the next session:

1. **The floor did not move** - it is still 31 on the d4 chassis (172 insns).
   The structurally-exact chassis improved 43 -> 42 (173 insns) and is banked
   as `memory/grind/func_8003DE14/chassis_h1_structure_exact_42.c`. The h1 form
   keeps `r_src`, `g_src`, `b_src` and `r_ch` as named locals and writes the
   green and blue channel expressions inline in the final store.

2. **The blend block's spelling space is swept, on BOTH chassis** - 1,800
   spellings on each, plus 768 more with the operand-swap axis on top. Nothing
   in "which sub-expression is a named local", "what order do the declarations
   sit in", or "which way round is each product written" reaches the target on
   either chassis. A later session should not hand-probe another blend-arm
   respelling; that region is finished.

3. **Declaration/birth order in the block where src and dst are born is not the
   lever for the src/dst seat.** All 120 orderings of
   `total / src / dst / factor / j` were measured; the seat never moved and the
   whole axis is worth one point (42 vs 43). This corroborates s4's ra_solver
   finding from the C side: the goal really is a pure reference-count move on
   pseudo 109, not a live-range or birth-order move.

4. **`src++` position inside the blend arm is byte-neutral** (43/173 whether it
   sits before the innermost brace, inside it after the declarations, or after
   the store). Useful for building future enumeration chassis: hoisting it
   before the brace turns the blend block into an all-declaration region.

5. **The operand-swap axis is nearly inert here** - all 768 swap spellings land
   in a 4-point band (42-45). Under -msoft-float GCC 2.7.2 canonicalises the
   multiply and add operand order in this expression shape before RTL, so
   commutative swaps are not a lever worth a future session on this function.

Where the residual must be, by elimination: not in the blend block's spelling,
not in the outer declaration block's order, not in operand order, not in the
reference-count spellings s4 swept, and not in what the permuter reaches from
either chassis. What remains untouched is declaration SCOPE and BLOCK STRUCTURE
above the blend block (where `src` and `dst` are declared relative to the outer
do-loop, and whether the two arms live in one block or two), and the object
model for `rect` - currently a bare `s16 *` read through three different casts,
never yet tried as a struct.

- [s5] Floor unchanged at 31 / 172 (d4 chassis, memory/grind/func_8003DE14/candidate.c). The structurally-exact 173-instruction chassis improved from 43 to 42 and is banked as memory/grind/func_8003DE14/chassis_h1_structure_exact_42.c — it differs from f1 only in the blend block's naming (r_src/g_src/b_src/r_ch stay named; green and blue are inline in the store) and in src++ sitting one statement earlier.

- [s5] 4,488 distinct ordinary-C spellings were compiled and scored this session across four regions and three chassis, with zero forms at distance 0 and zero forms below either chassis baseline apart from the single 43 -> 42 improvement.

- [s5] The inner-loop blend block's spelling space is now swept on BOTH chassis (1,800 spellings each) plus 768 more with commutative operand swaps on the h1 chassis. No hand-probing of another blend-arm respelling is worth a future session.

- [s5] All 120 def-before-use orderings of the outer-loop declaration block 'total / src / dst / factor / j' were measured on the h1 chassis: 40 score 42, 80 score 43, and the src/dst register seat never moved. This corroborates s4's ra_solver result from the C side — the 108/109 goal is a pure reference-count move, not a live-range or birth-order move.

- [s5] The commutative-operand-swap axis in the blend products spans only 42-45 over 768 spellings, so GCC 2.7.2 canonicalises this expression shape's multiply/add operand order before RTL; swaps are not a lever for this function.

- [s5] src++ placement inside the blend arm is byte-neutral on the f1 chassis (43 / 173 in all three positions measured).

- [s5] tools/sweep_variants.py restores src/code6cac_c2.c byte-exact after every sweep; the tree is clean of src edits and only the ledger files under memory/grind/func_8003DE14/ were modified.

## s6 (2026-09-10, synthesis) — the seat question is RE-FRAMED; floor stays 31

Chassis re-measured at dispatch (the brief's CHASSIS CHECK said "measurement
unavailable"): `candidate.c` (s3 d4) = **31 / 172**, `chassis_h1...c` = **42 /
173**. Both reproduce the ledger exactly. Neither banked form contains a FAKE
construct, so `tools/fake_ablate.py` is a no-op on them; the KILL RE-AUDIT was
discharged by re-measuring the two closest-to-target banked forms on the
current chassis (both unchanged) and then re-deriving the seat question from
the dumps rather than from the banked inverse.py verdict.

### 1. reg_n_refs is loop-depth weighted, and this function's counts are exact

`flow.c:2081 / 2329 / 2515 / 2725` all do `reg_n_refs[regno] += loop_depth;`,
where `loop_depth` is `basic_block_loop_depth[]` (flow.c:456/471), seeded at
**1** for the function's top level and incremented per enclosing
`NOTE_INSN_LOOP_BEG` (a `loop_depth == 0` is an abort at flow.c:1453). For
func_8003DE14 that makes the outer do-loop depth 2 and the per-pixel inner
loop depth 3, and it turns `reg_n_refs` into a countable property of the
emitted arm structure:

    src : (4 x `addiu a3,a3,2` [set+use = 2 refs] + 2 x `lhu ..,0(a3)` [1 ref])
          x depth 3  +  `addiu a3,sp,0x10` [set = 2 refs] x depth 2  =  32
    dst : (4 x `sh ..,0(a2)` [1 ref] + 2 x `addiu a2,a2,2` [2 refs])
          x depth 3  +  `addiu a2,sp,0x410` [2 refs] x depth 2       =  26

Those are exactly the numbers `tools/ra_solver` extracts from our build
(pseudo 108 refs 32, pseudo 109 refs 26), which validates the accounting.

### 2. The TARGET'S OWN refs are the same 32 / 26 — so the banked "refs move" framing cannot be how the original did it

Counted straight off `asm/funcs/func_8003DE14.s`: `$a3` (src) is referenced by
2 `lhu` (8003DF40, 8003DF6C) and 4 `addiu $a3,$a3,2` (DF58, DF68, DF88, DFE4)
plus the `addiu $a3,$sp,0x10` init at 8003DEE8 — 32 weighted refs. `$a2` (dst)
is referenced by 4 `sh` (DF50, DF5C, DF80, E020) and 2 `addiu $a2,$a2,2`
(DF60, E024) plus the `addiu $a2,$sp,0x410` init at 8003DEEC — 26 weighted
refs. Identical to ours.

Consequence: **the original compiled this function with src at 32 refs and dst
at 26 refs and still put src in `$a3`.** So `inverse.py`'s size-1 vectors
(refs_up 109 to 32 / refs_down 108 to 26), which s4 and s5 treated as the
description of the residual, describe a way to FORCE our allocation, not the
way the original obtained its own. Any session that spends itself trying to
lift dst's reference count is reproducing a compilation the original did not
perform.

### 3. On the h1 chassis the residual is NOT a src/dst swap

The ledger's live frontier said "the src/dst seat" as if the pair were
transposed. Re-measured with `tmp/grind/func_8003DE14/s4/sbs.sh` on h1 and
with `ra_solver simulate --trace`:

    h1  : 108 (src) -> $a1 (reg 5)      109 (dst) -> $a2 (reg 6)
    tgt : src        -> $a3 (reg 7)     dst        -> $a2 (reg 6)

**dst is already correctly seated on h1**; src sits two registers low. The
find_reg trace explains it exactly: `108 pri 27118 hard_conf=[2,3,4,29]
someone=[3] -> best 5`. Only $v0/$v1/$a0 are excluded, so the ascending scan
stops at $a1.

The h1 side-by-side also shows the biggest remaining block is not the seat at
all: ~25 of the 42 differing instructions are the blend arm, where the target
computes ALL SIX products and both adds before it starts the OR chain, while
h1 interleaves an `or` after each channel.

### 4. The blend block's naming controls how many allocnos outrank src (k1)

Six new blend spellings were measured on h1 (`tmp/grind/func_8003DE14/s6/`):

| form | blend block | score | insns |
|---|---|---|---|
| h1 (base) | r_src/g_src/b_src/r_ch named; green+blue inline in the store | 42 | 173 |
| k6 | k1 shape + `out` accumulator (`out \|= ...`) | 42 | 173 |
| **k1** | **r_ch, g_ch masked-named; `b_shift` named UNMASKED, `& 0x7C00` in the store — the TARGET's own shape** | **43** | 173 |
| k5 | r_ch masked-named; g_shift, b_shift named unmasked | 43 | 173 |
| k2 | r_ch, g_ch, b_ch all masked-named | 44 | 173 |
| k3 | r_ch, g_ch named; blue wholly inline | 48 | 173 |
| k4 | the three SUMS named; shift+mask inline in the store | 52 | 173 |

k1 scores one WORSE than h1 and is nonetheless the better chassis (the s3
"score is not monotone in per-item correctness" lesson again). Its extra named
intermediate creates one more short-lived, high-priority allocno; that allocno
takes $a1, and the pair moves to the classic swap:

    k1  : 108 (src) pri 27118 hard_conf=[2,3,4,5,29]   -> $a2
          109 (dst) pri 17931 hard_conf=[2,3,4,5,6,29] -> $a3

Banked as `memory/grind/func_8003DE14/chassis_k1_target_blend_naming_43.c`.

### 5. The seat IS reachable, and it costs exactly the 173rd instruction (k8)

`k8` = k1 + the s3 "d4" dose (last-frame zero arm respelled `*dst++ = pixel;
src++; goto loop_check;`). That is +2 dst RTL references at depth 3 = +6
weighted refs, dst 26 -> 32, which crosses the `floor_log2` step in
`allocno_compare` (global.c:643):

    k8  : 109 (dst) refs 32 pri 27118 -> $a2      108 (src) pri 26666 -> $a3

**the target's seat, confirmed by `ra_solver simulate --trace` on
`tmp/grind/func_8003DE14/s6/k8.model.json`.** Score 31 / **172** — the arm's
tail is now byte-identical to the colour arm's (`sh / addiu a2 / j loop_check
/ addiu a3`) and jump2 cross-jumps it away. So the seat and the 173rd
instruction are, on every form measured so far, mutually exclusive: the ONLY
C-level way found to give dst six more weighted references is to add a
dst-modifying instruction, and the only place to add one hands jump2 a second
identical tail.

Banked as `memory/grind/func_8003DE14/chassis_k8_target_seat_172insn_31.c`.

### 6. The preference route is mechanically FORECLOSED for this function

`find_reg` (global.c:952) has exactly three ways to skip a register for src:
`hard_reg_conflicts`, pass-0's `regs_someone_prefers`, and the class/fixed
mask. `regs_used_so_far` cannot be the discriminator — global.c:367 seeds it
with every `call_used_reg` before any allocation, so $a1/$a2/$a3 are in it
from the start. `prune_preferences` (global.c) builds
`regs_someone_prefers[src]` as the union of `hard_reg_full_preferences` of the
LOWER-priority allocnos that conflict with src. `inverse.py global` on the k1
model reports that route dead in so many words:

    FORECLOSED - 50 preference atom(s) NOT emitted (mechanically unreachable
    from C):  "$a2 never appears as a hard reg in this function's pre-RA RTL,
    so global.c set_preference can never record a preference for it."

$a2 is argument register 3; this function's four callees (`DrawSync`,
`StoreImage`, `LoadImage`, `func_80052BE4`) take at most two arguments, and
there is no call anywhere inside src's live range. So no allocno can ever
prefer $a2, and pass 0 can never deflect src off it.

`inverse.py global --goal '{"108": 7, "109": 6}'` on the k1 model: minimal
solution size **1 atom, 8 vectors**, ALL of them `refs_down 108: 32->26` or
`refs_up 109: 26->32`. No conflict, live-length, birth-order or preference
atom exists at size 1 on this chassis either.

### 7. Declaration/birth order is inert on the k1 chassis too

s5 swept all 120 orderings of the outer-loop declaration block on the h1
chassis. Because h1's seat turned out not to be the swap, that sweep did not
actually test the swap. All 24 `src`-first orderings of
`total / src / dst / factor / j` were therefore re-swept on the k1 chassis
(`tmp/grind/func_8003DE14/s6/declperm_results.json`): **every one scores 43**,
identical to k1 itself. Birth order cannot create a short-lived temp that
conflicts with src but not with dst, because both cursors are born one
instruction apart and die at the same inner-loop exit.

### Where that leaves the residual

On any chassis that reproduces the target's 173-instruction arm structure the
reference counts are pinned at 32 / 26 by that structure (section 1), the live
lengths are pinned within one LUID of each other by the fact that both cursors
span the same inner loop (`allocno_compare` would need L_dst < 9.6 or
L_src > 357 to flip on live length alone), the preference route is foreclosed
(section 6) and birth order is inert (section 7). The next lever is therefore
not "find another spelling": it is the `refs_up` lever that inverse.py itself
names as SANCTIONED — `duplicated-statement-into-arms` — applied so that jump2
duplicates the statement BACK at codegen instead of cross-jumping it away.
That family carries a FAKE annotation, a byte-neutrality proof and a
lever-exhaustion ledger, and this session is the first that can point at a
concrete exhaustion argument for it.

- [s6] [s6] Chassis re-measured at dispatch (the brief reported 'measurement unavailable'): candidate.c (the s3 d4 form) = 31 / 172 and chassis_h1_structure_exact_42.c = 42 / 173, both reproducing the ledger exactly. Neither banked form contains a FAKE construct, so tools/fake_ablate.py is a no-op on them; the mandated KILL RE-AUDIT was discharged by re-measuring the two closest-to-target banked forms on the current chassis and then re-deriving the seat question from the dumps instead of trusting the banked inverse.py verdict.

- [s6] [s6] REUSABLE PROJECT-WIDE: GCC 2.7.2's reg_n_refs is loop-depth weighted (flow.c:2081, 2329, 2515, 2725; basic_block_loop_depth seeded at 1 at flow.c:456, loop_depth==0 aborts at flow.c:1453). A reference in a doubly-nested loop is worth 3, in a singly-nested loop 2, at function scope 1, and an `addiu rX,rX,K` counts TWO references (set + use) while a load or store counts one. That makes reg_n_refs a countable property of the emitted instruction stream, not a free spelling variable - on any chassis whose arm structure matches the target's, the reference counts are pinned.

- [s6] [s6] The accounting is exact for this function: src = (4 x addiu + 2 x lhu) at depth 3 + init at depth 2 = 32; dst = (4 x sh + 2 x addiu) at depth 3 + init at depth 2 = 26. Both numbers match tools/ra_solver/extract.py's model for pseudos 108 and 109.

- [s6] [s6] THE TARGET'S OWN reference counts are the same 32 and 26, counted directly off asm/funcs/func_8003DE14.s ($a3: lhu at DF40/DF6C, addiu at DF58/DF68/DF88/DFE4, init at DEE8; $a2: sh at DF50/DF5C/DF80/E020, addiu at DF60/E024, init at DEEC). So the original compiled with src at 32 refs and dst at 26 refs and still seated src in $a3. The s4/s5 frontier's premise - that reaching the target means lifting dst's reference count - describes a way to force OUR allocation, not the original's.

- [s6] [s6] On the h1 chassis the residual is NOT the src/dst swap the ledger describes: dst is already at $a2 (correct) and src is at $a1. find_reg trace: `a=108 pri=27118 hard_conf=[2,3,4,29] someone=[3] best=5`.

- [s6] [s6] The h1 side-by-side also shows the seat is no longer the biggest diff block: roughly 25 of the 42 differing instructions are the blend arm, where the target emits all six mult/mflo pairs and both channel adds before any `or` while our builds interleave an `or` after each channel. That is a first-pass sched.c question, not a naming one (s5 exhausted naming).

- [s6] [s6] Blend-block naming controls how many allocnos outrank the cursors. The target's own shape (r_ch and g_ch masked-named, blue named carrying only the shift with its 0x7C00 mask inline in the store) adds one more short-lived high-priority allocno; it takes $a1 and moves the pair to the classic swap. Scores on h1: k6 42, k1 43, k5 43, k2 44, k3 48, k4 52, all 173 insns. k1 is banked as memory/grind/func_8003DE14/chassis_k1_target_blend_naming_43.c and is the chassis future work should start from even though h1 scores one better.

- [s6] [s6] k8 (k1 + the s3 d4 dose on the last-frame zero arm) proves the target's seat is reachable and prices it: +2 dst RTL refs at depth 3 = +6 weighted refs takes dst 26 -> 32, crossing the floor_log2 step in allocno_compare (global.c:643), and simulate --trace gives dst $a2 / src $a3. Score 31 / 172 - jump2 cross-jumps the arm's now-identical tail and eats the 173rd instruction. Banked as memory/grind/func_8003DE14/chassis_k8_target_seat_172insn_31.c.

- [s6] [s6] find_reg's regs_used_so_far can never be the discriminator on this function: global.c:367 seeds it with every call_used_reg before any allocation, so $a1/$a2/$a3 are in it from the start. The only pass-0 exclusion that can move src is regs_someone_prefers, and inverse.py reports that axis mechanically foreclosed - $a2 never appears as a hard reg in this function's pre-RA RTL (no callee takes three arguments and no call lies inside either cursor's live range), so global.c set_preference can never record a preference for it.

- [s6] [s6] inverse.py global on the k1 model with --goal '{"108": 7, "109": 6}': minimal solution size 1 atom, 8 distinct vectors, every one refs_down 108: 32->26 or refs_up 109: 26->32. No conflict, live-length, birth-order or preference atom exists at size 1 on the swap-showing chassis either.

- [s6] [s6] Declaration/birth order is inert on the k1 chassis: all 24 src-first permutations of `total / src / dst / factor / j` score 43, identical to k1. Both cursors are born one instruction apart and die at the same inner-loop exit, so the block cannot produce a temp that conflicts with src but not with dst.

- [s6] [s6] Tooling: tmp/grind/func_8003DE14/s6/sweep.ps1 is a repo-pinned wrapper around tools/sweep_variants.py (the worktree contamination guard blocks the unpinned Bash form; the wrapper hardcodes the main repo's absolute WSL path, which is the same guarantee wteng.ps1 gives engine calls). tmp/grind/func_8003DE14/s6/mkvars.py and mkdecl.py generate the blend-shape and declaration-order variant families off a banked chassis body.

- [s6] [s6] src/code6cac_c2.c was restored to its committed INCLUDE_ASM state before the session ended; the only tracked changes are the ledger files under memory/grind/func_8003DE14/.

- [s7] The residual is FOUR pseudos, not two. `goal_from_tgt.py classify code6cac_c2 func_8003DE14` (the object-level path; the text-stream classifier refuses on a zero-rule function) types the whole gap as FIRST DIVERGENCE: RA with ours 173 / target 173 insns, and `goal_from_tgt.py goal --model .../k1.model.json` attributes it uniquely to {108 src -> $a3, 109 dst -> $a2, 115 j -> $t4, 116 complement -> $t5}. Every session before s7 ran the solver against the cursor pair alone.

- [s7] Against that full goal inverse.py's minimal solution is 2 atoms / 72 vectors, always one cursor atom x one j/complement atom - the two pairs are independent sub-problems and can be attacked separately. Reports: tmp/grind/func_8003DE14/s7/inverse_fullgoal_d2.txt and inverse_cursor_d2.txt.

- [s7] The j/complement pair is a pure live_length question: both allocnos carry refs 11, so allocno_compare (global.c:643) decides on live_length alone, and on an exact tie it falls through to `return *v1 - *v2` (global.c:653) - the lower allocno wins, and j IS the lower allocno (115 vs 116). The validated simulator says j livelen 52 (equal to complement's) already yields 115=$t4 / 116=$t5, the target's disposition; 53 does not. The requirement is ONE insn, not the 8 inverse.py's live grid samples. Harness: tmp/grind/func_8003DE14/s7/tie_test.py.

- [s7] j's live length bottoms out at 53 by birth placement. Moving `s32 j = 0;` inside the `if (total > 0)` guard takes it 57 -> 53; additionally moving src, dst and/or factor into the guarded block ahead of it leaves it at exactly 53 in all six r-forms (scores 44-46, all 173 insns). The residual insn is loop.c's hoisted `complement` subu, which is emitted immediately before the loop start note and therefore always after j's init.

- [s7] Writing complement in the outer body ahead of j (the live_extend 116 atom) costs the `count - 1` inline property: all four u-forms score 65/173, and the side-by-side against the target object shows `addiu s6,s2,-1` hoisted into the outer preheader at stream position 48 where the target has `li s8,4096`. It also RENUMBERS the pseudos (complement becomes 115, j becomes 116) - the emitted `subu $t4 / move $t5` pair is byte-identical to k1's, so the pair is not fixed. Any model read-out on a form that moves a declaration must be keyed on ROLE, not on the pseudo number.

- [s7] `if (j < total)` as the inner-loop guard (j is 0 there, so the predicate is identical) buys j refs 11 -> 13, pri 5789 -> 6842, and DOES seat j at $t4. It costs 18 points elsewhere - 61/173 - because materialising j for the compare replaces the single `blez` guard with an slt/branch pair and shifts the preheader. Banked at memory/grind/func_8003DE14/rejected/s7-j-guard-refs-up-seats-t4-but-61.c.

- [s7] Exact cursor thresholds, measured through the validated simulator (tmp/grind/func_8003DE14/s7/cursor_thresh.py): dst livelen must go 58 -> <=38, or src livelen 59 -> >=90, or dst nrefs 26 -> 32. Both cursors are live across the entire inner loop (each is incremented and re-read every iteration, so each is live over the back edge) and that loop's block span is ~50 insns, so neither live-length door is a property any spelling of this loop can have. The cursor seat is a reference-count question and nothing else.

- [s7] In the target's stream three of the four `addiu $a3,$a3,2` sit in `j` delay slots (DF58 -> .L8003E024, DF68 -> .L8003E028, DF88 -> .L8003E024) and the fourth is inline mid-blend-arm (DFE4). None of the three fills copies its branch target's first insn (those labels start with `addiu $a2,$a2,2` and the loop latch), so on the face of it they are ordinary reorg sinks of each arm's own preceding insn and the target's src refs really are 32 - the s6 contradiction stands. The decoupling that would resolve it has to be a C shape with FEWER `src++` statements pre-RA than `addiu $a3` insns post-reorg; the three-way shared advance tail is the untested shape.

- [s7] Tooling added under tmp/grind/func_8003DE14/s7/: models.py (splice a body into src, run extract.py, print refs/livelen/pri/hardreg/order for the four goal pseudos), tie_test.py and cursor_thresh.py (perturb one model input and re-run simulate.Sim.simulate(overrides=...) - the right way to price an atom that falls between inverse.py's grid points), sbs.py (splice + sandbox + positional side-by-side against build/src/code6cac_c2.o).

- [s7] src/code6cac_c2.c was restored to its committed INCLUDE_ASM state before the session ended; the only tracked changes are the ledger files under memory/grind/func_8003DE14/.

- [s7] goal_from_tgt.py classify code6cac_c2 func_8003DE14 (object-level; the text-stream classifier refuses on a zero-rule function and says so) types the k1 residual as FIRST DIVERGENCE: RA with ours 173 / target 173 insns; the register substitution histogram is $a2->$a3 x8, $v1->$a0 x5, $t5->$t4 x4, $v0->$v1 x4, $a0->$v1 x4, $a3->$a2 x3, $t4->$t5 x3, plus a $v0/$v1/$a0 rotation in the blend arm.

- [s7] The unique pseudo attribution of that histogram is exactly four allocnos: 108 (src) -> $a3, 109 (dst) -> $a2, 115 (j) -> $t4, 116 (complement) -> $t5. Every prior session ran the solver against the first two only.

- [s7] inverse.py global on the k1 model against the full four-pseudo goal: minimal solution 2 atoms / 72 vectors, always one cursor atom crossed with one j/complement atom - the pairs are independent and separately attackable.

- [s7] j and complement both carry reg_n_refs 11, so allocno_compare decides the pair on live_length alone; on an exact tie global.c:653 returns *v1 - *v2 and the lower allocno (115 = j) wins. The validated simulator gives 115=$t4 / 116=$t5 at j livelen 52, 51 and 49, and 115=$t5 at 53. The requirement is ONE insn, not the eight inverse.py's grid samples.

- [s7] j's live length bottoms out at 53 by birth placement: `s32 j = 0;` inside the `if (total > 0)` guard takes it 57 -> 53, and six further forms that move src/dst/factor into the guard ahead of it all measure 53 (scores 44-46, all 173 insns). The remaining insn is loop.c's hoisted complement subu, emitted immediately before the loop start note and therefore always after j's init.

- [s7] Writing complement in the outer body ahead of j costs the count-1 inline property (all four u-forms 65/173; the side-by-side shows `addiu s6,s2,-1` hoisted into the outer preheader at stream position 48 where the target has `li s8,4096`) and renumbers the pseudos so complement becomes 115 - the emitted $t4/$t5 pair is unchanged.

- [s7] `if (j < total)` as the guard buys j refs 11 -> 13 (pri 5789 -> 6842) and does seat j at $t4, but costs 18 points: 61/173.

- [s7] Exact cursor thresholds through the validated simulator: dst livelen 58 -> 38, src livelen 59 -> 90, or dst nrefs 26 -> 32. Both cursors are live across the whole inner loop (block span ~50 insns), so the cursor seat is a reference-count question and nothing else.

- [s7] In the target's stream the four `addiu $a3,$a3,2` are DF58 (delay slot of `j .L8003E024`), DF68 (`j .L8003E028`), DF88 (`j .L8003E024`) and DFE4 (inline in the blend arm); none of the three fills copies its target label's first insn, so the target's pre-RA src refs read as 32, the same as ours.

- [s7] New reusable tooling under tmp/grind/func_8003DE14/s7/: models.py (splice a body into src, run extract.py, print refs/livelen/pri/hardreg/allocation-order for chosen pseudos), tie_test.py and cursor_thresh.py (perturb ONE model input and re-run simulate.Sim.simulate(overrides=...) - the way to price an atom that falls between inverse.py's grid points), sbs.py (splice + sandbox + positional side-by-side against build/src/code6cac_c2.o).

- [s7] src/code6cac_c2.c was restored to its committed INCLUDE_ASM state; the only tracked changes are the ledger files under memory/grind/func_8003DE14/.

## s8 (2026-09-10, forensics) — the cursor seat is a STATEMENT-COUNT question, and reorg cannot manufacture the 173rd instruction

### Chassis re-audit (mandated: floor flat 3 sessions)

Re-measured on HEAD 2026-09-10 with `sweep_variants.py` before any new probe.
No drift, and no FAKE construct exists anywhere in the bank (every banked form is
ordinary C, so `fake_ablate.py` has nothing to ablate — the ablation half of the
re-audit is vacuous here, not skipped):

| form | score | build_insns |
|---|---|---|
| `candidate.c` (s3/d4 lineage) | 31 | 172 |
| `chassis_k8_target_seat_172insn_31.c` | 31 | 172 |
| `chassis_k1_target_blend_naming_43.c` | 43 | 173 |

### reg_n_refs, counted exactly (flow.c:2081)

`reg_n_refs[regno] += loop_depth` — every reference is weighted by the loop depth
of the block holding it, and the per-pixel loop sits at depth 3. That makes the
whole cursor question arithmetic on STATEMENTS:

    src (k1) = (2 reads + 4 `src++` x 2 RTL refs) * 3 + 2 (the outer-loop init) = 32
    dst (k1) = (4 stores + 2 `dst++` x 2 RTL refs) * 3 + 2                       = 26

One `src++` statement inside the inner loop is worth **6 weighted refs**. s7's
`cursor_thresh.py` measured the allocno_compare (global.c:643) threshold as
"src 32 -> 26 or dst 26 -> 32"; in statement terms that is "**exactly three
pre-RA `src++` statements**" or "one extra dst-touching statement" (= k8).

### The three-statement door is REAL and it is ordinary C — chassis q1

`q1` (banked as `chassis_q1_target_seat_refs26_172insn_37.c`) is k1 with both
zero-pixel arms routed through a shared

    advance_src:
        src++;
    advance_dst:
        dst++;
    loop_check:
        j++;

tail, and the blend arm given an explicit `goto advance_dst;` so it does not fall
through the shared increment. Three `src++` statements remain (blend arm, colour
arm, shared tail). Measured:

    k1: src refs=32 live=59 pri=27118 -> $a2 | dst refs=26 live=58 pri=17931 -> $a3
    q1: src refs=26 live=58 pri=17931 -> $a3 | dst refs=26 live=57 pri=18245 -> $a2

**q1 is the first chassis in this grind to reach the target's cursor seat without
k8's extra dst reference.** It scores 37 / 172 — six WORSE than the floor, which
is why the read-out is the disposition, not the score (the s3/s6 lesson again).

Two earlier forms from the same sweep are SEMANTICALLY BROKEN and are banked only
as the measurement that established the rule: `p1`/`p2`/`p3` inserted the shared
`advance_src:` block without giving the blend arm a way past it, so the blend arm
increments src twice (`rejected/s8-p1-shared-src-blend-double-increments-semantics-broken.c`).
Their RA numbers are still valid (the RTL is what it is) and they agree with q1.

### Why k8 is one instruction short — jump.c's find_cross_jump, read not guessed

Instrumented cc1 (`tools/gcc-2.7.2/cc1`) with `BB2_XJUMP_DEBUG=1`, plus the
`.greg` RTL (post-reload, pre-jump2) for k8:

    (insn 160) sh   pixel -> (mem (a2))     (insn 175) sh   s6 -> (mem (a2))
    (insn 161) a2 = a2 + 2                  (insn 176) a2 = a2 + 2
    (insn 164) a3 = a3 + 2                  (insn 179) a3 = a3 + 2
    (jump 166) j 277 (loop_check)           (jump 181) j 277 (loop_check)

Both last-frame arms end in the SAME two insns and jump to the SAME label, so
`find_cross_jump (insn, target, 2, ...)` (jump.c:2020, minimum = 2) matches two
insns and `do_cross_jump` deletes them: 174 pre-jump2 -> **172** emitted.

The TARGET's two arms survive the same pass because their tails match on only ONE
insn: `.L8003DF54` is `sh $v0,0($a2); j; addiu $a3` and `.L8003DF5C` is
`sh $s6,0($a2); addiu $a2,$a2,2; j; addiu $a3` — the `addiu $a3` matches, the next
insn back (`sh $v0` vs `addiu $a2`) does not, and 1 < minimum 2.

### The reorg "steal the shared increment into the delay slot" route is CLOSED

This was s7's frontier-2 mechanism (three `src++` statements pre-RA, a fourth
`addiu $a3` manufactured by reorg's delay-slot fill). reorg.c DOES have that code
— reorg.c:3169-3202, "If there are slots left to fill and our search was stopped
by an unconditional branch, try the insn at the branch target", which literally
does `add_to_delay_list (copy_rtx (next_trial), ...)` and then
`reorg_redirect_jump (trial, new_label)` — i.e. a genuine post-count instruction
copy. But the whole block is gated at **reorg.c:3057-3061**:

    if (slots_filled != slots_to_fill
        && (GET_CODE (insn) != JUMP_INSN
            || ((condjump_p (insn) || condjump_in_parallel_p (insn))
                && ! simplejump_p (insn)
                && JUMP_LABEL (insn) != 0)))

An UNCONDITIONAL jump (`simplejump_p`) never enters it: its delay slot can only be
filled by the backward scan over its own block (reorg.c:2954 onward). Measured
consequence on all three shared-tail forms — every `j advance_src` slot is filled
with that arm's own `sh`, never with the shared `addiu $a3`:

    target : sh $v0,0($a2) / j .L8003E024 / addiu $a3,$a3,2   (3 insns)
    q1     :                 j L          / sh $v0,0($a2)     (2 insns)

So the target's four `addiu $a3` were four pre-RA instructions in its own C, which
puts the target's own src reference count at 32 — the same number k1 has. The s6
"same refs, different seat" contradiction therefore stands, and it is now sharper:
BOTH measured doors to the seat land exactly one instruction short of 173, from
opposite sides (q1: three statements, 172; k8: four statements plus an extra dst
insn, 174 pre-jump2 - 2 cross-jumped = 172).

### Sweep results (all ordinary C, `sandbox --disable all`, HEAD 2026-09-10)

| form | what changed vs k1 | score | insns | src | dst |
|---|---|---|---|---|---|
| p1 | both zero arms share (blend double-increments: BROKEN) | 36 | 170 | refs 26 -> $a3 | -> $a2 |
| p2 | last-frame zero arm shares only | 44 | 170 | refs 32 -> $a2 | -> $a3 |
| p3 | mid-frame zero arm shares only | 44 | 170 | — | — |
| p4 | zero arms + blend share (2 statements) | 43 | 169 | refs 20 -> $t0 | -> $a2 |
| q1 | p1 + blend `goto advance_dst` (correct) | 37 | 172 | refs 26 -> $a3 | -> $a2 |
| q2 | shared block after the latch, jumps back | 37 | 174 | refs 26 -> $a3 | -> $a2 |
| q3 | q1 with the blend arm's `src++` written early | 39 | 172 | refs 26 -> $a3 | -> $a2 |

Note p2/p3: sharing ONE arm leaves the statement count at four (three arms + the
shared tail), so refs stay 32 and the seat does not move — the count that matters
is the number of `src++` STATEMENTS, not the number of arms that share.

### Artifacts

`tmp/grind/func_8003DE14/s8/` — `sbs2.py` (length-tolerant aligned side-by-side vs
the target object; s7's sbs.py assumed equal length and is unusable at 172 vs 173),
`rtl.py` (one-line-per-insn view of one function out of a cc1 -da dump), `dump.sh`
(instrumented-cc1 dump with an arbitrary `BB2_*_DEBUG` env knob), `show.py`,
`mkp.py`/`mkq.py` (form generators), `d_k8/` (full -da dump set + the
`BB2_XJUMP_DEBUG` trace for k8).

- [s8] Chassis re-audit on HEAD 2026-09-10: candidate.c 31/172, chassis_k8 31/172, chassis_k1 43/173 - no drift. No FAKE construct exists in this function's bank, so the fake_ablate half of the re-audit is vacuous rather than skipped.

- [s8] flow.c:2081 is 'reg_n_refs[regno] += loop_depth'; the per-pixel loop is at depth 3, so one in-loop src++ statement = 6 weighted refs. k1's numbers decompose exactly: src (2 reads + 4 incs x 2 RTL refs) x 3 + 2 = 32, dst (4 stores + 2 incs x 2) x 3 + 2 = 26.

- [s8] NEW CHASSIS q1 (memory/grind/func_8003DE14/chassis_q1_target_seat_refs26_172insn_37.c): 37 / 172, ordinary C, src refs 26 -> $a3 and dst -> $a2 - the target's cursor seat WITHOUT k8's duplicated *dst++ statement. The read-out is the disposition, not the score.

- [s8] p2/p3 measured refs 32 with the seat unmoved: routing ONE arm through a shared tail leaves four src++ statements (three arms + the tail). The seat tracks the STATEMENT count, not how many arms share.

- [s8] p1/p2/p3 are semantically broken (the blend arm falls through the inserted shared increment and advances src twice); banked as rejected/s8-p1-shared-src-blend-double-increments-semantics-broken.c. q1 fixes this with an explicit 'goto advance_dst;' in the blend arm.

- [s8] jump.c:2020 find_cross_jump minimum is 2: k8's two last-frame arms share [a2+=2],[a3+=2] before a jump to the same label, so do_cross_jump eats both (174 pre-jump2 -> 172). The target's arms share only [addiu $a3] and survive.

- [s8] reorg.c:3057-3061 gates the entire steal-from-branch-target copy path (reorg.c:3169-3202) on the insn NOT being a simplejump, so an unconditional 'j advance_src' can never copy the shared increment into its delay slot - measured on q1/q2/p1, every such slot holds the arm's own store.

- [s8] Consequence: the target's four addiu $a3 were four pre-RA insns, so the TARGET's own src reference count is 32, identical to k1's. s6's same-refs-different-seat contradiction is unresolved and now sharper - the remaining decoupling channels are passes that DELETE a counted insn (jump2, measured) or CREATE one for a conditional branch (reorg fill_slots_from_thread, reorg.c:3528, untested here).

- [s8] New reusable tooling: tmp/grind/func_8003DE14/s8/sbs2.py (length-tolerant aligned side-by-side - s7's sbs.py assumes equal length and is unusable at 172 vs 173), s8/rtl.py (one-line-per-insn view of one function out of a cc1 -da dump), s8/dump.sh (instrumented-cc1 dump with an arbitrary BB2_*_DEBUG knob).

## s9 (rederive, 2026-09-10)

**Chassis re-audit.** candidate.c 31/172, k8 31/172, q1 37/172, f1 43/173 —
all identical to s8. No drift, no FAKE constructs anywhere on this function, so
`fake_ablate` has nothing to strip and every banked kill stays chassis-valid.

**Fresh m2c decompile (rederive deliverable 1).**
`tmp/grind/func_8003DE14/s9/m2c.c`. m2c reconstructs the arm topology
independently of everything this ledger assumes, and it lands on the f1 chassis
statement for statement: the `i == count - 1` test first, the last-frame zero arm
doing `*dst = pixel; src++; goto advance_dst;`, the last-frame colour arm
INLINING `dst++` and jumping past the shared tail, the blend-zero arm sharing the
tail, the blend arm falling into it. **f1 is the target's emitted shape.** The one
thing m2c shows that f1 does not spell is the second `count - 1` at the bottom of
the loop body; the emitted stream carries `addiu $v0,$s2,-0x1` at BOTH 8003DF34
and 8003E048 with the loop label `.L8003DF38` between them, which is reorg's
steal-and-redirect copy of the loop-top insn into the loop-back branch's delay
slot (reorg.c:3169 `add_to_delay_list (copy_rtx (next_trial))`, reached because
the loop-back insn is a CONDITIONAL branch and so passes the reorg.c:3057-3061
gate), not a source statement. See [[reorg-peel-is-not-a-source-statement]].

**cc1psx self-disproof (rederive deliverable 2).** The s6 contradiction — the
target's own weighted reference counts are src 32 / dst 26, the same as ours, yet
the target seats src at `$a3` while every chassis we build seats it at `$a2` —
has stood unexplained for three sessions. The one premise nobody had tested is
that our decompals cc1 allocates the way PsyQ's cc1psx did. Compiling the f1 body
through `tools/cc1psx_wrapper.sh` gives `addu $6,$sp,16` (src_buf -> `$a2`) and
`addu $7,$sp,1040` (dst_buf -> `$a3`): **the original compiler produces the same
transposed seat from this C.** The contradiction is a property of the C, not of
the toolchain, and the s6/s7/s8 allocator modelling is modelling the right
allocator. Artifact: `tmp/grind/func_8003DE14/s9/f1.psx.s`.

**The instruction-budget identity (new, and the reason both doors are closed).**
Counting the target's own emitted stream:

    src ($a3): 1 set at depth 2 (2) + 2 `lhu` at depth 3 (6) + 4 `addiu` x 2 refs
               at depth 3 (24)                                            = 32
    dst ($a2): 1 set at depth 2 (2) + 4 `sh` at depth 3 (12) + 2 `addiu` x 2 refs
               at depth 3 (12)                                            = 26

`allocno_compare` (global.c:643) then gives src 27118 and dst 17931, so src is
allocated first, and because src and dst carry byte-identical conflict sets
(hard [2,3,4,5,29,64,65,66]) `find_reg`'s pass-1 scan hands the first-allocated
one `$a2`. To flip that, one of these must hold:

  (a) dst reaches 32 weighted refs -> +6 -> one more depth-3 dst statement ->
      pre-RA 174 -> the only pass that can give an instruction back is jump2's
      cross-jump, whose minimum is TWO matching insns (jump.c:2020) -> 172. This
      is exactly k8.
  (b) src drops to 26 weighted refs -> -6 -> one fewer `src++` statement ->
      three emitted `addiu $a3` against the target's four -> 172, unless a pass
      CREATES the fourth after flow. reorg's copy path is the only such pass and
      it is class-killed for unconditional jumps (reorg.c:3059, s8); all three of
      the target's delay-slot `addiu $a3` sit in UNCONDITIONAL `j` slots and are
      backward-scan moves of the arm's own insn, not copies. This is exactly q1.
  (c) live_length moves: dst 58 -> <=38 or src 59 -> >=90. Measured in C this
      session (not just in the model): the largest C-reachable shrink is
      dst 58 -> 53 (declaring `dst` inside the `if (total > 0)` block), because
      both cursors are re-read across the inner loop's back edge and so are live
      over its whole block span. 5 against the 20 needed.
  (d) a blocker allocno takes `$a2` before either cursor. NEW this session, and
      the first route that does not need the priority order to flip: it needs an
      allocno X with pri > 27118 that conflicts with src but NOT dst and that
      cannot reach $v0/$v1/$a0/$a1. c3 produces the first src-only conflicting
      allocno this function has ever shown (pseudo 101), but at pri 11428; and
      for X to be pushed as far down as `$a2` there would have to be five
      simultaneously-live high-priority short-lived pseudos in the outer-loop
      preheader, which the target's seven-insn preheader does not contain.

Doors (a) and (b) are the two the ledger already knew, each landing one
instruction short from opposite sides. (c) and (d) are now measured rather than
inferred. What the identity says is that the seat is fully determined by the
EMITTED stream, which is fixed by the target — so the resolution cannot be a
re-spelling of the same emitted stream. Either one of the four accountings above
is wrong, or the target's pre-RA stream differed from its emitted stream by more
than the single reorg copy we have identified.

**Shape class killed.** Index-addressed cursors (`src_buf[j]`, `dst_buf[j]`, no
pointer variables) measure 56 / 165: loop.c strength-reduces both into single-
update givs, so the four `addiu $a3` the target spreads across its arms cannot
exist. `memory/grind/func_8003DE14/rejected/s9-indexed-no-cursor-givs-165insn-56.c`.

- [s9] Honest floor unchanged at 31 / 172 (memory/grind/func_8003DE14/candidate.c and chassis_k8_target_seat_172insn_31.c); f1/k1 remain 43 / 173; q1 37 / 172.

- [s9] m2c's independent decompile lands on the f1 arm topology exactly, including dst++ inlined on the last-frame COLOUR arm only - f1 IS the target's emitted shape, so no further control-flow rederivation is owed.

- [s9] The emitted target carries addiu $v0,$s2,-0x1 at BOTH 8003DF34 and 8003E048 with the loop label .L8003DF38 between them: reorg's steal-and-redirect copy (reorg.c:3169) into the loop-back CONDITIONAL branch's delay slot. That is the one post-flow instruction-creating event identified in this function, and it creates the comparand, not a cursor increment.

- [s9] cc1psx (the original PsyQ GCC 2.7.2.SN.1) compiles the f1 body to addu $6,$sp,16 / addu $7,$sp,1040 - src in $a2, dst in $a3, the same transposed seat our cc1 produces. The allocator model this ledger has been using is the right allocator.

- [s9] Instruction-budget identity for the target's own stream: src = 1 depth-2 set (2) + 2 lhu (6) + 4 addiu x 2 refs (24) = 32; dst = 1 depth-2 set (2) + 4 sh (12) + 2 addiu x 2 refs (12) = 26. allocno_compare (global.c:643) gives 27118 vs 17931 and find_reg's pass-1 scan hands the first-allocated cursor $a2, because the two carry byte-identical conflict sets.

- [s9] Consequence of the identity: the cursor seat is fully determined by the EMITTED stream. Door (a) dst 26->32 costs a pre-RA insn that only jump2's 2-insn-minimum cross-jump can give back (k8, 172). Door (b) src 32->26 costs the fourth emitted addiu $a3, which only a reorg copy could restore and reorg.c:3059 forecloses that for unconditional jumps (q1, 172). Door (c) live_length is 5 C-reachable against 20 needed. Door (d) an $a2 blocker allocno needs pri > 27118 plus five simultaneously-live preheader pseudos.

- [s9] src and dst hard_conflicts are [2,3,4,5,29,64,65,66] on every chassis measured - $v0,$v1,$a0,$a1,$sp,hi,lo,fake - which is why the first-allocated cursor lands on $a2 and not on a lower argument register.

- [s9] mips.h defines no REG_ALLOC_ORDER, so find_reg's scan is plain ascending hard-reg number: the first-allocated of two allocnos with identical conflicts always takes the lower register.

## s10 (forensics, 2026-09-10) - FLOOR 31 -> 29, and the cursor seat is SOLVED

**Chassis re-audit.** `chassis_f1_structure_exact_43.c` re-measured on HEAD
2026-09-10: 43 / 173, allocation bit-identical to s8/s9 (ALLOCDBG rows below).
No drift. Still no FAKE construct in the ordinary-C bank, so the `fake_ablate`
half of the mandated re-audit remains vacuous rather than skipped.

### The pass-order correction - this is what unblocked the function

`tools/gcc-2.7.2/toplev.c` runs, in this order:
`flow_analysis` (toplev.c:2983) -> `combine_instructions` (toplev.c:3004) ->
`schedule_insns` pass 1 (toplev.c:3033) -> `regclass`/`local_alloc`
(toplev.c:3049-3052) -> `global_alloc` (toplev.c:3080).

Consequences, each measured on the f1 dump set
(`tmp/grind/func_8003DE14/s10/d_f1/`):

- **`reg_n_refs` is counted on the PRE-COMBINE stream.** The f1 `.flow` dump
  holds 140 insns for this function; the `.combine` dump holds 121. **Combine
  deletes 19 insns AFTER the allocator's reference counts have been taken.**
  That is 19 insns of decoupling between "what reg_n_refs saw" and "what the
  target emits", and it is the premise the s8/s9 instruction-budget identity
  did not have: s9 derived the target's reg_n_refs from the target's EMITTED
  asm, which is only sound if combine deleted nothing.
- **`reg_live_length` is NOT a flow output.** `schedule_insns` pass 1
  recomputes it and overwrites flow's value wholesale
  (`sched.c:5106  reg_live_length[regno] = sched_reg_live_length[regno]`).
  The f1 `.sched` dump prints the deltas verbatim for this function:
  `;; register 108 life shortened from 70 to 59` and
  `;; register 109 life shortened from 67 to 58`. So 59/58 are SCHEDULER
  outputs over the post-combine stream, not insn counts of the emitted stream.
- `reg_n_refs` survives combine unchanged for the cursors: combine only zeroes
  a pseudo's refs when its SOLE set was combined away (combine.c:2309-2314 /
  2331-2337), which cannot apply to a multi-set multi-block cursor; and the
  scheduler is barred from moving insns across LOOP_BEG/LOOP_END notes
  precisely so the loop_depth weighting stays correct (sched.c:2076-2079).
  Counted directly in the dumps: `(reg 108)` occurs 11 times in `.flow` and 11
  times in `.combine`; `(reg 109)` 9 and 9.

### The allocator forensics, read not guessed (BB2_FINDREG_DEBUG=108, f1)

    FINDREGDBG pseudo=108 alt=0 acc=0 retry=0
      conflicts:        2 3 4 5 29
      someone_prefers:  (empty)
      used_so_far:      0..15 24..29 31
      pass0_used:       0 1 2 3 4 5 16..23 26..31
      own_copy_prefs:   (empty)
      own_full_prefs:   30
      pass1_used:       0 1 2 3 4 5 26 27 28 29 31   class=1 mode=4 size=1

- src is seated in **pass 0** of find_reg: `regs_used_so_far` is seeded with
  every call-used register (global.c:364-367), so $a2 is a pass-0 candidate,
  and 6 is the lowest hard reg outside src's conflict set. There is no cost
  model - find_reg's scan is plain ascending hard-reg number (mips.h defines
  no REG_ALLOC_ORDER), so **whichever cursor is allocated first takes $a2.**
- `regs_someone_prefers[108]` is EMPTY. The only way to divert src off $a2 in
  pass 0 (global.c:1001) is for a LOWER-priority conflicting allocno to carry
  hard reg 6 in its `hard_reg_full_preferences`, and those bits are created
  only by `set_preference` on a register-to-register copy with a hard reg on
  one side (global.c:1717-1735). In func_8003DE14 hard $a2 is never a copy
  endpoint: the function takes two parameters ($a0,$a1) and every call it makes
  takes at most two arguments (DrawSync 1, StoreImage 2, LoadImage 2,
  func_80052BE4 1). The pass-0 divert route is closed for this call graph.
- NEW, previously unrecorded asymmetry: `own_full_prefs` for src is {30} and
  dst has none. `src_buf` sits at virtual-frame offset 0, so its address
  expands as a plain `(set p (reg 30 $fp))` copy and set_preference fires;
  `dst_buf` expands as `(plus (reg 30) 1024)` and does not. It is inert today
  ($fp is not in `regs_used_so_far`, so pass 0 excludes it and the
  preference-override loop at global.c:1133 clears the bit), but it is a real
  declaration-order-sensitive asymmetry a future session should know about.

### THE FIX: dst's reference count lifted through a combine-folded round-trip

The census tool prices the lift exactly
(`python3 tools/nrefs_census.py --func func_8003DE14 --file code6cac_c2
--above 109:108`): **pseudo 109 must gain +6 weighted refs to rank above 108.**
Because flow weights each reference by loop_depth (flow.c:2081) and the pixel
loop is at depth 3, +6 means two more occurrences of dst inside the inner loop.

Form `r2` = f1 with the shared inner-loop tail changed from a single `dst++`
to `dst++;` followed by a `dst++; dst--;` round-trip.

**Measured: score 29 / build_insns 173 / target_insns 173 - a NEW FLOOR (was
31, and that 31 form was 172 insns, one short).** ALLOCDBG on r2:

    ord=2 pseudo=109 hardreg=6 nrefs=38 livelen=58 pri=32758   (dst -> $a2)
    ord=4 pseudo=108 hardreg=7 nrefs=32 livelen=59 pri=27118   (src -> $a3)

i.e. **the target's cursor seat**, reached for the first time in this grind at
the target's own instruction count. The extra pair contributes 4 occurrences x
depth 3 = +12 (26 -> 38); combine then folds `(dst+2)-2` back to `dst` and the
emitted stream is unchanged at 173 instructions. +6 (refs 32) would have
sufficed arithmetically (160/58 = 27586 > 27118); no ordinary-C spelling that
adds exactly two depth-3 dst occurrences at zero emitted cost is known yet.

The self-assign spelling does NOT work: it is deleted before flow_analysis,
refs stay 26, and the allocation and score are bit-identical to f1 (43 / 173).
Banked as `rejected/s10-dst-self-assign-deleted-before-flow-43.c`.

### What the 29 now is (sbs2.py aligned diff of r2 against the target object)

    ... 70 equal (tgt 0-69)                <- prologue, outer loop, BOTH cursors
    rep  70  move t4,zero        | move t5,zero      <- j / complement swapped
    rep  71  subu t5,s8,t3       | subu t4,s8,t3
    ... 16 equal
    rep  88..123   blend-arm temp naming ($a0/$a1/$v0/$v1) and the mflo/srl
                   interleave: the target runs the BLUE channel's
                   srl/andi/mult before the first mflo, we run it after
    rep 127-133    the loop-bottom lh/lh/mult operand order + the j register
    ... 39 equal (tgt 134-172)

Both cursor halves are byte-exact. The residual is exactly the two sub-problems
s7 already isolated as independent: the j/complement seat pair, and the blend
arm's temp naming/order. **The cursor sub-problem, which consumed s6-s9, is
closed.**

### Artifacts

`tmp/grind/func_8003DE14/s10/` - `apply.py` (apply a body over the
INCLUDE_ASM line), `dump.sh` (instrumented-cc1 -da dump with an arbitrary
BB2_*_DEBUG knob), `nref.sh`, `bank.py`, `r1.c`, `r2.c`, `d_f1/` (full -da dump
set + ALLOCDBG), `d_fr108/` (BB2_FINDREG_DEBUG=108 trace), `d_r2/` (ALLOCDBG
for the new floor).

- [s10] toplev.c:2983 flow_analysis runs BEFORE toplev.c:3004 combine_instructions, so reg_n_refs is counted on the PRE-COMBINE stream; measured on f1, combine deletes 19 of 140 insns in this function. The s8/s9 instruction-budget identity, which derived the target's reg_n_refs from its EMITTED asm, is only valid if combine deleted nothing, and is therefore not a sound foreclosure of doors (a)/(b).
- [s10] sched.c:5106 overwrites reg_live_length with the scheduler's own recomputation; the f1 .sched dump prints ";; register 108 life shortened from 70 to 59" and ";; register 109 life shortened from 67 to 58". The 59/58 that feed allocno_compare are scheduler outputs over the post-combine stream.
- [s10] BB2_FINDREG_DEBUG=108 on f1: conflicts {2,3,4,5,29}, regs_someone_prefers EMPTY, $a2 taken in pass 0 because regs_used_so_far is seeded with every call-used reg (global.c:364-367). find_reg has no cost model and mips.h defines no REG_ALLOC_ORDER, so the first-allocated cursor always takes $a2.
- [s10] hard-reg preferences are created only by set_preference on a reg-to-reg copy with a hard reg on one side (global.c:1717-1735); hard $a2 is never a copy endpoint in func_8003DE14 (2 params, every call <= 2 args), so regs_someone_prefers can never contain bit 6 and the pass-0 divert route is closed here.
- [s10] src's own_full_prefs = {30} and dst's is empty, because src_buf is at virtual-frame offset 0 (plain (set p (reg 30)) copy) while dst_buf expands as (plus (reg 30) 1024). Inert today but declaration-order sensitive.
- [s10] NEW FLOOR 29 / 173 (memory/grind/func_8003DE14/candidate.c): f1 plus a two-statement dst round-trip in the shared inner-loop tail. dst nrefs 26 -> 38, pri 32758 > src 27118, so dst is allocated first and takes $a2 while src takes $a3 - the TARGET's cursor seat, at the target's instruction count.
- [s10] The self-assign spelling of the same lift is deleted before flow_analysis (cse / delete_trivially_dead_insns): refs stay 26 and the score is bit-identical to f1 at 43 / 173. Only a spelling that survives to flow AND is folded by combine lifts the count.
- [s10] tools/nrefs_census.py --above 109:108 prices the lift at exactly +6 weighted refs (two depth-3 occurrences); refs 32 would suffice (160/58 = 27586 > 27118). The +12 the round-trip delivers is more than needed, which leaves room for a cheaper ordinary-C spelling.
- [s10] After the seat flip the residual is entirely the blend block: insns 0-69 and 134-172 of the target are byte-exact. What is left is (b) j/complement seated $t5/$t4 instead of $t4/$t5 and (c) the blend arm temp naming plus the mflo/srl interleave (target does the blue channel's srl/andi/mult before the first mflo).

- [s10] Pass order read from tools/gcc-2.7.2/toplev.c: flow_analysis at 2983, combine_instructions at 3004, schedule_insns pass 1 at 3033, regclass/local_alloc at 3049-3052, global_alloc at 3080. reg_n_refs is therefore counted on the PRE-COMBINE stream, and in this function combine deletes 19 of 140 insns - 19 insns of decoupling that the s8/s9 instruction-budget identity (which read the target's reg_n_refs off its EMITTED asm) did not account for.

- [s10] reg_live_length is overwritten by the scheduler: sched.c:5106 assigns reg_live_length[regno] = sched_reg_live_length[regno], and the f1 .sched dump prints ';; register 108 life shortened from 70 to 59' and ';; register 109 life shortened from 67 to 58'. The 59/58 that feed allocno_compare are scheduler outputs, not emitted-stream insn counts.

- [s10] reg_n_refs is safe from both later passes for these pseudos: combine zeroes a pseudo's refs only when its SOLE set was combined away (combine.c:2309-2314 / 2331-2337), and the scheduler is barred from moving insns across LOOP_BEG/LOOP_END notes precisely so the loop_depth weighting stays valid (sched.c:2076-2079). Counted in the dumps: (reg 108) appears 11 times in .flow and 11 in .combine; (reg 109) 9 and 9.

- [s10] BB2_FINDREG_DEBUG=108 on f1: conflicts {2,3,4,5,29}, regs_someone_prefers EMPTY, own_copy_prefs EMPTY, own_full_prefs {30}, pass0_used excludes 6. src is seated in pass 0 because regs_used_so_far is seeded with every call-used register (global.c:364-367); find_reg has no cost model and mips.h defines no REG_ALLOC_ORDER, so the first-allocated cursor always takes $a2.

- [s10] New asymmetry: src's hard_reg_full_preferences is {30} ($fp) while dst's is empty, because src_buf sits at virtual-frame offset 0 and expands as a plain (set p (reg 30)) copy that set_preference fires on, whereas dst_buf expands as (plus (reg 30) 1024). Inert today but sensitive to local declaration order.

- [s10] tools/nrefs_census.py --func func_8003DE14 --file code6cac_c2 --above 109:108 prices the flip at exactly +6 weighted refs on pseudo 109 (two depth-3 occurrences); refs 32 suffices arithmetically (160/58 = 27586 > 27118). The banked candidate delivers +12, so there is headroom for a cheaper spelling.

- [s10] NEW FLOOR 29 / 173 (memory/grind/func_8003DE14/candidate.c), with ALLOCDBG confirming dst -> $a2 (nrefs 38, pri 32758) and src -> $a3 (nrefs 32, pri 27118). The previous best was 31 at 172 insns; the ordinary-C best remains 31 and is preserved as memory/grind/func_8003DE14/chassis_s3d4_ordinary_c_31.c.

- [s10] sbs2.py aligned diff of the new candidate against the target object: target insns 0-69 and 134-172 are byte-exact. The whole residual is the blend block - the j/complement pair seated $t5/$t4 where the target has $t4/$t5, plus the blend arm's temp naming ($a0/$a1/$v0/$v1) and the mflo/srl interleave (the target runs the blue channel's srl/andi/mult before the first mflo; we run it after).

- [s10] src/code6cac_c2.c was restored to HEAD (INCLUDE_ASM) before the session ended; the only working-tree changes are under memory/grind/func_8003DE14/ and tmp/.

## s11 (rederive) — the reference lift has an ordinary-C spelling; the j/complement seat is priced to a single insn

### THE HEADLINE: the s10 FAKE round-trip is retired

`memory/grind/func_8003DE14/candidate.c` is now form `a1`
(`tmp/grind/func_8003DE14/s11/a1.c`, also banked as
`chassis_a1_ordinary_arms_29.c`): the f1 chassis with the shared
`advance_dst: dst++;` label REPLACED by an arm-local `dst++` in each of the
three inner-loop arms. No label, no goto to it, no net-zero statement pair.

    score 29 / build_insns 173 / target_insns 173   (identical to the s10 form)
    ALLOCDBG (tmp/grind/func_8003DE14/s11/d_a1/stderr.log):
      ord=3 pseudo=109 hardreg=6 nrefs=38 livelen=60 pri=31666   (dst -> $a2)
      ord=4 pseudo=108 hardreg=7 nrefs=32 livelen=61 pri=26229   (src -> $a3)

Mechanism, measured not guessed: flow.c:2081 weights each reference by
loop_depth and the pixel loop is depth 3, so ONE `dst++` is 2 occurrences x 3 =
6 weighted refs. One shared copy (6) becomes three arm-local copies (18), i.e.
dst's weighted `reg_n_refs` goes 26 -> 38 — bit-for-bit the count the s10
`dst++; dst--;` round-trip produced. jump2's cross-jump then tail-merges the
three copies back (jump.c:2020 needs >= 2 matching insns; the merged tail here
is `addiu dst,1 / addiu j,1 / slt / bne`, so it qualifies) and the emitted
stream stays at 173 insns. This is the sanctioned duplicated-statement-into-arms
shape (.claude/rules/duplicated-statement-into-arms.md), not the dead-store
family the round-trip fell into.

The obvious compression of the same body is NOT equivalent: writing the zero
arms as `*dst++ = pixel;` instead of `*dst = pixel; src++; dst++;` folds two
insns away — 171 insns, score 35
(`rejected/s11-star-dst-plusplus-in-zero-arms-folds-two-insns-35.c`). The split
store/increment is load-bearing.

### The j/complement seat, priced exactly

`allocno_compare` (global.c:635-648) is
`pri = floor_log2(nrefs) * nrefs / live_length * 10000 * size`, and the
tie-break at global.c:652-653 is `*v1 - *v2` — ascending allocno, i.e. ascending
pseudo number. j is pseudo 115 and complement is pseudo 116, so **a TIE seats j
first, which is the target's order.**

Measured across every form tried this session, j and complement ALWAYS carry the
same `nrefs` (11) and j's `live_length` is ALWAYS 1..5 greater than
complement's:

    form          j (115)          complement (116)     seat
    a1            11 / 59  5593    11 / 54  6111        wrong ($t5/$t4)
    b1            11 / 56  5892    11 / 55  6000        wrong, score 45
    C1            11 / 55  6000    11 / 54  6111        wrong, score 31

nrefs 11 decomposes as: set at depth 2 (weight 2) + `j++` read/write at depth 3
(6) + trip test at depth 3 (3) for j; set at depth 2 (2) + three multiplies at
depth 3 (9) for complement. Both are pinned there by the arithmetic, and every
single-step change is a multiple of 3 (depth-3) or 2 (preheader):
  - j nrefs 12 -> 3*12/59 = 6101, still BELOW complement's 6111 (10 short);
  - j nrefs 13 -> 6610, ABOVE — needs exactly +2 weighted, i.e. one preheader
    occurrence, and no ordinary-C preheader reference of j is known that is not
    a dead store;
  - j nrefs 14 (one more depth-3 occurrence) -> 7118, above;
  - complement nrefs 10 -> 5555, below j — needs -1 weighted, which no
    depth-multiple can deliver;
  - live_length: j needs to REACH complement's, i.e. lose one more insn than C1
    already did, or complement needs to gain 5 over a1.

So the whole sub-problem is now a **one-insn live-length gap or a +2 weighted
reference**, and it is arithmetic, not guesswork.

### What was measured and did not move it

  - complement hoisted out of the inner loop in C (b1/b2): LICM had already
    hoisted it, so nrefs stays 11; live lengths become 56/55 (gap 1, still
    wrong) and the score REGRESSES to 45 because the preheader arithmetic moves.
    `rejected/s11-complement-hoisted-out-of-inner-loop-45.c`.
  - `s32 j;` at the top with `j = 0;` moved inside `if (total > 0)` (C1):
    j live_length 59 -> 55, gap down to ONE insn, but the score regresses to 31
    (the preheader `move t4,zero` shifts).
    `rejected/s11-j-init-inside-if-livelen-55-still-one-short-31.c`.
  - C1 + `while (++j < ...)` (D2): 31. `while (++j < ...)` alone (C3): 29, seat
    unchanged.
  - j declared first among the outer-body locals (C4) / C4+C1 (C6): 31.
  - j reused as the outer tail's `new_y` carrier (D1): 41 / 172 insns.
    `rejected/s11-j-reused-as-new-y-carrier-outer-tail-41.c`.
  - `j++` duplicated into the three arms the way `dst++` now is (D3): cross-jump
    does NOT re-merge (the arms diverge above the increment) — 175 insns,
    score 47. `rejected/s11-j-increment-duplicated-into-arms-175-insns-47.c`.
  - `blend_base` literalised to 0x1000 (C5): 29, neutral.
  - complement declared in the outer body and assigned inside the loop (D4): 29,
    neutral. Multiplies written `complement * x_src` (D6): 29, neutral.

### The blend block is not a source-shape question at this level

Twelve structurally distinct spellings of the blend arm were swept on the NEW
(correct-cursor-seat) 29 chassis — inlined extractions, staged products,
channel reorder (b-first, g-first, b-computed-first), an OR accumulator, `pixel`
vs `px` in the red extraction, a pre-masked `b_ch`, re-associated OR, and
swapped `ch*factor + x_src*complement` operand order
(`tmp/grind/func_8003DE14/s11/bl/`, sweep_variants). **Best = 29, i.e. nothing
beat the incumbent; four spellings tie at 29 and the rest are 30-53.** This
re-confirms s5's negative sweep on the corrected-seat chassis: the blend
residual (target 88-123, the $a0/$a1/$v0/$v1/$t7 naming plus the mflo/srl
interleave) does not move under source-level reshaping of that block and is a
scheduler / allocation-order question.

### s11 facts

- [s11] The +12 weighted-reference lift on the dst pseudo that seats the cursors like the target has an ORDINARY-C spelling: `dst++` duplicated into each of the three inner-loop arms in place of the shared `advance_dst:` label. Score 29 / 173 insns, ALLOCDBG dst(109) -> $a2 nrefs 38 and src(108) -> $a3 nrefs 32 — identical seat and identical instruction count to the s10 `dst++; dst--;` round-trip, with no net-zero statement anywhere in the body. candidate.c is now this form.
- [s11] jump2's cross-jump re-merges the three arm-local `dst++` tails (the merged tail is 4 matching insns, well over the jump.c:2020 minimum of 2), which is why the reference lift is free of emitted cost. The same duplication applied to `j++` (D3) does NOT re-merge — the arms diverge above the increment — and costs 2 insns (175, score 47).
- [s11] `*dst++ = pixel;` in the zero-pixel arms is NOT a neutral rewrite of `*dst = pixel; src++; dst++;`: it folds two insns away (171 insns, score 35). The split store/increment is load-bearing for the 173-insn budget.
- [s11] allocno_compare (global.c:635-648) is pri = floor_log2(nrefs)*nrefs/live_length*10000*size, and its tie-break (global.c:652-653) is ascending allocno, i.e. ascending pseudo number. j is pseudo 115 and complement is pseudo 116, so an exact TIE on (nrefs, live_length) seats j first — which is the target's $t4/$t5 order. The sub-problem does not need j to WIN, only to TIE.
- [s11] j and complement carry the same nrefs (11) on every form measured, and j's live_length is always 1..5 longer than complement's: a1 59/54, b1 56/55, C1 55/54. The C1 spelling (`s32 j;` outer, `j = 0;` inside `if (total > 0)`) closes the gap to ONE insn but costs 2 score elsewhere.
- [s11] The reference arithmetic for the j/complement flip is exact: j at nrefs 12 gives 6101 and still loses to complement's 6111; j at 13 (one preheader occurrence, +2 weighted) gives 6610 and wins; j at 14 (one depth-3 occurrence) gives 7118 and wins; complement at 10 gives 5555 and loses. Every depth-3 change is a multiple of 3 and every preheader change a multiple of 2, so the reachable lifts are +2, +3, +6, ... — +2 is reachable in principle but no ordinary-C preheader reference of j is known that is not a dead store.
- [s11] Hoisting `complement` out of the inner loop in C does not change its reg_n_refs (LICM had already hoisted the set into the preheader; nrefs stays 11) and regresses the score to 45 by moving the preheader arithmetic.
- [s11] Twelve structurally distinct blend-arm spellings swept on the corrected-seat 29 chassis (tmp/grind/func_8003DE14/s11/bl/): best 29, i.e. none beat the incumbent. This re-confirms s5's negative blend sweep on the new chassis — the blend residual is a scheduler/allocation-order question, not a source-shape one.
- [s11] src/code6cac_c2.c was restored to HEAD (INCLUDE_ASM) before the session ended; the only working-tree changes are under memory/grind/func_8003DE14/ and tmp/.

- [s11] The floor stays at 29 / 173 insns but the FORM changed: memory/grind/func_8003DE14/candidate.c is now form a1, which carries no net-zero statement pair, no dead store and no FAKE-family construct. The s10 candidate's dst++/dst-- round-trip is retired and banked knowledge only.

- [s11] The construct that replaces it is one real statement (dst++) written into each of the three inner-loop arms instead of a shared advance_dst: label - the duplicated-statement-into-arms shape described verbatim in .claude/rules/duplicated-statement-into-arms.md, including its 'cross-jump re-merges the copies to identical bytes and the effect is a reg_n_refs priority lift' clause. A submitting session must still read that rule end-to-end for its annotation prerequisites.

- [s11] ALLOCDBG on a1 (tmp/grind/func_8003DE14/s11/d_a1/stderr.log): ord=3 pseudo=109 hardreg=6 nrefs=38 livelen=60 pri=31666 (dst -> $a2); ord=4 pseudo=108 hardreg=7 nrefs=32 livelen=61 pri=26229 (src -> $a3). The target's cursor seat at the target's own instruction count, from ordinary C.

- [s11] allocno_compare is pri = floor_log2(nrefs) * nrefs / live_length * 10000 * size (tools/gcc-2.7.2/global.c:635-648) and its tie-break is *v1 - *v2, i.e. ascending allocno = ascending pseudo number (global.c:652-653). j is pseudo 115 and complement is pseudo 116, so an exact TIE on (nrefs, live_length) seats j first - which is the target's $t4/$t5 order. The j/complement sub-problem does not need j to win, only to tie.

- [s11] j and complement carry identical nrefs (11) on every form measured this session, and j's live_length is always 1..5 longer than complement's: a1 59/54 (pri 5593 vs 6111), b1 56/55 (5892 vs 6000), C1 55/54 (6000 vs 6111). nrefs 11 decomposes as set-at-depth-2 (2) + j++ read/write at depth 3 (6) + trip test at depth 3 (3) for j, and set-at-depth-2 (2) + three multiplies at depth 3 (9) for complement.

- [s11] The reference arithmetic for the j/complement flip is exact: j at nrefs 12 gives 6101 and STILL loses to complement's 6111; j at 13 (one preheader occurrence, +2 weighted) gives 6610 and wins; j at 14 (one depth-3 occurrence) gives 7118 and wins; complement at 10 gives 5555 and loses. Every depth-3 change is a multiple of 3 and every preheader change a multiple of 2, so the reachable lifts are +2, +3, +6, ...

- [s11] jump2's cross-jump re-merges the three arm-local dst++ tails (four matching insns against the two-insn minimum at jump.c:2020), which is exactly why the reference lift is free of emitted cost. The same duplication applied to j++ does NOT re-merge - the arms diverge above the increment - and costs two insns (175 build_insns, score 47).

- [s11] *dst++ = pixel; in the zero-pixel arms folds two insns away (171 build_insns, score 35). The split *dst = pixel; ... dst++; is load-bearing for the 173-insn budget.

- [s11] Hoisting complement out of the inner loop in C does not change its reg_n_refs (LICM had already hoisted the set; nrefs stays 11) and regresses the score to 45 by moving the preheader arithmetic.

- [s11] Twelve structurally distinct blend-arm spellings on the corrected-seat 29 chassis all score 29 or worse - four tie at 29, the rest are 30-53. Combined with s5's earlier negative sweep on the old chassis, the blend residual is not a source-shape question at this level.

- [s11] src/code6cac_c2.c was restored to HEAD (INCLUDE_ASM) before the session ended; the only working-tree changes are under memory/grind/func_8003DE14/ and tmp/.

## s12 (structural) - floor 29 -> 28; the j/complement seat is SOLVED

**The one-line change.** The s11 body wrote `s32 j = 0;` in the outer do-body and
`s32 complement = blend_base - factor;` as the first statement of the pixel
loop. The s12 body writes both inside the guard, complement first:

```c
if (total > 0) {
    s32 complement = blend_base - factor;
    s32 j = 0;
    do { ... } while (j < rect[2] * rect[3]);
}
```

Score 29 -> 28 at 173 build_insns / 173 target_insns, and target insns 70/71,
131 and 133 become byte-exact: j is seated in $t4 and complement in $t5, the
target's assignment. `memory/grind/func_8003DE14/candidate.c` is this body.

**The model that predicted it (and that closes the axis).** j (pseudo 115) and
complement (pseudo 116) both carry weighted nrefs 11, so allocno_compare
(tools/gcc-2.7.2/global.c:635-648) collapses to `pri = 33/live_length`, with
global.c:652-653 breaking an exact tie on ascending allocno number. Both pseudos
are set only in the inner loop's preheader and read on every iteration, so each
is live across the WHOLE loop body; the loop body contributes the same constant
to both live lengths and the only variable is the distance between their two set
insns in the preheader. That is why s5's and s11's large blend/ordering sweeps
could never move this pair: nothing inside the loop body is capable of moving
either number.

loop.c's scan_loop appends a hoisted invariant at the END of the preheader (just
before loop_start), so a LICM-hoisted `complement` set always lands BELOW
`j = 0` and complement always wins the race. Writing the subtraction inside the
guard removes it from LICM's hands - it is already in the block LICM would have
put it in - and lets ordinary statement order place `j = 0` below it.

**Measured live lengths (BB2_ALLOC_DEBUG=1 on tools/gcc-2.7.2/cc1, dumps in
tmp/grind/func_8003DE14/s12/d_*/stderr.log):**

| form | placement | j | complement | seat |
|---|---|---|---|---|
| base (s11) | complement in loop body, `j = 0` in outer body | 59 | 54 | complement $t4 - WRONG |
| W6 | complement inside the guard, `j = 0` still outside | 59 | 54 | WRONG |
| H2 | `j = 0` then complement, both inside the guard | 55 | 54 | WRONG (31) |
| W2 | complement inside the guard, then `j = 0` (j declared outer) | 54 | 55 | **j $t4 - RIGHT (28)** |
| W5 | both declared inside the guard, complement first | 55 | 54* | **RIGHT (28)** |
| W7 | as W2 with `s32 j;` at the top of the do-body | 54 | 55 | **RIGHT (28)** |
| W9 | complement in the outer body above `j = 0` | 55 | 56 | RIGHT but 45 |

(*W5 swaps the pseudo NUMBERS too - complement becomes 115 and j 116 - so its
rows read complement 54 / j 55 by pseudo; the seat is the same as W2/W7.)

**The coupling that bounds the head residual.** reorg.c fills the guard `blez`'s
delay slot with the closest movable insn preceding the branch. In the s11 body
that was `move t4,zero`, which is exactly what the target emits at insn 70.
Moving `j = 0` below the branch - the move that flips the seat - leaves
`addiu a2,sp,1040` (the `dst = dst_buf` init the target emits at insn 54) as the
closest candidate, so reorg sinks it into the slot. The whole four-insn head
difference is that exchange, and it is directly coupled to the seat: H2 buys the
delay slot back and loses the seat, scoring 31. H1 (dst declared before src) and
H3 (total computed last) both stay at 28, so neither denying reorg the `addiu a2`
candidate nor moving the guard's operand computation recovers the slot.

**Residual at 28 (tmp/grind/func_8003DE14/s12/sbs.sh aligned diff).** Target
insns 0-53, 55-69, 72-87, 90-93, 96-98, 102-103, 108-110, 113, 120-121, 124-126,
129 and 131-172 are byte-exact. What is left:

  (a) head, 4 insns: target `addiu a2,sp,1040` at 54 and `move t4,zero` in the
      delay slot at 70, against our `addiu a2,sp,1040` in the slot at 70 and
      `subu t5,s8,t3` at 71 ahead of `move t4,zero`;
  (b) the blend arm, target 88-123: register naming (target holds px in $a0, the
      red product in $a1 and the blue product in $t7 where we use $v1/$a1/$v1)
      plus the blue channel's `srl/andi/mult` at target 104-107 against ours at
      109-112. Twelve source reshapings re-measured on THIS chassis (B1w..B12w)
      all score 28 or worse, so the blend residual is not seat-dependent and is
      still a scheduler/allocation-order question;
  (c) the trip test, target 127-130: `lh v0,4(s0) / lh v1,6(s0) / mult v0,v1`
      against our `lh v1,4(s0) / lh v0,6(s0) / mult v1,v0` - the same two loads
      in the same order with the two short-lived pseudos named the other way
      round. This is a NEW, small, previously-invisible residual: it only became
      readable once the counter seat stopped dominating the diff.

**Target head, for the next session (objdump insn index):** 49 `lh v1,4(s0)`,
50 `lh v0,6(s0)`, 52 `mult v1,v0`, 53 `addiu a3,sp,16` (src), 54 `addiu a2,sp,1040`
(dst), 55 `mflo v1` (total), 56-68 the `((i+1)<<12)/count` divide ending
`mflo t3`, 69 `blez v1` with 70 `move t4,zero` in the slot, 71 `subu t5,s8,t3`,
72-73 the `i == count - 1` test.

**Trip-test cluster (c) probed and closed for source spelling (s12).** Three
spellings of `while (j < rect[2] * rect[3])` - operands swapped, comparison
reversed, operands explicitly widened - all score 28 and none changes the
$v0/$v1 naming on the two `lh` destinations. T1 (`rect[3] * rect[2]`) does change
the LOAD order, and moves it away from the target's. The two rows immediately
above it (target 122 `or v0,v0,v1` / 123 `andi v1,a0,K` against our
`or v0,v0,a0` / `andi v1,v1,K`) are identical across all three variants, so this
cluster reads as a downstream consequence of the blend arm's $v0/$v1 pressure
rather than an independent sub-problem: fix the blend naming first.

- [s12] FLOOR 29 -> 28 on HEAD 2026-09-10, 173 build_insns / 173 target_insns. The whole change is where two locals are declared: `if (total > 0) { s32 complement = blend_base - factor; s32 j = 0; do { ... } }` instead of `s32 j = 0;` in the outer do-body with the subtraction written inside the loop. Ordinary C; no FAKE-family construct anywhere in the body.

- [s12] The j/complement pair is no longer a residual: j is seated in $t4 and complement in $t5, the target's assignment, and target insns 70/71, 131 and 133 are byte-exact.

- [s12] Structural model, now measured rather than inferred: two pseudos that are set only in a loop's preheader and read every iteration are both live over the whole loop body, so the body contributes an identical constant to both live lengths and ONLY the preheader distance between their set insns can move their allocno priorities. This is why every previous in-loop sweep (s5's 4,488 spellings, s11's twelve blend bodies) was structurally incapable of moving this pair.

- [s12] loop.c's scan_loop appends a hoisted invariant at the END of the preheader, immediately before loop_start, so a LICM-hoisted set is always below anything the C wrote in the preheader. Writing a loop invariant explicitly inside the loop's guard is therefore the lever that puts it ABOVE an initialisation GCC would otherwise place first - a reusable structural technique, not a func_8003DE14 quirk.

- [s12] Measured live lengths (BB2_ALLOC_DEBUG=1, tools/gcc-2.7.2/cc1) for j/complement across seven placements: base 59/54, W6 59/54, H2 55/54, W2 54/55, W5 55/54 (pseudo numbers swapped by declaration order), W7 54/55, W9 55/56. Only W2/W5/W7/W9 flip the seat; only W2/W5/W7 do it for free.

- [s12] Residual at 28 is three clusters: (a) 4 insns in the head - the target emits `addiu a2,sp,1040` at 54 and fills the `blez` delay slot at 70 with `move t4,zero`, we sink `addiu a2` into the slot and emit `subu t5,s8,t3` ahead of `move t4,zero`; (b) the blend arm, target 88-123, register naming ($a0/$a1/$t7 vs our $v1/$a1/$v1) plus the blue channel's srl/andi/mult at 104-107 vs our 109-112; (c) a NEW small one at target 127-130, `lh v0,4(s0) / lh v1,6(s0) / mult v0,v1` against our `lh v1 / lh v0 / mult v1,v0` - same loads, same order, the two short-lived pseudos named the other way round.

- [s12] (a) is directly coupled to the seat and priced: H2 (j declared before complement inside the guard) buys the delay slot back for 3 score and loses the seat. H1 (dst before src) and H3 (total computed last) hold the seat at 28 without recovering the slot.

- [s12] Equivalent 28-scoring spellings this session: W2, W5, W7, H1, H3, and four of the rebased blend bodies (B1w, B3w, B7w, B8w). The floor is a plateau of equivalent placements, not a single fragile form.

- [s12] Target head for the next session (objdump insn index): 49 `lh v1,4(s0)`, 50 `lh v0,6(s0)`, 52 `mult v1,v0`, 53 `addiu a3,sp,16` (src), 54 `addiu a2,sp,1040` (dst), 55 `mflo v1` (total), 56-68 the `((i+1)<<12)/count` divide ending `mflo t3`, 69 `blez v1` with 70 `move t4,zero` in the slot, 71 `subu t5,s8,t3`, 72-73 the `i == count - 1` test.

## s13 (structural) — floor 28 -> 26

Chassis at dispatch: HEAD 2026-09-10, s12 candidate (W5) re-measured **28 / 173
insns / 173 target insns** — unchanged, so every s12 conclusion is still
chassis-valid.

### E1. The blend arm has NO scheduling residual — s12's frontier item was a difflib artifact
`tmp/grind/func_8003DE14/s13/sxs.py` prints a raw index-by-index side-by-side of
the target object stream against the sandbox object stream (no SequenceMatcher).
On the s12 chassis, target[84..131] and ours[84..131] carry **identical opcodes
in identical slots**; the only differences are register names. The s12 frontier
entry "the target runs the blue channel's srl/andi/mult before its first mflo
(104-107) where we run it after (109-112)" came from `sbs2.py`'s difflib
alignment inserting a del/ins block around a pure rename, and is WRONG. There is
nothing in the blend arm for `tools/sched_solver` to solve; the whole blend
residual is register allocation.

### E2. A C local is one pseudo — reusing the src variable for the factor product fixes the r channel
Target insns 94/95/98 are `sll a1,v0,0x3` / `mult a1,t5` / `mflo a1`: ONE hard
register ($a1) carries both the shifted r component and the `r * factor`
product. The s12 body spelled those as two distinct expressions inside one
statement, so they became two pseudos and landed in $v0 and $a1. GCC 2.7.2 gives
a non-address-taken C local exactly one pseudo for its whole scope, so writing
both values into one C variable forces them to share a hard register:

    s32 rp;
    rp    = r_src * complement;
    r_src = r * factor;            /* same variable, second value */
    r_ch  = ((rp + r_src) >> 15) & 0x1F;

That is form **C2**, measured **26** (173 insns), rows 94/95 now byte-exact.
This is ordinary C: `rp` is once-written/once-read and its value is consumed in
the target's bytes, and `r_src` carries two real, consecutively-live values —
no dead code, no annotation-bearing construct.

The reuse only pays when it is spelled per channel with the *complement product
named*. Every neighbouring spelling is worse (all 173 insns, all this chassis):

| form | shape | score |
|---|---|---|
| C1 | C2 but the three sums folded inline into the final `or` chain | 43 |
| C2 | per-channel `rp/gp/bp` + `X_src` reused for `X*factor` | **26** |
| C3 | `r_src *= complement;` (self-multiply, keep the factor product inline) | 44 |
| C4 | r,g reuse for the factor product; b self-multiplies | 40 |
| C5 | full accumulate `r_src = r_src*complement + r*factor;` | 38 |
| C6 | C3 with `src++` after the multiplies | 44 |
| C7 | the `X*factor` products named first, src vars left alone | 54 |
| F1 | C2 but the b channel self-multiplies (target's $a0 shape) | 40 |
| F2 | C2 but the g channel self-multiplies | 27 |
| F3 | C2 with the sums folded into the `or` chain | 43 |
| F4 | C2 with g_src/b_src declared lazily at first use | 26 |
| F5 | factor product first, then `X_src *= complement` | 59 |

### E3. `px` as a user variable vs a CSE temp is byte-neutral
Hypothesis: the target's zero-extended pixel is a compiler temp, not a user
variable, and that is why it seats differently. **Disproved.** Form **D1**
deletes `s32 px = pixel & 0xFFFF;` entirely, tests `if (pixel == 0)` and writes
`(pixel >> 2) & 0xF8` / `(pixel >> 7) & 0xF8` on the u16 directly — the
zero-extend then exists only as a CSE temp. D1 measures **26** and its aligned
diff is row-for-row IDENTICAL to C2's. D5 (same with explicit `(u32)` casts) is
also 26. So `px`'s pseudo class does not move its seat.

Two spellings that change px's reference count DO move it, both the wrong way:
`(px & 0x1F) << 3` for the r extraction (D3) = 27, `px & 0x8000` for the alpha
mask (D2) = 40 **and 174 insns**.

### E4. The head residual is a fall-through delay-slot fill, not a backward steal
Raw side-by-side of insns 48-80 (sxs.py 48 80):

    target                              ours
     53 addiu a3,sp,16   (src=src_buf)   53 addiu a3,sp,16
     54 addiu a2,sp,1040 (dst=dst_buf)   -- (absent)
     ...  div/mflo sequence identical ...
     69 blez v1,...                      68 blez v1,...
     70 move t4,zero     <- delay slot   69 addiu a2,sp,1040  <- delay slot
     71 subu t5,s8,t3                    70 subu t5,s8,t3
                                         71 move t4,zero

So in the target `dst = dst_buf` STAYS at 54 and the slot is filled with `j = 0`
from the fall-through side of the branch, with `complement` (the subu) landing
below it. Our build hands reorg `addiu a2,sp,1040` instead. The two facts are in
tension with the j/complement seat: for `j` to keep $t4 its set must be BELOW
complement's set (s12's mechanism, global.c:635-653 — equal nrefs, so the
shorter live length wins), but for reorg to fill the slot with `move t4,zero`
the j = 0 store must be the FIRST insn of the fall-through thread, i.e. ABOVE
complement's set. Six statement-order spellings were measured (all 173 insns):

| form | shape | score |
|---|---|---|
| E1 | `j = 0;` first, `complement` second, both inside the guard | 30 |
| E2 | C2 + `dst = dst_buf;` moved inside the guard | 28 |
| E3 | E1 + `dst = dst_buf;` inside the guard | 32 |
| E4 | `total` computed after src/dst/factor | 26 |
| E5 | dst declared before src | 26 |
| E6 | `total` deleted; guard written `if (rect[2] * rect[3] > 0)` | 26 |

E4/E5/E6 are byte-identical to C2 — moving or deleting `total` and swapping the
src/dst declaration order does not change which insn reorg picks. E2 (denying
reorg the `addiu a2` candidate by sinking dst into the guard) costs 2 rather
than gaining: the slot then takes something else.

### E5. Pass attribution for the surviving blend residual
`pwsh`-equivalent dump of the s12 chassis (`tmp/grind/func_8003DE14/s13/d_base/`,
instrumented `tools/gcc-2.7.2/cc1`, BB2_ALLOC_DEBUG=1). `px` is pseudo 122, a
**global** allocno (it is live across the `bnez` at 89), `nrefs 12 / livelen 11 /
pri 32727`, allocated 3rd (ord=2) and taking hardreg 3 = `$v1`. The target has it
in `$a0`. ord=0 (pseudo 118) takes `$v0`, ord=1 is a special reg, so px simply
takes the lowest register still free. Note that the other global allocno holding
hardreg 3 is pseudo 101 (ord=10) — it does NOT conflict with px, so re-ordering
those two cannot change px's seat. For px to land on `$a0`, some allocno that
CONFLICTS with px must hold `$v1` before px is seated; in the target that role is
played by the g-channel value (target rows 100/104 both write `$v1`).

- [s13] Chassis re-measured at dispatch: the s12 candidate (W5) still scores 28 / 173 insns / 173 target insns on HEAD 2026-09-10, so all s12 conclusions remain chassis-valid.

- [s13] New floor 26, form C2, saved to memory/grind/func_8003DE14/candidate.c with a full header. Ordinary C: the only change from s12 is that the three channel sums are staged through named per-channel locals (rp/gp/bp) and each X_src local is reused to hold that channel's X*factor product. No annotation-bearing construct anywhere in the body.

- [s13] The blend arm carries ZERO scheduling residual: target[84..131] and ours[84..131] are opcode-identical, only register names differ (tmp/grind/func_8003DE14/s13/sxs.py). s12 frontier item 3 (feed the blend block to tools/sched_solver) is retired as based on a difflib alignment artifact.

- [s13] The whole surviving 19-insn blend residual cascades from ONE seat: the zero-extended pixel (px, pseudo 122) is a GLOBAL allocno with nrefs 12 / livelen 11 / pri 32727, allocated 3rd (ord=2) and taking hardreg 3 = $v1 because ord=0 took $v0 and ord=1 is a special register; the target has px in $a0. Dump: tmp/grind/func_8003DE14/s13/d_base/stderr.log (instrumented tools/gcc-2.7.2/cc1, BB2_ALLOC_DEBUG=1).

- [s13] The other global allocno holding hardreg 3 in our build is pseudo 101 (ord=10, nrefs 4 / livelen 7) and it does NOT conflict with px, so reordering those two allocnos cannot move px's seat. For px to reach $a0 an allocno that CONFLICTS with px must hold $v1 before px is seated - in the target that role is played by the g-channel value, which occupies $v1 at target rows 100 and 104.

- [s13] Head residual re-derived from a raw side-by-side of insns 48-80: the target's slot fill comes from the FALL-THROUGH side of the blez (move t4,zero = j = 0) with dst = dst_buf left in place at insn 54, not from a backward steal. Our build's slot fill (addiu a2,sp,1040) is a backward steal of dst = dst_buf.

- [s13] The j/complement seat and the delay slot are provably in tension on this chassis: j needs its set BELOW complement's to win $t4 on live length (s12's mechanism), while the delay slot needs j = 0 to be the first insn of the fall-through thread, i.e. ABOVE complement's set. Six spellings measured; none satisfies both.

- [s13] Every form measured this session held build_insns == 173 except D2 (px used for the 0x8000 alpha mask), which inflated to 174.

## s14 (structural) — the px seat has an exact priority arithmetic

Chassis re-measured at dispatch: the s13 candidate applied to src/code6cac_c2.c
scores 26 at build_insns 173 / target_insns 173 on HEAD 2026-09-10. Floor
unchanged this session.

### The allocation table (tmp/grind/func_8003DE14/s14/d_aac/stderr.log)
`BB2_ALLOC_DEBUG=1` on the candidate chassis prints, for func_8003DE14:

| ord | pseudo | what it is | hardreg | nrefs | livelen | pri |
|-----|--------|-----------|---------|-------|---------|-----|
| 0 | 118 | `count - 1` | 2 ($v0) | 9 | 3 | 90000 |
| 1 | 160 | LO (mult result) | 65 | 6 | 3 | 40000 |
| 2 | 122 | **px** (zero-extended pixel) | 3 ($v1) | 12 | 11 | 32727 |
| 3 | 109 | dst cursor | 6 ($a2) | 38 | 60 | 31666 |
| 4 | 126 | **g_src** | 4 ($a0) | 12 | 12 | 30000 |
| 5 | 123 | **r_src** | 5 ($a1) | 12 | 13 | 27692 |
| 6 | 108 | src cursor | 7 ($a3) | 32 | 61 | 26229 |

pri = weighted-reg_n_refs * 30000 / live_length (global.c:635-653). The weight
is the loop depth (3 here), so one C-level reference is worth 3 nrefs. px has
four source references (its set, `px == 0`, `px >> 2`, `px >> 7`) = 12.

The target's seats are px = $a0, g_src = $v1, r_src = $a1. r_src is already
right. So the entire blend residual is "g_src must be allocated before px".
The two ways to get there, with the exact numbers:
  * raise g_src above 32727 -> livelen <= 10 at nrefs 12 (36000), or nrefs >= 14
    at livelen 12 (35000);
  * drop px below 30000 -> livelen >= 13 at nrefs 12 (27692). That value ties
    r_src, and global.c:652-653 breaks an exact tie on ascending allocno number
    (122 < 123), so px still precedes r_src: the resulting order is g_src, px,
    r_src -> $v1, $a0, $a1, which is exactly the target.
$v0 is unavailable to px in every spelling because p118 (`count - 1`) conflicts
with px and is allocated first at pri 90000.

### What the rename would buy
A raw v1<->a0 rename of our stream makes target rows 88, 89, 99, 100, 101, 104,
105, 116 and 122 byte-exact (verified by hand against s14/sxs.py output). The
rows that would still differ after the rename are the three channel sums
(111/112, 114/115, 118/119), the b source register (106/107/108/117) and the
trip test (127/128/130) — all of which are the same local-alloc phenomenon
described below.

### Local-alloc coalescing is the second-order difference
Our build systematically coalesces a dying source register with its
destination where the target does not:
  * target `srl v0,a0,0x2` / `andi v1,v0,0xf8` vs ours `srl v0,v1,0x2` /
    `andi v0,v0,0xf8` (form ACA);
  * target `addu v0,t2,a1` / `sra a1,v0,0xf` vs ours `addu a1,t2,a1` /
    `sra a1,a1,0xf`;
  * likewise at the g and b sums.
In the target the sums all land in $v0 and are then shifted into the channel
register; in ours they overwrite an operand. Spelling that as a shared C
variable is measured wrong (N1 = 53), so it is a local-alloc quantity decision,
not a source-level one.

### Structural axes measured dead this session
* the full 3x3 per-channel product/reuse lattice (27 forms, s14/G_*.c): min 26
  at AAA (incumbent) and AAC; AAC reproduces the target's b-channel register
  PATTERN and still scores 26;
* all six channel-block orderings (s14/O_*.c): 26 / 33 / 34 / 34 / 40 / 40;
* all six `src++` placements inside the blend arm (s14/P_s*.c): all 26,
  byte-identical — sched1 normalises the cursor bump;
* one shared `sum` local for the three channel totals (s14/N1,N3,N4): 53/45/46;
* deriving the b source from the g source's shift to cut a px reference
  (s14/Q1.c): combine refolds it, byte-identical at 26, px unchanged at
  nrefs 12 / livelen 11 / pri 32727.

- [s14] Chassis re-measured at dispatch: the s13 candidate applied to src/code6cac_c2.c scores 26 at build_insns 173 / target_insns 173 on HEAD 2026-09-10. The floor did not move this session.

- [s14] global.c priority is pri = depth-weighted reg_n_refs * 30000 / live_length; at loop depth 3 one C-level reference is worth 3 nrefs. Verified against six allocnos in the dump (e.g. p118: nrefs 9 / livelen 3 -> 90000).

- [s14] The candidate chassis allocation order is p118 ($v0, count-1, pri 90000), the LO pseudo, px = p122 ($v1, 32727), dst = p109 ($a2), g_src = p126 ($a0, 30000), r_src = p123 ($a1, 27692), src = p108 ($a3).

- [s14] px cannot take $v0 in any spelling measured this session: p118 (count - 1) conflicts with px and is allocated first at pri 90000.

- [s14] r_src is ALREADY on the target's $a1 in the incumbent; only px and g_src are swapped relative to the target.

- [s14] A v1<->a0 rename of our stream would make target rows 88, 89, 99, 100, 101, 104, 105, 116 and 122 byte-exact; the rows that would still differ are the three channel sums (111/112, 114/115, 118/119), the b source register (106/107/108/117) and the trip test (127/128/130).

- [s14] Our local-alloc systematically coalesces a dying source register with its destination where the target keeps them apart: target 'srl v0,a0,0x2 / andi v1,v0,0xf8' vs our 'srl v0,v1,0x2 / andi v0,v0,0xf8' (form ACA); target 'addu v0,t2,a1 / sra a1,v0,0xf' vs our 'addu a1,t2,a1 / sra a1,a1,0xf'. That is the second-order difference behind the sum rows.

- [s14] Form AAC (b channel with both products named) scores 26 like the incumbent but is byte-different, and it reproduces the target's b-channel register pattern, so the b-channel rows are naming, not shape.

- [s14] sched1 normalises the src++ cursor bump: six different source placements produce byte-identical objects.

## S15 (enumerate) — six exhaustive spelling families, 3,966 valid spellings, ZERO hit

Chassis re-measured at dispatch: `memory/grind/func_8003DE14/candidate.c` =
**26 / 173 insns** on HEAD 2026-09-10 (`tools/sweep_variants.py`), so every s13/s14
conclusion remains chassis-valid and the floor did not move this session.

Design + generators: `tmp/grind/func_8003DE14/s15/README.md`.  Every variant is a
complete function body spliced into `src/code6cac_c2.c` and scored with the honest
`sandbox func_8003DE14 --disable all`.  Per-variant scores in `s15/<family>.json`.

| family | region | axis enumerated | n | histogram | best |
|---|---|---|---|---|---|
| enum_il | blend arm, the nine channel assignment statements | every interleaving of the three per-channel chains, 9!/(3!^3) | 1680 | {26:196, 30:84, 31:84, 33:280, 34:476, 40:392, 41:84, 43:84} | **26 = baseline** |
| enum_head | outer-loop declaration block + guard | hoist subsets of {complement, j} above the `if (total > 0)` guard x every def-before-use order | 588 | {26:24, 28:20, 29:40, 30:84, 40:60, 44:360} | **26 = baseline** |
| enum_head2 | same block | src/dst declared at function scope and merely ASSIGNED in-loop (4 combos) x 24 orders | 96 | {26:40, 59:8, 108:8, 109:8, 116:16, 123:16} | **26 = baseline** |
| enum_test | the last-pass arm test `i == count - 1` | algebraic respellings of the comparison | 7 | {27:1, 32:2, 34:2, 51:2} | 27 |
| enum_latch | the inner-loop latch `j < rect[2] * rect[3]` | operand order / hoisted `total` / comparison form | 8 | {26:4, 28:1, 36:2, 40:1} | **26 = baseline** |
| enum_ssa | blend arm in NO-REUSE (SSA) form | tools/spelling_enum.py inline-subset x declaration-order lattice | 550 | see S15.6 | — |

### S15.1 The blend arm's statement-interleaving space is flat at the floor
All 1680 interleavings of the nine channel assignments (per-channel order fixed by
data dependence: product, reuse-write, sum) collapse onto EIGHT distinct scores,
and 196 of them — 11.7% — are byte-identical ties at 26.  The incumbent
`r,r,r,g,g,g,b,b,b` is one of them.  Crucially the TARGET'S OWN statement order,
read off `asm/funcs/func_8003DE14.s` rows 103-131 (`r` product pair, `g` product
pair, `b` product pair, then the three sums = tag `rrggbbrgb`), is also in the
tie-band at 26, NOT below it: writing the arm exactly the way the target's own
insn stream is laid out buys nothing.  sched1 normalises this entire axis.

### S15.2 The head region (delay-slot residual) has no spelling below 26 either
588 spellings covering everything s12/s13's hand-probed E-series did NOT: hoisting
`j = 0` and/or `complement` OUT of the guard entirely (480 of the 588 are novel),
crossed with every def-before-use order of `total / src / dst / factor`.  The two
bands that contain every hoisted-`j` spelling are 40 and 44 (420 of 588); hoisting
`complement` alone lands at 29/30.  The 24 ties at 26 are exactly the spellings
that keep BOTH `complement` and `j` inside the guard in that order — i.e. the
incumbent's shape, with the four per-iteration statements in any order (confirming
s13's E4/E5/E6 byte-identity result across the full 24-order space, not just 3 of it).

### S15.3 The cursor declaration-SITE axis is strictly worse
Declaring `src` and/or `dst` at function scope and assigning them in the loop
costs 33 to 97 points (59 / 108 / 109 / 116 / 123 bands).  Only the all-in-loop
combination (40 spellings) ties 26.  The `src`/`dst` pseudos must be born inside
the outer loop body for the $a3/$a2 seat.

### S15.4 The `i == count - 1` comparison spelling is not a lever on p118
p118 (`count - 1`, pri 90000) is the allocno that blocks `$v0` for `px`.  Seven
respellings measured: `count - 1 == i` 27, `count - i == 1` 32 (174 insns),
`i - count == -1` 32 (174), `i >= count - 1` 34 (174), `!(i < count - 1)` 34 (174),
`i + 1 == count` 51, `count == i + 1` 51.  Nothing reaches 26, and the three forms
that change the arithmetic shape all ADD an instruction.

### S15.5 The latch's operand order is inert; `!=` changes the instruction count
`j < rect[3] * rect[2]`, `j < (rect[2] * rect[3])`, `rect[2] * rect[3] > j` and
`rect[3] * rect[2] > j` are all byte-identical to the incumbent at 26.
`j != rect[2] * rect[3]` drops to 172 insns (28).  `j < total` (the hoisted
product) is 40, and the `((u16 *)rect)[2] * ((u16 *)rect)[3]` sub-word forms are
36 — the target's `lh` pair is what the incumbent already emits.  So the 3-insn
trip-test residual is NOT reachable from the latch's own spelling: it is downstream
register naming, exactly as s13 recorded.

### S15.6 enum_ssa: generator defect found and corrected mid-session
The first enum_ssa chassis put `src++;` immediately BEFORE the region, so every
spelling that keeps at least one declaration is a declaration-after-statement in
the same block — a C89 parse error under GCC 2.7.2.  549 of 550 variants therefore
measured a degenerate 100 / 116 insns (only the fully-inlined v549, which has no
declarations at all, compiled: 45 / 173).  Corrected chassis
(`s15/enumc_base2.c`) moves `src++;` AFTER the region — s14 measured all six
`src++` placements byte-identical on this chassis, so the move is free.  RESULT
OF THE CORRECTED SWEEP: see the s15 outcome JSON / the line below.

**Lesson for future enumerate sessions on this function: an ENUM region that
begins with declarations must not be preceded by a statement in the same block.**

### S15.6 result (corrected chassis) — the no-reuse blend lattice tops out at 28
550 spellings, histogram {28:37, 33:37, 34:37, 35:37, 41:74, 44:37, 45:83, 54:67,
55:104, 56:37}.  Best = **28**, reached by 37 spellings — exactly the s12 shape.
Nothing in the entire inline-subset x declaration-order lattice of the NO-REUSE
blend arm reaches the reuse form's 26, which independently re-confirms s13's
finding (one C variable carrying both a channel's shifted source and its
`X * factor` product is worth 2 points and is not reachable by naming/ordering
alone).

- [s15] [s15] Chassis re-measured at dispatch: memory/grind/func_8003DE14/candidate.c = 26 / 173 insns on HEAD 2026-09-10 (tools/sweep_variants.py), so every s13/s14 conclusion remains chassis-valid and the floor did not move this session.

- [s15] [s15] 3,966 valid spellings measured across six families; the global minimum over all of them is 26, reached only by bodies byte-identical to the incumbent. Four of the six families contain byte-identical ties at 26 (196 + 24 + 40 + 4 spellings) - large flat plateaus, not near-misses.

- [s15] [s15] The TARGET'S OWN blend statement order is now known and measured. asm/funcs/func_8003DE14.s rows 103-131 run r's product pair, then g's, then b's, and only then the three sums - source order r,r,g,g,b,b,r,g,b. Written that way our build emits BYTE-IDENTICAL code to the incumbent (26), so the residual is provably not a statement-order fact in the blend arm.

- [s15] [s15] The head region's two constraints are now measured over their whole space rather than sampled: hoisting j = 0 above the guard costs 14-18 points in every one of the 420 spellings that do it, and all 24 spellings at the floor keep complement-then-j inside the guard. s13's tension finding (j must sit below complement for the seat but above it for the delay slot) survives the exhaustive sweep.

- [s15] [s15] The cursor pseudos must be born inside the outer loop body: all 56 function-scope declaration spellings cost 33-97 points.

- [s15] [s15] The per-channel variable reuse introduced in s13 is worth exactly 2 points against the ENTIRE no-reuse naming/order lattice (550 spellings, best 28).

- [s15] [s15] TOOLING: an ENUM region that begins with declarations must not be preceded by a statement in the same block - GCC 2.7.2 is C89, so every such variant is a parse error that measures as a degenerate score (549 of 550 at 100 / 116 insns). Cost this session: one 550-variant sweep re-run (s15/enumc_base.c vs enumc_base2.c).

## s16 (synthesis) - the seat is REACHABLE; it is the schedule that is not

Chassis re-measured first: the incumbent body (memory/grind/func_8003DE14/candidate.c)
scores **26 at 173 insns** on HEAD 2026-09-10, and so do the two closest banked kills
(s14's AAC, s15's target-shaped interleave `rrggbbrgb`). The body carries no
annotation-bearing construct, so there is nothing for tools/fake_ablate.py to ablate;
the kill re-audit is a chassis re-measurement and both kills stand.

### 1. The residual's pseudo map, corrected

The instrumented cc1 (`tools/gcc-2.7.2/cc1`, BB2_ALLOC_DEBUG=1) plus the `.lreg` RTL
names every allocno in the blend arm:

| pseudo | variable | nrefs | livelen | pri | ours | TARGET |
|---|---|---|---|---|---|---|
| p121 | pixel   | 15 | 26 | 17307 | $t0 | $t0 |
| p122 | px      | 12 | 11 | 32727 | **$v1** | **$a0** |
| p128 | b_src   | 12 | 11 | 32727 | **$v0** | **$a0** (reuses px's reg after px dies) |
| p126 | g_src   | 12 | 12 | 30000 | **$a0** | **$v1** |
| p123 | r_src   | 12 | 13 | 27692 | $a1 | $a1 |

s13/s14 recorded this as a two-way px/g_src swap and believed b_src was a
local-alloc quantity. It is not: b_src is the SECOND member of the 32727 tie, and
it is b_src - not the target's scratch - that occupies $v0 in our build. The
target leaves $v0 to local-alloc entirely (the srl temps and the three channel
sums) and parks b_src on $a0 after px dies.

### 2. Declaration-site renumbering is a real, byte-neutral lever (384/384 = 26)

global.c:635-653 sorts allocnos by `pri = weighted nrefs * 30000 / live_length` and
breaks EQUAL priorities on ascending pseudo number; pseudo numbers follow
DECLARATION order. Splitting a channel local into (declaration, assignment) and
moving only the declaration renumbers pseudos without moving a statement or
changing a live range. All 384 spellings of that axis score 26 at 173 insns, and
the dumps prove the renumbering is not a no-op: in `R_gbr_000` g_src becomes p121
(was p126) and the 32727 pair swaps seats ($v1 <-> $v0). It cannot reach the target
seat on its own because px (32727) and g_src (30000) are never tied.

### 3. The cross-channel carrier lattice: 1,483 spellings, alloc tables for all of them

s14 swept the 27 WITHIN-channel spellings. This session swept the cross-channel
lattice - which local carries each of the six products, restricted by a simulated
liveness so every emitted body is semantics-preserving (`s16/gen_carrier.py`), then
compiled all 1,483 with the instrumented cc1 and recorded the allocno table plus
the pseudo->variable map for each (`s16/batch_alloc.py`, `s16/carrier_alloc.json`).

  * **4 forms hit three of the four target seats exactly** (r_src $a1, g_src $v1,
    px $a0): `C_FFrSgSgSF`, `C_FFrSgSgSbS`, `C_FFrSgSgSrG`, `C_FFrSgSgSrP`. All four
    make g_src carry two products (nrefs 18, pri 45000) and drop px to livelen 10.
  * 89 forms hit two seats; 108 forms produce a px/g_src PRIORITY TIE.
  * s14's arithmetic is independently confirmed: `C_FFFgSFrG` puts px at exactly
    nrefs 12 / livelen 13 / pri 27692 and does seat g_src on $v1 and px on $a0.

**But none of them pays.** Scored: the seat-correct forms are 38 (173 insns); the
best of the 25 top-ranked is 34; the lattice minimum is 26, reached only by
incumbent-shaped spellings. The side-by-side (s13/sxs.py, rows 94-131) of the best
seat-correct form shows why - giving g_src a second product re-prices the arm for
sched1, the products and sums re-interleave, and even the loop's own registers move
(complement $t5->$t4, factor $t3->$t2, j $t4->$t3). The seat win is real and is
swamped by the re-schedule.

### 4. Tie + renumber (the conjunction nobody had tried): best 34

All 108 tie forms were given the byte-neutral declaration hoist so that g_src would
win the tie-break by pseudo number (`s16/gen_tie.py`, `s16/tie.json`). Zero hit;
histogram 34..56. The renumbering lever is free on the incumbent and worthless on
every form that creates a tie, because creating the tie costs the schedule.

### What this session changes about the frontier

Before s16 the open question was "is the px/g_src seat reachable at all?" It is -
in ordinary C, four different ways. The question is now strictly narrower:

> Is there a change that raises pri(g_src) above 32727 (livelen <= 10 at nrefs 12,
> or nrefs >= 15 at livelen <= 13) while leaving the AAA value->variable mapping,
> and therefore the sched1 priorities of the nine arm statements, untouched?

Every knob INSIDE the arm that moves g_src's live length also moves the arm's
schedule; the two are the same knob. So the next probe has to come from OUTSIDE the
arm: the insns sched1 interleaves INTO the arm (the cursor bump at row 110 and the
`mflo` at row 117 come from the latch/trip-test region), the loop structure around
it, or the conflict set that excludes $v0 (in the target, nothing global sits in
$v0 at all - which is itself a measurable sub-goal: find a form where b_src is
excluded from $v0).

Artifacts: tmp/grind/func_8003DE14/s16/{audit,renum3,carrier,tie,top}*.json,
s16/carrier_alloc.json (3.0 MB, every allocno table), s16/d_base, s16/d_gbr000,
s16/gen_carrier.py, s16/gen_renum2.py, s16/gen_tie.py, s16/batch_alloc.py,
s16/analyze.py.

- [s16] HEAD honest floor for func_8003DE14 on 2026-09-10 is 26 at 173 insns (target 173); the incumbent body in memory/grind/func_8003DE14/candidate.c is unchanged and is ordinary C with no annotation-bearing construct.

- [s16] Corrected pseudo map from the .lreg RTL: p121 = pixel, p122 = px, p123 = r_src, p126 = g_src, p128 = b_src. b_src is a GLOBAL allocno (nrefs 12 / livelen 11 / pri 32727) and is what holds $v0 in our build; s14's note that it is a local-alloc quantity is wrong.

- [s16] Our seats vs the target's: px $v1 vs $a0; b_src $v0 vs $a0 (target reuses px's register after px dies at row 118); g_src $a0 vs $v1; r_src $a1 vs $a1. The target has NO global allocno in $v0 at all.

- [s16] Declaration-site renumbering (declaration split from assignment, assignment left in place) is byte-neutral across all 384 spellings and demonstrably moves pseudo numbers and the seats of the tied 32727 pair - a free lever available to any future form that produces a px/g_src tie.

- [s16] The cross-channel carrier lattice is 1,483 liveness-checked spellings; allocation tables for every one of them are banked in tmp/grind/func_8003DE14/s16/carrier_alloc.json, together with tools (gen_carrier.py, batch_alloc.py, analyze.py) that regenerate and re-rank them on any future chassis.

- [s16] Seat reachability is proven: 4 forms hit r_src $a1 + g_src $v1 + px $a0, and C_FFFgSFrG reproduces s14's predicted px = nrefs 12 / livelen 13 / pri 27692 route. Affordability is disproven on this chassis: 38 for the seat-correct forms, 34 for tie+renumber, 26 for the incumbent.

- [s16] Mechanistic conclusion for the next session: inside the blend arm, g_src's live length and the arm's sched1 priorities are the SAME knob - every carrier change that moves one moves the other. pri(g_src) must exceed 32727 (livelen <= 10 at nrefs 12, or nrefs >= 15 at livelen <= 13) while the AAA value->variable mapping is left alone, so the change has to originate outside the arm.

## s17 (solver) - the scheduler axis is closed; the residual is 100% RA + one reorg.c choice

Chassis re-measured first: candidate.c spliced into `src/code6cac_c2.c`,
`sandbox func_8003DE14 --disable all` -> **score 26, build_insns 173,
target_insns 173** on HEAD 2026-09-10 (post -mel, post -msoft-float). The
ledger floor is confirmed, not inherited.

KILL RE-AUDIT (mandated by the flat-floor rule). `tools/fake_ablate.py --func
func_8003DE14 --file code6cac_c2 --candidate memory/grind/func_8003DE14/candidate.c`
prints *"no FAKE-annotated constructs found ... nothing to ablate"*
(tmp/grind/func_8003DE14/s17/fake_ablate.log). The incumbent is ordinary C end to
end, so every instance kill s13-s16 recorded was measured with ZERO FAKE carriers
occupying any pseudo - the "a lever measured inert while a FAKE carrier sat on its
target pseudo is not a kill" failure mode cannot apply to this ledger. The
re-audit is therefore satisfied by the chassis re-measurement above, and the
banked kills stand as written.

### 1. `inverse_compose.py classify` - FIRST DIVERGENCE: **RA**

Run in object mode (the only legal mode for an INCLUDE_ASM-routed function):

    python3 tools/ra_solver/inverse_compose.py classify code6cac_c2 func_8003DE14 \
      --target-object build/src/code6cac_c2.o \
      --ours-object   tmp/sandbox/func_8003DE14/code6cac_c2.o

    func_8003DE14 (code6cac_c2): honest 173 insns, target 173 insns
    FIRST DIVERGENCE: RA
      next tool: tools/ra_solver/inverse.py  (global / local)

(log: `tmp/grind/func_8003DE14/s17/classify.log`.) The classifier's PRE-RA test
is the register-BLANKED multiset comparison, and it passes: our 173-insn stream
and the target's 173-insn stream contain exactly the same instructions modulo
register names. **There is no front-end / cse / combine / loop divergence
anywhere in this function.** Every one of the 26 points is a register name or a
placement - never a different instruction, never a different count. That closes
the whole "is some expression being materialised differently?" family of
hypotheses without a single compile.

### 2. `tools/sched_solver` - ONE flagged block, and it is an alignment artifact

Model extracted cleanly for the incumbent body:

    python3 tools/sched_solver/extract.py code6cac_c2
    code6cac_c2: parity=True funcs=82 (pass1=41 pass2=41) blocks=750 picks=3989

Goal derived from the target OBJECT (`--target-object build/src/code6cac_c2.o`,
`--ours-object tmp/sandbox/func_8003DE14/code6cac_c2.o`), atoms `luid,luid_move`,
depth 2, run for **both** passes. Both runs print the same one-line verdict:

    pass 2:  block 10: 34 insns, goal differs from ours
             SKIPPED -- goal is not a topological order (8 violations):
             the target alignment mis-paired duplicate instruction text here
    pass 1:  block 10: 28 insns, goal differs from ours
             SKIPPED -- goal is not a topological order (3 violations): ...

(logs: `s17/perturb_pass1.log`, `s17/perturb_pass2.log`.)

Block 10 is the blend arm: its `nodes` table carries the twelve unit-1 insns
(six `mult` at icost 12 and their six paired icost-1 partners, uids
227/397, 230/400, 238/403, 241/406, 249/409, 252/412). "Goal is not a topological
order" means the target-vs-ours alignment could not produce a self-consistent
order for it - which is exactly the difflib mis-pairing s13 diagnosed by raw
index-by-index comparison, and it happens here for the same reason: the blend
arm's instruction TEXTS differ only in register names, so the aligner pairs the
wrong `mult`/`mflo`/`andi` copies with each other. **There is no real order
divergence in the blend arm**, and the solver has nothing to solve there.

**Every other block of func_8003DE14 matched the target's order in both passes.**
That includes the outer-loop head block - the one carrying the 4-insn
blez-delay-slot residual (target rows 54-71 vs ours). So:

  * s16 frontier item 3 ("run sched_solver on the outer-loop head basic block ...
    ask whether an emission order exists in which both cursor addiu's precede
    the mflo, and what INSN_PRIORITY delta it requires") is **answered and
    dead**: our head block already emits the target's order out of sched2. No
    INSN_PRIORITY delta is required because there is no priority disagreement.
  * s16 frontier item 1 ("ask which emission orders are legal for the
    interleaved cursor bump and trip-test insns and what INSN_PRIORITY delta
    moves them out of rows 110-117") is **dead on the same evidence**: the
    interleaving of the `src++` bump (row 110) and the trip-test `mflo` (row
    117) into g_src's second live segment is what the TARGET's schedule does
    too. g_src's 12-long live length cannot be shortened by re-scheduling,
    because there is no alternative schedule to reach - ours is already the
    target's. If g_src's live length is to fall, the change must come from the
    live range itself (fewer / earlier references), not from the pick order.

### 3. What the head residual actually is: reorg.c, not sched

Raw index-by-index diff (`s17/sxs_diff.txt`), rows 53-71:

    53  addiu a3,sp,16                    | addiu a3,sp,16
    54  addiu a2,sp,1040                  | mflo v1
    ...
    68  mflo t3                           | blez v1,...
    69  blez v1,...                       | addiu a2,sp,1040     <- our delay slot
    70  move t4,zero                      | subu t5,s8,t3
    71  subu t5,s8,t3                     | move t4,zero

One instruction - `addiu a2,sp,1040`, the `dst = dst_buf` cursor init - accounts
for the entire rows-54..71 rotation. The target emits it inside the multiply
latency shadow next to the `src` cursor and then fills the `blez` delay slot
with `move t4,zero` taken from the FALL-THROUGH thread; we leave the slot to be
filled with the `addiu` itself. Since the sched model (order- AND clock-exact,
6978/6978 blocks project-wide) says our head block's emission order equals the
target's, the divergence is created after sched2 - in `reorg.c`'s
`fill_simple_delay_slots`, which prefers an independent insn from BEFORE the
branch over one from the fall-through. The sched_solver README is explicit that
`reorg.c` is "downstream of this model and out of its scope", so the solver
cannot rank this; the lever is reorg ELIGIBILITY, i.e. making the last
pre-branch insn something reorg cannot hoist into the slot (an `mflo` is
ineligible - which is precisely what sits there in the target).

### 4. Tooling: how to run the solver on this function (reusable)

Two obstacles, both now solved in `tmp/grind/func_8003DE14/s17/`:

1. `tools/sched_solver/mkasm.sh` produces the wrong honest stream here. It
   predates `--prefill-label-funcs=maspsx_prefill_label_funcs.txt` (owner ruling
   2026-09-04) and it runs the FULL `prologue_fix`, whereas the sandbox builds
   with `cheats.empty_overrides` (empty prologue config) and a cheat-stripped
   src. `s17/mkasm3.py` regenerates `tmp/sched_map/code6cac_c2.{cc1,hon,tgt}.s`
   by calling `engine.pipeline.c_pipeline_cmd` with the sandbox's own override
   dict and dropping the final `as` stage - guaranteed same source state.
2. `goalmap.build_map`'s same-source checksum rejects the function even then:
   the honest text body has 179 lines against 173 object insns. The six extra
   lines are maspsx's mult/mflo interlock `nop` PAIRS. `objdump` renders a run
   of zero words as `...`, so `engine.score.normalized_insns` drops them - from
   BOTH streams symmetrically (checked with `objdump -z`: target and ours both
   carry `nop; nop` at 0x25c8/0x25cc, 0x25e8/0x25ec, ...). `s17/perturb2.py`
   wraps `perturb.py` and patches `goalmap._macro_expand_counts` so that a `nop`
   which is part of a RUN of nops expands to 0 object insns (an ISOLATED nop
   still counts 1 - there are two of those in this body, and 179 - 6 = 173
   checks out). `tools/` is not an editable surface for a grind session, so the
   patch lives in scratch; a future operator-side fix belongs in `goalmap.py`.

- [s17] Chassis re-measured this session: memory/grind/func_8003DE14/candidate.c spliced into src/code6cac_c2.c gives sandbox --disable all score 26, build_insns 173, target_insns 173 on HEAD 2026-09-10. The ledger's floor of 26 is confirmed, not inherited. src/ was restored to HEAD before the session ended.

- [s17] KILL RE-AUDIT satisfied: tools/fake_ablate.py --func func_8003DE14 --file code6cac_c2 --candidate memory/grind/func_8003DE14/candidate.c reports 'no FAKE-annotated constructs found ... nothing to ablate'. The incumbent is ordinary C end to end, so no instance kill in this ledger was ever measured with a FAKE carrier occupying a target pseudo; the s13-s16 kills stand as written.

- [s17] inverse_compose.py classify (object mode) returns FIRST DIVERGENCE: RA with honest 173 / target 173 and identical register-blanked multisets - the PRE-RA axis is closed for this function.

- [s17] tools/sched_solver/extract.py code6cac_c2 reports parity=True, 82 funcs (41 pass1 / 41 pass2), 750 blocks, 3989 picks - the model is valid on this TU with the candidate body in place.

- [s17] perturb with the goal derived from the target OBJECT flags exactly one block of func_8003DE14 in each pass: block 10 (34 insns in pass 2, 28 in pass 1), the blend arm, and skips it because the goal is not a topological order (8 / 3 violations) - the aligner mis-paired register-renamed duplicate instruction text. Every other block, including the outer-loop head block, has goal == ours.

- [s17] The head residual is one instruction: addiu a2,sp,1040 (dst = dst_buf). The target emits it at row 54 inside the multiply latency shadow next to addiu a3,sp,16 and fills the blez delay slot at row 70 with move t4,zero from the fall-through thread; we leave the slot for the addiu itself at row 69, pushing subu t5,s8,t3 to 70 and move t4,zero to 71. Rows 55-68 are a pure shift artifact of that one move.

- [s17] TOOLING: tools/sched_solver/mkasm.sh cannot be used on this function - it predates --prefill-label-funcs (owner ruling 2026-09-04) and runs the FULL prologue_fix, while the sandbox builds with cheats.empty_overrides and a cheat-stripped src. tmp/grind/func_8003DE14/s17/mkasm3.py rebuilds the three sched_map streams through engine.pipeline.c_pipeline_cmd with the sandbox's own override dict.

- [s17] TOOLING: goalmap.build_map's same-source checksum rejects this function even with the correct streams (179 honest text lines vs 173 object insns). The six extra lines are maspsx mult/mflo interlock nop PAIRS that objdump renders as '...' and engine.score.normalized_insns drops - symmetrically from BOTH streams (objdump -z confirms target and ours both carry nop;nop at 0x25c8/0x25cc and 0x25e8/0x25ec). tmp/grind/func_8003DE14/s17/perturb2.py patches goalmap._macro_expand_counts so a nop inside a RUN of nops expands to 0 object insns while an isolated nop still counts 1 (179 - 6 = 173). A permanent fix belongs in tools/sched_solver/goalmap.py, which a grind session may not edit.


## s18 (forensics, 2026-09-10) - the head residual is reorg.c's backward scan, and it is now CLOSED as a structure question

Chassis re-measured at dispatch: candidate.c (s13 body) installed in
src/code6cac_c2.c scores 26 / 173 insns on HEAD 2026-09-10 (post -mel, post
-msoft-float).  Unchanged from s13-s17.

### 1. PASS ATTRIBUTION, from the pass's own instrumentation (not inferred)

`tools/gcc-2.7.2/cc1` with `BB2_DBR_DEBUG=1`
(tmp/grind/func_8003DE14/s18/dbr/stderr.log:2185-2191) prints the entire
backward scan of `fill_simple_delay_slots` for the head block's `blez`
(jump_insn 131):

    DBRDBG simp insn=131 trial=128 refset=0 setset=0 setneed=0
    DBRDBG simp insn=131 trial=128 elig=0
    DBRDBG simp insn=131 trial=125 refset=1 setset=1 setneed=1
    DBRDBG simp insn=131 trial=124 refset=0 setset=1 setneed=1
    DBRDBG simp insn=131 trial=394 refset=1 setset=0 setneed=1
    DBRDBG simp insn=131 trial=121 refset=0 setset=0 setneed=0
    DBRDBG simp insn=131 trial=121 elig=1

Reading it against reorg.c:2963-3020: insn 128 is the divmod parallel, resource-
clean but `eligible_for_delay` = 0 (multi-insn template); 125 (`sll v0`), 124
(`addiu v0,s1,1`) and 394 (`mflo v1`) all collide with the accumulated
set/needed sets; insn 121 - `(set (reg a2) (plus (reg sp) (const_int 1040)))`,
i.e. `dst = dst_buf` - is clean AND eligible, so reorg deletes it from the block
and puts it in the slot.  THE SCHEDULER IS INNOCENT: the .sched2 dump
(tmp/grind/func_8003DE14/dumps/code6cac_c2.sched2, function region lines
18900-20509) already emits 118 (`addiu a3,sp,16`) immediately followed by 121
(`addiu a2,sp,1040`) - exactly the target's rows 56/57.  reorg is what moves it.
This confirms the s17 solver verdict from the other direction.

### 2. THE PASS-INPUT ENUMERATION THAT FOLLOWED

reorg takes the CLOSEST eligible insn before the branch.  The only source-side
input that changes the outcome is therefore WHICH insn is last in the head block
before the `blez`.  Hoisting `s32 j = 0;` out of the `if (total > 0)` guard and
writing it as a sibling declaration-initialiser after `factor` (form h1,
memory/grind/func_8003DE14/chassis_s18_h1_head_exact_28.c) makes `move j,zero`
the last pre-branch insn - and reorg takes it, exactly as the target does.

Result (tmp/grind/func_8003DE14/s18/h1.txt against asm/funcs/func_8003DE14.s):

    target 55-57  mult v1,v0 | addiu a3,sp,0x10 | addiu a2,sp,0x410
    h1     2514-  mult v1,v0 | addiu a3,sp,16   | addiu a2,sp,1040     IDENTICAL
    target 73-77  mflo t3 | blez v1 | [slot] addu t4,zero,zero | subu t5,fp,t3 | addiu v0,s2,-1
    h1     2554-  mflo t3 | blez v1 | [slot] move t5,zero      | subu t4,s8,t3 | addiu v0,s2,-1

The 4-insn head residual that has been on the frontier since s13 is GONE in h1.
What replaces it is a pure two-register NAME swap: h1 seats j on $t5 and
complement on $t4; the target and the incumbent seat j on $t4 and complement on
$t5.  Because complement is read three times in the blend arm and j twice in the
latch, the swap costs ~6 rows, so h1 scores 28 against the incumbent's 26.

### 3. THE SEAT IS ONE global.c PRIORITY COMPARE, WITH THE ARITHMETIC IN HAND

`BB2_ALLOC_DEBUG=1` on the h1/h6 chassis
(tmp/grind/func_8003DE14/s18/alloc_h6/stderr.log):

    ord=15 pseudo=116 (complement) hardreg=12 ($t4) nrefs=11 livelen=54 pri=6111
    ord=16 pseudo=115 (j)          hardreg=13 ($t5) nrefs=11 livelen=59 pri=5593

pri = floor_log2(nrefs) * nrefs * 10000 / livelen (global.c:605-612 is the dump
site).  Both pseudos carry nrefs 11 because reg_n_refs is loop-depth weighted in
flow.c: complement = def at depth 2 + three reads at depth 3 = 2 + 9; j = def at
depth 2 + `j++` (a read and a write) at depth 3 + the trip compare at depth 3 =
2 + 6 + 3.  The ONLY term that differs is livelen, and it differs by exactly the
5 insns by which hoisting `j = 0` lengthens j's live range.  In the incumbent the
same pair prices the other way round, which is why the incumbent has the seats
right and the structure wrong.

Three flips exist arithmetically and s18 measured all three families:
  (a) nrefs(j) >= 14 (one extra depth-3 read of j: +3) => pri 7118 > 6111.  No
      byte-neutral spelling found: `if (j < total)` and `if (j < rect[2]*rect[3])`
      as the guard both defeat the blez (slt + branch) and score 45.
  (b) livelen(j) <= 54.  j's def is already the last insn of the head block (it
      IS the delay-slot insn) and its last use is the latch; the range cannot be
      shortened without moving the def back inside the guard, i.e. the incumbent.
  (c) livelen(complement) >= 60.  complement's def is already the first insn of
      the preheader and its last use (the b-channel complement product) is 3
      insns before j's last use.  The blend arm's statement order cannot push it
      6 insns later: each channel's complement product must precede the
      `X_src = X * factor` overwrite of that channel's carrier, so the current
      r/g/b-in-full order already places complement's last use as late as the
      per-channel reuse allows.

### 4. WHY IT IS WORTH 4 POINTS, NOT 2

The incumbent's 26 decomposes head 4 + blend-arm seats 19 + trip test 3.  h1's 28
decomposes head 0 + blend 19 + trip 3 + t4/t5 swap 6.  A form with h1's head AND
the incumbent's j/complement seats scores 22 - a 4-point drop and the first
movement of this floor since s13.

### 5. FORMS MEASURED THIS SESSION (all on HEAD 2026-09-10, no FAKE constructs)

    base (candidate.c, s13 body)                                   26
    h1  j=0 hoisted above the guard as a decl-initialiser          28   head EXACT
    h2  j declared bare, `j = 0;` as a statement above the guard    28
    h3  `s32 j = 0;` declared before `total`                        30
    h4  inner loop rewritten `while (j < rect[2]*rect[3])`,
        complement inside the body for LICM                        59
    h5  complement+j declared (uninitialised) above the guard,
        assigned in place, complement first                        28
    h6  same, j declared first                                     28
    h7  complement AND j hoisted above the guard, complement first 44
    h8  same, j first                                              44
    h9  h1 with the guard spelled `if (j < total)`                 45
    h10 h1 with the guard spelled `if (j < rect[2] * rect[3])`     45

h5/h6 also kill the declaration-order lever for THIS pair: splitting the
declarations from the assignments and swapping their order leaves the score at
28 and does not move the $t4/$t5 seats, so the pair's seat is decided purely by
the priority compare above and not by pseudo numbering (contrast s16, where
renumbering did move the 32727-tie pair).

h7/h8's 44 has a named cause: making complement live before the guard forces the
factor division ahead of the total multiply, so the head block emits
addiu v0,s1,1 / sll / div / mflo t3 BEFORE lh/lh/mult and `total` lands in $t6
instead of $v1 (tmp/grind/func_8003DE14/s18/h7.txt:2508-255c).  This reproduces
s15's "every hoisted-j spelling is 40+" result on the s18 chassis and localises
it: it is complement, not j, that costs the 16 points.

- [s18] Chassis re-measured at dispatch: the s13 candidate.c body installed in src/code6cac_c2.c scores 26 / 173 insns on HEAD 2026-09-10; src was restored to INCLUDE_ASM before the session ended.

- [s18] reorg.c's fill_simple_delay_slots backward scan for the head blez (jump_insn 131) is printed verbatim by the instrumented cc1: trial=128 refset=0 setset=0 setneed=0 but elig=0; trial=125 refset=1 setset=1 setneed=1; trial=124 setset=1 setneed=1; trial=394 refset=1 setneed=1; trial=121 clean and elig=1 -> taken (tmp/grind/func_8003DE14/s18/dbr/stderr.log:2185-2191).

- [s18] The .sched2 dump for func_8003DE14 (tmp/grind/func_8003DE14/dumps/code6cac_c2.sched2, function region lines 18900-20509) emits insn 118 `addiu a3,sp,16` immediately followed by insn 121 `addiu a2,sp,1040` - the target's rows 56/57 - so the head residual is entirely downstream of sched2.

- [s18] Form h1 (j=0 hoisted out of the guard) makes the head region byte-identical to the target: rows 55-57 and 73-77 match opcode-for-opcode, with only the j/complement pair exchanging $t4 and $t5 (tmp/grind/func_8003DE14/s18/h1.txt).

- [s18] The j/complement seat is decided by one global.c priority compare: ord=15 pseudo=116 (complement) hardreg=12 nrefs=11 livelen=54 pri=6111, ord=16 pseudo=115 (j) hardreg=13 nrefs=11 livelen=59 pri=5593 (tmp/grind/func_8003DE14/s18/alloc_h6/stderr.log). Both carry nrefs 11 under flow.c's loop-depth weighting; only livelen differs, by exactly the 5 insns the hoist costs.

- [s18] Score decomposition: the incumbent's 26 = head 4 + blend-arm seats 19 + trip test 3; h1's 28 = head 0 + blend 19 + trip 3 + 6 rows of t4/t5 naming. A form carrying h1's head and the incumbent's seats therefore scores 22.

- [s18] Ten forms measured this session, all on HEAD 2026-09-10 with no FAKE constructs: base 26, h1 28, h2 28, h3 30, h4 59, h5 28, h6 28, h7 44, h8 44, h9 45, h10 45.

- [s18] The livelen route to the flip is arithmetically bounded: j's def is already the last insn of the head block (it is the delay-slot insn) and its last use is the latch, so livelen(j) cannot drop to 54 without moving the def back inside the guard; complement's def is already the first insn of the preheader and its last use is 3 insns before j's, and the blend arm cannot push it 6 insns later because each channel's complement product must precede the `X_src = X * factor` overwrite of that channel's carrier.

## S19 (rederive) — the t4/t5 seat problem is now a closed-form arithmetic window

Chassis re-measured at dispatch: `memory/grind/func_8003DE14/candidate.c` (the s13
body) scores **26 / 173 insns** on HEAD 2026-09-10, and
`chassis_s18_h1_head_exact_28.c` scores **28 / 173**. Both numbers reproduce
exactly; nothing in the ledger was chassis-void this session.

### The full h1 allocation table around the seat (BB2_ALLOC_DEBUG, s19/alloc_a1)

    ord=14 pseudo=111/110 (factor)     nrefs=13 livelen=56 pri=6964 -> $t3 (11)
    ord=15 pseudo=116 (complement)     nrefs=11 livelen=54 pri=6111 -> $t4 (12)
    ord=16 pseudo=110/101 (j)          nrefs=11 livelen=59 pri=5593 -> $t5 (13)

The target wants `factor=$t3, j=$t4, complement=$t5` (asm/funcs/func_8003DE14.s:75-76,
104/112/120, 148-150). global.c allocates in DESCENDING `allocno_pri` and hands each
allocno the lowest free hard register, so the target's register assignment is
reachable **iff**

    pri(factor) > pri(j) > pri(complement) = 6111           (and pri(factor) <= 6964
                                                              unless factor also moves)

i.e. **pri(j) must land strictly inside the open window (6111, 6964)**.
`pri = floor_log2(nrefs) * nrefs * 10000 / livelen`, and flow.c weights
`reg_n_refs` by loop depth (+1 per ref at function level, +2 inside the outer
do-while, +3 inside the inner do-while). With j's measured livelen of 59-61 that
admits exactly two solutions:

    nrefs(j) = 13  (one extra DEPTH-2 reference)  -> pri 6610 (LL 59) / 6393 (LL 61)
    nrefs(j) = 14  (one extra DEPTH-3 reference)  -> pri 7118 (LL 59, TOO HIGH)
                                                     pri 6885 (LL 61, IN WINDOW)

Every other reachable `nrefs` overshoots: duplicating the `j++` latch into the arms
moves nrefs to 17 / 23 / 29 (pri 11333+), and the livelen needed to bring those back
into the window (98-150) exceeds the whole loop's live length (~60).

### livelen(j) IS a controllable quantity; livelen(complement) and livelen(factor) are not

Measured by moving the `s32 j = 0;` statement through the five head statements
(`total / src / dst / factor / j=0`), all 173 insns:

    j=0 last   (= h1)        score 28   livelen(j)=59  pri 5593
    j=0 4th (before factor)  score 28   livelen(j)=59  pri 5593   (s19/f4.c)
    j=0 3rd                  score 29   livelen(j)=60-61
    j=0 2nd                  score 30   livelen(j)=61
    j=0 1st                  score 30   livelen(j)=61  pri 5409   (s19/f1.c, banked as
                                                       chassis_s19_f1_jfirst_livelen61_30.c)

complement stays at livelen 54 / pri 6111 and factor at livelen 56 / pri 6964 in ALL
five spellings — their defs are pinned by the guard branch and by the div/mflo pair.
So the ONLY free variable in the window equation is j's, and the LL-61 chassis
(f1, score 30) is the one that pairs with `nrefs(j) = 14`.

### global.c's preference machinery cannot be used to steer this seat (new, structural)

`regs_someone_prefers[]` (global.c:911-929) collects the registers preferred by
LOWER-priority conflicting allocnos, and find_reg excludes them — but ONLY in pass 0,
and pass 0 also ORs in `~regs_used_so_far` (global.c:1000-1001), i.e. "we never
allocate a register for the first time in pass 0". $t4 and $t5 are both first-time
assignments at ord 15/16, so pass 0 cannot assign either and pass 1 uses `used1`,
which does not contain `regs_someone_prefers`. Conclusion: no copy-preference or
conflict construct can make complement skip $t4 — `allocno_pri` is the sole lever.
(The complementary route, "make some allocno X occupy $t4 before complement's turn
while not conflicting with j", is empty by construction: j's live range strictly
CONTAINS complement's, so every allocno conflicting with complement also conflicts
with j.)

### Where the missing reference cannot come from (all measured this session)

DEPTH-2 sites (+2 refs, the nrefs=13 solution) partition into two classes and both
are eliminated before flow.c counts anything:
  * pre-loop (head block or inner-loop preheader): j is provably 0 there, so cse2
    folds a read (`complement = (blend_base - factor) + j`, s19/c4.c) and deletes a
    redundant store (`j = 0;` repeated inside the guard, s19/c2.c). BOTH produce an
    alloc table byte-identical to h1's — nrefs stays 11 — and both score 28/173.
  * post-loop: j is dead, so a store is a dead store (flow.c deletes it before
    mark_used_regs) and a read needs a consumer, which materialises bytes.

DEPTH-3 sites (+3 refs, the nrefs=14 solution) need a read of j inside the inner loop
whose consumer survives cse2 but is removed later. cse2 already does copy propagation,
so every plain staging copy (`jt = j; ... jt ...`) is folded and deleted pre-flow;
every non-copy extender that cse2 cannot simplify (shift pairs, masks) materialises
bytes. Duplicating the `j++` latch into the exit arms does NOT get re-merged by
cross-jumping on this chassis (build_insns 174/175 vs 173 — see the kills), so it is
both an overshoot and a byte cost.

- [s19] Chassis re-measured at dispatch: memory/grind/func_8003DE14/candidate.c = 26 / 173 insns, chassis_s18_h1_head_exact_28.c = 28 / 173, both on HEAD 2026-09-10. No banked conclusion was chassis-void.

- [s19] h1 alloc table around the seat: factor nrefs 13 / livelen 56 / pri 6964 -> $t3; complement nrefs 11 / livelen 54 / pri 6111 -> $t4; j nrefs 11 / livelen 59 / pri 5593 -> $t5. The target needs factor $t3, j $t4, complement $t5 (asm/funcs/func_8003DE14.s:75-76, 104/112/120, 148-150).

- [s19] Closed form for the whole remaining h1 residual: global.c allocates in descending allocno_pri and hands each allocno the lowest free hard register, so the target seats are reachable iff pri(j) lands strictly inside the OPEN window (6111, 6964).

- [s19] Complete solution set for that window, given pri = floor_log2(nrefs)*nrefs*10000/livelen and flow.c's loop-depth weighting (+1 function level, +2 outer loop, +3 inner loop): nrefs(j) = 13 at livelen 59-61 (pri 6610 / 6393), or nrefs(j) = 14 at livelen 61 (pri 6885). nrefs 17 / 23 / 29 (the arm-duplication ladder) would need livelen 98-150, which exceeds the whole loop's live length (~60).

- [s19] Depth-2 reference sites for j partition into pre-loop (j is provably 0, so cse2 folds reads and deletes redundant stores before flow.c counts - measured, alloc tables unchanged) and post-loop (j is dead, so a store is a dead store flow.c deletes and a read needs a consumer that materialises bytes).

- [s19] The tension that makes this the last residual: reorg's delay-slot fill wants j's def to be the last pre-branch insn (h1, head byte-exact), but whichever of j / complement is defined FIRST gets the longer live range and hence the LOWER priority, so the def order that wins the slot is exactly the one that loses the seat. The incumbent 26 form takes the opposite trade (seats right, 4-insn head residual).

- [s19] target and build are both 173 insns in every form measured this session except the arm-duplication ladder (174/175), which confirms the residual remains purely register-naming plus the one reorg rotation.


## S20 (forensics) — the seat model is now complete, and two of s19's premises were wrong

Chassis re-measured at dispatch (all three reproduce, all 173 / 173 insns):
`candidate.c` = **26**, `chassis_s18_h1_head_exact_28.c` = **28**,
`chassis_s19_f1_jfirst_livelen61_30.c` = **30**.  Nothing was chassis-void.

### CORRECTION 1 — s19 labelled the incumbent's pseudos backwards

s19 recorded "complement pri 6111 -> $t4, j pri 5593 -> $t5" as if the *incumbent*
shared h1's seat inversion.  It does not.  Pseudo numbers follow **declaration
order**, and the incumbent declares `complement` BEFORE `j` inside the guard, so in
the incumbent p115 = complement and p116 = j — the opposite of h1, where `j` is a
head declaration (p115) and `complement` the guard declaration (p116).  Both alloc
tables print "p115 -> hardreg 12, p116 -> hardreg 13"-shaped rows, but they mean
opposite things:

    incumbent (26): ord=15 p116 = j          nrefs 11 LL 54 pri 6111 -> hardreg 12 = $t4  CORRECT
                    ord=16 p115 = complement nrefs 11 LL 55 pri 6000 -> hardreg 13 = $t5  CORRECT
    h1        (28): ord=15 p116 = complement nrefs 11 LL 54 pri 6111 -> hardreg 12 = $t4  WRONG
                    ord=16 p115 = j          nrefs 11 LL 59 pri 5593 -> hardreg 13 = $t5  WRONG

global.c allocates in DESCENDING allocno_pri and hands each allocno the lowest free
hard register, so the HIGHER-priced member of the pair takes $t4.  (tmp/grind/
func_8003DE14/s20/alloc_base26, alloc_h1, alloc_e2, alloc_e3, alloc_e4.)

Note for future probes: `allocno_compare` (tools/gcc-2.7.2/global.c:650-654) breaks an
exact priority TIE by lower allocno number, i.e. by declaration order — so the
declaration-site renumbering lever s16 proved byte-neutral WOULD decide this pair if
the two priorities were ever equal.  At nrefs 11/11 that requires equal live lengths,
and the two defs are adjacent by construction (see below), so a tie is not reachable.

### CORRECTION 2 — declaration order and ASSIGNMENT order are INDEPENDENT levers

s18's h5/h6 probe concluded "declaration-order respelling does not reach this pair".
That is true for the pseudo NUMBER, but the pair is decided by `allocno_pri`, and
**`reg_live_length` follows the ASSIGNMENT order** while the pseudo number follows the
DECLARATION order.  Measured, all 173 insns:

    e2  decl j,complement  / assign j,complement        score 30  (j LL 55, comp LL 54)
    e3  decl complement,j  / assign j,complement        score 30  (j LL 55, comp LL 54)
    e4  decl j,complement  / assign complement,j        score 26  (j LL 54, comp LL 55)
    base26 decl complement,j / assign complement,j      score 26  (j LL 54, comp LL 55)

So the rule is simply: **whichever of the pair is ASSIGNED first gets the longer live
range and therefore the LOWER priority**, and at equal nrefs the later-assigned one
takes $t4.  e4 is banked as `chassis_s20_e4_decl_assign_split_26.c` — a second,
independent 26 with the declaration/assignment orders split, which is the form to
perturb if a future probe needs the incumbent's seats with h1's pseudo numbering.

### The target's head block is SPLIT BY THE DIVISION'S TRAP CHECKS (new, decisive)

Reading asm/funcs/func_8003DE14.s:50-80 rather than the score: `factor`'s `div`
expands to `div $zero,$v0,$s2` followed by PsyQ's divide-by-zero / INT_MIN trap
checks (`bnez $s2` + `break 7`, `bne $s2,$at` + `lui` + `bne $v0,$at` + `break 6`),
each of which is a BRANCH.  The block that ends in `blez $v1` therefore contains
exactly TWO insns in the target: `mflo $t3` and the `blez` itself.  Consequences:

  * reorg.c's backward scan inside `fill_simple_delay_slots` has almost nothing to
    choose from in the target; the slot insn `addu $t4,$zero,$zero` is j's def and
    `subu $t5,$fp,$t3` (complement) immediately follows it at row 76.
  * in h1 our `move j,zero` sits in that same two-insn block (it is emitted after
    `factor` and before the guard compare), which is why the backward scan takes it
    and the head becomes byte-exact — confirming s18's attribution.
  * in the incumbent / e4 that block holds no eligible insn of its own, so reorg's
    backward scan (which runs BEFORE any fall-through attempt) walks further back and
    takes `addiu a2,sp,1040`, which is the 4-insn head residual.  e2/e3 (j assigned first inside the guard, so `move j,zero` IS the
    first fall-through insn) still score 30, i.e. reorg does NOT take it from the
    fall-through either — the fall-through route is not available here.

This makes the h1-vs-incumbent trade structural, not accidental: **j's def must sit
in the blez's own basic block to win the delay slot, and any def in that block
necessarily precedes complement's def, which necessarily makes LL(j) > LL(complement)
and hence pri(j) < pri(complement) at equal nrefs.**  Minimum observed gap is 1
insn (e2/e3: 55 vs 54, pri 6000 vs 6111), so even the tightest spelling loses.

### The target's own reference counts are IDENTICAL to ours (11 / 11)

asm/funcs/func_8003DE14.s: $t4 (j) appears exactly 3 times — row 75 `addu $t4,$zero,$zero`
(def, depth 2), row 148 `addiu $t4,$t4,1` (set+use, depth 3), row 150 `slt $v0,$t4,$t6`
(use, depth 3) = 2 + 6 + 3 = 11.  $t5 (complement) appears 4 times — row 76 `subu`
(def, depth 2) and rows 104 / 112 / 120 `mult` (depth 3) = 2 + 9 = 11.  So the ORIGINAL
source's post-combine reference structure is exactly ours; any difference must be in
references that existed before `flow.c` ran and were removed afterwards.

### THE BYTE-NEUTRAL reg_n_refs ADDER EXISTS, AND IT IS QUANTIZED AT +2*loop_depth

`toplev.c:2984` runs `flow_analysis` (which fills `reg_n_refs` / `reg_live_length`)
BEFORE `combine_instructions` at `toplev.c:3004`, and life analysis is never re-run —
so a reference on an insn that combine later folds away is counted permanently.
Measured this session with the `(S + P) - P` passenger form, where S is the real
subject and P the passenger whose count we want to lift:

    d3  `b_shift = (((bp + j) - j) + b_src) >> 5;`  (depth 3)
          build_insns 173  (BYTE-NEUTRAL)   nrefs(j) 11 -> 17, LL(j) 59 unchanged,
          pri 11525, j allocated at ord=11 and takes hardreg 9 ($t1).  score 33.
    d2  `jt = j + factor; jt = jt - factor;` on the latch operand (depth 3)
          build_insns 173  (BYTE-NEUTRAL)   nrefs(factor) 13 -> 19 (pri 13333),
          nrefs(j) UNCHANGED at 11.  score 34.
    d5  d3 on the f1 chassis: 173, nrefs(j) 17, LL 61, pri 11147.  score 35.

Two rules fall out, and they are the whole story of this axis:
  (a) the pattern is **ref-conserving for the subject and ref-DOUBLING for the
      passenger** — d2 proves it: spelling the chain with j as the subject moved j's
      latch reference onto the chain insn and left nrefs(j) at 11, while `factor`
      (the passenger) gained +6.  So to lift X, X must ride as the passenger.
  (b) the lift is **+2 per reference-pair times the loop depth**, i.e. +2 at depth 1,
      +4 at depth 2, +6 at depth 3.  There is no ODD lift: every identity that
      cancels a passenger reads it twice.

### …AND IT IS UNAVAILABLE FOR `j`, BECAUSE cse2 KNOWS `j == 0` EVERYWHERE OUTSIDE THE INNER LOOP

The window from s19 stands (pri(j) must land strictly inside (6111, 6964)).  Combined
with (b), the reachable prices for j are:

    nrefs 11          pri 330000/LL   window needs LL 48..53   (LL >= 55 forced, see above)
    nrefs 13 (+2 passenger) pri 390000/LL   window needs LL 56..63   <-- h1's LL 59 = 6610 HITS
    nrefs 15 (+4 passenger) pri 450000/LL   window needs LL 65..73
    nrefs 17 (+6 passenger) pri 680000/LL   window needs LL 98..111  (> whole loop)

The bullseye is therefore a DEPTH-1 passenger (+2) on the h1 chassis.  Measured dead:
every passenger site outside the inner loop is folded by cse2 before flow.c counts,
because `j` is a known constant 0 at every such point.  Three spellings, all 173 insns
with alloc tables byte-identical to h1's (nrefs(j) 11, LL 59, pri 5593, score 28):

    p1  `s32 complement = ((blend_base - factor) + j) - j;`   (depth 2, inside guard)
    p5  `if (((total + j) - j) > 0) {`                        (depth 2, the guard test)
    p8  `s32 blend_base = (0x1000 + j) - j;` with `s32 j = 0;` hoisted to the
        `if (count > 0)` block                                (depth 1)

and `(j - j)` (d4) is folded by fold-const at tree level before cse2 even sees it.
This closes the gap s19 left open: the +2/+4 lifts are not merely "hard to spell",
they are removed by the same cse2 constant-propagation that killed s19's c2/c4, and
the only depth at which a j passenger survives is depth 3, where the quantum is +6.

### The livelen side of the window is pinned in both directions

  * `factor` LL is 56 in EVERY head spelling measured across s19 and s20 — including
    factor first / second / third in the head (g1/g2/g3, all 28) and splitting the
    numerator into a named `s32 num = (i + 1) << 12;` local (g4, 28).  Its def is the
    `mflo` that the trap-check blocks pin immediately before the `blez`.
  * `complement` LL is 54 whenever it is assigned second and 55 whenever assigned
    first; it cannot exceed that because complement's live range is a strict SUBSET
    of j's whenever j's def is in the blez block, and extending it past the inner
    loop needs a post-loop USE (a def does not extend a range, and a dead store is
    deleted by flow.c:1490 before `mark_used_regs` runs, so it is never counted).
  * padding j's live range with byte-neutral neutral-passenger insns inside the guard
    (p6 `dst = (dst + total) - total;`, p7 also on `src`) does lengthen BOTH ranges
    (j 62 / 63, complement 55 / 56) but lifts the padded variables' own counts and
    re-prices the whole blend arm: 50 / 50, both 173 insns.

### Raw numbers, all measured on HEAD 2026-09-10, all 173 build / 173 target insns

    base26 26 | h1 28 | f1 30 | g1 28 | g2 28 | g3 28 | g4 28
    d1 28 | d2 34 | d3 33 | d4 28 | d5 35
    e2 30 | e3 30 | e4 26
    p1 28 | p2 30 | p5 28 | p6 50 | p7 50 | p8 28

- [s20] Chassis re-measured at dispatch: candidate.c 26/173, chassis_s18_h1_head_exact_28.c 28/173, chassis_s19_f1_jfirst_livelen61_30.c 30/173, all on HEAD 2026-09-10. No banked conclusion was chassis-void.
- [s20] s19's pseudo->variable mapping for the INCUMBENT was inverted: pseudo numbers follow DECLARATION order, so the incumbent (complement declared first) has p115 = complement and p116 = j, while h1 (j declared in the head) has p115 = j and p116 = complement. The incumbent's seats are correct (j pri 6111 -> $t4), h1's are not.
- [s20] reg_live_length follows ASSIGNMENT order and the pseudo number follows DECLARATION order, and the two are independent levers: e3 (decl complement,j / assign j,complement) = 30 and e4 (decl j,complement / assign complement,j) = 26. Whichever of the pair is assigned FIRST gets the longer live range and the lower priority.
- [s20] The target's `blez $v1` sits in a TWO-INSN basic block (`mflo $t3` + the blez), because `factor`'s division expands to PsyQ trap checks (`bnez $s2`/`break 7`, `bne $s2,$at`/`bne $v0,$at`/`break 6`) that split the head into four blocks (asm/funcs/func_8003DE14.s:50-80).
- [s20] Therefore j's def must sit in the blez's own block to win the delay slot, and any insn in that block precedes complement's def, so LL(j) > LL(complement) in every head-exact spelling; the minimum observed gap is 1 insn (e2/e3: 55 vs 54 -> pri 6000 vs 6111).
- [s20] The TARGET's own reference counts equal ours exactly: $t4 = 3 occurrences (def depth 2, addiu set+use depth 3, slt use depth 3) = 11 weighted; $t5 = 4 occurrences (def depth 2, three mults depth 3) = 11 weighted.
- [s20] flow_analysis (toplev.c:2984) runs once, BEFORE combine_instructions (toplev.c:3004), and is never re-run, so references on insns that combine later folds away are counted permanently in reg_n_refs and reg_live_length.
- [s20] The `(S + P) - P` chain extender is BYTE-NEUTRAL here (d2, d3, d5 all build 173 insns) and lifts the PASSENGER's reg_n_refs by 2 per loop-depth level while leaving the SUBJECT's count unchanged: d3 took nrefs(j) 11 -> 17 at depth 3, d2 took nrefs(factor) 13 -> 19 and left nrefs(j) at 11.
- [s20] The lift quantum is even by construction (every cancelling identity reads the passenger twice), so nrefs(j) can only be 11, 13, 15 or 17 — and 13 (depth 1) / 15 (depth 2) are unreachable because cse2 constant-folds any j-reading expression outside the inner loop, where j is provably 0: p1 (depth 2 on complement), p5 (depth 2 on the guard test) and p8 (depth 1 on blend_base) all return alloc tables byte-identical to h1's.
- [s20] nrefs(j) = 17 at LL 59-61 prices j at 11147-11525, which lifts it to ord=11 and hands it hardreg 9 ($t1) (d3, d5) — above the 10000/10909 allocnos that must stay ahead of it for the $t4 seat.
- [s20] livelen(factor) is 56 in every head spelling measured (g1 factor first, g2 second, g3 third, g4 numerator split into a named local) — its def is the `mflo` pinned before the blez by the trap-check blocks, so the window's upper bound 6964 cannot be raised from the head.
- [s20] Byte-neutral live-range padding inside the guard (p6 `dst = (dst + total) - total;`, p7 plus the same on `src`) lengthens j to 62/63 and complement to 55/56 but re-prices the blend arm: both score 50 at 173 insns.
- [s20] flow.c:1490 (`if (final && insn_is_dead)` -> PUT_CODE NOTE, `goto flushed`) is the predicate that makes every dead store invisible to reg_n_refs: the deletion happens before mark_set_regs / mark_used_regs run for that insn.

- [s20] Chassis re-measured at dispatch: candidate.c 26/173, chassis_s18_h1_head_exact_28.c 28/173, chassis_s19_f1_jfirst_livelen61_30.c 30/173, all on HEAD 2026-09-10; nothing was chassis-void.

- [s20] Pseudo numbers follow DECLARATION order and reg_live_length follows ASSIGNMENT order; the two are independent levers (e3 = decl complement,j / assign j,complement scores 30; e4 = decl j,complement / assign complement,j scores 26).

- [s20] s19's pseudo-to-variable mapping for the incumbent was inverted: the incumbent has p115 = complement and p116 = j, so its seats are already the target's; only h1's are inverted.

- [s20] The target's blez $v1 sits in a TWO-INSN basic block (mflo $t3 + the blez) because factor's division expands to PsyQ trap checks - bnez $s2 + break 7, bne $s2,$at, bne $v0,$at + break 6 - which split the head into four blocks (asm/funcs/func_8003DE14.s:50-80).

- [s20] The target's own reference counts equal ours exactly: $t4 (j) occurs 3 times (def depth 2, addiu set+use depth 3, slt use depth 3) = 11 weighted; $t5 (complement) occurs 4 times (def depth 2, three mults depth 3) = 11 weighted. Any difference from the original must therefore lie in references that existed before flow.c ran and were removed afterwards.

- [s20] flow_analysis (toplev.c:2984) runs once, before combine_instructions (toplev.c:3004), and is never re-run, so references on insns combine later folds away are permanent in reg_n_refs and reg_live_length.

- [s20] The (S + P) - P chain extender is byte-neutral here (d2, d3, d5 all 173 insns) and lifts the PASSENGER by +2 per loop-depth level while leaving the SUBJECT unchanged: d3 took nrefs(j) 11 -> 17, d2 took nrefs(factor) 13 -> 19 and left nrefs(j) at 11.

- [s20] The lift quantum is even because every cancelling identity reads the passenger twice, so nrefs(j) can only be 11, 13, 15 or 17; the window (6111, 6964) is hit by 13 at livelen 56-63 and by 15 at livelen 65-73, and missed by 17 (needs livelen 98-111).

- [s20] Every passenger site outside the inner loop is constant-folded by cse2 before flow.c counts, because j is provably 0 there: p1 (depth 2 on complement), p5 (depth 2 on the guard test) and p8 (depth 1 on blend_base) all return alloc tables byte-identical to their baseline and score 28 / 28 / 28.

- [s20] nrefs(j) = 17 at livelen 59-61 prices j at 11147-11525, which lifts it to ord=11 and hands it hardreg 9 ($t1) (d3, d5), above the 10000 / 10909 / 12000 allocnos that must stay ahead of it for the $t4 seat.

- [s20] livelen(factor) is 56 in every head spelling measured (g1 factor first, g2 second, g3 third, g4 numerator named as a local), because its defining mflo is pinned into the pre-blez block by the trap-check blocks.

- [s20] Byte-neutral live-range padding inside the guard (p6 `dst = (dst + total) - total;`, p7 also on src) lengthens j to 62/63 and complement to 55/56 but re-prices the blend arm: both 50 at 173 insns.

- [s20] allocno_compare (tools/gcc-2.7.2/global.c:650-654) breaks an exact priority TIE by lower allocno number, i.e. by declaration order - so s16's byte-neutral declaration-site renumbering lever decides any tied pair, and the newly-isolated assignment-order lever is what creates or destroys ties.

- [s20] Raw scores this session, all 173 build / 173 target insns: base26 26, e4 26, h1 28, g1 28, g2 28, g3 28, g4 28, d1 28, d4 28, p1 28, p5 28, p8 28, e2 30, e3 30, f1 30, p2 30, d3 33, d2 34, d5 35, p6 50, p7 50.


## s21 (forensics) - THE FLOOR MOVES 26 -> 16 (first drop since s13)

### Chassis re-measure at dispatch (kill re-audit)
candidate.c (s13 body) 26/173, chassis_s18_h1_head_exact_28.c 28/173, both on
HEAD 2026-09-11.  No banked conclusion was chassis-void.

### THE s20 DEPTH-1/DEPTH-2 KILL WAS AN INSTANCE KILL AND IT IS NOW OVERTURNED
s20 measured p1 / p5 / p8 (passenger sites on `complement`, the guard test, and
`blend_base`) and concluded the +2 / +4 lift on `j` is unavailable because cse2
constant-folds any j-reading expression outside the inner loop, where j is
provably 0.  That is true of every site s20 tried - and every one of them sits
BEFORE the inner loop.  AFTER the inner loop, j is the loop-EXIT value: it is
not a known constant, cse2 cannot fold the chain, and flow.c counts it.

    q3  `LoadImage((s32)rect, ((s32)dst_buf + j) - j);`   on the h1 chassis
        build_insns 173 (BYTE-NEUTRAL), nrefs(j) 11 -> 15, livelen(j) 59 -> 73,
        pri 5593 -> 6164 - INSIDE the s19 window (6111, 6964).
        alloc: ord=15 pseudo=115 (j) hardreg=12 ($t4)
               ord=16 pseudo=116 (complement) hardreg=13 ($t5)   = TARGET SEATS
        score 28 -> 23, and rows 0..87 of the function are now byte-identical
        to asm/funcs/func_8003DE14.s.  The head residual that has been on the
        frontier since s13 is CLOSED.

Carrier sweep for the same lift (all 173 insns, h1 chassis):
    q1  `new_y = (((u16*)rect)[1] + j) - j + ((u16*)rect)[3];`  nrefs 15 LL 62
        pri 7258 -> ord 14, ahead of factor.  31
    q4  same on the other addend                                31
    q2  `i = ((i + j) - j) + 1;`  nrefs 15 LL 73 pri 6164 but re-prices the
        whole outer loop                                        60
    q3  the LoadImage dst argument                              23   <-- winner
    r1  j REUSED as the new_y carrier (sanctioned variable-reuse family):
        nrefs 17 LL 62 pri 10967 -> ord 11, 172 insns            39
    r3  j REUSED to stage the LoadImage argument: nrefs 15 LL 59
        pri 7627 -> ord 14                                       31
The window is narrow in BOTH directions: the lift must be exactly +4 at
loop-depth 2 AND must extend livelen far enough that 450000/LL lands under
factor's 6964.  Only the LoadImage-argument site does both.

### THE BLEND ARM'S 19-INSN SEAT RESIDUAL IS ALSO A PRIORITY ORDERING, AND IT
### YIELDS TO THE SAME LEVER
px = pseudo 122 (nrefs 12 / livelen 11 / pri 32727) is allocated at ord=2 and
takes hardreg 3 ($v1); g_src = pseudo 126 (nrefs 12 / livelen 12 / pri 30000)
follows at ord=5 and takes hardreg 4 ($a0).  The target has them the other way
round.  A depth-3 passenger on g_src prices it at 60000:

    u3  `b_shift = (((bp + b_src) + g_src) - g_src) >> 5;`   on the q3 chassis
        build_insns 173 (BYTE-NEUTRAL), nrefs(g_src) 12 -> 18, pri 60000
        alloc: ord=1 p126 (g_src) hardreg=3 ($v1)   = target
               ord=3 p122 (px)    hardreg=4 ($a0)   = target
               ord=6 p123 (r_src) hardreg=5 ($a1)   = target (unchanged)
        score 23 -> 16.

### ABLATION MATRIX (every form 173 build / 173 target insns, HEAD 2026-09-11)
    base26  s13 incumbent, no lift .......... 26
    h1      s18 head-exact body, no lift .... 28
    q3      h1 + j lift ..................... 23
    y1      h1 + g_src lift ................. 22
    y2      base26 + g_src lift ............. 19
    u3      h1 + BOTH lifts ................. 16   <- new candidate.c
The two levers are independent and compose.

### PSEUDO -> VARIABLE MAP, read from the .combine dump (not inferred)
Insn numbers are combine-pass numbers (tmp/grind/func_8003DE14/dumps/*.combine):
    p118 = `pixel` in the i == count-1 ARM (insns 151/155/159), nrefs 9, LL 3
    p121 = `pixel` in the blend arm (190/192/199/214/258)
    p122 = px      (192/195/217/220)
    p123 = r_src   (215/227/230/233)
    p126 = g_src   (218/238/241/244)
    p128 = b_src   (221/249/252/255)
    p130 = rp (227/233)  p131 = gp (238/244)  p132 = bp (249/255)
    p115 = j (h1 numbering), p116 = complement (h1 numbering)
This corrects the standing assumption that p118 is `count - 1`: it is the FIRST
arm's pixel load, and it is what blocks $v0 for px and g_src.

### WHAT THE REMAINING 16 IS
13 insns of b_src seat cascade (rows 106-108, 114-119, 122, 123) + 3 insns of
trip test (rows 127/128/130).  b_src takes hardreg 2 ($v0) because $v0 is free
and non-conflicting at its allocation point; the target seats it on $a0, the
register px vacates one insn earlier (px dies row 105, b_src is defined row
106 - they do not conflict and CAN share, which is exactly what the target
does).  Forcing $a0 requires a CONFLICTING allocno to be holding $v0 across
b_src's range.

Measured dead this session for that seat (all on the u3 chassis unless noted):
    v1  depth-3 passenger on b_src itself: pri 65454, ord=1 - STILL hardreg 2.
        b_src's seat is not a priority question, it is a conflict question.  16
    v2  passenger on px (pri 65454, ord=1)                              23
    v3  passenger on r_src (pri 55384, ord=2; px -> 5, b_src -> 2)      23
    v4  passenger on bp (p132 -> ord=3, spilled to local alloc)         16
    w1/w2  ONE shared `sum` local for all three channel sums: it DOES become a
        global allocno (p136, nrefs 18, pri 90000) and DOES take hardreg 2, and
        it DOES push b_src off $v0 - but it conflicts with all three channels
        and scatters g_src to $a2 / b_src to $a3.                       52 / 52
    x1/x2  three per-channel `r_sum` / `g_sum` / `b_sum` locals: once-written,
        once-read, so combine folds them back into the shift; they never appear
        in the allocno table and the alloc table + score are unchanged. 16 / 23
    t1..t4  moving the b_src COMPUTATION later in the arm (four sites, from
        immediately after `r_src = r * factor` to immediately before
        `bp = b_src * complement`) with the DECLARATION pinned so pseudo
        numbers do not move: all four produce alloc tables that are identical
        to q3's row for row, including livelen(px) = 11.  Statement order
        inside the blend arm does not move any live range.            23 x4
    u1/u2  hoisting the g_src (and b_src) DECLARATION out of the blend arm to
        sit before `px` (to win the global.c:654 lower-pseudo tie-break):
        restructures the arm, 116 build insns.                        100 / 100

### MECHANISM, NAMED
flow_analysis (toplev.c:2984) -> combine_instructions (toplev.c:3004), life
analysis never re-run.  reg_n_refs is loop-depth weighted; global.c
allocno_compare (tools/gcc-2.7.2/global.c:650-654) orders by descending
pri = floor_log2(nrefs)*nrefs*10000/livelen and find_reg gives each allocno the
lowest-numbered non-conflicting hard register.  Both levers this session are
pure global.c ordering changes bought with references that combine deletes.

- [s21] Chassis re-measured at dispatch: candidate.c (s13 body) 26/173 and chassis_s18_h1_head_exact_28.c 28/173 on HEAD 2026-09-11; no banked conclusion was chassis-void.

- [s21] NEW FLOOR 16/173, the first drop since s13 (eight sessions at 26). Ablation matrix, every form 173 build / 173 target: base26 26, h1 28, q3 (h1 + j lift) 23, y1 (h1 + g_src lift) 22, y2 (incumbent + g_src lift) 19, u3 (both lifts) 16.

- [s21] s20's kill of the +2/+4 depth-1/depth-2 passenger lift on j was an INSTANCE kill and is overturned: every site s20 measured sits BEFORE the inner loop, where cse2 proves j == 0 and folds the chain. A site AFTER the inner loop reads the loop-EXIT value, which cse2 cannot fold.

- [s21] q3 (LoadImage((s32)rect, ((s32)dst_buf + j) - j);) takes nrefs(j) 11 -> 15, livelen(j) 59 -> 73, pri 5593 -> 6164 - inside the s19 window (6111, 6964) - and seats j on $t4 (ord 15) and complement on $t5 (ord 16) while keeping 173 build insns.

- [s21] On the q3 chassis the whole head region is closed: rows 0..87 of the built function are byte-identical to asm/funcs/func_8003DE14.s, including the blez delay slot move t4,zero and subu t5,s8,t3. The 4-insn head residual that has been on the frontier since s13 no longer exists.

- [s21] u3 adds b_shift = (((bp + b_src) + g_src) - g_src) >> 5;: nrefs(g_src) 12 -> 18, pri 60000, so global.c allocates g_src at ord=1 (hardreg 3, $v1) and px at ord=3 (hardreg 4, $a0) - the target's pair - with r_src unchanged at $a1. Score 23 -> 16.

- [s21] PSEUDO -> VARIABLE MAP read from the .combine dump (not inferred): p118 = the pixel load in the i == count-1 ARM (insns 151/155/159, nrefs 9, livelen 3), p121 = pixel in the blend arm, p122 = px (192/195/217/220), p123 = r_src (215/227/230/233), p126 = g_src (218/238/241/244), p128 = b_src (221/249/252/255), p130/131/132 = rp/gp/bp. This corrects the standing assumption that p118 is count - 1; p118 is what blocks $v0 for px and g_src.

- [s21] The remaining 16 insns are 13 of b_src seat cascade (rows 106-108, 114-119, 122, 123) plus 3 of trip test (rows 127/128/130). b_src (p128) takes hardreg 2 ($v0) because $v0 is free and non-conflicting at its allocation point; the target seats it on $a0, the register px vacates one insn earlier - px dies at row 105, b_src is defined at row 106, so they do not conflict and CAN share, which is exactly what the target does.

- [s21] b_src's seat is a CONFLICT question, not a priority question: v1 lifts b_src to pri 65454 / ord=1 and it still takes hardreg 2. Forcing $a0 requires a conflicting allocno holding $v0 across b_src's range - the target's $v0 holds all three channel sums (rows 111, 114, 118).

- [s21] Once-written / once-read named locals for the channel sums are folded back by combine and never become allocnos (x1/x2: alloc table and score unchanged); a single shared sum local DOES become an allocno at $v0 but conflicts with all three channels and scores 52.

- [s21] Statement order inside the blend arm was inert for live ranges across four placements of the b_src computation (t1-t4), with the declaration pinned: alloc tables identical row for row including livelen(px) = 11.

- [s21] Both new levers are F1 combine-foldable chain extenders (owner ruling 2026-07-01) and are /* FAKE */-annotated in candidate.c with mechanism and lever-exhaustion pointers. No ordinary-C carrier for either price has been found; six carriers were swept for the j lift and only the LoadImage dst argument lands the window.


## s22 (rederive) - floor 16 -> 14

### Measurements (all on HEAD 2026-09-11, post -mel / post -msoft-float; every
### form 173 build insns against 173 target insns, so all of these are
### byte-neutral restructurings of the blend arm)

    u3   s21 incumbent (blue reuses b_src for b*factor)            16
    a1   blue PRODUCT reuses b_src, b*factor into bp               38
    a2   blue written as one expression, `bp` local dropped        15   <- -1
    a3   all three channels invert the reuse                       50
    b1   fully inline arm, no channel locals at all                43
    c1   `px` reused as the blue carrier                           23
    c2   a2 minus the g_src chain extender                         23
    c3   a2 minus the j chain extender                             21
    c5   neither chain extender                                    28
    d1   shared `sum` local for ALL THREE channel sums             30
    d2   shared `sum` for the GREEN and BLUE sums only             14   <- -1
    d4   d1 minus the g_src extender                               43
    e1   output word accumulated into a named `out` before blue    48
    e2   `out` opened after red                                    41
    e3   `out` opened at the top of the arm                        36
    f1   shared `sum` for RED and GREEN                            30
    f2   `sum` for RED only (single-use)                           15
    f3   `sum` for red and blue                                    17
    g1   blue chained through ONE variable to the mask             31
    g2   g1 + the shared sum                                       55
    g4   g1 minus the g_src extender                               31
    h1   latch `rect[3] * rect[2]`                                 14
    h2   latch `(s32)rect[2] * rect[3]`                            14
    h3   latch `rect[2] * rect[3] > j`                             14
    h4   `total` re-read inside the latch                          16
    i1   red sum operand flip                                      14
    i2   green sum operand flip                                    14
    i3   blue sum operand flip                                     30
    i4   all three flipped                                         30

Logs: tmp/grind/func_8003DE14/s22/batch_results.json, per-form allocno tables in
tmp/grind/func_8003DE14/s22/alloc_<tag>/stderr.log.

### FACT 1 - the incumbent's blue-channel variable reuse was costing 1 insn.
s21's body wrote `bp = b_src * complement; b_src = b * factor;` - reusing b_src
for the second product.  Dropping `bp` and writing the channel as one expression
(`b_shift = (b_src * complement + b * factor) >> 5`) is worth 1 insn (16 -> 15)
and makes our rows 106-108 structurally identical to the target's (b_src and the
complement product share one register in both builds; only WHICH register
differs).  Inverting the reuse instead (product into b_src, b*factor into bp,
form a1) costs 22.

### FACT 2 - the target's channel sums are in $v0 because they are LOCAL
### quantities, not because of any global priority.
Target rows 111/114/118 are `addu $v0,...` with the shift moving the value to a
different register (`sra $a1,$v0,0xf`).  Ours coalesces each sum into the dying
channel register (`addu $a1,$t2,$a1` / `sra $a1,$a1,0xf`).  Naming the sums
changes this, but only when the name is written MORE THAN ONCE: a single-use
`sum` (f2, red only) is folded back by combine and the alloc table is unchanged,
while a multi-write `sum` survives as a global allocno.  A shared sum across red
and green (f1) or all three (d1) DOES reproduce the target's `addu $v0` /
`sra $a1,$v0` pair on rows 111-112 - but it adds one entry to the allocno order,
and every lower-priority allocno shifts one hard register (complement
$t5 -> $t6, factor $t3 -> $t4, rp $t2 -> $t3), which costs more than the rows it
buys (30 vs 14).  Sharing only GREEN and BLUE (d2) is the one split that buys a
row without adding an allocno: 14.

### FACT 3 - the remaining 11 blend insns are ONE register choice, read directly
### out of global.c.
`BB2_FINDREG_DEBUG=139` on the d2 body (tmp/grind/func_8003DE14/s22/
findreg_d2_139/stderr.log) prints, for the blue temp's last find_reg call:
    conflicts:        3 5 6 7 8 16 29          (neither 2 nor 4)
    someone_prefers:  (empty)
    used_so_far:      0..16 24..29 31
    pass0_used:       0 1 3 5 6 7 8 16 17..23 26..31
Regno 2 ($v0) is absent from pass0_used, so find_reg's first-fit loop
(global.c:1053-1080) returns 2; the target's seat is 4 ($a0), the register px
vacates at row 105.  Only two routes put 2 into pass0_used:
  (a) an allocno already assigned $v0 that CONFLICTS with the blue temp.  The
      only $v0 holder today is pseudo 118 (the or-chain accumulator, pri 90000,
      rows 120-125), which does not overlap the blue temp's 106-118 range.
      Extending the accumulator backwards by naming it (`out`, forms e1-e3)
      breaks the px/g_src/r_src triple instead (g_src leaves $v1) and costs
      22-34.  Making the sums a $v0-holding global allocno (d1/f1) works but
      pays the allocno-shift tax above.
  (b) `regs_someone_prefers` - but global.c set_preference (global.c:1671) only
      fires on a reg-reg copy where one side is already hard-numbered
      (`reg_renumber[x] >= 0`), i.e. it needs a LOCALLY-allocated $v0 pseudo
      copied into a global allocno.  There is no such copy in the arm today.
The structural reading: the target has no extra global allocno here, so its sums
must be local quantities that local-alloc seats on $v0 BEFORE global_alloc runs,
which is exactly what makes 2 a hard-reg conflict for the blue temp.  In our
build the red sum is instead merged by local-alloc's combine_regs into r_src's
quantity - pseudo 123 carries nrefs 12 / livelen 13 and sits on $a1, i.e. it is
the merged {r_src, red sum, r_ch} quantity.  Breaking that merge, without adding
a global allocno, is the next lever.

### FACT 4 - the 3-insn trip-test residual is POSITIONAL, not expression-driven.
Target `lh $v0,4($s0) / lh $v1,6($s0) / mult $v0,$v1`; ours `lh $v1,4 /
lh $v0,6 / mult $v1,$v0`.  Writing the latch as `rect[3] * rect[2]` (h1) swaps
which OFFSET each load carries but leaves $v1 on the first load and $v0 on the
second - so the register pair is attached to the emission slot, not to the
operand.  `rect[2] * rect[3] > j` (h3) and `(s32)rect[2] * rect[3]` (h2) are
byte-identical to the incumbent latch.  Re-reading `total` in the latch (h4)
costs 2.  Operand flips on the three channel adds are inert for red and green
(i1/i2, 14) and cost 16 on blue (i3/i4, 30).

### FACT 5 - both s21 chain extenders are still load-bearing on the new chassis.
On the d2 body the g_src extender is worth 9 (c2 = 23) and the j extender 7
(c3 = 21); with neither, the body is back at the s18 head-exact 28 (c5).  So the
14 floor is still FAKE-carrying and this body is NOT submittable.

- [s22] NEW FLOOR 14/173 on HEAD 2026-09-11 (d2 body, saved as memory/grind/func_8003DE14/candidate.c): the s18 head-exact chassis + the two s21 F1 chain extenders + a re-derived blend arm in which the blue channel does not reuse its source variable and the green and blue channel sums are staged through one shared `sum` local.

- [s22] Every form measured this session built 173 insns against 173 target insns, so all 30 of them are byte-neutral restructurings and the whole sweep is a pure register/ordering comparison.

- [s22] Lever accounting on the new chassis: d2 14; d2 minus the g_src chain extender (c2) 23; d2 minus the j chain extender (c3) 21; neither extender (c5) 28 = the s18 head-exact body. Both s21 FAKE extenders are still load-bearing, so the 14 floor is FAKE-carrying and NOT submittable.

- [s22] A named sum carrier only survives if it is written more than once: f2 (red only, once-written/once-read) is folded back by combine and leaves the allocno table and the score unchanged, while d1/f1/d2 (two or three writes) become real carriers.

- [s22] A shared sum that becomes a GLOBAL allocno reproduces the target's rows 111-112 exactly (`addu $v0,$t3,$a1` / `sra $a1,$v0,0xf`) but adds one entry to allocno_order, shifting every lower-priority allocno one hard register (complement $t5 -> $t6, factor $t3 -> $t4, rp $t2 -> $t3); net 30 vs 14. The target therefore cannot have an extra global allocno there - its sums must be local quantities local-alloc seats on $v0 before global_alloc runs.

- [s22] In our build the red sum is merged by local-alloc's combine_regs into r_src's quantity: pseudo 123 carries nrefs 12 / livelen 13 on $a1 and emits `addu $a1,$t2,$a1` / `sra $a1,$a1,0xf` where the target emits `addu $v0,$t2,$a1` / `sra $a1,$v0,0xf`.

- [s22] BB2_FINDREG_DEBUG=139 on d2 (tmp/grind/func_8003DE14/s22/findreg_d2_139/stderr.log): conflicts {3,5,6,7,8,16,29}, regs_someone_prefers EMPTY, regs_used_so_far {0..16,24..29,31}, pass0_used {0,1,3,5,6,7,8,16,17..23,26..31}. Regno 2 is free, so find_reg's first-fit loop returns $v0; the target's seat is 4 ($a0).

- [s22] global.c set_preference (tools/gcc-2.7.2/global.c:1671) only fires when one side of a reg-reg copy is already hard-numbered (reg_renumber >= 0), so the regs_someone_prefers route into pass0_used requires a LOCALLY-allocated $v0 pseudo copied into a global allocno - there is no such copy in the blend arm today.

- [s22] The trip-test register pair is positional, not expression-driven: `rect[3] * rect[2]` swaps the two load offsets but still puts $v1 on the first load and $v0 on the second.

- [s22] s21's frontier item 1 (enumerate source shapes that put a MULTI-referenced value in $v0 across rows 106-118 without adding an insn) is now answered: the shapes exist (d1/f1/d2), they are byte-neutral, and only the green+blue split avoids the allocno-shift tax. s21's frontier item 3 (latch spellings decide the trip test) is killed.
