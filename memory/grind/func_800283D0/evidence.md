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


## s16 (synthesis, 2026-08-26) - FLOOR 11 -> 10; E-s15-5 REFUTED, target's arm identified bit-for-bit

**E-s16-0 (chassis).**  The banked s15/s14 candidate body re-measured **11 / 216**
on HEAD at session start; `t4` (variant A's pinning arm + the no-pointer tail)
re-measured **17 / 216**.  Both identical to the ledger.  Every s12-s15
conclusion is chassis-valid.

**E-s16-1 (the cluster-A model is now COMPLETE: find_reg contributes nothing).**
E-s15-1 reduced the seat exchange to `pri(73) > pri(143)` under
`allocno_compare`, but that only fixes the allocation ORDER; which hard reg
each allocno then takes is `find_reg`'s call, and find_reg has three further
inputs (`regs_someone_prefers`, `regs_used_so_far` gating pass 0,
`hard_reg_copy_preferences`).  Dumped both allocnos with BB2_FINDREG_DEBUG on
the store-pinned t4 body (`tmp/grind/func_800283D0/s16/findreg_t4_143.txt`,
`findreg_t4_73.txt`):

    pseudo 143: someone_prefers: (empty)  own_copy_prefs: (empty)  own_full_prefs: (empty)
                used_so_far: 0..17 24..29 31   (18 and 19 both absent)
    pseudo  73: someone_prefers: (empty)  own_copy_prefs: (empty)  own_full_prefs: (empty)
                used_so_far: 0..18 24..29 31   (18 now present, 19 absent)

Every preference set is EMPTY, and neither 18 nor 19 is in `regs_used_so_far`
when 143 is allocated, so both allocnos fall through find_reg's pass 0 into
pass 1 and take the first non-conflicting register in `reg_alloc_order`.  The
seat is therefore a pure function of allocation order, i.e. of pri, with no
preference dial to steer.  **The "steer it with a copy preference" axis for
cluster A is KILLED**; E-s15-1's closed form is the whole model.  (Read
`tools/gcc-2.7.2/global.c:583-599`: one qsort on `allocno_compare`, one
sequential `find_reg` loop, no second phase.)

**E-s16-2 (new instrument: the whole callee-saved seat table in one command).**
`tmp/grind/func_800283D0/s16/al.py <tag>` compiles the current `src/` with the
instrumented cc1 under `BB2_ALLOC_DEBUG=1` and prints every allocno that landed
in `$s0..$s7` with its ord / pseudo / nrefs / livelen / pri.  This is strictly
better than tracking two pseudo numbers by hand, because the pseudo numbering
shifts between bodies.  Baselines measured this session:

    t4   : s0=72(19/156) s1=77(9/73) s2=143(3/14,pri2142) s3=73(7/92,pri1521) s4=75 s5=90 s6=79
    base : s0=72(21/164) s1=77(9/74) s2=73(9/99,pri2727)  s3=143(3/14,pri2142) s4=75 s5=90 s6=79
    v2   : s0=72(21/163) s1=77(9/74) s2=73(9/98,pri2755)  s3=143(3/14,pri2142) s4=75 s5=90 s6=79

**E-s16-3 (THE FINDING - E-s15-5's re-attribution is REFUTED, and target's `<`
arm is now identified bit-for-bit).**  E-s15-5 argued from target's
`.L80028520` that target's arm must be variant A's arm (no duplicated calls,
nrefs(73) = 7) and therefore that target must win cluster A through
`livelen(143) >= 20`.  That deduction is wrong.  Target's arm at `.L80028610`
is:

    beqz  $s1, .L80028518      <- branches INTO path1's `addiu $v0,0xB`
     addiu $v0, 0x19
    j     .L80028520           <- lands AFTER path1's store
     sh   $v0, 0x286($s0)      <- the arm keeps its own store, in the delay slot

i.e. the 0xB edge is fully merged into path1 (value assignment AND store) while
the 0x19 edge keeps its own value insn and its own store and rejoins after
path1's store.  That is precisely what an arm with **duplicated calls** looks
like after jump2 has cross-jumped the common `calls; return` tail and walked
one insn further back on the 0xB edge only.  Spelled in C, target's arm is

    s16 var_v0_4 = 0x19;
    if (var_s1 == 0) { goto set_0xB; }     /* set_0xB is path1's `var_v0 = 0xB;` */
    *(s16 *)(arg0 + 0x286) = var_v0_4;
    func_80032854(..., 1, arg1, ...);
    func_80032854(..., 0x25, arg1, ...);
    return ret;

**Measured: 10 / 216 - a new floor** (`tmp/grind/func_800283D0/s16/v2.c`, now
`candidate.c`), with ALLOCDBG confirming target's seats (73 -> $s2 at nrefs 9 /
pri 2755, 143 -> $s3 at pri 2142).  So target compiles with **nrefs(73) = 9**,
the duplicated call pair is TARGET'S OWN mechanism for the cluster-A seat
rather than our workaround, and there is no store-pin / cluster-A tension to
resolve.  The s15 frontier item 1 (lengthen `livelen(143)` from the death end
to >= 20) is therefore NOT the route and should not be spent; the four-dial
closed form of E-s15-1 stands, but target's dial is `nrefs(73) = 9`, which we
already hold.

**E-s16-4 (what makes the merge happen - the value carrier is load-bearing).**
Three spellings of the same arm were measured:

  * `v1` - identical control flow, but the 0x19 store written with a literal
    (`*(s16 *)(arg0 + 0x286) = 0x19;`): **14 / 219**.  jump2 refuses to merge
    the duplicated call pair AT ALL (+3 insns).  Banked
    `rejected/arm-goto-set0xB-literal-store-blocks-call-merge-14-219i.c`.
  * `v2` - the 0x19 value held in a fresh `s16 var_v0_4`: **10 / 216**.
  * `vA` - as v2 but the 0xB edge spelled `var_v0 = 0xB; goto do_store_calls;`
    (landing on path1's STORE rather than on path1's value assignment):
    **10 / 216**, same build_insns.  Codegen-equivalent to v2; kept as a note,
    not banked as a rejection (`tmp/grind/func_800283D0/s16/vA.c`).
  * `vD` - as v2 but the 0x19 edge jumps to the shared `do_calls` after its own
    store (so no duplicated pair): **17 / 216**, seats swap back.  Banked
    `rejected/arm-goto-docalls-after-own-store-loses-nrefs9-17.c`.  This is a
    third independent confirmation of the E-s15-1 arithmetic on this chassis.

**E-s16-5 (the residual at 10, read from a fixed normalized objdump diff).**
`tmp/grind/func_800283D0/s16/mk.sh` regenerates it (the s12 normalizer's
`ours.txt` extraction was broken by an objdump format change - the s16 copy
parses the tab-separated columns in Python instead of with `sed`).  Ignoring
the unlinked-object artefacts (`lui at,0` / `lh v1,0(v1)` / `jal 0` - relocs,
not diffs), the real residual is:

  1. **Cluster B, ~4 pts** and the entire 216-vs-215 surplus.  Emitted 45-48
     (ours `j / nop`, target `nop / j`) and 126/131 (ours `nop` in the
     `beqz $v0,.L800285DC` delay slot plus a later `li v0,1`; target steals the
     branch TARGET block's `li v0,1` into the slot).  Unmoved since s10.
  2. **The store sink, ~2 pts.**  Emitted 83-88: target `sh v0,0x286(s0)` at
     the head of the shared store/calls block, ours in the first jal's delay
     slot.
  3. **The arm store, ~1-2 pts.**  Emitted 148: target's arm `j` carries the
     arm's OWN `sh v0,0x286(s0)` in its delay slot; ours has no store left
     there - jump2's walk-back consumed it on BOTH edges - so reorg fills the
     slot by stealing `li a1,1` from the branch target block instead.
  4. Two commutative `addu` operand orders (96, 162), both previously measured
     score-neutral in either spelling.

  Items 2 and 3 are ONE question: whether path1's store sits at a basic-block
  boundary.  If the arm's 0x19 edge stopped its walk-back one insn earlier and
  landed after path1's store (target's `.L80028520`), path1's store would end a
  block - sched1 could not sink it into the jal delay slot (item 2) and the arm
  would still own a store for reorg to put in its own delay slot (item 3).
  s12 already proved the walk-back stopping point is steered by the
  initialisation order of the arm's selection variable relative to path1's;
  that dial has never been swept on a body whose arm carries the `goto set_0xB`
  shape.

- [s16] Chassis: candidate body (s14/s15) re-measures 11/216, t4 re-measures 17/216 - ledger values confirmed on HEAD.

- [s16] FLOOR 11 -> 10 (216 insns).  The `<` arm's 0xB edge respelled as a source `goto set_0xB` into path1's selection, with the 0x19 value carried in a fresh `s16 var_v0_4` and the duplicated call pair retained.  Reproduces target's `.L80028610` branch structure exactly and keeps nrefs(73) = 9, so cluster A stays won (ALLOCDBG on the new body: 73 ord 17 nrefs 9 livelen 98 pri 2755 -> $s2; 143 ord 20 nrefs 3 livelen 14 pri 2142 -> $s3).

- [s16] E-s15-5 IS REFUTED.  Target's `<` arm is NOT variant A's arm: `beqz $s1,.L80028518 / addiu $v0,0x19 / j .L80028520 / sh $v0,0x286($s0)` is a DUPLICATED-CALLS arm after jump2 cross-jumped the common `calls; return` tail and walked one insn further back on the 0xB edge only.  Target therefore compiles with nrefs(73) = 9, the duplicated pair is target's own cluster-A mechanism, and `livelen(143) >= 20` is NOT required.  Do not spend the s15 frontier item 1.

- [s16] find_reg contributes NOTHING to the cluster-A seat: BB2_FINDREG_DEBUG on both allocnos shows someone_prefers / own_copy_prefs / own_full_prefs all EMPTY and neither $s2 nor $s3 in regs_used_so_far at 143's allocation, so both fall to pass 1 and take the first non-conflicting reg in reg_alloc_order.  The seat is a pure function of allocation order (pri).  The preference-steering axis is KILLED.

- [s16] The 0x19 value carrier is load-bearing: the same arm with the store written as a literal (`= 0x19;`) measures 14/219 - jump2 refuses to merge the duplicated call pair at all.  Banked rejected/arm-goto-set0xB-literal-store-blocks-call-merge-14-219i.c.

- [s16] The same arm jumping to the shared `do_calls` after its own store (no duplicated pair) measures 17/216 and the $s2/$s3 seats swap back - a third independent confirmation of the E-s15-1 pri arithmetic on this chassis.  Banked rejected/arm-goto-docalls-after-own-store-loses-nrefs9-17.c.

- [s16] New instrument: tmp/grind/func_800283D0/s16/al.py prints the full $s0-$s7 seat table (ord / pseudo / nrefs / livelen / pri) for the current src in one command, which survives pseudo renumbering between bodies.  tmp/grind/func_800283D0/s16/mk.sh regenerates the normalized objdump-vs-target diff (the s12 normalizer's ours.txt extraction was broken; the s16 copy parses objdump's columns in Python).

- [s16] Residual at 10: cluster B ~4 (emitted 45-48 `j/nop` vs `nop/j`, and 126/131 the reorg steal of `li v0,1` into the `beqz $v0` delay slot), the store sink ~2 (emitted 83-88), the arm store ~1-2 (emitted 148: target's arm `j` delay slot carries the arm's own `sh`, ours steals `li a1,1` because jump2 merged the arm's store away), plus two score-neutral commutative addu operand orders (96, 162).  Items 2 and 3 are one question - whether path1's store ends a basic block.

- [s16] Chassis re-measured at session start: the s14/s15 candidate body = 11 / 216 and t4 (variant A's arm + the no-pointer tail) = 17 / 216 on HEAD, both identical to the ledger, so every s12-s15 conclusion is chassis-valid.

- [s16] NEW FLOOR 10 / 216: the `<` arm's 0xB edge respelled as a source `goto set_0xB` into path1's selection, with the 0x19 value carried in a fresh `s16 var_v0_4` and the duplicated call pair retained. Saved as memory/grind/func_800283D0/candidate.c.

- [s16] E-s15-5 IS REFUTED. Target's `<` arm (.L80028610: `beqz $s1,.L80028518 / addiu $v0,0x19 / j .L80028520 / sh $v0,0x286($s0)`) is a duplicated-calls arm after jump2 cross-jumped the common tail and walked one insn further back on the 0xB edge only. Target compiles with nrefs(73) = 9. The s15 frontier item 1 (lengthen livelen(143) from the death end to >= 20 on top of a store-pinned arm) is retired and must not be spent.

- [s16] find_reg contributes NOTHING to the cluster-A seat: BB2_FINDREG_DEBUG shows someone_prefers / own_copy_prefs / own_full_prefs all EMPTY for pseudos 73 and 143, and neither $s2 nor $s3 is in regs_used_so_far when 143 is allocated, so both fall to pass 1 and take the first non-conflicting reg in reg_alloc_order. The seat is a pure function of allocation order (pri); E-s15-1's closed form is the complete model.

- [s16] Seat tables measured this session (ord / pseudo / nrefs / livelen / pri): t4 -> $s2=143 (3/14/2142), $s3=73 (7/92/1521); base(11) -> $s2=73 (9/99/2727), $s3=143 (3/14/2142); v2(10) -> $s2=73 (9/98/2755), $s3=143 (3/14/2142).

- [s16] The 0x19 value carrier is load-bearing: the same arm with a literal store measures 14 / 219 because jump2 refuses to merge the duplicated call pair at all. Banked rejected/arm-goto-set0xB-literal-store-blocks-call-merge-14-219i.c.

- [s16] The same arm with `goto do_calls` in place of the duplicated pair measures 17 / 216 and swaps the seats back. Banked rejected/arm-goto-docalls-after-own-store-loses-nrefs9-17.c.

- [s16] Residual at 10, from a repaired normalized objdump-vs-target diff: cluster B ~4 (emitted 45-48 ours `j / nop` vs target `nop / j`, and 126/131 the reorg steal of `li v0,1` into the `beqz $v0,.L800285DC` delay slot, which is also the whole 216-vs-215 insn surplus); the store sink ~2 (emitted 83-88); the arm store ~1-2 (emitted 148: target's arm `j` delay slot carries the arm's own `sh v0,0x286(s0)`, ours steals `li a1,1` from the branch target block because jump2 merged the arm's store away); plus two commutative `addu` operand orders (96, 162) previously measured score-neutral.

- [s16] The store sink and the arm store are ONE question - whether path1's store ends a basic block. If the arm's 0x19 edge stopped its walk-back one insn earlier and landed after path1's store (target's .L80028520), path1's store would end a block, sched1 could not sink it into the jal delay slot, and the arm would still own a store for reorg to place in its own delay slot.

- [s16] New instruments for the next session: tmp/grind/func_800283D0/s16/al.py prints the whole $s0-$s7 seat table for the current src in one command (survives pseudo renumbering); tmp/grind/func_800283D0/s16/mk.sh regenerates the normalized objdump-vs-target diff (the s12 normalizer's ours.txt extraction was broken by an objdump format change and the s16 copy parses the columns in Python); tmp/grind/func_800283D0/s16/fr.py dumps ALLOCDBG plus the FINDREGDBG exclusion sets for named pseudos.


## s17 (solver, 2026-08-27) - floor HELD at 10 / 216; the cross-jump law behind the store pin is now MECHANISTIC, and E-s16-3 is refuted

**E-s17-0 (chassis).**  The banked s16 candidate body re-measured **10 / 216**
on HEAD at session start (twice, per the E-s13-6 staleness trap).  Ledger value
confirmed; every s12-s16 measurement is chassis-valid.

**E-s17-1 (TRIAGE - `inverse_compose.py classify` is BLIND on an
asm-until-matched function; `goal_from_tgt.py classify` is the correct entry
point).**  `tools/ra_solver/inverse_compose.py classify code6cac_b
func_800283D0` reports **IDENTICAL, 213 vs 213 insns**.  That verdict is
fiction: `mkasm_honest.sh` builds `<stem>.tgt.s` as "current src + regfix +
asmfix", and under asm-until-matched func_800283D0 carries ZERO rules, so the
"target" stream is our own build.  The object-level classifier
`tools/ra_solver/goal_from_tgt.py classify code6cac_b func_800283D0` compares
`tmp/sandbox/func_800283D0/code6cac_b.o` against `build/src/code6cac_b.o` (the
canonical build, where the function IS the split asm file) and gives the honest
verdict:

    func_800283D0 (code6cac_b): ours 216 insns, target 215   [replace_with_asmfile-safe]
    FIRST DIVERGENCE: PRE-RA
      ours only  : li #,1   x1
      ours only  : nop      x1
      target only: sh #,646(#)   x1

This is a reusable tooling fact for the whole pipeline: for any
asm-until-matched function, `inverse_compose classify` must not be used - it
will report IDENTICAL (or PRE-RA fiction) because its target stream is
self-referential.  Use `goal_from_tgt.py classify`.

**E-s17-2 (variant A re-measured on THIS chassis: it is byte-exact at BOTH
disputed regions).**  `tmp/grind/func_800283D0/s17/tA.c` = the s16 candidate
with the arm's 0x19 edge changed to `store; goto do_calls;`.  Measured
**17 / 216**.  Its normalized objdump-vs-target diff has **NO entry at emitted
83-88 and NO entry at 144-148** - path1's `sh $v0,0x286($s0)` sits at the head
of the shared block with `move $a3,$zero` in the `jal` delay slot, and the arm
emits target's four insns with its own store in the `j` delay slot.  tA's whole
residual is cluster A (the twelve `move $a2,$s2` seats plus the prologue
`sw $s2` / `sw $s3` pair), cluster B, and the `addu $a0,$s4,$a0` operand order.

**E-s17-3 (E-s16-3 IS REFUTED - the emitted arm does NOT discriminate).**
E-s16-3 argued that target's `.L80028610` arm must be a DUPLICATED-CALLS arm
because of its emitted shape.  E-s17-2 shows variant A's arm emits the identical
four instructions (this was already recorded in E-s13-2 and was overlooked in
s16).  The emitted arm is therefore evidence for NEITHER spelling.  What DOES
discriminate is emitted 83-88 together with 148: the s16 candidate (duplicated
calls) gets them WRONG, variant A gets them RIGHT.  Consequently s16's
instruction "do not spend the livelen(143) route" is WITHDRAWN, and s15's
E-s15-5 is restored as a live hypothesis - though E-s17-5 shows it is not the
only route.

**E-s17-4 (the closed form's cheapest dial is +1 REFERENCE, not +2 - measured).**
`inverse.py global model_tA.json --swap 73,143 --depth 3` re-derives exactly
E-s15-1's four dials on this chassis (minimal solution size 1 atom, 6 vectors:
`refs 73: 7->8/9/10/11`, `refs 143: 3->2`, `live_extend 143: 14->22`; 36
preference atoms reported FORECLOSED because callee-saved regs never appear in
pre-RA RTL, so `global.c set_preference` can never record them).
`tmp/grind/func_800283D0/s17/tB.c` supplies exactly ONE extra `arg1` reference
(the arm's 0xB edge duplicates the FIRST call only and jumps to a `do_call2:`
label placed between path1's two calls).  ALLOCDBG on tB:

    ord=18 pseudo=73  $s2  nrefs=8 livelen=101 pri=2376
    ord=21 pseudo=143 $s3  nrefs=3 livelen=14  pri=2142

i.e. **the seats flip at nrefs(73) = 8**, exactly as `3*8/92*10000 = 2608 >
2142` predicted.  Every previous body reached the seat with nrefs 9; half that
lift is enough.  tB itself scored 17 / 222 (its duplicated call block did not
merge).

**E-s17-5 (THE FINDING: cluster A and the store pin ARE simultaneously
satisfiable - `tC` = 12 / 221 - and what is left is a CROSS-JUMP SCHEDULING law,
read from the instrumented jump.c rather than guessed).**
`tmp/grind/func_800283D0/s17/tC.c` spells the arm as

    s16 var_v0_4 = 0x19;
    if (var_s1 == 0) {                    /* the 0xB edge supplies the pin */
        var_v0_4 = 0xB;
        *(s16 *)(arg0 + 0x286) = var_v0_4;
        goto do_calls;
    }
    *(s16 *)(arg0 + 0x286) = var_v0_4;    /* the 0x19 edge supplies the refs */
    func_80032854(*(s16 *)(arg0 + 4), 1, arg1, (s16 *)0);
    func_80032854(*(s16 *)(arg0 + 4), 0x25, arg1, (s16 *)0);
    return ret;

Measured **12 / 221**.  Its diff has NO cluster-A entry (all seats correct) AND
path1's store at the head of the shared block (emitted 83 correct).  This is the
first competitive body to hold both; s13's variant C held both at 20 / 222.
tC's ENTIRE residual over target is that the arm's four argument-setup insns
fail to cross-jump (+5 insns), plus cluster B and the two `addu` orders.

`BB2_XJUMP_DEBUG=1` on tC (the env-gated trace already built into
`tools/gcc-2.7.2/jump.c`; log at `tmp/grind/func_800283D0/s17/xj_tC.txt`) names
the refusal exactly:

    XJDBG: enter e1=467 e2=262 min=2 (chain-partner)
    XJDBG:   MATCH i1=462 i2=257 parallel     min->1     <- call 0x25
    XJDBG:   MATCH i1=460 i2=255 set(reg<-0)  min->0
    XJDBG:   MATCH i1=454 i2=249 set          min->-1
    XJDBG:   MATCH i1=458 i2=253 set          min->-2
    XJDBG:   MATCH i1=456 i2=251 set(reg<-37) min->-3
    XJDBG:   MATCH i1=447 i2=242 parallel     min->-4    <- call 1 (the jal)
    XJDBG:   PAT-MISMATCH i1=432 set vs i2=240 set(reg<-0) lose=0
    XJDBG: result e1=467 min=-4 last1=447 => WIN
    XJDBG: DO_CROSS_JUMP jump=467 newjpos=447 newlpos=242

i1=432 is the ARM's own `sh $v0,0x286($s0)`; i2=240 is path1's
`move $a3,$zero`.  THE LAW: `find_cross_jump` walks the two streams backwards
and stops at the first pattern mismatch.  path1's store is PINNED by the
`do_calls` label so it stays at its block head, while the arm's store is
UNPINNED (its block runs store -> args -> call), so sched1 sinks the arm's store
to just before its `jal`.  The two blocks are then scheduled differently and the
walk-back dies one insn past the `jal`.  A duplicated call pair merges 100% ONLY
when the two blocks are scheduled IDENTICALLY - both stores pinned, or both
sunk.  This supersedes E-s13-3's empirical statement of the tension with a
mechanism, and it explains every measurement from s11 onward.

**E-s17-6 (a NEW, cheaper pin for path1's store that costs no arm goto: `tD`).**
Path1 respelled so each selection edge does its own store -

    var_v0 = 0x19;
    if (var_s1 == 0) {
        var_v0 = 0xB;
        *(s16 *)(arg0 + 0x286) = var_v0;
        goto do_calls;
    }
    *(s16 *)(arg0 + 0x286) = var_v0;
    do_calls: ...

- makes path1's own 0xB edge supply the `do_calls` label, so path1's store is
pinned WITHOUT spending the arm's goto.  With the arm pinned the same way
(`tmp/grind/func_800283D0/s17/tD.c`) the build measures **11 / 212** and its
emitted 80-88 is BYTE-EXACT with target (`bnez $s1 / li 0x19 / li 0xB / sh /
a1 / a2 / a0 / jal / a3` - jump2 cross-jumps the two per-edge stores back into
one).  This is a strictly better store-pin primitive than E-s13-2's arm-side
`goto do_calls` and should be the starting point for every future pinned body.

**E-s17-7 (why tD is 3 insns SHORT: the over-merge, and the narrow window target
occupies).**  In tD the arm's two edges become textually IDENTICAL to path1's two
edges once the calls block is merged, so jump2 collapses the entire `<` arm and
the `<` test branches straight into path1 (emitted `bnez $v0` where target has
`beqz $v0`; target's four arm insns are absent).  Target therefore sits in a
narrow window: path1's store pinned, the arm's store pinned, AND the arm's 0x19
edge textually DISTINGUISHABLE from path1's 0x19 edge.  In target's asm path1's
0x19 edge has NO store of its own (it branches to `.L8002851C`) while the arm's
0x19 edge does.  Eight bodies were measured this session and none lands in the
window:

    base (s16 candidate)  both stores sunk, arm merges 100%             10 / 216
    tA   variant A arm (store; goto do_calls)                           17 / 216
    tB   +1 ref via duplicated call1 + a `do_call2:` label              17 / 222
    tC   path1 pinned by the arm's 0xB edge; 0x19 edge duplicates calls 12 / 221
    tD   path1 AND arm both split-store self-pinned                     11 / 212
    tE   tD's path1 + the s16 arm (arm store unpinned)                  12 / 221
    tG   arm rewritten 0x19-edge-first with an `arm_calls:` pin         10 / 216
    tH   tD's path1 + tG's arm                                          12 / 221

- [s17] Chassis re-measured at dispatch: the banked s16 candidate body = 10 / 216 on HEAD, identical to the ledger, so every s12-s16 conclusion is chassis-valid. Floor HELD at 10 this session.

- [s17] TOOLING (pipeline-wide): tools/ra_solver/inverse_compose.py classify is BLIND on asm-until-matched functions - its <stem>.tgt.s is "current src + regfix", and with zero rules that is our own build, so it reported IDENTICAL / 213-vs-213 for func_800283D0. Use tools/ra_solver/goal_from_tgt.py classify <stem> <func>, which compares tmp/sandbox/<func>/<stem>.o against build/src/<stem>.o at the object level. Its verdict here: PRE-RA, ours 216 vs target 215, "ours only: li #,1 x1 + nop x1; target only: sh #,646(#) x1".

- [s17] E-s16-3 IS REFUTED. Variant A's arm (store 0x19; goto do_calls) emits target's four instructions at 144-148 EXACTLY (already recorded in E-s13-2), so the emitted arm discriminates nothing. tA = 17 / 216 and its diff has NO entry at emitted 83-88 or 144-148: variant A reproduces BOTH disputed regions byte-exactly, and its sole residual is cluster A + cluster B + the addu $a0 operand order. s16's "do not spend the livelen(143) route" instruction is WITHDRAWN.

- [s17] The cluster-A seat flips at nrefs(73) = 8, not 9: tB (the arm's 0xB edge duplicates the FIRST call only, jumping to a do_call2: label between path1's two calls) gives ALLOCDBG 73 -> $s2 nrefs 8 livelen 101 pri 2376 and 143 -> $s3 pri 2142. Exactly ONE extra byte-free arg1 reference is required, half what every prior body spent. tB itself scored 17 / 222 (its duplicated call block did not merge).

- [s17] inverse.py global on model_tA.json (--swap 73,143 --depth 3) reproduces E-s15-1's four dials on this chassis: minimal solution size 1 atom, 6 vectors (refs 73: 7->8..11, refs 143: 3->2, live_extend 143: 14->22), plus 36 FORECLOSED preference atoms (callee-saved regs never appear in pre-RA RTL, so global.c set_preference can never record them).

- [s17] NEW BEST COMBINATION MEASURED: tC (arm's 0xB edge = own store + goto do_calls, 0x19 edge = own store + duplicated calls + return) = 12 / 221 - the first competitive body to hold cluster A (seats correct) AND path1's pinned store (emitted 83 correct) at once. Its entire residual over target is that the arm's four argument-setup insns fail to cross-jump (+5 insns).

- [s17] THE CROSS-JUMP LAW (read from BB2_XJUMP_DEBUG on the instrumented jump.c, not guessed): on tC, find_cross_jump matched 6 insns (call 0x25's block and call 1's jal) then PAT-MISMATCH i1=432 (the ARM's own sh $v0,0x286($s0)) vs i2=240 (path1's move $a3,$zero), and do_cross_jump merged only from the jal (newjpos=447 newlpos=242). path1's store is pinned by the do_calls label; the arm's store is unpinned so sched1 sinks it to just before the arm's jal. A duplicated call pair merges 100% ONLY when the two blocks are scheduled IDENTICALLY - both stores pinned or both sunk. This supersedes E-s13-3's empirical coupling with a mechanism.

- [s17] NEW CHEAPER PIN: path1 respelled so EACH selection edge does its own store and the 0xB edge ends with `goto do_calls` supplies the do_calls label itself - path1's store is pinned WITHOUT spending the arm's goto, and emitted 80-88 comes out BYTE-EXACT with target (jump2 cross-jumps the two per-edge stores back into one). Use this as the store-pin primitive from now on instead of E-s13-2's arm-side goto do_calls.

- [s17] tD (that path1 + the arm pinned the same way) = 11 / 212, THREE INSNS SHORT of target: with both sides pinned identically the arm's two edges become textually identical to path1's and jump2 collapses the entire `<` arm (emitted bnez $v0 where target has beqz $v0; target's four arm insns absent). Target's window is narrow: path1 pinned, arm pinned, AND the arm's 0x19 edge textually distinguishable from path1's 0x19 edge - in target path1's 0x19 edge has NO store of its own (it branches to .L8002851C) while the arm's does.

- [s17] Full measurement table this session: base 10/216, tA 17/216, tB 17/222, tC 12/221, tD 11/212, tE 12/221, tG 10/216 (codegen-equivalent to base), tH 12/221. All banked under memory/grind/func_800283D0/rejected/.

- [s17] Chassis re-measured at dispatch: the banked s16 candidate body = 10 / 216 on HEAD (twice, per the E-s13-6 staleness trap), identical to the ledger, so every s12-s16 conclusion is chassis-valid. Floor HELD at 10 this session.

- [s17] PIPELINE-WIDE TOOLING FACT: tools/ra_solver/inverse_compose.py classify is BLIND on any asm-until-matched function - its <stem>.tgt.s is 'current src + regfix/asmfix', and with zero rules that is our own build, so it reported FIRST DIVERGENCE: IDENTICAL / 213-vs-213 here. The correct triage tool is tools/ra_solver/goal_from_tgt.py classify <stem> <func>, which compares tmp/sandbox/<func>/<stem>.o against build/src/<stem>.o at the object level; its verdict here is PRE-RA, ours 216 vs target 215, 'ours only: li #,1 x1 + nop x1; target only: sh #,646(#) x1'.

- [s17] E-s16-3 IS REFUTED. tA (variant A's arm: `store 0x19; goto do_calls;`) = 17 / 216 and its normalized diff has NO entry at emitted 83-88 and NO entry at 144-148 - it reproduces target's store block AND target's four arm instructions byte-exactly. The emitted arm discriminates nothing between variant A and a duplicated-calls arm (E-s13-2 already said so); emitted 83-88 discriminates against the duplicated-calls arm. s16's instruction 'do not spend the livelen(143) route' is WITHDRAWN and s15's E-s15-5 is restored as a live hypothesis.

- [s17] tA's entire residual is cluster A (twelve `move $a2,$s2` seats plus the prologue `sw $s2` / `sw $s3` pair), cluster B, and the `addu $a0,$s4,$a0` operand order - nothing else.

- [s17] inverse.py global on model_tA.json (--swap 73,143 --depth 3) reproduces E-s15-1's four dials on this chassis: minimal solution size 1 atom, 6 vectors (refs 73: 7->8/9/10/11, refs 143: 3->2, live_extend 143: 14->22), plus 36 FORECLOSED preference atoms because callee-saved regs never appear in pre-RA RTL, so global.c set_preference can never record a preference for $s2 or $s3.

- [s17] THE SEAT FLIPS AT nrefs(73) = 8, NOT 9. tB (the arm's 0xB edge duplicates the FIRST call only and jumps to a `do_call2:` label between path1's two calls) gives ALLOCDBG 73 -> $s2 nrefs 8 livelen 101 pri 2376 and 143 -> $s3 nrefs 3 livelen 14 pri 2142. Exactly ONE extra byte-free arg1 reference is required - half what every prior body spent. tB itself scored 17 / 222 because its duplicated call block did not merge.

- [s17] NEW BEST COMBINATION: tC (arm's 0xB edge = own store + `goto do_calls`, 0x19 edge = own store + duplicated call pair + return) = 12 / 221, the first competitive body to hold cluster A (all seats correct) AND path1's pinned store (emitted 83 correct) at the same time. Its whole residual over target is the arm's four argument-setup insns failing to cross-jump (+5 insns), plus cluster B and the two addu orders.

- [s17] THE CROSS-JUMP LAW, read from BB2_XJUMP_DEBUG on the instrumented tools/gcc-2.7.2/jump.c rather than guessed: on tC find_cross_jump matched 6 insns then PAT-MISMATCH i1=432 (the ARM's own `sh $v0,0x286($s0)`) vs i2=240 (path1's `move $a3,$zero`), and do_cross_jump merged only from the jal (newjpos=447 newlpos=242). path1's store is pinned by the do_calls label; the arm's store is unpinned so sched1 sinks it to just before the arm's jal. A duplicated call pair merges 100% ONLY when the two blocks are scheduled IDENTICALLY - both stores pinned or both sunk. This supersedes E-s13-3's empirical coupling with a mechanism and explains every measurement from s11 onward.

- [s17] NEW, CHEAPER STORE-PIN PRIMITIVE: respelling path1 so EACH selection edge does its own store and the 0xB edge ends with `goto do_calls` makes path1 supply its own pin label - the arm's goto is no longer spent on it - and emitted 80-88 comes out BYTE-EXACT with target (jump2 cross-jumps the two per-edge stores back into one). Use this instead of E-s13-2's arm-side `goto do_calls` from now on.

- [s17] tD (that path1 + the arm pinned the same way) = 11 / 212, THREE INSNS SHORT of target: with both sides pinned identically the arm's two edges become textually identical to path1's and jump2 collapses the entire `<` arm (emitted `bnez $v0` where target has `beqz $v0`). Target's window is narrow - path1 pinned, arm pinned, AND the arm's 0x19 edge textually distinguishable from path1's 0x19 edge; in target's asm path1's 0x19 edge has NO store of its own (it branches to .L8002851C) while the arm's does.

- [s17] Full measurement table this session, all banked under memory/grind/func_800283D0/rejected/: base 10/216, tA 17/216, tB 17/222, tC 12/221, tD 11/212, tE (tD's path1 + the s16 arm) 12/221, tG (arm rewritten 0x19-edge-first with an `arm_calls:` pin) 10/216 and codegen-equivalent to base, tH (tD's path1 + tG's arm) 12/221.

## s18 (forensics, 2026-08-27) - floor HELD at 10 / 216; cluster A's SECOND route (livelen) is CONFIRMED WORKING and priced, and cluster B's dbr refusal is NAMED

**E-s18-0 (chassis).**  The banked s16/s17 candidate body re-measured **10 / 216**
on HEAD at session start and again at session end (E-s13-6 staleness discipline).
tA re-measured **17 / 216**.  Every s12-s17 number is chassis-valid.

**E-s18-1 (the complete ref map for the two cluster-A allocnos, read out of the
.flow dump rather than inferred).**  On tA:

    pseudo 143 (temp_s3) - 3 refs, ALL of them:
      insn 277  (set (reg/v:SI 143) (plus:SI (reg/v:SI 72) (reg:SI 145)))   <- def, arg0 + temp_a1_2*2
      insn 280  (set (reg/v:HI 146) (mem:HI (plus 143 648)))                <- temp_v0_3
      insn 321  (set (reg:HI 158)   (mem:HI (plus 143 648)))                <- the `== 5` reload
    pseudo 73 (arg1) - 7 refs, ALL of them:
      insn 6    (set (reg/v:SI 73) (reg:SI 5 a1))                           <- the param copy
      insn 238 / 253   (set (reg:SI 6 a2) (reg 73))   <- path1's calls 1 / 0x25
      insn 299 / 314   (same)                         <- the `==` arm's calls
      insn 443 / 458   (same)                         <- the `>` path's calls 0x26 / 0x2D

There is no fourth site.  Every arg1 reference in this function is a call
argument, and every temp_s3 reference is the def plus the two `0x288` loads
target's own asm also performs (`addu $s3,$v0,$s0`, `lh $v0,0x288($s3)`,
`lh $v1,0x288($s3)`).  Target therefore emits the SAME ref counts we do, which
means the seat exchange cannot be explained by target having "one more arg1
use" that is visible in the asm - see E-s18-6.

**E-s18-2 (allocno_compare on tA, digit-exact, and the corrected threshold set).**
ALLOCDBG on tA:

    ord=11 pseudo=72  $s0 nrefs=19 livelen=156 pri=4871
    ord=14 pseudo=77  $s1 nrefs=9  livelen=74  pri=3648
    ord=20 pseudo=143 $s2 nrefs=3  livelen=14  pri=2142     <- target wants $s3 here
    ord=22 pseudo=73  $s3 nrefs=7  livelen=92  pri=1521     <- target wants $s2 here

pri = floor_log2(nrefs)*nrefs/livelen*10000*size.  On THIS body livelen(73) is
**92**, not the 98 s15 recorded, so the single-atom threshold set is:

    nrefs(73) >= 8        -> 3*8/92*10000  = 2608 > 2142   (E-s17-4's route)
    nrefs(143) = 2        -> 1*2/14*10000  = 1428 < 1521   (newly in range)
    livelen(143) >= 20    -> 30000/20      = 1500 < 1521   (E-s15-5's route)
    livelen(73) <= 65     -> 14/65*10000   = 2153 > 2142   (never reached; see E-s18-5)

**E-s18-3 (THE FINDING: livelen(143) >= 20 FLIPS THE SEATS - first body ever to
reach target's callee-saved assignment WITHOUT a duplicated call pair).**
`tmp/grind/func_800283D0/s18/V6.c` = tA with the two `(u32)(*(u16 *)(... + 0xE)
- 6) < 2U` sub-expressions hoisted out of the `== 5` block into two `s32 c0, c1`
locals evaluated immediately after the shared call pair.  ALLOCDBG on V6:

    ord=23 pseudo=73  $s2 nrefs=7 livelen=92 pri=1521      <- TARGET SEAT
    ord=24 pseudo=143 $s3 nrefs=3 livelen=20 pri=1500      <- TARGET SEAT

and the normalized objdump diff has **NO cluster-A entry at all** - the twelve
`move $a2,$s2` sites and the prologue `sw $s2`/`sw $s3` pair are byte-exact,
and emitted 96 becomes `addu s3,s0,v0` vs target's `addu s3,v0,s0` (right
register, commuted operands).  E-s15-5's livelen route is therefore REAL and
independent of the ref-count route: cluster A has two disjoint solutions.

**E-s18-4 (the livelen dial is LINEAR in displaced insns, measured over five
bodies - 1 insn moved between the last `jal` and the `== 5` reload = +1
livelen).**

    body   what was hoisted above the `== 5` reload            livelen(143)  seats
    tA     nothing                                              14           no
    V8     the `!= 5` 0x19/0xB selection (3 insns)               17           no
    V5     the two `0xE` loads + their `-6` (4 insns)            18           no
    V9     V8 + the two bare `0xE` loads (5 insns)               19           no
    V10    V8 + one `0xE` load + one load-and-subtract (6)       20           YES
    V6     the two full `(x-6) < 2U` booleans (6 insns)          20           YES

The threshold is exactly 20 as predicted, and reg_live_length counts one per
insn scheduled inside the def-to-last-use span.  This makes the route
PRICEABLE for the first time.

**E-s18-5 (THE PRICE, and why the livelen route still loses on this chassis).**
V6 measures **19 / 214** and V10 measures **38 / 207**.  V6's residual is
cluster B plus ~14 diff slots concentrated in emitted 111-127: target computes
the two `0xE` booleans AFTER the `== 5` reload, we compute them before, so six
hoisted insns cross the four-insn reload/compare group and ~10-14 emitted
positions shift.  V10 is worse still because hoisting the shared 0x19/0xB
selection lets jump2 dedupe target's two IDENTICAL 4-insn selection blocks
(.L800285CC and .L800285F8), costing 8 insns (207 vs 215).  The arithmetic:
tA is 17 of which cluster A is ~12, so a seat-flipped tA is worth ~5 - and the
cheapest possible displacement (6 insns across a 4-insn group) prices at ~10.
**On a store-pinned body the livelen route is measured NET-NEGATIVE vs the
unpinned 10-floor base**, which is why the floor held.

**E-s18-6 (why target's seats cannot be explained by refs or by this livelen
dial - the jump2-runs-after-RA corollary).**  E-s18-1 shows target's emitted
ref counts equal ours and E-s18-4 shows target's emitted insn ORDER gives
livelen(143) = 14, so under our C target's own compile would allocate
143 -> $s2.  It does not.  The only remaining explanation is that target's
SOURCE contains refs that no longer exist in target's ASM: **jump2 (cross-jump)
runs AFTER global_alloc/reload in GCC 2.7.2's rest_of_compilation**, so a
duplicated call block raises `reg_n_refs` at allocation time and can still be
merged to zero emitted cost afterwards.  This is the mechanism behind the whole
s11-s17 duplicated-call family, stated for the first time as a pass-order fact
rather than an empirical coincidence.

**E-s18-7 (spellings that CANNOT move nrefs(143), measured).**
  - V3 (`*(s16 *)(arg0 + temp_a1_2*2 + 0x288) == 5`, recomputing the address
    at the second load) and V4 (recomputing at the FIRST load instead) are both
    exactly ALLOCDBG-identical to tA: nrefs 3, livelen 14, pri 2142.  cse2
    re-unifies the recomputed address with the pointer pseudo in both
    directions.  The s14 "recompute at the == 5 test" hypothesis is now KILLED
    on this chassis in both of its spellings.
  - V7 (`s16 *base288 = (s16 *)(arg0 + 0x288); ... base288[temp_a1_2] == 5`)
    DOES split the pseudo - 145 (3 refs) and 146 (2 refs) - but creates an
    extra live pointer, pushes the function to EIGHT callee-saved allocnos
    ($s0..$s7, arg1 landing in $s4) and costs a prologue save.  Splitting the
    address to lower nrefs is self-defeating.
  - V1 (declaring `u8 *temp_s3` at the head of block_20, above the
    `temp_v1_3 == 0` test) is worse than either: cse merges it with the `== 0`
    path's identical address, producing ONE pseudo (123) with nrefs 4 and
    livelen 16 -> pri 5000, which outranks arg0 and takes $s0.  The s15
    "hoist the pointer decl to lengthen its range" hypothesis is KILLED: you
    cannot hoist the def past the branch without merging the two paths'
    addresses, and the merged pseudo's priority explodes.

**E-s18-8 (CLUSTER B's dbr refusal is NAMED: `insn_sets_resource_p (trial,
&opposite_needed)`, i.e. mark_target_live_regs - NOT LABEL_NUSES).**
`BB2_DBR_DEBUG=1` on the instrumented `tools/gcc-2.7.2/cc1` over the 10-floor
base body (log `tmp/grind/func_800283D0/s18/dbr_base.txt`, RTL
`tmp/grind/func_800283D0/s18/code6cac_b.i.dbr`) resolves frontier item 3's
binary question.  The branch is `jump_insn 344` = `beqz $v0,<label 365>` (the
`(u32)(*(u16 *)(temp_s4 + 0xE) - 6) < 2U` test), and the insn target steals is
`insn 368` = `(set (reg:HI 2 v0) (const_int 1))`, the head of label 365's block
(the `D_800A38A8 = 1` block).  `fill_eager_delay_slots` -> `fill_slots_from_thread`
reports:

    DBRDBG thr insn=344 thread=368 opp=731 own=1 likely=0 tif=1
           oppregs=20630084_00000000 oppmem=1
    DBRDBG thr insn=344 trial=368 refset=0 setset=0 setneed=0 setsopp=1 trap=0
    DBRDBG thr LOSE insn=344 trial=368

Four of the five eligibility predicates PASS (`insn_references_resource_p
(trial,&set)` = 0, `insn_sets_resource_p (trial,&set)` = 0,
`insn_sets_resource_p (trial,&needed)` = 0, `may_trap_p` = 0).  The single
failing predicate is `insn_sets_resource_p (trial, &opposite_needed, 1)` = 1:
insn 368 writes `$v0`, and `$v0` (hard reg 2, bit 0x4 of the printed
`oppregs = 0x20630084`) is in `mark_target_live_regs (opposite_thread)`.  The
opposite thread is `insn 731`, the already-filled sequence
`(jump_insn 354: bnez $s1 -> 632)` + delay `(insn 351: v0 = 0x19)` - i.e. the
fall-through `.L800285CC`-equivalent selection block.  MIPS has no annulled
slots in this port, so a trial that clobbers an opposite-thread live register
can never be taken and the slot stays a nop.
`DBRDBG mtlr target=731 block=27` shows the block WAS found (no
`find_basic_block () == -1` fallback), so this is a genuine liveness answer, not
the conservative everything-live path - and the same `oppregs` word also
carries `$a3` (bit 7), a register that is provably dead there, so
`basic_block_live_at_start[27]` is over-approximate at this point in reorg.
**The C dial for cluster B is therefore what the FALL-THROUGH arm of that
branch does to `$v0` - the 0x19/0xB selection - not the number of gotos landing
on the D_800A38A8 block.**  The LABEL_NUSES alternative in the s17 frontier is
eliminated.

- [s18] Chassis: base = 10 / 216 at session start AND at session end; tA = 17 / 216. Floor HELD at 10.

- [s18] COMPLETE REF MAP (from the .flow dump, not inferred): pseudo 143 (temp_s3) has exactly 3 refs - def `(plus 72 145)` at insn 277, `temp_v0_3` load at 280, the `== 5` reload at 321. Pseudo 73 (arg1) has exactly 7 - the param copy at insn 6 plus six `(set (reg 6 a2) (reg 73))` call arguments at 238/253 (path1), 299/314 (the `==` arm), 443/458 (the `>` path). There is no other site for either pseudo anywhere in the function.

- [s18] allocno_compare on tA, digit-exact: 143 -> $s2 nrefs 3 livelen 14 pri 2142; 73 -> $s3 nrefs 7 livelen 92 pri 1521. livelen(73) is 92 on this body, NOT the 98 s15 recorded, so the corrected single-atom threshold set is nrefs(73)>=8, nrefs(143)=2 (pri 1428), livelen(143)>=20 (pri 1500), livelen(73)<=65.

- [s18] CLUSTER A HAS A SECOND, INDEPENDENT SOLUTION AND IT IS NOW MEASURED: V6 (tA with the two `(u32)(*(u16*)(...+0xE)-6) < 2U` booleans hoisted into `s32 c0, c1` evaluated right after the shared call pair) gives livelen(143) = 20, pri 1500 < 1521, and ALLOCDBG shows 73 -> $s2 and 143 -> $s3, i.e. TARGET'S SEATS, with nrefs(73) still 7. Its normalized diff has NO cluster-A entry: all twelve `move $a2,$s2` sites and the prologue sw pair are byte-exact. This is the first body in eighteen sessions to reach the target seats without a duplicated call pair.

- [s18] THE LIVELEN DIAL IS LINEAR AND NOW PRICED: one insn scheduled between the last `jal` and the `== 5` reload = +1 livelen(143). Measured ladder on tA: nothing 14; the `!=5` selection hoisted (3 insns) 17; the two 0xE loads + their -6 (4 insns) 18; selection + two bare loads (5) 19; selection + one load + one load-and-subtract (6) 20 FLIP; the two full booleans (6) 20 FLIP.

- [s18] THE PRICE OF THE LIVELEN ROUTE, AND WHY THE FLOOR HELD: V6 = 19 / 214, V10 = 38 / 207. Target computes the two 0xE booleans AFTER the `== 5` reload; any hoist moves six insns across the four-insn reload/compare group, shifting ~10-14 emitted positions. V10 is worse because hoisting the shared 0x19/0xB selection lets jump2 dedupe target's two IDENTICAL 4-insn selection blocks (.L800285CC / .L800285F8), losing 8 insns. A seat-flipped tA is worth ~5 (tA 17 minus cluster A ~12) and the cheapest displacement prices at ~10, so on a STORE-PINNED body the livelen route is NET-NEGATIVE against the unpinned 10-floor base.

- [s18] PASS-ORDER COROLLARY (explains the entire s11-s17 duplicated-call family as a mechanism, not a coincidence): jump2 runs AFTER global_alloc/reload in GCC 2.7.2's rest_of_compilation, so a duplicated call block raises reg_n_refs at ALLOCATION time and can still be cross-jumped to zero EMITTED cost afterwards. Combined with E-s18-1 (target's emitted ref counts equal ours) and E-s18-4 (target's emitted order gives livelen(143)=14), the only consistent reading of target's seats is that target's SOURCE carries a duplicated call pair that jump2 later merges away.

- [s18] KILLED - nrefs(143) 3 -> 2 is unreachable by respelling the address. V3 (recompute at the `== 5` test) and V4 (recompute at the FIRST load) are both ALLOCDBG-IDENTICAL to tA (3/14/2142): cse2 re-unifies the recomputed address in both directions. V7 (`s16 *base288 = (s16*)(arg0+0x288); base288[temp_a1_2] == 5`) does split the pseudo into 145 (3 refs) + 146 (2 refs) but pushes the function to EIGHT callee-saved allocnos with arg1 in $s4 and an extra prologue save. The s14 recompute hypothesis is dead in both spellings.

- [s18] KILLED - hoisting the temp_s3 declaration to the head of block_20 (the s15 birth-end lengthening) is not merely ineffective, it is destructive: cse merges the hoisted def with the `== 0` path's identical `arg0 + temp_a1_2*2` address into ONE pseudo (123) with nrefs 4 / livelen 16 / pri 5000, which outranks arg0 (pri 4871) and takes $s0, reshuffling every callee-saved seat. You cannot lengthen 143 from the birth end without merging the two paths' addresses.

- [s18] CLUSTER B IS NAMED (frontier item 3's binary question is ANSWERED - it is mark_target_live_regs, NOT LABEL_NUSES). BB2_DBR_DEBUG=1 on the base body: the branch is jump_insn 344 (`beqz $v0,<label 365>`, the `(u32)(*(u16*)(temp_s4+0xE)-6) < 2U` test) and the stolen insn target uses is insn 368 (`v0 = 1`, the head of the D_800A38A8 block). fill_slots_from_thread reports refset=0 setset=0 setneed=0 trap=0 and setsopp=1 -> LOSE: `insn_sets_resource_p (trial, &opposite_needed, 1)` is the SOLE failing predicate, because insn 368 writes $v0 and $v0 (bit 0x4 of oppregs=0x20630084) is live in mark_target_live_regs(opposite_thread=731), the already-filled `bnez $s1 -> 632` + delay `v0 = 0x19` selection sequence. `DBRDBG mtlr target=731 block=27` proves the block was found, so this is a real liveness answer rather than the find_basic_block()==-1 conservative fallback - though the same word also marks $a3 live, which is provably dead there, so basic_block_live_at_start[27] is over-approximate. MIPS has no annulled slots in this port, so the slot can never be filled while $v0 is in that set. The C dial is what the FALL-THROUGH 0x19/0xB selection arm does to $v0.

- [s18] Chassis: the banked s16/s17 candidate body measured 10 / 216 at session start AND at session end (E-s13-6 staleness discipline); tA re-measured 17 / 216. Floor HELD at 10.

- [s18] COMPLETE REF MAP, read out of the .flow dump rather than inferred: pseudo 143 (temp_s3) has exactly 3 refs - the def `(set (reg 143) (plus (reg 72) (reg 145)))` at insn 277, the temp_v0_3 load at 280, the `== 5` reload at 321. Pseudo 73 (arg1) has exactly 7 - the param copy `(set (reg 73) (reg 5 a1))` at insn 6 plus six `(set (reg 6 a2) (reg 73))` call arguments at 238/253 (path1), 299/314 (the `==` arm), 443/458 (the `>` path). No fourth site exists for either pseudo.

- [s18] allocno_compare on tA, digit-exact: 143 -> $s2 nrefs 3 livelen 14 pri 2142; 73 -> $s3 nrefs 7 livelen 92 pri 1521. livelen(73) is 92 on this body, NOT the 98 s15 recorded, so the corrected single-atom threshold set is nrefs(73)>=8, nrefs(143)=2 (pri 1428), livelen(143)>=20 (pri 1500), livelen(73)<=65.

- [s18] CLUSTER A HAS TWO DISJOINT SOLUTIONS. V6 reaches target's seats via livelen(143)=20 with nrefs(73) unchanged at 7, and its diff has no cluster-A entry at all; emitted 96 becomes `addu s3,s0,v0` vs target's `addu s3,v0,s0` (right register, commuted operands only).

- [s18] The livelen dial is LINEAR: one insn scheduled between the last `jal` and the `== 5` reload = +1 livelen(143). Ladder measured on tA: 0 insns -> 14, 3 -> 17, 4 -> 18, 5 -> 19, 6 -> 20 (flip).

- [s18] PRICE OF THE LIVELEN ROUTE: V6 = 19/214, V10 = 38/207. Target computes the two 0xE booleans AFTER the reload, so any hoist shifts ~10-14 emitted positions; a seat-flipped tA is worth only ~5. On a store-pinned body the route is measured NET-NEGATIVE against the unpinned 10-floor base.

- [s18] PASS-ORDER COROLLARY (explains the whole s11-s17 duplicated-call family as a mechanism rather than a coincidence): jump2 runs AFTER global_alloc/reload in GCC 2.7.2's rest_of_compilation, so a duplicated call block raises reg_n_refs at ALLOCATION time and can still be cross-jumped to zero EMITTED cost. Since target's asm shows the same 7/3 ref counts we produce and the same emitted order (livelen 14), the only consistent reading is that target's SOURCE carries a duplicated call pair that jump2 later merges away.

- [s18] KILLED: nrefs(143) 3->2 by address respelling. V3 and V4 are ALLOCDBG-identical to tA (3/14/2142) - cse2 re-unifies the recomputed address in both directions. V7's differently-based pointer does split the pseudo but adds an eighth callee-saved allocno and a prologue save.

- [s18] KILLED: birth-end lengthening of livelen(143). V1's hoisted declaration is cse-merged with the `== 0` path's identical address into one pseudo (nrefs 4, livelen 16, pri 5000) that outranks arg0 and takes $s0.

- [s18] CLUSTER B NAMED: jump_insn 344's empty delay slot is refused solely by insn_sets_resource_p(trial=368, &opposite_needed) - $v0 is live in mark_target_live_regs of the fall-through 0x19/0xB selection sequence (insn 731). The s17 frontier's LABEL_NUSES alternative is ELIMINATED. Note the same oppregs word (0x20630084) also marks $a3 live, a register provably dead there, so basic_block_live_at_start[27] is over-approximate at that point in reorg - which is itself the next lever.

- [s18] TOOLING for the next session: tmp/grind/func_800283D0/s18/apply.py swaps a body into src/code6cac_b.c whether the current state is INCLUDE_ASM or a C body; s18/al.sh <tag> prints the callee-saved ALLOCDBG table; s18/dbr.sh <tag> writes both the BB2_DBR_DEBUG log and the -da pass dumps into s18/; s18/mk.sh + s18/norm.py produce the normalized objdump-vs-target diff (run sandbox twice before objdumping, E-s13-6).

## s19 (forensics, 2026-08-27) - floor HELD at 10 / 216; CLUSTER B IS CONFIRMED CLOSABLE BY A PURE-C DIAL (first time in nineteen sessions), and the dbr refusal is resolved one level deeper than s18

**E-s19-0 (chassis).**  The banked s16/s17/s18 candidate body measured **10 / 216**
on HEAD at session start and twice again at session end (E-s13-6 staleness
discipline).  Every s12-s18 conclusion remains chassis-valid.  All s18 insn UIDs
reproduce exactly: `DBRDBG thr insn=344 thread=368 opp=731 own=1 likely=0 tif=1
oppregs=20630084_00000000 oppmem=1` / `trial=368 ... setsopp=1` / `LOSE`
(tmp/grind/func_800283D0/s19/dbr_base.txt lines 613-615).

**E-s19-1 (what `opposite_needed` actually is - it is NOT the fall-through
block's live-in, and s18's "basic_block_live_at_start[27] is over-approximate"
reading is CORRECTED).**  The pre-RA CFG is printed verbatim by `dump_flow_info`
in the `.lreg` dump.  For this function:

    Basic block 27: first insn 270, last 287.
      Registers live at start: 29 30 72 73 75 77 79 89 90 128
    Basic block 30: first insn 338, last 344.     <- ends with the beqz
      Registers live at start: 29 30 72 75 77 79
    Basic block 31: first insn 347, last 354.     <- the 0x19/0xB selection
      Registers live at start: 29 30 72 77 79
    Basic block 33: first insn 365, last 377.     <- the D_800A38A8 block
      Registers live at start: 29 30 79

Through `reg_renumber` (from the `.greg` dispositions: 72 to 16 s0, 73 to 18 s2,
75 to 20 s4, 77 to 17 s1, 79 to 22 s6, 89 to 5 a1, 90 to 21 s5, 128 to 3 v1)
NONE of those sets contains `$v0` or `$a3`.  `basic_block_live_at_start` is
therefore CLEAN - the poisoning happens later, in `mark_target_live_regs`'s
forward walk.

**E-s19-2 (why the walk starts ~74 insns too early: `find_basic_block` keys off
the previous BARRIER, not the nearest label).**  `reorg.c:2236 find_basic_block`
scans BACKWARD from the insn to the previous `BARRIER`, then forward over the
run of `CODE_LABEL`s that follows it, returning the first one that is a
`basic_block_head`.  Conditional branches emit no barrier, so from the
fall-through insn the scan runs back over the whole `==` arm and lands on
`code_label 270` = `basic_block_head[27]`.  That is exactly the `block=27` s18
recorded, and it is NOT the block that contains the fall-through (that is block
31, head `code_label 347`).  `mark_target_live_regs` then walks forward from
insn 270 all the way to the opposite thread.

**E-s19-3 (the walk's rule: a SET makes a register LIVE; a REG_DEAD note is
only honoured when a CODE_LABEL flushes it).**  In the walk body
(reorg.c:2618-2712) each real insn does `note_stores (PATTERN, update_live_status)`,
and `update_live_status` (reorg.c:2392) does `SET_HARD_REG_BIT (current_live_regs, i);
CLEAR_HARD_REG_BIT (pending_dead_regs, i);` for every SET destination.  REG_DEAD
notes only accumulate into `pending_dead_regs`, which is subtracted from
`current_live_regs` ONLY at a `CODE_LABEL`.  Consequently any register written
between the barrier-derived block head and the opposite thread is reported live
unless a label intervenes after its last write.  This is the mechanism behind
BOTH over-approximated bits s18 observed (`$v0` and `$a3`).

**E-s19-4 (the specific poisoner: `update_block`'s `(use (insn N))` marker).**
`jump_insn 344` DOES carry `REG_DEAD (reg:SI 2 v0)` and `code_label 347`
immediately follows it (s19/code6cac_b.i.sched2 and .dbr, insns 339/341/343/
344/347), so the label flush would clear `$v0` - if nothing after the label
re-set it.  Something does.  `fill_simple_delay_slots` runs before
`fill_eager_delay_slots` and fills the FALL-THROUGH block's own branch
(`jump_insn 354`, `bnez $s1`) by pulling its preceding insn `351`
(`(set (reg:HI 2 v0) (const_int 25))`) backward into the slot, producing
`sequence 731`.  `update_block` (reorg.c:2270) then leaves
`(use (insn 351))` at insn 351's ORIGINAL position - between `code_label 347`
and the new opposite thread.  The walk dereferences that USE to the real insn
(`real_insn = XEXP (PATTERN (insn), 0)`) and applies `note_stores`, so `$v0`
becomes live again after the label flush.  `insn_sets_resource_p (trial=368,
&opposite_needed)` is then 1 and the slot is refused.  Note `update_block`
returns EARLY without emitting a marker when `INSN_FROM_TARGET_P (insn)` - so
only fill_simple's backward steals poison a preceding branch, never fill_eager's
target steals.

**E-s19-5 (THE MEASURED CONFIRMATION - body `W1`, the first body in nineteen
sessions to close cluster B).**  tmp/grind/func_800283D0/s19/W1.c is the
banked candidate with ONE change: the `var_v0_2 = 0x19;` initialisation is
hoisted out of the fall-through selection block to just above the `||` test,

    var_v0_2 = 0x19;
    if (((u32)(*(u16 *)(arg0 + 0xE) - 6) < 2U) || ((u32)(*(u16 *)(temp_s4 + 0xE) - 6) < 2U)) {
        if (var_s1 == 0) { var_v0_2 = 0xB; }
        goto block_48;
    }
    D_800A38A8 = 1; D_800A3876 = -1; goto block_49;

so the fall-through block no longer owns a `$v0` set for fill_simple to steal.
`BB2_DBR_DEBUG=1` on W1 (s19/dbr_W1.txt lines 603-604):

    DBRDBG thr insn=347 thread=368 opp=354 own=1 likely=0 tif=1
           oppregs=20630088_00000000 oppmem=1
    DBRDBG thr insn=347 trial=368 refset=0 setset=0 setneed=0 setsopp=0 trap=0

`oppregs` loses bit 0x4 (`$v0`) and gains bit 0x8 (`$v1`); the opposite thread
is now the PLAIN `jump_insn 354` instead of the filled `sequence 731`;
`setsopp` is 0 and there is NO `LOSE` line.  dbr steals `v0 = 1` into the
branch's delay slot exactly as target does, and the normalized diff has NO
entry at the `beqz v0 / li v0,1` pair.  **Cluster B is a real, reachable,
pure-C-controllable divergence - it is not a compiler wall.**

**E-s19-6 (the price of W1, measured: 32 / 216, and the two costs are
separable).**  Normalized diff vs target:
  - emitted 120 ours `li v1,25` vs target `nop`, and emitted 128 ours `nop` vs
    target `li v0,25`: the hoist BUYS `jump_insn 344`'s slot by GIVING UP
    `jump_insn 354`'s slot.  Target has BOTH filled.
  - emitted 159-203 (~20 points): with the initialisation hoisted above the
    `||`, `var_v0_2` no longer lives in `$v0` across the tail; the whole
    `>` path's 0x1A / 0x13..0x16 selection and the multiply block are emitted
    with `$v1` where target uses `$v0` (`li v1,26`, `li v1,20`, `li v1,19`,
    `sh v1,646(s0)`, plus a `move v1,a1` / `negu v1,a1` reshuffle).
The second cost is incidental to THIS spelling (a fresh local scoped to the
`== 5` subtree should avoid it); the first is structural - see E-s19-7.

**E-s19-7 (the structural tension, and why target cannot be reached by simply
hoisting).**  `fill_simple_delay_slots`'s backward search is bounded by
`stop_search_p`, which stops at a `CODE_LABEL`.  So whatever fills
`jump_insn 354`'s slot from before the branch MUST come from between
`code_label 347` and the branch - i.e. its `update_block` marker ALWAYS lands
inside the walk window that poisons `jump_insn 344`.  Under this code path
slots 344 and 354 are MUTUALLY EXCLUSIVE for any body whose fall-through
selection block owns its own `$v0` write, which is exactly what W1's 120/128
pair measures.  Target has both filled, so target's RTL must differ in one of
exactly three ways: (a) `jump_insn 354`'s slot is filled by fill_EAGER from its
target (`INSN_FROM_TARGET_P`, so `update_block` emits no marker at all), or
(b) the insn that fills 354's slot writes a register other than `$v0`, or
(c) `code_label 347` does not exist at reorg time so the two blocks are one.
(c) is refuted by target's own asm (`.L800285CC` is the target of the first
0xE test's `bnez`), so the next session's question is (a) vs (b).

- [s19] Chassis: the banked candidate measured 10 / 216 at session start and twice at session end. Floor HELD at 10.

- [s19] CORRECTION to E-s18-8's reading: `basic_block_live_at_start` is NOT over-approximate here. The pre-RA CFG dump (dump_flow_info in the .lreg dump) shows block 27 live-in = pseudos 29 30 72 73 75 77 79 89 90 128, which renumber to sp/fp/s0/s2/s4/s1/s6/a1/s5/v1 - no $v0 and no $a3. Both bits are ADDED by mark_target_live_regs' forward walk.

- [s19] find_basic_block (reorg.c:2236) resolves a block by scanning back to the previous BARRIER, then forward over the following run of CODE_LABELs. Conditional branches emit no barrier, so the fall-through of jump_insn 344 resolves to block 27 (head code_label 270) rather than to its own block 31 (head code_label 347), and the liveness walk therefore runs across the entire `==` arm.

- [s19] The walk's rule (reorg.c:2618-2712 + update_live_status at reorg.c:2392): any SET destination becomes LIVE; REG_DEAD notes only accumulate into pending_dead_regs and are subtracted from the live set ONLY at a CODE_LABEL. jump_insn 344 does carry REG_DEAD (reg:SI 2 v0) and code_label 347 immediately follows it, so the flush works - and is then undone.

- [s19] THE POISONER IS NAMED: update_block's `(use (insn 351))` marker. fill_simple_delay_slots (which runs BEFORE fill_eager) fills the fall-through block's own branch jump_insn 354 (`bnez $s1`) by stealing its preceding insn 351 (`set (reg:HI 2 v0) (const_int 25)`) backward into the slot; update_block leaves a `(use (insn 351))` marker at 351's old position, between code_label 347 and the new opposite thread. mark_target_live_regs dereferences the USE to the real insn and note_stores marks $v0 live again, so insn_sets_resource_p(trial=368, &opposite_needed) = 1 and jump_insn 344's slot is refused. update_block emits NO marker when INSN_FROM_TARGET_P, so only fill_simple's backward steals can poison a preceding branch.

- [s19] CLUSTER B IS CONFIRMED CLOSABLE IN PURE C (body W1, s19/W1.c): hoisting `var_v0_2 = 0x19;` above the `||` test removes the fall-through block's own $v0 write; oppregs goes 0x20630084 -> 0x20630088 (bit 0x4 gone), the opposite thread becomes the plain jump_insn 354 instead of the filled sequence 731, setsopp becomes 0, there is no LOSE line, and dbr steals `v0 = 1` into the delay slot byte-exactly as target does. Nineteen sessions of "no measured C dial for cluster B" is over.

- [s19] W1's PRICE: 32 / 216. Two separable costs. (i) STRUCTURAL: emitted 120/128 show the hoist buys jump_insn 344's slot by giving up jump_insn 354's (`li v1,25` where target has `nop`; `nop` where target has `li v0,25`). (ii) INCIDENTAL: ~20 points at emitted 159-203 because the hoisted initialisation moves var_v0_2 out of $v0 for the whole `>` path tail (li v1,26 / li v1,20 / li v1,19 / sh v1,646(s0) and a move/negu reshuffle). Cost (ii) should be removable with a fresh local scoped to the `== 5` subtree.

- [s19] THE REMAINING STRUCTURAL TENSION: fill_simple_delay_slots' backward search is bounded by stop_search_p, which stops at a CODE_LABEL, so any insn that fills jump_insn 354's slot from before the branch necessarily sits between code_label 347 and the branch - i.e. its update_block marker always lands in the window that poisons jump_insn 344. Slots 344 and 354 are therefore MUTUALLY EXCLUSIVE for any body whose fall-through selection block owns its own $v0 write. Target has BOTH filled, so target differs in one of exactly three ways: (a) 354's slot is filled by fill_eager from its TARGET (INSN_FROM_TARGET_P leaves no marker), (b) the filling insn writes a register other than $v0, or (c) code_label 347 does not exist at reorg time. (c) is refuted by target's own asm.

- [s19] Chassis: the banked s16/s17/s18 candidate body measured 10 / 216 at session start and twice again at session end (E-s13-6 staleness discipline). All s18 insn UIDs and the oppregs word 0x20630084 reproduce exactly. Floor HELD at 10.

- [s19] The pre-RA CFG for this function, read verbatim from dump_flow_info in the .lreg dump: block 27 first insn 270 last 287 live-in {29 30 72 73 75 77 79 89 90 128}; block 30 first 338 last 344 live-in {29 30 72 75 77 79}; block 31 first 347 last 354 live-in {29 30 72 77 79}; block 33 first 365 last 377 live-in {29 30 79}. Renumbering (.greg dispositions: 72->16 s0, 73->18 s2, 75->20 s4, 77->17 s1, 79->22 s6, 89->5 a1, 90->21 s5, 128->3 v1) shows no set contains $v0 or $a3.

- [s19] find_basic_block (reorg.c:2236) resolves a block by scanning back to the previous BARRIER and then forward over the following run of CODE_LABELs. Conditional branches emit no barrier, so the fall-through of jump_insn 344 resolves to block 27 (head code_label 270) and mark_target_live_regs' forward walk therefore runs across the whole `==` arm.

- [s19] The walk's rule (reorg.c:2618-2712 with update_live_status at reorg.c:2392): every SET destination becomes LIVE and clears its own pending-dead bit; REG_DEAD notes only accumulate into pending_dead_regs and are subtracted from current_live_regs ONLY at a CODE_LABEL. jump_insn 344 does carry REG_DEAD (reg:SI 2 v0) and code_label 347 immediately follows it, so the flush happens - and is then undone by a later set.

- [s19] THE POISONER IS NAMED: update_block's `(use (insn 351))` marker. fill_simple_delay_slots runs before fill_eager_delay_slots and fills the fall-through block's own branch jump_insn 354 (`bnez $s1`) by stealing its preceding insn 351 (`set (reg:HI 2 v0) (const_int 25)`) backward into the slot, producing sequence 731; update_block leaves the USE marker at 351's old position, between code_label 347 and the new opposite thread. update_block returns EARLY without emitting a marker when INSN_FROM_TARGET_P, so only fill_simple's backward steals can poison a preceding branch.

- [s19] CLUSTER B IS CONFIRMED CLOSABLE IN PURE C (body W1, tmp/grind/func_800283D0/s19/W1.c): oppregs 0x20630084 -> 0x20630088, opposite thread becomes the plain jump_insn 354 instead of the filled sequence 731, setsopp 1 -> 0, no LOSE line, and dbr steals `v0 = 1` into jump_insn 344's delay slot byte-exactly as target does.

- [s19] W1's PRICE is 32 / 216 and splits into two separable costs. STRUCTURAL: emitted 120 `li v1,25` vs target `nop` and emitted 128 `nop` vs target `li v0,25` - the hoist buys slot 344 by giving up slot 354. INCIDENTAL (~20 points, emitted 159-203): the hoisted initialisation moves var_v0_2 out of $v0 for the whole `>` path tail (`li v1,26`, `li v1,20`, `li v1,19`, `sh v1,646(s0)`, plus a `move v1,a1` / `negu v1,a1` reshuffle). The incidental cost should be removable with a fresh local scoped to the `== 5` subtree.

- [s19] fill_simple_delay_slots' backward search is bounded by stop_search_p, which stops at a CODE_LABEL - so the filler of jump_insn 354's slot can only come from between code_label 347 and the branch, and its update_block marker always lands in the poisoning window. Slots 344 and 354 are mutually exclusive for any body whose fall-through selection block owns its own $v0 write.

- [s19] src/code6cac_b.c was restored to the banked candidate body before the session ended and re-measured 10 / 216 twice.


## s20 (rederive, 2026-08-27) - floor HELD at 10 / 216; the s19 cluster-B frontier is MEASURED AND PRICED: the incidental cost is real and removable (32 -> 12), the structural cost is NOT, so closing cluster B remains net-negative at -2

**E-s20-0 (chassis).**  The banked s16-s19 candidate body measured **10 / 216** on
HEAD at session start and twice again at session end (E-s13-6 staleness
discipline).  The normalized objdump diff reproduces the s19 residual map
exactly (tmp/grind/func_800283D0/s20/diff_base.txt): emitted 45-47 (the
`li v0,1` / `j` / `nop` vs target's `nop` / `j` / `li v0,1` slot swap, ~2 pts),
83-88 (the store sink, ~2 pts), 126 + 131-133 (cluster B, ~3 pts AND the entire
216-vs-215 insn surplus - the surplus is ONE insn, the un-stolen `li v0,1`),
148 (the arm store, ~1 pt), 96 and 162 (two commutative `addu` operand orders,
~1 pt each).  Every s12-s19 conclusion remains chassis-valid.

**E-s20-1 (THE HEADLINE: s19's frontier item 1 is CONFIRMED on its incidental
half and KILLED on its structural half).**  s19 predicted that W1's 32 / 216
splits into a ~20-point INCIDENTAL cost (var_v0_2 leaving `$v0` across the `>`
path tail) plus a structural slot trade, and that "a fresh local scoped to the
`== 5` subtree" would remove the incidental part.  Measured, four spellings:

| body | shape of the `== 5` subtree | score / insns |
|---|---|---|
| base | `var_v0_2 = 0x19;` INSIDE the `\|\|`-true block | 10 / 216 |
| s19 W1 | `var_v0_2 = 0x19;` hoisted above the `\|\|` | 32 / 216 |
| **W2b** | **`s16 sel5 = 0x19;` above the `\|\|`; `if (var_s1==0) sel5=0xB;` then `var_v0_2 = sel5; goto block_48;`** | **12 / 216** |
| W2a | same, but the arm stores `*(s16*)(arg0+0x286)=sel5` itself and `goto block_49` | 12 / 216 |
| W2c | W2b with `s32 sel5` instead of `s16` | 12 / 216 |
| W2d | W2b with `var_v0_2 = sel5;` written BEFORE the `var_s1` test | 10 / 216 (sel5 const-folds away; byte-identical to base) |

The prediction is CONFIRMED: the fresh scoped local removes the whole ~20-point
`>` path renaming (32 -> 12).  The prediction that the remainder closes "for
FREE" is REFUTED: **W2b is 12, base is 10, so closing cluster B costs a net +2.**

**E-s20-2 (what W2b actually buys and what it pays - the normalized diff).**
W2b's diff has NO entry at emitted 126: `li v0,1` sits in the `beqz`'s delay
slot exactly as target, i.e. **cluster B is CLOSED byte-exactly in a body that
is otherwise the banked candidate.**  What W2b pays instead:
  - emitted 120 ours `li v1,25` vs target `nop` - the structural slot trade
    E-s19-7 named (buying `jump_insn 344`'s slot gives up `jump_insn 354`'s).
    This survives the fresh-local respelling, confirming it is a property of
    fill_simple's `stop_search_p` window, not of the variable.
  - emitted 128-129 ours `move v0,v1` / `li v1,11` vs target `li v0,25`, and
    emitted 131 ours `move v0,v1` - `sel5` is allocated `$v1` (its live range
    now spans the `||` test, which itself computes in `$v0`), so the
    `sel5 -> var_v0_2` copy does not coalesce and materializes twice.
So the +2 is: one lost delay slot (+1 vs the +3 cluster B refunds is a net -3
... measured net is +2, i.e. the two `move v0,v1` copies cost more than the
cluster-B refund).

**E-s20-3 (why `sel5` cannot be made to live in `$v0`, and why target's
mechanism must therefore be DIFFERENT from the hoist).**  Target's own asm
carries NO materialization of `0x19` before the `||` test at all: target
emitted 126-130 is `li v0,1` (in the `beqz` slot) / `bnez s1,.L800286F8` /
`li v0,25` (in the `bnez` slot) / `j .L800286F8` / `li v0,11` (in the `j`
slot).  The `li v0,25` in `jump_insn 354`'s slot is a BACKWARD steal from
inside the block - which is exactly the steal whose `update_block` marker
E-s19-4 proved poisons `jump_insn 344`.  Target therefore closes BOTH slots
with the marker present, so target's route is NOT "remove the block's own `$v0`
write" (W1/W2b's route).  The hoist family can reach cluster B but can never
reach target's emitted code, because target has no insn to hoist.

**E-s20-4 (the disjoint-path variable split is score-NEUTRAL: jump2 re-merges
it, so it cannot isolate a live range).**  Hypothesis: the `== 5` subtree and
the `>` path are on disjoint control-flow paths, so giving the `>` path its own
local (`s16 var_v0_5` + its own `*(s16*)(arg0+0x286)` store + `goto block_49`)
should stop var_v0_2's extended live range from renaming the `>` path.
Measured: **V5 (split alone, no hoist) = 10 / 216 - byte-identical to base**,
because jump2 cross-jumps the two stores back together and the two pseudos
coalesce.  **V4 (split + W1's hoist) = 15 / 218** - better than W1's 32 but two
insns LONGER and still 5 worse than base.  The split is not a live-range lever
on this body; it is a no-op that the tail-merge undoes.

**E-s20-5 (inverting the `== 5` selection so the `0x19` edge is the branch
target is regressive).**  s19's frontier item 2 asked for a shape whose branch
slot can only be filled from its target.  Body W3 spells the selection as
`if (var_s1 != 0) { var_v0_2 = 0x19; goto block_48; } var_v0_2 = 0xB; goto
block_48;`.  Measured **13 / 216**: GCC does not lay the arms out in source
order here - it re-inverts the branch sense and the emitted block gains an
extra divergence at the `bnez`/`beqz` pair.  W2a shows the same effect (its
`beqz s1` / `li v0,11` pair is inverted relative to target's `bnez s1` /
`li v0,25`), i.e. **any respelling of this selection that moves the `0x19`
initialisation out of the fall-through position flips the emitted branch sense
away from target.**

**E-s20-6 (the emitted-45-47 slot swap is NOT an exit-form question).**  The
range-check's `return 1;` is emitted as `beq v1,v0,T` / `li v0,1` (slot) /
`j T` / `nop`; target is `beq v1,v0,T` / `nop` / `j T` / `li v0,1` (slot) -
same insn count, different branch owns the `li`.  Body E4 respells the exit as
`goto ret_one;` with a trailing `ret_one: return 1;` label after `block_13`.
Measured **37 / 215**.  The insn count drops to target's 215, but the score
triples: the trailing label re-shapes the whole epilogue region.  The exit-form
dimension is closed for this residual (consistent with s10's D4/D5 kills on the
23-floor chassis, now re-confirmed on the 10-floor chassis).

- [s20] Chassis: the banked candidate measured 10 / 216 at session start and twice at session end. Floor HELD at 10. The 216-vs-215 insn surplus is exactly ONE insn - the `li v0,1` that target steals into `jump_insn 344`'s delay slot and we emit standalone.
- [s20] s19's frontier item 1 is MEASURED. The ~20-point incidental cost of W1 IS removable by a fresh local scoped to the `== 5` subtree (W1 = 32/216 -> W2b = 12/216), exactly as predicted. The claim that cluster B then closes "for FREE" is REFUTED: W2b is 12 against a base of 10, so closing cluster B is net -2.
- [s20] W2b (tmp/grind/func_800283D0/s20/W2b.c, banked at rejected/clusterB-freshlocal-sel5-copy-to-varv02-CLOSES-B-but-12.c) is the FIRST body that closes cluster B at a survivable price: its normalized diff has no entry at emitted 126, i.e. `li v0,1` is in the `beqz` delay slot byte-exactly as target. It is the reference body for any future cluster-B work.
- [s20] W2b's residual cost is two `move v0,v1` copies (emitted 128 and 131) plus the structural slot trade at emitted 120. `sel5` is allocated `$v1` because its live range spans the `||` test, which itself computes in `$v0`; the `sel5 -> var_v0_2` copy therefore does not coalesce. Spelling `sel5` as `s32` (W2c) does not change the seat: 12 / 216.
- [s20] TARGET DOES NOT USE THE HOIST. Target's asm has NO materialisation of 0x19 before the `||` test - its `li v0,25` sits in `jump_insn 354`'s delay slot as a backward steal from inside the block, i.e. WITH the `update_block` marker that E-s19-4 proved poisons `jump_insn 344`. Target closes both slots with the marker present. The whole hoist family (W1/W2a/W2b/W2c) can therefore close cluster B but can NEVER reproduce target's emitted code in this region; the next session must look for the reason the marker is harmless in target (cache staleness in `target_hash_table`/`bb_ticks`, or a different `find_basic_block` resolution for the marker's insn), not for a better hoist.
- [s20] The disjoint-path variable split is a NO-OP: giving the `>` path its own `s16 var_v0_5` plus its own `*(s16*)(arg0+0x286)` store and `goto block_49` measures 10 / 216, byte-identical to base (V5), because jump2 cross-jumps the two stores and the pseudos coalesce. Under W1's hoist the same split measures 15 / 218 (V4). A source-level variable split cannot isolate a live range on this body.
- [s20] Inverting the `== 5` selection so the 0x19 edge is the branch target (W3) = 13 / 216: GCC re-inverts the branch sense and diverges from target's `bnez s1`. Any respelling that moves the 0x19 initialisation out of the fall-through position flips the emitted branch sense away from target (also visible in W2a).
- [s20] Respelling the range-check exit as `goto ret_one;` with a trailing `ret_one: return 1;` after `block_13` = 37 / 215. The insn count reaches target's 215 but the epilogue region re-shapes and the score triples. The exit-form dimension is closed on the 10-floor chassis, re-confirming s10's D4/D5 kills.

- [s20] Chassis: the banked s16-s19 candidate measured 10 / 216 on HEAD at session start and twice again at session end (E-s13-6 staleness discipline). Floor HELD at 10. Every s12-s19 conclusion remains chassis-valid.

- [s20] The 216-vs-215 insn surplus is exactly ONE insn: the `li v0,1` that target steals into jump_insn 344's delay slot and that we emit standalone. Closing cluster B closes the surplus.

- [s20] s19's frontier item 1 is MEASURED. The ~20-point incidental cost of W1 IS fully removable by a fresh local scoped to the `== 5` subtree (W1 = 32/216 -> W2b = 12/216), exactly as s19 predicted. The claim that cluster B then closes 'for FREE' is REFUTED: W2b is 12 against a base of 10, so closing cluster B is net -2.

- [s20] W2b (tmp/grind/func_800283D0/s20/W2b.c, banked at memory/grind/func_800283D0/rejected/clusterB-freshlocal-sel5-copy-to-varv02-CLOSES-B-but-12.c) is the FIRST body that closes cluster B at a survivable price: its normalized diff has no entry at emitted 126, i.e. `li v0,1` is in the beqz delay slot byte-exactly as target. It is the reference body for any future cluster-B work.

- [s20] W2b's residual cost is two `move v0,v1` copies (emitted 128 and 131) plus the structural slot trade at emitted 120. sel5 is allocated $v1 because its live range spans the `||` test, which itself computes in $v0, so the sel5 -> var_v0_2 copy does not coalesce. Spelling sel5 as s32 (W2c) does not change the seat: 12 / 216.

- [s20] TARGET DOES NOT USE THE HOIST. Target's asm has NO materialisation of 0x19 before the `||` test - its `li v0,25` sits in jump_insn 354's delay slot as a backward steal from inside the block, i.e. WITH the update_block marker that E-s19-4 proved poisons jump_insn 344. Target closes both slots with the marker present. The next session must look for the reason the marker is harmless in target (mark_target_live_regs' target_hash_table / bb_ticks cache staleness, or a different find_basic_block resolution for the marker's insn), not for a better hoist.

- [s20] The disjoint-path variable split is a NO-OP: giving the `>` path its own s16 var_v0_5 plus its own *(s16*)(arg0+0x286) store and `goto block_49` measures 10 / 216, byte-identical to base (V5), because jump2 cross-jumps the two stores and the pseudos coalesce. Under W1's hoist the same split measures 15 / 218 (V4).

- [s20] Inverting the `== 5` selection so the 0x19 edge is the branch target (W3) = 13 / 216: GCC re-inverts the branch sense and diverges from target's `bnez s1`. Any respelling that moves the 0x19 initialisation out of the fall-through position flips the emitted branch sense away from target (also visible in W2a, 12 / 216).

- [s20] Respelling the range-check exit as `goto ret_one;` with a trailing `ret_one: return 1;` after block_13 = 37 / 215. Insn count reaches target's 215 but the epilogue region re-shapes. Exit-form dimension closed on the 10-floor chassis.

- [s20] The base residual map is unchanged and re-confirmed (tmp/grind/func_800283D0/s20/diff_base.txt): emitted 45-47 slot swap (~2), 83-88 store sink (~2), 126 + 131-133 cluster B (~3 plus the whole insn surplus), 148 arm store (~1), 96 and 162 commutative addu operand orders (~1 each).


## s21 (rederive, 2026-08-27) - cluster A reduced to a CLOSED-FORM inequality on the store-pinned chassis, and both of its solutions measured

Chassis re-measured at session start and at session end with candidate.c applied:
`sandbox func_800283D0 --disable all` = **10 / 216** (target 215).  Unchanged.

### E-s21-1  Target's `<` arm is variant A, byte-for-byte - it is NOT a duplicated-call arm

Read directly out of `asm/funcs/func_800283D0.s` (1-based line numbers as the
file is written, i.e. glabel is line 1):

```
 87:  bnez  $s1, .L8002851C      # path1's selection
 88:  addiu $v0, $zero, 0x19
 90:  addiu $v0, $zero, 0xB      # .L80028518
 92:  sh    $v0, 0x286($s0)      # .L8002851C   <- the store, ALONE in its block
 94:  addiu $a1, $zero, 0x1      # .L80028520   <- do_calls
 ...
159:  beqz  $s1, .L80028518      # the `<` arm
160:  addiu $v0, $zero, 0x19
161:  j     .L80028520
162:  sh    $v0, 0x286($s0)      # in the `j` delay slot
```

The arm is FOUR insns: a `goto set_0xB`, a local 0x19, and a `goto do_calls`
with the arm's OWN store in the delay slot.  It contains no calls at all.  This
DIRECTLY REFUTES the s16 attribution recorded in candidate.c's header ("the
duplicated call pair IS target's own mechanism for the cluster-A seat").  It is
not: target's `<` arm is exactly s13's variant A.

Corroborating ref census over the whole target body (`grep '$s2' / '$s3'`):
`$s2` = 1 def (`addu $s2,$a1,$zero` at insn 6) + 6 `addu $a2,$s2,$zero` call
arguments = **7 refs**; `$s3` = 1 def (`addu $s3,$v0,$s0`) + 2 `lh ...,0x288($s3)`
= **3 refs**.  There is no fifth call pair and no stack argument (frame 0x38 =
0x10 arg area + 8 callee-saved slots; no `sw` below 0x18($sp)), so target really
does compile with nrefs(arg1) = 7.

### E-s21-2  Body V1: porting target's arm onto the 10-floor chassis byte-CLOSES the store sink AND the arm store

V1 = candidate.c with the `<` arm's `store; call(1); call(0x25); return ret;`
replaced by `store; goto do_calls;`.  Measured **17 / 216**.

The normalized objdump diff (`tmp/grind/func_800283D0/s21/norm.py`) has NO entry
at emitted 83-88 and NO entry at emitted 148 - i.e. residual cluster 2 (the
store sink, ~2 pts) and residual cluster 3 (the arm store, ~1-2 pts), open since
s13, are both **byte-closed**.  Mechanism, now confirmed rather than inferred:
`goto do_calls` makes `do_calls` a REFERENCED label sitting between path1's
store and path1's call sequence, so the store is alone in its basic block and
sched1 has nowhere to sink it.  Without that goto the label is unreferenced,
GCC deletes it, store and call-argument setup share one block, and sched1 sinks
the store into the first `jal`'s delay slot (E-s13-1).  **The store pin requires
a source-level `goto do_calls` and nothing else supplies it.**

V1's entire remaining residual is cluster A (the $s2/$s3 seat swap: prologue
`sw` order, six `move a2,sX`, three `lh ...(sX)`) plus cluster B plus the two
known commutative `addu` operand orders.

### E-s21-3  Cluster A's closed form, verified digit-for-digit against tools/gcc-2.7.2/global.c

`global.c:635-656 allocno_compare` sorts allocnos by

        pri = (double)(floor_log2(n_refs) * n_refs) / live_length * 10000 * size

descending; the first-sorted of two conflicting allocnos takes $s2, the second
$s3.  Every ALLOCDBG line measured this session reproduces the formula exactly
(e.g. V1: 72 -> floor_log2(19)*19*10000/156 = 4871; 73 -> 2*7*10000/92 = 1521;
143 -> 1*3*10000/14 = 2142).

On the V1 (store-pinned) chassis the numbers are

        pseudo 143 (temp_s3)  nrefs=3  livelen=14  pri=2142  -> $s2   [WRONG]
        pseudo  73 (arg1)     nrefs=7  livelen=92  pri=1521  -> $s3   [WRONG]

so cluster A is exactly the inequality **pri(arg1-carrier) > 2142**, which has
precisely two solutions given livelen(temp_s3)=14 is immovable (E-s21-6):

  (a) nrefs(carrier) >= 8 at livelen 92  ->  floor_log2(8)*8*10000/92 = 2608
  (b) livelen(carrier) <= 65 at nrefs 7  ->  2*7*10000/65 = 2153

### E-s21-4  Solution (a) MEASURED (probe P2): one extra arg1 ref flips both seats and leaves an otherwise CLEAN diff

P2 = V1 + one extra `func_80032854(*(s16 *)(arg0 + 4), 1, arg1, (s16 *)0);` in
the `<` arm before the `goto do_calls`.  P2 is SEMANTICALLY WRONG (it calls
once too often) and exists only as a measurement; it is not banked as a form.

        pseudo  73 (arg1)     nrefs=8  livelen=97  pri=2474  -> $s2   [TARGET]
        pseudo 143 (temp_s3)  nrefs=3  livelen=14  pri=2142  -> $s3   [TARGET]

Score **13 / 222**.  Its normalized diff, after discounting the seven insns the
bogus call adds, contains ONLY: cluster B (emitted 45-48 `j`/`nop` inversion,
emitted 126 `nop` vs target's stolen `li v0,1`, and the surplus `li v0,1` at
131) and the single commutative `addu s3,s0,v0` vs `addu s3,v0,s0`.  **Nothing
else.**  This is the first measurement in 21 sessions showing that the store
pin and cluster A are simultaneously satisfiable, and it prices the prize: a
legitimate solution to the inequality on the V1 chassis lands at roughly
**4-6 / 215**, with only cluster B and one operand order left.

### E-s21-5  Solution (b) MEASURED (body P4): a later-defined arg1 carrier flips the seats with semantics intact - but pays the prologue

P4 = V1 + `u8 *p;` declared with the other block locals and assigned `p = arg1;`
at `block_15` (after the whole top range-check chain), with all six call sites
using `p`.  Semantics preserved.

        pseudo  91 (p)        nrefs=7  livelen=58  pri=2413  -> $s2   [TARGET]
        pseudo 144 (temp_s3)  nrefs=3  livelen=14  pri=2142  -> $s3   [TARGET]

Score **16 / 216**.  Everything from emitted 87 to the end matches target except
cluster B and the one `addu` operand order - the tail is as clean as P2's.  The
entire residual has MOVED INTO THE PROLOGUE, and the cause is structural: the
carrier's source operand is the incoming `$a1`, which must therefore stay live
from entry to block_15.  That pins `$a1` across the range-check chain, so the
chain's two scratch values are pushed one register each - ours `lhu a3,106(s0)`
/ `li t0,4` against target's `lhu a1,106(s0)` / `li a2,4`, cascading through the
`andi`/`beq`/`addiu` chain - and the `move s2,a1` lands at emitted 48 instead of
target's emitted 4.  ~10 points.

This is a HARD property of solution (b): ANY assignment point late enough to
give livelen <= 65 is by definition after the range-check chain, so `$a1` is
always pinned across it.  Solution (b) is therefore capped around 16 and cannot
reach target - **the reachable route is solution (a).**

### E-s21-6  Two ways of lengthening livelen(temp_s3) KILLED

Cluster A's third conceivable solution - drive pri(temp_s3) below pri(arg1) by
raising livelen(temp_s3) from 14 to >= 20 (3*10000/20 = 1500 < 1521) - is closed
from both directions on the V1 chassis:

  * **P1** (hoist `u8 *temp_s3 = arg0 + temp_a1_2*2;` above the `temp_v1_3 == 0`
    test and route path1's load through it): cse1 commons path1's load into the
    pointer, so nrefs goes 3 -> 4 while the live range stays SHORT
    (nrefs=4 livelen=16 pri=5000).  The pseudo then outranks `arg0` itself and
    takes $s0, displacing every callee-saved seat in the function.  Wrong
    direction by construction: any spelling that lengthens the range by moving
    the def up necessarily hands path1's load to the same pseudo.
  * **P3** (invert the three-way compare so the `!=` case is the if-body and the
    `==` handling is the tail, moving temp_s3's last use far later in both the
    source and the emitted layout): ALLOCDBG comes back **identical to V1
    digit-for-digit** (72: 19/156/4871, 77: 9/74/3648, 143: 3/14/2142,
    73: 7/92/1521).  flow.c's `reg_live_length` accumulates only over blocks in
    which the pseudo is actually live, and temp_s3 is dead on the `<` and `>`
    paths however they are laid out.  **Block reordering can never move
    livelen(temp_s3).**

Counting the target's own window confirms there is no room: between
`addu $s3,$v0,$s0` (insn 107) and `lh $v1,0x288($s3)` (insn 122) target has
def + load + branch + 2 calls (5 RTL insns each) + load = 14 RTL insns, exactly
what V1 measures.  Target does not use this solution either.

### E-s21-7  The duplicated-statement-into-arms route to solution (a) KILLED (P5)

P5 = V1 with the `>` path's `0x26` / `0x2D` call pair duplicated into both arms
of `if (var_s1 == 0)`, chosen because it is the only OTHER call site that could
supply extra arg1 references while leaving the `<` arm free to be target's pure
double-goto.  The allocation half works exactly as the closed form predicts -
nrefs(arg1) 7 -> 9, pri = 3*9*10000/100 = 2700 > 2142, seats become target's -
but the body scores **81 / 183**: the duplication is not byte-neutral, `temp_s5`
loses its callee-saved seat entirely (only six callee-saved allocnos survive and
`ret` slides $s6 -> $s5), and the `>` tail collapses to 183 insns against
target's 215.  The two arms diverge immediately after the duplicated calls, so
jump2 never gets a mergeable tail.

### E-s21-8  The open question this session hands forward, stated exactly

Target compiles with nrefs(arg1) = 7, livelen(arg1) = 92 (E-s21-1), a store
pinned by a source `goto do_calls` (E-s21-2), and temp_s3 at nrefs 3 /
livelen 14 (E-s21-6) - and under `allocno_compare` that combination gives
pri 1521 < 2142, i.e. the WRONG seats.  Since the seats in target's shipped
bytes are the right ones, one of those four measurements must differ in
target's RTL at `global_alloc` time from what the V1 body produces.  The only
mechanism that can hide an eighth arg1 reference from the FINAL asm while
exposing it to `regclass` is a duplicate that is deleted AFTER global_alloc -
i.e. by post-reload `jump2` cross-jumping (the s16 body proves that path works:
its duplicated arm call pair is merged post-RA and the insn count stays 216).
The next session's job is to find the source site whose duplicate cross-jumps
cleanly AND leaves the `<` arm's `goto do_calls` intact.  The `<` arm itself is
excluded (it can only do one of the two), and the `>` path is excluded by
E-s21-7.

- [s21] Chassis re-measured with candidate.c applied at session start AND at session end: sandbox --disable all = 10 / 216 against target's 215. Unchanged; the ledger's recorded floor is current.

- [s21] Target's `<` arm (asm insns 159-162) is `beqz $s1,.L80028518 / addiu $v0,$zero,0x19 / j .L80028520 / sh $v0,0x286($s0)` - four insns, no calls: a source `goto set_0xB` plus a `goto do_calls` with the arm's own store in the jump's delay slot. This is s13's variant A verbatim and it refutes the s16 attribution recorded in candidate.c's header.

- [s21] Whole-body register census of target: $s2 = 1 def (`addu $s2,$a1,$zero`, insn 6) + six `addu $a2,$s2,$zero` call arguments = 7 refs. $s3 = 1 def (`addu $s3,$v0,$s0`) + two `lh ...,0x288($s3)` = 3 refs. Frame 0x38 = 0x10 argument area + 8 callee-saved slots, no `sw` below 0x18($sp), so func_80032854 takes four register arguments and there is no hidden fifth call site.

- [s21] Body V1 (target's arm ported onto the 10-floor body) = 17 / 216 with NO normalized-diff entry at emitted 83-88 or 148: the store sink and the arm store, open since s13, are byte-closed. The pin is that `goto do_calls` keeps a referenced label between path1's store and path1's calls, leaving the store alone in its basic block where sched1 cannot sink it.

- [s21] global.c:635-656 allocno_compare sorts by pri = floor_log2(n_refs)*n_refs/live_length*10000*size, first-sorted of two conflicting allocnos takes $s2. Verified digit-for-digit on six bodies this session. On V1: temp_s3 pri 2142 -> $s2 and arg1 pri 1521 -> $s3, both inverted from target.

- [s21] Cluster A therefore has exactly two solutions given livelen(temp_s3)=14 is immovable: nrefs(carrier) >= 8 at livelen 92 (pri 2608), or livelen(carrier) <= 65 at nrefs 7 (pri 2153).

- [s21] Probe P2 (V1 + one extra arg1 call reference; semantically wrong, measurement only) = 13 / 222 with seats flipped to target's (pri 2474 vs 2142) and a residual diff containing ONLY cluster B and one commutative addu. A legitimate nrefs>=8 spelling on the V1 chassis prices at roughly 4-6 / 215.

- [s21] Body P4 (semantics-preserving carrier `u8 *p;` assigned at block_15) = 16 / 216 with target's seats (pri 2413) and a clean tail from emitted 87 onward, but the whole residual moves into the prologue: the live incoming $a1 displaces the range-check chain's scratch registers (a1->a3, a2->t0) and `move s2,a1` lands at emitted 48 instead of 4. Structurally capped, because any assignment point short enough to shorten the range is after the chain.

- [s21] P1 KILLED: hoisting temp_s3's definition above the `temp_v1_3 == 0` test lets cse1 common path1's load into it, giving nrefs=4 livelen=16 pri=5000 - it outranks arg0 and takes $s0, wrecking every callee-saved seat.

- [s21] P3 KILLED: inverting the three-way compare leaves ALLOCDBG identical to V1 digit-for-digit. flow.c's reg_live_length is path-sensitive, not layout-sensitive, so no block reordering can lengthen livelen(temp_s3).

- [s21] P5 KILLED: duplicating the `>` path's call pair into both arms of `if (var_s1 == 0)` produces the right seats (nrefs 9, pri 2700) but scores 81 / 183 - temp_s5 loses its callee-saved seat, `ret` slides $s6 -> $s5, and the `>` tail collapses.

- [s21] The open contradiction handed forward: target has nrefs(arg1)=7, livelen(arg1)=92, a source-pinned store, and temp_s3 at 3/14 - which under allocno_compare gives the WRONG seats. One of those must differ in target's RTL at global_alloc time, and the only mechanism that hides a reference from the final asm while exposing it to regclass is a duplicate deleted by post-reload jump2 cross-jumping (the s16 body proves that path works and keeps the insn count at 216).


## s22 (structural, 2026-08-27) - FLOOR 10 -> 6.  Cluster A closed by a sanctioned do-while(0) wrap

Chassis re-measured at session start with the inherited candidate.c applied:
`sandbox func_800283D0 --disable all` = **10 / 216** (target 215), matching the
ledger.  Re-measured at session end with the NEW candidate.c applied: **6 / 216**.

### E-s22-1  The policy premise s4 worked under is SUPERSEDED, and that is what unlocked the floor

s4 (2026-07) found a `do { } while (0)` wrap that lowered the then-floor from 28
to 26, measured its mechanism precisely (flow.c `REG_N_REFS (regno) += loop_depth`,
loop_depth 2 inside the NOTE_INSN_LOOP_BEG/END pair the wrap leaves behind,
feeding global.c:635-656 `allocno_compare`), and KILLED it on the ground that
`.claude/rules/do-while-zero-exception.md` scoped the carve-out to the
LABEL_OUTSIDE_LOOP_P / reorg.c interaction ONLY, so an RA-weighting use would be
a first reach of an unsanctioned mechanism.

**That ground no longer exists.**  The rule file of record on main
(`.claude/rules/do-while-zero-exception.md`, owner ruling 2026-07-06) reads, in
its own words: *"`do { <any body> } while (0);` - including empty bodies - is a
sanctioned pure-C match device for ANY codegen effect, including register
allocation.  The former scoping to the reorg.c label-note mechanism is
abolished"*, subject to (1) an inline `/* FAKE */` annotation at the construct
site, (2) natural geometry preferred first but exhaustion explicitly NOT a hard
gate for single-level wraps, (3) written justification only for NESTED wraps.
s4's measurements stand; s4's policy conclusion is RETRACTED.  The task brief's
family table still carries the pre-2026-07-06 scoping sentence - the RULE FILE
is the policy of record, and it was re-read end-to-end this session.

### E-s22-2  One wrap around the `==` arm's call pair closes cluster A exactly, at zero emitted cost

Body A3 = s21's V1 chassis (target's `<` arm, store sink + arm store byte-closed,
17 / 216) + `do { ... } while (0);` around the two EXISTING
`func_80032854(*(s16 *)(arg0 + 4), 1 / 0x25, arg1, (s16 *)0);` calls in the
`temp_v0_3 == temp_v1_3` arm.  No statement added, removed, reordered or
duplicated; the two calls execute exactly once either way.

ALLOCDBG, instrumented cc1, `tmp/grind/func_800283D0/s22/alloc_V1.txt` vs
`alloc_A3.txt`:

```
V1 (no wrap)   ord=11 72 arg0    $s0 nrefs=19 livelen=156 pri=4871
               ord=14 77 var_s1  $s1 nrefs=9  livelen=74  pri=3648
               ord=20 143 temp_s3 $s2 nrefs=3 livelen=14  pri=2142   <- WRONG
               ord=22 73 arg1    $s3 nrefs=7  livelen=92  pri=1521   <- WRONG
               ord=23 75 temp_s4 $s4 / ord=25 90 temp_s5 $s5 / ord=26 79 $s6

A3 (wrapped)   ord=11 72 arg0    $s0 nrefs=21 livelen=156 pri=5384
               ord=14 77 var_s1  $s1 nrefs=9  livelen=74  pri=3648
               ord=18 73 arg1    $s2 nrefs=9  livelen=92  pri=2934   <- TARGET
               ord=21 143 temp_s3 $s3 nrefs=3 livelen=14  pri=2142   <- TARGET
               ord=23 75 temp_s4 $s4 / ord=25 90 temp_s5 $s5 / ord=26 79 $s6
```

Every live length is byte-identical.  Only the two pseudos referenced inside the
wrap move, by exactly +1 per reference (arg0 19 -> 21, arg1 7 -> 9).  Every other
callee-saved seat is untouched.  This is s21's solution (a) - nrefs(carrier) >= 8
at livelen 92 - reached at ZERO emitted cost, which is the property P5's
duplicated call pair could not have (P5 = 81 / 183).

**Score: 6 / 216.**  Cluster A, the store sink (emitted 83-88) and the arm store
(emitted 148) are all closed simultaneously - the outcome s21's frontier item 1
priced at "roughly 4-6 / 215".

### E-s22-3  The wrap must contain BOTH calls; site is otherwise free

| body | wrap | score |
|---|---|---|
| V1  | none | 17 / 216 |
| A1  | `==` arm, first call only | 10 / 216 |
| A2  | `==` arm, second call only | 10 / 216 |
| A3  | `==` arm, both calls | **6 / 216** |
| B1  | `>` path, first call only | 10 / 216 |
| B2  | `>` path, both calls | 6 / 216 |
| C1  | shared `do_calls`, second call only | 10 / 216 |
| C2  | shared `do_calls`, both calls | 6 / 216 |
| D1/D2/D3 | any TWO of the three pairs | 6 / 216 |
| D4  | all THREE pairs | 24 / 216 |

A1's ALLOCDBG (`alloc_A1.txt`): 73 arg1 nrefs=8 **livelen=94** pri=2553 -> $s2,
143 temp_s3 3/14/2142 -> $s3.  The seats are already target's at nrefs 8, so the
extra 4 points of A1/A2/B1/C1 are NOT a cluster-A failure: the loop note lands
BETWEEN the two calls, lengthens livelen(arg1) from 92 to 94 and perturbs the
call block's scheduling.  Three sites give the identical 6, so the site is a free
choice; A3 was kept because it leaves the shared `do_calls` block - which carries
the store pin - untouched.

### E-s22-4  s20's frontier item 3 ANSWERED: the cluster-B hoist's +2 price is chassis-independent

F1 = A3 + s20's W2b construction verbatim (`s16 sel5 = 0x19;` scoped to the
`== 5` subtree, `sel5 = 0xB` in the `var_s1 == 0` arm, `var_v0_2 = sel5;` before
`goto block_48`).  **8 / 216** against A3's 6 - the same +2 W2b paid on the s16
chassis (10 -> 12).  s20 asked whether the two `move v0,v1` copies would coalesce
once the store is no longer sunk into the first jal delay slot.  They do not.
The hoist family's price is a property of the family, not of the chassis, and
cluster B needs a different attack.

### E-s22-5  Cluster B is NOT a branch-prediction problem - the mostly_true_jump route is measured dead

Reading `tools/gcc-2.7.2/reorg.c:1379 mostly_true_jump`: scanning back from the
branch's target label through NOTEs, a NOTE_INSN_LOOP_BEG makes the branch
predicted taken (`return 2`), which is what steers `fill_eager_delay_slots`
(reorg.c:3784) to fill from the TARGET thread rather than the fall-through.  Both
of the residual's two halves have exactly the shape that predicate would fix:

  * emitted 45-48: ours fills the last range-check `beq`'s slot from the
    fall-through (`li v0,1`) and leaves the following `j` empty; target does the
    opposite.
  * emitted 126: ours leaves the `beqz v0` slot empty; target steals the target
    block's `li v0,1` (the value stored to D_800A38A8) into it.

Six probes, all measured:
  * **G1** (wrap `D_800A38A8 = 1; D_800A3876 = -1;`) = 6 / 216 - byte-neutral.
    Mechanism why: `expand_end_cond` emits the if-false LABEL before the wrapped
    statement expands, so the NOTE_INSN_LOOP_BEG lands AFTER the label, on the
    wrong side of `mostly_true_jump`'s backward scan.
  * **G2** (wrap the `if (var_s1 == 0) var_v0_2 = 0xB;` selection) = 6 / 216.
  * **G3** (wrap the whole `var_v0_2 = 0x19; if (...) ...;` selection) = 6 / 216.
  * **G5** (wrap the range-check `return 1;`) = 6 / 216.  **G6** (wrap the whole
    range-check `if`) = 14 / 213.
  * **H4** - the shape built specifically to put the LOOP_BEG on the RIGHT side:
    the `||` test respelled `if (!(A || B)) goto rare5;` with
    `do { rare5: D_800A38A8 = 1; D_800A3876 = -1; } while (0);` so the branch's
    own target label sits immediately after the loop note.  **6 / 216 -
    byte-neutral.**
  * **H5** - the same shape for the range-check chain
    (`if (!(chain)) goto lcont; return 1; do { lcont: var_s1 = 0; } while (0);`)
    = **10 / 213**: the `return 1` cross-jumps into the shared epilogue and the
    insn count drops below target's.  **H6** (both) = 10 / 213.

So prediction is not the dial.  E-s18-8's attribution stands unchallenged: the
refusal is `insn_sets_resource_p (trial, &opposite_needed)` - a RESOURCE test
that runs whichever way the branch is predicted - with the `$v0` bit put into
`opposite_needed` by E-s19-4's `update_block` `(use (insn N))` marker.

### E-s22-6  The two commutative `addu` operand orders are re-confirmed inert

F2 = A3 with `u8 *temp_s3 = arg0 + (temp_a1_2 * 2);` respelled
`(temp_a1_2 * 2) + arg0` = **6 / 216**, byte-identical.  GCC canonicalises the
PLUS operands before RA, so the emitted `addu s3,s0,v0` vs target's
`addu s3,v0,s0` is a consequence of allocation order, not of source order.  Same
conclusion earlier sessions reached; now re-confirmed on the 6-floor chassis.

- [s22] Chassis: the inherited candidate.c measured 10 / 216 at session start; the NEW candidate.c measures 6 / 216 at session end. FLOOR 10 -> 6, the first move since s16.
- [s22] POLICY CORRECTION (the whole session turns on it): `.claude/rules/do-while-zero-exception.md` on main, owner ruling 2026-07-06, sanctions `do { <any body> } while (0);` for ANY codegen effect INCLUDING register allocation, with a mandatory inline FAKE annotation and NO exhaustion gate for single-level wraps. s4's 2026-07 kill of this construct rested on the superseded pre-2026-07-06 mechanism-scoping and is RETRACTED as to policy; its measurements stand. The task-brief family table still quotes the old scoping sentence - read the rule file, not the table.
- [s22] Body A3 (= s21's V1 chassis + one `do { } while (0);` around the two EXISTING calls in the `temp_v0_3 == temp_v1_3` arm) = 6 / 216. No statement added, removed, duplicated or reordered.
- [s22] MECHANISM MEASURED, not inferred (alloc_V1.txt vs alloc_A3.txt): the wrap raises nrefs(arg0) 19 -> 21 and nrefs(arg1) 7 -> 9 (flow.c REG_N_REFS += loop_depth, loop_depth 2 inside the NOTE_INSN_LOOP_BEG/END pair), leaves EVERY live length byte-identical, and flips exactly the two cluster-A seats: arg1 pri 1521 -> 2934 takes $s2, temp_s3 stays 2142 and takes $s3 - target's seats. Every other callee-saved seat ($s0/$s1/$s4/$s5/$s6) is unchanged. This is s21's solution (a) at ZERO emitted cost.
- [s22] Cluster A, the store sink (emitted 83-88) and the arm store (emitted 148) are ALL closed in this body. What remains at 6 is cluster B (emitted 45-48 slot ownership + emitted 126/131, which is the entire 216-vs-215 insn surplus) and the two commutative `addu` operand orders.
- [s22] The wrap must contain BOTH calls of a pair: wrapping one only (A1/A2/B1/C1) still gives target's seats (nrefs 8, pri 2553) but scores 10, because the loop note lands between the calls and lengthens livelen(arg1) 92 -> 94. Any ONE of the three call pairs works identically (A3 = B2 = C2 = 6); any TWO also give 6 (D1/D2/D3); all THREE give 24 (D4).
- [s22] s20's frontier item 3 ANSWERED: re-applying W2b's cluster-B hoist on this chassis (F1) = 8, the same +2 it paid on the s16 chassis. The hoist family's price is chassis-independent; the two `move v0,v1` copies do not coalesce once the store is un-sunk. Cluster B needs a different attack.
- [s22] Cluster B is NOT a branch-prediction problem. reorg.c:1379 mostly_true_jump returns 2 when a NOTE_INSN_LOOP_BEG immediately precedes the branch's target label, which would steer fill_eager_delay_slots to the target thread. Six probes measured: bare wraps at the D_800A38A8 store pair (G1), the 0x19/0xB selection (G2/G3) and the range-check return (G5) are all byte-neutral at 6; the purpose-built shape that actually puts the loop note before the branch's own target label (H4, `if (!(A||B)) goto rare5;` with `do { rare5: ... } while (0);`) is ALSO neutral at 6; the same shape on the range-check chain (H5/H6) is regressive at 10 / 213 because the `return 1` cross-jumps into the shared epilogue. E-s18-8's resource-test attribution therefore stands.
- [s22] G1's neutrality has a named cause: `expand_end_cond` emits the if-false label BEFORE the following statement expands, so a `do {` written after the `if` puts NOTE_INSN_LOOP_BEG on the far side of mostly_true_jump's backward scan. Only an explicit source `goto` into a label written INSIDE the wrap can place the note correctly - and H4 shows that placing it correctly is still not sufficient.
- [s22] The two commutative `addu` operand orders are inert on this chassis too: respelling `arg0 + (temp_a1_2 * 2)` as `(temp_a1_2 * 2) + arg0` (F2) is byte-identical at 6. GCC canonicalises PLUS operands before RA.

- [s22] Chassis: the inherited candidate.c measured 10 / 216 (target 215) at session start; the NEW candidate.c measures 6 / 216 at session end. FLOOR 10 -> 6, the first move since s16.

- [s22] POLICY CORRECTION, and the whole session turns on it: .claude/rules/do-while-zero-exception.md on main (owner ruling 2026-07-06) sanctions `do { <any body> } while (0);` for ANY codegen effect INCLUDING register allocation, with a mandatory inline FAKE annotation, and states in terms that 'the former scoping to the reorg.c label-note mechanism is abolished'. Exhaustion is explicitly NOT a hard gate for single-level wraps; only NESTED wraps need a written single-level-insufficient justification. s4's 2026-07 kill of this construct rested on the superseded pre-2026-07-06 mechanism-scoping and is retracted as to policy (its measurements stand). NOTE FOR FUTURE SESSIONS AND FOR THE DRIVER: the task-brief family table still quotes the old 'LABEL_OUTSIDE_LOOP_P / reorg.c interaction ONLY' scoping sentence - the rule file is the policy of record and should be read directly.

- [s22] New candidate body = s21's V1 chassis (target's `<` arm: `var_v0_4 = 0x19; if (var_s1 == 0) goto set_0xB; store; goto do_calls;`) plus ONE `do { ... } while (0);` around the two EXISTING func_80032854 calls in the `temp_v0_3 == temp_v1_3` arm, carrying the required inline /* FAKE: ... */ annotation naming what, the GCC mechanism and the lever-exhaustion ledger. No statement is added, removed, duplicated or reordered; both calls execute exactly once either way.

- [s22] MECHANISM MEASURED, not inferred (tmp/grind/func_800283D0/s22/alloc_V1.txt vs alloc_A3.txt, instrumented cc1 ALLOCDBG): the wrap raises nrefs(arg0) 19 -> 21 and nrefs(arg1) 7 -> 9 (flow.c REG_N_REFS += loop_depth, loop_depth 2 inside the NOTE_INSN_LOOP_BEG/END pair), leaves every live length byte-identical (arg1 92, temp_s3 14, arg0 156), and flips exactly the two cluster-A seats: arg1 pri 1521 -> 2934 -> $s2, temp_s3 2142 -> $s3. Every other callee-saved seat is unchanged. This is s21's solution (a) at ZERO emitted cost.

- [s22] Cluster A, the store sink (emitted 83-88) and the arm store (emitted 148) are ALL closed in this body - the three residual clusters s13 through s21 could never satisfy simultaneously. s21 priced this outcome at 'roughly 4-6 / 215'; it landed at 6.

- [s22] What remains at 6 (tmp/grind/func_800283D0/s22/diff_final.txt; `lui at,0` entries are unresolved-relocation artifacts of objdumping the .o, not diffs): (1) emitted 45-48, ours `beq / li v0,1 / j / nop` vs target `beq / nop / j / li v0,1` - same insn count, different branch owns the `li`; (2) emitted 126 + 131, ours `nop` in the `beqz v0` delay slot plus a standalone `li v0,1`, target steals the branch TARGET block's `li v0,1` (the value stored to D_800A38A8) into the slot - this is cluster B and it is the ENTIRE 216-vs-215 insn surplus; (3) emitted 96 `addu s3,s0,v0` and 162 `addu a0,a0,s4`, commutative operand orders, re-measured inert.

- [s22] The wrap must contain BOTH calls of a pair: wrapping one only (A1/A2/B1/C1) still yields target's seats (nrefs 8, pri 2553) but scores 10, because the note lands between the calls and lengthens livelen(arg1) 92 -> 94. Site is otherwise a free choice - the `==` arm, the `>` path and the shared do_calls block all give the identical 6 (A3 = B2 = C2), any two pairs give 6 (D1/D2/D3), all three give 24 (D4). A3 was kept because it leaves the shared do_calls block, which carries the store pin, untouched.

- [s22] s20's frontier item 3 is ANSWERED: W2b's cluster-B hoist re-applied on this chassis (F1) = 8, the same +2 it paid on the s16 chassis. The two `move v0,v1` copies do not coalesce once the store is un-sunk; the hoist family's price is chassis-independent.

- [s22] Cluster B is NOT a branch-prediction problem. reorg.c:1379 mostly_true_jump returns 2 when a NOTE_INSN_LOOP_BEG immediately precedes the branch's target label, which steers fill_eager_delay_slots to the target thread. Six probes measured: bare wraps at the D_800A38A8 store pair (G1), the 0x19/0xB selection (G2/G3) and the range-check return (G5) are byte-neutral at 6; G6 = 14 / 213; the purpose-built H4, which actually places the note before the branch's own target label via `if (!(A||B)) goto rare5;` + `do { rare5: ... } while (0);`, is ALSO neutral at 6; H5/H6 (same shape on the range-check chain) = 10 / 213 because the `return 1` cross-jumps into the shared epilogue.

- [s22] G1's neutrality has a named cause worth carrying forward: expand_end_cond emits an if-false LABEL before the following statement expands, so a `do {` written after the `if` puts NOTE_INSN_LOOP_BEG on the far side of mostly_true_jump's backward scan. Only an explicit source `goto` into a label written INSIDE the wrap places the note correctly - and H4 proves that placing it correctly is still not sufficient.

## s23 (structural) - CLUSTER B CLOSED; floor 6 -> 4 and 216 -> 215 insns

**E-s23-1 (the body, and exactly what changed).**  Start of session the banked
s22 candidate re-measured 6 / 216 on HEAD (chassis unchanged).  The only edit
this session is inside the `== 5` selection of the `==` arm.  s22 spelled it
`var_v0_2 = 0x19; if (var_s1 == 0) { var_v0_2 = 0xB; } goto block_48;`.  The new
body (tmp/grind/func_800283D0/s23/X2.c, banked as candidate.c) spells the SAME
selection with the 0x19 edge as the branch's own target thread, carrying its own
copy of the already-existing store:

    if (var_s1 != 0) { goto sel19; }
    var_v0_2 = 0xB;
    goto block_48;
  sel19:
    *(s16 *)(arg0 + 0x286) = 0x19;
    goto block_49;

Semantically identical (s1 != 0 stores 0x19, s1 == 0 stores 0xB, both fall into
`return ret`).  **Measured 4 / 215.**  The duplicated store re-merges in jump2:
emitted 128-131 are byte-identical to target (`bnez s1,.L800286F8 / li v0,25 /
j .L800286F8 / li v0,11`), i.e. both edges still reach the ONE shared store at
block_48.  The duplication is invisible in the emitted bytes; what it changes is
which reorg.c pass fills a delay slot.

**E-s23-2 (why: E-s19-7 option (a), spelled).**  s19 named three possible ways
target could have BOTH `jump_insn 344`'s and `jump_insn 354`'s slots filled, and
option (a) was "354's slot is filled by fill_EAGER from its TARGET
(INSN_FROM_TARGET_P, so update_block emits no marker at all)".  Moving the
`var_v0_2 = 0x19` write from BEFORE the selection branch to the head of that
branch's TARGET thread is precisely that spelling: `fill_simple_delay_slots` has
nothing to steal backward (the block between `code_label 347` and the branch is
now empty), so no `(use (insn N))` marker is emitted into the window
`mark_target_live_regs` walks; `$v0` therefore stays cleared by the label flush
and `insn_sets_resource_p (trial, &opposite_needed)` is 0 for the `li v0,1`
trial.  Ordering reinforces it: `fill_eager_delay_slots` walks insns in order,
so the `beqz` is processed while the selection branch is still unfilled and
cannot poison it; the selection branch is then filled afterwards out of its own
target thread, where `update_block` (reorg.c:2270) returns early on
`INSN_FROM_TARGET_P`.  This closes cluster B at ZERO cost, where the entire
hoist family (s19 W1 = 32, s20 W2b/W2a/W2c = 12, s22 F1 = 8) pays a
chassis-independent +2 because it buys slot 344 by giving up slot 354.

**E-s23-3 (the shared-store variant is NOT equivalent - X1 = 6).**  X1 is X2
with the `sel19` arm going to the shared block_48 store
(`sel19: var_v0_2 = 0x19; goto block_48;`) instead of storing itself.  It
measures 6 / 216, byte-identical to s22's A3.  Reason: with both edges assigning
the same variable and jumping to the same store, jump2 re-merges the arm back
into `li v0,25` BEFORE the branch, so fill_simple's backward steal - and its
marker - come back.  The arm must own its store for the write to stay on the
far side of the branch.  X3 (the mirrored sense, 0xB edge as branch target)
measures 9: GCC re-inverts the branch sense away from target's `bnez s1`, the
same re-inversion s20's W3 and W2a exhibited.

**E-s23-4 (the remaining 4 points, and the mechanism of the last delay slot).**
Normalized diff (tmp/grind/func_800283D0/s23/diff_X2.txt; `lui at,0` / `jal 0`
entries are unresolved-relocation artifacts of objdumping the .o):
  - emitted 45-47: ours `beq / li v0,1 / j / nop`, target `beq / nop / j /
    li v0,1`.
  - emitted 96 `addu s3,s0,v0` vs `addu s3,v0,s0`, emitted 161 `addu a0,a0,s4`
    vs `addu a0,s4,a0`: commutative operand order, both source spellings banked
    neutral (s3, s22).
DBRDBG + the .sched2/.dbr dumps name the first one exactly.  The pre-reorg
stream is `jump_insn 78` (the range chain's last `beq v1,v0 -> label 87`),
`insn 82 (set (reg/i:SI 2 v0) (const_int 1))`, `jump_insn 84 (j -> label 667)`,
`barrier 85`, `code_label 87`.  `DBRDBG simp insn=84 trial=82 refset=0 setset=0
setneed=0` + `elig=1` shows **fill_simple already builds target's arrangement**:
insn 82 goes into the `j`'s slot.  A later pass then runs
`fill_slots_from_thread` on insn 78 - `DBRDBG thr insn=78 thread=84 opp=270
own=1 likely=0 tif=0 oppregs=20000380_00000000` followed by `WINNER insn=78
trial=84` - and steals the delay insn back OUT of the fall-through sequence
(`steal_delay_list_from_fallthrough`).  The steal is permitted because
`oppregs = 0x20000380` (= {a3, t0, t1, sp}) does not contain `$v0`; that word is
the exact flow live-in of the `beq`'s TARGET block (`code_label 87`, found
cleanly by find_basic_block from `barrier 85`, so it is not the -1 conservative
fallback).  Target's compile must have `$v0` in that word.  **The lever is
therefore liveness at the `beq`'s target block, not placement and not spelling
of the exit** - which is why Q1/Q2 (relocating `return 1;` to a `ret1:` label at
the function end / before block_13) measure 10 / 215 and 8 / 216 respectively:
they deny insn 78 its stealable fall-through but pay 4-6 points elsewhere to buy
at most 2.

- [s23] Chassis: the s22 candidate re-measured 6 / 216 at session start; the new candidate measures 4 / 215 twice, with the body in place in src/code6cac_b.c.
- [s23] FLOOR 6 -> 4 AND THE INSTRUCTION COUNT NOW MATCHES TARGET (215).  Cluster B - the unfilled `beqz $v0` delay slot plus the surplus `li v0,1`, open since s3, named since s18, first closed-at-a-price by s19's W1 - is CLOSED AT ZERO COST by respelling the `== 5` selection so the `0x19` edge is the branch's TARGET thread and carries its own copy of the `*(s16 *)(arg0 + 0x286)` store (body X2 = candidate.c).
- [s23] MECHANISM: reorg.c:2270 `update_block` returns early on `INSN_FROM_TARGET_P`, so a target-thread steal leaves no `(use (insn N))` marker; fill_simple's backward steal always leaves one, and that marker is the sole reason `$v0` is live on the opposite thread when fill_eager reaches the `beqz` (E-s19-4).  Moving the `$v0` write across its own branch removes the marker without giving up either slot.  This is E-s19-7's option (a), which s19 named and no session had spelled.
- [s23] X1 (same double-goto but the 0x19 arm goes to the SHARED block_48 store) = 6, byte-identical to s22: jump2 re-merges the arm and the `li v0,25` returns to before the branch.  The arm must own its store.  X3 (0xB edge as branch target) = 9: GCC re-inverts the branch sense away from target's `bnez s1`.
- [s23] The last delay-slot divergence (emitted 45-47) is a `steal_delay_list_from_fallthrough` on `jump_insn 78`: fill_simple already produces target's arrangement (`simp insn=84 trial=82 elig=1`) and insn 78 steals it back because `oppregs=0x20000380` - the exact flow live-in of the `beq`'s target block - lacks `$v0`.  Q1/Q2 (relocating the `return 1;` exit) = 10 / 215 and 8 / 216: KILLED.
- [s23] POLICY NOTE for the next session: the `sel19` arm's `*(s16 *)(arg0 + 0x286) = 0x19;` is a REAL statement duplicated into a branch arm that re-merges byte-neutrally, which is the shape `.claude/rules/duplicated-statement-into-arms.md` covers and that rule mandates a FAKE annotation.  It is also literally target's own control flow (the same double-goto shape s21 read off target's `<` arm, already in the body un-annotated).  Settle that - annotate or ruling-request - BEFORE any candidate-ready submission.

- [s23] Chassis re-measured at session start: the s22 candidate scores 6 / 216 on HEAD, matching the ledger. The new candidate measures 4 / 215, twice, with the body in place in src/code6cac_b.c.

- [s23] FLOOR 6 -> 4 AND build_insns 216 -> 215: the instruction count matches target exactly for the first time in 23 sessions. Cluster B (unfilled `beqz $v0` delay slot + surplus `li v0,1`) is CLOSED, at ZERO cost, where the entire hoist family pays a chassis-independent +2 (s19 W1 = 32, s20 W2b/W2a/W2c = 12, s22 F1 = 8).

- [s23] The winning edit is confined to the `== 5` selection: `if (var_s1 != 0) goto sel19; var_v0_2 = 0xB; goto block_48; sel19: *(s16 *)(arg0 + 0x286) = 0x19; goto block_49;` - semantically identical to s22's form, and emitted 128-131 are byte-identical to target (`bnez s1 / li v0,25 / j / li v0,11`), i.e. the duplicated store re-merges to the one shared store in jump2.

- [s23] MECHANISM (named, from reorg.c source + DBRDBG): update_block (reorg.c:2270) returns early on INSN_FROM_TARGET_P, so a target-thread steal leaves no `(use (insn N))` marker; fill_simple's backward steal always leaves one, and that marker is the sole reason $v0 is live on the opposite thread when fill_eager reaches the beqz (E-s19-4). Putting the $v0 write on the far side of its own branch removes the marker without giving up either slot. This is E-s19-7's option (a).

- [s23] X1 (same double-goto but the 0x19 arm goes to the SHARED block_48 store) = 6 / 216, byte-identical to s22: jump2 re-merges the arm and `li v0,25` returns to before the branch. The arm MUST own its store.

- [s23] X3 (mirrored sense: 0xB edge as the branch target) = 9 / 216 - GCC re-inverts the branch sense away from target's `bnez s1`, the same re-inversion s20's W3 and W2a exhibited.

- [s23] Residual at 4 is exactly three divergences: emitted 45-47 (ours `beq / li v0,1 / j / nop` vs target `beq / nop / j / li v0,1`), emitted 96 `addu s3,s0,v0` vs `addu s3,v0,s0`, emitted 161 `addu a0,a0,s4` vs `addu a0,s4,a0`. The two addu orders are a banked-dead source-order axis (s3, s22: both spellings byte-neutral).

- [s23] Emitted 45-47 fully attributed: the pre-reorg stream is jump_insn 78 (`beq v1,v0 -> label 87`), insn 82 `(set (reg/i:SI 2 v0) (const_int 1))`, jump_insn 84 (`j -> label 667`), barrier 85, code_label 87. fill_simple puts insn 82 in the `j`'s slot (target's arrangement); insn 78 then steals it back out of the fall-through sequence, allowed because oppregs = 0x20000380 = {a3,t0,t1,sp} has no $v0.

- [s23] Q1 / Q2 (relocating the range check's `return 1;` to a `ret1:` label at the function end / before block_13) = 10 / 215 and 8 / 216. KILLED and banked to rejected/.

- [s23] POLICY FLAG for the next session: the sel19 arm's `*(s16 *)(arg0 + 0x286) = 0x19;` is a real statement duplicated into a branch arm with byte-neutral re-merge - the shape .claude/rules/duplicated-statement-into-arms.md covers, and that rule mandates a FAKE annotation. It is also literally target's own control flow (the same double-goto shape s21 read off target's `<` arm, already in the body un-annotated). Settle it - annotate or ruling-request - BEFORE any candidate-ready submission.

## s24 (synthesis, 2026-08-27) - FLOOR 4 -> 3; the `addu` operand-order axis is RESOLVED AS A MECHANISM and half of it is CLOSED

**E-s24-0 (chassis).**  s23's banked candidate re-measured 4 / 215 on HEAD at
session start - chassis unchanged.  The new body (tmp/grind/func_800283D0/s24/A.c,
banked as candidate.c) measures **3 / 215**, twice, with the body in place in
src/code6cac_b.c.

**E-s24-1 (the two commutative `addu` orders are FIXED AT EXPAND - the s22/s23
frontier question is answered).**  Read on the 4-floor chassis from
tmp/grind/func_800283D0/s24/code6cac_b.i.{rtl,combine,lreg,greg}.  The `.rtl`
dump - i.e. the stream straight out of expand, before cse/combine/regmove/RA -
ALREADY carries the final operand order at both sites:

    .rtl   insn 277  (set (reg/v 143) (plus:SI (reg/v 72) (reg 145)))   <- (ptr, shift), emitted 96
    .rtl   insn 518  (set (reg 203)   (plus:SI (reg 202) (reg/v 75)))   <- (shift, ptr), emitted 161

and `.combine` / `.greg` carry the identical order (only the register names
change).  **No post-expand pass ever touches it**, so there is no combine /
regmove / reload lever, and the previously-suspected "canonicalised before RA"
model is WRONG.  The order is decided entirely by the tree and the expansion
CONTEXT.

**E-s24-2 (the law: expansion context, not source operand order).**  Every
`plus` of a pointer and a scaled index in this function obeys one rule:
  - expanded as a **MEM ADDRESS** (`EXPAND_SUM`, i.e. written inline inside a
    `*(T *)(...)`), it comes out **(shift, pointer)**.  Insns 183
    (`temp_s4 + temp_s5*2 + 0x288`), 199 (`arg0 + temp_a1_2*2 + 0x288`) and
    518 (`temp_s4 + temp_s5*0x10 + 0x114`) all do, regardless of how the source
    is written.
  - expanded as a **VALUE** (assigned to a pointer local), it comes out in
    **SOURCE order**, i.e. (pointer, shift).  Insn 277 (`u8 *temp_s3 = arg0 +
    temp_a1_2 * 2;`) did.
The address-form order is NOT a source-order axis: C1 (swap the source operands:
`(temp_s5 * 0x10) + temp_s4 + 0x114`), C2 (constant before the shift:
`temp_s4 + 0x114 + (temp_s5 * 0x10)`) and C5 (ARRAY_REF spelling
`((s32 *)temp_s4)[(temp_s5 * 4) + 0x45]`) are ALL byte-neutral at 3.  This
promotes the s3 / s22 "both source orders neutral" observations from unexplained
neutrality to a named law.

**E-s24-3 (site 96 CLOSED: delete the `temp_s3` pointer local).**  Target wants
the ADDRESS form at emitted 96 (`addu s3,v0,s0`).  Deleting
`u8 *temp_s3 = arg0 + (temp_a1_2 * 2);` and spelling both of its uses as
`*(s16 *)(arg0 + (temp_a1_2 * 2) + 0x288)` moves that `plus` into address
context: `.lreg` insn 277 becomes `(plus (reg 145) (reg/v 72))` and emitted 96
becomes `addu s3,v0,s0` = target.  cse still forms the shared address pseudo and
it still lives in $s3 across the intervening call pair, so nothing else moves.
**This is ordinary C with no construct and no annotation.**  Floor 4 -> 3.
(Note this respelling was measured on the s3-era 17-floor chassis as
`varA-arm-plus-noptr-tail-clusterA-lost-17.c` and rejected because it lost
cluster A there; cluster A is now closed by s22's do-while(0) wrap, which does
not depend on livelen(temp_s3), so the respelling is free on this chassis.)

**E-s24-4 (site 161 is a TWO-BODY problem, modelled end to end).**  Target wants
the VALUE form at emitted 161 (`addu a0,s4,a0`).  Introducing the mirror pointer
local `s32 *tail = (s32 *)(temp_s4 + (temp_s5 * 0x10));` and spelling the three
loads `tail[0x45] / tail[0x46] / tail[0x47]` DOES produce target's operand order
(measured: body AB emits `addu a1,s4,a1`) - but it costs the $a0 seat and the
whole tail renames a0<->a1 (7 diffs), 9 / 215.  BB2_QTY_DEBUG on block 43 gives
the exact arithmetic (pri = floor_log2(refs)*refs*10000/span; local-alloc
allocates in descending pri):

    body A  (no tail local):  qty2 ptr   refs=6 span=20 pri=6000 -> ord5 -> $a0  (correct seat)
                              qty3 Judge refs=2 span=4  pri=5000 -> ord6 -> $a1
    body AB (tail local):     qty3 Judge refs=2 span=4  pri=5000 -> ord5 -> $a0  (WRONG)
                              qty1 ptr   refs=6 span=26 pri=4615 -> ord7 -> $a1

The pointer local moves the quantity's BIRTH from 12 to 6 (span 20 -> 26) and
that alone flips the order.  A winning body needs the VALUE form AND
pri(tail qty) > 5000, i.e. **span <= 23** (currently 26) or **refs >= 7**
(currently 6; it is a COMBINED quantity = the `sll` producer + the pointer, so
one more genuine reference is enough: refs=7 gives 5384, refs=8 gives 9230).
Alternatively LOWER the Judge quantity below 4615, which needs span >= 5
(currently 4, refs 2 -> pri 4000 at span 5).  Source position does NOT move the
tail quantity's birth/death: AB1 (0x118 load first) and AB2 (tail declaration
hoisted above `temp_v1_4`) both stay at 9.

**E-s24-5 (emitted 45-47 unchanged; one more route killed).**  R1a - merging the
two `var_s1 = 0;` blocks so the range chain's last `beq` targets a block with a
different flow live-in (s23 frontier item 1, attack (i)) - is **byte-neutral at
3**: GCC had already cross-jumped the two blocks, so the source-level merge
changes nothing flow sees.  The divergence and its mechanism are exactly as s23
left them (steal_delay_list_from_fallthrough on jump_insn 78, permitted because
oppregs = 0x20000380 lacks $v0).

**E-s24-6 (measurement-hygiene incident, worth inheriting).**  `mk.sh` objdumps
`tmp/sandbox/func_800283D0/code6cac_b.o`, which is whatever the LAST `sandbox`
run built - not whatever `apply.py` last wrote to src/.  Applying a body and
then running `mk.sh` without re-running `sandbox` diffs the PREVIOUS body and
produces a completely wrong residual attribution (it cost this session one
turn, and briefly manufactured a phantom a0/a1 cluster).  Always
`run.ps1 -Tags <tag>` immediately before `mk.sh`.  Separately:
`tools/gcc-2.7.2/cc1` (instrumented) and `tools/gcc-2.7.2/build/cc1` (build)
were byte-compared this session on this TU and emit IDENTICAL asm, and `-dp -da`
does not perturb codegen - so dump-based attribution from the instrumented cc1
is sound.

- [s24] Chassis: s23's candidate re-measured 4 / 215 on HEAD at session start; the s24 candidate measures 3 / 215 twice with the body in place in src/code6cac_b.c.
- [s24] FLOOR 4 -> 3.  Residual cluster 2 (emitted 96 `addu s3,s0,v0` vs target `addu s3,v0,s0`) is CLOSED by ORDINARY C: delete the `u8 *temp_s3 = arg0 + (temp_a1_2 * 2);` local and spell both of its uses as `*(s16 *)(arg0 + (temp_a1_2 * 2) + 0x288)`.  No construct, no annotation, no family claim.
- [s24] The commutative-`addu` operand order is FIXED AT EXPAND: the `.rtl` dump already carries the final order at both sites (insn 277 and insn 518) and `.combine`/`.greg` are identical.  There is no combine / regmove / reload lever.  The s22/s23 "canonicalised before RA" model is REFUTED.
- [s24] LAW (measured, 3 sites): a pointer+scaled-index `plus` expanded as a MEM ADDRESS comes out (shift, pointer); expanded as a VALUE assigned to a pointer local it comes out in SOURCE order (pointer, shift).  Source operand order is inert in the address form - C1 (source swap), C2 (constant before the shift) and C5 (ARRAY_REF) are all byte-neutral at 3.
- [s24] Target uses the ADDRESS form at emitted 96 and the VALUE form at emitted 161.  The two sites therefore need OPPOSITE spellings; s23's body had both backwards.
- [s24] Site 161 is a two-body problem: the `s32 *tail` pointer local DOES give target's operand order (body AB emits `addu a1,s4,a1`) but moves the tail-pointer local-alloc quantity's birth 12 -> 6 (span 20 -> 26), dropping pri from 6000 to 4615, below the Judge[]-element quantity's 5000, so it loses the $a0 seat and the tail renames (9 / 215).
- [s24] CLOSED-FORM TARGET for site 161: a body with the VALUE form AND pri(tail qty) > 5000, i.e. span <= 23 or refs >= 7 (pri = floor_log2(refs)*refs*10000/span; the quantity is COMBINED = sll producer + pointer, currently refs=6 span=26).  Equivalent alternative: push the Judge[]-element quantity's span from 4 to >= 5, which drops it to 4000.
- [s24] KILLED: AB1 (0x118 load first) and AB2 (tail declaration hoisted above temp_v1_4) both 9 - source position does not move the tail quantity's birth/death.  A1 (0x118 first, A chassis) 9; A2/A3 (multiply operand swap) 10; C4 (`s32 tail_off` intermediate) 9; B (tail local on the s23 chassis) 10.
- [s24] KILLED: R1a - merging the two `var_s1 = 0;` blocks so the range chain's last `beq` targets a block with a different flow live-in (s23 frontier attack (i)) - byte-neutral at 3.  GCC had already cross-jumped the two blocks.
- [s24] METHOD: a spelling rejected on an OLD chassis because it cost a cluster that a LATER session closed by a different route must be RE-MEASURED, not inherited as dead.  The s24 win is literally `varA-arm-plus-noptr-tail-clusterA-lost-17.c`'s edit - dead at 17 because it lost cluster A, free at 4 because s22 closed cluster A via nrefs(arg1) instead of livelen(temp_s3).
- [s24] TOOLING: `mk.sh` objdumps the last `sandbox`-built .o, NOT src/ - always re-run `run.ps1 -Tags <tag>` immediately before `mk.sh` or the diff describes the previous body.  `tools/gcc-2.7.2/cc1` (instrumented) and `tools/gcc-2.7.2/build/cc1` emit byte-identical asm on this TU, and `-dp -da` does not perturb codegen.

- [s24] Chassis: s23's banked candidate re-measured 4 / 215 on HEAD at session start; the s24 candidate measures 3 / 215 twice with the body in place in src/code6cac_b.c. Instruction count still matches target exactly (215).

- [s24] FLOOR 4 -> 3. s23 residual cluster 2 (emitted 96 `addu s3,s0,v0` vs target `addu s3,v0,s0`) is CLOSED by ORDINARY C: delete `u8 *temp_s3 = arg0 + (temp_a1_2 * 2);` and spell both uses as `*(s16 *)(arg0 + (temp_a1_2 * 2) + 0x288)`. No construct, no annotation, no sanctioned-family claim.

- [s24] The commutative-addu operand order is FIXED AT EXPAND: the .rtl dump already carries the final order at insn 277 and insn 518, and .combine/.greg are identical. There is no combine / regmove / reload lever. The s22/s23 'canonicalised before RA' model is REFUTED.

- [s24] LAW (measured, four sites): a pointer+scaled-index plus expanded as a MEM ADDRESS comes out (shift, pointer); expanded as a VALUE assigned to a pointer local it comes out in SOURCE order (pointer, shift). Source operand order is inert in the address form - C1 (source swap), C2 (constant before the shift) and C5 (ARRAY_REF) are all byte-neutral at 3.

- [s24] Target uses the ADDRESS form at emitted 96 and the VALUE form at emitted 161, so the two sites need OPPOSITE spellings; s23's body had both backwards.

- [s24] Site 161 is a two-body problem: the `s32 *tail` pointer local DOES give target's operand order (AB emits `addu a1,s4,a1`) but moves the tail-pointer local-alloc quantity's birth 12 -> 6 (span 20 -> 26), pri 6000 -> 4615, below the Judge[]-element quantity's 5000, so it loses the $a0 seat (9 / 215).

- [s24] CLOSED-FORM TARGET for site 161 (BB2_QTY_DEBUG block 43, pri = floor_log2(refs)*refs*10000/span): a body with the VALUE form AND pri(tail qty) > 5000, i.e. span <= 23 (currently 26) or refs >= 7 (currently 6 - the quantity is COMBINED = sll producer + pointer; refs=7 gives 5384, refs=8 gives 9230). Equivalent alternative: raise the Judge[]-element quantity's span from 4 to >= 5, dropping it to 4000.

- [s24] KILLED this session: AB1 (0x118 load first) 9, AB2 (tail declaration hoisted above temp_v1_4) 9, A1 (0x118 first, A chassis) 9, A2/A3 (multiply operand swap) 10, C4 (`s32 tail_off` intermediate) 9, B (tail local on the s23 chassis) 10, R1a (merge the two `var_s1 = 0;` blocks) byte-neutral at 3.

- [s24] METHOD (inherit this): a spelling rejected on an OLD chassis because it cost a cluster that a LATER session closed by a different route must be RE-MEASURED, not inherited as dead. The s24 win is literally the edit banked as rejected/varA-arm-plus-noptr-tail-clusterA-lost-17.c - dead at floor 17 because it lost cluster A, free at floor 4 because s22 closed cluster A via nrefs(arg1) instead of livelen(temp_s3).

- [s24] TOOLING: mk.sh objdumps tmp/sandbox/func_800283D0/code6cac_b.o - whatever the LAST sandbox run built, NOT src/. Applying a body and running mk.sh without re-running sandbox diffs the previous body (it manufactured a phantom a0/a1 cluster for one turn this session). Always run.ps1 -Tags <tag> immediately before mk.sh.

- [s24] TOOLING: tools/gcc-2.7.2/cc1 (instrumented) and tools/gcc-2.7.2/build/cc1 (build) were byte-compared on this TU and emit IDENTICAL asm, and -dp -da does not perturb codegen - dump-based attribution from the instrumented cc1 is sound.

- [s24] POLICY (unchanged, still open): the body carries two family-relevant constructs that MUST be settled before any candidate-ready submission - the annotated `do { calls } while (0);` wrap (.claude/rules/do-while-zero-exception.md) and the sel19 arm's duplicated `*(s16 *)(arg0 + 0x286) = 0x19;` store, which is the shape .claude/rules/duplicated-statement-into-arms.md covers and that rule mandates a FAKE annotation.

## s25 (synthesis, 2026-08-27) - FLOOR 3 -> 2; site 161 CLOSED in ordinary C, and the last residual's mechanism is CONFIRMED BY DIRECT PASS ABLATION

**E-s25-0 (chassis).**  s24's banked candidate re-measured **3 / 215** on HEAD at
session start (the driver's dispatch line said "measurement unavailable"; the
ledger number was correct).  The s25 body (tmp/grind/func_800283D0/s25/V2.c,
banked as candidate.c) measures **2 / 215**, re-verified with the body in place
in src/code6cac_b.c after banking.

**E-s25-1 (SITE 161 CLOSED - the two-body problem is solved by delaying the
pointer's RTL BIRTH, not by touching refs).**  s24 left the closed-form target:
a body needs the VALUE expansion of `temp_s4 + temp_s5 * 0x10` (source order
(ptr, shift) = target's `addu a0,s4,a0`) AND `pri(tail qty) > 5000`, where
`pri = floor_log2(refs) * refs * 10000 / span`, and s24's body AB had
refs=6 span=26 pri=4615.  s24 proposed attacking refs (>= 7) or the Judge
quantity's span; BOTH are unnecessary.  The winning move is to shrink the tail
quantity's SPAN back to 20 by pushing its BIRTH from luid 6 back to luid 12 -
i.e. by making expand emit the Judge index chain BEFORE the pointer's `plus`.
Concretely, on top of s24's body:

    s32 temp_v1_4 = -*(s16 *)(arg0 + 0x1CA);
    s32 idx0 = (temp_v1_4 + 0x400) & 0xFFF;      /* NEW */
    s32 idx1 = temp_v1_4 & 0xFFF;                /* NEW */
    s32 *tail = (s32 *)(temp_s4 + (temp_s5 * 0x10));
    s32 temp_v1_5 = (s32)((&Judge)[idx0] * tail[0x45] + (&Judge)[idx1] * tail[0x47]) >> 0xC;
    s32 temp_a0_2 = tail[0x46];

BB2_QTY_DEBUG block 43 on the 3-floor chassis read
`qty2 reg208 birth=12 death=32 refs=6 got=4` for the address pseudo and
`qty3 reg204 birth=16 death=20 refs=2 got=5` for the Judge element; body AB
moved the pointer to `birth=6 death=32` (pri 4615) and handed $a0 to the Judge
element.  With the index locals in front, the pointer quantity is born after the
index chain again, keeps span 20 / pri 6000 / ord5 / $a0, and the `addu` carries
target's operand order because the sum is expanded as a VALUE.  **2 / 215.**
Three spellings all measure 2: V1 (`idx0` only), V2 (`idx0` + `idx1`, banked),
V3 (the first Judge ELEMENT hoisted into `s32 j0` instead of its index).

**E-s25-2 (the value-vs-address law needs a real pointer LOCAL).**  V4 -
respelling the three loads as `((s32 *)(temp_s4 + (temp_s5 * 0x10)))[0x45]` etc.
(an ARRAY_REF on a parenthesised cast, no local) - measures **3 / 215** with the
operand order UNCHANGED.  So s24's law (E-s24-2) is sharper than "value context":
the sum must be the RHS of an assignment to a pointer object.  A cast expression
used as the base of an ARRAY_REF is still expanded through `memory_address` /
EXPAND_SUM and still comes out (shift, pointer).

**E-s25-3 (the last residual's pass is CONFIRMED, not inferred).**  The
instrumented cc1 already carries a `BB2_NO_FT_STEAL` ablation hook at
reorg.c:3817 (`if (own_fallthrough && ! BB2_NO_FT ())`).  Building this TU with
`BB2_NO_FT_STEAL=1` makes emitted 44-47 byte-identical to target
(`beq v1,v0,L` with an EMPTY slot, then `j L` with `li v0,1` in its slot),
while perturbing several other sites.  So the entire remaining 2-point residual
is exactly one call: `fill_eager_delay_slots` -> `fill_slots_from_thread` on the
range chain's last `beq` (RTL jump_insn 78) with `own_fallthrough = 1`, stealing
the fall-through block's `li v0,1` (DBRDBG: `thr insn=78 thread=84 opp=270
own=1 likely=0 tif=0 oppregs=20000380 oppmem=1`, `setsopp=0`, `WINNER
trial=84 annul=0`).  This is the first time the residual has been reproduced by
disabling the pass rather than reasoned about.

**E-s25-4 (a SECOND, cheaper C route to the same refusal is now named - and its
first four spellings are KILLED).**  reorg.c:3817 only tries the fall-through
thread when `own_fallthrough = own_thread_p (NEXT_INSN (insn), NULL_RTX, 1)` is
non-zero, and `own_thread_p` (reorg.c:2195) returns 0 as soon as it walks over
ANY `CODE_LABEL` between the branch and the first active insn - with
`label == NULL_RTX` even a `LABEL_NUSES == 0` label still parked in the insn
chain suffices.  So the residual does NOT require making `$v0` live at the
branch's target (the s19-s24 oppregs route); it only requires a surviving
CODE_LABEL immediately after the `beq`.  Four spellings were measured and all
are byte-inert:
  - Y1: `ret_one:` on the chain fall-through `return 1;` + `goto ret_one;` from
    the `temp_v0 == 4 / == 0x14` exit -> 2 / 215, same residual.
  - Y2: the same label + `goto ret_one;` from `block_13` -> 8 / 216.
  - Y3: the same label + `goto ret_one;` from the `temp_v1 == 0x14` exit ->
    **byte-IDENTICAL** to the banked body (only `.L` numbering differs).
  - Y4: Y3 + Y1 together -> byte-identical as well.
The reason is measured, not guessed: `ret` lives in `$s6` from the top of the
function, so cse rewrites every OTHER `return 1;` in the function as
`move v0,s6` and those paths jump to the shared `.L137` (`move v0,s6`) block,
never referencing `ret_one`.  The single goto that DOES reference it (the
`temp_v1 == 0x14` exit) is consumed inside reorg itself: `fill_slots_from_thread`
takes a COPY of `li v0,1` into that branch's delay slot with `INSN_FROM_TARGET_P`
and `reorg_redirect_jump`s the branch to the epilogue label - which is precisely
target's own emitted 18-19 (`beq v1,a0,T / li v0,1`), and is what the banked
body already emits.  A surviving label therefore needs a SECOND inbound edge
that cse cannot rewrite to `move v0,s6`.

**E-s25-5 (measurement hygiene, inherited from s24 and re-confirmed).**  `mk.sh`
objdumps `tmp/sandbox/func_800283D0/code6cac_b.o`; always run
`run.ps1 -Tags <tag>` immediately before it.  New this session: `plain.sh` /
`noft.sh` (in tmp/grind/func_800283D0/s25/) compile src/code6cac_b.c straight
through the instrumented cc1 to `.s`, which is far cheaper than a sandbox cycle
when the question is "did this label/branch survive to reorg" rather than "what
is the score" - `diff plain_A.s plain_B.s` answers byte-neutrality in one call.

- [s25] Chassis: s24's candidate re-measured 3 / 215 on HEAD at session start; the s25 candidate measures 2 / 215 with the body in place in src/code6cac_b.c.  Instruction count still exactly 215.
- [s25] FLOOR 3 -> 2.  Site 161 (`addu a0,a0,s4` vs target `addu a0,s4,a0`) is CLOSED by three ordinary-C locals: `s32 idx0` / `s32 idx1` for the two Judge table indices, declared BEFORE `s32 *tail = (s32 *)(temp_s4 + (temp_s5 * 0x10));`, with the three loads spelled `tail[0x45] / tail[0x46] / tail[0x47]`.
- [s25] MECHANISM: the pointer local supplies the VALUE expansion (target's operand order), and the two index locals push the pointer's local-alloc quantity BIRTH from luid 6 back to luid 12, restoring span 20 / pri 6000 (`pri = floor_log2(refs)*refs*10000/span`) so it keeps the $a0 seat ahead of the Judge[]-element quantity's 5000.  s24's proposed axes (refs >= 7, or widening the Judge quantity's span) are unnecessary - SPAN was reachable through BIRTH all along.
- [s25] V1 (idx0 only) and V3 (the first Judge ELEMENT hoisted into `s32 j0`) also measure 2 / 215; V2 (both indices named) is banked as the symmetric spelling.
- [s25] KILLED: V4, the ARRAY_REF-on-a-cast spelling `((s32 *)(temp_s4 + (temp_s5 * 0x10)))[0x45]` - 3 / 215, operand order unchanged.  The value-vs-address law (E-s24-2) requires a real pointer LOCAL; a parenthesised cast used as an ARRAY_REF base is still EXPAND_SUM address context.
- [s25] CONFIRMED BY ABLATION: building the TU with `BB2_NO_FT_STEAL=1` (the instrumented cc1's hook at reorg.c:3817, `if (own_fallthrough && ! BB2_NO_FT ())`) makes emitted 44-47 byte-identical to target.  The entire remaining 2-point residual is one `fill_slots_from_thread` call on jump_insn 78 with own_fallthrough = 1.
- [s25] NEW FRONTIER MODEL: the refusal does NOT need `$v0` live at the branch target (the s19-s24 oppregs route).  `own_thread_p` (reorg.c:2195) returns 0 for ANY CODE_LABEL sitting between the branch and the first active fall-through insn, even at LABEL_NUSES == 0.  A surviving label immediately after the `beq` closes the cluster.
- [s25] KILLED: Y1 (label + goto from the temp_v0 exits) 2 / 215; Y2 (goto from block_13) 8 / 216; Y3 (goto from the temp_v1 == 0x14 exit) and Y4 (Y3+Y1) BYTE-IDENTICAL to the banked body.  `ret` lives in $s6, so cse rewrites every other `return 1;` as `move v0,s6` and those paths jump to the shared `.L137`, never referencing the label; the one goto that does reference it is consumed by reorg's own `reorg_redirect_jump` when it copies `li v0,1` into that branch's delay slot (which is target's own emitted 18-19).
- [s25] TOOLING: `plain.sh` / `noft.sh` in tmp/grind/func_800283D0/s25/ compile src/code6cac_b.c directly through the instrumented cc1 to `.s`; `diff plain_<A>.s plain_<B>.s` decides byte-neutrality in one call, far cheaper than a sandbox cycle, and is how Y3/Y4 were proven inert.

- [s25] Chassis: s24's banked candidate re-measured 3 / 215 on HEAD at session start (the dispatch line said 'measurement unavailable'; the ledger number was right). The s25 body measures 2 / 215, re-verified with candidate.c applied to src/code6cac_b.c. Instruction count still exactly 215.

- [s25] FLOOR 3 -> 2. Site 161 closes with three ordinary-C locals: `s32 idx0 = (temp_v1_4 + 0x400) & 0xFFF;` and `s32 idx1 = temp_v1_4 & 0xFFF;` declared BEFORE `s32 *tail = (s32 *)(temp_s4 + (temp_s5 * 0x10));`, with the three loads spelled tail[0x45] / tail[0x46] / tail[0x47]. No construct, no annotation, no family claim for this hunk.

- [s25] MECHANISM: the pointer local gives the VALUE expansion (target's operand order); the two index locals push the pointer's local-alloc quantity BIRTH from luid 6 back to luid 12, restoring span 20 / pri 6000 so it keeps $a0 ahead of the Judge[]-element quantity's 5000. s24's proposed axes (refs >= 7, widening the Judge quantity's span) are unnecessary - SPAN was reachable through BIRTH all along.

- [s25] V1 (idx0 only) and V3 (the first Judge ELEMENT hoisted into `s32 j0`) also measure 2 / 215; V2 (both indices named) is banked as the symmetric spelling.

- [s25] KILLED: V4, the ARRAY_REF-on-a-cast spelling, 3 / 215 with unchanged operand order - the value-vs-address law needs a real pointer LOCAL, not a parenthesised cast used as an ARRAY_REF base.

- [s25] CONFIRMED BY ABLATION: BB2_NO_FT_STEAL=1 (instrumented cc1, reorg.c:3817) makes emitted 44-47 byte-identical to target. The whole remaining 2-point residual is one fill_slots_from_thread call on jump_insn 78 with own_fallthrough = 1.

- [s25] NEW FRONTIER MODEL: the refusal does NOT require $v0 live at the branch target (the s19-s24 oppregs route). own_thread_p (reorg.c:2195) returns 0 for ANY CODE_LABEL between the branch and the first active fall-through insn, even at LABEL_NUSES == 0.

- [s25] KILLED: Y1 (2/215), Y2 (8/216), Y3 and Y4 (byte-identical to the banked body) - the label route's first four spellings. `ret` lives in $s6 so cse rewrites every other `return 1;` as `move v0,s6`; the one goto that reaches the block is consumed by reorg's own reorg_redirect_jump (which is target's emitted 18-19 and already matches).

- [s25] KILLED: W1 (2/215 byte-neutral), W2 / W3 (8/216) - spelling other exits as literal `return 1;` does not create a second `li v0,1` block for cross-jumping to merge.

- [s25] TOOLING: tmp/grind/func_800283D0/s25/plain.sh and noft.sh compile src/code6cac_b.c straight through the instrumented cc1 to .s; `diff plain_<A>.s plain_<B>.s` decides byte-neutrality in ONE call and is how Y3/Y4 were proven inert - far cheaper than a sandbox cycle for structural questions.

- [s25] POLICY (unchanged, still open before any candidate-ready): the body carries the annotated `do { calls } while (0);` wrap (.claude/rules/do-while-zero-exception.md) and the sel19 arm's duplicated `*(s16 *)(arg0 + 0x286) = 0x19;` store (.claude/rules/duplicated-statement-into-arms.md, which mandates a FAKE annotation the body does not yet carry).

## s26 (synthesis, 2026-08-27) - floor HELD at 2 / 215; the last residual's PASS-INTERNAL mechanism is RE-ATTRIBUTED (it is `steal_delay_list_from_fallthrough`, not a plain thread steal), its full guard list is enumerated against the source, and the label route's remaining C-level supply is FORECLOSED BY EXIT TOPOLOGY

**E-s26-0 (chassis).**  s25's banked candidate re-measured **2 / 215** on HEAD at
session start and again after this session's probes were reverted (the driver's
dispatch line said "measurement unavailable"; the ledger number was correct).
Instruction count still exactly 215 == 215.  Nothing this session changed the
banked body; candidate.c is unchanged from s25.

**E-s26-1 (NEW TOOLING - single-function DBRDBG isolation).**  `tmp/grind/func_800283D0/s26/iso.py`
slices the preprocessed TU (`code6cac_b.i`) down to declarations + the single
definition of func_800283D0 by top-level brace tracking, and `iso.sh` compiles
that `iso.i` through the instrumented cc1 with `BB2_DBR_DEBUG=1 -dp -da`.  The
isolated build REPRODUCES THE RESIDUAL EXACTLY (same emitted 44-47 divergence)
while shrinking the DBRDBG trace from 3763 lines to 260 and removing all UID
ambiguity - `code6cac_b.i` restarts insn UIDs per function, and s25's quoted
`thr insn=78 thread=84 opp=270 oppregs=20000380` line belongs to a DIFFERENT
function of the TU, not to ours.  Any future reorg/dbr forensics on this
function should use `iso.sh`, not `dbr.sh`.

**E-s26-2 (THE RESIDUAL'S REAL MECHANISM - re-attributed).**  Our function's RTL
names (stable from .greg through .dbr) are: `jump_insn 78` = the range chain's
last `beq v1,v0,.L45`; `insn 82` = `li v0,1` (the chain fall-through `return 1;`);
`jump_insn 84` = `j 662` (the jump to the epilogue label); `code_label 87` = .L45.
`MAX_REORG_PASSES` is 2, so simple/eager/relax each run twice, but the whole
residual is decided in round 0:

  1. `fill_simple_delay_slots (first, 0)` reaches jump_insn 84 and fills its slot
     with insn 82 - DBRDBG `simp insn=84 trial=82 refset=0 setset=0 setneed=0`
     then `simp insn=84 trial=82 elig=1`.  **After the simple pass the function
     is byte-identical to target at this site**: beq 78 has an empty slot, `j 84`
     carries `li v0,1`.  Target's arrangement is produced for free and then lost.
  2. `fill_eager_delay_slots` then reaches jump_insn 78 with prediction <= 0, so
     it tries the fall-through thread first (reorg.c:3817).  `fallthrough_insn`
     is now the SEQUENCE that step 1 created, so the walk loop in
     `fill_slots_from_thread` immediately stops on it and control reaches
     reorg.c:3596-3615: "If we stopped on a branch insn that has delay slots, see
     if we can steal some of the insns in those slots" ->
     `steal_delay_list_from_fallthrough (reorg.c:1743)`, which **pulls `li v0,1`
     BACK OUT of the `j`'s delay slot** (`delete_from_delay_slot`) and puts it in
     the beq's.  That function contains NO DBRDBG print, which is why s25 saw a
     `thr insn=... thread=...` line with no `trial`/`WINNER` line following it and
     read the event as a plain fall-through steal.  It is not: it is a
     delay-slot-to-delay-slot theft from an already-filled unconditional jump.

**E-s26-3 (the guard list, enumerated against reorg.c:1762-1795 - only two knobs
are C-visible).**  For the steal of `trial = li v0,1` out of `seq = [j 84; li 82]`:
  - `simplejump_p (XVECEXP (seq,0,0))` must hold - it does (`j 662`), and any
    spelling that keeps a `return 1;` block ending in a jump to the epilogue keeps
    it holding.
  - `insn_references_resource_p (trial, sets, 0)`, `insn_sets_resource_p (trial,
    needed, 0)`, `insn_sets_resource_p (trial, sets, 0)`: `sets` and `needed` are
    both EMPTY here, because `fill_slots_from_thread` clears them (reorg.c:3388-3389)
    and the thread's FIRST element is already the sequence, so no insn is walked
    over before the steal.  Poisoning `needed` with $v0 would require an extra
    real instruction between the beq and the `li` -> 216 insns -> dead by
    construction.
  - `insn_sets_resource_p (trial, other_needed, 0)`: `other_needed` =
    `mark_target_live_regs` at .L45.  Isolated build measures
    `oppregs=0x20750000` = {s0, s2, s4, s5, s6, sp}; $v0 (bit 0x4) absent.  This
    is the s19-s24 route and it stays FORECLOSED for the reason s21/s24 measured:
    block_15 recomputes D_800A3824, `*(s16 *)(arg0 + 4)` and `*(s16 *)(temp_s4 + 4)`,
    and every value that genuinely crosses the chain is seated callee-saved in
    target, so nothing can be alive in $v0 at .L45 without adding an instruction.
  - the outer gate `if (own_fallthrough && ! BB2_NO_FT ())` (reorg.c:3817).
So exactly ONE cheap axis survives: `own_fallthrough == 0`, i.e. a CODE_LABEL
surviving between jump_insn 78 and insn 82 (`own_thread_p`, reorg.c:2195,
returns 0 for ANY CODE_LABEL there because `label == NULL_RTX`).

**E-s26-4 (why our label slot is empty - located exactly).**  In our RTL the slot
between jump_insn 78 and insn 82 holds `(note 80 78 82 "" NOTE_INSN_DELETED)`,
and it is ALREADY a deleted note in the `.rtl` dump - i.e. the chain
fall-through `return 1;` block has never had a CODE_LABEL at any point in the
pipeline, in any of .rtl/.jump/.cse/.loop/.combine/.jump2/.lreg/.greg.  The
`temp_v1 == 0x14` exit does NOT reference it: `jump_insn 31` carries
`(label_ref 640)`, and `code_label 640` is a SECOND, physically separate
`li v0,1; j 662` block in the function tail (insn 643 + jump_insn 645, sitting
just before `code_label 649` = block_13).  GCC never cross-jump-merges the two
identical `return 1;` blocks (if it did, the function would be 213 insns, and
both our build and target are 215).  reorg then
`steal_delay_list_from_target`s `li 643` into jump_insn 31's slot and
`reorg_redirect_jump`s it to the epilogue label 662, after which
`code_label 640`'s block is dead and disappears - that is target's own emitted
18-19 (`beq v1,a0,.L80028700 / li v0,1`), which our build already reproduces
bit-for-bit.  So the s25 model ("the single surviving reference is consumed by
reorg's own reorg_redirect_jump") was pointing at the WRONG block: the reference
that reorg consumes belongs to the tail duplicate at label 640, and the chain
fall-through block is simply unlabelled from birth.

**E-s26-5 (the label route's remaining C-level supply is FORECLOSED BY EXIT
TOPOLOGY - measured).**  For a CODE_LABEL to be born in front of insn 82, some
C-level `goto` must target the chain fall-through `return 1;`.  Target's exit
topology is fixed and our body already reproduces it exactly: FOUR
`addu $v0,$s6,$zero` exits (target.txt lines 69, 96, 138, 204) and ONE literal
`li v0,1` exit (the chain fall-through), plus the tail duplicate at label 640
that reorg consumes.  Every C spelling of a second inbound edge converts one of
those four `return ret;` statements into `goto ret_one;`, which DELETES that
`move v0,s6` exit and changes the instruction count.  Measured this session on
the 2-floor chassis, with `ret_one:` on the chain fall-through `return 1;`:
  - Z1, `goto ret_one;` from the `temp_v1_2 == 0xE` early exit inside block_15:
    **20 / 213**.
  - Z2, `goto ret_one;` from `block_49`: **9 / 216**.
  - Z3, `goto ret_one;` from the `do_calls` exit: **4 / 213**.
Together with s25's Y1 (temp_v0 exits, 2/215 byte-neutral), Y2 (block_13, 8/216),
Y3 (temp_v1 == 0x14 exit, byte-IDENTICAL) and Y4 (Y1+Y3, byte-identical), every
`return`-carrying site in the function has now been tried as the donor of the
second edge, and every one of them either folds away (the donor was already
routed to the $s6 block, so the goto is byte-inert) or costs an exit (so the
instruction count moves off 215).  There is no spare exit to donate: the label
route is CLOSED for edge-supply spellings.

**E-s26-6 (what is left, stated precisely).**  A CODE_LABEL in front of insn 82
that does NOT come from an exit edge.  Three shapes have never been measured:
  (i) making jump_insn 31's `(label_ref 640)` point at the CHAIN fall-through
      block instead of the tail duplicate, while keeping the tail duplicate alive
      for an independent reason - reorg's redirect would then drop the chain
      block's NUSES from 2 to 1 and the label survives;
  (ii) a spelling in which jump_insn 31's delay slot is filled by the SIMPLE pass
      (backward scan) instead of by `steal_delay_list_from_target`, so no
      `reorg_redirect_jump` runs at all in round 0 - DBRDBG currently shows
      `simp insn=31 trial=30 setneed=1` (the only backward candidate sets a
      resource the branch needs);
  (iii) a source form in which the range chain's fall-through `return 1;` is
      itself a labelled join of two source paths that BOTH already exist in
      target's 215 insns (i.e. the label is paid for by code target already has,
      not by a converted exit).

- [s26] Chassis: 2 / 215 confirmed at session start and again at session end with candidate.c applied to src/code6cac_b.c; candidate.c unchanged from s25.
- [s26] TOOLING: tmp/grind/func_800283D0/s26/iso.py + iso.sh isolate func_800283D0 into its own .i and reproduce the residual with a 260-line (vs 3763-line) DBRDBG trace and unambiguous insn UIDs. s25's quoted `thr insn=78 thread=84 opp=270 oppregs=20000380` line is a DIFFERENT function of the TU - UIDs restart per function in a whole-TU dump.
- [s26] RE-ATTRIBUTION: the 2-point residual is `steal_delay_list_from_fallthrough` (reorg.c:1743, called from reorg.c:3615), NOT a plain fall-through thread steal. Round-0 `fill_simple_delay_slots(first,0)` already produces TARGET'S EXACT arrangement (`simp insn=84 trial=82 elig=1`: `li v0,1` into the `j`'s slot, beq 78's slot empty); round-0 `fill_eager_delay_slots` then steals it back OUT of the `j`'s delay slot into the beq's. That function has no DBRDBG print, which is why s25 saw a `thr` line with no `trial`/`WINNER` follow-up.
- [s26] RTL names for this site (stable .greg -> .dbr): jump_insn 78 = chain's last `beq v1,v0,.L45`; insn 82 = `li v0,1`; jump_insn 84 = `j 662`; code_label 87 = .L45; code_label 662 = epilogue.
- [s26] GUARD ENUMERATION (reorg.c:1762-1795): `sets` and `needed` are EMPTY at the steal because the thread's first element IS the sequence, so the only resource knob is `other_needed` = live-at-.L45 (measured `oppregs=0x20750000` = {s0,s2,s4,s5,s6,sp}; $v0 absent) - the s19-s24 route, still foreclosed. The only cheap axis is the outer `own_fallthrough` gate at reorg.c:3817.
- [s26] The chain fall-through `return 1;` block has NEVER had a CODE_LABEL: `(note 80 78 82 "" NOTE_INSN_DELETED)` sits there from the `.rtl` dump onward, in every dump through `.greg`.
- [s26] CORRECTION TO THE s25 MODEL: the `temp_v1 == 0x14` exit does not reference the chain fall-through block. `jump_insn 31` carries `(label_ref 640)`, and `code_label 640` is a SECOND, separate `li v0,1; j 662` block in the function tail (insn 643 + jump_insn 645). reorg steals `li 643` into jump_insn 31's slot and redirects it to label 662 - that is target's emitted 18-19, which our build already matches. The reference reorg consumes belongs to the tail duplicate, not to the block we need labelled.
- [s26] GCC never cross-jump-merges the two identical `li v0,1; j 662` blocks; if it did the function would be 213 insns, and both our build and target are 215.
- [s26] KILLED (this session, on the 2-floor chassis, `ret_one:` on the chain fall-through `return 1;`): Z1 `goto ret_one;` from the `temp_v1_2 == 0xE` exit -> 20 / 213; Z2 from `block_49` -> 9 / 216; Z3 from the `do_calls` exit -> 4 / 213. Each deletes one of the four `move v0,s6` exits target has.
- [s26] FORECLOSED BY EXIT TOPOLOGY: target has exactly four `addu $v0,$s6,$zero` exits (target.txt 69/96/138/204) plus one literal `li v0,1` exit, and our body reproduces all five. Every C-level second inbound edge to the chain fall-through block must convert one `return ret;` into a `goto`, deleting a `move v0,s6` exit. Combined with s25's Y1/Y2/Y3/Y4, every return-carrying site in the function has now been tried as the donor; the label route is closed for edge-supply spellings.
- [s26] STILL OPEN (never measured): (i) making `jump_insn 31` reference the CHAIN fall-through block while the tail duplicate stays alive independently, so reorg's redirect drops NUSES 2 -> 1 and the label survives; (ii) a form in which jump_insn 31's slot is filled by the SIMPLE backward scan (currently `simp insn=31 trial=30 setneed=1`) so no `reorg_redirect_jump` runs; (iii) a source form where the chain fall-through `return 1;` is a labelled join of two paths target already pays for.

- [s26] Chassis re-measured: the s25 banked body is 2 / 215 on HEAD at session start and again at session end with candidate.c applied to src/code6cac_b.c. Instruction count still exactly 215 == 215. candidate.c body unchanged this session (header note added).

- [s26] NEW TOOLING: tmp/grind/func_800283D0/s26/iso.py slices the preprocessed TU down to declarations + the single definition of func_800283D0 by top-level brace tracking; iso.sh compiles that iso.i through the instrumented cc1 with BB2_DBR_DEBUG=1 -dp -da. It reproduces the residual exactly with a 260-line DBRDBG trace instead of 3763 and with unambiguous insn UIDs. Use iso.sh, not dbr.sh, for any further reorg forensics on this function.

- [s26] UID HAZARD (cost s25 a mis-attribution): a whole-TU cc1 dump restarts insn UIDs per function, so `insn=78` appears in several functions of code6cac_b.i. s25's quoted `thr insn=78 thread=84 opp=270 oppregs=20000380 WINNER trial=84` line is NOT func_800283D0's.

- [s26] RE-ATTRIBUTION: the 2-point residual is steal_delay_list_from_fallthrough (reorg.c:1743, called at reorg.c:3615), not a plain fall-through thread steal. Round-0 fill_simple_delay_slots(first,0) already produces target's exact arrangement (DBRDBG `simp insn=84 trial=82 refset=0 setset=0 setneed=0` / `elig=1`: li v0,1 into the j's delay slot, beq 78's slot empty), and round-0 fill_eager_delay_slots then steals it back OUT of that delay slot into the beq's. MAX_REORG_PASSES is 2 but the whole residual is decided in round 0.

- [s26] RTL names for this site, stable from .greg to .dbr: jump_insn 78 = the range chain's last `beq v1,v0,.L45`; insn 82 = `li v0,1`; jump_insn 84 = `j 662`; code_label 87 = .L45; code_label 662 = the epilogue label; code_label 649 = block_13.

- [s26] GUARD ENUMERATION (reorg.c:1762-1795): at the steal, `sets` and `needed` are both empty (fill_slots_from_thread clears them at reorg.c:3388-3389 and the thread's first element IS the already-filled sequence), so the only resource knob is other_needed = live-at-.L45, measured oppregs=0x20750000 = {s0,s2,s4,s5,s6,sp} with $v0 absent. The only cheap remaining axis is the outer own_fallthrough gate at reorg.c:3817.

- [s26] The chain fall-through `return 1;` block has NEVER carried a CODE_LABEL: (note 80 78 82 "" NOTE_INSN_DELETED) sits between jump_insn 78 and insn 82 in every dump from .rtl through .greg.

- [s26] CORRECTION TO THE s25 MODEL: the temp_v1 == 0x14 exit does not reference the chain fall-through block. jump_insn 31 carries (label_ref 640), and code_label 640 is a separate tail `li v0,1; j 662` block (insn 643 + jump_insn 645). reorg steal_delay_list_from_target()s li 643 into jump_insn 31's slot and reorg_redirect_jump()s it to label 662 - that is target's emitted 18-19 (`beq v1,a0,.L80028700 / li v0,1`), which our build already matches bit-for-bit.

- [s26] GCC never cross-jump-merges the two identical `li v0,1; j 662` blocks; if it did the function would be 213 insns, and both our build and target are 215.

- [s26] KILLED this session on the 2-floor chassis (ret_one: on the chain fall-through `return 1;`): Z1 `goto ret_one;` from the temp_v1_2 == 0xE exit -> 20 / 213; Z2 from block_49 -> 9 / 216; Z3 from the do_calls exit -> 4 / 213.

- [s26] FORECLOSED BY EXIT TOPOLOGY: target has exactly four `addu $v0,$s6,$zero` exits (target.txt 69/96/138/204) plus one literal `li v0,1` exit, and the banked body reproduces all five. Every C-level second inbound edge to the chain fall-through block must convert a `return ret;` into a `goto`, deleting a `move v0,s6` exit and moving the insn count off 215. With s25's Y1/Y2/Y3/Y4 this exhausts every return-carrying site as a donor.

- [s26] POLICY (unchanged, still open before any candidate-ready): the banked body carries the annotated `do { calls } while (0);` wrap (.claude/rules/do-while-zero-exception.md) and the sel19 arm's duplicated `*(s16 *)(arg0 + 0x286) = 0x19;` store (.claude/rules/duplicated-statement-into-arms.md, which mandates a FAKE annotation the body does not yet carry).

## s27 (solver, 2026-08-27) - FLOOR 2 -> 0.  BYTE MATCH.  The reorg fall-through steal is defeated by reading the CODE_LABEL out of TARGET'S OWN BRANCH TARGETS instead of hunting for a goto donor

**E-s27-0 (chassis).**  s25/s26's banked body re-measured **2 / 215** on HEAD at
session start (the driver's dispatch line again said "measurement unavailable";
the ledger number was correct).

**E-s27-1 (SOLVER TRIAGE - the whole solver axis is FORECLOSED for this function).**
`python3 tools/ra_solver/inverse_compose.py classify code6cac_b func_800283D0`
returns **`FIRST DIVERGENCE: IDENTICAL` - "the honest stream already equals target
for this function"** (honest 213 insns, target 213 insns after the classifier's
delay-slot normalisation).  This is a TYPED verdict, not a guess: the residual is
invisible to all three ra_solver/sched_solver models because it lives BELOW them -
the insn multiset, the register assignment and the sched.c order were all already
target-exact, and only the post-sched reorg.c delay-slot arrangement differed.  Any
future session that reaches for `inverse.py` / `inverse_reload.py` /
`inverse_sched.py` / `perturb.py` on a residual that classify calls IDENTICAL is
searching a model that cannot express the difference.

**E-s27-2 (three NEW mechanism kills inside reorg.c, derived from the source, that
close the axes s26 left implicitly open).**
  - **The `prediction` axis is dead twice over.**  reorg.c:3784 computes
    `prediction = mostly_true_jump (insn, condition)` and only `prediction > 0`
    takes the target-thread-first path at reorg.c:3790.  For our beq it is
    structurally pinned to 0: `rare_destination` returns 0 IMMEDIATELY for any
    CODE_LABEL, so `rare_dest` is 0 for every labelled branch target;
    `rare_fallthrough` is also 0 because the fall-through block ends in a
    simplejump whose JUMP_LABEL is the epilogue CODE_LABEL; the difference is 0,
    the switch breaks, and the condition switch then returns 0 for `EQ`.  The
    branch sense is byte-pinned (target's emitted 44 is itself a `beq`), so the
    `NE -> 1` arm is unreachable without a divergence.  AND EVEN IF prediction
    were > 0, reorg.c:3797 still calls `fill_slots_from_thread` on the
    fall-through when the target thread yields nothing, so the same steal runs.
  - **The `condition == 0` skip (reorg.c:3764-3765) is unreachable from C.**
    `get_branch_condition` returns 0 only when the pattern's LABEL_REF does not
    equal `JUMP_LABEL (insn)` - an RTL bookkeeping inconsistency, not a source
    property.
  - **`eligible_for_delay` is not a lever.**  `li v0,1` is `can_delay = yes`
    (type not in the excluded set, `hazard = none`, `length = 1`), and target
    itself puts that exact insn in a delay slot, so no C spelling makes it
    ineligible.
  So the ONLY reachable gate really was `own_fallthrough` (reorg.c:3817) - s26's
  conclusion is now closed-form rather than enumerated.

**E-s27-3 (THE ANSWER - the label was in target's asm the whole time).**  s25/s26
searched for a C-level `goto` that would put a CODE_LABEL in front of the chain
fall-through `li v0,1`, and foreclosed every donor (Y1-Y4, Z1-Z3) on the grounds
that each converts one of target's four `move v0,s6` exits into a goto and moves
the insn count off 215.  That reasoning assumed the three early exits were
`move v0,s6` exits.  **They are not.**  Reading target's branch operands:
  - `.L80028488` = 0x80028488 = func + 0xB8 = **emitted index 46**, which is the
    chain fall-through block's `j .L80028700` (its delay slot at 47 is `li v0,1`).
  - `beq $v1,$a2,.L80028488` at emitted 15   (`temp_v1 == 4`)
  - `beq $v0,$a2,.L80028488` at emitted 22   (`temp_v0 == 4`)
  - `beq $v0,$a0,.L80028488` at emitted 24   (`temp_v0 == 0x14`)
  So THREE of target's own early exits jump straight into the shared `return 1;`
  block - the same block the range chain falls through to.  That is where the
  CODE_LABEL comes from, and it is FREE: those three exits were never
  `move v0,s6` exits, so nothing is converted and target's four `move v0,s6`
  exits all survive untouched.

**E-s27-4 (why every previous BB2 body missed it, and why the s26 foreclosure was
locally right).**  Every body from s1 to s26 spelled those three exits as
`return ret;` reached through `block_13`.  `ret` lives in $s6, so cse rewrites
`return ret;` / `return 1;` at those sites into `move v0,s6` and routes them to
the shared tail block at func + 0x32c - which is why adding a `goto ret_one;`
edge on top of that body (s25 Y1/Y3/Y4) was byte-inert or (Y2/Z1-Z3) cost an
exit.  The fix is not to ADD an edge to `block_13`; it is to DELETE `block_13`.

**E-s27-5 (P1 - the shape that fails, measured).**  P1 keeps the s26 nesting
(`if (temp_v1 != 4) { if (temp_v1 != 0x14) { ... goto ret_one; } return ret; }
goto ret_one;`) and only adds `ret_one:` + redirects.  Measured **8 / 216**,
i.e. exactly s25's Y2.  The emitted stream shows why: GCC MOVES the shared
`return 1;` block out of the chain to the function tail (it lands at func + 0x330
just before the epilogue), the chain's last test inverts to
`bne v1,v0,<tail> / li v0,1`, and the old fall-through path needs a new
`j 0x668` to get back - +1 insn, and the `move v0,s6` tail exit is lost.  The
trailing `goto ret_one;` (the fall-through of the outer `if`) is what pulls the
block out.  Banked as
rejected/flat-goto-ret-one-keeps-block13-tail-move-8-216.c.

**E-s27-6 (P2 - the shape that MATCHES, measured 0 / 215).**  Flatten the two
outer tests into early guards so no trailing jump exists:

    if (temp_v1 == 4) { goto ret_one; }
    if (temp_v1 == 0x14) { return ret; }
    {
        u16 temp_v0 = *(u16 *)(temp_s4 + 0x6A);
        if (temp_v0 == 4) { goto ret_one; }
        if (temp_v0 == 0x14) { goto ret_one; }
        { ... unchanged s25/s26 body, with `ret_one:` on the range chain's
          fall-through `return 1;` ... }
    }

`block_13` disappears entirely.  `sandbox func_800283D0 --disable all` prints
`"score": 0, "target_insns": 215, "build_insns": 215`.  Emitted 15 is now
`beq v1,a2,func+0xb8` (target's `.L80028488`), and emitted 44-47 is
`beq v1,v0,func+0xc4 / nop / j func+0x330 / li v0,1` - byte-identical to target.
Adding the five `/* FAKE: ... */` annotations and removing the vestigial brace
pair left it at 0 / 215.

- [s27] FLOOR 2 -> 0.  BYTE MATCH at 215 == 215 insns with the P2 body in src/code6cac_b.c.  candidate.c updated; self_vet.md written.
- [s27] SOLVER VERDICT (typed, mechanical): `inverse_compose.py classify code6cac_b func_800283D0` = `FIRST DIVERGENCE: IDENTICAL`.  The residual lived below every ra_solver/sched_solver model (post-sched reorg.c), so the entire solver axis was FORECLOSED for this function.  Treat an IDENTICAL classify verdict as "stop searching RA/sched, the difference is in reorg or in emission".
- [s27] KILLED (reorg.c source, closed form): the `prediction` axis - `mostly_true_jump` is pinned to 0 because `rare_destination` returns 0 immediately for any CODE_LABEL (so rare_dest = 0) and the fall-through block ends in a simplejump to the epilogue label (so rare_fallthrough = 0), leaving the `EQ -> 0` arm; and even prediction > 0 falls back to the same fall-through steal at reorg.c:3797.
- [s27] KILLED: the `condition == 0` skip at reorg.c:3764 (reachable only via a JUMP_LABEL/pattern inconsistency, not from C) and the `eligible_for_delay` gate (`li v0,1` is can_delay=yes and target itself delay-slots it).
- [s27] THE ANSWER: target's `.L80028488` is emitted index 46 - the chain fall-through block's own `j` - and target's `temp_v1 == 4`, `temp_v0 == 4` and `temp_v0 == 0x14` exits (emitted 15, 22, 24) all branch to it.  Those three exits are gotos into the shared `return 1;` block, NOT `move v0,s6` exits.  That supplies the CODE_LABEL that makes own_thread_p return 0, own_fallthrough = 0, and steal_delay_list_from_fallthrough never runs - at zero exit cost.
- [s27] CORRECTION TO THE s26 FORECLOSURE: "every C-level second inbound edge must convert one of target's four `move v0,s6` exits" was true only for bodies that keep `block_13: return ret;`.  The three donors target actually uses were never `move v0,s6` exits.  The move was to DELETE block_13, not to add an edge to it.
- [s27] KILLED: P1 (keep the nesting, add `ret_one:` + a trailing `goto ret_one;`) = 8 / 216, identical to s25's Y2 - the trailing jump makes GCC relocate the shared block to the function tail, inverting the chain's last branch to `bne` and costing a `j` back.  rejected/flat-goto-ret-one-keeps-block13-tail-move-8-216.c.
- [s27] METHOD NOTE worth generalising: when a residual is a delay-slot arrangement, read the TARGET's branch operands and resolve every `.L` address to an emitted index before theorising about the pass.  Three of this function's labels resolved to positions that immediately named the missing control-flow edge; twenty-six sessions of pass forensics never asked where `.L80028488` pointed.
