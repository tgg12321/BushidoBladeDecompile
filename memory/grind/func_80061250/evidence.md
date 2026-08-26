# Evidence bank — func_80061250

## s1 (2026-08-26, recon) — MATCH FOUND; full-build SHA1 == oracle; sandbox floor 2 is a symbol-alias artifact

- **Target shape (asm/funcs/func_80061250.s, 59 insns):** head = flag test/clear on the
  D_800F1154 flag block bytes +5/+6 (splat names byte +5 as its own symbol `D_800F1159`),
  with ONE base register ($v1) la-materialized once (`lui/addiu %hi/%lo(D_800F1159)`) and
  every flag access offset off it (`lbu 0/1($v1)`, `sb 1/0($v1)`, re-read of byte 0 after
  the clear); `addiu $a0,$v1,1` computed in a branch delay slot and later stored to
  D_800F1180; per-arm FULL constant materialization (lui/ori 0x210009 in one arm, lui-in-
  delay-slot/ori 0x21000A in the other — no shared `lui 0x21`); L2B0 arm reloads the s32*
  handle from gp-rel D_800A3468 and stores &flags to +0x14 off it; tail = jal func_80060A68
  then 3-load/3-store copy `0/4/8($s0)` → D_800F1140/44/48 with the 0xFF0060 mask store to
  gp-rel D_800A3464 interleaved between load 3's lui/ori and the last store.
- **The function is a structural clone of COMPLETED-C sibling func_8006156C**
  (src/text1b.c:3343, Judge PASS 2026-07-22 21:27; find_duplicates similarity 1.000,
  59/59). 6156C's target anchors its base at `D_800F1154+0x1` with the SAME la-base +
  offset-access + per-arm-constant shape, and its committed C spells the flags as
  `(&D_800F1154)[1]/[2]` with a `*p++` walking-pointer tail.
- **Measured spelling ladder (sandbox --disable all, 59-insn target):**
  - `(&D_800F1159)[0]/[1]` + walking-pointer tail: **19** (62 insns). Tail matched target
    EXACTLY (the `*p++` folds to `0/4/8($s0)` with the mask interleaved right); head
    diverged — every flag access folded to absolute `lui/%lo` macro addressing, no base
    register formed. Banked: rejected/s1-absolute-fold-19.c.
  - `u8 *flag = &D_800F1159;` + flag[k]: **25** (59 insns). Base register formed for the
    [0] accesses and the stored address value, but all three [+1] accesses folded to
    absolute `D_800F1159+1`, AND the two 0x2100xx constants shared one hoisted
    `lui 0x21` in the beqz delay slots (target materializes each arm's constant fully).
    Banked: rejected/s1-ptr-local-plus1-folds-25.c.
  - `volatile u8 *flag = &D_800F1159;` (+ volatile extern): sandbox-INERT — the ungranted
    volatile is stripped before scoring (cheat_asm_stripped 169→170); D_800F1159 is NOT in
    volatile_extern_allowlist.txt. Raw cc1 probe (tmp/grind/func_80061250/s1/
    probe_volatile.s) shows volatile fixes the offset-0 accesses but the +1 accesses STILL
    fold absolute and the constants still share the lui. Not needed anyway (see below);
    volatile is NOT the mechanism. Banked: rejected/s1-volatile-stripped-inert.c.
  - **`(&D_800F1154)[5]/[6]` (the 6156C-mirror block-offset spelling): sandbox 2, 59/59,
    and `build` full-clean SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa (MATCH,
    measured THIS session with the candidate in src/text1b.c).** This is the current
    src/text1b.c body and memory/grind/func_80061250/candidate.c.
- **Mechanism of the shape (measured, not guessed):** cc1 (GCC 2.7.2) keeps a la-anchored
  base register for a flag-block access run ONLY when the anchor's cse equivalence is a
  COMPOUND constant (`(const (plus D_800F1154 5))`). With a plain-symbol anchor
  (`&D_800F1159`, addend 0) cse substitutes the symbol into every derived MEM address and
  folds the accesses to absolute assembler-macro addressing — measured in both the [0]/[1]
  spelling (19) and the pointer-local spelling (25, +1 folds only). The compound-equiv
  anchor also stops the cross-arm `lui 0x21` share (per-arm full li, matching target).
  This is exactly why sibling 6156C's committed `(&D_800F1154)[1]` spelling matched: its
  first-access anchor is symbol+1 ≠ plain symbol.
- **The residual sandbox 2 is a scoring artifact, not a byte difference:** the two diffs
  are the two `addiu` instructions carrying `%lo(D_800F1154)+5` (imm field 5, reloc vs
  D_800F1154) where target.s carries `%lo(D_800F1159)+0` (imm field 0, reloc vs
  D_800F1159). D_800F1159 = 0x800F1159 = D_800F1154+5 (undefined_syms_auto.txt:516), so
  both resolve to identical instruction words at link — proven by the full-build SHA1
  MATCH above. engine/score.py deliberately does NOT mask named-symbol reloc addends
  (auto-memory project/sandbox-lo16-text-addend-false-distance, FIXED-scope note: "Named-
  symbol relocs are NOT masked"), so NO C spelling that anchors off D_800F1154 can score
  below 2, and (measured above) no D_800F1159-addend-0 spelling reproduces the target
  shape. The sandbox floor for this function is artifact-locked at 2 with the bytes
  already exact.
- Canonical verdict C (queue record; family verdict per docs/grind/decisions.md:1347).
- Zero regfix/asmfix rules; the retired chassis' `register asm("$2"/"$3")` pins are GONE
  (the walking-pointer tail replaces them, same cure as 6156C); no volatile, no FAKE
  constructs — every statement in the candidate is semantic and mirrors the Judge-PASSed
  6156C body verbatim (offsets/codes/mask differ only as the data differs).
- Artifacts: tmp/grind/func_80061250/s1/cc1_probe.sh (raw-pipeline cc1 probe harness),
  tmp/grind/func_80061250/s1/probe_volatile.s (raw cc1 output, volatile spelling).

## s1 re-run (2026-08-26, recon; HEAD 351b99be) — sandbox 0 + oracle MATCH re-proven; the "artifact floor 2" claim is FALSIFIED

- The banked candidate (identical body, `(&D_800F1154)[5]/[6]` block-offset spelling +
  `*p++` tail) was re-applied to src/text1b.c, replacing `extern volatile u8 D_800F1159;`
  + `INCLUDE_ASM("asm/funcs", func_80061250);` at old lines 3271-3272.
- **`sandbox func_80061250 --disable all` printed score 0, 59/59 insns** (rules_dropped 0,
  cheat_asm_stripped 169 — all from other functions; nothing stripped from this body).
- **Full clean `build`: SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa — oracle MATCH**,
  measured this session with the body in place.
- The discarded prior s1's central claim — "engine/score.py does not mask named-symbol
  reloc addends, so no D_800F1154-anchored spelling can score below 2" — does NOT
  reproduce on this chassis. target.s still relocates its two flag-base addius against
  D_800F1159 (asm/funcs/func_80061250.s:5-6,31-32) and the C against D_800F1154+5, yet
  the scorer prints 0. No engine/score.py or symbol-file commit landed in between (last
  scorer commit is 4dfc7223, the 2026-08-07 section-symbol masking fix), so the prior
  "2" was a mis-measurement or a transient state, not a scorer property. The three
  escalation remedies proposed in the discarded decisions.md entry (scorer fix /
  aggregate merge / manual integration) are all UNNECESSARY — the normal pipeline path
  is open.
- Spelling-ladder kills from the prior s1 (rejected/s1-absolute-fold-19.c,
  s1-ptr-local-plus1-folds-25.c, s1-volatile-stripped-inert.c) remain banked and valid
  as measured comparisons; only the artifact interpretation of the final form is
  superseded.
- self_vet.md written this session (all six tests PASS; no FAKE constructs, no
  sanctioned-family claims — every construct mirrors Judge-PASSed sibling
  func_8006156C's committed body).
