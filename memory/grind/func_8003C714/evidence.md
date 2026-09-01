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
