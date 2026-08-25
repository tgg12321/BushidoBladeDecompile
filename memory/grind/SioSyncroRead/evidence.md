# Evidence bank — SioSyncroRead

## [s1] 2026-08-25 — recon → sandbox 0 in one session (ruling-request on one grant)

**Identity.** PsyQ LIBCOMB COMB.OBJ static `SioSyncroRead` @0x8008BF04 (census
memory/closer/psyq-library-census.md:313; queue item renamed from
cpu_side_move_dir_3). Verbatim-linked Sony module. Its module siblings are ALL
solved in this TU: SioAnsyncRead (main.c:2895, matched), SioAnsyncWrite
(main.c:3019, matched), and crucially **SioSyncroWrite (main.c:3034, COMPLETED-C
2026-08-19)** — the structural twin whose ledger produced the D_800F1AE2 volatile
grant (allowlist line 46) and the FAKE pointer-handle spellings this session
reused. Read the twin's committed body FIRST in any future session.

**Chassis.** Rule-era committed body (6 regfix: $19<->$20 @0-134, $16<->$17
@68-121, subst 32/28 @1+5, fill_delay @66<-68, $3<->$6 @39-44) carried `la`-asm
handles + memory barriers; sandbox baseline (cheat-stripped) = **41**, 154/160
insns, with a phantom callee-save $s5 and frame 0x30 (vs target 0x28)
manufactured by the stripped asm.

**Measured ladder this session (each step = one sandbox measurement):**
| variant | change | score | insns |
|---|---|---|---|
| rule-era | (baseline, cheat-stripped) | 41 | 154 |
| A | fresh twin-modeled body, direct globals, goto loop | 24 | 158 |
| B | + FAKE pointer handles p_ae2/p_b04 (volatile, granted syms) | 22 | 160 |
| C | + outer/inner loops as real do-while (loop notes) | 11 | 160 |
| D | + `count = 0;` moved ABOVE the `if (flag[2]==0)` test | 10 | 159* |
| E | + p_ae0 handle made volatile (UNGRANTED — see blocker) | 9 | 160 |
| F | + duplicated return compute into both cleanup arms (FAKE) | 3 | 160 |
| G | + main_work B00/B04 accesses respelled flag[1]/flag[2] | **0** | 160 |

*D's 159 = AE0 addiu still missing; the delay-slot fill it bought removed a nop.

**Key mechanisms proven:**
1. **Structured do-while beats goto-loop (24→11 class of wins).** The goto-spelled
   loop lacks NOTE_INSN_LOOP_BEG; with real `do{}while` the s0/s1 seat swap
   ($16<->$17 rule) disappeared AND the loop region became byte-perfect.
2. **beqz delay-slot fill** (`addu $s0,$zero,$zero` in the slot, rule-era
   fill_delay @66<-68): cc1 must emit `count=0` BETWEEN the `lw` and the `beqz`;
   writing `count = 0;` BEFORE `if (flag[2] == 0)` does it (sched1 drops the
   independent move into the load-delay position; maspsx/gas reorder then puts it
   in the branch delay slot). Placement AFTER the if measured: never filled.
3. **Volatile pointer handles ARE the addiu producers.** Target's
   lui/addiu/load-0 shape on D_800F1AE0, D_800F1AE2, D_800F1B04(return) needs the
   address in a pseudo; combine.c folds a NON-volatile single-use handle back to
   lui/%lo(sym) (measured: A→B and the p_ae0 volatile flip each moved the count).
   MEM_VOLATILE_P is what blocks the fold — the twin's "combine.c symbol-fold
   defeat" annotations implicitly rode on their symbols' volatility.
4. **s3<->s4 seat swap ($19<->$20 rule) solved via solver + sanctioned ref-lift.**
   ra_solver extract (tmp/ra_solver_work/SioSyncroRead.model.json): pkt_len=p78
   (3 refs / livelen 56) vs r_arg1=p74 (3 refs / livelen 102) — allocno_compare
   priority (refs*log2(refs)*size/livelen) orders pkt_len first → it takes s3.
   No honest respelling changes that ordering (equal refs are structural: set+use
   each; livelen ratio is dictated by target's own insn placement). The
   duplicated-statement-into-arms family (the documented byte-free reg_n_refs
   lift for global-RA priority walls) applied to the return compute
   (`return r_arg1 - *p_b04;` duplicated into cleanup_A and cleanup_B) lifts
   r_arg1 to 4+ refs → wins s3; cross-jump re-merges all three copies into the
   single .L8008C14C tail — byte-neutrality VERIFIED (word-level diff clean).
5. **Scorer reloc-addend aliasing (do not chase as a real diff).** With direct
   `D_800F1B04 = ...` spellings the scorer reported 3 vs the build/src/main.o
   reference even though every linked byte matches: reference (rule-era) relocs
   are D_800F1AFC+8/+4 while direct spellings emit D_800F1B04+0/D_800F1B00+0 —
   same VA. Respelling as flag[2]/flag[1] (variant G) satisfies the scorer.
   Target splat asm itself uses %lo(D_800F1B04), so both spellings are
   byte-faithful; keep G's spelling while the reference object is rule-era.

**THE BLOCKER — D_800F1AE0 volatile grant.** p_ae0 must be `volatile u16 *` for
the byte shape (non-volatile measured 159i: combine folds the addiu). AE0 has no
allowlist row. Evidence for a Ruling-4-class grant is identical in kind to AE2's
(line 46): same verbatim Sony COMB module state block (AE0 @0x800F1AE0, AE2 @+2,
AE6 @+6), ground-truth codegen measured unreachable without volatile. Note the
asymmetry is target-real: AE6's read is direct lui/lhu %lo (non-volatile shape) —
only AE0 and AE2 carry the volatile fingerprint, so a per-symbol grant (not a
block grant) is the right ask.

**Artifacts:** tmp/grind/SioSyncroRead/s1/build.dis (final 160/160 disasm),
s1/worddiff.py + scorediff.py (diff tooling), s1/dis.sh,
tmp/ra_solver_work/SioSyncroRead.model.json (RA model, dispositions confirm
final seats s0=count s1=st s2=retries s3=r_arg1... — model extracted at the
pre-F chassis showing p74=s4/p78=s3, i.e. the swap this session's F-variant
fixed).
