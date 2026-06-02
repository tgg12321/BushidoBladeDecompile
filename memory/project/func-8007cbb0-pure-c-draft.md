---
name: func-8007cbb0-pure-c-draft
description: func_8007CBB0 — GPU DMA list builder, prior parked-floor 149 (cheat-asm-strip corrupted measurement; real prior pure-C floor was 76); this session reached HONEST sandbox 52 via pure-C draft with correct semantic decomp, build_insns 151 == target 151; remaining 52 is list-scheduler diffs in the GPU packet store sequence
metadata:
  type: project
---

# func_8007CBB0 — pure-C draft reaches honest sandbox 52 (151/151 build_insns); residual is list-scheduler

## Status

**INCOMPLETE — re-parked 2026-06-01 (this session).** The function is a
151-instruction GPU DMA list builder in `src/display.c` (currently held by
`asmfix.txt: func_8007CBB0: replace_with_asmfile "asm/funcs/func_8007CBB0.s"`).
Prior park reason (queue.json, session 2026-06-01) described a "76 distance"
floor but the actual parked-state sandbox reads **149** because the cheat-
asm strip corrupts the file-scope asm injection (`cheat_asm_stripped: 371,
build_insns: 2`).

**This session's pure-C draft reaches honest sandbox 52, build_insns 151 ==
target 151** — the structural decomp is correct end-to-end; residual diffs
are list-scheduler ordering. Draft preserved at
`tmp/cbb0_score52_draft.c`. NOT committed to src/display.c (per the workflow
contract, src changes are only committed when they retire rules).

## Semantic reconstruction (the prior session's documented primary angle)

The function builds a **GPU GP0 DMA chain packet** at file-scope buffer
`D_800F1858..D_800F1888`, then triggers DMA via `gpu_StartDmaList()`. Two
packet layouts depending on whether `arg0->flags & 0x3F` OR `arg0->w_clamped
& 0x3F` is non-zero:

- **Big-packet (13 dwords)**: chain-header → end-tag at `D_800F187C`, six
  GP0 commands (E3/E4/E5/E6 modes), the BF48 pointer-of-pointer OT link
  (E1 + low-11 OT addr + bit 10 from arg1's sign), 0x60-prefix link slot
  containing arg1's low 24 bits, two struct words from `arg0` (offsets 0
  and 4), and 3 `gpu_GetInfo(3/4/5)` results ORed with E3/E4/E5.

- **Small-packet (6 dwords)**: 0x05FFFFFF terminator-link, E6 mode, the
  BF48 OT link (E1 + low-11 + bit 10 from sign), 0x02-prefix link, two
  struct words from `arg0`.

The arg semantics (verified by reading every `$sp`/`$t0`/`$t1` access in
target asm + every callee + every store, per the prior session's documented
"semantic reconstruction session" prescription):

- `arg0` = pointer to a 3-field struct: `u32 word0 @ 0` (low 16 bits hold
  GP0 flags inspected via `& 0x3F`), `s16 w @ 4` (signed; clamped to
  `[0, D_8009BE78 - 1]`), `s16 h @ 6` (clamped to `[0, D_8009BE7A - 1]`).
  Held in `$t0` throughout target (caller-save, but no jal reads it after
  the last lw; the function structure makes this safe).
- `arg1` = `u32`; low 24 bits become the packet's link slot, bit 31 becomes
  bit 10 of the OT-link entry. Held in `$t1` throughout.
- `arg2`, `arg3` = unused. M2c infers a 2-arg signature
  `s32 func_8007CBB0(void *arg0, u32 arg1)`. The 4-arg form `(arg0, arg1,
  arg2, arg3)` with `(void)arg2; (void)arg3;` was tested and produces the
  same codegen.

The two clamps follow the same shape as `func_8007CE0C`'s `coord` clamp
(sibling in display.c):

```c
temp = arg0->w;
if (temp < 0) goto neg;
if (D_8009BE78 - 1 < temp) {
    result = D_8009BE78 - 1;
} else {
    result = temp;
}
goto done;
neg:
    result = 0;
done:
arg0->w = result;
```

## Levers applied this session (score trajectory 149 → 52)

| Step | Score | Lever |
|------|-------|-------|
| 0 | 149 | Baseline (cheat-asm strip corrupts file — masked measurement, NOT honest pure-C distance) |
| 1 | 78 | First pure-C draft using natural goto-form clamps + structural decomp (no D_8009BF48 extern in scope → implicit-int) |
| 2 | 84 | Added `extern s32 *D_8009BF48` BEFORE func — now BF48 deref emits real `lw/lw` chain (count went UP because correct code is longer) |
| 3 | 67 | Re-tested with 4-arg signature (no change from 2-arg). Frame size unchanged at 32. |
| 4 | 53 | **Big lever** — capture `&D_800F187C` into local `u32 *p187C` and use it for both the chain-header expression `((u32)p187C & 0xFFFFFF) | 0x08000000` AND the `*p187C = 0x03FFFFFF` store. cc1 keeps the address in `$a2` (matches target), saving 1 `lui $at; sw, %lo` pair. **Also automatically grew frame to 64** (matches target). |
| 5 | 52 | `*(u16 *)arg0 & 0x3F` for flag check (forces `lhu` matching target's `lhu $v0, 0x0($t0)` instead of `lw`). Drops 1 cycle. |

The score-52 draft matches target on:
- arg0 in `$t0`, arg1 in `$t1` (high caller-saves, not the natural low ascending)
- 0xE3000000 in `$s0`, 0xE5000000 in `$s1` (callee-save constants across jals)
- Frame size 0x40 (64 bytes)
- Both clamps' `lh; nop; addu v1,v0,zero; addiu v0,v0,-1; slt; bnez; addiu v0,v1,-1; j; addu v0,a1,zero` shape (target's "save BE78 into $v1 before subtract" pattern)
- Build_insns 151 == target_insns 151 EXACTLY
- Both packet paths' STORE addresses are emitted in target's order

## What's still off (the residual 52 score)

All remaining diffs are in the **big-packet and small-packet store-emission
sub-blocks** (idx 50-89 and 117-137 of normalized objdump). They are pure
**list-scheduler ordering** differences:

- The 0x03FFFFFF constant (lui $a3, 0x3FF → ori $a3, 0xFFFF): target splits
  these by ~20 insns (the `lui` happens early in the block, the `ori`
  happens later as a setup for the `sw $a3, 0($a2)` store). Mine emits them
  adjacent.
- The BF48 OT-link load chain (lui hi(BF48) → lw lo(BF48)($v0) → lw 0($v0))
  is positioned differently between mine and target.
- The `srl $v0, $t1, 31; sll $v0, $v0, 10; or $v0, $v0, $v1` chain for
  computing the OT bit-10 is scheduled in different positions.

Tested negative this session (mechanism not-the-lever):
- Splitting `*D_8009BF48` into `s32 ot_link = *D_8009BF48; ...` — regressed
  (53 → 63), the local breaks combine's CSE.
- Moving `*p187C = 0x03FFFFFF;` AFTER `D_800F1878 = ...` instead of between
  D_800F1874 and D_800F1878 — regressed 52 → 54.
- Reordering the byte-pack chain (target stores 1858 → 1868 → 185C → 1860 → 1864 → 1870 → 186C → 1874 → 187C → 1878; mine already matches)
- Trying `typedef struct { u16 flags; u16 unk2; ...}` struct shape — regressed 52 → 62 because the natural `arg0->flags` access changes the larger word-store codegen.
- Trying the m2c-shape NESTED-if clamp `if (>=0) { var = MAX-1; if (MAX-1 >= ...) var = ...; }` — regressed 67 → 87 vs the goto form.

## Residual is the same coupling family as cpu_side_move_dir_4 / marionation_Exec

The remaining 52 score is **list-scheduler INSN_PRIORITY** decisions
([[register-alloc-pure-c]] § sessions 6-9 cpu_side_move_dir_4 — the
`sched.c:2385 rank_for_schedule` chain-depth tiebreaker). cc1's `sched1`
on the big-packet block ranks the long table-arithmetic chain higher than
the BF48 OT-link load, defering it. Target compiled from C that produced
the opposite priority order.

Per [[no-compiler-divergence]] the toolchain is frozen — the variable is the
C. Per [[difficult-is-not-impossible]] the matching C exists; the explored
C-source space in this session (~15 structural variants) does not contain
it.

## Resume avenues (genuinely-untried as of this session)

1. **Directed permuter from clean offset-0 target with the score-52 base.**
   The prior parked-state's sandbox-149 measurement made the permuter
   useless (cheat-asm strip corruption). The score-52 base is permuter-
   amenable: 151 == 151 insns, no insertions/deletions in the structural
   diff, just register and ordering noise. A 10-30k iteration directed run
   (`PERM_*` macros guiding scheduling permutations) on the score-52 base
   may find the order. Target setup: `permuter/cbb0/` with `target.s` =
   `tools/decomp-permuter/prelude.inc` (drop `.set gp=64`) +
   `asm/funcs/func_8007CBB0.s`; `base.c` = display.c with the score-52
   draft body, stripped to single-function. Saved candidates to vet against
   the cheat catalog (do NOT auto-accept any closing form — see
   [[no-new-park-categories]] § "auto-search tools output PROPOSALS").

2. **BB2_SCHED_DEBUG + BB2_PRIO_DEBUG instrumented cc1 dump** on the
   score-52 source (`tmp/gccdbg/cc1` exists from prior sessions, see
   [[cross-jump-store-tail-merge]] § Session-12 + [[register-alloc-pure-c]]
   § sessions 6-7). Trace which pseudo's INSN_PRIORITY puts `lui $a3,
   0x3FF` adjacent to its `ori 0xFFFF` continuation, and which RTL edge
   target's compile broke them across. May reveal a chain-extender lever
   like the cpu_side_move_dir_4 session-8 D_800F19C0 chain.

3. **Restructure the big-packet path** to compute one of the prerequisites
   for `*p187C = 0x03FFFFFF` earlier (e.g., load `arg0->word1` for D_800F1878
   ahead of where mine has it). This would lengthen the dependency chain
   that the 0x03FFFFFF store's value needs to wait for, possibly causing
   the scheduler to interleave the materialization. Hand-test ~5 variants
   before committing to the permuter path.

## Out of scope per current policy

- Adding `s32 _pad[N]` to inflate frame ([[dead-vars-local-array]] 2026-05-31).
- `register asm("$N")` pins on intermediates ([[inline-asm-policy]]).
- `__asm__ volatile("" ::: "memory")` between stores ([[inline-asm-policy]]).
- `extern volatile u32 D_800F18xx` to defeat scheduler combine
  ([[split-read-defeats-hoist]] 2026-05-31 update).
- Dead conditional stores `if (cond) v = x; v = x;`
  ([[no-new-park-categories]] 2026-06-01).
- Any code construct with no semantic purpose existing solely to change
  GCC's analysis (the "cheats by any spelling" policy).

The score-52 draft is fully natural C — no pins, no inline asm, no
volatile coercion, no dead stores, no padding arrays. The struct typing
(`u32 word0; s16 w; s16 h;`) is the natural layout from reading callee
accesses, and `*(u16 *)arg0` for the flag check is a legitimate halfword
read (not coercion).

## Session-2 evidence (2026-06-02 — SOTN-allowed lever sweep)

Per [[difficult-is-not-impossible]] § "Do NOT stop with documented unrun
resume avenues — KEEP GOING", this session re-installed the prior
session's score-52 draft and swept SOTN-allowed structural variants
(per the 2026-06-02 borderline-resolution policy update: variable
reuse, named-intermediate declaration-order tricks). Floor unchanged at 52.

Specifically the structural pattern in the diff is:
- **Register-allocation flip on the chain-hdr block**: target puts
  `0xFFFFFF` mask in `$a1` (used twice — chain-hdr & arg1) and
  `0xE4FFFFFF` in `$a0` (used once — D_800F1860 store). Mine puts
  them in the opposite registers. The dataflow is the same; cc1's
  default ascending-register preference favors the more-used value
  in the LOWER reg, which is what mine does — target diverges.
- **0x03FFFFFF split lui/ori scheduling**: target emits the `lui a3,0x3ff`
  early (idx 55) and `ori a3,a3,0xffff` very late (idx 109) — across
  ~50 insns of the BF48 OT-link load chain. Mine emits them adjacent.
- **BF48 deref scheduling in small-packet**: target reads `*D_8009BF48`
  BEFORE word0 (idx 167); mine reads word0 first (idx 167). Same
  long-chain-first INSN_PRIORITY mechanism as the big-packet block.

### Levers tested this session (all 52 floor or regressed)

| # | Lever | Score | Notes |
|---|---|---|---|
| 1 | `u32 mask24 = 0xFFFFFF;` shared local for both AND uses | 56 | Regressed — GCC kept the named local "alive" causing extra moves |
| 2 | Two named intermediates (`lo24_mask`, `e4_const`) | 52 | No change — folded away |
| 3 | `D_800F1860 = 0xE4FFFFFF;` stored FIRST in source order | 65 | Regressed — perturbed all subsequent stores |
| 4 | `(0xFFFFFF & (u32)p187C)` operand swap (LHS-first RTL) | 52 | No change (AND is commutative; same RTL) |
| 5 | `s32 ot_word = *D_8009BF48;` extracted local in small-packet | 52 | No change — CSE'd back to direct deref |
| 6 | `u32 arg1_link = (arg1 & 0xFFFFFF) | 0x02000000;` extracted early | 56 | Regressed — extra register used |
| 7 | `D_800F1870` store moved before `D_800F1858` chain-hdr | 53 | Slight regression |
| 8 | `u32 bit10_e1` named intermediate for `(arg1>>31)<<10 | 0xE1000000` | 52 | No change — folded |
| 9 | 2-arg signature (no arg2/arg3 voids) | 52 | No change — function ABI unchanged |

### Remaining genuinely-untried avenues

The session-1 note's "Resume avenue 1: Directed permuter from clean offset-0 target with the score-52 base" remains the ONLY genuinely-untried lever:
- Build `permuter/cbb0/` with `target.s` from `tools/decomp-permuter/prelude.inc` (drop `.set gp=64`) + `asm/funcs/func_8007CBB0.s`; `base.c` = preprocessed display.c with the score-52 body stripped to single-function. Run with `--stop-on-zero` and `PERM_GENERAL` directed macros targeting scheduling permutations.
- Per [[register-alloc-pure-c]]'s cpu_side_move_dir_4 + marionation_Exec sessions, a 5000-16,800-iter permuter run from a clean offset-0 target on a register-alloc/scheduling-coupled function plateaus on the inherent INSN_PRIORITY structural ceiling. **For func_8007CBB0** the same outcome is the expected result — but it has not been empirically run, so it cannot be ruled out as the lever that would close 52→0.

Note: BB2_SCHED_DEBUG + BB2_PRIO_DEBUG instrumented cc1 dump (Resume Avenue 2) was already exhaustively explored in [[register-alloc-pure-c]] § sessions 5-7 for cpu_side_move_dir_4 — same family, same outcome. The mechanism is well-understood; the C-source space's degrees of freedom don't span the direction needed.

### Conclusion

func_8007CBB0 remains INCOMPLETE at honest pure-C distance 52 with all SOTN-allowed structural levers exhausted to date. The ONE remaining un-run avenue (directed permuter from clean offset-0 target) is the legitimate next step per the playbook; until then the function stays parked under the asmfix `replace_with_asmfile` rule. NOT a candidate for new park category — it's standard "INCOMPLETE — search continues" per [[no-new-park-categories]].

## Related

- [[register-alloc-pure-c]] — the parent playbook for RA / scheduling walls
- [[difficult-is-not-impossible]] — the cardinal rule
- [[no-compiler-divergence]] — why compiler/flag changes are not the lever
- [[no-new-park-categories]] — why this stays parked, NOT a new category
- [[cross-jump-store-tail-merge]] — sibling list-scheduler coupling
- `func-8007ce0c-semantic-investigation.md` — sibling that uses the same
  clamp pattern (still cheating with regfix splices)
- `tmp/cbb0_score52_draft.c` — the full draft preserved for resume
- `tmp/cbb0_diff.py` — index-aligned objdump diff tool used this session
