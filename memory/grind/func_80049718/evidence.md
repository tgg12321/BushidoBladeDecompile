# Evidence bank — func_80049718

## s1 (recon) — sandbox 11 -> 0

- Baseline this session: candidate floor 6 confirmed (verdict C, 197/197 insns,
  2 regfix rules stripped inert). Removed do{}while(0)+if(0){} already applied.
- Precise byte diff (asm/funcs vs sandbox .o, relocation-normalized): only TWO
  real residual clusters at floor 6; every other objdump diff is an unrelocated
  %hi/%lo placeholder (lui/addiu/lw/sw/lh with symbol shown as 0x0).
- Cluster B (store schedule): block-2 `if(var_s3!=1)` — target emits
  `sh zero,8; sh zero,0xA; sh v0,4(=6)`; our fork emitted `sh v0,4` first.
  FIX: reorder the 4 independent stores so obj+4=6 follows obj+8/obj+0xA.
  Measured 6 -> 4. Pure statement reorder, behavior-identical.
- Cluster A (prologue `&D_800EF980[arg0]`): our fork's sched1 hoists the 1-insn
  `sll` (index) above the 2-insn base chain -> index=$v0, base=$v1,
  `addu s0,v0,v1`. Target computes base first (base=$v0, index=$v1,
  `addu s0,v1,v0`) — critical-path, since s0 feeds the very next `lh`.
  FIX: stage base into named local `s16 *tbl = D_800EF980; p_anim = tbl + arg0;`
  -> base pseudo born first. Measured 4 -> 0 (byte-match modulo relocation).
- REJECTED: bare `p_anim = D_800EF980 + arg0;` (no named local) = floor 11.
  The named intermediate is the lever (SOTN named-intermediate family).
- artifact: tmp/grind/func_80049718/s1/mine_objdump.txt + normdiff.py (diff tool).


- WIP rejected_form: {'form': 'p_anim = D_800EF980 + arg0;  (pointer-add spelling)', 'score': 11, 'reason': 'WORSE than &D_800EF980[arg0] (floor 6). Keep the array-index spelling.'}

- WIP rejected_form: {'form': 'p_anim = (s16*)((s32)&D_800EF980[0] + arg0*2);', 'score': 6, 'reason': "No change (still 6); the addu operand order didn't flip. Not worth re-deriving."}

- == imported from memory/wip notes.md ==
# func_80049718 (text1b.c) — WIP, floor 6 (HEAD 11), BLOCKED

## TL;DR (2026-06-14)
HEAD carries 2 regfix reorder rules (16-insn prologue @3-18 + 3-insn store
@167-169), honest floor 11. **Removing two semantically-empty constructs — a
`do { ... } while (0)` wrapper and an empty `if (0) { }` — drops the floor to
6** (they were cheat-by-spelling scheduling perturbers). The candidate (clean,
0 cheat-asm) is saved. Residual 6 is an operand-order + store-scheduling
coupling; not byte-identical (retire SHA1 mismatch 3fd1338c).

## Residual gap (floor 6)
1. `p_anim = &D_800EF980[arg0]`: target computes lui/addiu (BASE) then
   sll (index), `addu s0,v1(index),v0(base)`. Mine does sll first → index in
   $v0, base in $v1 (operands swapped). Need base computed first.
2. One `sh v0,4(s2)` store reordered (the 2nd regfix `reorder 168,169,167`).

## Resume steps
1. Paste candidate.c; confirm sandbox 6.
2. Work the s0 operand order (base-first) and the sh store position. Verify via
   retire (full SHA1) — the operand swap may be masked, so trust retire.
3. Likely modality: decomp-permuter from the floor-6 base.

## Ruled out (do not re-derive)
- `p_anim = D_800EF980 + arg0;` → floor 11 (worse). Keep `&D_800EF980[arg0]`.
- `(s16*)((s32)&D_800EF980[0] + arg0*2)` → no change (6).

## Finding worth noting
The `do{...}while(0)` + empty `if(0){}` here were INFLATING the floor (11→6
when removed) — a reminder that these no-op constructs are not free; they can
perturb sched1. Worth a cheat-by-spelling detector note (empty-if already has a
detector per the 2026-06-02 audit; the do-while(0)-around-real-statements
wrapper is the perturber here).

## Pointers
- `.claude/rules/prologue-fix-redundant-reorder.md`, `compare-operand-order-register.md`
- `.claude/rules/register-alloc-pure-c.md`


- [s1] Baseline floor 6 confirmed this session (verdict C, 197/197 insns, 2 regfix rules stripped inert).

- [s1] Only two real residual clusters at floor 6; every other objdump diff is an unrelocated %hi/%lo symbol placeholder.

- [s1] Cluster B fix (block-2 store reorder obj+4=6 after obj+8/obj+0xA): floor 6 -> 4; layer-1 cheat-reviewer PASS.

- [s1] Cluster A RTL mechanism: fold canonicalizes address PLUS with symbol_ref as operand-2 -> index (ashift) insn emitted before base (symbol_ref) -> local-alloc index->$v0/base->$v1 -> `addu s0,v0,v1` (target wants the reverse). text1b.i.greg:7783-7800.

- [s1] Named base-pointer alias closes Cluster A to sandbox 0 (byte-match modulo relocation) but is cheat-reviewer FAIL (pointer-alias family; missing exhaustion ledger + /* FAKE */). Saved to memory/grind/func_80049718/candidate_gated_alias.c; src reverted to clean floor 4.

- [s1] REJECTED clean-C alternatives: bare `D_800EF980 + arg0` = floor 11 (fold differs, worse); `(s16*)((s32)&D_800EF980[0]+arg0*2)` = no change at 6 (prior); prologue statement swap (var_s3<->p_anim) = no change at 4.

- [s1] src left at clean floor 4 (changes 1+2 only).

## s2 (structural) — clean pure-C exhaustion of Cluster A; base-pointer local closes to 0

- [s2] Fresh greg (floor-4 src) reconfirms Cluster A: insn18 index ashift -> $v0
  (born first), insn20 base symbol_ref -> $v1, `addu s0,v0,v1`. FINAL asm
  coalesces base directly into $s0 (lui $s0; addiu $s0; addu $s0,$v0,$s0) and
  hoists the addr comp early; target keeps base in own temp $v0, placed late
  (lui $v0; addiu $v0; sll $v1,$s6,1; addu $s0,$v1,$v0). The v0/v1 swap +
  in-place-$s0 coalesce + early hoist together = the 4-point residual.
- [s2] CLEAN structural respellings (no base-holding pseudo) — ALL measured dead:
    (s16*)((s32)D_800EF980 + arg0*2)               -> 4 (fold-identical)
    { s32 idx=arg0*2; (s16*)((s32)D_800EF980+idx) } -> 4 (index precompute)
    p_anim = D_800EF980; p_anim += arg0;            -> 9 (WORSE; cse2 re-folds the reused var)
    [s1] p_anim = D_800EF980 + arg0                 -> 11
    [s1] (s16*)((s32)&D_800EF980[0]+arg0*2)         -> no change
- [s2] BOTH base-holding-pseudo forms reach floor 0 (re-confirmed this session):
    { s16 *tbl = D_800EF980; p_anim = tbl + arg0; }              -> 0
    { s32 base=(s32)D_800EF980; p_anim=(s16*)(base+arg0*2); }    -> 0
  Same construct = the base-pointer local (pointer-alias family). No clean
  non-alias form exists: flipping the addu operand order REQUIRES the base in
  its own live pseudo, which is precisely the alias construct.
- [s2] PRECEDENT CONFLICT surfaced: register-alloc-pure-c "Confirmed CLOSURES"
  documents the IDENTICAL lever `s32 *base = arr; elem = base + idx;` for
  tslPolyF4Init as "Ordinary pointer arithmetic" -> COMPLETED-C (NO FAKE). But
  s1's layer-1 cheat-reviewer FAILed the identical `tbl` here as pointer-alias-
  fake-exception (needs /* FAKE */ + exhaustion). Exhaustion is now complete;
  emitted ruling-request for owner classification.
- [s2] No prior owner ruling/escalation for func_80049718 in docs/grind/decisions.md.
- artifacts: tmp/grind/func_80049718/s2/text1b.i.greg (+ .sched/.jump2/etc),
  tmp/grind/func_80049718/s2/greg.sh, diff.py.
