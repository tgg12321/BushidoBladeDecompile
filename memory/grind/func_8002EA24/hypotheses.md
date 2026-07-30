# Hypothesis ledger — func_8002EA24

## Live frontier (after session 1, recon)

### H1 — the `$t4` GTE-operand copy (dominant, ~9 of 20 points)
**Statement.** Target routes all three GTE operands through an extra
`addu $t4, <src>, $zero` before the `lwc2`/`swc2`/`mtc2`. Finding the C that
makes GCC emit that copy closes 3 of the 5 missing instructions and, because
the copy lands in `$t4`, simultaneously fixes ~6 register mismatches on the
canonical GTE instructions themselves.

**Mechanism.** The copy at the `mtc2` site cannot be a reload artifact —
`$a0` (a0_var) is live after the `mtc2`, so target held a genuinely separate
value. That means the original C had a *second variable* holding a copy and
GCC's local-alloc failed to coalesce it away. Session 1 KILLED the obvious
spelling (one long-lived scratch reused across all four sites, both `s32` and
`s32 *` typed: score 24, and the copy still did not appear — GCC computed
straight into the operand register).

**Next probes, in order.**
1. Per-site *pairs* of locals rather than one shared scratch:
   `s32 *base = (s32 *)(obj + 0xF8); s32 *p = base;` then the asm on `p`.
   Coalescing may be blocked when both pseudos are referenced rather than
   when one is a plain single-use copy.
2. Make the copy source live across the asm at all three sites (that is the
   one property the `mtc2` site demonstrably has). E.g. keep `vp` live past
   the `lwc2` by deriving the store address from it: `rp = vp + 2;`
   (`0x100 - 0xF8 = 8` bytes = 2 `s32`s — exactly target's relationship
   between the two addresses, and the natural way a human would write it).
3. Read the `.greg` register-disposition dump (`cc1 <flags> -da base.i`) and
   find which pseudo GCC assigns to `$t4` in any variant — that tells you
   what live-range shape earns `$t4`.
4. If 1-3 stall: decomp-permuter with `PERM_*` variants over the operand
   spelling in this region.

### H2 — the tail `xori` fold (2 missing insns + 1 wrong insn)
**Statement.** `if (y + a0 < min_y) return 0; return 1;` is if-converted by
GCC `jump.c`'s store-flag path into `slt` + `xori $v0,$v0,1`; target keeps
the unfolded diamond (`bnez` to the shared epilogue, `addu $v0,$zero,$zero`
in the delay slot, `addiu $v0,$zero,1` on fall-through).

**Mechanism.** `jump.c`'s store-flag transform requires a single-set arm.
Only the LAST `return 0` folds; the four earlier ones (not followed by a
`return 1`) keep the diamond — so the trigger is the 0/1 pair, not the
comparison shape.

**Next probes, in order.**
1. Pure-C restructurings first: shared end label
   (`ret = 0; goto end; … end: return ret;`, see `[[shared-end-label]]`),
   mixed exit forms, or a differently-shaped final comparison. These need no
   exception machinery — exhaust them first.
2. Only if 1 is exhausted: `[[dead-store-fake-exception]]` documents this
   EXACT closure ("target keeps an unfolded 0/1 diamond that your build folds
   to `return X` → dead `ret = 1;` INSIDE the else arm; a two-set arm breaks
   jump.c's store-flag single-set precondition; detached placement does NOT
   work"). That path requires documented lever-exhaustion, a `/* FAKE */`
   annotation and layer-2 review — do not reach for it early.

### H3 — `x` in `$a1` / `neg_threshold` in `$t1`
**Statement.** The compare chain allocates `x` to `$a0` and `neg_threshold`
to `$a1`; target wants `$a1` and `$t1`. This is what regfix 3454-3456
currently paper over.

**Mechanism.** Ordinary register allocation. `$t1` for `neg_threshold` is an
unusual first choice, suggesting its live range in the original spanned more
than the two comparisons (target computes `negu $t1,$a2` before the first
`slt` and reuses it for the `z` test). Likely COUPLED to H1 — closing the GTE
region changes which pseudos are live entering the compare chain.
**Re-measure H3 only after H1 lands**; expect it to move on its own.

**Next probe.** After H1: re-diff; if it persists, apply the
`[[register-alloc-pure-c]]` levers (block-local split, declaration order,
hoisting `-threshold` above the `x` load) and read `.greg`.

## Killed (do NOT re-propose)
- **Shared GTE-operand scratch variable** — one function-scope `s32 gp;` or
  `s32 *gp;` assigned before each of the four GTE asm sites. Measured score
  **24** (worse than both the 20 honest form and the HEAD 18); build 98
  insns; no `$t4` copy appeared, and the long cross-block live range
  additionally perturbed the `$a0`/`$a1`/`$v0` assignment in the compare
  chain. Saved at `rejected/shared-gte-operand-scratch.c`.
- **Anti-CSE work on the triple `lw $v1,0x60($t0)`** — unnecessary; the
  interleaved `sh` stores already prevent the CSE, and the plain-C form
  reproduces all three loads.
- **Re-deriving the `srl`/`sll` schedule (regfix 3457-3460) or the
  `slt $v0,$a1,$v0` form (regfix 3461/3462)** — the plain-C form already
  emits both in target order; those rules are stale relics of the pinned
  spelling.
- **`"\n\t"` separators inside multi-instruction GTE asm templates** — not a
  C hypothesis but a hard constraint: the classifier misreads them and the
  sandbox deletes the block (see evidence.md, TOOL ARTIFACT). Always use
  bare `"\n"`.

## [s1] The honest pure-C distance at HEAD (18) is dominated by how the GTE ops are spelled, not by the C dataflow: every one of the 11 net-missing instructions lies inside the cop2 regions.
- mechanism: HEAD writes the GTE work as register asm("t4") pins + addu %0,%1,$zero moves + standalone __asm__("nop") + .word 0xE99A0004 encodings carrying a pinned operand. engine.inlineasm strips all of those as cheat-asm, deleting 12 instructions (and leaving 1 extra xori), which is exactly 104-93=11.
- probe: Disassembled tmp/sandbox/func_8002EA24/code6cac_b.o and diffed it instruction-by-instruction against asm/funcs/func_8002EA24.s (tmp/grind/func_8002EA24/s1/dis.sh, diff.sh); then rewrote the GTE work in PsyQ-macro shape (one __asm__ per GTE group, %0 placeholders bound to an ordinary "r"(pointer) operand, pipeline nops inside the canonical template) and re-ran sandbox --disable all.
- result: Honest form builds 99 insns vs target 104 (5 structural gaps, down from 11). gas/maspsx accept swc2 $26/$27 and mtc2 %0,$30 verbatim; only mvmva still needs .word 0x4A486012. Score 20 rather than 18 solely because HEAD's hardcoded 0($12) templates match target's $t4 operand register for free.
- verdict: CONFIRMED

## [s1] Target's extra `addu $t4, <src>, $zero` before each GTE op comes from a single reused C scratch variable feeding all four asm sites (which would also explain why $t4 is the operand register at every site).
- mechanism: A long-lived scratch spanning basic blocks would be global-allocated to one hard register and, at the mtc2 site where a0_var is live afterwards, would force a genuine copy.
- probe: Declared one function-scope scratch (tried both `s32 gp;` and `s32 *gp;`) and assigned it before each of the four lwc2/swc2/mtc2 blocks; sandbox --disable all on each.
- result: Score 24 for both spellings (worse than the 20 per-site-local form and the 18 HEAD form); build shrank to 98 insns. No $t4 copy appeared at all — GCC computed the address straight into the operand register — and the long cross-block live range additionally perturbed the $a0/$a1/$v0 assignment in the compare chain.
- verdict: KILLED

## [s1] The regfix rules covering the sqrt-tail schedule (3457-3460: sll/srl/li 0x13 reordering) and the final slt operand order (3461/3462) reflect real C-level divergence that still needs a lever.
- mechanism: Those rules rewrite instruction order and register choice in the D_8008D118 shift block and the max_y comparison, which would normally indicate a scheduling/RA gap.
- probe: Compared the plain-C (unpinned, unrule'd) sandbox disassembly against target for those exact regions.
- result: The plain-C form already emits `lbu; srl v1,v1,1; li v0,19; subu v0,v0,v1; sll a0,a0,16; srlv a0,a0,v0` and `slt v0,a1,v0` in target order. Those six rules are stale relics of the pinned spelling and correspond to no residual diff.
- verdict: KILLED

## [s1] The triple `lw $v1,0x60($t0)` in target (one reload per coordinate) needs a CSE-defeat lever in the C.
- mechanism: GCC would normally CSE the repeated `*(s32**)(obj+0x60)` load into one.
- probe: Inspected the plain-C sandbox disassembly of the first three statements.
- result: All three reloads appear for free — the interleaved `sh` stores through the u8* base alias-kill the CSE. No lever needed.
- verdict: KILLED
