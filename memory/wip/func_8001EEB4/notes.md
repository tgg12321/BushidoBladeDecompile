# func_8001EEB4 WIP — score 6 → 3 via [[hoist-call-arg-local-flips-jal-delay]]

## TL;DR

`cpu_get_move_pattern_table_number` (kengo:HIGH). Entry checker that
gates a 3-call sequence (`func_800218C8` → `func_80021A3C` →
`func_80021A98`) on 4 conditions reading `entry+0x6A/0x72/0x96`.

HEAD floor: sandbox `--disable all` = 6. Candidate floor: 3.

## How to resume

1. Apply `candidate.c` to `src/code6cac.c` line 1901–1920 (function body).
2. Confirm: `& tools/eng.ps1 sandbox func_8001EEB4 --disable all` reports
   score 3, build_insns 58, target 59.
3. Continue from there. Three diffs remain in the comparison block (see
   `meta.json#remaining_gap`).

## What this session did

Applied [[hoist-call-arg-local-flips-jal-delay]] to the second call
(`func_80021A98`). Adding `s32 idx2 = D_800A3748;` as the FIRST stmt
in the inner block before `*(entry+0x5E)=1` gives `idx2`'s RTL pseudo
a low LUID, scheduling its `lb $a0,...` arg-setup EARLIER. cc1's
`reorg` then picks the store `sh $s0, 94($s1)` as the natural delay-
slot fill — exactly target's pattern. This retires the
`reorder 36,38,40,39,37,41 @ 36-41` regfix rule and 3 score points.

Critical detail: use **`s32`** for `idx2`, not `s8`. Despite
`D_800A3748` being `s8`, declaring `idx2` as `s8` triggers
`lbu + sll 0x18 + sra 0x18` expansion (+2 insns regression).
`s32 idx2 = D_800A3748;` keeps it as a single `lb` instruction.

## What this session DID NOT solve

The comparison block at maspsx idx 14-22:

```
target:                          build:
lhu  $a1, 0x6A($s1)              lhu  $v1, 0x6A($s1)
li   $v0, 0xA                    li   $v0, 0xA
andi $v1, $a1, 0xFFFF    <---    [no andi]
beq  $v1, $v0, .L                beq  $v1, $v0, .L
nop                              nop
lh   $v0, 0x72($s1)              lh   $v0, 0x72($s1)
nop                              nop
bnez $v0, .L                     bnez $v0, .L
addiu $v0, $a1, -23              addiu $v0, $v1, -23
```

3 diffs: lhu reg, missing andi, addiu reg. The andi is REDUNDANT
semantically (lhu already zero-extends to 32 bits) but target emits
it. cc1's `combine` pass folds the redundant mask in build. Forms
tested for forcing the andi all collapsed (see
`meta.json#rejected_forms`).

## Sibling functions with the same pattern

Same `lhu/andi/beq` target shape appears in:
- `func_8001A820` — INCOMPLETE, body empty in src (no source to
  diff). 1 rule, distance 574 (ASM-PARTIAL).
- `func_8001F938` — INCOMPLETE, distance 11, 13 rules. Source has
  `u16 kind = *((u16 *)(arg0 + 0x6A));` and `if (kind == 0x11 ||
  kind == 0xF || ...)` — same lhu-then-equality-on-low-16 shape.
- `calc_loc_mat_fw_80055B60` — ASM-STRUCTURAL park bucket.

None has solved the andi materialization yet. The pattern is a
recurring unsolved issue in this codebase.

## Why this is parked (not COMPLETED-C)

The remaining 3 diffs cluster around forcing GCC to materialize a
redundant `andi $v1, $a1, 0xFFFF` from C source. Per
[[no-new-park-categories]] cheats-by-any-spelling, using an opaque
mask variable (`u32 mask = 0xFFFF; if ((a1 & mask) != 0xA ...)`)
purely to defeat combine's fold is a borderline cheat (no semantic
purpose to the mask on an already-u16 value). Other pure-C levers
tested in `rejected_forms` all left score at 3.

Resume avenues in `meta.json#next_hypotheses` — including the
instrumented-cc1 combine.c probe approach per
[[difficult-is-not-impossible]].

## Related rules
- [[hoist-call-arg-local-flips-jal-delay]] — the applied lever
- [[register-alloc-pure-c]] — RA-via-C-structure playbook
- [[difficult-is-not-impossible]] — keep grinding (the andi gap is
  a coupling, not a wall)
- [[no-new-park-categories]] — cheats-by-any-spelling policy that
  ruled out the opaque-mask-var "fix"
