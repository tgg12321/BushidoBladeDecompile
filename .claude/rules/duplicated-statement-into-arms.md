---
name: duplicated-statement-into-arms
paths: [".claude/rules/duplicated-statement-into-arms.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
description: "NARROW SANCTIONED EXCEPTION (owner ruling 2026-07-01): duplicating a REAL statement into 2+ arms (instead of label-sharing) is legitimate — incl. when cross-jump re-merges the copies to identical bytes and the effect is a reg_n_refs priority lift. SOTN duplicates assignments across arms routinely (7-arm, 11-arm instances). Prerequisites: byte-neutrality verified, lever-exhaustion, FAKE annotation when match-motivated."
metadata:
  type: rule
---

# Duplicated statement into arms — the sanctioned spelling

**Owner ruling 2026-07-01** (issued after a layer-2 cheat-reviewer FAIL
was appealed with fresh direct evidence). Writing the SAME real
statement in two or more control-flow arms — instead of sharing one
copy via a label/goto — is a legitimate matching technique,
**including** when:

- GCC's jump2 cross-jump re-merges the copies so the final bytes are
  identical to the shared-label form, and
- the duplication's surviving effect is the extra `reg_n_refs` count
  flow.c records (allocno-priority lift for global RA), and
- the label placement among the copies is chosen to steer the merge
  DIRECTION (which copy survives inline vs becomes the jump).

## Why (the evidence)

- **Cross-arm duplication is routine SOTN committed style** (matched
  PSX-era code, capped scan found 30+ instances immediately):
  `DOPPLEGANGER.animSet = ANIMSET_OVL(1)` identical across **7 arms**
  and `D_us_801D3D24 = THINK_STEP_0` across **11 arms**
  (`src/boss/bo4/doppleganger.c`); identical multi-statement blocks
  across arms (`src/boss/bo4/doors.c`, `unk_365FC.c`). Whether
  cross-jump merges them is invisible to the author; both merged and
  unmerged instances ship.
- **Redundant duplicated stores, match-annotated, in SOTN master:**
  `color_fake = color;` written twice with `color` unchanged between
  (`src/dra/42398.c`); `j = menu->unk1D; // FAKE?` at three sites
  (`src/dra/menu.c:1993,2009,2017`).
- **MGS** (`source/weapon/socom.c:89-91`): "no reason to assign these
  again but no match if we don't."
- **Reconstruction logic:** the duplicated and label-shared spellings
  compile to IDENTICAL bytes, so the binary cannot distinguish them —
  but when only the duplicated spelling reproduces the target's
  register allocation pin-free, that is evidence the ORIGINAL source
  was the duplicated form.

## Prerequisites (cheat-reviewer FAILs if any is missing)

1. **The statement is REAL on its path** (a genuine def/effect the
   path needs — not a dead store; dead stores are governed by
   [[dead-store-fake-exception]]).
2. **Byte-neutrality verified**: the emitted function is byte-identical
   vs the canonical reference (objdump diff and/or full SHA1) — the
   duplication must NOT materialize extra instructions.
3. **Lever-exhaustion documented** when used as a last-resort RA lever
   (WIP ledger / commit body).
4. **`/* FAKE: <reason> */` annotation** on the duplicated copy when
   the duplication is match-motivated (SOTN's convention).
5. **Layer-1 + layer-2 review** per [[review-discipline-before-commit]].

## Confirmed closures

- **motion_SetMotion** (code6cac_c_mid.c, 2026-07-01): load_sel2's
  `sel2 = D_800A3350;` duplicated into case 0, label moved to case
  13/17's copy → sel2 reg_n_refs 3→4 (pri 851 > result's 412) → RA
  lands sel2→$s2 / result→$s3 (target) with BOTH pins retired;
  cross-jump re-merges the tails (function byte-diff empty; full SHA1
  == oracle). Label placement was load-bearing: the mirror labeling
  emitted a 4-diff flipped layout. Also measured: dead stores are
  INERT for global RA in 2.7.2 (flow deletes without counting) — real
  duplication is the working spelling of the ref-lift.

## Non-extension

Does NOT sanction: dead stores (see [[dead-store-fake-exception]]),
duplicating CALLS (changes bytes), or any duplication that survives to
the final bytes (fails prerequisite 2 — that's a real code change).
Per [[no-new-park-categories]], other spellings need their own evidence.

## Related

- [[sotn-family-research-2026-07-01]] — the evidence framework.
- [[split-read-defeats-hoist]] — the sibling READ-duplication family.
- [[register-alloc-pure-c]] — the RA-priority context; this is the
  proven byte-free ref-lift for global-alloc walls.
