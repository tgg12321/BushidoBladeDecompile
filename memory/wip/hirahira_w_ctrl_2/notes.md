# hirahira_w_ctrl_2 — WIP (current state 2026-08-05, round 2)

`src/text1a_c.c:407`, stem **`text1a_c`** (not `text1a`). A 3x3 rotation-matrix builder:
six `Judge[]` sine-table reads, ten `mult`s, nine `a1[N]` halfword stores, no calls, no loop.
Measure: `wsl bash tmp/csz/hw_sweep.sh <variant>` (driver `tmp/csz/hw_var.py`),
loads `python tmp/csz/hw_loads.py <asm> <label>`, frame `wsl bash tmp/csz/gn_frame.sh text1a_c hirahira_w_ctrl_2`.

## Baselines

| form | score | insns (target 122) | frame |
|---|---|---|---|
| HEAD (carries the volatile cheat) | 58 | 118 | 56 / vars 48 |
| **honest** (volatile removed) | **62** | 116 | 56 / vars 48 |

`src/text1a_c.c:449` `cosA = (s16)*(volatile u16 *)(&Judge[...])` is a volatile coercion
([[inline-asm-policy]] expanded catalog) and is part of the work, not the baseline. It is
worth **4 points and 2 insns** — so the honest starting distance is 62, not 58.

## The load-form split is SOLVED as a mapping (round 2)

Target's six `Judge` reads, mapped by index-expression provenance (`$a2`=`a0[1]`=angB,
`$t0`=`a0[2]`=angC, `$v1`=`a0[0]`=angA):

| read | target form | target idx |
|---|---|---|
| **cosB** | `lh` | 18 |
| **cosC** | `lh` | 21 |
| sinB | `lhu` + `sll 16` + `sra 16` | 29 |
| sinA | `lhu` + `sll 16` + `sra 16` | 36 |
| sinC | `lhu` + `sll 16` + `sra 16` | 45 |
| cosA | `lhu` + `sll 16` + `sra 16` | 63 |

So the 2/4 split is **cosB+cosC vs everything else** — and cosB/cosC are exactly the pair
whose product (`cosB_cosC`, `mult` at idx 23) is computed first. Our honest build emits
**all six as `lh`**. The three `a0[N]` param reads are already correct (3× `lhu`, two
sign-extended) — do not disturb them.

## The cheat-free mechanism is IN-TREE and it TRANSFERS

The sibling **`replay_camera_rob_back_loose3`** (same file, `src/text1a_c.c:336`, banked at
score 12, zero rules/pins/volatile) solved this exact problem; its recipe is documented in
the comment block at `src/text1a_c.c:283-297` and reads:

```c
u16 rawA;
rawA = Judge[((s16)angA + 0x400) & 0xFFF];
a1[5] = -sinA;              /* a REAL store, sitting between load and cast */
cosA = (s16)rawA;
```

`can_combine_p` refuses to fold a MEM load into a later user across an insn that may WRITE
memory, so `simplify_shift_const` never collapses
`ashiftrt(ashift(zero_extend(mem),16),16)` into `sign_extend(mem)` = `lh`, and the
`lhu`+`sll`+`sra` shape survives. **sched1 runs after combine and hoists the store back
out**, so it costs zero instructions — which is why target's nine `sh` stores are all at the
tail (idx 104-127) even though the source interleaved one.

**Both halves are load-bearing, re-confirmed here:** a `u16` staging local WITHOUT an
interleaved store is completely inert — `honest+u16sin` and `honest+u16all` both measure
62/116 with all six loads still `lh`. The store is the lever.

## Probes run (mechanism confirmed, spelling not yet found)

- `probe1` — hoist cosB/cosC above the sin reads, then
  `rawB = Judge[..]; a1[0] = cosB_cosC >> 12; sinB = (s16)rawB;`.
  **Flips sinB to `lhu`** (mechanism confirmed) but costs **2 extra param loads** (11
  halfword loads vs 9): with the masked uses moved later, GCC folds `(s16)angB` back into a
  signed `lh $3,2($4)` and re-loads `a0[1]` separately. Frame also degrades to vars 56.
- `probe2` — minimal delta, move ONLY sinB down past `cosB_cosC` with the same interleave.
  **3 `lh` + 3 `lhu`** Judge loads (moving one read flipped three), **121 insns vs target's
  122** (honest is 116), frame back to 48. Score 79 — worse than 62, but the register
  shuffle is expected mid-restructure and the instruction count is the structural signal.
  One param read still regresses to `lh $3,2($4)`.

## The frame is a consequence, not a separate item

Our `vars=48` is six 8-byte `spill_new_p*` slots, **all six untouched** by any instruction
(FRAMEDBG + `tmp/csz/gn_census.py`); target's `vars=40` is five of the same. Each orphan is
a one-use address computation combine folds into the `MEM` it feeds — at `.flow`, p106 is
`(set (reg 106) (plus (reg 105) (reg 103)))` consumed by `(set (reg/v:HI 81) (mem/s:HI (reg
106)))`, gone by `.combine`, empty conflict list, skipped by `global_alloc`, paid an 8-byte
slot by `alter_reg` for zero instructions. Six orphans = the six `Judge` reads. **A read that
becomes `lhu`+`sll`+`sra` no longer folds its address the same way**, so the six-vs-five
count should fall out of the load-form fix — work the loads, not the frame directly.
`tmp/csz/gn_orph2.py <dump>.greg hirahira_w_ctrl_2` lists the orphans.

## Next

1. Search the statement ordering that yields **exactly cosB/cosC as `lh` and the other four
   as `lhu`+sext** while keeping the three `a0[N]` param reads as `lhu`. probe2 is the
   closest start (121/122 insns); the open defect is the one param read folding to `lh`.
   The likely shape is compute-and-store interleaved per matrix element rather than
   "compute everything, then store everything" — that gives each of the four sites a real
   store to hide behind, and sched1 hoists them all back to the tail.
2. Re-check the frame (expect 40) and the register rotation once the loads are right.
3. **Do NOT commit src.** The owner runs the gate; layer-2 before any completion claim.
