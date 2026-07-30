# Hypothesis ledger — replay_camera_rob_back_loose3

Floor: engine metric **17** on the HEAD (cheat-carrying) form; **26** cheat-free
(build_insns 113 vs target 114). Work the 26.

## KILLED (session 1 — do NOT re-propose)

1. **Local declaration order of `angC` steers its hard register.**
   Mechanism: GCC assigns pseudos in `expand_decl` order, so decl position could flip a
   local-alloc quantity tie-break. Probe: `angC` declared (a) first, (b) in the
   `s32 angA, angB, angC;` group, (c) on its own line after angA/angB, (d) last.
   Result: **all four measured exactly 26 / build_insns 113** — inert.
   Banked as `rejected/declorder-angC-inert.c`.

2. **A narrower-type view of the `Judge` element reproduces target's `lhu; sll 16; sra 16`.**
   Mechanism: read the element unsigned and sign-extend explicitly (the sanctioned
   `u16-global-lhu-lbu-low-byte` narrow-pointer-view family). Probes, all 26 / 113:
   `u16 raw = Judge[i]; cosA = (s16)raw;` · `cosA = (s16)*(u16 *)&Judge[i];` ·
   `u16 cosA` with `(s16)cosA` at all four multiply sites · the last one repeated with all
   four `extern s16 Judge[]` decls in text1a_c.c retyped `extern u16 Judge[]`.
   Cause (confirmed): GCC 2.7.2 combine folds `ashiftrt(ashift(zero_extend(mem)))` into
   `sign_extend(mem)` = `lh` because the zero-extended value has exactly ONE use.
   Banked as `rejected/u16-retype-and-cast-at-uses-folds-to-lh.c` and
   `rejected/u16-ptr-view-cosA-folds-to-lh.c`.

## FRONTIER

**A. The raw `cosA` halfword needs a SECOND use of the zero-extended value, so combine
cannot fold `lhu+sll+sra` into `lh`.**
Mechanism: `simplify_shift_const` only rewrites the chain when the `zero_extend` dest is
single-use and dead after the shifts (`dead_or_set_p`); the matched precedent
`func_80065344` (text1b.c:14641) keeps its `lhu` precisely because the u16 value is also
stored back. Note the second use must survive to combine — GCC 2.7.2 runs `flow` (dead-code
deletion) BEFORE `combine`, so a purely dead second use is deleted first and cannot help;
it has to be a use that appears in the emitted output or is otherwise flow-live.
Next probe: find what that live second use IS in the target rather than inventing one —
re-derive the block with m2c (`asm/funcs/replay_camera_rob_back_loose3.s`) and check the
two rule-carrying siblings `_SelectSection` and `hirahira_w_ctrl_2`, whose targets show the
same `lhu`+sext signature (hirahira ×2), for a shared shape that consumes the raw u16.
Also worth checking: whether the sign-extension in target belongs to a value that reaches
the multiplies through a DIFFERENT C variable than the load's destination (two variables,
not one), which gives the load's dest a second reader naturally.

**B. `angC` must be allocated `$v1` (freeing `$v0` for the index temp and then for
`sinAxsinB_12`) by a local-alloc quantity-priority change — 13 of the 26 diffs.**
Mechanism: local-alloc (not global) already hard-assigns `angC` → `$v0` at greg insn 44;
priority comes from `reg_n_refs` / live-range length per quantity. Decl order is dead
(above), so the lever must change the ref count or live length of `angC` vs the index
temp — e.g. splitting the cos-index chain into its own named local (shortening `angC`'s
range), or computing the sin index through a separately-named local so the temp's quantity
becomes the longer-lived one. `.claude/rules/register-alloc-pure-c` Levers A–C are the
playbook; use the `BB2_ALLOC_DEBUG` / `BB2_PRIO_DEBUG`-instrumented cc1 if the plain
`.lreg` "used N times across M insns" numbers do not explain the tie.

**C. Cross-check against the two family siblings before more single-function probing.**
`_SelectSection` (10 rules) and `hirahira_w_ctrl_2` (63 rules) are the SAME 3x3
rotation-matrix shape in the same file, and their regfix comments describe the SAME two
mechanisms (`# Fix sinAxsinB_12 register: GCC puts in $4 (a0), target has $2 (v0)` and the
`la`→`$at` volatile-addressing block). A form that closes any one of the three is very
likely to close all three; conversely a mechanism present in all three is a property of the
original *source idiom*, not of this function's local details.

## [s1] The local declaration order of angC steers which hard register local-alloc gives it ($v0 vs target's $v1).
- mechanism: GCC creates pseudos in expand_decl order at function start, so declaration position can flip a local-alloc quantity priority tie-break.
- probe: Cheat-free baseline (pin + volatile removed) with angC declared four ways: first; inside 's32 angA, angB, angC;'; on its own line after angA/angB; last. sandbox --disable all on each.
- result: All four measured exactly score 26 / build_insns 113 — bit-identical outcome. Declaration order is inert for this allocation.
- verdict: KILLED

## [s1] Reading the cosA Judge element through a narrower unsigned view reproduces target's 'lhu; sll 16; sra 16' split shape without volatile.
- mechanism: Sanctioned narrow-view family (u16-global-lhu-lbu-low-byte): a u16-typed read forces lhu and an explicit (s16) cast forces the sign-extend pair, which is the 3-insn shape target has (and the reason target is 114 insns vs our 113).
- probe: Four spellings, each sandboxed: (a) 'u16 raw = Judge[i]; cosA = (s16)raw;'  (b) 'cosA = (s16)*(u16 *)&Judge[i];'  (c) cosA typed u16 with (s16)cosA at all four multiply sites  (d) spelling (c) repeated with all four 'extern s16 Judge[]' decls in src/text1a_c.c retyped 'extern u16 Judge[]'. objdump of each sandbox .o to inspect the emitted load.
- result: All four emit a single 'lh' — score 26 / build_insns 113 in every case. Confirmed cause: GCC 2.7.2 combine (simplify_shift_const) folds ashiftrt(ashift(zero_extend(mem))) into sign_extend(mem) whenever the zero-extended value is single-use and dead after the shift chain. Only 'volatile' (an un-combinable MEM) escapes the fold — and that is the cheat currently in HEAD.
- verdict: KILLED

## [s1] The cosA load shape (mechanism A) and the angC/sinAxsinB_12 register assignment (mechanism B) are one coupled root cause.
- mechanism: If the load shape drove the register cadence, fixing A would resolve B's 13 diffs as a cascade.
- probe: Compared the diff of the with-volatile build (which HAS target's correct lhu+sll/sra shape, score 17) against the no-volatile build (score 26).
- result: Both builds show the identical 13 B-diffs (angC $v0 vs $v1, index temp $v1 vs $v0, sinAxsinB_12 $a0 vs $v0). A and B are independent axes and must be closed separately.
- verdict: CONFIRMED
