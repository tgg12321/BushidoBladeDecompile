# Hypothesis ledger — func_80061250

## s1 (2026-08-26, recon)

- H1 "The function mirrors COMPLETED-C sibling func_8006156C (similarity 1.000) and the
  6156C body template closes it" — **CONFIRMED** (with the block-offset flag spelling
  `(&D_800F1154)[5]/[6]` and the `*p++` tail: 59/59 insns, sandbox 2 = reloc-attribution
  artifact only, full-build SHA1 == oracle with the candidate applied, measured s1).
- H2 "The flags are volatile (MEM_VOLATILE_P unfolded-la signature) and the chassis-era
  `extern volatile u8 D_800F1159` was load-bearing" — **KILLED**: volatile is sandbox-
  stripped (ungranted, not in volatile_extern_allowlist.txt) and the raw cc1 probe shows
  it does NOT fix the +1-access absolute folds nor the shared `lui 0x21`; the real
  mechanism is the compound-const cse anchor from the D_800F1154+5 spelling, no volatile
  involved. The stray volatile decls on main (text1b.c old line 3271, text1b_b.c:591) were
  chassis debt; the candidate replaces the text1b.c one with `extern u8 D_800F1154;`.
- H3 "A D_800F1159-addend-0 spelling can reproduce the la-anchored shape (and thus score
  sandbox 0)" — **KILLED** (three spellings measured: `(&D_800F1159)[k]` = 19 with
  absolute folds everywhere; `u8 *flag = &D_800F1159` = 25 with +1 folds + shared lui;
  volatile variant = same folds in raw cc1). Plain-symbol cse equivalence always folds the
  derived addresses; only the compound `(const (plus sym k))` equiv survives, which
  requires anchoring at D_800F1154+5 and therefore relocs the two addius as
  `D_800F1154+5` ≠ target's `D_800F1159+0` — same address, 2 unmaskable scorer diffs.

## Frontier (for the operator/driver — the C is done)

1. Integration: the candidate in src/text1b.c is byte-exact at link (SHA1 == oracle,
   0 rules, 0 cheat-asm). Blocked from sandbox-0 ONLY by engine/score.py's named-symbol
   reloc conservatism vs the splat symbol split (D_800F1159 aliases D_800F1154+5).
   Resolutions (any one): (a) scorer resolves named-symbol relocs to absolute addresses
   via undefined_syms/symbol_addrs before comparing (extends the 2026-08-07 section-symbol
   masking fix); (b) aggregate-merge the D_800F1154..D_800F115C flag-byte splat symbols
   into one `extern u8` array (base-register+offset stride evidence is in the target
   itself; the family — 6133C/613C8/61454/614E0/6156C/61250 — all index this block) and
   re-express the target reloc as D_800F1154+0x5; (c) operator integrates directly
   (verify-oracle green with the body in place; retire is a no-op — 0 rules).

## s1 re-run (2026-08-26, recon; HEAD 351b99be)

- H4 "The sandbox floor is artifact-locked at 2 by unmasked named-symbol reloc addends
  (D_800F1154+5 vs D_800F1159+0) and only an operator-side remedy can close it" —
  **KILLED**: the identical candidate body scored sandbox 0 (59/59) this session and
  full-build SHA1 == oracle. No scorer/symbol commit landed between the two
  measurements; the artifact theory does not describe the current chassis. The
  escalation/handoff framing is retired — the function closes through the normal
  candidate-ready pipeline.

## Frontier

(empty — sandbox 0 + oracle MATCH proven this session with the body in src/text1b.c;
nothing remains but driver verification → layer-1 → Judge.)

## s2 (2026-08-26, recon; HEAD 95de1421)

- H5 "The s1 layer-1 FAIL was pure citation drift (pre- vs post-application line
  numbers) and re-citing against the post-edit file closes it" — **CONFIRMED
  (structurally)**: sibling func_8006156C grepped at src/text1b.c:3370-3397 with
  the candidate applied vs 3342-3369 without; the prior self-vet used the
  without-numbers while the reviewer read the with-file. s2 self_vet.md carries
  only post-edit re-grepped citations + the sibling's Match commit 450f69d1.
  Final confirmation rests with layer-1/Judge.

## Frontier

(empty — sandbox 0 proven s2 with the body in src/text1b.c; self_vet.md carries
verified citations; nothing remains but driver verification → layer-1 → Judge.)

## s3 (2026-08-26, recon; HEAD ecc1e876)

- H6 "A real aggregate declaration — `extern u8 D_800F1154[];` + `D_800F1154[5]/[6]`
  ARRAY_REF indexing (the reviewer's prescribed direction, mirroring committed
  sibling func_800619F0) — produces the same compound-const cse anchor and thus the
  same 59/59 bytes as the banned pointer-pun" — **CONFIRMED**: sandbox 0 (59/59)
  measured this session with the body applied; probe banked at
  tmp/grind/func_80061250/s3/probe_array_spelling_sandbox0.c; src reverted after
  measurement.

## Frontier

1. (blocked on ruling) Option A — resubmit with the TU-consistent array-extern
   spelling as ordinary C (committed precedent func_800619F0, census buffer model).
   Mechanism: identical RTL to the proven form; risk is layer-1 calling it a
   respelling of the banned pun. Next probe: none — measurement complete; needs the
   s3 ruling answer.
2. (blocked on ruling) Option B — full 5-prong aggregate merge: shared-header
   `extern u8 D_800F1154[N];` (or struct), respell func_8006156C's committed pun +
   text1b_b.c externs, remove D_800F1159 (+ extent-dependent others) from
   undefined_syms_auto.txt at integration (mechanically possible: only the self .s
   and unlinked asm/text1b.s reference it — verified s3). Needs driver
   scope-widening (sibling TU + header + config are outside one-function scope).
   Next probe: on a YES, byte-neutrality sweep of every respelled consumer +
   verify-oracle --rebuild.
3. If the ruling rejects both A and B: the only remaining honest spellings anchored
   at D_800F1159 addend-0 are measured dead (H3, s1: 19/25/inert) — re-open the
   ladder at the flag-access shape level (e.g. struct-typed extern at 0x800F1159
   with 2 u8 members — untried, but expected to fold absolute per the H3 mechanism).

## s4 (2026-08-26, recon; HEAD 2b38d6f6)

- H7 "The D_800F1159 retirement the Judge made binding (delete
  undefined_syms_auto.txt:516 + the two dead `extern volatile u8 D_800F1159;`
  declarations) is oracle-safe — i.e. the symbol is genuinely dead once this
  function's INCLUDE_ASM is replaced, and deleting it does not perturb the link" —
  **CONFIRMED**: with all three edits staged, `sandbox func_80061250 --disable all`
  printed 0 (59/59, rules_dropped 0) and a full clean `build` printed SHA1
  62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle. A pre-edit tree-wide grep had
  already shown the only other references were asm/text1b.s (unlinked) and this
  function's own asm/funcs/func_80061250.s.
- H8 "The array-extern spelling plus the retirement can be staged entirely within
  the driver's scope grant, so the function closes through the normal pipeline
  rather than needing another handoff" — **CONFIRMED**: the grant at
  tools/grinder/scope_allow.txt:34 covers exactly the three files the change needs
  (src/text1b.c, src/text1b_b.c, undefined_syms_auto.txt); nothing outside it was
  touched.

## Frontier

(empty — the function is byte-proven and fully staged. Remaining path is mechanical:
driver sandbox-0 re-verify → scope check → layer-1 cheat-reviewer → Judge →
retire/verify-oracle → queue done. The one non-blocking loose end banked in
evidence.md s4 is the now-arguably-redundant census NAME entry
named_syms.txt:1913 for 0x800F1159, which is outside this function's scope grant and
is inert against the oracle.)
