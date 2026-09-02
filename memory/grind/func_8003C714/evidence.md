# Evidence bank — func_8003C714

## s1 (2026-09-01, recon) — floor 104 (no body) → 33 (clean respell of old pinned form) → **15**

### Function identity
Small init routine: `s0 = func_80077D00()`; three lib calls; then a 3-iteration
loop copying "time records" from `D_80106A58` (stride-8 records: `u8 c` at +0,
`s32 ticks` at +4) into byte fields at `s0 + 0x21 + i*4 + {0,1,2,3}` as
(ticks/1800, ticks/30%60, ticks%30*100/30, c) — i.e. min/sec/frac/char of 3
best-time entries (4-byte record array at s0+0x21); then `func_8001CD68(buf)`
readback of 4 bytes into s0+0x2D..0x2F, `s0+0x30 = (u16)D_80101ED2`,
`disp_SetFramebufferMode(1,0,0,0)`, two global stores, `gpu_DisableDisplay()`.
Tail + prologue/epilogue match trivially; ALL divergence ever seen is in the
loop preheader + body.

### Measured floors (all `sandbox func_8003C714 --disable all`, this chassis)
| form | score |
|---|---|
| pre-migration body (register-asm pins t0/a2/a1 — CHEATS, superseded) | 33 |
| clean plain-pointer spelling (src+=8, dst+=4, i mid-body) | 33 |
| index-derived pointers (`src=(u8*)&D+i*8; dst=(u8*)s0+i*4`), i mid-body | 37 |
| index-derived pointers, `i += 1` LAST statement | **15 (candidate.c)** |
| + `tp = (s32*)(src+4)` named pointer for the 3 s32 reads | 15 (byte-identical; insn_count unchanged — cse folds the +4) |
| + `(i+i)*4` / `(i+i)+(i+i)` index spellings | 15 (cse canonicalizes to shifts; count 56→57 only) |
| + `i+=3; i-=1; i-=1;` split increment (DIAGNOSTIC ONLY — F6-banned shape) | 28, and the movable STILL moved at 59 real insns |

### Mechanism map (from `tmp/grind/func_8003C714/dumps/code6cac_c2.loop` at each step)
1. **Plain-pointer spelling is structurally dead:** bivs src(+8)/dst(+4) get
   strength-reduced the WRONG way — dst's biv 75 is ELIMINATED into a combined
   giv biased to `s0+36` (stores at -3..0) and src grows a SECOND IV for src+4
   (giv reduced to its own reg + own addiu 8). Target has unbiased offsets
   0x21..0x24 / 4,0 and single src IV.
2. **Index-derived pointers give the target giv structure:** `src`/`dst`
   computed from `i` each iteration are GENERAL givs (mult 8/4 add base);
   strength_reduce reduces them to reduced-giv regs (init `addu a1,s0` /
   `lui+addiu a2`, steps 8/4, increments at loop end — target delay-slot
   `addiu a1,a1,4` exactly), replaces all address uses IN PLACE (offsets
   preserved), and deletes the in-loop sll/addu computations. The i*8/i*4
   sub-givs are "not worth while, 0 vs N" (ignored, deleted).
3. **`i += 1` must be last:** mid-body increment makes uses after it read the
   next giv value → loop.c/flow inserts `move a0,a3` copy (that was the 15→37
   regression); with i++ last, sched1 hoists the addiu into the 4th mult's
   mult→mfhi latency shadow, matching target insn 66.
4. **d15 residual is the 0x91A2B3C5 (/1800 magic) placement + one seat:**
   - build: const hoisted to preheader (2 asm insns `lui t1/ori t1`), mfhi temp
     lands t2.
   - target: const IN-LOOP as `lui v0 / lw v1 / ori v0 / mult v1,v0` —
     INTERLEAVED, which proves the const was TWO separate RTL insns at sched1
     in the ORIGINAL compile (an assembler `li` macro expands adjacent; only
     the compiler scheduler can put the lw between). With the hoist gone, t1
     frees and the mfhi temp (target t1) plausibly falls into place → d0.
5. **The hoist is NOT killable by loop size (measured + rule-confirmed):**
   `move_movables` takes a movable when `threshold*savings*lifetime >= insn_count`,
   threshold = `2*(1+n_non_fixed_regs)` = **122** call-free on this chassis
   (hard-float config counts 32 FP regs; n_non_fixed_regs == 60 — measured and
   documented in `.claude/rules/defeat-licm-hoist-var-reuse.md` §threshold).
   Confirmed empirically: savings-1/life-1 movable still moved at 59 real
   insns. Inflating the body count is unwinnable (needs ≥123).
6. **Movable-admission attack surface (the sanctioned lever family) doesn't
   directly reach this const:** the multi-set-reuse lever
   (defeat-licm-hoist-var-reuse, SOTN-sanctioned "variable reuse") works on
   C-nameable invariants; the /1800 magic constant is generated internally by
   expand_divmod→force_reg as a fresh single-set pseudo (RTL dump insn 55,
   single `(set reg (const_int -1851608123))` at expand) and cannot be routed
   through a C variable.
7. **The split (lui-part + iorsi3-0xB3C5, same reg) form of the const appears
   in OUR build only in the preheader**, created at/after the loop pass's move
   re-emission (absent from .rtl/.cse/.loop/.combine greps, present from .sched
   on, insns ~219-224). So the mechanism that splits big consts exists in this
   cc1; the question is what makes it fire IN-LOOP without the move.
8. `reg_single_usage` substitution path (loop.c:~735): in loops WITH calls,
   single-set single-use invariant regs get their use REPLACED by the source
   and the insn deleted — for a big const feeding mult, validate_replace_rtx
   must fail (mulsi3 rejects const operand), so it falls back to normal
   movable handling. Our loop has no call anyway.

### Artifacts (this session)
- `tmp/grind/func_8003C714/s1/` — diff.sh (rebuildable side-by-side differ),
  build.txt/target.txt, premigration.c (the old pinned body), loopinsns.py
  (lists post-cse loop-region RTL).
- `tmp/grind/func_8003C714/dumps/` — full cc1 pass dumps (regenerate with
  `pwsh tools/grinder/dump.ps1 func_8003C714`; dumps reflect the LAST measured
  spelling — the split-increment diagnostic, NOT candidate.c).
- Read loop decisions with:
  `awk '/^;; Function func_8003C714/{f=1} f&&/^;; Function/&&!/func_8003C714/{exit} f' tmp/grind/func_8003C714/dumps/code6cac_c2.loop | grep -E "Loop from|savings|giv|biv"`

- [s1] Honest floor THIS session: 15 (sandbox --disable all, candidate.c body in src). Old queue d33 came from a register-pin body; a clean respell of it also scores 33 without pins.

- [s1] All 15 residual diffs are the const hoist (2 preheader insns + missing 2 in-loop insns + alignment shift) and the mfhi temp seat t2-vs-t1; t1 is currently occupied by the hoisted const, so killing the hoist plausibly closes to d0.

- [s1] Movable threshold on this chassis is 122 call-free / 61 with-call (n_non_fixed_regs=60, hard float; .claude/rules/defeat-licm-hoist-var-reuse.md documents the same numbers). Empirically confirmed: savings-1 life-1 movable still moved at 59 real insns.

- [s1] The multi-set-reuse (sanctioned variable-reuse) lever cannot reach this invariant: the /1800 magic is an internal expand_divmod force_reg pseudo, not C-nameable.

- [s1] The split const form (lui-part + iorsi3, same reg) is created by the loop pass's own move re-emission in our build (absent through .combine grep, present from .sched), i.e. the splitter exists; trigger-in-loop unknown.

- [s1] dst field layout: 4-byte record array at s0+0x21 (min/sec/frac/char per entry); src records stride 8 (u8 at +0, s32 ticks at +4) at D_80106A58.

---

## s2 (2026-09-01, structural) — floor 15 unchanged on this chassis; **root cause identified and proven exactly**

### The residual is ONE scalar in the build config, not a C spelling

`candidate.c` (unchanged, ordinary C, zero cheats) measures **distance 0** when
cc1 is given either of two flags, and **15** without them:

| cc1 flags | `score_func(o, build/src/code6cac_c2.o, func_8003C714)` |
|---|---|
| project `CC_FLAGS` (as shipped) | **15** (build 105 insns vs target 104) |
| `CC_FLAGS -msoft-float` | **0** (104 == 104) |
| `CC_FLAGS -ffixed-$f0 … -ffixed-$f31` (32 flags) | **0** (104 == 104) |

Both flag sets produce a BYTE-IDENTICAL `code6cac_c2.o` (sha1 `525998f989…`),
so the entire effect is register-file bookkeeping, not the float ABI.

### Mechanism, read out of loop.c and confirmed in the dumps
`loop.c:532`  `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs);`
`loop.c:1631` a movable is hoisted iff `threshold * savings * lifetime >= insn_count`
`loop.c:1719 / 1904` after EACH successful move: `threshold -= 3;`
`regclass.c:380-387` `n_non_fixed_regs` = count of `!fixed_regs[i]` hard regs.

`-msoft-float` (via mips.h CONDITIONAL_REGISTER_USAGE) and `-ffixed-$fN` both
mark the 32 FP hard registers fixed, so `n_non_fixed_regs` goes **60 → 28** and
the initial threshold goes **122 → 58**.

func_8003C714's loop is `Loop from 25 to 146: 56 real insns` with exactly three
movables (from `.loop`, identical set on both chassis):

| movable | what it is | life | savings | hard (T=122) | soft (T=58) |
|---|---|---|---|---|---|
| insn 33, regno 78 | `%hi/%lo(D_80106A58)` base | 1 | 1 | 122≥56 → **moved** (T→119) | 58≥56 → **moved** (T→55) |
| insn 46, regno 84 | **0x91A2B3C5** (/1800 magic) | 1 | 1 | 119≥56 → **moved** (T→116) | 55<56 → **not desirable** |
| insn 60, regno 91 | 0x88888889 (/30 magic) | 31 | 1 | 116·31≥56 → moved | 55·31≥56 → moved |

That is the whole d15: the `threshold -= 3` decrement after the FIRST move puts
the soft-float threshold (55) one unit below the loop's insn_count (56), so the
single-use short-lifetime /1800 magic stays in the loop and is re-emitted there
in the split `lui`+`ori` form that sched1 interleaves with the first `lw` —
exactly target insns 17-20 (`lui v0 / lw v1 / ori v0 / mult v1,v0`), and the
freed register lets the mfhi temp land in `$t1` as the target has it. The /30
magic (lifetime 31) and the D_80106A58 base still hoist on both chassis, which
is why the target preheader keeps them.

### Why no C spelling can reach it on the shipped chassis
On the hard-float chassis reg 84 faces `threshold = 119` at its turn, so the
loop would need `insn_count > 119` RTL insns to lose the hoist. The whole target
function is 104 machine instructions and the loop body is ~40; a >119-insn loop
cannot exist in it. `lifetime` and `savings` are both already at their minimum
(1 and 1). This closes the axis that s1 left open as "attack ADMISSION or const
FORM": there is no admission escape either — the const pseudo is compiler-
generated (`! REG_USERVAR_P && ! REG_LOOP_TEST_P` ⇒ admitted unconditionally at
`loop.c:697-700`) and the `reg_single_usage` substitution path needs a loop with
a call.

### Project-wide effect of the flag (measured, all 32 C stems)
Rebuilt every stem twice (with/without the flag) and sha1-compared the objects:
**30 of 32 stems byte-identical.** Only two differ:
- `code6cac_c2` — 43 of 44 functions identical; `func_8003C714` **15 → 0**.
- `code6cac_b` — 87 of 88 functions identical; **`func_800324D0` 0 → 3**.

`func_800324D0` regresses by the SAME mechanism in reverse: its loop is
`Loop from 66 to 270: 72 real insns` and its first movable (insn 78, regno 85,
life 1, savings 1 — the `li 255` compared in the loop) is hoisted on the
hard chassis (122 ≥ 72) but "not desirable" on the soft chassis (58 < 72). The
3-insn diff is exactly `li t0,255` leaving the preheader plus the branch-target
shift.

**The two functions are mutually exclusive under any single threshold:**
func_8003C714 needs `threshold < 56` at its second movable, func_800324D0 needs
`threshold >= 72` at its first. No value of `n_non_fixed_regs` satisfies both,
so at most one of the two committed C bodies has the original loop size. On the
hard chassis func_8003C714 is unreachable at any loop size; on the soft chassis
func_800324D0 becomes reachable again if its loop can be spelled at ≤58 RTL
insns (currently 72). That asymmetry is the evidence that the soft/28-register
count is the self-consistent one.

### Artifacts (s2)
- `tmp/grind/func_8003C714/s2/dump_extra.py`, `dump_extra2.py` — cc1 `-da` dump
  with arbitrary extra flags into `s2/dumps_<tag>/`.
- `tmp/grind/func_8003C714/s2/dumps_soft/` — the `-msoft-float` pass dumps for
  code6cac_c2 (`.loop` carries the "not desirable" line).
- `tmp/grind/func_8003C714/s2/dumps_bsoft/` — same for code6cac_b (func_800324D0).
- `tmp/grind/func_8003C714/s2/build_soft.py`, `score_soft.py` — build+score one
  stem with extra flags.
- `tmp/grind/func_8003C714/s2/softfloat_sweep.py`, `ffixed_sweep.py` — the
  32-stem project-wide neutrality sweep (`sweep/`, `sweep_ffixed/`).
- `tmp/grind/func_8003C714/s2/b_diff.py`, `c2_diff.py`, `d0_cmp.sh` —
  per-function scoring of the two differing stems.
- `tmp/grind/func_8003C714/s2/ffixed.py` — the `-ffixed-$fN` ABI-neutral proof.

- [s2] func_8003C714's honest floor on the SHIPPED chassis is 15 and is C-foreclosed: the residual is the loop.c movable threshold (122 vs the 56-insn loop), and no C spelling can push a 104-instruction function's loop past 119 RTL insns.
- [s2] candidate.c is byte-exact (distance 0) under `-msoft-float` or 32 `-ffixed-$fN`; both give a byte-identical object, so the effect is purely `n_non_fixed_regs` 60->28 -> threshold 122->58.
- [s2] The flag is byte-neutral for 30 of 32 C stems and for 43/44 functions in code6cac_c2; the single cost is func_800324D0 (code6cac_b) going 0 -> 3 by the same threshold rule in reverse.

## §s2b (2026-09-01, structural) — re-verification + two measured kills

Chassis re-measured at HEAD this session (the brief reported "measurement
unavailable"): **floor 15**, target 104 / build 105, with `candidate.c` spliced
over the `INCLUDE_ASM` line. `src/code6cac_c2.c` was restored to HEAD before the
session ended; the candidate is unchanged and remains the best known form.

| probe | file | insn_count | score | verdict |
|---|---|---|---|---|
| baseline (candidate.c) | `memory/grind/func_8003C714/candidate.c` | 56 | 15 (105) | best known |
| `do{}while(0)` around the /1800 store | `rejected/dowhile0-inner-loop-is-phony.c` | 56 | 15 (105) | KILLED — `Loop from 43 to 68 is phony.` |
| inline index-derived addresses (no `src` local) | `rejected/inline-index-addr-insn-count-costs-bytes.c` | 58 | 28 (108) | KILLED — +6.5 score per +1 RTL insn |

Key dump lines (`tmp/grind/func_8003C714/dumps/code6cac_c2.loop`, regenerated
per probe by `pwsh tools/grinder/dump.ps1 func_8003C714`):
- baseline: `Loop from 25 to 146: 56 real insns.` + `Insn 33 / regno 78`,
  `Insn 46 / regno 84`, `Insn 60 / regno 91` all `moved to`.
- do-while(0): `Loop from 43 to 68 is phony.` then `Loop from 25 to 160: 56 real
  insns.` with the identical three-movable table — the wrap changes nothing.
- inline-address: `Loop from 25 to 166: 58 real insns.`

Compiler-source citations used (all in `tools/gcc-2.7.2/loop.c`):
loop.c:344-345 (`moved_once` is allocated once per FUNCTION in `loop_optimize`,
so an inner-loop hoist would carry over — the mechanism is real, it is the phony
classification that blocks it), loop.c:532 (threshold), loop.c:573 (phony-loop
message), loop.c:791 (`m->lifetime` / `m->savings`), loop.c:1221
(`force_movables`), loop.c:1594 (safe-to-move guard), loop.c:1609-1611
(`insn_count *= 2`), loop.c:1631 (desirability), loop.c:1719 + 1904
(`threshold -= 3`), loop.c:1912 (`moved_once[regno] = 1`).

Artifacts: `tmp/grind/func_8003C714/s2/apply.py` (splices a candidate body over
the `INCLUDE_ASM` line or over a previous body), `vA_dowhile0_1800.c`,
`vB_inline_index_addr.c`, and `tmp/grind/func_8003C714/dumps/code6cac_c2.loop`.

- [s2b] The do{}while(0) -> moved_once doubling frontier item is KILLED: degenerate loops are PHONY (loop.c:573) and scan_loop never runs on them, so moved_once is never set. No ruling-request is owed for the do-while-zero-exception scope question.
- [s2b] The insn_count axis is measured dead: +2 RTL insns costs +13 score; reaching the required insn_count >= 120 is impossible inside a 104-instruction function.
- [s2b] s2's movable table and floor-15 claim were independently reproduced this session from a fresh dump — the integration-handoff claim rests on re-verified measurements, not on an inherited assertion.

- [s2] HEAD chassis re-measured this session (the brief reported 'measurement unavailable'): with candidate.c spliced over the INCLUDE_ASM line, sandbox func_8003C714 --disable all prints score 15, target_insns 104, build_insns 105.

- [s2] The .loop dump reproduces s2's movable table verbatim: Loop from 25 to 146: 56 real insns; Insn 33 regno 78 (life 1) moved to 203; Insn 46 regno 84 (life 1) moved to 205; Insn 60 regno 91 (life 31) moved to 207. Regno 84 is the 0x91A2B3C5 /1800 magic — the sole residual.

- [s2] do{}while(0) around the /1800 store measures 15 (unchanged) and produces 'Loop from 43 to 68 is phony.' (tools/gcc-2.7.2/loop.c:573) — loop_optimize never calls scan_loop on a back-edge-less loop, so moved_once[regno] (loop.c:1912) is never set and the insn_count *= 2 branch (loop.c:1609-1611) cannot fire. This kills the whole degenerate-loop family, and with it the only remaining need for a do-while-zero-exception ruling on this function.

- [s2] Inflating the loop by ordinary C costs ~6.5 score points per +1 RTL insn (56 -> 58 insns took the score 15 -> 28), so the insn_count >= 120 required by loop.c:1631 at threshold 119 is unreachable inside a 104-instruction function.

- [s2] savings == 1 and lifetime == 1 are both at their structural floor for regno 84 (loop.c:791: lifetime = uid_luid[last_uid] - uid_luid[first_uid], and the const's set is adjacent to its single mult use); lifetime cannot be 0 because a reg used only by its own setting insn does not exist.

- [s2] moved_once is allocated once per FUNCTION in loop_optimize (loop.c:344-345), not per loop — so the cross-loop mechanism s1 hypothesised is real; it is the PHONY classification that blocks it, which is why the probe had to be measured rather than reasoned about.

- [s2] The bytes are proven: candidate.c is 100% ordinary C (no cheats, no /* FAKE */, no inline asm, no register pins) and scores distance 0 (104 == 104) under -msoft-float or the 32 -ffixed-$fN flags, both of which emit a byte-identical code6cac_c2.o. Mechanism: loop.c:532 threshold = 2 * (1 + n_non_fixed_regs) is 122 with the FP regs free and 58 with them fixed; at 58 the post-hoist 55 falls one under the loop's insn_count 56 and the /1800 magic stays in-loop as the target's split lui/ori.

- [s2] The blocking surface is CC_FLAGS (Makefile + engine/buildconfig.py) — explicitly outside a grind session's allowed surface. That makes this an INTEGRATION HANDOFF, not an exhaustion claim: the function should stay ACTIVE.

- [s2] The previous session's identical handoff was discarded on a title-regex technicality, not on substance: grind.ps1:1063 requires the decisions.md header line naming the function to carry OWNER-ESCALATION or CANONICAL-ASM GRANT PATH, while grind.ps1:1148 (which handles INTEGRATION HANDOFF) runs later in the chain. The re-filed entry at docs/grind/decisions.md:19622 carries both markers on the header line.

- [s2] grindlib.py:568 states 'Integration handoffs and gate-PASSING escalations stay legal anywhere', so this disposition is legal in structural modality and is not a self-applied exhaustion claim.

## s3 (2026-09-01, structural modality) — measurements

All measurements taken with `memory/grind/func_8003C714/candidate.c` (or the
listed variant) spliced over the `INCLUDE_ASM("asm/funcs", func_8003C714);` line
at `src/code6cac_c2.c:629`; `src/code6cac_c2.c` was restored to HEAD at the end
of the session.

| # | Form | sandbox score | build_insns | `.loop` insn_count | regno-84 movable |
|---|---|---|---|---|---|
| 1 | `candidate.c` (baseline, chassis re-check) | 15 | 105 | 56 | moved to 205 |
| 2 | `rejected/byte-neutral-masks-fold-before-loop.c` (`& 0xFF` on all four stores) | 15 | 105 | **56 (+0)** | moved |
| 3 | `s3/body_callC.c` (baseline + one call in the loop) | — | — | 57 | moved |
| 4 | `s3/body_callD.c` (K8 inlined-address respelling + one call in the loop) | 55 | 116 | **59** | **not desirable** |

Baseline movable table (form 1), identical to s2's:
```
Loop from 25 to 146: 56 real insns.
Insn 33: regno 78 (life 1), move-insn savings 1  moved to 203
Insn 46: regno 84 (life 1), move-insn savings 1  moved to 205
Insn 60: regno 91 (life 31), move-insn savings 1  moved to 207
```

Form-4 movable table (`tmp/grind/func_8003C714/s3/callD_loop_table.txt`):
```
Loop from 25 to 169: 59 real insns.
Insn 36: regno 77 (life 50), move-insn savings 2  moved to 226
Insn 45: regno 84 (life 1), move-insn savings 1 not desirable
Insn 66: regno 95 (life 31), move-insn savings 1  moved to 228
Insn 142: regno 138 (life 1), move-insn forces 36 savings 1  moved to 230
```

Form-4 emitted code around the un-hoisted constant
(`tmp/grind/func_8003C714/s3/callD_inloop_split_const.txt`,
`mipsel-linux-gnu-objdump -d tmp/sandbox/func_8003C714/code6cac_c2.o`):
```
     d98:  3c0291a2   lui   v0,0x91a2
     d9c:  8e230000   lw    v1,0(s1)
     da0:  3442b3c5   ori   v0,v0,0xb3c5
     da4:  00620018   mult  v1,v0
     da8:  00004010   mfhi  t0
```
Target, `asm/funcs/func_8003C714.s:8003C754..8003C764`:
```
  lui  $v0, (0x91A2B3C5 >> 16)
  lw   $v1, 0x4($a2)
  ori  $v0, $v0, (0x91A2B3C5 & 0xFFFF)
  mult $v1, $v0
  mfhi $t1
```
Instruction-for-instruction identical (the base register and the mfhi
destination differ only because form 4 carries the diagnostic call). This is the
first POSITIVE reproduction of the target's residual shape on the shipped
chassis: the split, sched1-interleaved constant is what this cc1 emits whenever
`loop.c:1631` declines the movable — no chassis change is required to obtain the
shape, only to obtain the decision.

Source lines verified this session in `tools/gcc-2.7.2/loop.c` (quoted in
hypotheses.md H7): 344, 532, 791, 793, 1609-1613, 1631, 1719, 1904, 1912.

- [s3] Chassis re-check: candidate.c in place of the INCLUDE_ASM line at src/code6cac_c2.c:629 measures score 15, target_insns 104, build_insns 105 — unchanged from s1/s2; the .loop movable table (`Loop from 25 to 146: 56 real insns.` / regno 78 moved to 203 / regno 84 moved to 205 / regno 91 (life 31) moved to 207) reproduces s2's verbatim.

- [s3] POSITIVE reproduction of the residual shape on the shipped chassis: with the movable declined, objdump of the sandbox object shows `lui v0,0x91a2 / lw v1 / ori v0,0xb3c5 / mult v1,v0 / mfhi` — identical to target 8003C754..8003C764. The C body is right; only the loop.c decision differs.

- [s3] loop.c source-verified this session: 532 (threshold formula), 791 (lifetime), 793 (savings = n_times_used, hence always >= 1), 1609-1613 (moved_once doubling), 1631 (desirability), 1719/1904 (threshold -= 3), 344/1912 (moved_once is per-function and only set on a real move).

- [s3] Byte-neutral redundant masks add 0 to insn_count (56 -> 56) because they fold at expand/cse1, before loop.c counts the loop — the zero-cost-inflation channel is empty.

- [s3] With loop_has_call the movable is declined at insn_count 59 (measured: `not desirable`) and still moves at 57 — so the exact residual condition on the shipped chassis is: (loop_has_call AND insn_count >= 59) OR (no call AND insn_count >= 120). The first is excluded by the absence of any jal in the target loop; the second by the target loop being 59 machine instructions inside a 104-instruction function.

- [s3] The two constant movables in the target are structurally IDENTICAL inputs to loop.c:1631 (both savings 1, both lifetime 1) — 0x88888889 is hoisted to the preheader and 0x91A2B3C5 is not — so they can only be separated by the `threshold -= 3` decay the first hoist applies. That pins the original compile's initial threshold to the window [56,58] and identifies the residual as a build-configuration scalar with no C-side input.

- [s3] src/code6cac_c2.c was restored to its HEAD content (INCLUDE_ASM) at the end of the session; the only dirty tracked file is metrics/events.jsonl.

## s4 (2026-09-01) — permuter modality: a validated permuter workspace, a flat basin, and the corrected joint-channel arithmetic

### The permuter workspace (new infrastructure, reusable)
No permuter workspace had ever existed for this function. One was hand-built
this session and, critically, VALIDATED against the ledger before it was trusted:

  build script : tmp/grind/func_8003C714/s4/mkws.sh
  compile.sh   : tmp/grind/func_8003C714/s4/compile.sh   (copied into the workspace)
  workspace    : tmp/perm_8003C714_s4/  (base.c, compile.sh, settings.toml, target.o)

Pipeline mirrored from the Makefile exactly, because a permuter that compiles
with different flags searches a different problem:
  * `mipsel-linux-gnu-cpp` with the project's full CPP_DEFS **plus `-DPERMUTER`**
    — include/include_asm.h:33 makes `INCLUDE_ASM` a no-op under `PERMUTER`, which
    is what stops the sibling `asm/funcs/*.s` files from being `.include`d into
    the permuter's assembly step. Without it the workspace does not build at all.
  * `tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1
    -mno-abicalls -fno-builtin -w -mel` (`-mel` is load-bearing per the Makefile).
  * `prologue_fix` -> `maspsx` with the full MASPSX_FLAGS set -> the
    `code6cac_c2` RODATA_ALIGN2 `sed 's/.align\t3/.align\t2/'` -> `multu_pad`.
  * Function extraction: the maspsx output labels are NOT tab-indented
    (`.globl\tfunc_8003C714` / `.ent\tfunc_8003C714` / `.end\tfunc_8003C714` all
    start at column 0), so the awk range must anchor on `^\.globl[ \t]+<fn>$` ..
    `^\.end[ \t]+<fn>$`. The `tools/mar_perm_workspace.sh` template anchors on
    `^\t\.ent\t<fn>$`, which silently matches nothing and hands the assembler the
    whole file tail — the visible symptom is a cascade of
    `macro used $at after ".set noat"` plus
    `.size expression for <fn> does not evaluate to a constant`.
  * Do NOT prepend a `.set noat` / `.set noreorder` prelude to the extracted
    compiler output: maspsx already emits its own `.set` directives inline and
    the extra prelude is what triggers the `$at` cascade. (The prelude IS needed
    for `target.s`, which is glabel-macro asm.)

Validation before launch — base object vs target object, per-function objdump
diff, was EXACTLY the known d15 residual and nothing else:
    < lui t1, / < ori t1,t1,        (our hoisted 0x91A2B3C5, preheader)
    > lui v0, / > ori v0,v0,        (target's in-loop split pair)
    < mfhi t2 / > mfhi t1  (x4)     (the downstream seat)

### The campaign
  launch : python3 tools/permuter_campaign.py launch --func func_8003C714 \
             --dir tmp/perm_8003C714_s4 --label s4-candidate-basin -j 8
  base_score 590 (permuter's weighted metric; the objdump-distance is 15)
  waits  : three in-turn `wait` calls, 553.5 s / 552.2 s / 551.8 s,
           iterations 19,709 -> 43,363 -> 65,711, `novel: []` every time
  harvest: --stop, elapsed_s 1776.1, iterations 66,016,
           finds_total 0, finds_new 0, best_new_score null, procs_killed 9

Sixty-six thousand randomized spellings did not once improve on the base score.
That is the strongest available empirical statement that the residual has no
C-side input, and it is consistent with the source-level account in H7/K9.

### The lever switch: the joint threshold/insn_count sweep
Per the fresh-seed rule the flat basin was closed and the session changed lever
rather than reseeding. The new lever came from re-reading the two `threshold -= 3`
sites (tools/gcc-2.7.2/loop.c:1719 and loop.c:1904) and noticing that the
inherited K9 arithmetic treats threshold reduction and insn_count inflation as
independent, when in fact each added invariant movable moves BOTH terms of
loop.c:1631 in the favourable direction.

Sweep artifacts: tmp/grind/func_8003C714/s4/gen_k.py (body generator),
sweep_k.sh / sweep_k2.sh (drivers), tmp/grind/func_8003C714/s4/kdumps/
(body_k<k>.c, k<k>.i, k<k>.s, k<k>.i.loop for k = 0,4,8,12,13,14,15,16,20,24).

| k  | insn_count | 0x91A2B3C5 movable | verdict         |
|----|-----------|--------------------|-----------------|
| 0  | 56        | regno 84           | moved to 205    |
| 4  | 66        | regno 89           | moved to 238    |
| 8  | 74        | regno 93           | moved to 264    |
| 12 | 82        | regno 97           | moved to 290    |
| 13 | 84        | regno 98           | **not desirable** |
| 14 | 86        | regno 99           | **not desirable** |
| 15 | 88        | regno 100          | **not desirable** |
| 16 | 90        | regno 101          | **not desirable** |
| 20 | 98        | regno 105          | **not desirable** |
| 24 | 106       | regno 105/…        | **not desirable** |

k16.s confirms the payoff shape directly: `li $2,-1851654144` (0x91A2B3C5) sits
INSIDE the loop, immediately after the loop-top label `.L133:`, feeding
`mult $3,$2` — reproducing the target's construct on the SHIPPED chassis with no
call in the loop, which K11 could not do.

Why it is still foreclosed: only movables that are actually MOVED pay the
`threshold -= 3`, and loop.c:1700-1717 emits each moved movable into the loop
preheader. Thirteen extra preheader instructions are required ahead of the magic;
the target preheader 8003C73C..8003C750 is six instructions and every one is
accounted for (biv init, the single hoisted 0x88888889 movable as lui+ori, and
two strength-reduction giv inits). Available room: zero.

- [s4] Chassis re-check at session start: candidate.c spliced over the INCLUDE_ASM line at src/code6cac_c2.c:629 measures score 15 / target_insns 104 / build_insns 105 with `sandbox func_8003C714 --disable all`. The ledger floor of 15 is chassis-current. src/ was restored to the INCLUDE_ASM line before the session closed; the working tree carries no src edits.

- [s4] First permuter workspace ever built for this function, and it is validated rather than assumed: tmp/perm_8003C714_s4, recipe committed to tmp/grind/func_8003C714/s4/mkws.sh + compile.sh. Its base-vs-target objdump diff is exactly the d15 residual and nothing else.

- [s4] Two reusable workspace gotchas discovered and written up in evidence.md: (1) the cpp step needs -DPERMUTER, because include/include_asm.h:33 no-ops INCLUDE_ASM under it -- without that the sibling asm/funcs/*.s files get .include'd and the workspace cannot build; (2) maspsx emits `.globl`/`.ent`/`.end` at column 0, so the function-extraction awk must anchor on `^\.globl[ \t]+<fn>$` .. `^\.end[ \t]+<fn>$`, not on the tab-indented `^\t\.ent\t<fn>$` that tools/mar_perm_workspace.sh uses (the mismatch silently hands the assembler the whole file tail and shows up as a `macro used $at after ".set noat"` cascade). Also: do not prepend a .set noat/.set noreorder prelude to extracted compiler output -- maspsx emits its own.

- [s4] Campaign telemetry: base_score 590, elapsed 1776.1 s, 66,016 iterations, finds_total 0, finds_new 0, best_new_score null, stopped with --stop and 9 processes killed. No campaign outlives this session.

- [s4] loop.c:1719 and loop.c:1904 were read this session: BOTH `threshold -= 3` sites are unconditional and fire on every move (the first for the move-insn path, the second for the consec-copy path).

- [s4] The k-sweep movable tables (tmp/grind/func_8003C714/s4/kdumps/k<k>.i.loop) give the crossover to the insn: k=12 insn_count 82 -> moved; k=13 insn_count 84 -> not desirable; k=16 insn_count 90 with threshold 122-3*11=89 -> not desirable.

- [s4] tmp/grind/func_8003C714/s4/kdumps/k16.s places `li $2,-1851654144` (0x91A2B3C5) inside the loop under `.L133:` feeding `mult $3,$2` -- the target's 8003C754..8003C760 construct, on the shipped chassis, with no call in the loop.

- [s4] Foreclosure arithmetic, unchanged in conclusion but now with an exact number: 13 additional hoisted preheader instructions are required ahead of the magic; the target preheader 8003C73C..8003C750 is 6 instructions with room for 0.

- [s4] Banked this session: memory/grind/func_8003C714/rejected/invariant-hoist-threshold-decrement-needs-13-preheader-insns.c (the k=13 diagnostic body, explicitly labelled a cheat and a diagnostic, never a candidate). candidate.c is unchanged as C and carries a new s4 header note. hypotheses.md and evidence.md both carry full s4 sections.
