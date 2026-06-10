# InitHiraRmd_80041AC8 (text1a.c) — WIP, floor 6 → 2

## TL;DR

HEAD: 2 regfix reorder rules, honest distance 6, two independent diff clusters.
This session closed cluster 2 cleanly (reviewer PASS) and **fully characterized**
cluster 1 down to the exact compiler mechanism — but both byte-perfect closing
forms for cluster 1 FAILED adversarial cheat review. Candidate floor 2.

## Resume instructions

1. Apply `candidate.c` to `src/text1a.c` (replace the function body).
2. `& tools/eng.ps1 sandbox InitHiraRmd_80041AC8 --disable all` → expect **2**.
3. Read `meta.json.rejected_forms` — do NOT re-derive the two FAIL forms or the
   three measured-negative probes.
4. The remaining work is cluster 1 only (see below). Start from
   `meta.json.next_hypotheses` — the first is a user policy question, the
   third is the most promising mechanical avenue (array-typed store).

## Cluster 2 — CLOSED by the candidate (reviewer PASS)

Target preheader order `li s5,16; li s4,1; la s1,D_800A9A24`; ours emitted la
first because the rect width/height constants written inline in the loop body
get hoisted by loop.c `move_movables` to right before NOTE_INSN_LOOP_BEG —
i.e. AFTER the explicit pointer init. Naming them as locals (`s32 w = 0x10;
s32 h = 1;`) before `var_s1 = &D_800A9A24;` makes materialization follow
source order. Named-intermediate declaration order family (SOTN-accepted).
Retires `reorder 44,45,43 @ 43-45`.

## Cluster 1 — OPEN (the floor-2 residual, maspsx idx 21-23)

Target:
```
lui at; sh a1,%lo(D_800A9A20)(at)   ; store FIRST
lh  v0,8(a0)                         ; genuine post-store reload of arg0[4]
```
Ours: lh first (sched1 hoist), covered at HEAD by `reorder 22,21 @ 21-22`.

### The mechanism (verified in RTL dumps + compiler source, this session)

- The reload is REAL: the store `D_800A9A20 = arg0[4]` may alias `arg0` (s16*
  vs s16 global), so cse keeps the re-read. Value-cached forms delete it
  (72 insns vs 75) — measured, see rejected_forms.
- `arg0[4]` (pointer indexing → INDIRECT_REF of PLUS_EXPR) is flagged
  `MEM_IN_STRUCT_P=1` by expr.c (~line 4570): *"If address was computed by
  addition, mark this as an element of an aggregate."* Verified in the .combine
  dump: insn 51 is `(mem/s:HI (plus (reg 72) (8)))`; the store insn 46 mem has
  no /s.
- sched.c `true_dependence` (~line 816) escape clause: in-struct +
  varying-address + non-QImode load vs non-struct + fixed-address store ⇒
  treated as NON-conflicting ⇒ no dependence ⇒ sched1 hoists the lh by chain
  priority (lh→lbu→sll→lw depth ≫ sh). Verified: order flips between the
  .combine and .sched dumps.
- Therefore the TARGET's order is only producible when the reload has /s=0,
  which in this compiler requires the INDIRECT_REF operand to not be a bare
  PLUS_EXPR — i.e. a plain pointer-var deref or a NOP_EXPR(cast)-wrapped
  address. **The original 1998 source provably did not spell this read as
  plain pointer-indexing.**
- Struct-field access (COMPONENT_REF) also gets /s=1 — a struct-typed packet
  reconstruction does NOT fix this.
- Both /s=0 spellings tried reach sandbox 0 / 75 insns / 0 full-register
  diffs / oracle SHA1 (retire passed before revert), and both FAILED the
  cheat-reviewer as same-bytes alias-analysis coercion (see rejected/).

### The open question (for user/orchestrator)

Reviewer's line: a respelling that emits identical bytes and is justified
only by its effect on GCC's analysis is the forbidden volatile-coercion
family; the sanctioned byte-cast rules all produce *different* bytes.
Worker's line: the /s=0 requirement is mechanical evidence about what the
original source did, making a pointer/cast spelling the source-faithful
reconstruction (and the regfix rule it replaces is pure paperwork).
This is a policy call neither agent can settle — recorded as
next_hypotheses[0].

### Tooling note

`tmp/ihr_dumps.sh` regenerates the -da RTL dumps; `tmp/ihr_order.py` tracks
the sh/lh order across passes; `tmp/dump_ihr.sh` is the full-register
objdump diff vs canonical build. All reusable on resume.

### Incident note (resolved)

A buggy first version of `tmp/ihr_cc1.sh` (Makefile CPP_DEFS extraction left
a trailing `\` arg) made cpp treat `src/text1a.c` as its OUTPUT file and
deleted it. Restored byte-identical via `git checkout -- src/text1a.c`
(verified 0 CR bytes, clean diff). The committed script version is fixed
(hardcoded defs).
