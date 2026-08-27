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

## Session 6 (2026-08-26, synthesis) — merged attack

- [s6] CHASSIS re-verified at dispatch: the s2/s5 candidate body applied to src/code6cac_b.c
  measures `sandbox --disable all` = **28**, build_insns 215 == target_insns 215. The
  ALLOCDBG table is byte-identical to s5's (72: 19 refs/155/pri 4903 -> $s0; 77: 9/73/3698 ->
  $s1; 143: 3/14/2142 -> $s2; 73: 7/92/1521 -> $s3; 75: 6/88/1363 -> $s4; 90: 3/32/937 -> $s5;
  79: 6/322/372 -> $s6). Trace banked at tmp/grind/func_800283D0/s6/alloc_trace.txt.

- [s6] FULL CLUSTER MAP re-derived from a fresh pairdiff (tmp/grind/func_800283D0/s6/pd_base.txt,
  32 differing instructions). The ledger has carried a three-cluster picture since s3; the
  correct decomposition at the current chassis is FIVE clusters:
  | cluster | diffs | emitted slots | pass |
  |---|---|---|---|
  | A  s2/s3 callee-saved rotation | 12 | 3,4,10,85,90,96,97,102,107,111,148,153 | global.c allocno_compare |
  | B  return-1 exit-block sharing | 4 | 45,46,47 (insert+delete) | jump2 block placement -> reorg.c fill |
  | C  v0/v1 rename, D_800A38A8 block | 4 | 126,132,133,135 | downstream of D |
  | D  diamond-2 selection order | 4 | 138,139,140,141 | jump2 find_cross_jump |
  | E  tail a0/a1 quantity order | 7 | 159,161,168,169,171,178,181 | local-alloc.c qty_compare_1 |
  Cluster B has been logged since s1 as "4 near-neutral j/nop reorg-fill diffs, possibly
  downstream of diamond 2" and never probed. It is neither near-neutral nor downstream of
  diamond 2 (see below), and it is now the cheapest live axis on the function.

- [s6] CLUSTER B ATTRIBUTED (H21). Target's `.L80028488` (asm/funcs/func_800283D0.s:48-50) is a
  two-instruction block `j .L80028700` / `addiu $v0,$zero,0x1` carrying a LABEL with three
  branch predecessors (lines 17, 24, 26 — the `temp_v1 == 4`, `temp_v0 == 4` and
  `temp_v0 == 0x14` early exits) plus fallthrough from the last rejection-chain `beq`. Because
  the block is labelled and multiply-reached, reorg cannot sink its `addiu $v0,1` into the
  preceding `beq`'s delay slot, so target emits `beq / nop` then `j / li v0,1`. OUR build emits
  the identical two instructions at the identical position but UNLABELLED (only the chain
  fallthrough reaches them), so reorg's eager fill legally sinks `li v0,1` into the beq slot and
  leaves the `j` a `nop`. Target ALSO keeps the `move v0,s6` convergence block
  (`.L800286FC`, s.213-225) for the block_48/block_49 tails — exactly as we do. The divergence
  is therefore a SOURCE fact: the original's three early exits return the literal 1, ours
  return `ret`, so ours cross-jump into the `move v0,s6` block instead of forming a second,
  constant-1 exit block.

- [s6] CLUSTER B, both constant-1 spellings measured and KILLED (H22). v4 (`block_13:
  return ret;` -> `return 1;`) and v5 (one shared `ret_one:` label sited at the chain
  fallthrough, reached by `goto ret_one;` from block_13 and from the temp_v0 exits) BOTH
  measure 34 / **216** insns. In each case jump2 cross-jumps the two return-1 tails and keeps
  the LATE copy: the chain's last `beq v1,v0,<block_15>` inverts to `bne v1,v0,<end>` with
  `li v0,1` in its delay slot and the shared exit block relocates to the function end, costing
  one instruction. The content of the hypothesis is right (target does want a constant-1 block);
  the open question is PLACEMENT, and it is the same jump_chain-ordering question as diamond 2.
  Banked: rejected/early-exit-return-const1-relocates-shared-block.c and
  rejected/shared-ret-one-label-relocates-shared-block.c.

- [s6] H19 KILLED — the last escape hatch under s5's H17 is closed with a mechanism.
  s5 refuted "some ordinary-C spelling can supply the 6-10 insns of liveness the s2/s3 flip
  needs" by counting EMITTED instructions between the equal arm's second call and the `== 5`
  load. That comparison is formally invalid: `reg_live_length` is computed by flow.c BEFORE
  reload, and `final` silently drops any set whose SRC == DEST after allocation, so insns could
  in principle exist at flow time and be byte-invisible. Measured: two byte-neutral
  named-intermediate spellings inside pseudo 143's live range (`s16 id1/id2` carrying the two
  calls' first argument; `u8 *p1 = arg1` carrying the third) leave EVERY ALLOCDBG row
  byte-identical — cse/combine delete the copies before life_analysis ever runs. Under GCC
  2.7.2 there is no coalescing pass that could preserve them that far, so in OUR RTL any insn
  that raises live_length is also an emitted insn, and adding 6 of them breaks 215 == 215.
  s5's H17 verdict stands, now on a mechanism rather than an instruction count.

- [s6] NEW: the s2/s3 window has a SECOND solution branch (H20) that five sessions have missed.
  Every prior derivation (s2 H9, s5 H16) solved `pri(143) < pri(73)` for L143, giving the narrow
  and now-foreclosed "raise L143 from 14 to 20-24" route. But target's map only requires the
  SORTED ORDER 72, 77, 73, 143, 75, 90, 79, and that order also holds if pri(73) rises ABOVE
  pri(143) while staying below pri(77): 2142 < 140000/L73 < 3698, i.e. **L73 in [38, 65]**
  (currently 92). That is a 28-value window instead of a 3-value one, and — decisively — it is
  satisfied by REMOVING arg1 liveness rather than by adding instructions, so it is not blocked
  by the 215 == 215 budget that killed the L143 route. Untried; it is frontier item 2 for s7.

- [s6] Chassis re-verified at dispatch: the s2/s5 candidate body applied to src/code6cac_b.c measures sandbox --disable all = 28 with build_insns 215 == target_insns 215; ALLOCDBG table byte-identical to s5's.

- [s6] The residual is FIVE clusters, not the three the ledger has carried since s3. Fresh masked pairdiff = 32 differing instructions: A s2/s3 callee-saved rotation 12 diffs (slots 3,4,10,85,90,96,97,102,107,111,148,153, global.c allocno_compare); B return-1 exit-block sharing 4 diffs (slots 45-47, jump2 block placement then reorg fill); C v0/v1 rename in the D_800A38A8 block 4 diffs (126,132,133,135); D diamond-2 selection order 4 diffs (138-141, jump2 find_cross_jump); E tail a0/a1 quantity order 7 diffs (159,161,168,169,171,178,181, local-alloc qty_compare_1).

- [s6] Cluster B is a SOURCE fact, not a compiler wobble: the original's three early exits (temp_v1 == 4, temp_v0 == 4, temp_v0 == 0x14) return the literal 1 into a shared labelled block at the rejection chain's fallthrough; ours return `ret` and cross-jump into the `move v0,s6` block, leaving the constant-1 tail unlabelled and letting reorg sink its `li v0,1` into the preceding beq's delay slot.

- [s6] Both constant-1 respellings of the early exits measure 34/216 - jump2 keeps the late copy of the merged block and inverts the chain's last beq. Banked at memory/grind/func_800283D0/rejected/early-exit-return-const1-relocates-shared-block.c and rejected/shared-ret-one-label-relocates-shared-block.c.

- [s6] The named-intermediate route to extra flow-time liveness is closed by measurement: cse/combine delete the copies before life_analysis, so every ALLOCDBG row is unmoved. In our RTL any insn that raises live_length is also an emitted insn, and the 215==215 budget therefore genuinely forecloses the 'raise L143 to 20-24' branch of the s2/s3 window (s5 H17 stands, now with a mechanism).

- [s6] NEW window for the s2/s3 rotation: target's map is just the sorted allocno order 72,77,73,143,75,90,79, which also holds when pri(73) rises above pri(143)=2142 while staying below pri(77)=3698 - i.e. L73 in [38,65] against its current 92. Unlike the L143 branch this needs liveness removed, not instructions added, so the insn budget does not foreclose it.

- [s6] Body left in src/code6cac_b.c is the unchanged s2/s5 candidate; the only delta against HEAD is the single INCLUDE_ASM line for func_800283D0.

## 2026-08-26 — session 7 (solver)

Body: the s2-s6 candidate, re-applied to src/code6cac_b.c from
`tmp/grind/func_800283D0/s5/base_body.c`. Chassis re-measured this session:
**sandbox --disable all = 28, build_insns 215 == target_insns 215**. src was restored to
exactly this body at session end.

### 1. Layer triage (run FIRST, per the solver playbook)

`goal_from_tgt.py classify code6cac_b func_800283D0` (object-level; the text path via
`mkasm_honest.sh` cannot be used now that src carries our C body — its "target" half would
rebuild OUR code). Verdict: **PRE-RA**, with the entire pre-RA delta being a single
instruction shape:

    ours only  : beqz #,@   x1
    target only: bnez #,@   x1

Alignment of the two 215-insn streams: equal 187, replace 24, moved 4, delete 0, insert 0.
So the residual is fully accounted for as **1 branch-sense instruction + 4 scheduler moves +
21 register renames + 2 pairs whose skeletons differ on an immediate/reloc**. There is no
hidden instruction-selection difference anywhere else in the function.

The 21 renames, with their emitted slots:

| slots | substitution | cluster |
|---|---|---|
| 4, 85, 90, 96, 97, 102, 107, 111, 148, 153 | `$s3<->$s2` (x7 / x3) | A — s2/s3 rotation |
| 126, 132, 133, 135 | `$v1 -> $v0` (x4) | C — v0/v1 rename |
| 159, 161, 168, 169, 171, 178, 181 | `$a1<->$a0` (x6 / x2) | E — tail quantity order |

Slot 96 is `ours: addu s2,s0,v0` vs `tgt: addu s3,v0,s0`. The tool reads that as
`$s2->$s3, $s0->$v0, $v0->$s0`; the last two are an artefact of `addu`'s commutative operand
ORDER, not a rename (s1 already banked rejected/addu-commutativity-laundered.c for that
operand order). Do NOT put `72: $v0` in a solver goal — it is fiction, and the honest goal for
the callee-saved map is the 73/143 exchange with 72,77,75,90,79 held fixed.

### 2. The global cluster (A): a 73<->143 exchange, and NOTHING else can reach it

Model: `extract.py func_800283D0 code6cac_b` -> 25 allocnos.
Baseline callee-saved map `72:$s0 77:$s1 143:$s2 73:$s3 75:$s4 90:$s5 79:$s6`;
target's is `72:$s0 77:$s1 73:$s2 143:$s3 75:$s4 90:$s5 79:$s6`.
Identities: **pseudo 73 = `arg1`** (its 7 references are the prologue `move s3,a1` plus the six
`move a2,s3` argument set-ups); **pseudo 143 = `temp_s3` = `arg0 + temp_a1_2*2`** (RTL insn 277,
`(set (reg/v:SI 143) (plus (reg/v:SI 72) (reg:SI 145)))`; its 3 references are that `addu` and
the two `lh ...,648(...)` loads).

Model inputs: 73 `nrefs=7 livelen_flow=92 reg_live_length=132 calls_crossed=3`;
143 `nrefs=3 livelen_flow=14 reg_live_length=22 calls_crossed=2`.

An EXHAUSTIVE single-atom sweep of the validated forward model
(`simulate.Sim`, every allocno x livelen 1..250 x nrefs 1..24 —
`tmp/grind/func_800283D0/s7/sweep_all.py`) finds vectors on **exactly two** allocnos:

| dial | base | flips at |
|---|---|---|
| `livelen(73)` | 92 | **38 .. 65** |
| `livelen(143)` | 14 | **20 .. 21** (i.e. +6 or +7 — and +8 already overshoots) |
| `nrefs(73)` | 7 | **8 .. 11** |
| `nrefs(143)` | 3 | **2** |

Every other allocno is inert at every value. That is a mechanical closure: the callee-saved
cluster can only be attacked through `arg1`'s liveness/ref count or `temp_s3`'s.

The JOINT map (`tmp/grind/func_800283D0/s7/joint.py`) — the part no prior session had — shows a
diagonal region, so partial moves compose:

    L143 =  10 11 12 13 14 15 16 17 18 19 20 21
    L73 upper bound = 46 51 56 60 65 70 74 79 84 88 93 98   (lower bound is a constant 38)
    R73 = 5/6/7/8/9/10/11/12 -> L73 in [28,46]/[33,56]/[38,65]/[65,112]/[74,126]/[82,140]/[90,154]/[98,168]
    R143 = 2 -> L143 in {14};  R143 = 3 -> L143 in {20,21};  R143 = 4 -> L143 in [53,58]

Two consequences worth carrying forward:
* base `L73=92` lies INSIDE the `R73=8` band `[65,112]`, which is why "one more reference to
  `arg1`" is a single-atom solution at today's live length.
* the `L143` window is only +6/+7 wide. **`inverse.py global` returns NO livelen vector for this
  goal — that is a tool artefact**: its search bounds are "live length +/-2,4,8" and the true
  window sits in the 6/7 hole. Sweep `simulate.Sim` directly before crediting an inverse.py
  negative on live length.

### 3. The tail cluster (E) is LOCAL alloc — participants named, window measured

Named from the `.lreg` RTL rather than guessed: `(reg/v:SI 184)` is the tail pointer
`temp_s4 + temp_s5*0x10` (uses: `+276`, `+280`, `+284`), `(reg:SI 186)` its `ashift 4` producer,
and `(reg:SI 201)` / `(reg:SI 211)` the two `(&Judge)[...]` elements. **None of 184/186/201/211 is
in the global allocno order**, so this cluster belongs to `local-alloc.c`, confirming s3's
routing (and refuting the alternative reading that 191/192 were the participants — those are
different, global, pseudos).

`local_extract.py code6cac_b`, block 41 (`main` pass) baseline:

    ord 0 qty 2 reg198 birth10 death16 refs6 -> $v0      ord 5 qty 3 reg201 birth16 death20 refs2 -> $a0
    ord 1 qty 0 reg208 birth 4 death26 refs9 -> $v1      ord 6 qty 7 reg211 birth26 death30 refs2 -> $v1
    ord 2 qty 4 reg203 birth18 death20 refs2 -> $v0      ord 7 qty 0 reg184 birth 2 death32 refs6 -> $a1
    ord 3 qty 8 reg213 birth28 death30 refs2 -> $v0
    ord 4 qty11 reg215 birth38 death40 refs2 -> $v0

Target wants qty0 (reg 184) in `$a0` and qty3 (reg 201) in `$a1`.
`inverse.py local --block 41 --swap 0,3 --depth 2 --top 40` enumerates the COMPLETE
single-atom vector set:

* `span(qty0)` 30 -> **<= 24**, reachable from either end (birth 2 -> 8..19, or death 32 -> 16..26)
* `refs(qty0)` 6 -> **8, 9 or 10**

and nothing else. s3's hand-derived `span(qty0) <= 24` is CONFIRMED; s3's alternative
`span(qty3) >= 6` is **REFUTED** — no qty3-only perturbation reaches the goal.
(Local-mode caveat, printed by the tool and repeated here: a birth/span vector is a claim about
ALLOC-TIME order, which is not known to equal emission order — treat it as NECESSARY, not
sufficient, until re-derived from a QTYDBG dump of the actual candidate. This session did
exactly that for variant B below.)

### 4. First C lever that moves the local dial (partial, byte-neutral)

Variant **B_ptr_late** — a pure declaration reorder inside the `var_s1 == 0` tail block, putting
`s32 temp_v1_4 = -*(s16 *)(arg0 + 0x1CA);` BEFORE `u8 *temp_a0 = temp_s4 + (temp_s5 * 0x10);`
(no new locals, no new statements):

    sandbox --disable all: 28 / 215   (unchanged)
    block 41 QTYDBG:       the pointer quantity becomes qty1 / first_reg 188,
                           birth 6 (was 2), death 32, span 26 (was 30), got $a1

So the reorder buys **4 of the 6 insns** the window needs, and the assignment has not flipped.
This is the first measured, directed movement of cluster E's dial. The remaining 2 insns must
come from something else that can honestly precede the pointer in block 41. The variant source
is regenerable from `tmp/grind/func_800283D0/s7/probe.py B_ptr_late`.

The opposite end of the window is now closed: variants **A_abs_first** and **C_both** (hoisting
the `temp_a0_2` abs above the `temp_v1_5` product, so the pointer's last use dies earlier) both
measure **44, at 214 insns** — one instruction FEWER than target — banked as
rejected/tail-abs-hoisted-above-product-loses-insn.c. With s3's
tail-abs-sunk-below-shift-adds-insn.c (31/216) that pins the abs exactly where it is and kills
the death-side approach to `span(qty0) <= 24`.

### 5. Reconciliation with the s5/s6 ledger

s5 recorded the global window as "6-10 insns of extra live length inside pseudo 143's range".
The exhaustive sweep says **+6 or +7 and no more** (L143 14 -> 20/21); +8 already overshoots.
s5's "the ref-count axis is FORECLOSED" was reasoned from target's asm carrying our reference
counts. That remains the right reading for what the FINAL asm shows, but the solver's `R73`
vector (7 -> 8..11) is about `reg_n_refs` as `flow.c` counts it BEFORE combine/sched1/jump2 —
a reference that a later pass folds or cross-jump-merges away is counted here and invisible
there. Whether such a reference can be spelled honestly is untested; it is the one live
reading under which the ref axis is NOT foreclosed.

- [s7] Chassis re-measured this session with the s2-s6 candidate body applied to src/code6cac_b.c: sandbox --disable all = 28, build_insns 215 == target_insns 215. src restored to exactly that body at session end (only delta vs HEAD is func_800283D0's INCLUDE_ASM line).

- [s7] Layer triage verdict PRE-RA, and the whole pre-RA delta is ONE instruction: ours `beqz`, target `bnez`. Stream alignment equal 187 / replace 24 / moved 4 / delete 0 / insert 0 over 215 vs 215.

- [s7] Cluster map with emitted slots: A (s2/s3) at slots 4,85,90,96,97,102,107,111,148,153; C (v1->v0) at 126,132,133,135; E (a1<->a0) at 159,161,168,169,171,178,181.

- [s7] Slot 96 `ours: addu s2,s0,v0` vs `tgt: addu s3,v0,s0` is read by the tool as $s2->$s3 PLUS $s0->$v0/$v0->$s0; the last two are an artefact of addu's commutative operand ORDER, not renames. Do NOT put 72:$v0 into a solver goal - it is fiction. The honest global goal is the 73/143 exchange with 72,77,75,90,79 fixed.

- [s7] Pseudo identities, read from the .lreg RTL: 73 = arg1; 143 = temp_s3 = arg0 + temp_a1_2*2 (insn 277); 184 = the tail pointer temp_s4 + temp_s5*0x10; 186 = its ashift-4 producer; 201/211 = the two (&Judge)[...] elements. 184/186/201/211 are absent from the global allocno order, so cluster E is local-alloc, not global.

- [s7] Global model inputs: 73 nrefs=7 livelen_flow=92 reg_live_length=132 calls_crossed=3; 143 nrefs=3 livelen_flow=14 reg_live_length=22 calls_crossed=2. Baseline map 72:$s0 77:$s1 143:$s2 73:$s3 75:$s4 90:$s5 79:$s6; target 72:$s0 77:$s1 73:$s2 143:$s3 75:$s4 90:$s5 79:$s6.

- [s7] Exhaustive single-atom windows for the global exchange: L73 -> [38,65]; L143 -> [20,21]; R73 -> [8,11]; R143 -> 2. Only allocnos 73 and 143 have any vector; all 23 others are inert at every value.

- [s7] Joint window map: L143 = 10..21 pairs with L73 upper bounds 46,51,56,60,65,70,74,79,84,88,93,98 (lower bound constant at 38); R73 = 5..12 pairs with L73 ranges [28,46],[33,56],[38,65],[65,112],[74,126],[82,140],[90,154],[98,168]; R143 = 2 -> {14}, 3 -> {20,21}, 4 -> [53,58].

- [s7] Tool artefact banked: inverse.py global's search bounds are 'live length +/-2,4,8', so it reports NO livelen vector for this goal even though L143 +6/+7 solves it. Sweep simulate.Sim directly (sweep_all.py) before crediting an inverse.py negative on live length.

- [s7] Local-alloc block 41 baseline (main pass): qty2/reg198 b10 d16 r6 ->$v0; qty0/reg208 b4 d26 r9 ->$v1; qty4/reg203 b18 d20 r2 ->$v0; qty8/reg213 b28 d30 r2 ->$v0; qty11/reg215 b38 d40 r2 ->$v0; qty3/reg201 b16 d20 r2 ->$a0; qty7/reg211 b26 d30 r2 ->$v1; qty0/reg184 b2 d32 r6 ->$a1.

- [s7] Complete single-atom vector set for the E exchange (qty0<->qty3): span(qty0) 30 -> <= 24 (birth 2->8..19 or death 32->16..26), or refs(qty0) 6 -> 8/9/10. Nothing else. s3's 'span(qty3) >= 6' is refuted.

- [s7] B_ptr_late (declare temp_v1_4 before temp_a0; pure declaration reorder) measures 28 / 215 and moves the pointer quantity to birth 6 / span 26 - 4 of the 6 insns the local window needs, verified from a fresh QTYDBG dump rather than from the model's prediction.

- [s7] A_abs_first and C_both (abs hoisted above the temp_v1_5 product) both measure 44 at 214 insns - one insn short of target. With s3's abs-sunk variant (31/216) the abs is pinned and the death-side route to span(qty0) <= 24 is closed.

- [s7] Ledger correction: s5's global window 'L143 +6-10 insns' is really +6 or +7 exactly. s5's 'ref-count axis FORECLOSED' holds for what the FINAL asm shows, but reg_n_refs is counted by flow.c BEFORE combine/sched1/jump2, so a reference later folded or cross-jump-merged away would be counted there and invisible in target's asm - the one reading under which R73 7->8 is not foreclosed.


## Session 8 (2026-08-26, forensics) - measured

Chassis re-measured at session start with the s2..s7 candidate body applied:
sandbox --disable all = 28, build_insns 215 == target 215.  At session end the
best measured form is **25 / 215** (see below); src/code6cac_b.c was restored to
HEAD's INCLUDE_ASM at session end, and the winning body is in candidate.c.

### Method note - a per-function XJDBG log is now obtainable
The instrumented cc1's BB2_XJUMP_DEBUG output is whole-TU and carries no
function boundaries, and insn UIDs restart per function, so a whole-TU log
cannot be attributed.  tmp/grind/func_800283D0/s8/isolate.py builds
tmp/grind/func_800283D0/s8/iso.c - the same TU with every OTHER function body
emptied to `{ }` - and s8/dumpiso.sh compiles it with the instrumented cc1.
VALIDATED: the func_800283D0 segment of the .jump2 RTL dump from iso.c is
identical to the full-TU dump except for CODE_LABEL_NUMBER values (the TU-global
.LNN counter).  So the iso log is exactly this function's trace.  Reuse this for
any env-gated cc1 hook (QTY/ALLOC/XJUMP) that prints to stderr without a
function tag.

### FACT 1 (pass attribution correction, supersedes the s6/s7 framing)
The cross-jump pass is the LAST jump pass and runs POST-RA and POST-sched2:
tools/gcc-2.7.2/toplev.c:3141 `jump_optimize (insns, 1, 1, 0)` sits after the
sched2 dump block and immediately before the .jump2 dump and reorg (.dbr).
Consequences, all binding on future sessions:
  - Cross-jump merges (clusters B and D) are decided on RTL in which every
    pseudo has already been replaced by its HARD register.  No local-alloc,
    global-alloc, live-length, ref-count or scheduling lever can change a
    cross-jump outcome, and no cross-jump lever can change an allocation.
    Clusters A/E and clusters B/D are therefore INDEPENDENT sub-problems.
  - s7's frontier item 3 ("one instrumented .jump2 read settles clusters B, C
    and the beqz/bnez insn together") is REFUTED as stated: the beqz/bnez sense
    at slot 138 is produced by RTL expansion of the source's branch shape, not
    by find_cross_jump's choice of surviving copy.

### FACT 2 (the 4-insn merge, with its exact arithmetic)
With BOTH var_v0_2 selection copies spelled canonically (`v = 0x19;
if (!var_s1) v = 0xB;` - the pre-s2 body), the trace for the tail copy's
`j block_48` is:
    XJDBG: enter e1=362 e2=397 min=2 (chain-partner)
    XJDBG:   MATCH i1=358 i2=393 set(reg<-11) min->1
    XJDBG:   MATCH i1=354 i2=389 set          min->0
    XJDBG:   MATCH i1=351 i2=386 set(reg<-25) min->-1
    XJDBG:   LABEL-BONUS i1=347 (label) min->-2; break
    XJDBG: result e1=362 min=-2 last1=351 => WIN
    XJDBG: DO_CROSS_JUMP jump=362 newjpos=351 newlpos=386
Reading: find_cross_jump starts at minimum=2 for a jump_chain partner (1 for the
own-label form), subtracts 1 per matching insn and 1 more if the backward walk
reaches a CODE_LABEL in stream 1, and the merge fires when the total is <= 0.
All three insns of the block match - including the two conditional branches,
whose patterns are equal because they test the same hard register and reference
the same label.  The merge therefore has THREE units of slack: shortening the
match by one or two insns is not enough; the walk must fail on the FIRST
compared insn.

### FACT 3 (the lever, and the 28 -> 25 measurement)
find_cross_jump compares patterns with rtx_renumbered_equal_p, which compares
GET_MODE as well as register numbers.  Because var_v0_2 is `s16`, BOTH copies'
constant loads are `(set (reg/v:HI 2 v0) (const_int 25))`, insn code 163
movhi_internal2 (verified in the .sched2 dump, uids 351/358 and 386/393).
Routing the TAIL copy's selection through an s32 intermediate makes its two
constant loads `(set (reg:SI 2 v0) ...)` / movsi_internal2 - a DIFFERENT RTL
pattern that assembles to the IDENTICAL byte (`addiu $v0,$zero,0x19`).  Result:
PAT-MISMATCH on the first compared insn; both copies survive.
MEASURED: 25 / 215 (from 28 / 215).  Cluster D is CLOSED - emitted slots 127-130
and 138-141 now match target exactly, including the `bnez $s1` sense and the
canonical 0x19-then-0xB constant order that the s2 if/else spelling could never
produce.  This is the first floor movement since session 2.

### FACT 4 (the sanctioned-family spelling of the same lever is dead)
Borrowing the existing s32 local `d_val` instead of a fresh intermediate
measures 33 / 216: reload does not coalesce the resulting copy (d_val's pseudo
carries conflicts from its real earlier live range), so the move survives as a
216th instruction.  Banked as
rejected/reuse-dval-selection-holder-no-coalesce.c.  The variable-reuse family
therefore cannot express this lever on this function; only a fresh,
conflict-free intermediate coalesces.

### FACT 5 (residual at 25, re-derived from the emitted diff)
Remaining slot-level differences after the mode split (normalized diff produced
by tmp/grind/func_800283D0/s8/diffasm.sh + ours.txt / target.txt):
  - cluster A, s2/s3 callee-saved rotation: slots 3, 4, 10, 85, 90, 96, 102,
    107, 148, 153 (slot 96 also shows an addu operand order difference).
  - cluster B, return-1 exit-block sharing: slots 45 and 47 (target `nop` /
    `addiu $v0,1`, ours `li v0,1` / `nop`).
  - cluster C, v0/v1 rename in the D_800A38A8 / D_800A3876 block: slots 126,
    132, 133, 135.
  - cluster E, tail a0/a1 quantity order: slots 159, 161, 168, 171.
Cluster D no longer appears.

### OPEN CLASSIFICATION QUESTION (do not spend it silently)
`s32 sel = 0x19; if (!var_s1) sel = 0xB; var_v0_2 = sel;` is a fresh local,
written twice, read once, carrying a real consumed value, byte-neutral.  It does
NOT fit the named-intermediate family (whose once-written / once-read prong
excludes a multi-write local) and no other frozen SOTN family covers it; the
variable-reuse spelling is measured dead (FACT 4).  Its mechanism is a named GCC
pass (jump.c find_cross_jump / rtx_renumbered_equal_p mode comparison), which is
exactly the T3 signal.  A session that wants to submit this must first obtain a
ruling, or find a spelling of the same mode split that a human would write for
program reasons.

- [s8] Chassis at session start with the s2..s7 candidate body applied: sandbox --disable all = 28, build_insns 215 == target 215 (HEAD itself carries INCLUDE_ASM, which is why the driver measured no floor at dispatch).

- [s8] PASS ATTRIBUTION: the cross-jump pass is the LAST jump pass, toplev.c:3141 jump_optimize(insns,1,1,0), running after sched2 and after local/global allocation + reload and immediately before reorg. Cross-jump merges therefore compare POST-RA hard-register RTL: clusters B and D cannot be moved by any allocation, live-length, ref-count or scheduling lever, and no cross-jump lever can move clusters A or E. The two sub-problems are independent.

- [s8] The merge costing 4 insns is exactly one decision: e1=362 vs jump_chain partner e2=397, minimum 2, three MATCHes (both constant loads AND the two conditional branches, whose patterns compare equal because they test the same hard register and reference the same label) plus a LABEL-BONUS, final min -2.

- [s8] The compared RTL, read from the .sched2 dump: (insn 351 (set (reg/v:HI 2 v0) (const_int 25)) 163 {movhi_internal2}) and its twin (insn 386 ...) - HImode because var_v0_2 is declared s16.

- [s8] MEASURED 25 / 215 with the tail copy routed through an s32 intermediate; MEASURED 33 / 216 with the same lever spelled as a borrow of the existing s32 local d_val; MEASURED 28 / 215 for the s2..s7 body and (from the trace) the canonical-both body merges to 211.

- [s8] Residual at 25, by emitted slot: cluster A (s2/s3 rotation) slots 3, 4, 10, 85, 90, 96, 102, 107, 148, 153; cluster B (return-1 exit block) slots 45, 47; cluster C (v0/v1 in the D_800A38A8 / D_800A3876 block) slots 126, 132, 133, 135; cluster E (tail a0/a1) slots 159, 161, 168, 171. Cluster D is gone.

- [s8] CLASSIFICATION IS OPEN on the 25-floor construct: 's32 sel' is a fresh local, written twice, read once, carrying a real consumed value, byte-neutral. It does not fit named-intermediate (that family's once-written / once-read prong excludes a multi-write local), the variable-reuse spelling is measured dead, and its mechanism is a named GCC pass - so it must not be submitted as candidate-ready without a ruling.

- [s8] METHOD: per-function attribution of the whole-TU BB2_XJUMP_DEBUG stream is now reproducible via tmp/grind/func_800283D0/s8/isolate.py (other function bodies emptied), validated by the .jump2 RTL segment being identical to the full-TU dump apart from CODE_LABEL_NUMBER.

- [s8] src/code6cac_b.c was restored to HEAD (INCLUDE_ASM) at session end; the 25-floor body lives in memory/grind/func_800283D0/candidate.c.


## Session 9 (2026-08-26, forensics) - floor 25 -> 23

Chassis re-measured at session start with the s8 body installed: 25 / 215, matching the
ledger, so every s8 conclusion is chassis-current.

### 1. The s8 classification blocker is gone, at zero byte cost
s8 closed cluster D with `{ s32 sel = 0x19; if (var_s1 == 0) sel = 0xB; var_v0_2 = sel; }`
and left the session flagged DO-NOT-SUBMIT because a fresh, MULTI-WRITE local fits no frozen
SOTN family.  s9 re-spelled it as a single assignment:

    { s32 sel = (var_s1 == 0) ? 0xB : 0x19; var_v0_2 = sel; }

25 / 215, and the emitted stream is byte-identical to s8's (all 215 lines equal).  The
intermediate is now fresh, once-written, once-read, int-typed, and carries a real consumed
value - the shape the family-selection table calls named-intermediate.  s9 did not write a
self-vet (the floor is not 0), but the form a future session must vet is this one.

Three neighbouring spellings were measured and are dead:
  * `(var_s1 != 0) ? 0x19 : 0xB` and `var_s1 ? 0x19 : 0xB` DO defeat the merge (215 insns)
    but invert the branch sense: `beqz $s1 / li 0xB / j / li 0x19` vs target's
    `bnez $s1 / li 0x19 / j / li 0xB`.  28.  The C arm order maps onto the emitted sense.
  * The same ternary written straight into the s16 `var_v0_2`, with no intermediate:
    30 / 211 - the conversion to short is folded into the selection, the constant loads are
    HImode again, and find_cross_jump merges the copies.
  * Declaring `var_v0_2` itself as `s32`: 30 / 211 - both copies become SImode and match
    each other again.  The lever is the mode SPLIT between the copies, not SImode.

### 2. The structural route to defeating the cross-jump is closed (instrumented)
The obvious no-mode-trick alternative is to stop the two selection copies from sharing the
block_48 tail: give one of them its own `*(s16 *)(arg0 + 0x286) = ...; return ret;`.  Four
spellings measured 29-30 at 211 insns - the 4-insn merge is back in every one.  Re-compiling
the ==5-direct variant with the instrumented cc1 (BB2_XJUMP_DEBUG, via the s8
isolate.py/dumpiso.sh recipe; log at tmp/grind/func_800283D0/s9/xjump_P2.log) shows why:

    XJDBG: enter e1=372 e2=407 min=2 (chain-partner)
    XJDBG:   MATCH i1=360 i2=403 set(reg<-11) min->1
    XJDBG:   MATCH i1=356 i2=399 set          min->0
    XJDBG:   MATCH i1=353 i2=396 set(reg<-25) min->-1
    XJDBG:   LABEL-BONUS i1=347 (label)       min->-2; break
    XJDBG: DO_CROSS_JUMP jump=372 newjpos=353 newlpos=396

The duplicated store+return tail has already been tail-merged back into block_48 by an
earlier jump pass, so by the time jump2's find_cross_jump looks, the two blocks are identical
again.  This closes the axis the ledger has speculated about since s2: the RTL-mode split is
the ONLY known lever for cluster D.

### 3. Cluster C attributed to sched1, then closed
Cluster C is the 4-diff v0-vs-v1 rename at emitted slots 126/132/133/135 (the
`D_800A38A8 = 1; D_800A3876 = -1;` stores).  Dumps regenerated for the 25-floor body with
`pwsh tools/grinder/dump.ps1 func_800283D0`:

  * .combine:2894-2918 and .flow:4370-4394 - the block, PRE-sched1, is
        368 (set (reg:HI 170) (const_int 1))
        370 (set (mem:HI D_800A38A8) (reg:HI 170))
        373 (set (reg:HI 171) (const_int -1))
        375 (set (mem:HI D_800A3876) (reg:HI 171))
        378 (set (reg/i:SI 2 v0) (reg/v:SI 79))     <- the `return ret;` value copy, LAST
  * .sched:4853 - sched1 HOISTS 378 to the FRONT: 378, 368, 370, 373, 375.
  * .greg:3150 - consequently `(set (reg:HI 3 v1) (const_int 1))`: v0 is live across the
    whole block, so local-alloc gives the constants v1.  Target's block has no such copy at
    its head and its constants take v0.

That names the pass (sched1) and the decision (the hoist of the return-value copy).  Two
in-block levers were measured and are dead: swapping the two stores is byte-neutral at 25
(and WORSE, 24, on the 23-floor body - target stores 1 first, and source order is emitted
order), and `return 1;` instead of `return ret;` measures 26 while leaving 126/132/133/135
completely unchanged (the copy is still there, still hoisted, still holds v0).

The lever that works is DELETING the copy: exit the globals block with `goto block_49;`
instead of an inline `return ret;`.  block_49 is an already-existing shared `return ret;`
label in this body, so this is plain ordinary C.  **25 -> 23**, and slots 131-136 now read
`li v0,1 / lui at / sh v0 / li v0,-1 / lui at / sh v0` - target's registers exactly.
Cluster C is CLOSED.

### 4. The cost: one unfilled delay slot (216 vs 215)
This is the first body in the ledger that is not at build_insns == 215.  A difflib alignment
against target shows the ONLY insertion is a `nop` at slot 126; from 131 onward our stream
re-aligns one slot late against target's 126..135.  On the 25-floor body reorg steals the
branch target's `li v1,1` into the delay slot of `beqz v0,<globals block>` at slot 125.  On
the 23-floor body the same candidate is `li v0,1` - a write of the very register the branch
reads - and reorg declines.  Target performs exactly that steal
(`beqz v0,.L800285DC` + `addiu v0,zero,1` in the delay slot), so it is reachable in this
compiler.  Why ours declines has not been read out of reorg.c / resource.c; that is frontier
item 1.

### 5. Exit-form changes are per-site, not a policy
Applying the same `inline return -> goto block_49` change to the `temp_v1_2 == 0xE` early
return measures 38 (a 15-point regression); applying it to the do_calls return measures 23,
byte-identical to the 23-floor body (jump2 re-merges it).  Together with s1's finding that
the range-check exit wants an INLINE `return 1;`, the exit-form dimension is now known to be
worth several slots per site and to be decided independently per site - which makes the
untried 2^N enumeration over the four early-exit sites a concrete lever for cluster B.

### 6. Symmetry note
Putting the s32 intermediate in the ==5 copy rather than the tail copy emits a byte-identical
stream.  Either placement works; the tail placement is kept in candidate.c.

### Artifacts
tmp/grind/func_800283D0/s9/ - install.py, probe.py, probe2.py, probe3.py, probe4.py,
extract.sh, cmp.py, bank.py, isolate.py, dumpiso.sh, xjump_P2.log (289 XJDBG lines), the
per-variant .c bodies and their extracted instruction streams (sel.txt, Q1.txt, tern_ne.txt,
D4_goto49.txt, ...).  Regenerated pass dumps in tmp/grind/func_800283D0/dumps/ (.combine,
.flow, .sched, .lreg, .greg, .jump2, .dbr).

- [s9] Chassis re-measured at session start with the s8 body installed: 25 / 215, matching the ledger - every s8 conclusion is chassis-current.

- [s9] New honest floor 23 (build_insns 216, target 215), re-verified with the banked candidate.c installed in src/code6cac_b.c at session end.

- [s9] s8's DO-NOT-SUBMIT classification blocker is retired: the mode-split intermediate is now `s32 sel = (var_s1 == 0) ? 0xB : 0x19;` - fresh, once-written, once-read, int-typed, carrying a real consumed value - and emits a stream byte-identical to s8's multi-write spelling (all 215 lines equal). No self-vet was written because the floor is not 0; the form a future session must vet is this one, not s8's.

- [s9] The ternary arm order is load-bearing: `(var_s1 != 0) ? 0x19 : 0xB` and `var_s1 ? 0x19 : 0xB` defeat the merge but invert the branch sense (28 / 215); only `(var_s1 == 0) ? 0xB : 0x19` reproduces target's `bnez $s1 / li 0x19 / j / li 0xB`.

- [s9] Without an intermediate the mode split evaporates: the same ternary assigned straight to the s16 var_v0_2 measures 30 / 211, and declaring var_v0_2 as s32 measures 30 / 211 (both copies SImode, still matching each other).

- [s9] The STRUCTURAL route to defeating the cluster-D cross-jump is closed by instrumented evidence: four direct-store spellings all measure 29-30 at 211 insns, and BB2_XJUMP_DEBUG on the ==5-direct body shows DO_CROSS_JUMP jump=372 newjpos=353 newlpos=396 still firing - an earlier jump pass has already tail-merged the duplicated store+return back into block_48.

- [s9] Cluster C's pass is NAMED from dumps, not guessed: pre-sched1 (.combine:2894-2918, .flow:4370-4394) the globals block ends with insn 378 (set (reg/i:SI 2 v0) (reg/v:SI 79)); .sched:4853 shows sched1 hoisting 378 to the front of the block; .greg:3150 shows the consequence, (set (reg:HI 3 v1) (const_int 1)).

- [s9] Cluster C is CLOSED on the 23-floor body: slots 131-136 emit `li v0,1 / lui at / sh v0 / li v0,-1 / lui at / sh v0`, target's registers exactly.

- [s9] The 216-vs-215 delta is exactly one `nop` at slot 126 - an unfilled delay slot on the `beqz v0,<globals block>` at slot 125. Target fills that slot by stealing `addiu v0,zero,1` from the branch target; reorg declines the equivalent steal for us.

- [s9] Exit-form choice is per-site and worth several slots: the same goto-for-return change measures 38 at the temp_v1_2 == 0xE site and 23 at the do_calls site - which makes an enumeration over the four early-exit sites a concrete, untried lever for cluster B.

- [s9] Placing the s32 intermediate in the ==5 copy instead of the tail copy emits a byte-identical stream; either placement works.

- [s9] candidate.c (23 / 216) and candidate_alt_215insn.c (25 / 215, the 215==215 fallback base) are both banked, with nine new rejected forms under memory/grind/func_800283D0/rejected/.

## s10 (rederive, 2026-08-26) - chassis 23 confirmed; m2c re-derivation and the exit-form dimension both closed

Chassis at dispatch: applying `memory/grind/func_800283D0/candidate.c` (the s9 body) to
src/code6cac_b.c and running `sandbox func_800283D0 --disable all` printed
`score 23, target_insns 215, build_insns 216`. The ledger floor of 23 is live and
unchanged; every s10 measurement below is relative to that body.

### 1. Fresh m2c re-derivation (the mandated modality) - the ledger body is a strict local optimum

`tools/m2c/m2c.py --target mipsel-gcc-c --valid-syntax -f func_800283D0 asm/funcs/func_800283D0.s`
(output banked at tmp/grind/func_800283D0/s10/m2c_fresh.c, 150 lines) reproduces the same
control-flow skeleton the ledger body already has, differing in exactly FIVE structural
choices. Transcribing the whole fresh shape to real typed field accesses - while keeping the
two levers s8/s9 proved load-bearing (the SImode `sel` intermediate in the tail selection and
`goto block_49;` out of the globals block) - gave variant R1 at **65 / 214**, a 42-point
regression. Each delta was then measured in isolation on the 23-floor body:

| variant | m2c structural choice | score / insns |
|---|---|---|
| BASE | the s9 ledger body | **23 / 216** |
| D1 | hoist `temp_v0_2 = temp_a1_2 * 2;` above the `temp_v1_3 == 0` test and reuse it for `temp_s3` | 30 / 217 |
| D2 | `var_s1 = 0;` written ABOVE the eight-way range check instead of below it | 29 / 216 |
| D3 | the shared store+calls tail owned by the `temp_v1_3 < temp_v0_3` arm (m2c's block_23/block_24), with the `== 0` arm jumping in - i.e. our set_0xB/do_store_calls/do_calls ownership inverted | 49 / 216 |
| D4 | range-check exit spelled `goto block_13;` (shares the outer `temp_v1 != 4` return) | 25 / 214 |
| D5 | no `ret` carrier at all - literal `return 1;` at every exit site | 33 / 211 |
| R1 | all five at once | 65 / 214 |

Conclusion: m2c's canonical shape is not closer to target on ANY axis. The rederive modality
is spent for the m2c route - a tenth session should not re-run m2c expecting a different
skeleton. (Sibling/transplant route also checked and empty: the only other functions touching
D_800A38A8 / D_800A3876 are func_8002006C, func_80026DA4, func_800288C8, func_8002AB08,
func_8002C61C, and all five are still `INCLUDE_ASM` - there is no matched sibling body in the
tree to transplant a shape from.)

Note D5's 211 insns: dropping the `ret` carrier makes GCC fold four returns into constants and
lose four instructions outright, which is why it scores 33 despite being shorter. The `ret`
carrier is what forces target's `addu $v0,$s6,$zero` tail (target holds the return value in the
callee-saved $s6 for the whole function), and it is confirmed load-bearing.

### 2. Exit-form enumeration (frontier item 2) - COMPLETE, and 23 is the floor of that dimension

Four early-exit sites, measured on the 23-floor body (s10 measurements marked *):

| site | inline `return` | `goto block_49` | `goto block_13` |
|---|---|---|---|
| 1 range-check exit | **23** (BASE) | 25 * (E5) | 25 * (D4) |
| 2 `temp_v1_2 == 0xE` exit | **23** | 38 (s9 E1) | - |
| 3 do_calls exit | **23** (BASE) | 23 * (E3, byte-identical) | - |
| 4 globals block | 25 (s8) | **23** (s9 D4) | - |

Crossed: site 1 `goto block_13` + site 3 `goto block_49` = 25 / 214 (E4) - the two axes are
additive-free, they do not interact. **No assignment over the four sites scores below 23**, and
in particular cluster B (the nop-vs-`li v0,1` divergence at emitted slots 45/47) does NOT move
under any exit-form assignment. Frontier item 2 is closed; do not re-open it.

Both goto spellings at site 1 buy exactly 2 instructions (216 -> 214) and cost exactly 2 points.
That is worth remembering if a future body ever needs to shed instructions rather than points.

### 3. The `||` guard spelling is pinned (new axis, killed)

De Morgan on the `== 5` guard - `if (!A && !B) { globals } else { selection }` instead of
`if (A || B) { selection } else { globals }` - inverts both branch senses and swaps which block
is the beqz's fall-through, which is exactly the geometry that decides reorg's steal. Measured
R2 (selection inline in the else arm) = 31 / 216 and R3 (selection reached by `goto block_sel;`
so the two selection copies stay merged) = 27 / 212. Both regress. Target's
`bnez v0,.L800285CC` / `beqz v0,.L800285DC` pair only comes out of the short-circuit `||`
spelling with the selection in the TRUE arm.

### 4. Frontier item 1 re-attributed with measurements (s9's stated mechanism was wrong)

s9 guessed reorg declines the delay-slot steal because `li v0,1` writes the register the
`beqz v0` itself reads. That is NOT the predicate. The gate in GCC 2.7.2 reorg.c:3466-3468 is

    if (condition == const_true_rtx
        || (! insn_sets_resource_p (trial, &opposite_needed, 1) && ! may_trap_p (pat)))

where `opposite_needed = mark_target_live_regs (opposite_thread)` and `opposite_thread` is the
FALL-THROUGH insn, not the branch. Re-running the s9 isolate/dump recipe on the 23-floor body
with `BB2_DBR_DEBUG=1` (the instrumented cc1 at tools/gcc-2.7.2/cc1 already carries DBRDBG
hooks in fill_slots_from_thread and mark_target_live_regs) produced, in
tmp/grind/func_800283D0/s10/iso_dumps/dbr.log:

    DBRDBG thr insn=344 thread=368 opp=686 own=1 likely=0 tif=1 oppregs=20630084_00000000 oppmem=1
    DBRDBG thr insn=344 trial=368 refset=0 setset=0 setneed=0 setsopp=1 trap=0
    DBRDBG thr LOSE insn=344 trial=368
    ... same LOSE for trials 370, 373, 375

insn 344 is `beqz v0 -> label 365` (the globals block); insn 368 is the `li v0,1` we want in the
slot. `oppregs = 0x20630084` decodes to {v0, a3, s0, s1, s5, s6, sp}: **v0 is reported LIVE at
the fall-through**, so `setsopp=1` and every insn in the globals block loses (the two `sh`
stores lose additionally on `oppmem=1`). There is no `DBRDBG thr insn=344 thread=686` line at
all, so `own_fallthrough` was 0 and the fall-through thread was never scanned either - the slot
is unfillable from both directions while v0 sits in opposite_needed.

Also measured: `mostly_true_jump` returns 0 here (an EQ condition with no rarity difference),
so fill_eager_delay_slots takes the `prediction <= 0` branch and tries fall-through first.

OPEN SUB-QUESTION for the next session (this is the whole of frontier item 1 now): WHY is v0 in
that live set? `DBRDBG mtlr target=686 block=28` shows find_basic_block(686) walked back to the
previous BARRIER and returned block 28 - the same block as insn 339, i.e. label 347 (the
fall-through label, also the target of the earlier `bnez v0`) is NOT a basic-block head and the
scan is one long forward walk from block 28's live-at-start. On that walk, insn 344 carries
`REG_DEAD (reg:SI 2 v0)` which goes into `pending_dead_regs`, and reorg.c:2709 flushes
pending_dead_regs at every CODE_LABEL - and label 347 IS on the walk between 344 and 686. By the
letter of the source v0 should therefore be clear. It is not. Either the flush is not reached
(a `next_insn_no_annul` / stop_insn subtlety) or v0 is re-introduced by the second phase that
steps forward from TARGET. The decisive next probe is to add a DBRDBG print inside
mark_target_live_regs' forward loop (insn UID + the running current_live_regs) and read where
bit 2 gets set; the instrumented cc1 and the isolate recipe are both in place
(tmp/grind/func_800283D0/s10/isolate.py + dumpiso.sh). Until that is known, no C-level shape can
be aimed at this residual - and note that no C shape can avoid the block writing v0, because
target's own block is `li v0,1 / lui at / sh v0 / li v0,-1 / lui at / sh v0`.

- [s10] Chassis re-measured at dispatch: the s9 candidate body applied to src/code6cac_b.c gives sandbox --disable all = score 23, target_insns 215, build_insns 216. The ledger floor of 23 is live and unchanged; every s10 measurement is relative to that body.

- [s10] Fresh m2c output (tmp/grind/func_800283D0/s10/m2c_fresh.c, 150 lines) reproduces the same control-flow skeleton the ledger body already has, differing in exactly five structural choices - all five measured regressive on the 23-floor chassis. The rederive/m2c route is spent; a later session should not re-run m2c expecting a different skeleton.

- [s10] The `ret` carrier local is CONFIRMED load-bearing: dropping it for a literal `return 1;` at every exit lets GCC fold four returns into constants and lose four instructions outright (211 insns) while scoring 33. Target holds the return value in the callee-saved $s6 for the whole function and ends with `addu $v0,$s6,$zero`.

- [s10] The exit-form dimension over all four early-exit sites is fully enumerated on the 23-floor body and 23 is its minimum. Cluster B (nop-vs-li v0,1 at emitted slots 45/47) does not move under any assignment. Frontier item 2 is CLOSED - do not re-open it.

- [s10] Both goto spellings at the range-check exit site (goto block_13 and goto block_49) are score-identical at 25/214: they buy exactly 2 instructions and cost exactly 2 points. Useful if a future body ever needs to shed instructions rather than points.

- [s10] `goto block_49;` at the do_calls exit is BYTE-IDENTICAL to the inline `return ret;` (both 23/216) - jump2 re-merges it, confirming s9's G2 finding on this chassis.

- [s10] The short-circuit `||` spelling of the ==5 guard is pinned; de Morgan regresses to 31 (selection inline in the else arm) and 27 (selection behind a shared goto label).

- [s10] No transplant source exists: the only other functions referencing D_800A38A8 / D_800A3876 are func_8002006C, func_80026DA4, func_800288C8, func_8002AB08 and func_8002C61C, and all five are still committed as INCLUDE_ASM.

- [s10] reorg.c gate measured exactly: 'DBRDBG thr insn=344 ... oppregs=20630084_00000000 oppmem=1' followed by setsopp=1 LOSE for trials 368, 370, 373 and 375. v0 is in opposite_needed, so nothing in the globals block is stealable, and own_fallthrough=0 means the fall-through thread was not scanned either.

- [s10] 'DBRDBG mtlr target=686 block=28' shows find_basic_block(686) walked back to the previous BARRIER and returned the same block as insn 339 - label 347 (the fall-through label, also the target of the earlier bnez v0) is NOT a basic-block head, so mark_target_live_regs does one long forward walk from block 28's live-at-start.

- [s10] No C-level shape can avoid the globals block writing v0: target's own block is `li v0,1 / lui at / sh v0 / li v0,-1 / lui at / sh v0`. The only lever on this residual is getting v0 out of opposite_needed.

## s11 (rederive, 2026-08-26) - FLOOR 23 -> 20; CLUSTER A ($s2/$s3) CLOSED by the s7 nrefs(73) dial

Chassis at dispatch: the s9/s10 candidate body applied to src/code6cac_b.c printed
`score 23, target_insns 215, build_insns 216`. Every s11 measurement is relative to that.

### 1. The rederive mandate, honestly spent - and the shape axis is CONVERGED

s10 closed the m2c route and the transplant route. This session took the two remaining
rederive angles and both came back score-NEUTRAL, which is itself the finding:

* **R1 - the matched-sibling idiom.** `func_8002798C` (src/code6cac_b.c:369) is a
  COMPLETED-C function in the same file by the same author, and it spells multi-way state
  tests as a run of positive `if (x == K) goto label;` statements with a fall-through
  `return`, never as a negated `&&` chain. Respelling our eight-way range check that way
  (`if ((u32)(temp_a1-0x19) < 2U) goto set_s1; if (temp_v1 == 2) goto set_s1; ... return 1;
  set_s1: var_s1 = 0; goto block_15;`) measures **23 / 216 - identical**. The author-
  idiomatic spelling is free. Banked at
  tmp/grind/func_800283D0/s11/variants/R1_sibling_dispatch.c.
* **R2 - operand order.** Target emits `addu $s3,$v0,$s0` (offset+base) where we emit
  `addu $s2,$s0,$v0` (base+offset). Writing the C as `(temp_a1_2 * 2) + arg0` measures
  **23 / 216 - identical**; GCC canonicalises the PLUS. The operand order is a CONSEQUENCE
  of which register the pointer lands in, not an independent diff. Do not chase it again.

An index-aligned objdump diff of the 23-floor body against target (built this session,
tmp/grind/func_800283D0/s11/mkdiff.py) confirms the shape conclusion mechanically: the two
streams agree instruction-for-instruction everywhere; every remaining difference is a
REGISTER NAME, a delay-slot fill, or the one extra nop. **There is no instruction-selection
or control-flow divergence left to rederive.** The residual is 100% register seats + reorg.

### 2. s7's cluster-A windows RE-MEASURED on the 23-floor body: UNCHANGED (frontier item 2, half closed)

`tools/ra_solver/extract.py func_800283D0 code6cac_b` on the 23-floor body, then an exhaustive
single-atom sweep of `simulate.Sim` (tmp/grind/func_800283D0/s11/sweep.py, every allocno x
livelen 1..250 x nrefs 1..24) reproduces s7's table EXACTLY:

    baseline callee-saved map 72:$s0 77:$s1 143:$s2 73:$s3 75:$s4 90:$s5 79:$s6
    target's                  72:$s0 77:$s1  73:$s2 143:$s3 75:$s4 90:$s5 79:$s6
    livelen(143)  14 -> [20,21]      livelen(73)  92 -> [38,65]
    nrefs(143)     3 -> [2]          nrefs(73)     7 -> [8,9,10,11]
    every other allocno inert at every value

So the s8 mode split and the s9 exit-form change did NOT move the global allocno model at
all. s7's windows are live on this chassis and a future session may spend them directly.

New this session - the arithmetic behind the window, which makes it obvious the cluster is
NOT a tie that could be nudged by luck. global.c allocno_compare ranks by
`pri = floor_log2(nrefs) * nrefs / allocno_live_length * 10000 * size`:

    pseudo 143 (temp_s3): floor_log2(3)*3 = 3,  / 14 -> 2142
    pseudo  73 (arg1):    floor_log2(7)*7 = 14, / 92 -> 1521

143 outranks 73 by ~40%, sorts first, and takes $s2. Nothing short of moving one of the four
dials can change that.

### 3. THE LEVER: duplicating a shared call pair raises nrefs(arg1) past the window

`arg1` has 7 references (the prologue copy plus six `addu $a2,$s2,$zero` argument set-ups).
The `temp_v1_3 < temp_v0_3` arm in the s9 body reaches the shared pair with `goto do_calls;`,
so it contributes none. Writing that arm out in full - its own selection, its own store, its
OWN copy of the two `func_80032854` calls, its own `return ret;` - is plain ordinary C and
raises the count.

    sandbox --disable all:  20 / 212      (was 23 / 216)

and the model, re-extracted on the new body, reports `nrefs_flow(73) = 9` (inside the [8,11]
window) and **`base meets goal? True`**. The emitted prologue is now target's
(`sw $s2,0x20($sp) / addu $s2,$a1,$zero ... sw $s3,0x24($sp)`) and all ten $s2<->$s3 slots
(4, 85, 90, 96, 97, 102, 107, 111, 148, 153) are correct. **Cluster A is CLOSED.**

Why this is a legitimate C-level lever and not laundering: jump.c cross-jumping runs in
jump2, which is AFTER reload. A source-level duplicate is therefore real RTL while global.c
sorts allocnos, and is only re-merged afterwards. That is a genuine ordering fact about the
pass pipeline, not a spelling trick - and it means "how many times does the source mention
`arg1`" is a real, reachable dial for any function whose callee-saved seats hinge on it.
NOTE FOR A FUTURE candidate-ready SESSION: the arm-duplication still has to be CLASSIFIED
before it can be submitted (it is byte-neutral by construction, which is the shape the
`duplicated-statement-into-arms` family covers, and that family mandates a `/* FAKE */`
annotation). This session did not need the vet - the floor is 20, not 0 - but do not submit
the construct without one.

### 4. The remaining 3-instruction shortfall is ONE cause, and it is measured

jump2's find_cross_jump merges the whole duplicated `<` arm away, leaving a single
`bnez $v0,<shared block>`; target keeps a four-instruction arm
(`beqz $s1,.L80028518 / addiu $v0,0x19 / j .L80028520 / sh $v0,0x286($s0)`) that enters the
shared block at two different points. Target's arm survives the common-tail walk because its
inner branch has the INVERTED sense relative to the shared block's (`beqz $s1` vs
`bnez $s1`) - which is exactly what the s9 body's `goto set_0xB;` spelling produced for free.

Two spellings that keep the inverted sense were measured. Both score the SAME 20 but are
longer, because with the `do_calls` label dead the `sh $v0,0x286($s0)` store sinks out of its
target position in front of `addiu $a1,$zero,1` and into the following `jal`'s delay slot:

| variant | shape | score / insns |
|---|---|---|
| R4/R5 | `<` arm fully written out, same branch sense | **20 / 212** (banked) |
| R6 | `goto set_0xB;` kept + duplicated calls | 20 / 219 |
| R7 | duplicate in the `0xB` path, `goto do_calls;` kept alive for the `0x19` path | 20 / 222 |

R5 additionally proved the arm's LAYOUT is not the lever: forcing the duplicated arm to be
the fall-through (`if (temp_v1_3 >= temp_v0_3) goto ge_block;`) emits a byte-identical 212.

So the score is flat at 20 across every duplication spelling tried, and the open question is
sharply defined: find a spelling that (a) duplicates the calls, for the nrefs dial,
(b) keeps the inner branch inverted, so the arm does not fully merge, and (c) keeps a live
label between the store and the argument set-up, which is what pins the store.

### 5. Killed this session

* **R3** - hoisting `u8 *temp_s3 = arg0 + temp_a1_2*2;` above the `temp_v1_3 == 0` test so
  both reads share it (a plausible original shape, and an attempt at the livelen(143) dial):
  **59 / 215**. It adds a FOURTH reference to the pointer, and at nrefs 4 the model needs
  livelen(143) in [53,58], nowhere near reachable. Banked
  rejected/shared-ptr-hoist-adds-4th-ref-59.c.

- [s11] Chassis re-measured at dispatch: the s9/s10 body gives score 23 / 216 insns. The rederive shape axis is now CONVERGED - an index-aligned objdump diff shows ours and target agree instruction-for-instruction, with every remaining difference a register name, a delay-slot fill, or one extra nop. No instruction-selection or control-flow divergence remains.

- [s11] The matched sibling func_8002798C (src/code6cac_b.c:369) spells multi-way state tests as positive `if (x == K) goto label;` runs with a fall-through return. Respelling our eight-way range check in that idiom is score-IDENTICAL (23/216). The author-idiomatic spelling is free; it is not a lever.

- [s11] `(temp_a1_2 * 2) + arg0` is score-identical to `arg0 + (temp_a1_2 * 2)`: GCC canonicalises the PLUS, and target's `addu $s3,$v0,$s0` operand order is a consequence of the register assignment, not an independent diff. Do not chase it again.

- [s11] s7's cluster-A single-atom windows re-measure UNCHANGED on the 23-floor body: livelen(73) 92 -> [38,65], nrefs(73) 7 -> [8,9,10,11], livelen(143) 14 -> [20,21], nrefs(143) 3 -> [2], every other allocno inert. The s8 mode split and the s9 exit-form change did not perturb the global allocno model.

- [s11] The cluster-A seat is a ~40% priority inversion, not a tie: global.c allocno_compare gives pri(143) = floor_log2(3)*3/14 = 2142 versus pri(73) = floor_log2(7)*7/92 = 1521.

- [s11] FLOOR 23 -> 20. Writing the `temp_v1_3 < temp_v0_3` arm out in full - own selection, own store, own copy of the two func_80032854 calls, own return - instead of `goto do_calls;` raises nrefs_flow(arg1) from 7 to 9, lands inside the [8,11] window, and CLOSES CLUSTER A: the ra_solver model reports `base meets goal? True` and all ten $s2<->$s3 slots are correct.

- [s11] The mechanism is a pass-ordering fact, not a spelling trick: jump.c cross-jumping runs in jump2, AFTER reload, so a source-level duplicate is real RTL while global.c sorts allocnos and is only re-merged before emission. "How many times the source mentions a param" is therefore a real dial on callee-saved seats.

- [s11] The banked 20-floor body is 212 insns, 3 SHORT of target: jump2 merges the whole duplicated arm away, where target keeps a 4-insn arm that survives because its inner branch has the inverted sense (`beqz $s1` vs the shared block's `bnez $s1`).

- [s11] Two inverted-sense duplication spellings measured: `goto set_0xB;` retained (20/219) and duplicate-in-the-0xB-path (20/222). Both score the same 20 but are LONGER, because with the `do_calls` label dead the `sh $v0,0x286($s0)` store sinks out of its target position into the following jal's delay slot. The score is flat at 20 across every duplication spelling tried.

- [s11] Arm LAYOUT is not a lever: forcing the duplicated arm to be the fall-through with `if (temp_v1_3 >= temp_v0_3) goto ge_block;` emits a byte-identical 212.

- [s11] KILLED: hoisting `temp_s3` above the `temp_v1_3 == 0` test so both reads share it measures 59/215 - it adds a fourth reference to the pointer, and at nrefs 4 the model needs livelen(143) in [53,58].

- [s11] Chassis re-measured at dispatch: the s9/s10 candidate body applied to src/code6cac_b.c gives sandbox --disable all = score 23, target_insns 215, build_insns 216.

- [s11] FLOOR 23 -> 20 (build_insns 212). The single change is that the `temp_v1_3 < temp_v0_3` arm is written out in full - own selection, own store, own copy of the two func_80032854 calls, own `return ret;` - instead of `goto do_calls;`. Plain ordinary C; the only new name is the arm-local `s16 var_v0_4`.

- [s11] Cluster A (the ten-slot $s2/$s3 callee-saved rotation, emitted slots 4, 85, 90, 96, 97, 102, 107, 111, 148, 153) is CLOSED. The prologue now reads target's `sw $s2,0x20($sp) / addu $s2,$a1,$zero ... sw $s3,0x24($sp)`.

- [s11] Mechanism measured, not guessed: ra_solver extract on the new body reports nrefs_flow(pseudo 73 = arg1) = 9 (was 7), inside s7's predicted [8,11] window, and the forward model prints `base meets goal? True`.

- [s11] The lever is a pass-ordering fact: jump.c cross-jumping runs in jump2, AFTER reload, so a source-level duplicate is real RTL while global.c sorts allocnos and is re-merged only before emission. 'How many times the source mentions a param' is therefore a genuine, reachable dial on callee-saved seats for any function in this tree.

- [s11] global.c allocno_compare arithmetic for this function: pri = floor_log2(nrefs)*nrefs/allocno_live_length*10000*size gives pri(143, temp_s3) = 3/14 -> 2142 versus pri(73, arg1) = 14/92 -> 1521. The cluster-A seat is a ~40% priority inversion, not a tie that could be nudged by luck.

- [s11] s7's cluster-A single-atom windows re-measure UNCHANGED on the 23-floor body (livelen(73) 92 -> [38,65]; nrefs(73) 7 -> [8..11]; livelen(143) 14 -> [20,21]; nrefs(143) 3 -> [2]; all other allocnos inert). The s8 mode split and the s9 exit-form change did not perturb the global allocno model. Half of standing frontier item 2 is closed.

- [s11] The rederive shape axis is CONVERGED: an index-aligned objdump diff shows ours and target agree instruction-for-instruction, with every remaining difference a register name, a delay-slot fill, or one extra nop. No instruction-selection or control-flow divergence remains to rederive.

- [s11] The matched sibling func_8002798C (src/code6cac_b.c:369) spells multi-way state tests as positive `if (x == K) goto label;` runs with a fall-through return. Respelling our eight-way range check in that author idiom is score-IDENTICAL (23/216) - free, but not a lever.

- [s11] `(temp_a1_2 * 2) + arg0` is score-identical to `arg0 + (temp_a1_2 * 2)`: GCC canonicalises the PLUS, and target's `addu $s3,$v0,$s0` operand order is a consequence of the register assignment. Do not chase it again.

- [s11] The banked 20-floor body is 3 instructions SHORT (212 vs 215), from ONE cause: jump2 merges the whole duplicated arm away, where target keeps a 4-insn arm (`beqz $s1,.L80028518 / addiu $v0,0x19 / j .L80028520 / sh $v0,0x286($s0)`) that survives because its inner branch has the inverted sense relative to the shared block's `bnez $s1`.

- [s11] Two inverted-sense duplication spellings both score 20 but are LONGER (R6 219, R7 222): keeping the inverted branch kills the `do_calls` label, and the `sh $v0,0x286($s0)` store then sinks into the following jal's delay slot. The score is flat at 20 across every duplication spelling tried, so the 212-insn form is banked as the compact one.

- [s11] Arm LAYOUT is not a lever: forcing the duplicated arm to be the fall-through with `if (temp_v1_3 >= temp_v0_3) goto ge_block;` emits a byte-identical 212.

- [s11] KILLED: hoisting temp_s3 above the `temp_v1_3 == 0` test so both reads share it measures 59/215 - it adds a fourth reference to the pointer, and at nrefs 4 the model needs livelen(143) in [53,58].

- [s11] CLASSIFICATION NOTE for a future candidate-ready session: the arm duplication is byte-neutral by construction (jump2 re-merges it), which is the shape the `duplicated-statement-into-arms` family covers, and that family mandates a /* FAKE */ annotation. This session did not need a self-vet (floor 20, not 0), but the construct must not be submitted without one.

---

## s12 (2026-08-26) — structural — floor 20 -> 17

Chassis at dispatch: the s11 candidate re-measured 20 / 212 insns on HEAD, so
every s11 conclusion below is chassis-valid.

### 1. A normalized asm diff now exists (tmp/grind/func_800283D0/s12/norm.py)

s11's `mkdiff.py` compared raw objdump text against the target listing, so every
`li` vs `addiu $rX,$zero,K`, `move` vs `addu $rX,$rY,$zero`, `0x14` vs `20` and
`%hi()` vs `lui rX,0` pair showed as a "replace" and buried the real residual in
noise. `s12/norm.py` canonicalises both sides (strip `$`, hex->dec, branch/jump
targets -> `T`, `addu rX,rY,zero` -> `move`, `addiu rX,zero,K` -> `li`) before
difflib. On the s11 body the entire residual then reads as SIX clusters, and
they line up 1:1 with the score of 20. Use it (`bash s12/dif.sh`) for every
future probe on this function; the unnormalized diff is not readable.

### 2. THE FINDING: jump2's walk-back is steered by the INITIALISATION ORDER of
### the arm's selection variable, and that single change closes cluster A and
### the arm shape simultaneously (floor 20 -> 17).

s11 banked the two requirements as independent and unsatisfiable together:
(a) the `<` arm must DUPLICATE the two `func_80032854` calls, because that is the
only reachable dial on `nrefs_flow(73 = arg1)` (7 -> 9, into s7's [8,11] window),
which is what makes `arg1` sort before `temp_s3` in `allocno_compare` and take
$s2; and (b) the arm must survive `find_cross_jump` as target's four
instructions. s11 measured that any duplicate spelled the same way as path1
collapses wholesale (212 insns, arm gone), and that the two half-goto spellings
(R6/R7) keep the arm but sink the store (219 / 222 insns). All three scored 20.

s12's measurement shows the resolution is not a third requirement but the
CONTENT of the walk-back. `find_cross_jump` compares insns pairwise from the two
block ends and stops at the first difference. With the arm's selection variable
initialised to the OPPOSITE constant from path1's:

    path1 : li v0,0x19 ; bne s1,0,Lsh ; li v0,0xB  ; Lsh: sh ; calls ; j end
    arm   : li v0,0xB  ; beq s1,0,Lsh ; li v0,0x19 ; Lsh: sh ; calls ; j end

the walk matches `j end`, both calls and the `sh`, then compares `li v0,0x19`
against `li v0,0xB`, differs, and stops. The arm keeps its two-insn selection and
its (redirected, hence inverted) branch — target's `beqz $s1,.L80028518 /
addiu $v0,0x19 / j .L80028520 / sh $v0,0x286($s0)` — while still carrying the
duplicated calls through global-alloc. With s11's spelling (both blocks
initialised to 0x19) the walk also matched the branch and the `li`, so the whole
arm collapsed to a single `bnez` and three instructions were lost.

Measured on the new body: all twelve $s2/$s3 slots (3, 4, 10, 85, 90, 96, 97,
102, 107, 111, 149, 154) are correct AND the arm's four instructions are
reproduced. Score 17 / 216 insns. The construct is ordinary C — a branch arm
that spells its two-way choice from the opposite end than its neighbour.

### 3. The nrefs(73) window is confirmed a second time, independently

`tools/ra_solver/extract.py` + s11's sweep re-run on the V1 (all-goto) body:
`nrefs_flow(73) = 7`, `livelen_flow(73) = 132`, `base meets goal? False`, and the
single-atom sweep reproduces s7's table EXACTLY on this chassis — livelen(143)
14 -> [20,21], nrefs(143) 3 -> 2, livelen(73) 92 -> [38,65], nrefs(73) 7 ->
[8,9,10,11]. V2/V3 (arm duplicates only the FIRST call, nrefs 8) flip the seats
correctly, which confirms the window's lower bound behaviourally: 8 is enough.

### 4. Ten structural spellings measured and dead (all banked in rejected/)

| form | score / insns | what it proves |
|---|---|---|
| V1 arm all-goto (`goto set_0xB` + `goto do_calls`) | 23 / 216 | arm bytes EXACT, seats all inverted — the two requirements ride the same construct |
| V2 arm dups first call, re-enters at `do_call2` | 22 / 219 | nrefs 8 flips seats; arm still merges |
| V3 as V2 with a selection variable | 22 / 212 | same |
| V4 arm both edges fully duplicated, 0x19 first | 24 / 224 | no merge at all |
| V5 same, 0xB edge first | 26 / 224 | worse |
| V7 0xB edge dups, 0x19 edge `goto do_calls` | 20 / 222 | = s11 R7 re-measured, unchanged |
| V8 0xB edge `goto set_0xB`, 0x19 edge dups | 20 / 219 | = s11 R6 re-measured, unchanged |
| V9 inverted-init selection + `goto do_calls` | 25 / 216 | the inverted init is worthless WITHOUT the duplicate — it is the walk-back stop, not a dial by itself |
| P1 path1's store+calls duplicated per selection edge | 29 / 224 | a second, independent route to more arg1 refs is badly regressive |
| P2 path1's store duplicated per edge, shared `do_calls` | 19 / 222 | close but worse |
| Q1 `(temp_a1_2 * 2) + arg0` | 17, NEUTRAL | re-confirms s11 R2 on this chassis: `addu $s3,$v0,$s0` operand order is a CONSEQUENCE of allocation |
| Q4 tail declaration order (product before the 0x118 load) | 17, NEUTRAL | declaration order does not move cluster E |

### 5. The residual at 17, exactly

1. **Store sink (~4 pts).** Ours: `li a1,1 / move a2,s2 / lh a0 / move a3,zero /
   jal / sh v0,0x286(s0)<delay>`. Target: `sh v0,0x286(s0) / li a1,1 /
   move a2,s2 / lh a0 / jal / move a3,zero<delay>`. Nothing separates our store
   from the argument set-up, so sched2 sinks it and reorg takes it as the jal's
   filler. In target `.L80028520` sits between them, alive because the arm's
   `j .L80028520` targets it — and that label exists precisely because target's
   arm merged only from AFTER the store on its 0x19 edge (it keeps its own `sh`
   in the `j`'s delay slot), while ours merges AT the store. This is the SAME
   walk-back mechanism s12 exploited, one instruction further along: we need the
   walk to stop one insn EARLIER on the 0x19 edge only.
2. **Cluster B (2 pts).** Unmoved since s10: ours fills the dispatch chain's last
   `beq` delay slot with `li v0,1` and leaves the following `j`'s empty; target
   does the reverse. Same shape again at emitted slot 126.
3. **Cluster E (~8 pts).** The $a0/$a1 exchange in the tail Judge product
   (emitted 159/161/168/169/171/178/181), a LOCAL-alloc seat. s7's window
   span(qty0) 30 -> <=24 or refs(qty0) 6 -> 8..10. Q4 shows plain declaration
   order does not reach it; the local half of s7's model has still never been
   re-derived on a current body.

- [s12] Chassis check: the s11 candidate re-measured 20 / 212 insns on HEAD at session start, so every s11 conclusion was chassis-valid going in.

- [s12] New floor 17 / 216 insns, banked in memory/grind/func_800283D0/candidate.c. The only diff versus the s11 body is the initialisation order of the '<' arm's selection variable (0xB first, overwritten with 0x19 when var_s1 != 0). Ordinary C - a branch arm that spells its two-way choice from the opposite end than its neighbour. No new locals, nothing dead, nothing annotated, no sanctioned-family claim needed.

- [s12] Cluster A (the $s2/$s3 callee-saved rotation) and the over-merged '<' arm are CLOSED together. Both were open in every prior session: s9/s10 had the arm right and the seats wrong (23), s11 had the seats right and the arm wrong (20).

- [s12] s7's single-atom RA windows reproduce EXACTLY on this chassis, re-extracted from the V1 body: livelen(143) 14 -> [20,21], nrefs(143) 3 -> 2, livelen(73) 92 -> [38,65], nrefs(73) 7 -> [8,9,10,11]; base flow for 73 is nrefs 7 / livelen 132 / calls_crossed 3, and 'base meets goal? False'. V2/V3 confirm the window's lower bound behaviourally: nrefs 8 is enough to flip the seats.

- [s12] A normalized asm differ now exists: tmp/grind/func_800283D0/s12/norm.py + dif.sh. s11's mkdiff.py compared raw objdump text against the target listing, so every li/addiu, move/addu, hex/dec and %hi()/lui pair read as a divergence and the real residual was unreadable. Normalized, the residual resolves into clusters that sum to the score. Use dif.sh for every future probe on this function.

- [s12] The residual at 17 is exactly three things. (1) THE STORE SINK, ~4 pts: ours emits 'li a1,1 / move a2,s2 / lh a0,4(s0) / move a3,zero / jal / sh v0,0x286(s0)<delay>' where target emits 'sh v0,0x286(s0) / li a1,1 / move a2,s2 / lh a0,4(s0) / jal / move a3,zero<delay>'. (2) CLUSTER B, 2 pts, unmoved since s10: ours fills the dispatch chain's last 'beq' delay slot with 'li v0,1' and leaves the following 'j' empty, target does the reverse; same shape again at emitted slot 126. (3) CLUSTER E, ~8 pts: the $a0/$a1 exchange in the tail Judge product at emitted 159/161/168/169/171/178/181, a LOCAL-alloc seat.

- [s12] Twelve structural forms measured and banked to memory/grind/func_800283D0/rejected/ this session (47 entries total): arm-goto-both-edges-nrefs7-seats-lost-23.c, arm-dup-first-call-only-22.c, arm-sel-dup-first-call-remerges-22.c, arm-both-edges-full-dup-24.c, arm-both-edges-full-dup-flipped-26.c, arm-0xB-dup-0x19-goto-docalls-20.c, arm-sel-inverted-goto-docalls-nrefs7-25.c, path1-both-edges-full-dup-29.c, path1-store-dup-goto-docalls-19.c, s3-operand-order-neutral-17.c, tail-decl-order-neutral-17.c.


## s13 (structural, 2026-08-26) - chassis floor 17 / 216 insns, unchanged

**E-s13-1 (PASS RE-ATTRIBUTION, supersedes the s12 frontier-1 mechanism).**
The sink of `sh $v0,0x286($s0)` out of the head of the shared store/calls block
and into the `jal` delay slot is produced by **sched1** (`schedule_insns`,
pre-reload), not by jump2's `find_cross_jump` walk-back and not by `reorg.c`.
Measured on the banked 17-floor body with `pwsh tools/grinder/dump.ps1
func_800283D0` (dumps in `tmp/grind/func_800283D0/dumps/`):
  - `.combine` and `.flow`: `(insn 226 223 227 (set (mem:HI (plus:SI (reg 72)
    (const_int 646))) ...))` - prev insn is 223, the `do_store_calls`
    `code_label`.  The store is the FIRST insn of the block.
  - `.sched`: the same insn is now `(insn 226 240 242 ...)` - prev is 240, the
    `a3 = 0` argument set-up.  sched1 moved it below all four argument set-ups.
  - `.jump2`: rows 115-128 of `tmp/grind/func_800283D0/s13/j2rows.txt` show the
    merged block as `code_label 223 ("do_store_calls") / a1=1 / a2=s2 / a0=lh /
    a3=0 / store 226 / call 242`.  jump2's anchor is label 223, which by then
    already precedes the argument set-up; jump2 inherits the sunk store.
This kills the s12 frontier-1 mechanism ("the store sink is the SAME walk-back
mechanism s12 exploited, one instruction further along") as stated.  The
walk-back is not involved; the C-level dial is whatever stops sched1.

**E-s13-2 (the pin, and it is exact).**  What stops sched1 is a basic-block
boundary between the store and the argument set-up, i.e. a **label that already
exists when sched1 runs**.  Because jump2 is downstream of sched1, only a
SOURCE-level `goto` can supply it.  Variant A -
`rejected/arm-goto-docalls-pins-store-but-loses-arg1-refs-23.c`, whose `<` arm
reads `if (var_s1 == 0) goto set_0xB; *(s16*)(arg0+0x286) = 0x19; goto
do_calls;` - reproduces TARGET BYTE-FOR-BYTE across both affected regions.
Verified by direct objdump of `tmp/sandbox/func_800283D0/code6cac_b.o`:
`6a4 li v0,11 / 6a8 sh v0,646(s0) / 6ac li a1,1 / 6b0 move a2,_ / 6b4 lh a0,4(s0)
/ 6b8 jal / 6bc move a3,zero` - the store at the head, the `a3` set-up in the
jal delay slot, exactly target's slots 83-88; and the arm itself emits target's
four instructions `beqz s1,.L80028518 / li v0,0x19 / j .L80028520 /
sh v0,0x286(s0)` at slots 144-147.

**E-s13-3 (the cost, and why the two requirements are now provably coupled).**
Variant A scores **23 / 216**.  The `goto do_calls` is exactly what deletes the
arm's two C-level references to `arg1`, so `nrefs_flow(pseudo 73)` falls 9 -> 7,
outside s7's [8,11] window, and all twelve $s2 seats revert to $s3 (`move
a2,s3` in the objdump above).  The coupling is now mechanistic rather than
empirical: the pinning label must predate sched1 => it must be a source `goto`
=> that edge cannot also carry the duplicated call pair.  A label manufactured
by jump2 cross-jumping a duplicated call pair arrives after sched1 and pins
nothing - which is the measured explanation for why that whole family
(s12's V8, and s13's B) sits at 20.

**E-s13-4 (variant C: both requirements held simultaneously, +6 insns).**
`rejected/arm-0x19-goto-docalls-0xB-dup-calls-pins-store-keeps-s2-plus6-20.c`:
`if (var_s1 != 0) { store 0x19; goto do_calls; } store 0xB; <both calls>;
return;`.  Measured **20 / 222**.  Objdump confirms BOTH the pinned store
(`6a8 sh` at the head of the shared block) AND `move a2,s2` at every seat - the
only body so far that holds cluster A and the store pin at the same time.  Its
entire residual is the 0xB edge: jump2 merges it only down to three insns
(`li v0,0xB / sh / j do_calls`) plus an unfilled `jal` delay slot.

**E-s13-5 (other measurements).**  B (arm 0xB edge `var_v0 = 0xB; goto
do_store_calls`; 0x19 edge stores + duplicates the calls) = **20 / 219**.
F (variant A's arm plus path1's own 0x19 selection edge duplicating the call
pair, to restore the arg1 refs from a different site) = **27 / 223**; the
duplicated path1 edge does not merge and costs more than the seats are worth.

**E-s13-6 (tooling).**  `tmp/grind/func_800283D0/s1x/dif.sh` objdumps
`tmp/sandbox/func_800283D0/code6cac_b.o` and was repeatedly observed reading the
object left by the PREVIOUS sandbox invocation, producing internally
inconsistent diffs (a head section from one build, a tail from another).  Run
`sandbox --disable all` TWICE before `dif.sh`, or objdump the `.o` directly.
Two of this session's turns were spent resolving a contradiction caused by it.

- [s13] Chassis re-measured at dispatch: the banked candidate.c body scores 17 with build_insns 216 vs target 215 - identical to the s12 ledger entry, so every s12 conclusion is still chassis-valid.

- [s13] PASS ATTRIBUTION (dumps read, not guessed): the store `sh $v0,0x286($s0)` is block-head in .combine and .flow (`(insn 226 223 227 ...)`, prev = code_label 223 = the C label `do_store_calls`) and is already below all four argument set-ups in .sched (`(insn 226 240 242 ...)`). sched1 sinks it; jump2 and reorg only inherit the result. This supersedes the s12 frontier-1 mechanism.

- [s13] tmp/grind/func_800283D0/s13/j2rows.txt rows 115-128 show jump2's merged block as `code_label 223 ("do_store_calls") / a1=1 / a2=s2 / a0=lh(s0+4) / a3=0 / store 226 / call 242` - the anchor label already precedes the argument set-up at jump2 time.

- [s13] Variant A (arm 0xB edge `goto set_0xB`, 0x19 edge own store + `goto do_calls`) reproduces target's slots 83-88 exactly - direct objdump: `6a4 li v0,11 / 6a8 sh v0,646(s0) / 6ac li a1,1 / 6b0 move a2,_ / 6b4 lh a0,4(s0) / 6b8 jal / 6bc move a3,zero` - and target's four-instruction arm at 144-147. Score 23/216; the sole regression is cluster A ($s3 at all twelve seats).

- [s13] Variant C (0x19 edge stores + `goto do_calls`, 0xB edge stores + duplicates both calls) is the first measured body to hold the pinned store AND the correct $s2 seats simultaneously: 20/222. Its whole residual is the 0xB edge, which jump2 merges only to `li v0,0xB / sh / j do_calls` (+3) plus an unfilled jal delay slot.

- [s13] Target pays ZERO for its arm's 0xB edge because that edge jumps into path1's own `li v0,0xB` at .L80028518; the only C construct that reproduces that (a bare `goto set_0xB`) supplies no arg1 reference at all.

- [s13] Variant B = 20/219 and variant F = 27/223 - banked; F specifically rules out path1's own 0x19 selection edge as a byte-neutral third site for the two arg1 references.

- [s13] TOOLING: tmp/grind/func_800283D0/s1x/dif.sh objdumps tmp/sandbox/func_800283D0/code6cac_b.o and was observed reading the object from the PREVIOUS sandbox invocation, yielding diffs whose head and tail came from different builds. Run `sandbox --disable all` twice before dif.sh, or objdump the .o directly.


## s14 (synthesis, 2026-08-26) - FLOOR 17 -> 11; CLUSTER E CLOSED

**E-s14-0 (chassis).**  The banked s12/s13 candidate body re-measured **17 /
216** on HEAD at session start, so every s12/s13 conclusion was chassis-valid
going in.  New floor this session: **11 / 216**, banked in
`memory/grind/func_800283D0/candidate.c`.

**E-s14-1 (THE FINDING: a named pointer local pins its own RTL to the head of
its block, and deleting the local is what moves the local-alloc dial).**
Cluster E (the $a0/$a1 exchange at emitted slots 159-182) is s7's LOCAL-alloc
quantity-order decision, and its complete single-atom vector set - re-derived
this session on the CURRENT body with `inverse.py local --func func_800283D0
--block 42 --swap 0,3` (the tail block is block **42** on this chassis, was 41
on s7's) - is `span(qty0) 30 -> <= 24`, reachable from the birth end for any
birth in **[8,19]** (base 2).  Every declaration-order probe ever run against
this window is score-neutral: s7's B_ptr_late (birth 2 -> 6, four of the six
insns needed), s12's Q4, and this session's p3 (`temp_v1_4` declared before the
pointer, 17) and t2 (`temp_v1_5`'s product computed before the `+0x118` load
with the pointer local retained, 17).  The reason is now measured: while
`u8 *temp_a0 = temp_s4 + (temp_s5 * 0x10);` exists as a NAMED LOCAL, its RTL is
emitted at the head of the block regardless of what statements follow it, so
the quantity's birth cannot be pushed past ~6.  **Deleting the local and
spelling the three field reads from the object base**
(`*(s32 *)(temp_s4 + (temp_s5 * 0x10) + 0x114)` and likewise for `+0x11C` and
`+0x118`), with `temp_v1_4` and the `temp_v1_5` product written before the
`+0x118` read, lets CSE form the pointer pseudo at its FIRST USE - now inside
the product, after both `(&Judge)[...]` index computations.  Fresh QTYDBG dump
of block 42 (main pass) on the new body:

    before:  qty0 reg192 birth 2  death 32 refs 6 -> $a1   (ours, wrong)
             qty3 reg209 birth 16 death 20 refs 2 -> $a0
    after :  qty2 reg208 birth 12 death 32 refs 6 -> $a0   (target)
             qty3 reg204 birth 16 death 20 refs 2 -> $a1   (target)

birth 2 -> 12, span 30 -> 20, inside the solver's window.  Score **17 -> 11**;
cluster E disappears from the normalized emitted diff.  This is the first
solver-PREDICTED flip on this function that a C spelling actually delivered,
and the generalizable lesson is the one in the first sentence: *a named pointer
local is a birth-order pin; to delay a quantity's birth you must delete the
local, not reorder the declarations around it.*

**E-s14-2 (cluster A's dial set is closed at depth 3, not just single-atom).**
`inverse.py global --swap 73,143 --depth 3 --top 30` on variant A's body
returns exactly five vectors and all five are ref-count atoms on the same two
pseudos: `refs(73) 7 -> 8/9/10/11` and `refs(143) 3 -> 2`.  No preference-,
conflict-, or birth-order vector reaches the goal at any depth up to 3.  This
closes the "maybe the seat can be won through prefs/conflicts instead" reading
that s5-s13 never tested, and it means cluster A has exactly two C-level dials.

**E-s14-3 (the `refs(143) 3 -> 2` dial is CSE-foreclosed - measured, killed).**
The only C spelling that removes a reference to `temp_s3` without deleting a
load is to recompute the address at the second use, i.e. write the `== 5` test
as `*(s16 *)(arg0 + (temp_a1_2 * 2) + 0x288)` (the spelling path1 already uses).
Measured on both bodies: on the 17-floor body **17, neutral**; on variant A's
body **23, neutral**; and `extract.py` on the recompute body reports pseudo 143
`nrefs_flow = 3` unchanged - CSE re-merges the address into the same pseudo
before flow.c counts references.  With this, `refs(73) 7 -> 8..11` is the ONLY
live dial on cluster A, which is exactly the dial the arm's duplicated call
pair supplies, and exactly the one the store pin destroys.

**E-s14-4 (target's own asm re-read: the arm and the store label).**  Target's
`<` arm is `beqz $s1,.L80028518 / addiu $v0,0x19 / j .L80028520 / sh
$v0,0x286($s0)` - the 0x19 edge keeps its OWN store and jumps PAST path1's
store to `.L80028520`; the 0xB edge jumps INTO path1's `addiu $v0,0xB` at
`.L80028518`, which falls through path1's own store.  Path1 is `bnez $s1,
.L8002851C / addiu $v0,0x19 / .L80028518: addiu $v0,0xB / .L8002851C: sh /
.L80028520: args+calls`.  So target carries THREE labels in this region and the
one that matters for the store, `.L80028520`, has exactly one incoming edge -
the arm's `j`.  Target's `$s3` (= `temp_s3`) has three references and its `$s2`
(= `arg1`) has seven, identical to ours, so target's extra pre-jump2 reference
is invisible in the final asm (s7's reg_n_refs-is-counted-before-jump2 point)
and its source is still unlocated.

**E-s14-5 (the store-pin / cluster-A tension survives the tail fix - measured).**
t4 = variant A's arm (source `goto do_calls`, which reproduces target's slots
83-88 and 144-147 byte-exactly per E-s13-2) combined with THIS session's
no-pointer tail: **17 / 216**.  Against t3's 11, the pin is still worth
+6 in the store region and -6 in cluster A, unchanged in magnitude by closing
cluster E.  The two requirements remain coupled at the arm exactly as E-s13-3
described.

**E-s14-6 (both commutative `addu` operand orders are inert on this chassis).**
On the 11-floor body the normalized diff still shows `addu s3,s0,v0` vs target
`addu s3,v0,s0` (slot 96) and `addu a0,a0,s4` vs target `addu a0,s4,a0`
(slot 162).  Writing the C operands in the other order - `(temp_a1_2 * 2) +
arg0`, `(temp_s5 * 0x10) + temp_s4`, and both together - measures **11 in all
three cases**.  Re-confirms s11's R2 / s12's Q1 on a new chassis and extends it
to the tail: emitted `addu` operand order is a consequence of allocation, not
an independent C lever.  Banked as three rejected forms.

**E-s14-7 (the residual at 11, exactly).**  From
`tmp/grind/func_800283D0/s14/diff11.txt`:
  1. **Store sink, ~4 pts** - emitted 83-88 (`sh v0,646(s0)` at the block head
     in target, in the jal delay slot for us) and 146-148 (target's arm keeps
     its own `sh`).  sched1, per E-s13-1.  Needs a source label between store
     and args; costs cluster A (E-s14-5).
  2. **Cluster B, ~4 pts** - emitted 45-48 (ours `j / nop`, target `nop / j`)
     and 126/131 (ours `nop` plus a later `li v0,1`, target `li v0,1` in the
     branch delay slot).  This is also the entire 216-vs-215 insn surplus.
     Unmoved since s10; no measured C dial.
  3. Two commutative `addu` operand orders (96, 162) - inert per E-s14-6.

- [s14] Chassis: banked s12/s13 body re-measured 17/216 at session start; new floor 11/216.
- [s14] Cluster E CLOSED by deleting the tail pointer local temp_a0 and spelling the three field reads from `temp_s4 + (temp_s5 * 0x10) + <off>`, with temp_v1_4 and the temp_v1_5 product written before the +0x118 read. QTYDBG block 42: the pointer quantity moves birth 2 -> 12 (span 30 -> 20) and takes $a0, the Judge element takes $a1 - both target's seats.
- [s14] GENERAL LESSON: a named pointer local pins its own RTL to the head of its basic block, so NO declaration reorder around it can delay the quantity's birth. s7's B_ptr_late, s12's Q4, s14's p3 and t2 are all score-neutral for this one reason. Delete the local; let CSE birth the pseudo at first use.
- [s14] The tail block is block 42 on this chassis (was 41 on s7's). inverse.py local --swap 0,3 gives birth window [8,19] for the pointer quantity; base 2, B_ptr_late 6, the no-local spelling 12.
- [s14] inverse.py global --swap 73,143 --depth 3 returns ONLY ref-count atoms: refs(73) 7->8/9/10/11 and refs(143) 3->2. No pref/conflict/birth vector exists at depth <= 3. Cluster A has exactly two C dials.
- [s14] refs(143) 3->2 is CSE-foreclosed: recomputing the address at the `== 5` site measures neutral on both bodies (17 and 23) and extract.py still reports nrefs_flow(143) = 3.
- [s14] t4 (variant A's pinning arm + the no-pointer tail) = 17/216: the store pin is still exactly -6 (cluster A) / +6 (store region) after cluster E is closed. The coupling is unchanged.
- [s14] Both commutative addu operand orders (slots 96 and 162) measure 11 in either spelling - inert, banked.

- [s14] Chassis: the banked s12/s13 candidate body re-measured 17 / 216 on HEAD at session start, so every s12/s13 conclusion was chassis-valid going in. New floor this session: 11 / 216, banked in memory/grind/func_800283D0/candidate.c.

- [s14] CLUSTER E IS CLOSED. Deleting the tail pointer local temp_a0 and spelling the three field reads from `temp_s4 + (temp_s5 * 0x10) + <off>`, with temp_v1_4 and the temp_v1_5 product written before the +0x118 read, moves the pointer quantity from birth 2 / span 30 / $a1 to birth 12 / span 20 / $a0 and gives the Judge element $a1 - both target's seats. Verified from a fresh QTYDBG dump of block 42, not from the model's prediction.

- [s14] GENERAL LEVER (new, and reusable across the project): a named local's initializer RTL is emitted at the head of its basic block, so NO declaration reorder around it can delay the quantity's birth in local-alloc. s7's B_ptr_late (birth 6), s12's Q4, and s14's p3 and t2 are all score-neutral for this single reason. To delay a birth, DELETE the local and let CSE create the pseudo at first use; to advance a birth, introduce one.

- [s14] The tail block is block 42 on this chassis (it was block 41 on s7's). inverse.py local --func func_800283D0 --block 42 --swap 0,3 gives the complete birth-end window [8,19]; base 2, B_ptr_late 6, the no-local spelling 12.

- [s14] inverse.py global --swap 73,143 --depth 3 --top 30 returns ONLY ref-count atoms: refs(73) 7->8/9/10/11 and refs(143) 3->2. No preference, conflict or birth-order vector reaches the goal at depth <= 3, so cluster A has exactly two C-level dials.

- [s14] The refs(143) 3->2 dial is CSE-foreclosed: recomputing temp_s3's address at the `== 5` site measures neutral on both bodies (17 and 23) and extract.py still reports nrefs_flow(143) = 3. refs(73) 7->8..11 is therefore the ONLY live dial on cluster A - the same dial the arm's duplicated call pair supplies and the store pin destroys.

- [s14] Target's asm re-read: the `<` arm is `beqz $s1,.L80028518 / addiu $v0,0x19 / j .L80028520 / sh $v0,0x286($s0)` - the 0x19 edge keeps its OWN store and jumps PAST path1's store; the 0xB edge jumps INTO path1's `addiu $v0,0xB`. Path1 carries three labels (.L80028518, .L8002851C at the store, .L80028520 after it) and .L80028520 has exactly ONE incoming edge, the arm's. Target's $s3 has 3 references and its $s2 has 7, identical to ours, so target's extra pre-jump2 arg1 reference is invisible in the final asm and its source is still unlocated.

- [s14] t4 (variant A's pinning arm + the no-pointer tail) = 17 / 216: after cluster E is closed the store pin is STILL worth exactly -6 in cluster A and +6 in the store region. The E-s13-3 coupling is unchanged, not resolved.

- [s14] Both commutative addu operand orders left in the diff (slots 96 and 162) measure 11 in either spelling - inert, consequences of allocation rather than levers. Three rejected forms banked.

- [s14] Residual at 11, from tmp/grind/func_800283D0/s14/diff11.txt: (1) the store sink, ~4 pts, emitted 83-88 and 146-148, owned by sched1; (2) cluster B, ~4 pts, emitted 45-48 and 126/131, which is also the entire 216-vs-215 insn surplus and still has no measured C dial; (3) the two inert addu operand orders.

- [s14] Eight forms banked to rejected/ this session (59 entries total): s3-refs-recompute-cse-remerges-neutral-17.c, varA-plus-s3-recompute-still-nrefs3-23.c, tail-v14-declared-before-ptr-neutral-17.c, tail-product-before-0x118-load-named-ptr-neutral-17.c, varA-arm-plus-noptr-tail-clusterA-lost-17.c, tail-ptr-operand-order-neutral-11.c, s3-operand-order-neutral-11.c, both-operand-orders-neutral-11.c.


## s15 (synthesis, 2026-08-26) - chassis floor 11 / 216 confirmed; cluster A reduced to CLOSED-FORM arithmetic, and target's own asm re-attributed to the livelen branch

**E-s15-0 (chassis).**  The banked s14 candidate body re-measured **11 / 216**
on HEAD at session start (two sandbox runs, `tmp/grind/func_800283D0/s15/m.ps1`).
Every s12/s13/s14 conclusion is therefore chassis-valid.  The floor did NOT
move this session; what moved is the MODEL of the last two clusters.

**E-s15-1 (THE FINDING: cluster A is now a closed-form inequality, not an
empirical window).**  `tools/gcc-2.7.2/global.c` `allocno_compare` is, verbatim:

    pri = (((double) (floor_log2 (allocno_n_refs[v]) * allocno_n_refs[v])
            / allocno_live_length[v]) * 10000 * allocno_size[v]);

and allocnos are allocated in DESCENDING pri.  Measured against the
instrumented ALLOCDBG dump of the store-pinned body (t4 = variant A's arm +
s14's no-pointer tail, `tmp/grind/func_800283D0/s15/model_t4.json`):

    pseudo 143 (temp_s3)  ord 19  nrefs 3  livelen 14  pri 2142  -> $s2 (18)  [wrong seat]
    pseudo  73 (arg1)     ord 21  nrefs 7  livelen 92  pri 1521  -> $s3 (19)  [wrong seat]

    1*3/14  * 10000 = 2142.8 -> 2142   (matches the dump exactly)
    2*7/92  * 10000 = 1521.7 -> 1521   (matches the dump exactly)

So the seat exchange is EXACTLY the condition `pri(73) > pri(143)`, and the
complete set of single-input solutions is arithmetic, not empirical:

  * `nrefs(73) 7 -> 8`   : 3*8/92  = 2608 > 2142  OK   (and 9/10/11 likewise)
  * `nrefs(143) 3 -> 2`  : 1*2/14  = 1428 < 1521  OK
  * `livelen(143) >= 20` : 1*3/20  = 1500 < 1521  OK   (14 -> 20 is the minimum)
  * `livelen(73) <= 65`  : 2*7/65  = 2153 > 2142  OK

This supersedes s7's empirically-swept "[8,11] window" framing with the
underlying formula, and it is reusable on any BB2 function whose residual is a
callee-saved seat exchange: dump ALLOCDBG, compute the two pris, read off which
of the four dials can close the gap and by how much.

**E-s15-2 (the solver agrees on the pinned body, and s14's "only ref atoms"
claim is corrected).**  `inverse.py global model_t4.json --swap 73,143 --depth 2
--top 40` (full output `tmp/grind/func_800283D0/s15/inv_t4.txt`) returns SIX
single-atom vectors, and one of them is NOT a ref-count atom:

    #1 refs_up   73: 7 -> 8
    #2 refs_down 143: 3 -> 2
    #3 live_extend 143: live length 14 -> 22      <-- the non-ref vector
    #4/#5/#6 refs_up 73: 7 -> 9 / 10 / 11

s14's E-s14-2 recorded "inverse.py --depth 3 returns ONLY ref-count atoms".
That was run against variant A's body BEFORE the tail rewrite; on the current
chassis the live_extend atom is present and is the cheapest non-ref route.  The
solver's suggested lever text for it is "move the last use later in the
statement order" / "reuse one variable for both values so the range spans both".

**E-s15-3 (livelen(73) <= 65 is structurally unreachable - reasoned, not
measured).**  Pseudo 73 is the `arg1` parameter.  Its live range opens in the
prologue (`addu $s2,$a1,$zero`, forced: `$a1` is call-clobbered and the first
call is far downstream) and closes at the `0x2D` call in the `>` path, the last
of the three call sites.  Every block between is on a path that reaches a call
site, so flow.c counts it live throughout; 92 is a structural floor, not a
spelling artefact.  The fourth dial of E-s15-1 is therefore dead for THIS
function even though it is live in general.

**E-s15-4 (the arm's emitted shape, read from the object rather than the
normalized diff).**  Direct objdump of the 11-floor body
(`tmp/sandbox/func_800283D0/code6cac_b.o`, offsets 0x7a0-0x7ac):

    7a0  beqz  s1,0x6a8        <- the 0xB edge
    7a4  li    v0,11           (delay slot)
    7a8  j     0x6a8           <- the 0x19 edge
    7ac  li    v0,25           (delay slot)

BOTH edges land on the SAME address 0x6a8, i.e. jump2's cross-jump walk-back
consumed the arm's own `sh $v0,0x286($s0)` along with the duplicated call pair
and redirected both edges to a point BEFORE path1's store.  Target
(`asm/funcs/func_800283D0.s`, `.L80028608`) is:

    beqz  $s1,.L80028518
     addiu $v0,0x19
    j     .L80028520
     sh   $v0,0x286($s0)       <- the arm KEEPS its own store

i.e. target's 0x19 edge keeps its store and jumps ONE INSN FURTHER ON, past
path1's store, to `.L80028520`.  Our duplicated call pair merges 100% - which
is exactly why `nrefs(73) = 9` is byte-free on the 11-floor body and why the
floor is 11 rather than 17.

**E-s15-5 (THE RE-ATTRIBUTION: target reaches the seat through livelen, not
refs - so the whole ref-count lever line is a workaround).**  Combine E-s15-4
with s13's pass attribution:

  (a) `.L80028520` cannot be a jump2-manufactured label.  If it were - i.e. if
      target's arm had duplicated `store; calls; return` and jump2 had merged it
      - the walk-back would not have stopped in front of an IDENTICAL
      `sh $v0,0x286($s0)`; it would have swallowed the store too and landed
      both edges before it, which is precisely what ours does (E-s15-4).  The
      walk-back stopped because the label was already there.
  (b) A label that is already there before jump2, and that also survives sched1
      as a basic-block boundary (which is what keeps target's store at the head
      of the shared block instead of in the `jal` delay slot - E-s13-1), can
      only come from a SOURCE `goto`.
  (c) Therefore target's `<` arm IS variant A's arm - `if (var_s1 == 0) goto
      set_0xB; store 0x19; goto do_calls;` - which supplies ZERO duplicated
      arg1 references.
  (d) Target's final asm has seven `$s2` references, and (c) says no
      pre-jump2 duplicate is hiding behind them, so target's C compiles with
      `nrefs(73) = 7`, exactly like variant A.
  (e) By E-s15-1, with nrefs 7/3 the ONLY remaining way to win the seat is
      `livelen(143) >= 20` (livelen(73) <= 65 being dead by E-s15-3).

So the store pin and cluster A are NOT genuinely in tension in target's source;
they are in tension only in OUR source, because every body we have that wins
cluster A wins it through the ref-count dial (s11's duplicated call pair), and
that dial is the one thing the store pin's `goto` removes.  **The correct
attack is no longer "find a third `do_calls` edge that pays for the arm's
duplicate" (s14's frontier item 1); it is "lengthen pseudo 143's live range
from 14 to >= 20 while holding nrefs at 3, on top of variant A's arm".**  That
single change would close the store sink (~4 pts) and cluster A together and
leave only cluster B.

**E-s15-6 (the obvious livelen(143) spelling is measured and dead).**  Probe h1
(`rejected/s3-hoist-block20-drops-143-from-global-53.c`): hoist
`u8 *temp_s3 = arg0 + (temp_a1_2 * 2);` out of the `temp_v1_3 != 0` block up to
the head of block_20, so it dominates path1 and its range covers path1's block
(+~7 insns, the exact lengthening required).  Measured **53 / 215** on the
store-pinned body.  The ALLOCDBG dump of that body
(`tmp/grind/func_800283D0/s15/model_h1.json`) shows pseudo 143 is GONE from the
global allocno set entirely - CSE unifies the hoisted address with path1's own
`arg0 + temp_a1_2*2 + 0x288` computation, the merged pseudo is handled by
local-alloc, and one insn disappears (215 vs 216, path1 loses its own `addu`).
73 stays at ord 21 / $s3.  So the birth-end route to the lengthening is
foreclosed by CSE for the same reason the `refs(143) 3->2` dial is
(E-s14-3): any spelling that makes temp_s3 visible to path1 unifies with it.
**The remaining livelen route is therefore the DEATH end** - move temp_s3's
last use (the post-call `*(s16 *)(temp_s3 + 0x288) == 5` re-read) later in the
`==` block, or give temp_s3 a genuinely later real use - and that is untried.

- [s15] Chassis: the banked s14 candidate body re-measures 11 / 216 on HEAD. Floor unchanged this session; the model of the residual changed.

- [s15] CLOSED FORM for cluster A (from tools/gcc-2.7.2/global.c allocno_compare, verified digit-for-digit against ALLOCDBG): pri = floor_log2(nrefs)*nrefs/livelen*10000*size, allocated in descending pri. On the store-pinned body pseudo 143 = 3 refs / livelen 14 / pri 2142 (ord 19, $s2) and pseudo 73 = 7 refs / livelen 92 / pri 1521 (ord 21, $s3). The seat exchange is exactly pri(73) > pri(143), and the four single-input solutions are nrefs(73)>=8, nrefs(143)=2, livelen(143)>=20, livelen(73)<=65.

- [s15] livelen(73) <= 65 is structurally unreachable: 73 is the arg1 parameter, live from the prologue home to the last of three call sites, so 92 is a floor, not a spelling artefact.

- [s15] inverse.py global --swap 73,143 --depth 2 on the store-pinned body returns SIX single-atom vectors including `live_extend 143: 14 -> 22`. s14's E-s14-2 ("only ref-count atoms") was measured on the pre-tail-rewrite body and is superseded on this chassis.

- [s15] Objdump of the 11-floor arm (0x7a0-0x7ac): `beqz s1,0x6a8 / li v0,11 / j 0x6a8 / li v0,25` - BOTH edges land on the same address, i.e. jump2's walk-back consumed the arm's own store as well as the duplicated call pair. Target's arm keeps its own store and lands one insn further on, at .L80028520.

- [s15] RE-ATTRIBUTION: target's `.L80028520` cannot be a jump2-manufactured label (the walk-back would not have stopped in front of an identical `sh $v0,0x286($s0)` - ours proves it does not), and a label that also survives sched1 as a block boundary can only be a source `goto`. So target's `<` arm IS variant A's arm, target compiles with nrefs(73) = 7, and by the closed form target must win cluster A through livelen(143) >= 20. Our ref-count lever (s11's duplicated call pair) is a WORKAROUND that happens to be incompatible with the store pin; target has no such tension.

- [s15] Probe h1 (hoist temp_s3 to the head of block_20 so its range covers path1) = 53 / 215, and ALLOCDBG shows pseudo 143 vanishes from the global allocno set: CSE unifies the hoisted address with path1's own computation and path1 loses its `addu`. The BIRTH end of the livelen(143) lengthening is CSE-foreclosed exactly like the refs(143) 3->2 dial. Banked as rejected/s3-hoist-block20-drops-143-from-global-53.c.

- [s15] Re-measured this session for chassis validity: candidate.c = 11/216, t4 (variant A arm + no-pointer tail) = 17/216 - both identical to the s14 ledger values.

- [s15] Chassis re-measured at dispatch: the banked s14 candidate body scores 11 / 216 on HEAD (build_insns 216 vs target 215), and t4 (variant A's pinning arm + the no-pointer tail) re-measures 17 / 216 - both identical to the s14 ledger values, so every s12/s13/s14 conclusion is chassis-valid.

- [s15] global.c's allocno_compare, read verbatim from tools/gcc-2.7.2/global.c, is pri = (floor_log2(n_refs)*n_refs / live_length) * 10000 * size, sorted descending. On the store-pinned body the instrumented ALLOCDBG dump gives pseudo 143 (temp_s3) nrefs 3 / livelen 14 / pri 2142 / ord 19 / $s2 and pseudo 73 (arg1) nrefs 7 / livelen 92 / pri 1521 / ord 21 / $s3 - reproducing the formula digit-for-digit.

- [s15] The complete single-input solution set for the cluster-A seat exchange is therefore arithmetic: nrefs(73) >= 8 (3*8/92 = 2608), nrefs(143) = 2 (1*2/14 = 1428), livelen(143) >= 20 (1*3/20 = 1500), livelen(73) <= 65 (2*7/65 = 2153). This is reusable on any BB2 function whose residual is a callee-saved seat exchange.

- [s15] livelen(73) <= 65 is structurally unreachable here: pseudo 73 is the arg1 parameter, its range opens at the prologue home `addu $s2,$a1,$zero` (forced, $a1 is call-clobbered and the first call is far downstream) and closes at the 0x2D call in the `>` path, with every intervening block on a path to a call site. 92 is a floor, not a spelling artefact.

- [s15] inverse.py global model_t4.json --swap 73,143 --depth 2 --top 40 returns six single-atom vectors, one of which is NOT a ref-count atom: live_extend 143 live length 14 -> 22. s14's E-s14-2 ("only ref-count atoms at depth <= 3") was measured on the pre-tail-rewrite body and is superseded on this chassis.

- [s15] Objdump of the 11-floor body's arm (tmp/sandbox/func_800283D0/code6cac_b.o, 0x7a0-0x7ac): `beqz s1,0x6a8 / li v0,11 / j 0x6a8 / li v0,25` - BOTH edges land on the same address, i.e. jump2's walk-back consumed the arm's own store along with the duplicated call pair. Target's .L80028608 arm is `beqz $s1,.L80028518 / addiu $v0,0x19 / j .L80028520 / sh $v0,0x286($s0)` - the 0x19 edge keeps its own store and lands one insn further on. Our duplicated call pair merges 100%, which is why nrefs(73)=9 is byte-free at floor 11.

- [s15] RE-ATTRIBUTION: target's .L80028520 cannot be a jump2-manufactured label (a walk-back that swallows an identical `sh $v0,0x286($s0)` in our build would have swallowed target's too), and a pre-jump2 label that also survives sched1 as a basic-block boundary can only come from a source goto. So target's `<` arm IS variant A's arm, target compiles with nrefs(73) = 7, and by the closed form target must win cluster A through livelen(143) >= 20. The store-pin / cluster-A tension recorded since E-s13-3 is an artefact of OUR ref-count workaround, not a property of target's source; there is no third `do_calls` edge to find.

- [s15] Probe h1 (hoist temp_s3 to the head of block_20) = 53 / 215 and its ALLOCDBG dump shows pseudo 143 gone from the global allocno set - CSE unifies the hoisted address with path1's computation and path1 loses its `addu`. The BIRTH end of the livelen(143) lengthening is CSE-foreclosed exactly like the refs(143) 3->2 dial. Banked as rejected/s3-hoist-block20-drops-143-from-global-53.c.

- [s15] Cluster B detail added from target's asm: at emitted 126 target fills the `beqz $v0,.L800285DC` delay slot with `addiu $v0,$zero,0x1`, the FIRST insn of the branch TARGET block (steal_delay_list_from_target), leaving `lui at / sh` behind; our build emits a nop there and the `li v0,1` afterwards even though the target block's insn order is identical. The refusal is therefore in reorg.c's eligibility test (LABEL_NUSES of .L800285DC, or mark_target_live_regs believing $v0 live on the fall-through), not in the C statement order.
