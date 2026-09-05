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

## s5 (2026-09-01) — synthesis modality: measurements

### Chassis re-check (first action of the session)
`memory/grind/func_8003C714/candidate.c` spliced over the `INCLUDE_ASM` line at
`src/code6cac_c2.c:629`; `& tools/wteng.ps1 main sandbox func_8003C714 --disable all`:

    "score": 15,  "target_insns": 104,  "build_insns": 105

Unchanged from s1-s4. (The dispatch brief reported "HEAD honest floor:
measurement unavailable"; that is a driver-side gap, not chassis drift.)
`src/code6cac_c2.c` was restored to the `INCLUDE_ASM` line before close.

### Pass order, read from source this session
`tools/gcc-2.7.2/toplev.c` `rest_of_compilation`:
`2865 cse_main` -> `2866 delete_dead_from_cse` -> `2895 loop_optimize` ->
`2926 cse_main (cse2)` -> `2984 flow_analysis` -> `3004 combine_instructions` ->
`3028 schedule_insns`.
`insn_count` is computed by `count_loop_regs_set` (`loop.c:2989-3092`) as a bare
`++count` over every insn of `GET_RTX_CLASS == 'i'` in the loop range — no
liveness filter, JUMP_INSN and CALL_INSN included. Two DCE-capable passes run
after it (`flow_analysis`, `combine_instructions`) and one runs before it
(`delete_dead_from_cse`).

### Sweep harness (validated)
`tmp/grind/func_8003C714/s5/sweep.sh <generator> <tag> <k...>` — generates a
whole-TU variant, preprocesses it with the project cpp flags, compiles with the
shipped `tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -mcpu=3000 -mips1
-mno-abicalls -fno-builtin -w -mel -dL`, then reports the `.loop` dump's
`Loop from A to B: N real insns.` line, the count of `moved to` /
`not desirable` movables, and the emitted asm line count for func_8003C714.

POSITIVE CONTROL — `sweep.sh gen_k.py ctl 0 12 13 16` (s4's K14 generator):

    k=0   Loop from 25 to 146: 56 real insns.  moved, 0 not-desirable   asm 107
    k=12  Loop from 25 to 207: 82 real insns.  moved, 0 not-desirable   asm 157
    k=13  Loop from 25 to 211: 84 real insns.  2 not-desirable          asm 159
    k=16  Loop from 25 to 225: 90 real insns.  7 not-desirable          asm 173

Reproduces K14 exactly, so the nulls below are real nulls.

### Measurement 1 (K15) — dead in-loop computation, `sweep.sh gen_dead.py dead 0 4 8 16 32`
Body adds k chained dead ALU ops on a loaded value (`dead = *(s32 *)(src + 4);
dead ^= C1; dead += C2; ...`), `dead` never read.

    k=0   Loop from 25 to 146: 56 real insns.   asm 107
    k=4   Loop from 25 to 167: 56 real insns.   asm 107
    k=8   Loop from 25 to 185: 56 real insns.   asm 107
    k=16  Loop from 25 to 221: 56 real insns.   asm 107
    k=32  Loop from 25 to 293: 56 real insns.   asm 107

`insn_count` pinned at 56; emitted function byte-identical. The UID range grows
(146 -> 293) because deleted insns keep their UIDs as NOTEs — positive proof the
insns were generated and then removed by `delete_dead_from_cse` at toplev.c:2866,
i.e. BEFORE loop.c counts.

### Measurement 2 (K16) — address/giv chains, `sweep.sh gen_chain.py ch 0 1 2 4 8 16`
Body replaces `dst[0x21] = v/1800;` with a chain `q0 = dst + 1; q1 = q0 + 1; ...;
q<k-1>[0x21-k] = v/1800;` — real SETs with real uses, so cse1 DCE cannot remove
them, and all givs of biv `i` with identical stride (the `combine_givs` shape).

    k=0   Loop from 25 to 146: 56 real insns.   asm 107
    k=1   Loop from 25 to 149: 56 real insns.   asm 107
    k=2   Loop from 25 to 152: 56 real insns.   asm 107
    k=4   Loop from 25 to 158: 56 real insns.   asm 107
    k=8   Loop from 25 to 170: 56 real insns.   asm 107
    k=16  Loop from 25 to 194: 56 real insns.   asm 107

`insn_count` pinned at 56; emitted function byte-identical. cse1 folds the entire
add chain into the single memory reference, so the construct never reaches
loop.c — the post-loop passes (`flow_analysis`, `combine_instructions`) are never
handed anything to delete.

### Source re-read: the `may_not_move` admission paths (loop.c:3035-3044)
`count_loop_regs_set` sets `may_not_move[regno]` in three situations: an explicit
`CLOBBER` of the reg; the reg being set in TWO basic blocks of the loop; the reg
being set twice in one basic block with a use in between. All three would keep
the 0x91A2B3C5 constant in the loop. All three require a handle on the MAGIC
pseudo, which `expand_divmod` creates via `force_reg` and sets exactly once, in
one basic block, with no C-level name. Closed — this is the source-level
confirmation of what K9 asserted.

### Artifacts
- `tmp/grind/func_8003C714/s5/sweep.sh` — the harness
- `tmp/grind/func_8003C714/s5/gen_dead.py`, `gen_chain.py`, `gen_k.py` (control)
- `tmp/grind/func_8003C714/s5/dumps/` — per-k `.c`, `.i`, `.s`, and `-dL` `.loop`
  dumps for all three sweeps
- `memory/grind/func_8003C714/rejected/dead-locals-deleted-before-loop-counts-them.c`
- `memory/grind/func_8003C714/rejected/giv-pointer-chain-folds-at-cse1.c`

- [s5] Chassis re-check (first action): candidate.c spliced over the INCLUDE_ASM line at src/code6cac_c2.c:629, `sandbox func_8003C714 --disable all` prints score 15, target_insns 104, build_insns 105 — identical to s1-s4. The brief's 'HEAD honest floor: measurement unavailable' is a driver-side gap, not chassis drift. src/code6cac_c2.c was restored to the INCLUDE_ASM line before close (git status clean on src/).

- [s5] Pass order read from tools/gcc-2.7.2/toplev.c this session: 2865 cse_main -> 2866 delete_dead_from_cse -> 2895 loop_optimize -> 2926 cse2 -> 2984 flow_analysis -> 3004 combine_instructions -> 3028 schedule_insns. loop.c's insn_count is therefore taken with one DCE pass already behind it and two DCE-capable passes still ahead — the exact asymmetry that makes the free-insn channel look plausible and, measured, closes it.

- [s5] loop.c:2989-3092 (count_loop_regs_set): insn_count is a bare ++count over every GET_RTX_CLASS=='i' insn in the loop range — JUMP_INSN and CALL_INSN included, no liveness filter. This is the first time the counter's definition (as opposed to the desirability test that consumes it) has been read into the ledger.

- [s5] loop.c:3035-3044 re-read: may_not_move[regno] is set by (a) an explicit CLOBBER of the reg, (b) the reg being set in TWO basic blocks of the loop, (c) the reg being set twice in one basic block with a use in between. All three would keep the 0x91A2B3C5 constant in-loop; all three need a handle on the magic pseudo, which expand_divmod creates via force_reg and sets exactly once in one basic block with no C-level name. This is the source-level confirmation of what s2's K9 asserted about admission.

- [s5] K15 sweep (dead ALU chains, k=0/4/8/16/32): insn_count 56 at every k; emitted asm 107 lines at every k; loop UID range grows 146 -> 293, proving the insns were generated and then deleted before loop.c counted.

- [s5] K16 sweep (pointer/giv chains, k=0/1/2/4/8/16): insn_count 56 at every k; emitted asm 107 lines at every k.

- [s5] Positive control on the same harness (s4's gen_k.py): k=12 -> 82 insns / 157 asm lines / all moved; k=13 -> 84 / 159 / two 'not desirable'; k=16 -> 90 / 173 / seven 'not desirable'. K14 reproduced exactly.

- [s5] Merged foreclosure arithmetic for the next session, in one line: the residual is loop.c:1631 `threshold * savings * lifetime >= insn_count` with savings and lifetime pinned at 1 (H7), threshold 119 after the first hoist, insn_count 56; every route to flipping it (insn_count alone +64 — K8/K10/K15/K16; threshold alone 23 hoists — K9; the compound route 13 hoists — K14; loop_has_call — K11; moved_once doubling — K7/K12; admission — K9 + this session's may_not_move read) is now measured, and each is foreclosed by the same 104-instruction target / 59-instruction loop / 6-instruction preheader budget.


## s6 (2026-09-01, synthesis) -- reachability measured; H8 falsified

Chassis: candidate.c spliced -> score 15, 104 target / 105 build (unchanged from
s1-s5). src/code6cac_c2.c restored to the INCLUDE_ASM line before the session
closed.

### The measurement that changes the function

| form | carrier | .loop insn_count | emitted | sandbox score |
|---|---|---|---|---|
| candidate.c (baseline) | -- | 56 | 105 insns | 15 |
| gen_biv k=1 (`j += 1`, test on j) | test biv | 57 | 107 asm lines | -- |
| gen_multi k=8 / 16 / 32 | test biv | 64 / 72 / 88 | 107 asm lines (byte-identical) | -- |
| gen_multi k=64 | test biv | 120 ("not desirable") | 104 insns | 22 |
| gen_bal k=32 (balanced, stride 1) | test biv | 121 ("not desirable") | 104 insns | 22 |
| gen_swap k=32 (names/roles swapped) | test biv | 121 | 104 insns | 22 |
| gen_bal2 / gen_noisei k=32 | giv-driving biv `i` | 120 / 121 | 108 / 109 asm lines | -- |
| **gen_fv k=32 (post-loop-use biv `z`)** | **post-loop biv** | **121** | **104 insns** | **0** |

The score-22 rows are NOT partial matches in the usual sense: the emitted
instruction SEQUENCE is already the target's, 104 for 104, and all 22 differences
are one cyclic register rotation (target dst=a1 src=a2 magic=a3 counter=t0; those
builds counter=a1 dst=a2 src=a3 magic=t0). `.lreg` gives the cause in one line --
`Register 73 used 263 times across 58 insns` -- the noise carrier's reg_n_refs is
local-alloc's priority key. Moving the carrier to a biv whose only use is
post-loop removes the pseudo before local-alloc runs and the rotation disappears.

### The score-0 form (inadmissible; banked as proof, not as a candidate)

    i = 0; z = 0;
    do {
        src = (u8 *)&D_80106A58 + i * 8;
        dst = (u8 *)s0 + i * 4;
        dst[0x21] = *(s32 *)(src + 4) / 1800;
        z += 3; z += 7; ... (32 balanced `z += c` / `z -= c` pairs, net 0,
                             spread across the four store statements)
        ...
        z += 1;
        i += 1;
    } while (i < 3);
    ...
    D_800A37B8 = z - 3;      /* loop.c final-value-replaces z with 3 -> stores 0,
                                which is the store the target already has */

`sandbox func_8003C714 --disable all` -> **score 0, 104 == 104**. It fails cheat
tests T1 (the pairs sum to zero and have no observable effect), T2 (no human
writes this), T3 (the mechanism IS loop.c:1631's insn_count) and T6, and matches
no frozen SOTN family, so it was not submitted.

### Why this was invisible to s1-s5

Every earlier free-insn_count probe (K10 redundant masks, K15 dead ALU chains,
K16 giv/address chains) was BASIC-BLOCK-LOCAL, and cse1 + delete_dead_from_cse
(toplev.c:2865-2866) are a redundancy fixpoint sitting immediately before
loop_optimize (2895). Loop-carried arithmetic is the one class cse1 cannot value-
number away, and strength_reduce's biv elimination -- inside loop_optimize,
after move_movables has consumed insn_count -- removes it for free.

### Artifacts

- `tmp/grind/func_8003C714/s6/sweep.sh` (harness, cloned from the s5 driver)
- `tmp/grind/func_8003C714/s6/gen_biv.py`, `gen_multi.py`, `gen_bal.py`,
  `gen_bal2.py`, `gen_swap.py`, `gen_noisei.py`, `gen_fv.py`
- `tmp/grind/func_8003C714/s6/body_mi64.c`, `body_bal32.c`, `body_sw32.c`,
  `body_fv32.c` (the score-0 form)
- `tmp/grind/func_8003C714/s6/mi64.dis`, `bal32.dis`, `cmp.py` (target-vs-build
  instruction comparator)
- `tmp/grind/func_8003C714/s6/dumps/` (per-k .i/.s/.loop dumps for every sweep)
- `tmp/grind/func_8003C714/dumps/code6cac_c2.lreg` (the reg_n_refs evidence)

- [s6] Chassis re-checked before anything was spent: candidate.c spliced -> score 15, target_insns 104, build_insns 105 — identical to s1-s5, so every inherited conclusion was chassis-current at session start.

- [s6] loop.c:1631 is `threshold * savings * m->lifetime >= insn_count` with savings and lifetime pinned at 1 (loop.c:791/793) and threshold 122 minus 3 per prior move (loop.c:1719/1904) = 119 at the 0x91A2B3C5 movable's test. insn_count must therefore reach 120. This session reached it: 121.

- [s6] The free channel measured end to end: gen_multi k=1..32 raises the loop from `Loop from 25 to 146: 56 real insns.` to 88 real insns with the emitted function byte-identical (107 asm lines) at every k — +32 RTL insns for +0 bytes.

- [s6] score-0 body (inadmissible): i and z both start at 0; 32 balanced `z += c; z -= c;` pairs spread across the four store statements; `z += 1; i += 1;` at the bottom; `while (i < 3)`; and the single post-loop use `D_800A37B8 = z - 3;` which loop.c final-value-replaces to the `D_800A37B8 = 0` the target already has.

- [s6] The intermediate score-22 forms already emit the target's exact 104-instruction SEQUENCE; the only difference is a one-position cyclic register rotation, and tmp/grind/func_8003C714/dumps/code6cac_c2.lreg names the cause: `Register 73 used 263 times across 58 insns` (the carrier's reg_n_refs is local-alloc's priority key).

- [s6] A second free channel exists and is uncharacterised: gen_biv.py k=2 (`j += 7`, exit test `j != 21`) raised the dump's `moved to` count from 8 to 9 — one EXTRA hoisted movable — with the emitted function still 107 asm lines. Each extra hoist is threshold -= 3, so the requirement generalises to `c > 63 - 3h`: 21 free hoists would need zero extra insns.

- [s6] Harness validity: the s6 sweep driver is the s5 driver verbatim (only the directory changed), and it reproduced the s5/s4 baseline numbers (`Loop from 25 to 146: 56 real insns.` / 107 asm lines) as its k=0 control in every sweep.

- [s6] src/code6cac_c2.c was restored to the INCLUDE_ASM line before the session closed; `git status` shows only memory/grind ledger files, the new rejected/ form, docs/grind/decisions.md and metrics/events.jsonl.

- [s6] docs/grind/decisions.md carries a new CORRECTION entry retracting the premise of the two 2026-09-01 INTEGRATION HANDOFF entries (decisions.md:19501 and :19622) — operators must not act on their step lists, since distance 0 needs no CC_FLAGS change.

## s7 (2026-09-01, solver modality) — the three remaining channels, all measured

Chassis re-check first, before anything was spent: `candidate.c` spliced over the
`INCLUDE_ASM` line at `src/code6cac_c2.c:629`,
`& tools/wteng.ps1 main sandbox func_8003C714 --disable all` prints
**score 15, target_insns 104, build_insns 105** — unchanged from s1–s6. Every
inherited measurement was chassis-current when this session started. `src/` was
restored to `INCLUDE_ASM` before the session ended (tree clean).

### The exact desirability arithmetic, re-read from the baseline dump

`tmp/grind/func_8003C714/s7/dumps/biv0.i.loop` (the candidate body, unmodified):

    Loop from 25 to 146: 56 real insns.
    Insn 33: regno 78 (life 1), move-insn savings 1  moved to 203   <- &D_80106A58
    Insn 46: regno 84 (life 1), move-insn savings 1  moved to 205   <- 0x91A2B3C5
    Insn 60: regno 91 (life 31), move-insn savings 1  moved to 207  <- 0x88888889

The 0x91A2B3C5 movable is the **second** entry in the movable list, so it is
tested after exactly one `threshold -= 3` (loop.c:1719/1904): the test at
loop.c:1631 is `119 * 1 * 1 >= 56`, comfortably true, and the constant is
hoisted. Note the asymmetry with 0x88888889, which the TARGET also hoists: its
lifetime is 31 (four `mult` uses spread over the body) versus 1 for the magic,
so on the original (soft-float, threshold 58) chassis 58*31 kept it moved while
58*1 declined the magic. Everything about the residual is that one lifetime-1
movable. `savings = n_times_used[regno] = 1` (loop.c:793) and
`lifetime = luid(last use) - luid(first use) = 1` (loop.c:791) are both already
at their structural minimum, so the ONLY dials are `insn_count` (needs >= 120)
and the number of movables moved BEFORE the magic (each worth -3 of threshold):
`c > 63 - 3h`.

### Solver triage (mandated first step of the modality)

`inverse_compose.py classify` refuses on this zero-rule function (it would
report a fictitious PRE-RA verdict from an absent `tgt.s`), and directs to the
object-level path. `goal_from_tgt.py classify code6cac_c2 func_8003C714`:

    ours 105 insns, target 104 insns   [object-level: replace_with_asmfile-safe]
    SCHED component: nop-only multiset difference (ours 1, target 0)
    AND an RA component: $t2 -> $t1 x10, $t1 -> $v0 x2
    (goal: 12 renamed pairs, 1 pair skipped — skeleton differs, reloc/immediate)

The one "skipped pair" is the lui/ori of the magic itself — the object-level
classifier can see the *consequences* of the hoist (a seat rotation and one
extra nop) but not its *cause*, which is upstream of both RA and sched. s6
already settled the causal question by measurement (at score 0 every seat lands,
K18), so the RA/SCHED components reported here are CONSEQUENT, not independent
residuals, and no RA or scheduler lever can move them while the hoist stands.
Recorded so no future session re-opens an RA search on this function because
`classify` names an RA component.

### Probe 1 — the `moved_once` insn_count doubling (loop.c:1609). KILLED (K20)

`gen_inner.py` puts the `/1800` statement in a real inner loop, so that the
inner scan (loop_optimize scans loops last-first, loop.c:425, and `moved_once`
is a per-FUNCTION array allocated once at loop.c:344) hoists the magic into the
inner preheader and sets `moved_once[regno]`, and the outer scan then doubles
`insn_count`.

    k=1  inner "Loop from 46 to 76: 10 real insns"
         outer "Loop from 25 to 168: 59 real insns."
               "Insn 225: regno 85 (life 14) ... halved since already moved  moved to 229"
    k=2  outer "Loop from 25 to 169: 60 real insns."
               "Insn 226: regno 85 (life 15) ... halved since already moved  moved to 230"

The doubling fires exactly as predicted (59 -> 118, 60 -> 120) and the magic is
STILL MOVED, because the same hoist that sets `moved_once` also separates the
SET from its single `mult` use: `m->lifetime` goes 1 -> 14/15, and lifetime
MULTIPLIES the left side of loop.c:1631 (119 * 1 * 15 = 1785 >= 120). The 2x
gain on the right is bought with a >=14x loss on the left. Self-defeating for
any spelling: the only way to set `moved_once` for this pseudo is a hoist out of
an enclosing inner loop, and every such hoist inflates lifetime past 2.
Banked: `rejected/inner-loop-moved-once-doubling-inflates-lifetime.c`.

### Probe 2 — a NATURAL loop-carried carrier (the s6 LIVE frontier item). KILLED (K21)

`gen_acc.py` sweep, baseline 56 real insns / 107 asm lines:

    variant                                     insn_count   asm lines
    acc += *(s32*)(src+4), used after loop         58 (+2)    112 (+5)
    acc += *(s32*)(src+4), DEAD after loop         56 (+0)    107 (+0)
    4x acc += mem, used after                      61 (+5)    115 (+8)
    acc = acc*31 + mem (checksum), used after      60 (+4)    114 (+7)
    16x acc += mem, used after                     74 (+18)   128 (+21)
    8x acc = acc*31 + mem, used after              82 (+26)   136 (+29)
    CONTROL acc += 1, post-use `acc - 3`           57 (+1)    107 (+0)

Dead data-dependent carrier: +0 count (cse1 / delete_dead_from_cse remove it
before `count_loop_regs_set` — the K10/K15/K16 result, re-confirmed). LIVE
data-dependent carrier: +N count and +1.12N .. +2.5N emitted instructions. The
reason is structural: `strength_reduce` can only delete a carrier whose exit
value it can FOLD, i.e. a biv with a CONSTANT increment (loop.c final-value
replacement). Anything that reads loop data has a non-constant increment, is
never a biv, and must be computed. Reaching insn_count 120 with a natural
carrier costs about +72 emitted instructions on a 104-instruction target.
The s6 free channel is therefore not "loop-carried arithmetic" in general — it
is exactly "small-immediate-constant-step biv used only after the loop", which
is dead code by construction and matches no frozen family.
Banked: `rejected/natural-accumulator-carrier-costs-1-1-in-bytes.c`.

### Probe 3 — free HOISTS ahead of the magic (the s6 cheaper-trade item). KILLED (K22)

`gen_hoist.py`: k loop-carried counters with LARGE constant steps at the TOP of
the body (so each step constant needs a lui/ori and becomes an invariant
movable ahead of the magic), each post-loop-folded to keep the store at 0.

    k=1  : 58 insns, moved 9,  notdesirable 0,  asm 116
    k=4  : 64 insns, moved 12, notdesirable 0,  asm 137
    k=8  : 72 insns, moved 16, notdesirable 0,  asm 165
    k=16 : 88 insns, moved 18, notdesirable 6,  asm 241
    k=22 : 100 insns, moved 14, notdesirable 16, asm 285

The hoists are created but cost ~9 emitted instructions each. The dump names the
mechanism: `Insn 49: possible biv, reg 74, const = (reg:SI 82)` — once the step
is a REGISTER (which is precisely what makes the step constant hoistable), the
increment is non-constant, loop.c cannot compute the final value, biv
elimination declines it, and the counter materialises. The s6 free carrier uses
small immediate steps, which fold into `addiu` and create NO movable. The two
channels are mutually exclusive; there is no free-hoist family.
The single free hoist s6 saw (gen_biv k=2, moved 8 -> 9 at 107 asm lines) is
`Insn 145: regno 132 (life 1)` — the literal 21 of the `j != 21` exit test, at
the BOTTOM of the loop. It is free only because that biv SUBSTITUTES for the
original counter, and being AFTER the magic in insn order it never reduces the
threshold the magic is tested at.
Banked: `rejected/bigconst-step-biv-creates-hoist-but-kills-biv-elimination.c`.

### Net position after s7

`c > 63 - 3h` still stands, and both terms are now measured closed for ordinary
C: `h` cannot be raised without ~9 emitted instructions per unit (K22), `c`
cannot be raised without >=1.12 emitted instructions per unit unless the carrier
is a constant-step biv used only after the loop (K21), and the one arithmetic
shortcut that would have halved the requirement is self-defeating (K20). The
only spelling that reaches distance 0 on the shipped chassis remains the s6
artificial-biv form, which is dead code by construction and matches no frozen
SOTN family.

- [s7] CHASSIS: candidate.c spliced over the INCLUDE_ASM line at src/code6cac_c2.c:629 measures score 15, target_insns 104, build_insns 105 - unchanged from s1-s6. src/ was restored to INCLUDE_ASM before the session ended; git tree is clean apart from metrics/events.jsonl.

- [s7] The exact desirability arithmetic, from the baseline .loop dump: the 0x91A2B3C5 movable is the SECOND entry in the movable list (Insn 33 &D_80106A58 life 1, then Insn 46 magic life 1, then Insn 60 0x88888889 life 31), so it faces one threshold -= 3 and is tested at 119 * 1 * 1 >= 56.

- [s7] savings = n_times_used[regno] = 1 (loop.c:793) and lifetime = 1 (loop.c:791) are both at their structural minimum for the magic; the target's asymmetric treatment of 0x88888889 (kept hoisted) versus 0x91A2B3C5 (in-loop) is entirely the lifetime 31 vs 1 split.

- [s7] moved_once is a per-FUNCTION array (loop.c:344) and loop_optimize scans loops LAST-FIRST (loop.c:425), so inner loops really do mark regnos for their enclosing loops - the doubling at loop.c:1611 is a genuine mechanism, measured firing at 59 -> 118 and 60 -> 120.

- [s7] The doubling is nonetheless useless: the inner-loop hoist that arms it moves the SET into the inner preheader and inflates m->lifetime from 1 to 14/15, and lifetime multiplies the threshold side of loop.c:1631.

- [s7] Natural-carrier sweep against baseline 56 real insns / 107 asm lines: acc += mem used-after 58/112; acc += mem dead 56/107; 4x 61/115; acc = acc*31+mem 60/114; 16x acc += mem 74/128; 8x checksum 82/136; control acc += 1 with post-use acc-3 57/107.

- [s7] Free-hoist sweep against the same baseline: k=1 58 insns moved 9 asm 116; k=4 64/12/137; k=8 72/16/165; k=16 88/18/241 (6 not desirable); k=22 100/14/285 (16 not desirable).

- [s7] 'Insn 49: possible biv, reg 74, const = (reg:SI 82)' is the one-line proof that a hoistable (large) step constant converts the counter into a register-increment biv, which loop.c cannot final-value-replace, so the counter materialises.

- [s7] inverse_compose.py classify explicitly refuses on this zero-rule function (it would report a FICTITIOUS PRE-RA verdict from an absent tgt.s) and directs to goal_from_tgt.py; that object-level classifier reports a nop-only SCHED difference plus $t2 -> $t1 x10 / $t1 -> $v0 x2, all consequent on the hoist per s6 K18.

## s8 (2026-09-01, forensics modality) — the ADMISSION axis and the THRESHOLD term, both closed at the mechanism level

Chassis re-checked FIRST, before anything was spent: `candidate.c` spliced over the
`INCLUDE_ASM` line in `src/code6cac_c2.c`, `& tools/wteng.ps1 main sandbox
func_8003C714 --disable all` prints **score 15, target_insns 104, build_insns 105,
cheat_asm_stripped 10, rules_dropped 0** — identical to s1–s7. Every inherited
measurement is still chassis-current.

Harness: `tmp/grind/func_8003C714/s8/sweep_adm.sh` (+ `gen_adm.py`),
`sweep_ord.sh` (+ `gen_ord.py`), `sweep_form.sh` (+ `gen_form.py`) — the s7 driver
with a named-variant generator instead of a numeric sweep. One `cc1 -O2 -G0 -mel
-dL` run per form; the gradient read straight out of the `.loop` dump
(`Loop from A to B: N real insns.` + the movable table) with `asm_lines` as the
byte proxy. No sandbox run is needed per form.

### Baseline movable table for the target loop (s8/dumps/base.i.loop)

    Loop from 25 to 146: 56 real insns.
    Insn 33: regno 78 (life 1),  move-insn savings 1  moved to 203   <- &D_80106A58
    Insn 46: regno 84 (life 1),  move-insn savings 1  moved to 205   <- 0x91A2B3C5
    Insn 60: regno 91 (life 31), move-insn savings 1  moved to 207   <- 0x88888889

Three movables, the magic SECOND. Emitted preheader (s8/dumps/base.s):
`move $8,$0 / li $9,0x91a20000 / ori $9,$9,0xb3c5 / li $7,0x88880000 /
ori $7,$7,0x8889 / la $6,D_80106A58 / move $5,$16`. The two `$9` instructions are
the whole d15 residual: the target has them INSIDE the loop, interleaved with the
first `lw` by sched1.

### Facts measured this session

- [s8] Chassis re-verified: candidate.c scores 15 (104 target / 105 build),
  unchanged since s1.

- [s8] ADMISSION SWEEP (6 forms, `sweep_adm.sh base cond twobb preloop postloop
  twosame`). Columns: loop range | insn_count | magic still hoisted | asm lines.

      base      25..146 |  56 | YES, 2nd, life 1              | 107
      cond      25..153 |  58 | YES, 3rd, life 1              | 109
      twobb     28..173 |  66 | YES, and now savings 2        | 123
      preloop   37..158 |  56 | YES, 2nd, life 1              | 120
      postloop  25..146 |  56 | YES, 2nd, life 1              | 118
      twosame   28..163 |  63 | YES, 2nd, life 8              | 119

  Not one of the five probes removes the `(set (reg N) (const_int 0x91A2B3C5))`
  insn from loop.c's movable list, and every one of them costs bytes (+2 .. +16
  asm lines against a 104-instruction target).

- [s8] The `twobb` form is the decisive one and it BACKFIRES: two `/1800`
  divisions in two different basic blocks of the loop produce two DISTINCT
  pseudos, so `may_not_optimize` is never set; instead loop.c's movable MATCHING
  merges them —
  `Insn 54: regno 86 (life 2), move-insn savings 2  moved to 234` and
  `Insn 73: regno 92 (life 1), done move-insn matches 54` — which DOUBLES
  `savings` and makes the hoist strictly more desirable.

- [s8] MOVABLE-ORDER SWEEP (`sweep_ord.sh ord_base ord_2213 ord_4z ord_2314`).
  Statement order inside the loop body changes which const loads precede the
  magic, hence how many `threshold -= 3` decrements it has already seen:

      0x21,0x22,0x23,0x24 (candidate) | 56 insns | magic slot 2 (r84)  | 107 asm
      0x22,0x23,0x21,0x24             | 56 insns | magic slot 3 (r126) | 107 asm
      0x23,0x22,0x21,0x24             | 56 insns | magic slot 3 (r126) | 116 -> 107 asm
      0x24,0x22,0x23,0x21             | 56 insns | magic slot 3 (r127) | 108 asm

  Reordering is insn_count-neutral AND emitted-instruction-count-neutral, so the
  order dial is genuinely free — but this loop has only THREE movables, so the
  magic can reach slot 3 and no further: threshold 122 -> 116 maximum, against a
  requirement of < 56. `threshold` is a scan_loop LOCAL (loop.c:532) recomputed
  per loop, so the file's other loops contribute nothing.

- [s8] The reordered forms are byte-DIVERGENT anyway. `diff` of ord_2213's
  emitted body against ord_base relocates the entire /1800 quartet
  (`lw $3,4($6) / mult $3,$9 / mfhi $10 / addu / sra / sra / subu / sb $2,33($5)`)
  past the /30 block and reshuffles `addu $8,$8,1`. The target's statement order
  IS 0x21, 0x22, 0x23, 0x24 and candidate.c already has it.

- [s8] LOOP-FORM SWEEP (`sweep_form.sh f_dowhile f_for f_while`): identical
  results for all three spellings — `56 real insns`, 3 movables with the magic
  2nd, 107 asm lines. The `for`/`while` forms widen the counted range from
  25..146 to 24..148 / 25..148 because `count_loop_regs_set` counts from
  `loop_top ? loop_top : loop_start` (loop.c:592), but the extra luids are NOTEs,
  not `'i'`-class insns, so `count` at loop.c:3007 is unchanged. The `loop_top`
  route contributes exactly zero.

- [s8] SOURCE READ, `tools/gcc-2.7.2/loop.c` scan_loop admission gate — the three
  tests a movable must pass, and why a compiler-generated CONST_INT set passes
  all three unconditionally:
  * loop.c:649 `! may_not_optimize[REGNO (SET_DEST (set))]`. The array is
    written ONLY by `count_loop_regs_set` (loop.c:3037 same-pseudo-in-two-basic-
    blocks, loop.c:3044 set-twice-with-a-use-between, and the explicit-CLOBBER
    case). Distinct source-level divisions give distinct pseudos, so it never
    fires — measured as `twobb`.
  * loop.c:695-700, second disjunct `(! REG_USERVAR_P && ! REG_LOOP_TEST_P)`, is
    TRUE for every compiler temp. The magic pseudo comes out of expand_divmod, is
    never a user variable and is never the loop-test reg, so this test can never
    reject it — independent of `maybe_never`, of where the division sits, and of
    `loop_reg_used_before_p`.
  * loop.c:715 `! ((maybe_never || call_passed) && may_trap_p (src))`. `src` is a
    CONST_INT (move_insn set from the REG_EQUAL note at loop.c:665) and
    `may_trap_p (const_int)` is 0.

- [s8] `m->cond` and `m->global` — the two fields s7's frontier named as escape
  hatches — are NOT admission gates. `m->global` (loop.c:790) is read only for
  `m->savemode` on PARTIAL movables (loop.c:888); the desirability test at
  loop.c:1631 never consults it. `m->cond` (loop.c:789) is `(tem|tem1|tem2) > 1`,
  i.e. `invariant_p` returned 2; `invariant_p` of a CONST_INT returns 1 always,
  so `m->cond` is structurally 0 here and the re-check at loop.c:1586 passes
  trivially. The `preloop` and `postloop` probes confirm by measurement: both
  make the divisor value live across the loop boundary and both still hoist.

- [s8] SOURCE READ, `loop.c:532`
  `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs);` — the threshold
  term has exactly two inputs and both are closed:
  * `n_non_fixed_regs` is set once in `init_reg_sets_1` (regclass.c:380-387)
    after `CONDITIONAL_REGISTER_USAGE` (regclass.c:369), which is how
    `-msoft-float` produces the 60 -> 28 / threshold 122 -> 58 that s2 measured.
    CC_FLAGS is a BARRED surface. The ONLY other writer in the entire compiler is
    `globalize_reg` (regclass.c:530, `n_non_fixed_regs--`), reached only from a
    file-scope global register variable `register T x asm("$k");` — the
    register-asm-pin cheat family, and it would take 32 of them TU-wide to reach
    threshold < 56. Not measured, deliberately: running it would produce nothing
    but a cheat artifact.
  * `loop_has_call` (set at loop.c:2202 when the loop contains a CALL_INSN)
    HALVES threshold to 61. Still not enough: 61 - 3 = 58 >= 56, the magic is
    STILL hoisted, and it would additionally need insn_count >= 59. It also
    changes the function (the target loop 8003C750..8003C7C8 contains no call).

- [s8] The one genuine ADMISSION KILL that exists in scan_loop — the
  `reg_single_usage` branch at loop.c:735-767, which DELETES an invariant set
  instead of making it a movable — is gated on `loop_has_call` (the array is only
  allocated at loop.c:586-589) AND on
  `validate_replace_rtx (SET_DEST (set), SET_SRC (set), use)`. The use here is the
  MIPS highpart multiply, whose operands are `register_operand`, so substituting
  a CONST_INT into it cannot validate. Even with a call in the loop this branch
  cannot fire for the 0x91A2B3C5 magic.

- [s8] Chassis re-verified FIRST, before anything was spent: candidate.c spliced over the INCLUDE_ASM line, `sandbox func_8003C714 --disable all` prints score 15, target_insns 104, build_insns 105, cheat_asm_stripped 10, rules_dropped 0 — identical to s1-s7. Every inherited measurement is chassis-current.

- [s8] Baseline movable table for the target loop (tmp/grind/func_8003C714/s8/dumps/base.i.loop): `Loop from 25 to 146: 56 real insns.` / `Insn 33: regno 78 (life 1), move-insn savings 1  moved to 203` (&D_80106A58) / `Insn 46: regno 84 (life 1), move-insn savings 1  moved to 205` (0x91A2B3C5) / `Insn 60: regno 91 (life 31), move-insn savings 1  moved to 207` (0x88888889). Three movables, the magic SECOND, tested at 119*1*1 >= 56.

- [s8] Emitted preheader of the candidate (s8/dumps/base.s): `move $8,$0 / li $9,0x91a20000 / ori $9,$9,0xb3c5 / li $7,0x88880000 / ori $7,$7,0x8889 / la $6,D_80106A58 / move $5,$16`. The two $9 instructions ARE the d15 residual — the target has them inside the loop, interleaved with the first `lw` by sched1.

- [s8] ADMISSION sweep, 6 forms (loop range | insn_count | magic still hoisted | asm lines): base 25..146 | 56 | YES 2nd | 107; cond 25..153 | 58 | YES 3rd | 109; twobb 28..173 | 66 | YES with savings 2 | 123; preloop 37..158 | 56 | YES 2nd | 120; postloop 25..146 | 56 | YES 2nd | 118; twosame 28..163 | 63 | YES 2nd life 8 | 119. Not one removes the movable, and every one costs bytes.

- [s8] The twobb form's dump is the decisive line: `Insn 54: regno 86 (life 2), move-insn savings 2  moved to 234` followed by `Insn 73: regno 92 (life 1), done move-insn matches 54`. Two /1800 divisions in two basic blocks give two DISTINCT pseudos (so may_not_optimize never fires) which loop.c then MATCHES and merges, doubling savings — the admission attack makes the hoist MORE desirable, not less.

- [s8] MOVABLE-ORDER sweep: 0x21,0x22,0x23,0x24 (candidate) 56 insns / magic slot 2 (r84) / 107 asm; 0x22,0x23,0x21,0x24 56 / slot 3 (r126) / 107; 0x23,0x22,0x21,0x24 56 / slot 3 (r126) / 107; 0x24,0x22,0x23,0x21 56 / slot 3 (r127) / 108. Statement reordering is genuinely free in both insn_count and emitted instruction count — and caps at threshold 116 because the loop has only three movables.

- [s8] The reordered forms are byte-divergent in sequence: diff of ord_2213's body against ord_base relocates `lw $3,4($6) / mult $3,$9 / mfhi $10 / addu $2,$10,$3 / sra $2,$2,10 / sra $3,$3,31 / subu $2,$2,$3 / sb $2,33($5)` past the /30 block and reshuffles `addu $8,$8,1`. The target's statement order is 0x21,0x22,0x23,0x24, which candidate.c already has.

- [s8] LOOP-FORM sweep: do-while / for / while spellings of the identical body all give 56 real insns, 3 movables with the magic 2nd, and 107 asm lines. The for/while forms widen the counted range from 25..146 to 24..148 / 25..148 (count_loop_regs_set counts from `loop_top ? loop_top : loop_start`, loop.c:592) but the extra luids are NOTEs, so `count` at loop.c:3007 is unchanged.

- [s8] PASS ATTRIBUTION (source-read, not guessed): scan_loop's admission gate is exactly three tests — loop.c:649 `! may_not_optimize[REGNO (SET_DEST (set))]`, loop.c:695-700 the uservar/basic-block test, loop.c:715 the trap test — and a compiler-generated CONST_INT set passes all three unconditionally. may_not_optimize is written only by count_loop_regs_set (loop.c:3037/3044) for a pseudo set in two basic blocks or set twice with a use between, or explicitly clobbered.

- [s8] m->cond (loop.c:789) and m->global (loop.c:790) are NOT admission gates, contrary to the s7 frontier's framing: m->global is consumed only for m->savemode on PARTIAL movables (loop.c:888) and the desirability test at loop.c:1631 never reads it; m->cond is `(tem|tem1|tem2) > 1` and invariant_p of a CONST_INT returns 1, so it is structurally 0 for this movable and the re-check at loop.c:1586 passes trivially.

- [s8] loop.c:532 `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` — a call inside the loop HALVES the threshold to 61. This looks like the biggest lever in the file and is not one: 61 - 3 = 58 >= insn_count 56, so the magic is still hoisted, and the target's loop contains no call.

- [s8] The only genuine admission KILL in scan_loop is the reg_single_usage branch at loop.c:735-767, which deletes an invariant set outright instead of making it a movable. It is doubly gated: the array is allocated only under loop_has_call (loop.c:586-589), and `validate_replace_rtx (SET_DEST, SET_SRC, use)` must substitute the CONST_INT into the use — which here is the MIPS highpart multiply with register_operand operands, so it can never validate.

- [s8] n_non_fixed_regs (60 on this chassis) is written in exactly two places in the whole compiler: init_reg_sets_1 (regclass.c:380-387, i.e. CC_FLAGS / CONDITIONAL_REGISTER_USAGE, which is how -msoft-float gives 28 and threshold 58) and globalize_reg (regclass.c:530, `n_non_fixed_regs--`), reachable only from a file-scope global register variable. Reaching threshold < 56 needs N <= 28, i.e. 32 register-asm pins TU-wide — the forbidden pin family, applied to a file carrying 43 other functions.

## s9 (2026-09-01) — escalation / disposition modality

No new grinding axis was opened. The driver assigned `escalation` after the floor
stayed FLAT at 15 across eight sessions and six distinct modalities; this session
re-measured the chassis, ran the two endgame-lock AND-gates, and filed the
foreclosure record.

### s9-1 Chassis re-measurement (FIRST action, before any reasoning)

`memory/grind/func_8003C714/candidate.c` applied over the `INCLUDE_ASM` line in
`src/code6cac_c2.c` (`tmp/grind/func_8003C714/s9/apply.py`), then
`& tools/wteng.ps1 main sandbox func_8003C714 --disable all`:

    {"score": 15, "target_insns": 104, "build_insns": 105, "scorable": true,
     "func": "func_8003C714", "file": "code6cac_c2", "disable": "all",
     "strip_cheat_asm": true, "rules_dropped": 0, "cheat_asm_stripped": 10}

Identical to s3-s8. The chassis has NOT drifted; every banked spelling conclusion
in this ledger is still chassis-current. `src/code6cac_c2.c` was reverted to its
`INCLUDE_ASM` line afterwards (`git checkout --`), so the tree is clean apart from
`metrics/events.jsonl`.

### s9-2 Gate (a): hand-coded scanner — FAIL (tier LOW, 0/8)

`python3 tools/scan_hand_coded.py --single func_8003C714`:

    HAND_CODED: tier=LOW  score=0/8  (func_8003C714, 104 insns)
      Reason: no strong hand-coded indicators
      [ ] S1 multu pacing   only 0 multu/mflo pair(s)
      [ ] S2 empty branch   no empty-body branches
      [ ] S3 no spills      104 insns, 2 spills, 9 distinct regs
      [ ] S4 front loads    max load burst was 1 in any 8-insn window
      [ ] S5 cluster        no high-similarity siblings (jaccard < 0.5)
      [ ] S6 BIOS jumptable no BIOS jumptable call pattern
      [ ] S7 unsaved $sN    all callee-save uses have $sp save
      [ ] S8 redundant mask no redundant mask-before-shift

None of the three STRONG signals (S1/S2/S6) fire. The canonical-asm grant path is
closed for this function — it is compiler output, exactly as s1-s8 assumed.
This is the first time the scanner has actually been RUN on this function; every
prior session assumed the tier rather than measuring it, so this closes a small
open assumption in the ledger.

### s9-3 Gate (b): SOTN-master precedent census — NEGATIVE

Construct requiring precedent: the s6 distance-0 form — 32 balanced
`z += c; z -= c;` pairs on a local whose only use is post-loop, present solely to
drive `count_loop_regs_set`'s `insn_count` from 56 past 120 so `loop.c:1631`
declines the `0x91A2B3C5` movable, with biv elimination then deleting the whole
carrier inside `loop_optimize` for zero emitted bytes
(`rejected/balanced-biv-noise-64-insns-d0-but-inadmissible.c`).

Census against `docs/reference/sotn-construct-index.md` (sotn-decomp master
`aa53500226ee84be763f3e8702b27de06456b3a7`, 1911 files scanned, 12 construct
classes, 2746 index lines):

    grep -iE "insn_count|licm|hoist|loop_optimize|invariant|strength|unroll|biv"  ->  0 hits
    grep -iE "loop-carried|balanced|noise"                                        ->  0 hits
    grep -i  "loop"  ->  19 hits, all of them either prose match-comments about
                        loop SHAPE ("This is just not a for-loop"), a
                        `score->read_pos = score->loop_pos;` dup-arm statement, or
                        `after_loop:` / `loop_30:` style exit labels. None is a
                        loop-body-inflating construct.

The twelve classes the index actually carries are `fake_comment`,
`fake_identifier`, `self_assign`, `match_comment`, `do_while_zero`,
`pad_dummy_local`, `new_var_temp`, `pointer_alias`, `dup_if_else_arm`,
`const_holder`, `empty_if`, `nested_exit_label`. SOTN master ships NO
loop-body-inflating dead-arithmetic construct. Note in particular that
`pad_dummy_local` (816 hits) is the nearest-looking class and does NOT cover this:
those are DECLARED unused locals, whereas this construct requires in-loop
arithmetic STATEMENTS that survive `cse1` — and BB2's already-sanctioned
dead-scalar-local / constant-holder families were measured worth +0 insn_count
here in s5 K15 for exactly that reason.

Gate (b) is a FAILED gate, not an open question (owner ruling 2026-08-24,
reaffirmed 2026-08-31: a construct outside the frozen family list is a clean
refusal and is not argued for beyond the negative census).

### s9-4 Disposition

Both AND-gates FAIL, so the owner's 2026-07-27 standing ruling applies and the
disposition is the silent foreclosure of `.claude/rules/ordinary-c-judge-decidable.md`
(2026-08-31). Proof-of-foreclosure record appended at
`docs/grind/decisions.md:19815`
(`## 2026-09-01 — func_8003C714 — **RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED**`),
carrying both gates' evidence, the term-by-term closure table for
`loop.c:1631`, the exhaustion counts (9 sessions / 6 modalities / 66,016 permuter
iterations / 16 banked rejected forms), and three named re-activation triggers.
No owner question is asked and no packet is filed.

- [s9] Chassis re-measured this session with candidate.c applied: sandbox func_8003C714 --disable all = score 15, 104 target / 105 build, rules_dropped 0, cheat_asm_stripped 10. The honest pure-C floor has been 15 in all nine sessions.

- [s9] Gate (a) FAILS: scan_hand_coded --single func_8003C714 reports tier=LOW score=0/8, with none of the STRONG signals S1/S2/S6 firing. The function is ordinary compiler output and the canonical-asm grant path is unavailable.

- [s9] Gate (b) FAILS: a precedent census of docs/reference/sotn-construct-index.md (sotn master aa53500226, 1911 files) returns ZERO hits for insn_count/LICM/hoist/loop_optimize/invariant/strength/unroll/biv and ZERO for loop-carried/balanced/noise. SOTN master ships no loop-body-inflating dead-arithmetic construct.

- [s9] The s6 distance-0 construct also fails the 6-test checklist on its own terms: T1 (the emitted function is byte-identical with and without it at every intermediate k), T2 (no human writes 32 balanced add/sub pairs into a three-iteration display loop), T3 (its whole justification is count_loop_regs_set / loop.c:1631), T6 (pure noise carrier). It matches no frozen family and is in the AUTO-REJECT class per the 2026-08-24 ruling.

- [s9] The loop.c:1631 inequality `threshold * savings * m->lifetime >= insn_count` is closed TERM BY TERM for ordinary C on this chassis: savings pinned at 1 (loop.c:791, s7 H11); lifetime pinned at 1 (loop.c:793) with the loop.c:1609 moved_once doubling self-defeating because the hoist that sets it inflates lifetime to 14/15 (s7 K20); threshold (loop.c:532) reachable only via CC_FLAGS (barred) or 32 register-asm pins via globalize_reg/regclass.c:530 (forbidden family), with loop_has_call only reaching 61 where 61-3=58 >= 56 still hoists (s8 K26); ADMISSION dead in five measured spellings and at the mechanism level (s8 K23); movable ORDER free but capped at -6 because the loop has exactly three movables (s8 K24); loop FORM count-neutral (s8 K25); and insn_count's only free carrier is a constant-step biv whose sole use is post-loop, i.e. dead code by construction (s6 H10, s7 K21/K22).

- [s9] Exhaustion: nine sessions across six distinct modalities (s1 recon, s2/s3 structural, s4 permuter, s5/s6 synthesis, s7 solver, s8 forensics, s9 escalation), floor 15 at every one; 66,016 decomp-permuter iterations on this exact body with zero improvements over base; 16 disproven forms banked in memory/grind/func_8003C714/rejected/, each named for its mechanism.

- [s9] Foreclosure record appended at docs/grind/decisions.md:19815 — `## 2026-09-01 — func_8003C714 — **RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED**` — carrying both gates' evidence, the term-by-term closure table, the exhaustion counts, and three named re-activation triggers. It asks the owner no question and files no packet.

- [s9] Tree state at end of session: src/code6cac_c2.c reverted to its INCLUDE_ASM line (clean); modified files are docs/grind/decisions.md and the three memory/grind/func_8003C714/ ledger files only (plus the engine's own metrics/events.jsonl).

- [operator 2026-09-02] owner ruling 2026-09-02 (decisions.md 'foreclosure mechanics'): re-activated — ledger floor 15 > ENDGAME_LOCK_MAX_FLOOR=5, so the 2026-07-27 standing ruling was never its subject; the ladder runs a second full cycle (20 flat sessions, >= 6 modalities) before any disposition. All standing banned_constructs remain in force. exhaustion_base=9

---

## s10 (2026-09-05) — rederive modality

**Chassis re-checked first.** `sandbox func_8003C714 --disable all` with
`memory/grind/func_8003C714/candidate.c` applied over the `INCLUDE_ASM` line in
`src/code6cac_c2.c`: **score 15, target_insns 104, build_insns 105,
rules_dropped 0, cheat_asm_stripped 9**. Unchanged from s2–s9. Re-verified again
at the end of the session after all probes were reverted (15 / 105).

### s10.1 — The desirability inequality read term by term FROM THE SOURCE

Baseline `.loop` movable table on this chassis (regenerated this session,
`tmp/grind/func_8003C714/dumps/code6cac_c2.loop:4223`):

```
Loop from 25 to 146: 56 real insns.
Insn 33: regno 78 (life 1),  move-insn savings 1   moved to 203   <- &D_80106A58
Insn 46: regno 84 (life 1),  move-insn savings 1   moved to 205   <- 0x91A2B3C5 (/1800)
Insn 60: regno 91 (life 31), move-insn savings 1   moved to 207   <- 0x88888889 (/30)
```

**CORRECTION to s7's H11 (the reason, not the number).** `m->savings`
(`tools/gcc-2.7.2/loop.c:793`) is `n_times_used[regno]`, and `n_times_used` is
NOT a use count: `loop.c:597` initialises it as a straight
`bcopy` of `n_times_set`, which `count_loop_regs_set` fills with the number of
times the pseudo is **SET** inside the loop. The dump proves it: the /30 magic
(regno 91) is read by FOUR in-loop `mult`s and still reports `savings 1`,
identical to the /1800 magic which is read once.

Consequences, both new:
* The two magics are distinguished **only by `m->lifetime`** (31 vs 1), never by
  how often they are consumed. The /30 magic hoists on `122 * 1 * 31`; the /1800
  magic is marginal on `119 * 1 * 1` vs `insn_count 56`.
* Any C spelling whose intent is "make the /1800 constant be read fewer times"
  is a NO-OP on the gate. That lever family is closed at the source level, not
  by measurement.

**`m->lifetime` has a hard floor of 1.** `loop.c:791` computes it as
`uid_luid[regno_last_uid] - uid_luid[regno_first_uid]`; `loop_optimize` assigns
luids strictly monotonically (+1 per non-line-number insn), so two distinct
insns differ by >= 1. `lifetime == 0` would require the pseudo to be mentioned
in exactly one insn — i.e. a set with no reader, which `cse1` /
`delete_dead_from_cse` remove before `loop_optimize` ever runs. So the LHS of
`threshold * savings * m->lifetime >= insn_count` (loop.c:1631) cannot be driven
below `threshold` itself for this movable.

**`threshold` recap on the shipped chassis** (`loop.c:532`,
`(loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)`): 122, i.e.
`n_non_fixed_regs == 60`. `loop_has_call` requires a real `CALL_INSN` between
the loop notes (`prescan_loop`); the target loop body contains no `jal`, so the
halving branch is not source-reachable here. This is K26 re-derived from the
source rather than inherited.

### s10.2 — MEASURED: a fourth ordinary-C movable DOES exist (frontier item 3 is CLOSED, and it was wrong)

s8's frontier stated "s8 measured the loop has exactly three movables". False.
Reading the +4 time field through its own index-derived pointer instead of
through `src + 4` materialises `&D_80106A58 + 4` as a distinct loop invariant
and produces a FOURTH movable, `m->forces`-chained to the base:

```
Loop from 25 to 154: 58 real insns.
Insn 33: regno 79 (life 3),  move-insn savings 2            moved   <- &D_80106A58
Insn 41: regno 82 (life 1),  move-insn forces 33 savings 1  moved   <- &D_80106A58+4  (NEW)
Insn 54: regno 88 (life 1),  move-insn savings 1            moved   <- 0x91A2B3C5
Insn 68: regno 95 (life 31), move-insn savings 1            moved   <- 0x88888889
```

The /1800 magic is pushed from movable slot 2 to slot 3, so it faces
`threshold = 122 - 3 - 3 = 116` (two `threshold -= 3` at loop.c:1719/1904)
instead of 119. `116 * 1 * 1 = 116 >= 58` → still hoisted.

**Price of the dial, measured for the first time:** one extra movable moves the
gap `threshold - insn_count` from `119 - 56 = 63` to `116 - 58 = 58` (−5 per
movable: −3 on threshold, +2 on insn_count) and costs **+3 emitted
instructions** (`build_insns` 105 → 108, score 15 → 28). Extrapolating the −5
slope reproduces s4's K14 crossover at ~13 added invariants from the opposite
direction and finally puts a byte price on it: **~+39 emitted instructions
against a 104-instruction target.** The dial is ordinary C and is not capped at
three movables — it is capped by bytes, by about an order of magnitude.
Form banked at `rejected/extra-movable-dial-costs-3-insns-per-slot.c`.

### s10.3 — MEASURED: the `may_not_optimize` admission escape is not reachable by statement order

s8's K23 enumerated `scan_loop`'s admission as "exactly three tests". There is a
fourth, upstream one: the movable scan is gated on
`! may_not_optimize[REGNO (SET_DEST (set))]` (loop.c:649), and
`count_loop_regs_set` sets that flag for a pseudo which is either (a) set in two
different basic blocks of the loop (`n_times_set > 0 && last_set == 0`,
loop.c:3037) or (b) set twice with a **use of the pseudo between the two sets**
(`reg_used_between_p`, loop.c:3044). K23 dismissed the flag on the grounds that
"two source-level /1800 divisions give two DISTINCT pseudos so it never fires".
That premise is measurably false for a repeated invariant: the s10.2 variant
materialises `&D_80106A58` twice and yields ONE pseudo (regno 79) with
`n_times_set == 2` — the dump prints it as `savings 2`.

So the escape is real in principle. It is however not reachable by ordering:
re-ordering the body so that `*src` (a reader of the base pseudo) is evaluated
between the two materialisations stretches the movable's life 3 → 7 but does
NOT trip `reg_used_between_p` — the entry still prints
`Insn 33: regno 79 (life 7), move-insn savings 2  moved`, i.e. GCC keeps the two
sets adjacent to their own uses and no reader lands strictly between them.
Independently of that, the escape cannot help THIS residual even if tripped: it
would have to fire on the 0x91A2B3C5 pseudo, which requires a second in-loop
`/1800` division (~+7 emitted instructions) or a second materialisation split
across a branch, and the target loop contains exactly one division by 1800 and
no conditional branch.

### s10.4 — Rederive corpus check: the codebase's own time-format sibling

`func_8003C714` calls `func_8001CD68`, which is already **matched C** at
`src/code6cac.c:1121-1139` and formats the *same* mm:ss.cc quantity from the
same 30 fps frame count. Its idiom is materially different from ours:
`minutes = val / 1800; seconds = val / 30 - minutes * 60;` and the centiseconds
term re-reads the GLOBAL (`(D_800A3858 % 30) * 100 / 30`) rather than the local.
That shape is **not** what `func_8003C714` compiled from: the target recomputes
`(t/30)/60` with the 0x88888889 magic at shift 5 (8003C79C..8003C7B8) instead of
reusing the `/1800` result, which is the signature of `(t / 30) % 60`, exactly
what candidate.c spells. The sibling therefore CONFIRMS the banked body's
expression shape by contrast and supplies no transplantable alternative. The
only other `0x91A2B3C5` carriers in the image are `func_80035280` and
`func_80067D14`, both still `INCLUDE_ASM`, so no matched in-loop precedent for
this constant exists anywhere in the project.

- [s10] Chassis re-verified twice this session: candidate.c applied over the INCLUDE_ASM line scores 15 (target_insns 104, build_insns 105, rules_dropped 0, cheat_asm_stripped 9), before and after all probes. src/code6cac_c2.c was reverted to INCLUDE_ASM at session end.

- [s10] Baseline movable table on the shipped chassis (tmp/grind/func_8003C714/dumps/code6cac_c2.loop:4223): 'Loop from 25 to 146: 56 real insns.' / 'Insn 33: regno 78 (life 1), move-insn savings 1' (&D_80106A58) / 'Insn 46: regno 84 (life 1), move-insn savings 1' (0x91A2B3C5) / 'Insn 60: regno 91 (life 31), move-insn savings 1' (0x88888889).

- [s10] loop.c:597 initialises n_times_used by bcopy from n_times_set, so m->savings (loop.c:793) counts SETS of the pseudo inside the loop, not uses. The /30 magic is read four times and still reports savings 1.

- [s10] loop.c:532 threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs) = 122 on this chassis (n_non_fixed_regs 60); prescan_loop sets loop_has_call only from a real CALL_INSN between the loop notes, and the target loop body contains no jal, so the halving branch is not source-reachable for this function.

- [s10] One extra movable measured: threshold at the magic's test 119 -> 116, insn_count 56 -> 58, desirability gap 63 -> 58, emitted instructions 105 -> 108, score 15 -> 28. The gap therefore closes by 5 per movable slot at a price of 3 emitted instructions.

- [s10] loop.c:649 may_not_optimize is a fourth admission gate omitted from s8's K23; a doubly-materialised invariant base yields ONE pseudo with n_times_set 2 (printed as savings 2), disproving K23's distinct-pseudo premise, but statement reordering did not trip reg_used_between_p (life 3 -> 7, still moved).

- [s10] func_8001CD68 (src/code6cac.c:1121-1139) is matched C for the same mm:ss.cc conversion and is the function func_8003C714 calls; its expression shape (val/30 - minutes*60, centiseconds re-read from the global) is contradicted by the target's recomputed (t/30)/60, confirming candidate.c's (t/30) % 60.

## s11 (2026-09-05, rederive modality) — the loop.c:1631 gate re-derived from the DUMP TABLE, and a corrected moved_once channel

Chassis re-checked FIRST and again at session end with `memory/grind/func_8003C714/candidate.c`
applied over the `INCLUDE_ASM` line in `src/code6cac_c2.c`:
`sandbox func_8003C714 --disable all` = **score 15, target_insns 104, build_insns 105,
rules_dropped 0**. Body unchanged this session.

### s11.1 The baseline movable table, read (not inferred)

`pwsh`-equivalent dump via `tmp/grind/func_8003C714/run_dump.sh`; the loop is
`Loop from 25 to 146: 56 real insns.` in `tmp/grind/func_8003C714/s11/base.loop`:

| slot | insn | regno | lifetime | savings | what it is | verdict |
|---|---|---|---|---|---|---|
| 1 | 33 | 78 | 1  | 1 | `&D_80106A58` (address base) | moved to 203 |
| 2 | 46 | 84 | 1  | 1 | `0x91A2B3C5` (the /1800 magic) | moved to 205 |
| 3 | 60 | 91 | 31 | 1 | `0x88888889` (the /30 magic)  | moved to 207 |

This is the first time the ledger records the *identities* rather than the counts. It
settles two things that earlier sessions carried as inference:
- the /1800 magic really is movable **slot 2**, so it is tested with `threshold` already
  decremented once (122 → 119) by the base-address hoist (loop.c:1719 `threshold -= 3`);
- `savings` is 1 for the /30 magic even though four in-loop `mult`s read it, which is the
  direct dump confirmation of s10's H15 (`m->savings = n_times_used[regno]` at loop.c:800,
  and loop.c:597 `bcopy`s `n_times_set` into `n_times_used`, so savings counts SETS).

### s11.2 The target-consistent arrangement, and its UPPER bound (new)

The shipped target's preheader (8003C73C..8003C750) hoists exactly the base address
(`lui/addiu $a2,%hi/%lo(D_80106A58)`) and the /30 magic (`lui/ori $a3,0x88888889`), and keeps
the /1800 magic IN-LOOP as `lui $v0 / ori $v0` at 8003C754/8003C75C. Against the table above
that is one and only one arrangement of loop.c:1631:

    slot 1 (base,  life 1 ) : 122 * 1 *  1 >= insn_count   -> MOVED    => insn_count <= 122
    slot 2 (/1800, life 1 ) : 119 * 1 *  1 >= insn_count   -> DECLINED => insn_count >= 120
    slot 3 (/30,   life 31) : 119 * 1 * 31 >= insn_count   -> MOVED    (3689, always true)

So the admissible window is **insn_count ∈ [120, 122]**, not "insn_count >= 120". s6's
distance-0 form sat at 121, inside the window; the ledger never recorded that pushing
further BREAKS the match, because at insn_count >= 123 the base-address movable is declined
too and the preheader loses its `lui/addiu $a2` pair. Any future insn_count-inflation attack
must land in a 3-wide window, not merely exceed a floor.

### s11.3 The moved_once channel, corrected (s7's K20 was measured on the wrong carrier)

loop.c:1605-1613:

    if (moved_once[regno]) { insn_count *= 2; ... "halved since already moved" ... }

`insn_count` is a scan_loop local and the doubling is **not undone** — it applies to every
movable tested after it. `moved_once[]` is per-function (`alloca` + `bzero` at loop.c:344-345)
and is written only at loop.c:1912, when a movable is actually moved out of a loop; loops are
scanned innermost/latest-first (`for (i = max_loop_num-1; i >= 0; i--)`, loop.c around 108 of
`loop_optimize`), so an invariant hoisted out of a loop NESTED INSIDE ours arrives at our scan
with `moved_once` already set.

s7's K20 attached that flag to the /1800 magic itself and concluded the channel is
self-defeating (the inner-loop hoist inflates that movable's own `m->lifetime` 1 -> 14/15, and
lifetime multiplies the LHS). **That is a property of the carrier, not of the channel.** With a
DIFFERENT, EARLIER movable carrying the flag, the magic's lifetime stays 1 and the doubling
still applies to its test.

Measured, three placements (all diagnostics, semantics deliberately wrong; dumps kept):

1. `tmp/grind/func_8003C714/s11/varB_movedonce_early.c`, first spelling — inner loop storing
   through `&D_80106A58 + j`. **No movable at all** in the inner loop: loop.c folded the symbol
   into the giv as `dest address ... add (symbol_ref:SI ("D_80106A58"))`, so nothing was moved
   and `moved_once` stayed clear. A symbol-address invariant is NOT a usable carrier.
2. Same file, second spelling — inner loop assigning a CONST_INT to a global, placed at the TOP
   of the outer body (before the base-address computation). Dump
   (`tmp/grind/func_8003C714/s11/varB.loop` is the pre-fix run; the const run is in
   `s11/varE.loop`'s predecessor): `Loop from 25 to 176: 63 real insns`, carrier moved with
   `halved since already moved`, and then **BOTH** the base address AND the /1800 magic printed
   `not desirable` (2*63 = 126 > 122). Channel proven live, arrangement wrong.
3. `tmp/grind/func_8003C714/s11/varD.c` / `varE.c` — same carrier moved to sit BETWEEN the
   base-address computation and the first division, so the base is still movable slot 1 and is
   tested against the UNDOUBLED count. Dump `tmp/grind/func_8003C714/s11/varE.loop`:

        Loop from 25 to 174: 62 real insns.
        Insn  33: regno 79 (life  1) move-insn savings 1  moved to 233        <- &D_80106A58
        Insn 231: regno 82 (life  9) move-insn savings 1 halved since already moved  moved to 235
        Insn  74: regno 87 (life  1) move-insn savings 1 not desirable        <- 0x91A2B3C5
        Insn  88: regno 94 (life 31) move-insn savings 1  moved to 237        <- 0x88888889

   **That is the target's preheader, movable for movable** — the first time this ledger has
   produced the target's exact loop.c arrangement from ordinary loop structure rather than from
   the s6 balanced-biv noise.

Arithmetic of placement 3: slot 1 is tested at threshold 122 against the undoubled
`insn_count` (62) and moves; the carrier is slot 2, doubles the count to 124, and moves
(119 * 9 >= 124), taking threshold to 116; the magic is slot 3 and 116 < 124 declines it; the
/30 magic is slot 4 at 116 * 31. Generalised: with the carrier between base and magic the
requirement is `2 * insn_count > 116` and `insn_count <= 122`, i.e. **insn_count >= 59** — a
+3 RTL-insn ask instead of the +64 the s5/s6 record implies.

### s11.4 What it costs, and why it is still byte-blocked

`sandbox func_8003C714 --disable all` on the two carrier spellings, honest (no FAKE, no cheat
constructs, only ordinary C):

| form | build_insns | score |
|---|---|---|
| `candidate.c` (baseline) | 105 | 15 |
| `varD.c` — carrier `D_800A37B8 = 0x12345678;` | 113 | 19 |
| `varE.c` — carrier `D_800A37B8 = 5;` (leanest found) | 112 | 18 |

So the cheapest measured moved_once carrier costs **+7 emitted instructions** (inner-loop
counter init, the store, the increment, the compare, the backward branch, and the hoisted
const in the outer preheader). Against a 104-instruction target that our honest body already
overshoots by one, there is no room. Note the score only rose 15 -> 18 while the instruction
count rose by 7: declining the /1800 hoist genuinely repairs several of the 15 residual
differences, which is independent corroboration of s6's distance-0 measurement.

The carrier is structurally required to be a LOOP: `moved_once[]` is written nowhere but
loop.c:1912, and the pseudo must be one our loop also sets exactly once, which for a sibling
loop would mean the same user variable assigned an invariant in both loops. Either way a
second backward branch is emitted, and the target has exactly one.

### s11.5 Sibling sweep

`func_80067D14` (src/text1b.c, the only sibling this ledger names) has **no `candidate.c`** in
`memory/grind/func_80067D14/`, so there is no spelling to transplant; nothing was inherited or
measured from it this session.

- [s11] Chassis re-measured twice with memory/grind/func_8003C714/candidate.c applied over the INCLUDE_ASM line: sandbox func_8003C714 --disable all = score 15, target_insns 104, build_insns 105, rules_dropped 0. Body unchanged this session; src/code6cac_c2.c restored to INCLUDE_ASM at session end.

- [s11] Baseline movable table (tmp/grind/func_8003C714/s11/base.loop, 'Loop from 25 to 146: 56 real insns'): slot 1 insn 33 regno 78 life 1 savings 1 = &D_80106A58; slot 2 insn 46 regno 84 life 1 savings 1 = 0x91A2B3C5; slot 3 insn 60 regno 91 life 31 savings 1 = 0x88888889. All three moved. This is the first time the ledger records the identities rather than the counts.

- [s11] The dump directly confirms s10's H15: the /30 magic is read by four in-loop mults and still prints 'savings 1', because loop.c:800 sets m->savings = n_times_used[regno] and loop.c:597 bcopies n_times_set into n_times_used.

- [s11] loop.c:1609 doubles scan_loop's insn_count local permanently for all subsequent movables and does not decrement threshold; threshold is decremented by 3 only at loop.c:1719 and loop.c:1904, i.e. only when a movable is actually moved. A declined movable therefore leaves threshold intact for the next one.

- [s11] moved_once[] is per-function (alloca + bzero at loop.c:344-345), written only at loop.c:1912, and loop_optimize scans loops with 'for (i = max_loop_num-1; i >= 0; i--)', so only a loop nested inside ours (or a later sibling sharing the pseudo) can pre-set the flag.

- [s11] force_movables (loop.c:1193-1225) can only link m->forces to an EARLIER movable, so the loop.c:1594 '(! m->forces || m->forces->done)' skip cannot be used to suppress a movable; and the STRICT_LOW_PART partial-movable path at loop.c:832-900 is unreachable on MIPS, so two movables can never share a regno inside one scan_loop call.

- [s11] Carrier cost table, all ordinary C with no FAKE constructs: candidate.c 105 insns / score 15; varE.c (carrier 'D_800A37B8 = 5;') 112 / 18; varD.c (carrier 'D_800A37B8 = 0x12345678;') 113 / 19.


## s12 (2026-09-05) - structural modality

### s12.0 Chassis re-check (first action)
`memory/grind/func_8003C714/candidate.c` applied over the `INCLUDE_ASM` line in
src/code6cac_c2.c and measured with `sandbox func_8003C714 --disable all`:
score 15, target_insns 104, build_insns 105, rules_dropped 0,
cheat_asm_stripped 9. Identical to the s11 record except that
cheat_asm_stripped moved 10 -> 9 (a TU-level count from neighbouring functions,
not this body). The ledger floor of 15 is therefore CURRENT, and every banked
spelling conclusion is still measured on this chassis.

### s12.1 Mandated kill re-audit (K33, the instance kill that sat closest to the target)
- `tools/fake_ablate.py --func func_8003C714 --file code6cac_c2 --candidate
  memory/grind/func_8003C714/candidate.c` -> "no FAKE-annotated constructs found
  ... nothing to ablate". The honest body carries no FAKE carrier, so no banked
  lever was measured while a FAKE construct occupied its pseudo.
- s11's leanest moved_once carrier (`tmp/grind/func_8003C714/s11/varE.c`,
  inner loop `j = 0; do { D_800A37B8 = 5; j += 1; } while (j < 2);` placed
  between the `dst` computation and the first division) re-measured on the
  current chassis: score 18, build_insns 112. Byte-for-byte the same result s11
  recorded (+7 emitted instructions over the honest 105). K33 STANDS.

### s12.2 The moved_once channel is byte-foreclosed by the target's own control flow (NEW, class)
This closes the s11 frontier's item 1 ("a moved_once carrier that emits zero net
bytes would close the function").
Chain, each link measured or read from the compiler source this session:
1. `moved_once[regno] = 1` is written at exactly one place, loop.c:1912 (grep of
   tools/gcc-2.7.2/loop.c), inside scan_loop and only after a movable has been
   MOVED out of some loop. The array is alloca'd and bzero'd per function at
   loop.c:343-344, so it starts clear.
2. scan_loop refuses to process any loop whose scan_start is not a CODE_LABEL:
   loop.c:568-576 prints "Loop from %d to %d is phony." and returns. So the
   carrier must be a loop that has a real top CODE_LABEL and a backward jump to
   it - which is precisely why `rejected/dowhile0-inner-loop-is-phony.c` failed,
   and why no `do { } while (0)` spelling can carry the flag.
3. A loop that satisfies (2) emits that label and that backward branch. Measured
   directly on the leanest known carrier: with varE.c applied,
   `tmp/grind/func_8003C714/dumps/code6cac_c2.s` contains TWO labels and TWO
   backward branches inside func_8003C714 (.L141 / `bne $2,$0,.L141` for the
   carrier, .L133 / `bne $2,$0,.L133` for the real loop). No pass after
   loop_optimize removes it: flag_unroll_loops is off in the shipped CC_FLAGS
   ("-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin
   -w -mel", engine/buildconfig.py:43), and a >=2-trip loop cannot be folded to
   straight-line code without unrolling.
4. The TARGET's bytes contain exactly ONE label and ONE branch:
   `asm/funcs/func_8003C714.s` has `.L8003C754:` at line 18 and
   `bnez $v0, .L8003C754` at line 77, and no other label, branch or jump besides
   the final return. So the shipped function has exactly one loop.
Consequence: any construct that sets moved_once adds a second loop to the
emitted code, and the target has no second loop. The whole moved_once /
insn_count-doubling route (s11 H16, the cheapest known route to the target's
movable arrangement, requiring only insn_count >= 59) is byte-foreclosed
independently of how lean the carrier body is. What remains of loop.c:1631 is
the no-carrier window H17 measured, insn_count in [120, 122], i.e. +64 RTL insns
at zero emitted cost - the requirement s6 could only meet with inadmissible
balanced-biv noise.

### s12.3 Structural sweep - five ordinary-C spellings of the body, all measured
Bodies and the apply script in `tmp/grind/func_8003C714/s12/`; each applied with
`s12/apply.py` and measured with `sandbox func_8003C714 --disable all`.
| body | spelling | score | build_insns |
|---|---|---|---|
| candidate.c | index-derived pointers, `i += 1`, limit 3 | 15 | 105 |
| pa_biv_step4.c | biv steps by 4 to 12; `src = base + i*2`, `dst = s0 + i` | 21 | 105 |
| pb_array_subscript.c | no pointer locals; `out[i*4 + 0x21]`, `base[i*8 + 4]` | 15 | 105 |
| pc_second_biv.c | second natural index `k += 8` feeding `src` | 15 | 105 |
| pd_three_indices.c | third natural index `m += 4` feeding `dst` | 23 | 107 |
Findings:
- pb is a BYTE-EQUIVALENT alternative spelling of the honest body (same score,
  same instruction count): the array-subscript form and the index-derived-pointer
  form converge after cse1, so future permuter seeds may use either.
- pc is byte-equivalent AND insn_count-equivalent. Its `.loop` dump
  (`Loop from 28 to 149: 56 real insns`, movables 77/83/90 all moved) shows why:
  the second index becomes a verified biv (reg 74, const 8) that replaces the
  `i * 8` giv multiply and is then eliminated, so the loop's RTL insn count is
  unchanged at 56. A natural extra induction variable is free in BYTES and free
  in insn_count - it buys nothing on the loop.c:1631 gate.
- pd shows the free ride ends at two indices: a third index costs +2 emitted
  instructions (107) and score 23.
- pa changes the biv's scale rather than adding one; the byte count stays at 105
  but the emitted arrangement moves further from the target (21).

- [s12] Chassis floor re-measured this session: candidate.c scores 15 (target 104 / build 105, rules_dropped 0, cheat_asm_stripped 9).

- [s12] asm/funcs/func_8003C714.s contains exactly one label (.L8003C754, line 18) and exactly one branch (bnez $v0, .L8003C754 at line 77) besides the return - the shipped function has exactly one loop.

- [s12] moved_once is alloca'd and bzero'd per function at loop.c:343-344 and written at exactly one site, loop.c:1912; scan_loop rejects any loop whose scan_start is not a CODE_LABEL at loop.c:568-576.

- [s12] With s11/varE.c applied, the emitted func_8003C714 in tmp/grind/func_8003C714/s12/varE_carrier.s carries TWO labels (.L133, .L141) and TWO backward branches.

- [s12] Shipped CC_FLAGS (engine/buildconfig.py:43) = '-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel' - no -funroll-loops, so a >=2-trip loop's backward branch always survives to emission.

- [s12] Structural sweep, all ordinary C, all measured with sandbox --disable all: candidate 15/105, pa_biv_step4 21/105, pb_array_subscript 15/105, pc_second_biv 15/105 (insn_count still 56), pd_three_indices 23/107.

- [s12] The .loop dump for pc_second_biv shows the added index recognised as biv reg 74 (const 8), 'biv 74 was eliminated', and the loop still at 56 real insns with movables 77 (life 1), 83 (life 1), 90 (life 31) all moved.


## s13 (2026-09-05) - structural modality

### s13.0 Chassis re-check (first action)
`memory/grind/func_8003C714/candidate.c` applied over the `INCLUDE_ASM` line in
src/code6cac_c2.c and measured with `sandbox func_8003C714 --disable all`:
score 15, target_insns 104, build_insns 105, rules_dropped 0,
cheat_asm_stripped 9 - identical to the s12 record. The ledger floor of 15 is
CURRENT and every banked spelling conclusion remains chassis-valid.
The `.loop` movable table regenerated this session is also unchanged:

    Loop from 25 to 146: 56 real insns.
    Insn 33: regno 78 (life 1), move-insn savings 1  moved to 203   <- symbol_ref D_80106A58
    Insn 46: regno 84 (life 1), move-insn savings 1  moved to 205   <- const_int -1851608123 = 0x91A2B3C5
    Insn 60: regno 91 (life 31), move-insn savings 1  moved to 207  <- const_int -2004318071 = 0x88888889

The post-loop RTL of the three hoisted insns was read this session for the first
time (tmp/grind/func_8003C714/s13/baseline.loop, insns 203/205/207): every one of
them is `(set (reg) <CONST_INT or SYMBOL_REF>)` with a matching REG_EQUAL note and
NO register operand in the source. That fact is what closes s13.2 below.

### s13.1 Mandated kill re-audit
- `tools/fake_ablate.py --func func_8003C714 --file code6cac_c2 --candidate
  memory/grind/func_8003C714/candidate.c` -> "no FAKE-annotated constructs found
  ... nothing to ablate". No banked lever was measured with a FAKE carrier on a
  target pseudo.
- The instance kill whose form sat closest to the target is s12's K37
  (`tmp/grind/func_8003C714/s12/pb_array_subscript.c`, the array-subscript
  chassis). Re-measured on the current chassis: score 15, build_insns 105,
  rules_dropped 0 - byte-equivalent to candidate.c, exactly as s12 recorded.
  K37 STANDS as a second permuter seed, not as an improvement.

### s13.2 The two remaining loop.c:1631 escape routes, both closed at the source
s11/s12 left the desirability inequality with two unmeasured escape routes that
do NOT go through `insn_count`. Both were run down in the compiler source this
session and one of them was measured.

**Route 1 - the `m->forces` skip (loop.c:1594).** `move_movables` only considers
a movable when `(! m->forces || m->forces->done)`. A movable whose `forces`
pointer targets a movable that was NOT moved is skipped entirely and stays in the
loop at zero insn_count cost - precisely the target's arrangement. `m->forces` is
written in exactly one place, `force_movables` at loop.c:1221, and only when
`INSN_UID (m->insn) == regno_last_uid[m1->regno]`, i.e. when the insn that SETS
m's pseudo is the last insn mentioning m1's pseudo. The 0x91A2B3C5 movable's
setting insn is `(set (reg:SI 84) (const_int -1851608123))`; it mentions no
register other than its own destination, so no earlier movable's register can
die in it and `force_movables` can never attach a `forces` pointer to it. The
same holds for the other two movables in this loop (symbol_ref and const_int).
Note also that `force_movables` does `m1->savings += m1->savings` and
`m1->lifetime += m->lifetime`, i.e. it makes the FORCING movable strictly more
desirable - so even if the shape existed it would drive m1 to `done` and then
loop.c:1632's third disjunct would force-move the magic anyway.

**Route 2 - `may_not_move` via a two-basic-block set (loop.c:3037).**
`count_loop_regs_set` sets `may_not_move[regno]` when `n_times_set[regno] > 0 &&
last_set[regno] == 0`, and `last_set` is zeroed only at a CODE_LABEL or a
JUMP_INSN (loop.c:3089-3090). So the trigger needs ONE pseudo SET twice with a
label or jump between the two sets, inside the loop. Measured spelling
(`tmp/grind/func_8003C714/s13/pa_twobb_div.c`, banked as
`rejected/twobb-division-gives-two-matched-pseudos-not-may-not-move.c`): the
/1800 division duplicated into both arms of an `if (i != 0)` inside the loop, the
else arm perturbed (`(t + 1) / 1800`) so jump.c cannot cross-jump the arms back
together. Result: score 27, build_insns 118 (+13 emitted instructions), and the
`.loop` movable table reads

    Loop from 25 to 173: 67 real insns.
    Insn 33: regno 78 (life 1), move-insn savings 1  moved to 230
    Insn 50: regno 84 (life 2), move-insn savings 2  moved to 232
    Insn 71: regno 92 (life 1), done move-insn matches 50
    Insn 87: regno 99 (life 31), move-insn savings 1  moved to 234

- `may_not_move` did NOT fire. Each division site expands its magic into its own
  FRESH pseudo (84 and 92), so `n_times_set` is 1 for both and the loop.c:3037
  test can never see a second set of the same regno.
- What fires instead is `combine_movables` (loop.c:1245-1254), whose gate is
  `n_times_used[m->regno] == 1` - satisfied by exactly these single-set pseudos.
  It MATCHES 92 to 84 and raises the surviving movable's savings 1 -> 2 and its
  lifetime 1 -> 2, i.e. it moves the desirability product the WRONG way by a
  factor of four (119*1*1 = 119 becomes 119*2*2 = 476).
This reproduces s8's K23 outcome and now supplies its mechanism from the dump
rather than from the score: the C programmer cannot name the pseudo that holds a
division's magic constant, and cse1's only transform is to replace USES, never to
add a second SET to an existing pseudo, so the two-BB admission trigger has no
C-level handle for a compiler-generated constant.

Independently of the pseudo question, the branch cost is structural: the extra
basic block cost +13 emitted instructions here, and any interior BB split needs
at least one emitted branch or an extra referenced label (jump.c deletes
unreferenced labels), while the target function contains exactly ONE CODE_LABEL
(.L8003C754) and exactly ONE branch (`bnez $v0, .L8003C754` at 8003C83C) besides
its five `jal`s and the closing `jr $ra`.

### s13.3 `threshold` re-derived once more, and the -3 decrement bounded
- loop.c:532 `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)`; with
  n_non_fixed_regs = 60 that is 122 here and 61 with a call in the loop.
- loop.c:1719 and loop.c:1904 are the ONLY writes to `threshold` inside
  `move_movables`, both `threshold -= 3`, and both are reached once per MOVED
  movable regardless of how many insns that movable carries (`m->consec`). So a
  consec-group movable still buys only -3 while emitting several preheader
  instructions - there is no cheaper order dial than K24/K30 already measured.
- `m->savings = n_times_used[regno]` (loop.c:793) and n_times_used is a straight
  bcopy of n_times_set (loop.c:597), re-confirming s10's H15: savings counts
  SETS, floor 1. `m->lifetime` floor is 1 (s10 K29). So the LHS of loop.c:1631
  is bounded below by `threshold` itself and the whole inequality reduces to
  H17's window `insn_count in [120, 122]`.

### s13.4 The target's 23-instruction tail admits no loop-carried consumer
The s12 frontier asked whether any SEMANTICALLY REAL loop-carried value could
supply the free insn_count s6 measured. That question is decided by the target's
post-loop tail, which was enumerated instruction by instruction this session
(asm/funcs/func_8003C714.s, 8003C844..8003C8B0, 23 instructions):

    jal func_8001CD68 / addiu $a0,$sp,0x10        -> func_8001CD68(buf)
    lhu 0x10($sp) ; sb 0x2D($s0)                  -> *(u8*)(s0+0x2D) = *(u16*)buf
    lbu 0x12($sp) ; sb 0x2E($s0)                  -> *(u8*)(s0+0x2E) = buf[2]
    lbu 0x13($sp) ; sb 0x2F($s0)                  -> *(u8*)(s0+0x2F) = buf[3]
    lui/lhu %hi/%lo(D_80101ED2) ; sb 0x30($s0)    -> *(u8*)(s0+0x30) = *(u16*)&D_80101ED2
    addiu $a0,1 ; addu $a1,0 ; addu $a2,0 ; addu $a3,0 ; jal disp_SetFramebufferMode
    addiu $v0,0x1F ; lui/sw %lo(D_800A37B8) ; lui/sh %lo(D_800A3834)
    lw $ra ; lw $s0 ; addiu $sp ; jr $ra ; nop    -> epilogue

Every instruction maps 1:1 onto the eight source statements candidate.c already
carries; none of them reads a register that the loop could have left live. So a
loop-carried accumulator would have to be dead at the loop exit, which is exactly
the inert-padding shape s6 banked as inadmissible
(rejected/balanced-biv-noise-64-insns-d0-but-inadmissible.c). The free
insn_count channel is real (s6 K17) and its only known carriers are dead.

### s13.5 Build-vs-target instruction diff (residual confirmed, no tail residual)
The candidate's cc1 output (`tmp/grind/func_8003C714/dumps/code6cac_c2.s`) was
diffed statement-for-statement against the target disassembly this session. The
ONLY structural difference is in the preheader/loop split: build emits
`li $9,0x91a20000 / ori $9,$9,0xb3c5` in the PREHEADER (8 preheader instructions
vs the target's 6) and therefore lacks the target's in-loop
`lui $v0,0x91a2 ... ori $v0,$v0,0xb3c5` pair; the mfhi temp lands in $10 (t2)
where the target uses $t1 because $t1 is occupied by the hoisted magic. The tail
and the prologue are instruction-for-instruction identical modulo the pseudo-op
spellings maspsx expands. There is no second, independent residual to attack.

- [s13] Chassis re-check (first action): candidate.c applied over the INCLUDE_ASM line scores 15, target_insns 104, build_insns 105, rules_dropped 0, cheat_asm_stripped 9 — identical to the s12 record, so every banked spelling conclusion remains chassis-valid.

- [s13] The baseline .loop movable table is unchanged: 'Loop from 25 to 146: 56 real insns' with Insn 33 regno 78 (life 1, symbol_ref D_80106A58), Insn 46 regno 84 (life 1, const_int -1851608123 = 0x91A2B3C5) and Insn 60 regno 91 (life 31, const_int -2004318071 = 0x88888889), all three moved.

- [s13] First read of the hoisted insns' RTL (tmp/grind/func_8003C714/s13/baseline.loop, insns 203/205/207): all three are (set (reg) <CONST_INT|SYMBOL_REF>) with a REG_EQUAL note and NO register operand in the source — the fact that closes the m->forces route.

- [s13] loop.c:532 threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs) = 122 here (61 with a call); loop.c:1719 and loop.c:1904 are the only writes inside move_movables, both 'threshold -= 3' and both once per MOVED movable regardless of m->consec.

- [s13] Two-BB probe measured: score 27, build_insns 118; movable table shows 'regno 84 (life 2) savings 2' and 'regno 92 (life 1) done matches 50' — combine_movables matched two distinct pseudos, may_not_move never fired.

- [s13] Target control-flow census: exactly one CODE_LABEL (.L8003C754) and exactly one branch (bnez $v0 at 8003C83C) in the whole function, besides five jal and the closing jr $ra — so no interior basic-block split fits in the target's bytes.

- [s13] Build-vs-target instruction diff of the candidate: the only structural difference is the preheader/loop split (build emits li/ori of 0x91A2B3C5 in an 8-instruction preheader vs the target's 6, and lacks the target's in-loop lui/ori pair; the mfhi temp lands in $10 instead of $t1 because $t1 holds the hoisted magic). Prologue and tail are instruction-for-instruction identical, so there is no second independent residual.

- [s13] Mandated kill re-audit: fake_ablate reports no FAKE constructs in candidate.c; s12's closest-to-target instance kill K37 re-measures at 15 / 105 on the current chassis and stands.
