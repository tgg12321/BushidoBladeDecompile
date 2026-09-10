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
