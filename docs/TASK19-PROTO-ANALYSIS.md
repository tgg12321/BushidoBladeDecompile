# Task #19 — text1b prototype-contradiction analysis + duplicate-glabel survey + w6_canon_check disposition

Prepared 2026-08-07 in an isolated worktree (branch `worktree-agent-acbdbab88162d8f4b`,
base `9fb39395`). Analysis only — NO build was run (toolchain absent in the worktree);
every codegen claim below is asm-evidence-based and must be oracle-verified on main
per `docs/TASK19-APPLY-PLAN.md`.

Source of the debt list: `docs/superpowers/specs/2026-08-06-tu-resplit-campaign.md`
("Recorded, not fixed — pre-existing signature debt in text1b").

## 0. Why the contradictions compiled at all (load-bearing context)

`extern s32 func_80052754(s32, s32, s32);` and
`extern s32 func_80052754(s32, s32, s32, s32);` at file scope in the SAME TU is a
C constraint violation — GCC 2.7.2's `duplicate_decls` emits
"conflicting types for ..." as an ERROR. The build still "succeeds" because the
Makefile compile recipe is an unguarded pipe
(`$(CPP) ... | $(CC1) ... | ... | $(AS) -o $@`, Makefile:159) with no
`pipefail`: only `as`'s exit status is observed, cc1's nonzero exit and stderr
are swallowed, and cc1 streams out the full .s despite the error. The oracle
(current SHA1 == `62efab4f...`) proves the emitted code is what we want — but
diagnostics for the whole file are being silently discarded. The apply session
should capture cc1 stderr for text1b.c / text1b_b.c once (step 0 of the plan)
to confirm the diagnostics disappear after the fix.

The same swallowed-error family also affects DATA decls in these TUs (e.g.
`extern s32 D_800A33F4;` vs `extern u8 *D_800A33F4;` in both text1b.c and
text1b_b.c; `D_8009BD24` declared `u8`, `s32`, and `u8[]` at text1b.c:3076 /
7381 / 7434). NOT fixed here — data-type unification changes
pointer-arithmetic typing inside matched bodies and needs its own
byte-verified pass. Recorded as follow-up debt.

## 1. func_80052754 — 3-arg vs 4-arg prototypes (HARD conflict, fixed)

* **True signature (asm evidence):** `asm/funcs/func_80052754.s` is a 13-insn
  GTE `sqr` leaf reading ONLY `$a0/$a1/$a2` (`mtc2 $a0,$9; mtc2 $a1,$10;
  mtc2 $a2,$11`), returning MAC1+MAC2+MAC3 in `$v0`. It is canonical-asm
  (authorized, `inline_asm_canonical.txt`). **`s32 func_80052754(s32, s32, s32)`.**
* **Contradiction:** declared `(s32,s32,s32)` at src/text1a_post.c:375,
  src/text1b.c:1467, src/text1b_b.c:138 — and `(s32,s32,s32,s32)` at
  src/text1b.c:1502 and src/text1b_b.c:145 (same-TU file-scope conflicts in
  both text1b TUs).
* **The 4-arg call:** `func_8005344C` (text1b.c) passed a 4th argument
  `*(s32 *)((u8 *)D_800A33F4 + 0x1C)`. Target-asm evidence that dropping it is
  byte-neutral: the sibling `func_80053304` makes the IDENTICAL call with 3
  args and its target call block (`asm/funcs/func_80053304.s:37-41`) is
  instruction-identical to `func_8005344C`'s (`asm/funcs/func_8005344C.s:33-37`)
  — the "4th arg" value already sits in `$a3` as the subtraction operand of
  arg1, so it costs zero instructions either way. GCC's emitted caller code is
  expected unchanged; MUST be oracle-verified.
* **Fix (committed):** unified all decls to `(s32, s32, s32)`; removed the 4th
  argument from the `func_8005344C` call; deleted the dead conflicting decl
  pair in text1b_b.c (that TU contains no calls — the block was resplit
  residue).
* **Affected TUs / blast radius:** src/text1b.c (callers func_80053304,
  func_8005344C), src/text1b_b.c (decls only), src/text1a_post.c (already
  correct, untouched; caller func_80041EB0).
* **Risk:** the call-arg drop in func_8005344C is the ONE edit in this task
  with a real (small) codegen-change risk. Fallback if oracle goes red: revert
  just that call edit and declare `extern s32 func_80052754();` (unprototyped)
  in the 1502-region instead — unprototyped calls pass all word-size args
  identically and legally permit the extra arg.

## 2. func_80052D00 — `()` vs `(s32,s32)` (soft conflict, fixed; bonus item)

Not in the task's four but interlocked with the same decl blocks (the spec
recorded it: `()`, `(s32,s32)`, `()`).

* **True signature (asm evidence):** `asm/funcs/func_80052D00.s` reads incoming
  `$a0`->`$s3`, `$a1`->`$s4`; `$a2/$a3` are locals (written via `lw` before any
  read); no incoming stack args; NO `$v0` set on the exit path (jr with nop
  delay after register restores) — effectively `void (s32, s32)`. Declared
  return kept `s32` (callers all ignore the result; caller codegen for an
  ignored return is identical, and shrinking the diff was preferred).
* **Fix (committed):** all decls unified to `(s32, s32)` (text1b.c:1467,
  1534 regions; text1b_b.c:139). All call sites already pass exactly
  `(arg2, arg3)` — K&R->prototype with exact-word-type args is
  codegen-identical in GCC 2.7.2.
* **Affected TUs:** text1b.c (callers func_80053304, func_8005344C,
  func_80053584, func_80053614), text1b_b.c (decl residue only).

## 3. func_80073728 — K&R `()` governing 7 call sites vs `(s32,s32)` (fixed)

* **True signature (asm evidence):** `asm/funcs/func_80073728.s` (358 lines)
  reads `$a0`->`$s2` (pointer: `lw 0x0/0x4($s2)`, stores through `0xC($s2)`)
  and `$a1`->`$s6` (mode selector compared against 4/5/6); `$a2/$a3` written
  before read; no incoming stack args; returns `$v0 = $s3` (a
  written-back pointer/offset). **`s32 func_80073728(<obj>*, s32)`** — typed
  `(s32, s32)` to agree with the pre-existing prototype at text1b.c:5653 and
  the `(s32)arg1` call at :5665.
* **Contradiction:** K&R `extern s32 func_80073728();` at text1b.c:2753 (was
  :2958 pre-resplit) governed 7 call sites (callers func_8005D46C,
  func_8005D554, func_8005FA98, func_80060544); `(s32, s32)` at :5653 governs
  func_8006A494. Legal C (the `()` merges), but signature-masking debt.
* **Fix (committed):** :2753 decl -> `(s32, s32)`; the 7 call sites' arg casts
  changed `(GameObj *)&s` / bare `&s` -> `(s32)&s` (same value, same register —
  byte-neutral; avoids ptr-to-int-param diagnostics under the new prototype).
  text1b_b.c residue decl :462 likewise unified.
* **Risk:** low. K&R->prototyped call with word-size args is
  codegen-identical; casts are value-preserving. Oracle-verify regardless.

## 4. func_8006E534 — RESOLVED, no edit

The spec's contradiction was between a (since-REMOVED) resplit stub declaring
`(s32,s32,s32,s32)` and the extern `(s32,s32,u8*,s32)`. Post-resplit the tree
has exactly ONE decl (text1b.c:7435, `void func_8006E534(s32, s32, u8*, s32)`)
and one caller (func_80077984, text1b.c:7437). Asm
(`asm/funcs/func_8006E534.s`): reads all four arg regs (`$a0`->`$s0` pointer
into gp-rel state, `$a1`->`$s2`, `$a2`->`$s1` pointer (`lw 0x14($s1)`),
`$a3`->`$s3`); 4 args confirmed. `u8*` vs `s32` for arg2 is
codegen-indistinguishable at the call (D_8009BD24 array decays to pointer).
**No change needed; contradiction already retired by the resplit.**

## 5. motion_ShiftControl (now func_80070F78) — RESOLVED, no edit

The phase-2 reset wave (2651e2e5) reset the name: glabel and all references
are now `func_80070F78` (`motion_ShiftControl -> func_80070F78` in
docs/naming/reset-wave-dryrun.json; docs/engine/motion.md still uses the old
name — prose only). The spec's "4 params against an extern with 2" 4-param
side was the removed resplit stub. Post-resplit the tree has two IDENTICAL
decls `extern void func_80070F78(s32 a0, s32 *prim);` (text1b.c:6306,
text1b_b.c:789) and one caller each. Asm (`asm/funcs/func_80070F78.s`, 810
insns): reads `$a0`->`$s7` (object pointer) and `$a1`->`$s1` (prim/output
pointer); `$a2/$a3` written before read; no `$v0` on exit — `void`, 2 args
confirmed. **No change needed.**

## 6. Duplicate-glabel / duplicate-address survey (census re-verification)

Scanner: `tmp/dup_glabel_scan.py` (worktree tmp, gitignored) + a start-address
variant. Ground truth at base `9fb39395`:

* **1,437 `asm/funcs/*.s` files** (census README says 1,438 — the phase-2
  reset wave DELETED `cpu_get_dist_2.s`, which duplicated `func_800324D0.s`
  at 0x800324D0).
* **Both duplicate-GLABEL pairs SURVIVE** the naming waves (the handoff's
  "cdrom pair now renamed" assumption is wrong — both files still exist;
  the rename wave rewrote the glabel INSIDE func_800806A4.s, it did not
  remove the extra file):
  * `cdrom_FramesToBcd.s` + `func_800806A4.s` — both `glabel cdrom_FramesToBcd`
  * `stage_InitCollision.s` + `func_8003F274.s` — both `glabel stage_InitCollision`
* **NEW finding — 5 more duplicate-ADDRESS pairs** invisible to a glabel scan
  (two files, same instruction stream, DIFFERENT glabels; the `func_*.s` side
  was re-added by the `67caeb5b` "55 new stubs" splat re-run after the
  `c402f457` Kengo rename wave had renamed the originals):
  * `cpu_get_dist.s` / `func_8003032C.s` (0x8003032C)
  * `cpu_set_move_command_and_dir.s` / `func_80030A2C.s` (0x80030A2C)
  * `cpu_check_same_dir_timer.s` / `func_8003339C.s` (0x8003339C)
  * `replay_camera_Init.s` / `func_80036D98.s` (0x80036D98)
  * `special_camera_get_rot_dir.s` / `func_80037348.s` (0x80037348)
* All 7 pairs verified instruction-stream-identical. None of the 14 files is
  referenced by any `INCLUDE_ASM` (all 7 functions are decompiled to C), so
  they are engine-reference/census pollution only — deleting the stale side
  is build-neutral by construction.
* **Stale side per pair:** for the two glabel pairs, the `func_*.s` file
  (its stem matches no glabel it contains); for the five address pairs, the
  `func_*.s` stub (the named file matches the C definition name still in use:
  `cpu_get_dist` etc. are KEEP names defined in src/code6cac_b.c, and
  named_syms.txt carries `cdrom_FramesToBcd = 0x800806A4`,
  `stage_InitCollision = 0x8003F274`).
* **What the census should now say:** 1,437 files; minus `D_8007E08C.s`
  (data-as-code blob); minus 7 stale duplicate files = **1,429 unique
  functions** (equivalently: 1,434 distinct glabels minus the 5 address-dup
  aliases). The `docs/naming/README.md` derivation table (currently
  1,438 -> 1,437 -> "1,436") needs this rewrite, and AGENTS.md quotes
  "1,436 functions" from it.

## 7. tmp/w6_canon_check.py — the build-trigger flag

* **The issue:** the script's docstring claims "Read-only: engine/queue.json
  is never written" — true for the queue, FALSE for the build. Each
  `Q.mark_done(f)` calls `oracle.verify(rebuild=False)` (engine/queue.py:491),
  and `oracle.verify` runs `P.build_all()` whenever `build/bb2.exe` is
  MISSING (engine/oracle.py:140-142). So the "check" script silently triggers
  a FULL clean-driver build (and seizes the build lock) the moment build/ is
  stale-empty — precisely the hazard engine/queue.py's own comment (:402-408)
  warns about for refusal paths, and a real conflict while the Grinder runs.
  (`Q.generate()` also compiles per-file cheat-stripped objects — that is
  inherent to regen and stays.)
* **Disposition: KEEP, promoted** — canonical-authorization payoff checks
  recur (65 canonical-extraction asmfix entries are standing debt per
  [[asmfix-all-debt-end-state]]; future waves need the same 4-conjunct
  check), and tmp/ is unbacked. Promoted to **`tools/canon_payoff_check.py`**
  (committed) with the fix: refuse up front with an explicit message when
  `build/bb2.exe` is absent instead of letting mark_done trigger a build.
  Behavior otherwise identical. The tmp/ original should be deleted on main
  after the promotion lands (apply plan step).
* Not runnable in this worktree (no .venv/toolchain); the promoted script is
  a faithful copy + guard, but its first run on main should be watched.

## Follow-up debt recorded (out of scope here)

1. Conflicting DATA extern decls in text1b.c / text1b_b.c (D_800A33F4,
   D_8009BD24, D_800A33F8 s16/u16, ...) — same swallowed-error family;
   needs its own byte-verified pass.
2. The Makefile pipe swallows cc1's exit status project-wide. A
   `SHELL := /bin/bash -o pipefail` experiment is NOT proposed (it would
   surface errors in other files and block the build) — but a one-shot
   diagnostic capture of cc1 stderr per TU would inventory the remaining
   swallowed diagnostics cheaply.
3. docs/engine/motion.md + docs/engine/cross_reference.md still say
   `motion_ShiftControl` (prose; harmless but stale after the reset wave).
