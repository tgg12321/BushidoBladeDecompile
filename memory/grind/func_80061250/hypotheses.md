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
