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

## 2026-08-26 session 2 (grinder, structural) — FLOOR 30 -> 28; insn count now EXACT (215 == 215)

Chassis re-measured at session start with the s1 candidate applied to src/code6cac_b.c:
sandbox --disable all = 30, ours 211 insns vs target 215. Confirmed the ledger floor.

### Diamond 2 root cause NAMED with the instrumented compiler (not inferred)

The +4-insn gap was the jump2 cross-jump merge of the two `var_v0_2 = 0x19 / 0xB`
selection copies (the in-range arm at C ~91-94 and the `!= 5` tail at C ~101-104).
This is now PROVEN, not hypothesised, by two independent instruments:

1. **Constant-count census across every -da dump** (tmp/grind/func_800283D0/s2/):
   `const_int 25)` occurrences inside the function region go
   rtl=5 jump=5 cse=8 loop=8 cse2=8 combine=8 sched=8 lreg=8 greg=8 **jump2=6** dbr=6.
   The two copies are alive through global allocation and disappear exactly at jump2.
   (`const_int 11)` mirrors it: 3/3/6/6/6/6/6/6/6/**4**/4.)
2. **`tools/gcc-2.7.2/cc1` with `BB2_XJUMP_DEBUG=1`** (trace banked at
   tmp/grind/func_800283D0/s2/xjump_trace.txt, harness xjump.sh). The decisive event:
   ```
   XJDBG: enter e1=362 e2=397 min=2 (chain-partner)
   XJDBG:   MATCH i1=358 i2=393 set(reg<-11) min->1
   XJDBG:   MATCH i1=354 i2=389 set       min->0
   XJDBG:   MATCH i1=351 i2=386 set(reg<-25) min->-1
   XJDBG:   LABEL-BONUS i1=347 (label) min->-2; break
   XJDBG: result e1=362 min=-2 last1=351 => WIN
   XJDBG: DO_CROSS_JUMP jump=362 newjpos=351 newlpos=386
   ```
   Route is the "chain-partner" arm of jump.c:2020 (both jumps have the same
   JUMP_LABEL = block_48), entered with `minimum = 2`; three backward insn MATCHes
   drive min to -1 and the merge fires. NOTE the mechanics: WIN needs only TWO
   matching insns, so a defeat must make the FIRST or SECOND insn before the `j`
   differ.

### The fix that measured: spell ONE copy as an if/else (30 -> 28, 211 -> 215 insns)

Changing the tail copy from
    `var_v0_2 = 0x19; if (var_s1 == 0) { var_v0_2 = 0xB; }`
to
    `if (var_s1 != 0) { var_v0_2 = 0x19; } else { var_v0_2 = 0xB; }`
makes the first backward comparison `set(reg<-11)` vs `set(reg<-25)` — a
PAT-MISMATCH on insn 1 — so find_cross_jump never reaches min<=0 and BOTH copies
survive. **build_insns 211 -> 215, exactly the target count.** Score 30 -> 28.
Mirror placement (if/else on the in-range copy instead of the tail) measures
IDENTICALLY 28/215, so the lever is the asymmetry itself, not which copy carries it.

### KNOWN-IMPERFECT and the open question this leaves

The if/else copy emits `beqz s1 / li v0,11 / j / li v0,25`; target has
`bnez s1 / li v0,25 / j / li v0,11` in BOTH copies (4 diffs at slots 138-141, plus
the v0/v1 rename at 126/132/133/135). So the byte-exact original had two IDENTICAL
copies that jump2 did not merge. Under the traced mechanism identical copies always
merge, so the original's non-merge must come from the pairing never being ATTEMPTED,
not from the comparison failing — i.e. from `jump_chain` / `INSN_UID(...) < max_uid`
ordering at jump.c:2012-2021, or from an earlier WIN in the same chain consuming e1.
That is the next session's question; it is a layout/UID question, not a spelling one.

### The s2/s3 cluster is INDEPENDENT of diamond 2 — measured, not assumed

Re-dumped .lreg/.greg with the 215-insn body in place: pseudo 73 (the arg1 home) is
still `7 refs / 92 insns` and pseudo 143 (temp_s3) still `3 refs / 14 insns`, the
allocation order string is byte-identical to the 211-insn build, and the dispositions
are unchanged (72->s0, 77->s1, 143->s2, 73->s3, 75->s4, 90->s5, 79->s6). Reason: jump2
runs AFTER reload, so nothing the cross-jump merge does can feed back into allocation.
Do not expect the two clusters to interact.

### The exact allocno arithmetic for the s2/s3 flip (global.c:635-655)

pri = floor_log2(n_refs) * n_refs * 10000 * size / live_length, ties -> lower allocno.
  pseudo 143 (temp_s3): floor_log2(3)=1, 1*3*10000/14  = **2142**
  pseudo  73 (arg1)   : floor_log2(7)=2, 2*7*10000/92  = **1521**
  pseudo  75 (temp_s4): floor_log2(6)=2, 2*6*10000/88  = **1363**
143 sorts first, and since MIPS defines no REG_ALLOC_ORDER (verified: no
REG_ALLOC_ORDER in tools/gcc-2.7.2/config/mips), find_reg walks hard regs ascending,
so the first-allocated call-crossing pseudo takes the lowest free callee-saved reg.
s0 goes to 72, s1 to 77, then 143 takes s2 and 73 takes s3. Target needs 73 first.
The flip therefore requires pri(143) to land strictly BETWEEN pri(75)=1363 and
pri(73)=1521 — otherwise 75 steals s3 and 143 lands in s4. With n_refs fixed at 3
that is 30000/L in (1363,1521), i.e. **live_length(143) must become 20 or 21**
(currently 14). Alternatively n_refs(73) would have to reach 8 (floor_log2 3 =>
pri 2608), which there is no honest use for — arg1 is genuinely referenced exactly
7 times. The ONLY honest axis is +6/+7 insns of live length on temp_s3.

### s2 negative results (all banked as rejected/ forms)

- **addu commutativity is laundered.** `temp_s3 = (temp_a1_2 * 2) + arg0` emits the
  identical `addu s2,s0,v0`; score flat 30. s1 frontier item 3 is KILLED: the
  operand-order diff at slots 96-97 is downstream of the register assignment, not an
  independent C lever. (rejected/addu-commutativity-laundered.c)
- **Distinct terminator on the tail copy is self-defeating.** Replacing the tail's
  `goto block_48` with an explicit `*(s16*)(arg0+0x286) = var_v0_2; return ret;`
  measured 29 / 211 insns: jump2 cross-jumps that tail into block_48 FIRST (restoring
  `j block_48`), sets `next = insn`, and then merges the selections on the re-scan.
  (rejected/tail-explicit-store-return-remerges.c)
- **Hoisting temp_s3 above the `temp_v1_3 == 0` branch is catastrophic (28 -> 63).**
  It was the obvious way to buy live length, but cse then folds the ==0 arm's own
  `arg0 + temp_a1_2*2 + 0x288` into temp_s3 (target keeps a separate `addu v0,v0,s0`
  there), pseudo 143 is renumbered out of existence, and the whole callee-saved map
  re-shuffles (72 s0->s1, 77 s1->s2). (rejected/hoist-temp-s3-cses-arm-addu.c)

### Residual at 28 (tmp/grind/func_800283D0/s2/pd_probeB.txt) — 32 diffs, insn count exact

1. s2/s3 rotation, 11 diffs: prologue pair (3,10), six `move a2,s3`->`s2`
   (85,90,102,107,148,153), `addu s2,s0,v0`/`lh 648(s2)` x3 (96,97,111). ONE decision:
   the allocno_compare order above.
2. Diamond 2 residual, 8 diffs: the if/else copy's reversed order (138-141) plus the
   v0/v1 rename on the D_800A38A8/D_800A3876 stores (126,132,133,135).
3. Tail a0/a1 swap, 7 diffs (159,161,168,169,171,178,181): `sll a0,s5,4`/`addu a0,s4,a0`
   pointer in a0 (target) vs a1 (ours), and the Judge `lh` in a1 vs a0. Caller-saved
   allocation in the last block; not yet modelled.
4. Delay-slot/jump wobble at 45-48 (4 diffs, net-zero insns): ours `li v0,1 / j / nop`,
   target `nop / j / li v0,1`. reorg fill.

### Tooling notes (s2)

- The instrumented cc1 (tools/gcc-2.7.2/cc1, NOT build/cc1) works here and
  `BB2_XJUMP_DEBUG=1` answers cross-jump questions in one run. Harness:
  tmp/grind/func_800283D0/s2/xjump.sh (cpp | cc1 with the env var, stderr captured).
  cc1 exits rc 33 on this TU (a pre-existing "too few arguments" error at
  src/code6cac_b.c:424 in an unrelated function) but still emits full output — ignore
  the exit code.
- The per-pass constant census (`awk` between `;; Function` markers, grep
  `const_int N)`) is a cheap way to localise a pass that adds/removes copies; grep for
  `const_int 25 ` with a trailing space finds nothing, the dump prints `25)`.

- [s2] [s2] Chassis re-measured at session start with the s1 candidate applied: sandbox --disable all = 30, ours 211 vs target 215 - the ledger floor confirmed before any edit.

- [s2] [s2] Best form this session measures 28 with build_insns 215 == target 215 EXACTLY; the only edit vs the s1 body is the if/else respelling of the tail var_v0_2 selection. Saved to memory/grind/func_800283D0/candidate.c and live in src/code6cac_b.c.

- [s2] [s2] Pass attribution done with instruments, not inference: a per-pass `const_int 25)` census inside the function region (rtl 5, cse 8, combine 8, greg 8, jump2 6, dbr 6) localises the copy loss to jump2, and BB2_XJUMP_DEBUG=1 on tools/gcc-2.7.2/cc1 names the event `DO_CROSS_JUMP jump=362 newjpos=351 newlpos=386` reached via jump.c:2020's chain-partner arm after three backward MATCHes.

- [s2] [s2] find_cross_jump's WIN condition needs only TWO matching insns walking back from the jump (chain-partner minimum=2), so any defeat must make the FIRST or SECOND insn before the terminating jump differ; the if/else respelling produces `set(reg<-11)` vs `set(reg<-25)` and aborts on insn 1.

- [s2] [s2] KNOWN-IMPERFECT: target has TWO IDENTICAL copies (`bnez s1 / li v0,25 / j / li v0,11`) that jump2 did not merge; our if/else copy is order-reversed (`beqz s1 / li v0,11 / j / li v0,25`), costing 4 diffs at slots 138-141 plus the v0/v1 rename at 126/132/133/135. Since identical copies always merge once paired, the original's non-merge must be a PAIRING failure (jump_chain order / the `INSN_UID(JUMP_LABEL(insn)) < max_uid` guard at jump.c:2012), not a comparison failure.

- [s2] [s2] The s2/s3 rotation is provably independent of diamond 2: with the 215-insn body the .lreg ref/length report, the .greg allocation-order string and every register disposition are byte-identical to the 211-insn build. jump2 is post-reload.

- [s2] [s2] Exact allocno_compare arithmetic for the s2/s3 flip: pri(143 temp_s3)=2142, pri(73 arg1)=1521, pri(75 temp_s4)=1363; MIPS has no REG_ALLOC_ORDER so find_reg assigns ascending. Flipping needs live_length(143) = 20 or 21 (currently 14) - any lower priority than 1363 and temp_s4 steals s3.

- [s2] [s2] Three forms banked as rejected: addu-commutativity respelling (laundered, flat 30), distinct-terminator on the tail copy (29/211, jump2 re-merges after cross-jumping the store+return into block_48), and hoisting temp_s3 above the temp_v1_3 branch (28 -> 63, cse eats the arm's own addu).

- [s2] [s2] Tooling: the instrumented cc1 is tools/gcc-2.7.2/cc1 (NOT build/cc1) and exits rc 33 on this TU because of a pre-existing 'too few arguments' error at src/code6cac_b.c:424 in an unrelated function - it still emits complete output, ignore the exit code. Harness banked at tmp/grind/func_800283D0/s2/xjump.sh.

## 2026-08-26 session 3 (grinder, structural) — FLOOR FLAT AT 28; the tail a0/a1 cluster is now MODELLED with ground truth

Chassis re-measured at session start with the s2 candidate applied to src/code6cac_b.c:
sandbox --disable all = 28, build_insns 215 == target 215. Ledger floor confirmed; no
regression. The session produced no floor drop — it converted frontier item 3 from
"not yet modelled" into an exact numeric requirement and killed eight ordinary-C
structural respellings across all three residual clusters.

### The tail a0/a1 swap (7 diffs) is a local-alloc QUANTITY-ORDER decision — measured

Frontier item 3 said "read the .lreg qty report BEFORE hypothesising". Done, and better:
`tools/gcc-2.7.2/cc1` carries a `BB2_QTY_DEBUG=1` hook (local-alloc.c:1581-1591) that
prints the post-sort quantity order with birth/death/refs/assignment. Harness banked at
tmp/grind/func_800283D0/s3/qty.sh, trace at s3/qty_trace.txt. For the tail block
(local-alloc block 41) it prints, in allocation order:

    QTYDBG blk=41 ord=0 qty=2  reg1=198 birth=10 death=16 refs=6 got=2
    QTYDBG blk=41 ord=1 qty=1  reg1=208 birth=4  death=26 refs=9 got=3
    QTYDBG blk=41 ord=2 qty=4  reg1=203 birth=18 death=20 refs=2 got=2
    QTYDBG blk=41 ord=3 qty=8  reg1=213 birth=28 death=30 refs=2 got=2
    QTYDBG blk=41 ord=4 qty=11 reg1=215 birth=38 death=40 refs=2 got=2
    QTYDBG blk=41 ord=5 qty=3  reg1=201 birth=16 death=20 refs=2 got=4   <- Judge[] element
    QTYDBG blk=41 ord=6 qty=7  reg1=211 birth=26 death=30 refs=2 got=3
    QTYDBG blk=41 ord=7 qty=0  reg1=184 birth=2  death=32 refs=6 got=5   <- temp_a0 pointer

`qty_compare_1` (local-alloc.c:1659-1685) is the SAME shape as global.c's allocno_compare
but over quantities: pri = floor_log2(n_refs) * n_refs * size / (death - birth) * 10000,
and — decisively — ties are broken by ASCENDING QTY NUMBER (`return *q1 - *q2;`).

    pri(qty0, temp_a0 pointer) = floor_log2(6)*6/30 * 10000 = 4000
    pri(qty3, Judge[] element) = floor_log2(2)*2/4  * 10000 = 5000

qty3 sorts first, `find_free_reg` walks hard regs ascending (MIPS defines no
REG_ALLOC_ORDER — re-verified), $2/$3 are already taken by qty2/qty1, so qty3 takes
$a0 (4) and qty0 takes $a1 (5). Target is the mirror: pointer in $a0, Judge[] element in
$a1 (asm/funcs/func_800283D0.s:175-199 — `addu $a0,$s4,$a0`, `lh $a1,%lo(Judge)($at)`,
`lw $v0,0x114($a0)`, `mult $a1,$v0`, `lw $a0,0x118($a0)`).

So the whole 7-diff cluster is ONE decision, and because qty0 < qty3 a TIE IS ENOUGH.
The requirement is therefore pri(qty0) >= 5000, i.e. one of:
  * `death - birth` for qty0 drops from 30 to <= 24 (birth/death step by 2 per insn, so
    that is 3 fewer insns of quantity span), with refs held at 6; or
  * `death - birth` for qty3 rises from 4 to >= 6 (one more insn scheduled between
    `lh Judge(at)` and `mult`), which drops pri(qty3) to 3333 < 4000; or
  * refs(qty0) reaches 8 (floor_log2 3 => 8000) — there is no honest 8th reference.

Note qty0 is NOT just the `temp_a0` pseudo: refs=6 and birth=2 mean local-alloc has
COMBINED the `sll` result pseudo (temp_a0's producer) and `var_a1`'s pseudo into the same
quantity, so the span runs from `sll $5,$21,4` at block index 2 all the way to var_a1's
last use (`slt`) at index 32. That is why the naive "Register 184 used 4 times across 14
insns" read of the .lreg summary line understates it — use BB2_QTY_DEBUG, not the .lreg
summary line, for local-alloc questions.

Caveat for the next session: birth/death are indices in the block AT LOCAL-ALLOC TIME,
i.e. after sched1 but BEFORE sched2/reorg. Our final emitted tail order is instruction-
for-instruction identical to target's (verified against asm/funcs/func_800283D0.s), so the
divergence must live in the PRE-reload order or in which pseudos local-alloc combines into
qty0 — not in anything visible in the final asm.

### Eight ordinary-C structural respellings measured; all laundered or worse

Every variant below was generated from the s2 candidate body and measured with
`sandbox --disable all`; sources are banked in tmp/grind/func_800283D0/s3/v/.

| probe | change | score / insns | verdict |
|---|---|---|---|
| A_s3_int | `temp_s3 = (u8*)((temp_a1_2*2) + (s32)arg0)` — integer-typed plus, to try to flip the `addu` operand order | 28 / 215 | laundered |
| B_a0_int | same integer-plus trick on the tail `temp_a0` | 28 / 215 | laundered |
| C_mul_swap | swap both multiply operand orders in `temp_v1_5` | 29 / 215 | WORSE |
| D_a0_2_late | declare `temp_a0_2` / `var_a1` after `temp_v1_5` | 28 / 215 | laundered |
| E_a0_decl_late | declare `temp_a0` after `temp_v1_4` (shrink its source-level span) | 28 / 215 | laundered |
| F_no_a0_local | delete the `temp_a0` local; spell `temp_s4 + temp_s5*0x10 + off` at all three loads | 28 / 215 | laundered (cse rebuilds the identical pseudo) |
| G_abs_late | move `if (temp_a0_2 < 0) var_a1 = -temp_a0_2;` below `var_v0_3 = temp_v1_5;` | 31 / 216 | WORSE — costs a whole insn |
| L_ternary_tail | tail selection as `var_v0_2 = (var_s1 != 0) ? 0x19 : 0xB;` | 28 / 215 | laundered (identical RTL to the if/else) |
| M_tail_two_gotos | tail selection as `if (var_s1==0) goto tail_0B; v=0x19; goto block_48; tail_0B: v=0xB; goto block_48;` | 28 / 215 | laundered — pairdiff hunk set BYTE-IDENTICAL to base |
| N_no_s3_local | delete the `temp_s3` local; spell `arg0 + temp_a1_2*2 + 0x288` at both sites | 28 / 215 | laundered |

Two conclusions worth banking hard:

1. The declaration-order / local-elimination axis is DEAD for this function. A, B, D, E,
   F and N all target the birth or death of exactly the pseudos the two allocation
   clusters hinge on, and all six are byte-neutral. GCC 2.7.2 rebuilds the same pseudo
   with the same birth from cse + sched1 regardless of where the C names it. This kills
   the cheap end of frontier items 2 and 3 — the remaining levers must change the amount
   of WORK inside the live range, not where the C declares the value.
2. The distinct-label axis for defeating the jump2 pairing is DEAD (M). jump.c threads the
   extra `goto` before the cross-jump phase and rebuilds exactly the two-simplejump shape,
   so the pairing is attempted identically. Combined with s2's KILLED distinct-terminator
   probe, both "make the two copies structurally different at the jump" ideas are now
   spent; only the UID / jump_chain-order route in frontier item 1 remains.

### Statement-order asymmetry (G) is a real, measured constraint

G is the only probe that changed the instruction COUNT (216). The two absolute-value
computations (`|temp_a0_2|` into var_a1 and `|temp_v1_5|` into var_v0_3) must appear in
source in the order abs(temp_a0_2) first, abs(temp_v1_5) second, or reorg loses the shared
`bgez`/`negu` layout and emits an extra instruction. Do not re-probe that ordering.

- [s3] Chassis re-measured at session start with the s2 candidate applied: sandbox --disable all = 28, build_insns 215 == target 215. Floor flat this session; no regression, no improvement.

- [s3] The tail a0/a1 cluster (7 diffs) is a local-alloc quantity-order decision, measured with BB2_QTY_DEBUG=1 on tools/gcc-2.7.2/cc1: pri(qty0 = temp_a0 pointer, birth 2 death 32 refs 6) = 4000 vs pri(qty3 = Judge[] element, birth 16 death 20 refs 2) = 5000, so qty3 is allocated first and takes $a0.

- [s3] qty_compare_1 (local-alloc.c:1659-1685) breaks priority ties by ASCENDING QTY NUMBER, and the pointer is qty0 while the Judge element is qty3 - so a TIE is sufficient for target's assignment. Requirement: span(qty0) <= 24 (from 30, i.e. 3 fewer insns) OR span(qty3) >= 6 (from 4, i.e. one more insn between the `lh Judge` and the `mult`).

- [s3] qty0 is a COMBINED quantity (the sll producer + temp_a0 + var_a1), which is why the .lreg summary line 'Register 184 used 4 times across 14 insns' understates it (refs 6, span 30). For local-alloc questions read BB2_QTY_DEBUG, not the .lreg summary line.

- [s3] Our emitted tail is instruction-for-instruction identical to target's (asm/funcs/func_800283D0.s:175-199) apart from the a0/a1 rename, so the qty-span divergence lives in the PRE-reload (post-sched1) order or in local-alloc's quantity COMBINING, not in anything visible in the final asm.

- [s3] Eight ordinary-C structural respellings measured, all byte-neutral at 28/215: integer-typed pointer plus on temp_s3 (A) and temp_a0 (B), declaring temp_a0_2/var_a1 late (D), declaring temp_a0 late (E), deleting the temp_a0 local (F), ternary tail selection (L), two-goto-label tail selection (M, pairdiff hunk set byte-identical), deleting the temp_s3 local (N). The declaration-order / local-elimination axis is dead for this function.

- [s3] Two probes are WORSE and banked as rejected: swapping the multiply operand orders in temp_v1_5 (29/215) and sinking the temp_a0_2 abs below the temp_v1_5 abs (31/216 - it costs a whole instruction, so the relative order of the two abs computations is load-bearing).

- [s3] Artifacts: tmp/grind/func_800283D0/s3/qty.sh + qty_trace.txt (local-alloc quantity trace), pd_start.txt and pd_M.txt (pairdiff at 28), v/*.c (all ten probe bodies), lreg.txt/greg.txt (function-sliced allocation dumps).

- [s3] LATE PROBE P_var_a1_ifelse: spelling the tail absolute value as `if (temp_a0_2 < 0) { var_a1 = -temp_a0_2; } else { var_a1 = temp_a0_2; }` (no `var_a1 = temp_a0_2;` initialiser, so no reg-to-reg copy for local-alloc's combine_regs to fuse onto the pointer's quantity) measures 28 / 215 AND leaves the block-41 BB2_QTY_DEBUG table byte-identical (qty0 still birth 2 death 32 refs 6, qty3 still birth 16 death 20 refs 2). GCC rebuilds the same copy insn from the two-arm form, so the 'break the var_a1 copy to shorten qty0' axis is KILLED before it was ever spent. Trace banked at tmp/grind/func_800283D0/s3/qty_trace_P.txt, body at s3/v/P_var_a1_ifelse.c.

- [s3] Chassis re-measured at session start with the s2 candidate applied to src/code6cac_b.c: sandbox --disable all = 28, build_insns 215 == target 215. Floor flat this session; the body in src/ and memory/grind/func_800283D0/candidate.c is unchanged and re-verified at 28 after all probes were reverted.

- [s3] tools/gcc-2.7.2/cc1 carries a BB2_QTY_DEBUG=1 hook (local-alloc.c:1581-1591) that prints the post-sort local-alloc quantity order with blk/ord/qty/reg1/birth/death/refs/got. It answers local-alloc questions in one run, exactly as BB2_XJUMP_DEBUG does for cross-jump questions. Full hook list: BB2_ALLOC_DEBUG, BB2_DBR_DEBUG, BB2_FINDREG_DEBUG, BB2_FLOW_DEBUG, BB2_FRAME_DEBUG, BB2_PRIO_DEBUG, BB2_QTY_DEBUG, BB2_RANK_DEBUG, BB2_RELOAD_DEBUG, BB2_SCHED_DEBUG, BB2_SUGG_DEBUG, BB2_XJUMP_DEBUG.

- [s3] Block-41 quantity order (ours): ord0 qty2 refs6 span6 -> $2; ord1 qty1 refs9 span22 -> $3; ord2 qty4 -> $2; ord3 qty8 -> $2; ord4 qty11 -> $2; ord5 qty3 (Judge[] element) refs2 span4 -> $4; ord6 qty7 -> $3; ord7 qty0 (temp_a0 pointer) refs6 span30 -> $5.

- [s3] qty_compare_1 (local-alloc.c:1659-1685) breaks priority ties by ascending qty number, and the pointer is qty0 while the Judge[] element is qty3 - so a TIE is sufficient for target's assignment. Requirement: span(qty0) <= 24 (from 30, i.e. 3 fewer insns) OR span(qty3) >= 6 (from 4, i.e. one more insn between the `lh Judge` and the `mult`).

- [s3] qty0 is a COMBINED quantity spanning the sll producer, the temp_a0 pointer and var_a1 (refs 6, birth 2, death 32). The .lreg summary line 'Register 184 used 4 times across 14 insns' describes only one member pseudo and is misleading for priority arithmetic - read BB2_QTY_DEBUG for local-alloc questions.

- [s3] Our emitted tail is instruction-for-instruction identical to target's (asm/funcs/func_800283D0.s:175-199) apart from the a0/a1 rename, so the qty-span divergence lives in the PRE-reload (post-sched1) insn order or in which pseudos local-alloc COMBINES into qty0, not in anything visible in the final asm.

- [s3] Eight ordinary-C structural respellings measured byte-neutral at 28/215: A_s3_int, B_a0_int, D_a0_2_late, E_a0_decl_late, F_no_a0_local, L_ternary_tail, M_tail_two_gotos (pairdiff hunk set proven byte-identical), N_no_s3_local. The declaration-order / local-elimination axis is dead for this function.

- [s3] Two probes measured WORSE and are banked as rejected forms: tail-mult-operand-swap-worse.c (29/215) and tail-abs-sunk-below-shift-adds-insn.c (31/216). diamond2-two-goto-labels-laundered.c is banked as the third rejected form (byte-identical, i.e. a proven-inert axis).

- [s3] The residual at 28 is unchanged in shape and now fully attributed: 11 diffs = the s2/s3 global.c allocno_compare rotation (needs live_length(pseudo 143) in {20,21,22}); 8 diffs = the diamond-2 selection copies (needs the jump2 pairing never to be ATTEMPTED); 7 diffs = the tail a0/a1 local-alloc quantity order (needs pri(qty0) >= 5000); 4 near-neutral j/nop reorg-fill diffs at slots 45-48.

- [s3] LATE PROBE P_var_a1_ifelse (the obvious next move on the tail cluster) is KILLED before the next session spends it: removing the `var_a1 = temp_a0_2;` initialiser in favour of an explicit two-arm write leaves both the score (28/215) and the entire block-41 BB2_QTY_DEBUG quantity table byte-identical.

## 2026-08-27 — session 4 (permuter modality)

Chassis at dispatch: `sandbox func_800283D0 --disable all` = **28 / 215 insns**, identical to
the s2/s3 ledger floor. The candidate.c body was re-applied to src/code6cac_b.c (which sits at
`INCLUDE_ASM` on main under asm-until-matched) before any measurement, and src was restored to
exactly that body at session end (re-verified 28 / 215).

### Permuter workspaces (all four launched AND harvested+stopped inside this session)

A standalone single-function permuter workspace was built for func_800283D0 for the first time
(`tmp/grind/func_800283D0/s4/mkws.sh`, modelled on the func_80023648 s5c workspace). Two
deviations from that template were required and are worth reusing:

- the base-object prelude must be **empty**, not `.set noat / .set noreorder`. func_800283D0's
  `lh $a1, Judge(...)` goes through an assembler macro that needs `$at`, so the SOTN-style
  `.set noat` prelude fails the build with "macro used $at after .set noat". The real Makefile
  passes no such prelude for compiled C either (`AS_FLAGS := -Iinclude -march=r3000 -mtune=r3000
  -no-pad-sections -O1 -G0`), so an empty prelude is also the faithful choice.
- the workspace validated on first build: base 215 insns vs target 215 insns, with the diff
  hunks matching the known cluster map, so the standalone TU is a faithful proxy for the
  in-TU compile.

| workspace | label | chassis | iterations | outcome |
|---|---|---|---|---|
| `tmp/grind/func_800283D0/s4/perm_a` | s4a-canonical-chassis | candidate.c body | (stopped early) | 2 finds; best = the do-while(0) wrap, sandbox 26 |
| `tmp/grind/func_800283D0/s4/perm_b` | s4b-mirror-chassis | if/else on the IN-RANGE copy | (stopped early) | 1 find, `arg0 - -(temp_a1_2*2)`, sandbox 28 (byte-neutral) |
| `tmp/grind/func_800283D0/s4/perm_c` | s4c-dowhile0-chassis-floor26 | seeded from perm_a's 26 body | 8859 | 2 finds; best sandbox 22 but SEMANTICALLY INVALID |
| `tmp/grind/func_800283D0/s4/perm_d` | s4d-chain-floor22 | seeded from perm_c's 22 body | 7000 | 1 find, also semantically invalid |

**The permuter's weighted score and the engine distance disagree freely.** perm_b's 468 -> 458
"improvement" was byte-neutral at 28; perm_a's 468 -> 460 was worth 2 real points. Every find
must be re-measured with `sandbox --disable all`, and read against the CFG for semantics, before
it means anything.

### The one semantically-valid find: `do { } while (0)` — 28 -> 26, and why it is not adopted

The only improving, semantics-preserving find in ~20k iterations wraps the region from
`if (temp_v1_3 < temp_v0_3)` through `block_48: *(s16*)(arg0+0x286) = var_v0_2;` in
`do { ... } while (0);`. It measures **26 / 215**.

Its mechanism was measured, not guessed, by re-dumping .lreg and diffing the per-pseudo report
against the pre-wrap snapshot (`tmp/grind/func_800283D0/s4/lreg_base28.txt`):

| pseudo | what it holds | live length 28-body -> 26-body | REG_N_REFS 28-body -> 26-body |
|---|---|---|---|
| 72 | arg0 pointer | 155 -> 155 | 19 -> 24 |
| 73 | arg1 home | 92 -> 92 | 7 -> 9 |
| 75 | temp_s4 | 88 -> 88 | 6 -> 7 |
| 77 | (call-crossing) | 73 -> 73 | 9 -> 11 |
| 90 | (call-crossing) | 32 -> 32 | 3 -> 4 |
| 128 | (tail) | 7 -> 7 | 4 -> 5 |
| 143 | temp_s3 | 14 -> 14 | 3 -> 3 (all its refs are OUTSIDE the wrap) |

Every live length is byte-identical; only the ref counts move, by exactly +1 per reference
sited inside the wrapped region. That is flow.c's loop-depth ref weighting
(`REG_N_REFS (regno) += loop_depth`; the do-while leaves a NOTE_INSN_LOOP_BEG/END pair behind,
so loop_depth is 2 inside it), feeding global.c:635-655 allocno_compare.

`.claude/rules/do-while-zero-exception.md` sanctions the do-while(0) wrap for the
**LABEL_OUTSIDE_LOOP_P / reorg.c interaction ONLY**. The measured mechanism here is a
global.c allocno-priority lever, which is outside that scope, and the no-new-park-categories
non-extension clause explicitly forbids generalizing one carve-out to the broader category. So
this is a first reach of an unsanctioned mechanism wearing a sanctioned family's syntax. It is
also not a match (26, not 0), and a FAKE construct independently requires demonstrated
modality-ladder exhaustion, which four sessions in does not have. Banked NOT adopted at
`memory/grind/func_800283D0/rejected/dowhile0-refweight-out-of-scope.c`; the honest floor
stays 28 and src carries the 28 body.

### What the wrap experiment PROVES (the session's main asset)

pairdiff of the 26 body against target (`tmp/grind/func_800283D0/s4/pd_dowhile.txt`, compare
`tmp/grind/func_800283D0/s3/pd_start.txt`):

- the prologue hunks `ours[3:5]` and `ours[10:11]` — the arg1-home s2/s3 rotation the ledger has
  chased since session 2 — **disappear**. Our `sw s3,36(sp)` / `move s3,a1` become target's
  `sw s2,32(sp)` / `move s2,a1`.
- but temp_s4's pseudo moves to `$s5` (target `$s4`) and temp_s5's to `$s3` (target `$s5`),
  creating six new hunks at `ours[13:14]`, `[20:21]`, `[52:53]`, `[59:60]`, `[69:71]`,
  `[121:122]`.

So the seven callee-saved pseudos are allocated as **one ordered permutation**: any lever that
reorders the priority list trades one cluster for another rather than fixing part of it. And
crucially, the order moved on **REG_N_REFS with every live length pinned** — the first clean
isolation of that axis. Session 2's H9 spent the live_length denominator and concluded
"n_refs(73) cannot honestly reach 8"; that arithmetic is right but it was the wrong search.
The live question for session 5 is whether an ordinary-C restructure can legitimately move the
reference count of pseudo 73 (arg1 home, 7 refs, pri 1521) or pseudo 75 (temp_s4, 6 refs,
pri 1363) — the adjacent pair in the priority order — by one, without disturbing the other five.

### The 22 body: semantically invalid, and directionally impossible

perm_c's best find (sandbox 22) reuses `temp_s3` as a store pointer:
`temp_s3 = arg0 + 0x286;` inside the `temp_v1_3 < temp_v0_3 && var_s1 != 0` arm, then
`*(s16 *)temp_s3 = var_v0_2;` at block_48. **block_48 is also reached from the
`temp_v1_3 >= temp_v0_3` (var_v0_2 = 0x1A) path, where temp_s3 still holds
`arg0 + temp_a1_2 * 2`** — that store would land at the wrong address. decomp-permuter's
randomizer does not guarantee semantic equivalence.

It is also directionally impossible: the target emits `sh $v0, 0x286($s0)` at **all three**
store sites (asm/funcs/func_800283D0.s:92, 162, 222) — base+offset off the arg0 pointer, never
register-indirect. No store-address pointer local can be the answer here, whatever it does to
the allocator. Banked at
`memory/grind/func_800283D0/rejected/permuter-temp-s3-store-pointer-reuse-SEMANTICALLY-INVALID.c`.

Three-way ablation of that body (each variant measured with `sandbox --disable all`):

| variant | body | score |
|---|---|---|
| `tmp/grind/func_800283D0/s4/perm_c/output-440-1/source.c` | wrap + store-pointer reuse + dead-store-in-condition | 22 |
| `tmp/grind/func_800283D0/s4/v_nowrap.c` | wrap REMOVED | 25 |
| `tmp/grind/func_800283D0/s4/v_nodeadstore.c` | `(var_s1 = 0)` restored to `0` | 22 |
| `tmp/grind/func_800283D0/s4/v_nos3reuse.c` | store-pointer reuse REMOVED | 26 |

So the wrap is worth 3, the (invalid) store-pointer reuse is worth 4, and the
`if (temp_a0_2 > (var_s1 = 0))` dead-store-in-condition is worth **exactly 0** — pure permuter
noise, killed as a lever for this function.

### Standing conclusion about the permuter for this function

~20k iterations across four chassis (including two seeded from lower-floor bodies) produced
exactly one semantically-valid improving find, and it is out-of-family. All remaining distance
is callee-saved register-assignment permutation, which the permuter's local source edits reach
only through constructs that are out-of-family or semantically wrong. **A further permuter
session on this chassis is not a good use of a modality slot** — record this so the ladder does
not spend one.

- [s4] Chassis re-measured at dispatch and again at session end: sandbox func_800283D0 --disable all = 28, build_insns 215 == target 215. src/code6cac_b.c carries exactly the candidate.c body at session end (git diff shows only the func_800283D0 INCLUDE_ASM -> C body swap; no other tracked file touched except the engine's own metrics/events.jsonl).

- [s4] First standalone single-function permuter workspace for func_800283D0 exists and validates: tmp/grind/func_800283D0/s4/mkws.sh builds base.c/compile.sh/target.o/settings.toml and reports 'base insns: 215  target: 215'. Two reusable deviations from the func_80023648 s5c template: (1) the base-object prelude must be EMPTY - func_800283D0's `lh $a1, Judge(...)` goes through an assembler macro needing $at, so a `.set noat` prelude fails with 'macro used $at after .set noat'; the real Makefile passes no prelude for compiled C either. (2) tools/pairdiff.py takes TWO arguments (stem then func): `python3 tools/pairdiff.py code6cac_b func_800283D0`.

- [s4] The permuter's weighted score and the engine distance disagree freely and in both directions: perm_b's 468 -> 458 find was byte-neutral (sandbox 28 -> 28) while perm_a's 468 -> 460 find was worth 2 real points. Every find must be re-measured with sandbox --disable all AND read against the CFG for semantics before it means anything.

- [s4] decomp-permuter's randomizer does NOT preserve semantics: two of the four improving finds this session (perm_c output-440-1 at sandbox 22, perm_d output-400-1) change the block_48 store target or move a store between branch arms. Reading the find against the CFG must precede scoring it.

- [s4] MECHANISM MEASURED, not guessed: the do-while(0) wrap leaves every live length byte-identical (72:155, 73:92, 75:88, 77:73, 90:32, 128:7, 143:14) and raises REG_N_REFS by exactly +1 per reference sited inside the wrap (72 19->24, 73 7->9, 75 6->7, 77 9->11, 90 3->4, 128 4->5). That is flow.c `REG_N_REFS (regno) += loop_depth` with loop_depth 2 inside the NOTE_INSN_LOOP_BEG/END pair the do-while leaves behind, feeding global.c:635-655 allocno_compare - NOT the LABEL_OUTSIDE_LOOP_P / reorg.c interaction that .claude/rules/do-while-zero-exception.md sanctions.

- [s4] The seven callee-saved pseudos are one ordered permutation, not independent clusters: the 26 body FIXES the arg1-home s2/s3 rotation (hunks ours[3:5], ours[10:11] disappear) and simultaneously BREAKS temp_s4 ($s5 vs target $s4) and temp_s5 ($s3 vs target $s5), adding six new hunks. Partial fixes to this cluster are not additive.

- [s4] The target emits `sh $v0, 0x286($s0)` at ALL THREE store sites (asm/funcs/func_800283D0.s:92, 162, 222) - base+offset off the arg0 pointer, never register-indirect. Any future proposal that names a pointer local for the 0x286 store address is directionally impossible and can be rejected without measurement.

- [s4] Three-way ablation of the 22 body, each measured with sandbox --disable all: full body 22; do-while wrap removed (v_nowrap.c) 25; `(var_s1 = 0)` restored to `0` (v_nodeadstore.c) 22; store-pointer reuse removed (v_nos3reuse.c) 26. So the wrap is worth 3, the (invalid) store-pointer reuse 4, and the dead-store-in-condition exactly 0.

- [s4] STANDING RECOMMENDATION for the modality ladder: do not spend another permuter slot on this chassis. ~20k iterations across four chassis, two of them seeded from lower-floor bodies, produced exactly one semantically-valid improving find and it is out-of-family. The residual is register-assignment permutation, which local source perturbation reaches only through out-of-family or semantically-wrong constructs.

- [s4] All four campaigns were harvested with --stop inside this session; `permuter_campaign.py status` reports alive=false / registered_active=false for every workspace. No orphaned campaign.

## 2026-08-26 — session 5 (synthesis) — FLOOR FLAT AT 28; the callee-saved model is now GROUND-TRUTHED, its flip window MEASURED, and the ref-count axis FORECLOSED by target's own asm

Chassis re-measured at session start with candidate.c applied to src/code6cac_b.c:
`sandbox func_800283D0 --disable all` = **28**, build_insns 215 == target 215.  src was
restored to HEAD (`INCLUDE_ASM`) at session end; candidate.c is unchanged.

### 1. The global-allocation model is no longer reconstructed by hand — it is dumped

Sessions 2-4 derived allocno_compare arithmetic by hand from the .lreg/.greg summary lines.
`tools/gcc-2.7.2/cc1` has a dedicated hook that prints the table directly:
`BB2_ALLOC_DEBUG=1` (global.c:379 seed dump, global.c:605 post-sort table).  Harness banked at
`tmp/grind/func_800283D0/s5/alloc.sh`, full trace at `s5/alloc_trace.txt`.  The callee-saved
rows for the candidate body:

    ord=10 pseudo=72  hardreg=16($s0) nrefs=19 livelen=155 pri=4903
    ord=13 pseudo=77  hardreg=17($s1) nrefs=9  livelen=73  pri=3698
    ord=18 pseudo=143 hardreg=18($s2) nrefs=3  livelen=14  pri=2142   <- temp_s3 pointer
    ord=20 pseudo=73  hardreg=19($s3) nrefs=7  livelen=92  pri=1521   <- arg1 home
    ord=21 pseudo=75  hardreg=20($s4) nrefs=6  livelen=88  pri=1363   <- temp_s4
    ord=23 pseudo=90  hardreg=21($s5) nrefs=3  livelen=32  pri=937    <- temp_s5
    ord=24 pseudo=79  hardreg=22($s6) nrefs=6  livelen=322 pri=372

Every number the ledger carried since s2 is confirmed exactly.  Two additions the hand model
did not have:

* `ALLOCDBG seed_used` = 0-15, 24-29, 31-51, 64-67 — **$s0-$s7 (16-23) are NOT in the seed**, so
  no callee-saved register is reachable in find_reg's pass 0 (pass 0 masks in
  `IOR_COMPL_HARD_REG_SET (used, regs_used_so_far)`, i.e. it only considers registers already
  used).  Every callee-saved assignment happens in pass 1, walking hard regs ascending
  (global.c:1052-1083; MIPS defines no REG_ALLOC_ORDER).  First-allocated takes the lowest free
  callee-saved register — the ledger's assumption, now read off the source and the trace.
* find_reg CAN override that choice from `hard_reg_copy_preferences` /
  `hard_reg_preferences` (global.c:1096-1163), and pass 0 additionally excludes
  `regs_someone_prefers`.  Those sets are built from copies involving HARD registers, and no
  callee-saved hard register appears anywhere in this function's pre-reload RTL, so they are
  empty for all seven pseudos here.  **The preference axis is inert for this function** — the
  assignment is a pure function of the priority ORDER.  (Checked because the hook list advertises
  BB2_FINDREG_DEBUG / BB2_SUGG_DEBUG and no prior session had looked.)

### 2. The REF-COUNT axis is FORECLOSED — target's own asm pins every count to ours

Counting register references directly in `asm/funcs/func_800283D0.s`:

    $s2 (arg1 home) : def line 6, uses 95,100,113,118,164,169                 = 7 refs
    $s3 (temp_s3)   : def line 107, uses 108,122                              = 3 refs
    $s4 (temp_s4)   : def line 15, uses 22,57,77,132,177                      = 6 refs
    $s5 (temp_s5)   : def line 64, uses 76,175                                = 3 refs

Identical to pseudos 73/143/75/90 in the table above.  So the original compile fed
allocno_compare the SAME numerator for all four.  Any construct that moves a reference count
moves us AWAY from the original's RTL — which is exactly what the s4 do-while(0) wrap did
(it raised every in-wrap ref count by 1, fixed the arg1 home and broke temp_s4/temp_s5).
**Do not propose ref-count levers for this cluster again.**  The only free variable left is
`reg_live_length`.

### 3. The flip window is WIDER than the ledger recorded — measured with a liveness dial

s2's H9 concluded `live_length(143)` must be exactly 20 or 21.  That was derived holding
L75 = 88 fixed, which is wrong: real instructions added inside 143's live range also lengthen
75's (temp_s4 spans nearly the whole function).  Decisively, they do NOT lengthen 73's — arg1
is **dead after the equal-arm's second `func_80032854` call**, so the window between that call
and the `lh 0x288($s3)` of the `== 5` test lies inside 143's range and outside 73's.

Harness `tmp/grind/func_800283D0/s5/dial.sh` inserts n throwaway stores
(`*(s16 *)(arg0 + 0x290 + 2i) = i+1;`, 2 insns each, chosen because they are neither
DSE-able nor constant-foldable — a same-global store chain folds and a repeated-constant
chain is dead-stored, both of which silently pinned livelen at +2/+3 in the first two attempts)
immediately before the `== 5` test and re-dumps the ALLOCDBG table:

    n=1 (k=2)  143 livelen=16 pri=1875  -> $s2   73 -> $s3   (unchanged)
    n=2 (k=4)  143 livelen=18 pri=1666  -> $s2   73 -> $s3   (unchanged)
    n=3 (k=6)  143 livelen=20 pri=1500  -> $s3   73 -> $s2   ** FLIPPED to target **
    n=4 (k=8)  143 livelen=22 pri=1363  -> $s3   73 -> $s2   ** FLIPPED **
    n=5 (k=10) 143 livelen=24 pri=1250  -> $s3   73 -> $s2   ** FLIPPED **
    n=6 (k=12) 143 livelen=26 pri=1153  -> $s4   75 -> $s3   BROKEN (temp_s4 overtakes)

So the requirement is the two-sided inequality

    pri(143) < pri(73)  ->  30000/L143 < 140000/92        ->  L143 >= 20
    pri(143) > pri(75)  ->  30000/L143 > 120000/(88+k)    ->  L143 < (88+k)/4

i.e. **k in [6,10] insns of extra liveness placed strictly inside 143's range and outside 73's**,
with 5 insns of slack.  The model predicted k in {6,8,10} before the dial was run and the dial
reproduced it exactly, including the k=12 break.  The narrow {20,21} window in the ledger is
superseded.

### 4. What that leaves for the s2/s3 cluster — and why it is now a structural question

The dial proves the mechanism but the filler is not adoptable: those are 6 real instructions and
our build already sits at exactly 215 == target 215.  Target's own asm shows NOTHING between the
equal-arm's second call and the `== 5` load (lines 113-122).  So there is no honest place to put
6-10 instructions.

The only mechanism that buys live length at zero instruction cost is making pseudo 143
**live-out along the not-equal arm** of `bne $v0,$v1,.L80028608` — liveness is accumulated over
every block where the pseudo is live, so a use reachable from that arm would add the whole `<`
/ `>=` region for free.  Target has no such use ($s3 is referenced exactly 3 times, all on the
equal path), so no byte-neutral spelling can create one.

**Conclusion: with the rest of the function's RTL fixed as it is, the s2/s3 rotation is
foreclosed.**  Since GCC is deterministic and target's ref counts match ours exactly, the
original's assignment can only come from a whole-function RTL that differs from ours somewhere
we have not yet located — a flow-time insn count difference inside 143's range that a later pass
removed, or a different block/liveness structure.  That reframes cluster 1 from "find a spelling
lever" (spent across four sessions) to "find where our RTL still differs from the original's",
which is a forensics/rederive question, not a structural one.

### 5. Diamond 2 — target re-read, and the last structural spelling route KILLED

Reading the target selection sites directly (`asm/funcs/func_800283D0.s`) there are FOUR, not two:

    (i)   ==0 arm       .L80028510-.L8002851C : `bnez $s1,.L8002851C / addiu 0x19 / .L80028518: addiu 0xB / .L8002851C: sh 0x286($s0)` then FALLS THROUGH into the call pair
    (ii)  `<` arm       .L80028608+           : `beqz $s1,.L80028518 / addiu 0x19 / j .L80028520 / sh 0x286($s0)`  -- jumps INTO (i)'s 0xB arm and (i)'s call block
    (iii) in-range arm  .L800285CC            : `bnez $s1,.L800286F8 / addiu 0x19 / j .L800286F8 / addiu 0xB`
    (iv)  `!= 5` tail   .L800285F8            : byte-identical to (iii)

(iii) and (iv) are BYTE-IDENTICAL four-insn blocks with the same jump label and jump2 did not
merge them — which is the s2 conclusion (the pairing was never attempted), now confirmed against
the emitted target rather than inferred.

NEW PROBE, killed: `v_nolabel` — delete the shared `block_48:` label entirely and write
`*(s16 *)(arg0 + 0x286) = var_v0_2; return ret;` into (iii) and (iv) with both copies in
target's canonical `v=0x19; if(!s1) v=0xB;` order (the theory: with no source-level shared
label, jump2's own cross-jump would CREATE the shared block, and a label created during jump
optimization has UID >= max_uid so `jump_chain` never offers the selection blocks as chain
partners).  **MEASURED 30 / 211 insns** — jump2 cross-jumps the store+return tails FIRST,
rebuilds the shared block, and then merges the selections on the rescan, exactly as s2's H7b
partial version did.  Banked at
`memory/grind/func_800283D0/rejected/diamond2-no-shared-label-remerges.c`.

With s2 (distinct terminator), s3 (two goto labels; ternary) and this one spent, EVERY
source-structural route to defeating the jump2 pairing is now measured dead.  The only remaining
route is the `jump_chain` / `INSN_UID (JUMP_LABEL (insn)) < max_uid` ordering itself, and
answering it needs a print of those two values inside jump.c — an edit to `tools/gcc-2.7.2`,
which is OUTSIDE a grind session's allowed surface.  Route it as an operator instrumentation
request rather than re-probing spellings.

- [s5] Chassis re-measured at session start (candidate applied) and the body restored to HEAD INCLUDE_ASM at session end: sandbox --disable all = 28, build_insns 215 == target 215. candidate.c unchanged.

- [s5] BB2_ALLOC_DEBUG=1 on tools/gcc-2.7.2/cc1 prints the post-sort global allocno table directly (global.c:605) plus the regs_used_so_far seed (global.c:379). It confirms every hand-derived number in the ledger: 72(19/155/4903) 77(9/73/3698) 143(3/14/2142) 73(7/92/1521) 75(6/88/1363) 90(3/32/937) 79(6/322/372). Use this hook instead of re-deriving from .lreg/.greg summary lines. Harness: tmp/grind/func_800283D0/s5/alloc.sh.

- [s5] $s0-$s7 are absent from ALLOCDBG's regs_used_so_far seed, so every callee-saved assignment happens in find_reg's PASS 1 walking hard regs ascending; and hard_reg_copy_preferences / hard_reg_preferences / regs_someone_prefers (global.c:1096-1163) are all empty here because no callee-saved hard reg appears in pre-reload RTL. The preference axis is INERT for this function - the callee-saved map is a pure function of the priority order.

- [s5] The REF-COUNT axis for the callee-saved cluster is FORECLOSED by target's own asm: $s2 has 7 refs (def line 6; uses 95,100,113,118,164,169), $s3 has 3 (def 107; uses 108,122), $s4 has 6 (def 15; uses 22,57,77,132,177), $s5 has 3 (def 64; uses 76,175) - identical to our pseudos 73/143/75/90. Any ref-count lever moves away from the original's RTL, which is why the s4 do-while(0) wrap traded one cluster for two.

- [s5] The s2 ledger's 'live_length(143) must be 20 or 21' window is SUPERSEDED and was too narrow: it held L75 = 88 fixed, but real insns inside 143's range also lengthen 75's. They do NOT lengthen 73's, because arg1 is dead after the equal-arm's second func_80032854 call. Correct constraint: L143 >= 20 AND L143 < (88+k)/4, i.e. k in [6,10] added insns.

- [s5] MEASURED with a liveness dial (tmp/grind/func_800283D0/s5/dial.sh, filler stores to distinct arg0-relative offsets inserted just before the `== 5` test): k=2 and k=4 leave 143 in $s2; k=6, k=8 and k=10 FLIP the assignment to target's (73 -> $s2, 143 -> $s3, 75 -> $s4); k=12 breaks it the other way (75 overtakes 143). The model predicted the window before the dial ran and the dial reproduced both ends exactly.

- [s5] Dial-harness gotcha worth reusing: filler that is a same-global store chain gets constant-folded (`D = D + 1; D = D + 2;` -> one add) and filler that repeatedly stores constants to one global gets dead-stored, so both silently pin livelen at +2/+3 regardless of n. Use stores to DISTINCT addresses (`*(s16 *)(arg0 + 0x290 + 2i) = i+1;`) to get a linear 2-insn-per-statement dial.

- [s5] With target proving there are no instructions between the equal-arm's second call and the `== 5` load, and with the only zero-insn liveness mechanism (making 143 live-out on the not-equal arm) requiring a use of `arg0 + temp_a1_2*2` that target does not have, the s2/s3 rotation is foreclosed under our current whole-function RTL. It is now a forensics question - where does our RTL still differ from the original's - not a spelling question.

- [s5] Target has FOUR selection sites, not two: (i) the ==0 arm at .L80028510 which stores and falls through into the call pair, (ii) the `<` arm at .L80028608 which jumps INTO (i)'s 0xB label .L80028518 and (i)'s call block .L80028520, and (iii)/.L800285CC + (iv)/.L800285F8 which are BYTE-IDENTICAL four-insn blocks both jumping to .L800286F8 and left unmerged by jump2.

- [s5] KILLED: deleting the shared block_48 label and writing `*(s16 *)(arg0 + 0x286) = var_v0_2; return ret;` into BOTH the in-range arm and the `!= 5` tail, with both copies in target's canonical order (theory: jump2 would create the shared block itself and the new label's UID >= max_uid would keep the selection blocks out of jump_chain). Measured 30 / 211 - jump2 cross-jumps the store+return tails first, rebuilds the shared block, and merges the selections on the rescan. Banked as rejected/diamond2-no-shared-label-remerges.c.

- [s5] Every SOURCE-STRUCTURAL route to defeating the jump2 selection pairing is now spent: distinct terminator (s2 H7b), two goto labels (s3 M), ternary (s3 L), and no-shared-label (s5). The only route left is the jump_chain / `INSN_UID (JUMP_LABEL (insn)) < max_uid` ordering at jump.c:2012-2021, which needs a print inside tools/gcc-2.7.2/jump.c - outside a grind session's allowed surface, so route it as an operator instrumentation request.

- [s5] Chassis re-measured at session start with candidate.c applied: sandbox func_800283D0 --disable all = 28, build_insns 215 == target 215. src/code6cac_b.c restored to HEAD (INCLUDE_ASM) at session end; candidate.c body unchanged (only its header comment gained an s5 note).

- [s5] BB2_ALLOC_DEBUG=1 on tools/gcc-2.7.2/cc1 prints the post-sort global allocno table (global.c:605) and the regs_used_so_far seed (global.c:379). It confirms every number the ledger has carried since s2 and should replace hand-derivation from .lreg/.greg summary lines for this and every future function. Harness: tmp/grind/func_800283D0/s5/alloc.sh.

- [s5] $s0-$s7 are absent from the ALLOCDBG seed, so callee-saved assignment happens entirely in find_reg's pass 1 walking hard regs ascending; hard_reg_copy_preferences / hard_reg_preferences / regs_someone_prefers (global.c:1096-1163) are all empty for this function because no callee-saved hard register appears in pre-reload RTL. The preference axis is inert - the callee-saved map is a pure function of the priority order.

- [s5] The REF-COUNT axis for the callee-saved cluster is foreclosed by target's own asm: $s2 7 refs, $s3 3, $s4 6, $s5 3 - identical to our pseudos 73/143/75/90 (asm/funcs/func_800283D0.s lines 6/95/100/113/118/164/169, 107/108/122, 15/22/57/77/132/177, 64/76/175).

- [s5] The s2 ledger's 'live_length(143) must be 20 or 21' is superseded: it held L75 fixed at 88, but insns inside 143's range also lengthen 75's. They do not lengthen 73's, because arg1 dies at the equal-arm's second func_80032854 call - so the window between that call and the `== 5` load is one-sided and the flip window is k in [6,10], with 5 insns of slack.

- [s5] Liveness dial measured (tmp/grind/func_800283D0/s5/dial.sh): k=6/8/10 added insns each flip the whole callee-saved map to target's (73->$s2, 143->$s3, 75->$s4); k=2/4 too few; k=12 breaks it the other way.

- [s5] Dial-harness gotcha worth reusing on any function: a same-global store chain gets constant-folded and a repeated-constant store chain gets dead-stored, both silently pinning live length regardless of n. Use stores to DISTINCT addresses for a linear 2-insn-per-statement dial.

- [s5] Target has FOUR selection sites, not two: the ==0 arm at .L80028510 (stores and falls through into the call pair), the `<` arm at .L80028608 (jumps INTO the ==0 arm's 0xB label .L80028518 and its call block .L80028520), and .L800285CC + .L800285F8 which are BYTE-IDENTICAL four-insn blocks both jumping to .L800286F8 and left unmerged by jump2 - confirming s2's 'the pairing was never attempted' against emitted target code rather than by inference.

- [s5] Every source-structural route to defeating the jump2 selection pairing is now spent: distinct terminator (s2 H7b), two goto labels (s3 M), ternary (s3 L), no-shared-label (s5 H18). The only remaining route is the jump_chain / INSN_UID(JUMP_LABEL) < max_uid ordering, which needs a print inside tools/gcc-2.7.2/jump.c - outside a grind session's allowed surface.
