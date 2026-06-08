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

## Session-2 (2026-06-07, HEAD 0d960614) — andi materialization lever found

`s32 a1_s = (s16)a1;` declared as a separate top-level local AND referenced in
the `(u32)(a1_s - 0x17) >= 2` arm of the && chain DOES trigger target's andi
emission. Confirmed via objdump:

- ✓ `andi $v1, $a1, 0xFFFF` materializes at idx 18 (matching target's
  exact position and operands)
- ✓ `addiu $v0, $a1, -23` in bnez delay slot uses `$a1` (matches target;
  was `$v1` before the lever)
- ✓ `lhu` lands in `$a1` register (matches target; was `$v1` before)
- ✓ build_insns 58 -> **59** = target's exact count

Two coupled regressions cost net +6 score (3 -> 9):

- ✗ load at idx 16 is `lh $a1, 106($s1)` (sign-extend) — target has
  `lhu` (zero-extend). 1 byte opcode diff.
- ✗ stack frame inflates by 8 bytes (addiu sp,sp,-40 vs target -32;
  three saved-reg sw offsets shift +8). The extra local needs a spill
  slot apparently.

Mechanism (hypothesis): the (s16) cast on a u16-loaded value forces GCC
combine to emit a SImode sign-extension RTL pattern. That RTL pattern
includes the `andi 0xFFFF` extraction the target wants. But combine
ALSO folds the sign-extension INTO the load itself (lhu -> lh).
Decoupling the andi-emission from the load-opcode change is the
remaining puzzle.

### Resume target (next session)

PRIMARY: find a C form that gets the (s16)-cast-on-local effect (andi
emission, register pattern) WITHOUT changing lhu -> lh and WITHOUT
inflating the stack. Try block-local declaration; try (s32)(s16)a1
vs (s16)(s32)a1 ordering; try reading a1 separately via different
type pointers in two distinct syntactic positions.

## Session-3 (2026-06-07, HEAD 23e0ff7c) — confirmed reproducible; no new lever

Re-applied candidate.c to src/code6cac.c: sandbox `--disable all` reports
score=3, build_insns=58, target=59 (matches session-1 measurement exactly).

Tested one untried form from session-2's next_hypotheses (ii): inline
`*(s16 *)(entry + 0x6A)` re-read at the 3rd-test position, keeping `u16 a1`
for the cached read used in tests 1 and 2. Result: score 3 but
build_insns 61 (+2 over target). Same +2 sll/sra penalty as the inline
(s16)a1 rejected_form — the s16 pointer dereference at the comparison
position emits sign-extension shifts when not folded into a stored local,
and the load opcode at the inline s16 read is its own `lh` (not the
desired `lhu`). The hoped-for separation between cached u16 load and
comparison-site sign-extension doesn't help.

Other candidate forms considered but NOT measured (vetted out as cheats
or duplicates of rejected forms):
- nested struct/union punning to read same memory as u16 then s16 —
  cheats-by-spelling concern, no semantic purpose
- opaque mask variable `u32 mask = 0xFFFF; (a1 & mask) != 0xA` —
  already documented as cheats-by-spelling violation (see § Why parked)
- DImode chain — forbidden per [[global-label-drift-sibling-cheat]]
- compound (s32)(s16)a1 vs (s16)(s32)a1 cast orderings — equivalent RTL

The 3-diff cluster is structurally coupled: the only known mechanism
that triggers target's andi emission (a separate top-level
`s32 a1_s = (s16)a1;` local) also forces (lh load + 8-byte stack
inflation) which costs more than the 3 diffs it would close.

Floor remains 3. The function stays at this WIP checkpoint awaiting
a fundamentally different angle (e.g. permuter from the candidate-3
base with PERM_GENERAL operand directives; instrumented combine.c
probe per session-2's hypothesis 3; or escalation as a documented
plateau if no further lever surfaces).

## What session-1 DID NOT solve

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
