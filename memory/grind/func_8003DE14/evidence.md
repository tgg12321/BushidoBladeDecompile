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
