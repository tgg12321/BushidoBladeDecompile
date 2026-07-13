# Hypothesis ledger — se_data_set

Two constructs were flagged by the 2026-06-22 regression audit. #1 is CLOSED CLEAN.
#2 is measured NECESSARY and now shown PRECEDENTED; only the permuter gate remains.

---

## H1 — the lhu closes with a cast-free spelling  [CONFIRMED, s2]

**Statement.** `D_800A390E = (s8)(u16)D_800A36A4;` (flagged construct #1) can be replaced by
plain `D_800A390E = D_800A36A4;` with no codegen cost.

**Mechanism.** mips.md `movhi`: a plain HImode load is `lhu`. `lh` is the fused
`extendhisi2` form, chosen only when the consumer needs SImode sign extension. An `sb`
consumer truncates, so it never asks for one. The double cast was m2c transcription of
`lhu`+`sb`, not a lever.

**Probe.** Sweep variant v00 (`tmp/grind/se_data_set/s2/variants/v00_candidate.c`).

**Result.** sandbox `--disable all` = **0**, and the sandbox `.o` is **register-exact and
reloc-exact identical to the HEAD build** (`diff -u head.txt v00_candidate.txt` → no
differences; HEAD is byte-verified on main). Not a masked 0.

**Verdict: CONFIRMED.** Flagged construct #1 is gone at zero cost. Do not re-derive.

---

## H2 — the $s0 address cache falls out of block-local CSE of two identical reads  [KILLED, s1; re-measured s2]

**Statement.** Writing `*(&D_8008E5A8 + (s8)D_8010277C)` directly at both body sites lets
cse.c unify the two identical lb-address computations into a call-surviving pseudo,
reproducing target's `$s0` cache with no pointer at all.

**Probe.** Sweep variant v01 (direct cast at both sites).

**Result.** **6** (93/93 insns). Each body site emits its own `lui %hi(D_8010277C)` +
`lb %lo(...)`; target emits `lui/addiu $s0` once + `lb 0($s0)` twice
(`diff -u head.txt v01_direct_cast.txt`, tmp/grind/se_data_set/s2/). The insn count is
93 either way — the 2-insn `la` is exactly paid back by the 2 saved `lui`s — so the 6 is
*purely* the address-materialization shape, nothing else.

**Verdict: KILLED.** cse never INSERTS insns; a non-PIC SYMBOL_REF is already a legitimate
MIPS address, so there is no separate address expression for it to unify.

---

## H3 — `la $sN, sym` comes from a C-level `&` bound to a variable  [REFUTED as stated, s2 — corrected below]

**Statement (s1's mechanism story).** GCC emits `la` only for a movsi of a SYMBOL_REF into
a pseudo, i.e. from an `&` bound to a variable; therefore an `&` is necessary and
sufficient.

**Probe.** v07: bind `&` to a variable for the *wrong* symbol — `u8 *t = &D_8008E5A8;`
(the lookup table), used as `t[i]`. The table address is live across `func_8005BA8C`, so
under H3-as-stated it must be materialized into a callee-save register.

**Result.** **16**, and the disassembly (`tmp/grind/se_data_set/s2/v07_table_pointer.txt`)
shows **no `la` at all**: all four table reads still emit
`lui $at,%hi(D_8008E5A8); addu $at,$at,$idx; lbu %lo(D_8008E5A8)($at)`. The pointer local
was constant-folded out of existence. The 16 is knock-on RA churn.

**Verdict: REFUTED as stated.** The `&` is not sufficient. **Corrected rule (the durable
finding):**

- A pointer local dereferenced as a **plain scalar** (`*p` = `MEM(reg)`) **survives** as an
  address pseudo → `la $sN, sym` + `lb 0($sN)`. `MEM(reg)` is a 1-insn address; folding the
  SYMBOL_REF back in would cost 2 (`lui`+`lb`), so GCC keeps the register — and across a
  call, that register is callee-save.
- A pointer local used as an **indexed base** (`t[i]` = `MEM(plus(reg,reg))`) is **folded
  back** to `MEM(plus(symbol,reg))`: MIPS addresses symbol+index natively
  (`lui`/`addu`/`lbu %lo`), the pointer buys nothing, and constant-propagation eats it.

This rule *explains the target bytes*: `&D_8010277C` is read as a plain scalar → cached in
`$s0`; `D_8008E5A8` is an indexed table base → re-`lui`'d at all four reads. The asymmetry
in the shipped 1998 bytes is the **fingerprint of this codegen rule**, not of a cheat.

---

## H4 — the condition/body asymmetry is an artifact of our spelling ("the tell")  [KILLED, s1+s2]

**Statement (the 2026-06-22 audit's core objection).** The body uses a pointer while the
condition uses a direct cast; a consistent spelling would suffice, so the inconsistency
betrays a coercion construct.

**Probes + results.**
| form | score |
|---|---|
| pointer used in the condition too (uniform aliasing) | **8** (92 insns — one FEWER than target) [s1] |
| pointer hoisted to function scope, body-only use | **6** [s1] |
| alias BOTH index bytes (`p`=&D_8010277C, `q`=&D_8010277D) | **6** [s2] |
| alias neither (direct cast everywhere) | **6** [s2] |

**Verdict: KILLED.** Target reads `D_8010277C` symbol-direct in the condition and caches it
only inside the block, and caches `D_8010277C` but *not* the adjacent `D_8010277D`. Both
asymmetries are **facts about the target bytes**. Every "consistent" spelling — alias all,
alias none, alias uniformly — is measurably NOT the original. **Consistency is therefore not
available as a completion bar for this function.** The declaration's block-local *scope* is
load-bearing too, not just its use sites (function-scope decl → 6).

Corollary (kills the array-decay idea for the pair): had the original reached both bytes
through one base pointer (`u8 arr[2]`), the second read would be `lb 1($s0)`. Target emits
`lui %hi(D_8010277D)` instead — so the two bytes were not reached through a common base.

---

## H5 — the &-free subspace contains a zero  [KILLED, s2 — subspace exhausted at the structural level]

**Probes.** Six measured forms (s2 sweep + s1 bank), all `&`-free or address-cache-free:

| form | score | note |
|---|---|---|
| direct cast at both sites | 6 | subspace floor |
| subscript spelling `(&X)[i]` | 6 | spelling-invariance control — same RTL, ties exactly |
| value-staging locals, re-read after the call | 9 | [s1] |
| table pointer instead of index pointer | 16 | mechanism probe (H3) |
| store re-association | 17 | the 6 is not a scheduling artifact |
| call args hoisted into block-local temps | 24 | 95 insns vs target 93 — temps spill across the 4 calls |

**Verdict: KILLED.** The `&`-free subspace floor is **6** and the missing 6 instructions are
exactly the address-materialization shape. Per the corrected H3 rule, no expression-level
form can produce an address pseudo — a pointer local dereferenced as a plain scalar is the
only C construct that does. **Structural modality is exhausted for this function.**

---

## H6 — the pointer alias is unprecedented in accepted code  [KILLED, s2 — matched-corpus check DONE]

This was the Judge's explicitly-required corpus check. Result: **the construct is
established in ACCEPTED COMPLETED-C code, including for this very global.**

- `src/code6cac_c_ab.c:431` **func_8003B2C8** — `u8 *p = &D_8010277C;` — not in queue ⇒ COMPLETED-C
- `src/code6cac_c_ab.c:444` **func_8003B328** — `u8 *p = &D_8010277C;` — not in queue ⇒ COMPLETED-C
- `src/text1b.c:14780` **func_800656EC** — `u16 *s0 = &D_800F0BC2;` — not in queue ⇒ COMPLETED-C;
  `asm/funcs/func_800656EC.s` shows `lui $s0,%hi` + `addiu $s0` + loads through `0($s0)`
  spanning two `jal`s — **our exact disputed shape, in accepted code**.
- 68 pointer-to-global locals in `src/`, 40 of them in COMPLETED-C functions. 16 COMPLETED-C
  functions exhibit the callee-save-global-address-across-a-call shape.
- For a **scalar** global, every accepted spelling of that shape in this tree is a pointer
  local (with `&`, or `&`-free array decay after re-declaring the global `extern T SYM[]`).
  The only pointer-free accepted cases are struct-object member access, structurally
  inapplicable to a lone `u8` byte.
- (Checked and DISCARDED as precedent: `saEft00Add` — it IS in the queue, so INCOMPLETE.)

**Verdict: KILLED** (the hypothesis that it is unprecedented). The construct is normal,
accepted, matched C in this codebase.

---

## Frontier — ONE gate item left

The Judge's constraint listed three prerequisites before the pointer form may be routed to
layer-2 review. Two are now discharged:

- [x] search the `&`-free subspace, not merely hand-sample it — **done, H5** (6 forms, floor 6)
- [x] matched-corpus check for a COMPLETED-C function caching a global's address in a
      callee-save reg across a call — **done, H6** (16 of them; 2 use this exact global)
- [ ] **directed permuter campaign seeded on the score-6 direct-cast body**, under the
      fresh-seed stopping rule ([[permuter-fresh-seed-discipline]], ~20-30 fresh seeds)

That permuter run is the last outstanding item and needs a **permuter-modality** session.
Seed it with `memory/grind/se_data_set/rejected/direct-cast-no-address-cache-scores-6.c`
(score 6, 93/93 insns). Prior expectation from H3/H5: it should find nothing, because the
only C construct that yields an address pseudo is a plain-scalar-dereferenced pointer local
and the permuter does not invent pointer aliases. A negative there completes the
lever-exhaustion record and the pointer form can be routed with the H6 precedent attached.

## [s3] The lhu closes with a cast-free spelling: plain `D_800A390E = D_800A36A4;` replaces `(s8)(u16)D_800A36A4` at no codegen cost (flagged construct #1)
- mechanism: mips.md movhi: a plain HImode load IS lhu. lh is the fused extendhisi2 form, chosen only when the consumer needs SImode sign extension; an sb consumer truncates and never asks for one. The double cast was m2c's transcription of lhu+sb, not a lever.
- probe: Sweep variant v00 (cast-free store + block-local u8 pointer), then objdump the sandbox .o and diff it against the HEAD build's disassembly (HEAD is byte-verified on main).
- result: sandbox --disable all = 0, and `diff -u head.txt v00_candidate.txt` reports NO differences — register-exact and reloc-exact, not a masked 0. Construct #1 is gone at zero cost.
- verdict: CONFIRMED

## [s3] `la $sN, sym` is emitted whenever a C-level `&` is bound to a variable (the mechanism story banked by session 1 to explain why the pointer is needed)
- mechanism: s1 claimed GCC emits la only for a movsi of a SYMBOL_REF into a pseudo, i.e. from an `&` bound to a variable — implying the `&` is both necessary and sufficient.
- probe: v07: bind `&` to a variable for the WRONG symbol — `u8 *t = &D_8008E5A8;` (the lookup table), used as `t[i]`. The table address is live across func_8005BA8C, so under the story-as-stated it MUST be materialized into a callee-save register. Disassemble and look for the la.
- result: REFUTED as stated. Score 16, and the disassembly shows NO la at all: all four table reads still emit lui $at,%hi + addu + lbu %lo — the pointer local was constant-folded out of existence. CORRECTED RULE (the durable finding): a pointer local dereferenced as a PLAIN SCALAR (*p = MEM(reg)) survives as an address pseudo, because MEM(reg) is a 1-insn address and folding the SYMBOL_REF back in would COST 2 (lui+lb); but a pointer used as an INDEXED BASE (t[i] = MEM(plus(reg,reg))) is folded back to MEM(plus(symbol,reg)), because MIPS addresses symbol+index natively and the pointer buys nothing. This rule EXPLAINS the target bytes: D_8010277C is a plain scalar read (cached in $s0) while D_8008E5A8 is an indexed table base (re-lui'd at all four reads).
- verdict: KILLED

## [s3] The &-free subspace contains a zero — some pure-C form without a pointer alias reproduces target's $s0 address cache
- mechanism: If cse.c could unify the two identical lb-address computations in the body's single basic block (calls do not end BBs), the direct spelling would produce a call-surviving address pseudo with no pointer at all.
- probe: Six measured forms (s2 sweep + s1 bank), all &-free or address-cache-free: direct cast at both sites; subscript spelling (&X)[i] (spelling-invariance control); value-staging locals re-read after the call; table pointer instead of index pointer; store re-association; call args hoisted into block-local temps.
- result: KILLED. Scores 6 / 6 / 9 / 16 / 17 / 24 respectively — subspace floor is 6 and nothing reaches 0. The 6 is PURELY the address-materialization shape: insn count is 93 either way (target's 2-insn la is exactly paid back by the 2 saved luis), and perturbing the store order makes it strictly worse (17), so the gap is not a scheduling artifact. cse never INSERTS insns, and a non-PIC SYMBOL_REF is already a legitimate MIPS address, so there is no separate address expression to unify. Structural modality is exhausted for this function.
- verdict: KILLED

## [s3] The condition/body asymmetry is an artifact of our spelling — 'inconsistency between condition (direct cast) and body (pointer) is the tell' (the 2026-06-22 audit's CORE objection)
- mechanism: The audit reasoned that a consistent spelling would suffice, so the inconsistency betrays a codegen-coercion construct with no semantic purpose.
- probe: Measure every 'consistent' spelling: alias BOTH index bytes (p=&D_8010277C, q=&D_8010277D); alias NEITHER (direct cast everywhere); use the pointer in the condition too (uniform aliasing); hoist the declaration to function scope.
- result: KILLED. alias-both = 6, alias-neither = 6, pointer-in-condition-too = 8 (92 insns — one FEWER than target), function-scope decl = 6. Target reads D_8010277C symbol-direct in the condition and caches it ONLY inside the block, and caches D_8010277C but NOT the adjacent D_8010277D. Both asymmetries are FACTS ABOUT THE SHIPPED 1998 BYTES, not our spelling. Every consistent form is measurably NOT the original, so consistency is not available as a completion bar here. The declaration's block-local SCOPE is itself load-bearing. (Corollary: also kills array-decay for the pair — had the original reached both bytes through one base, the second read would be `lb 1($s0)`; target emits lui %hi(D_8010277D) instead.)
- verdict: KILLED

## [s3] The pointer-to-global alias is unprecedented in accepted code (the Judge's explicitly-required matched-corpus check)
- mechanism: If no COMPLETED-C function reaches the callee-save-global-address-across-a-call shape, the construct is novel and the burden on it is high; if accepted matched code already does exactly this, it is normal C for this compiler and codebase.
- probe: Grep src/ for pointer locals bound to a global's address; cross-check each enclosing function against engine/queue.json (queue presence = INCOMPLETE, absence = COMPLETED-C); then confirm the asm shape (lui $sN,%hi + addiu $sN,%lo + loads through 0($sN) spanning a jal) in asm/funcs/. Verified the load-bearing hits by hand rather than trusting the sweep.
- result: KILLED (it IS precedented) — POSITIVE result. 68 pointer-to-global locals in src/, 40 of them in COMPLETED-C functions; 16 COMPLETED-C functions exhibit the disputed asm shape. Decisively: src/code6cac_c_ab.c:431 func_8003B2C8 and :444 func_8003B328 both spell it `u8 *p = &D_8010277C;` — the SAME construct on the SAME global — and neither is in the queue, i.e. both are accepted COMPLETED-C. src/text1b.c:14780 func_800656EC (`u16 *s0 = &D_800F0BC2;`) reproduces the exact shape: lui $s0,%hi + addiu $s0 + loads through 0($s0) across two jal calls. For a SCALAR global, every accepted spelling of this shape in the tree is a pointer local (the only pointer-free accepted cases are struct-object member access, structurally inapplicable to a lone u8 byte). saEft00Add was checked and DISCARDED as precedent — it is still in the queue.
- verdict: KILLED
