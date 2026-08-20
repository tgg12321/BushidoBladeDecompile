# Evidence bank — func_800283D0

- == imported from memory/wip notes.md ==
# saTan2KabutoWareMove (src/code6cac_b.c) — WIP checkpoint 2026-08-06

**Honest floor: 44 (unmoved this session — characterised only, not yet ground).**
Wiring: `asmfix.txt` `saTan2KabutoWareMove: replace_with_asmfile "asm/funcs/saTan2KabutoWareMove.s"`.

`volatile_cheat_count = 0` — the draft body is already detector-clean, so unlike
SetPacketData there is no free cheat-removal gradient to harvest here.

## Residual shape (ours 211 insns vs target 215)

**A. Prologue s-register rotation.** Target saves/uses `s2` where we use `s3` and vice
versa, and homes the second argument into a different register:
```
ours   : sw s3,36(sp) | move s3,a1   ... sw s2,32(sp)
target : sw s2,32(sp) | move s2,a1   ... sw s3,36(sp)
```
Same class as SetPacketData's cluster — a save-order/arg-home difference that then
propagates through the body.

**B. Constant lands in a different argument register.** `li a1,4` (ours) vs `li a2,4`
(target), and every downstream compare against it follows (`beq v0,a1` vs `beq v0,a2`).

**C. An explicit u16 widening we do not emit — the most actionable lead.**
```
ours   : lhu v1,106(s0) ...           beq v1,a1,@
target : lhu a1,106(s0) ... andi v1,a1,0xffff | beq v1,a2,@
```
Target keeps the raw `lhu` result in one register AND materialises a separately-masked
`andi ...,0xffff` copy for the compare; we fold the mask away because the `lhu` already
zero-extends. Target is +1 insn here, which is part of the 211-vs-215 count gap.
This is a **type/width** signal, not an allocation one: it says the value was held in
something wider than `u16` at the compare (e.g. read into an `s32`/`int` local, or compared
against a value whose type forces the promotion to be materialised). See
[[header-type-correction-from-use-sites]] and [[u16-global-lhu-lbu-low-byte]].

**D. A folded 0/1 diamond.** Target keeps `beq v1,v0,@ | nop` and a separate `li v0,1`
where we emit `bne v1,v0,@ | move v0,s6` — we reuse an already-live register for the
constant, target rematerialises it.

## Why C is already "correct" and still folds — the real blocker

The source ALREADY expresses target's structure:
```c
temp_a1 = *(u16 *)(arg0 + 0x6A);   /* raw  -> target keeps this in $a1 */
temp_v1 = temp_a1 & 0xFFFF;        /* mask -> target keeps this in $v1 */
...
if (temp_v1 != 4) ...              /* compares use the masked copy */
((u32)(temp_a1 - 0x19) >= 2U)      /* subtract uses the RAW copy    */
```
Target emits `lhu a1,106(s0)` + `andi v1,a1,0xffff` and keeps both live. We emit only the
`lhu` because `temp_a1` is `u16`, so combine proves the mask redundant via `nonzero_bits`
and folds it, collapsing the two values into one register.

**Measured negative (2026-08-06): widening `temp_a1` to `u32` does NOT defeat the fold —
score stayed 44.** The zero-extending `lhu` still tells combine the value fits in 16 bits,
so the declared type of the holder is irrelevant.

## 2026-08-06 session 2 — the `andi` mechanism is LOCALISED (floor still 44)

**The basic-block-boundary lever proposed above is REFUTED — do not run it.** The target's
own layout disproves the premise: its `andi v1,a1,0xffff` sits at index 14, immediately
after `lhu a1,106(s0)` (12) and `lw s4,0(s0)` (13), with no branch between. The mask is in
the SAME basic block as the load, so combine's block-local `nonzero_bits` was never the
reason the original kept it.

**The real signal is an asymmetry between the two `lhu`s.** The second one
(`lhu v0,106(s4)`, index 19 — same 0x6A offset, different base) gets NO `andi` in target.
The only structural difference: `temp_a1` has TWO uses (the mask at :522 and the
`- 0x19` subtraction at :534); the second load's value has one. So the `andi` exists
because the raw value must stay live for a second, differently-extended use.

Probes this session, all measured:

| probe | change | score | what it taught |
|---|---|---|---|
| P0 | `u16 temp_a1` -> `u32` | 44 | dead; the `lhu` proves 16-bit regardless of holder type |
| P1 | drop the mask, `temp_v1 = temp_a1;` | 44 | the explicit `& 0xFFFF` is not what enables the fold |
| **P2** | **`u16 temp_a1` -> `s16`** | **44** | **STRUCTURALLY RIGHT: `andi v1,a2,0xffff` appears at index 14, matching target exactly.** Net zero because both uses then sign-extend, so the load becomes `lh` where target has `lhu` |
| P3 | P2 + `(u16)` cast at the subtraction | 63 | much worse; reverted |

Tree is back at HEAD for this file (all probes reverted).

## What the residual actually requires

Target needs, simultaneously: a ZERO-extending load (`lhu`) for the raw value that feeds
`addiu v0,a1,-25`, AND a separate non-folded `andi` producing the compare operand. With a
`u16` holder both uses zero-extend and the mask is provably redundant (folds). With `s16`
the mask survives but the load turns signed. **P2 shows the `andi` is reachable; the open
question is narrowly "how to keep `lhu` while the mask survives".**

Untried ideas, in order:
1. Give `temp_v1` a second reaching definition so the conversion sits at a merge point and
   cannot be folded into the single-def load.
2. Look for a use that forces `temp_a1` to stay a 16-bit quantity (stored/passed as u16)
   rather than being promoted at both sites.
3. Only then the prologue rotation (known-hard save-order class).

## Recommended next moves

1. The fold is basic-block-local (`reg_last_set_nonzero_bits`). The lever is therefore to
   put a **basic-block boundary between the load and the mask**, or to give `temp_a1` two
   reaching definitions so the nonzero-bits union is not provably 16-bit. Neither has been
   tried. Do NOT retry type widening — it is measured dead.
2. Re-measure after any success on (1); B (`li a1,4` vs `li a2,4`) and D (the folded 0/1
   diamond) may be downstream of A/C rather than independent.
3. Only then treat the prologue rotation, which is the known-hard save-order class
   ([[no-new-park-categories]]).

## Tooling

Use `tools/pairdiff.py code6cac_b saTan2KabutoWareMove`.
`inverse_compose.py classify` does NOT work on this function (`replace_with_asmfile`
target is unreadable to `goalmap.asm_body`) — see the func_80089F3C checkpoint.


## 2026-08-19 session 1 (grinder, recon) — FLOOR 44 -> 30; clusters B, C, D(part) CLOSED

Chassis check: canonical verdict C, sandbox --disable all = 44 at HEAD (211 vs 215), matching the queue.
All edits are live in src/code6cac_b.c and mirrored in memory/grind/func_800283D0/candidate.c.

### Decisive discovery: the mask fold was TREE-LEVEL, not combine — the P0 conclusion was wrong

Dumps (tmp/grind/func_800283D0/s1/): with the old u16 temp_a1, the mask is absent even from the
INITIAL RTL (.rtl dump, grep 65535: nothing in this function's region). fold-const deletes it in
the front end from the declared type alone. So all RTL-pass reasoning about defeating combine's
nonzero_bits was moot — the mask never existed in RTL. The prior "the lhu proves 16-bit
regardless of holder type" (P0, u32) does NOT generalize: sibling func_8002872C in the SAME TU
ships the spelling `s32 a0_raw = *(u16 *)(ptr + 0x6A); v1 = a0_raw & 0xFFFF;` and its
zero_extend + and:SI 65535 pair SURVIVES through combine (.combine dump, its insns 74/76).
MEASURED: `s32 temp_a1` emits lhu + andi exactly like target (211 -> 212 build insns), keeping
the zero-extending load. Combine leaves the AND because the masked pseudo is multi-use.

### The a1/a2 swap: global.c allocno_compare, measured and flipped with clean C

With the s32 holder the residual was li a1,4 + lhu a2 (ours) vs li a2,4 + lhu a1 (target).
greg/lreg: pseudo 74 = raw lhu value (3 refs / 15 insns live), pseudo 81 = constant 4
(3 refs / 14 insns). allocno_compare (global.c:635-655): pri = floor_log2(refs)*refs*10000*size
/ live_length -> pri(74)=2000 < pri(81)=2142, const allocates first, takes a1. The TIEBREAK is
ascending allocno number and 74 < 81, so EQUAL lengths flip the order. The one insn inside 74's
range but outside 81's was the var_s1 = 0 store before the range-check if. block_13 is plain
`return ret;` and never reads var_s1, so moving `var_s1 = 0;` below the range-check if is
semantics-clean. MEASURED: 44 -> 39; slots 6/12/14/27 (li a2,4 / lhu a1 / andi v1,a1 /
addiu v0,a1,-25) all match. Cluster B+C closed.

MEASURED INERT: moving the lhu source statement later (after ret=1) — sched1 re-hoists loads
and live lengths are recomputed post-sched1; load statement order is laundered here.

### Mixed exit form at the rejection chain: 39 -> 37

Target slots 44-47 are beq v1,v0 / nop / j / li v0,1 — an inline return-1 tail. ret is the
constant 1, so `return 1;` instead of `goto block_13` at the chain exit is ordinary mixed-exit
C (SOTN-sanctioned family). MEASURED: 39 -> 37.

### In-arm recompute of temp_a1_2*2: 37 -> 30

Ours hoisted sll (named local s32 temp_v0_2 = temp_a1_2 * 2) above the temp_v1_3 branch; target
recomputes it inside the arm. Deleting the named local and inlining (temp_a1_2 * 2) at both use
sites is ordinary C. MEASURED: 37 -> 30; also resolved the bne/slt operand-order diffs at slots
98-100 downstream.

### Residual at 30 (tmp/grind/func_800283D0/s1/pairdiff_floor30.txt)

1. s2/s3 prologue rotation (~12 diffs): prologue pair + six move a2,s3->s2 +
   addu s2,s0,v0 / lh 648(s2) vs addu s3,v0,s0 / lh 648(s3). Known-hard arg-home wall
   (func_8008C1E8 sibling) — but note the addu OPERAND ORDER also differs (arg0+X vs X+arg0
   at the temp_s3 site): cheap commutativity probe before writing the cluster off.
2. Diamond 2, +4 insns (slots 125-131): target does not cross-jump-merge the second
   var_v0_2 = 0x19/0xB selection copy (C ~585-599); ours merges the two identical copies.
   Target shape: beqz (inverted sense) with li v0,1 in the delay slot feeding the D_800A38A8
   store, selection inline. The v0/v1 rename on the 1/-1 stores (3 diffs) looks downstream.
3. Tail a0/a1 swap (~6 diffs, Judge-table math): sll/addu pointer in a1 (ours) vs a0 (target),
   lh a0 vs lh a1, mult operand. Plausibly the same allocno_compare mechanism as the solved
   swap — redo the pseudo arithmetic from fresh dumps at floor 30.
4. Delay-slot/jump wobble at slots 45-48 (4 near-neutral diffs): ours j/nop, target nop/j —
   reorg fill difference, possibly downstream of diamond 2.

### Tooling notes
- pairdiff must run in WSL: bash tools/wsl.sh 'python3 tools/pairdiff.py code6cac_b func_800283D0'.
- dump.ps1 works; dumps are whole-TU — slice on ';; Function' boundaries (our region precedes
  func_8002872C). Artifacts banked in tmp/grind/func_800283D0/s1/.
- engine diagnose SKIPs this function (absent from tmp/scan purec object) — harmless.

- [s1] Chassis: canonical verdict C, HEAD floor 44 (211 vs 215) re-measured this session before edits.

- [s1] The mask fold was TREE-LEVEL (fold-const via declared type), not combine: 65535 absent from the .rtl dump with u16 holder. Prior P0 (u32) conclusion 'lhu proves 16-bit regardless of holder type' is wrong for s32 - same-TU sibling func_8002872C ships s32 raw + & 0xFFFF and keeps lhu+andi through combine.

- [s1] global.c allocno_compare arithmetic measured: pseudo 74 raw 3refs/15insns pri 2000 vs pseudo 81 const-4 3refs/14insns pri 2142; tiebreak is ascending allocno so equal lengths flip to target's assignment.

- [s1] Floor path this session: 44 -> 39 (var_s1 store placement) -> 37 (inline return 1) -> 30 (in-arm recompute). All edits ordinary C, zero FAKE constructs, live in src/code6cac_b.c and candidate.c.

- [s1] Residual at 30: s2/s3 prologue rotation ~12 diffs (known-hard wall, but an addu operand-order commutativity clue is untested), diamond-2 un-merged var_v0_2 selection +4 insns with downstream v0/v1 rename x3, tail a0/a1 swap ~6 diffs (same allocno_compare shape as the solved swap), 4 near-neutral j/nop layout diffs.

- [s1] Artifacts: combine/greg/lreg slices and pairdiff_floor30.txt in tmp/grind/func_800283D0/s1/.
