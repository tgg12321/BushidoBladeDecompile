---
name: global-label-drift-sibling-cheat
description: A pure-C retire that changes cc1's global .L label count shifts every later function's label numbers, breaking a sibling's hardcoded-absolute-label regfix rule
paths: ["src/*.c", "regfix.txt", "asmfix.txt"]
---

# A pure-C retire that changes cc1's `.L` label count can break a LATER sibling's hardcoded-global-label rule

## Symptom

You take a queue item, find a genuine pure-C structure that makes `sandbox
--disable all` read 0 **and** is byte-identical to the target in isolation
(real-pipeline, rules-dropped, full-register objdump = 0 diffs for *that
function*). But `retire` rolls back with a non-oracle SHA1, and a whole-file
function-by-function object diff shows the sole change is **1 instruction in a
DIFFERENT function later in the same .c file** — a branch/jump whose target
label moved (e.g. `bnez v0,+0x88` → `+0x48`).

## Cause — GCC `.L` labels are numbered globally across the translation unit

GCC 2.7.2 assigns `.L<N>` numbers from a single per-TU counter (`label_num`),
in RTL-generation order, top of file to bottom. If your pure-C rewrite of an
**earlier** function allocates a different number of labels than the committed
source did, **every function after it shifts** by that delta.

A common trigger: the target uses a **dbr delay-slot-threaded** call/branch
(an instruction duplicated into a jump's delay slot, the jump retargeted past
it). The committed source emits the simpler **full-merge** form (one shared
merge label) and a regfix `insert_after` manufactures the thread post-cc1 using
a **named, non-numbered** label (e.g. `.L_csmd2_b:`) — so the committed cc1
label count is unchanged. To get cc1 to emit the thread *itself* you restructure
(e.g. invert the `if` + add a `goto merge;` label), and the threadable basic-block
layout needs **>=1 extra cc1 `.L` label** than the full-merge form. That +N bumps
the global counter.

The break lands on a sibling whose **regfix/asmfix rule hardcodes an ABSOLUTE
global label number**, e.g.:

```
func_LATER: subst "\.L\d+" ".L280-4" @ 29   # forces the @29 branch to target .L280-4
```

After your +N shift, the label that *was* `.L280` is now `.L(280+N)`, so `.L280`
resolves to a different (earlier) label and the forced branch goes to the wrong
address. The rule is only correct for the exact global numbering the committed
source produced — it is a latent landmine for ANY edit to ANY earlier function.

## Diagnosis (decisive)

1. `sandbox --disable all` = 0 but `retire` rolls back (non-oracle SHA1).
2. Whole-file object diff (`mipsel-linux-gnu-objdump -d` both objects, group by
   `<func>:`, index-aligned, normalize branch-target hex): the only diff is a
   single branch target in a **later** sibling, your worked function is clean.
3. Count cc1 `.L` label defs for your function, committed vs your edit:
   `cpp ... | cc1 ... | grep -coE '^\.L[0-9]+:'` and `grep -oE '\.L[0-9]+' | sort -u`.
   A higher **max** label number = the global shift = the delta the sibling drifted.
4. Grep the sibling's rules for a hardcoded `\.L\d+` literal — that's the victim.

## Resolution — a POLICY decision (escalate in headless)

Two ways forward, both touching a **second function**, so they are out of scope
for a single-function Tier-4 pass and should be surfaced to the user:

- **Mechanical drift repair:** bump the sibling's hardcoded label by the delta
  (`.L280-4` → `.L(280+N)-4`). Verify by full SHA1 (the oracle is the only proof
  — do NOT trust the number alone; this is exactly the "auto-repair misdiagnoses
  label drift" trap in [[maspsx-noreorder-stripping]]).
- **De-cheat the sibling:** retire the sibling's hardcoded-label rule to pure C
  (removes the landmine entirely), then your function lands clean.

Until then, **park** your function with the full derivation (the pure-C
structure is ready; only the sibling coupling blocks it).

## Confirmed case — cpu_side_move_dir_2 (code6cac_c2.c, 2026-05-28)

Queue top, verdict C, distance 2, 2 regfix `insert_after` rules manufacturing a
dbr-threaded `func_8005FBC8(a0,0x80118800)` call after `if(D_800A38A4==9)a0=8;
else a0=D`. Pure-C trigger `if(D!=9){a0=D;goto call;}a0=8;call:func(...)` → cc1
emits the thread, `sandbox --disable all` 2→0, real-pipeline rules-dropped
objdump 0 diffs full-register vs target. BUT it allocated 2 extra global labels
(committed max `.L73`/13 labels → `.L75`/14). Sibling `func_8003DBE4` (line 1309,
after) has `subst "\.L\d+" ".L280-4" @ 29`; the +2 shift broke it (retire SHA1
`aa40fdb3...`, sole diff func_8003DBE4 idx31 `bnez v0` +0x88→+0x48). Parked
pending user policy decision on the sibling's brittle absolute-label rule.

## Confirmed case — tslPolyF4Init (system.c, 2026-05-28) — a NEGATIVE delta

Queue top, verdict C, distance 2, 4 regfix rules papering the done dispatch (idx
50 `li v0,-1`→`move v0,zero`; idx 56 `beq s0,v0`→`bnez v0`). Target reuses the
loop's `-1` sentinel (left in `v0` by `count != -1`) as the "exhausted" flag at
the merge and the 3rd-call success sets `v0=0` in the beqz delay slot — i.e. a
`bnez v0` flag test, not a `count == -1` re-test. Pure-C fix: thread a **separate**
`s32 result` (NOT the call-result var, or GCC threads the success edge straight to
the call and collapses the merge → distance 6): success `result = 0; goto done;`,
exhaust `result = -1;`, dispatch `if (result != 0) return 0;`. `sandbox --disable
all` 2→0; full-register objdump of the function vs canonical = **0/81 diffs**.

BUT the matching C consumes **one FEWER** global `.L` label than HEAD (cc1 max
`.L` 232 vs 233; 94 defs both — the function's 5th-emitted label drops 61→57, net
−1 by the next function). Every later system.c sibling shifts −1, breaking three
hardcoded-absolute-`.L` cheats: `marionation_Exec` (`subst ".L128" ".L999" @111/@129`
+ asmfix `beq …,.L128`), `saEft00Add` (`subst ".L199" ".L199+4" @25`, `".L207"
".L207+4" @92` + asmfix `bne/.L199`, `beq/.L207`), and `cpu_side_move_dir_4`'s
labels. Full-build SHA1 `a68874772…` (oracle `62efab…`); sole diffs =
`marionation_Exec` idx133/154 and `saEft00Add` idx26/102 branch targets off by
0x10. Parked pending policy decision on the three siblings (bump their hardcoded
`.L` by −1 + SHA1-verify, or de-cheat them). Note this is the **negative** drift
direction (cf. cpu_side_move_dir_2's +2); same landmine, opposite sign.

## Related
- [[maspsx-noreorder-stripping]] — the other source-change-shifts-a-later-branch
  case; same "fix the label reference, verify by SHA1" discipline, different root
- [[jtbl-rodata-split-infrastructure]] — also "a sibling's index/label-anchored
  rule breaks when output shifts"; there it's the sandbox that can't score
- [[lost-codegen-insert-cheat]] — hardcoded-label/instruction regfix rules are
  the brittle cheats that make this collision possible
