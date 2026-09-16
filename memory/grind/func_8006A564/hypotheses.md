# Hypotheses — func_8006A564

## H1 (session 1) — srl vs sra on the packed-color halving ops
**Statement:** casting the halved color value through `(u32)` before `>>1`
makes GCC emit `srl` instead of `sra`, matching target at both halving
sites.
**Mechanism:** GCC's shift-right codegen picks `sra`/`srl` based on the
shifted operand's signedness at the RTL level; `s32 v0` defaults to
arithmetic shift, `(u32)v0` forces logical.
**Probe:** objdump `tmp/sandbox/func_8006A564/text1b.o` before/after the
cast; grep for `sra`/`srl` at the two halving sites.
**Result:** CONFIRMED — both sites now emit `srl s0,s0,0x1` byte-identical
to `asm/funcs/func_8006A564.s:101` and `:148-149`. Sandbox masked score
unchanged at 137 (this diff was not the dominant residual — see H2).

## H2 (session 1, frontier — NOT yet probed) — register-allocation mapping
**Statement:** the +5 instruction / 137-point residual is entirely a
register-allocation mapping difference: our build's callee-save set is
{s0,s1,s2,s3,s4,s5} mapped {arg1,scratch,arg1... } vs target's
{s0,s1,s2,s3,s4} mapped {scratch,arg1,arg0,arg2,flag-local}. Fixing the
source structure (declaration order of `tile`/`obj2`/`v0`/`s4`, or
parameter-liveness shape) so GCC's global allocator assigns the same 5
registers (dropping our extra 6th) should close most or all of the
137-point gap.
**Mechanism (untested):** GCC 2.7.2's `global.c` allocno-priority ordering
is sensitive to first-use order and live-range length of each pseudo;
right now `tile` is reused/reassigned many times (long live range,
crosses all 3 tile-draw blocks + the final section) which may be pushing
it to a scratch register with a longer conflict list, forcing one extra
value to spill to a callee-save slot instead of reusing a temp already
freed by DCE.
**Next probe:** `pwsh tools/grinder/dump.ps1 func_8006A564` then read the
`.greg`/`.lreg` dump to see which pseudo lands in the extra register (s5
equivalent) and why it doesn't get folded into an already-freed hard reg;
try (a) declaring `s4`-equivalent local FIRST (before `tile`/`obj2`/`v0`)
to bias LUID, (b) splitting `tile`'s live range at the `obj2 = ...` /
`tile = *(obj2+0x1C)` reassignment into a distinctly-named local (the
asm literally reuses $s0 for two logically different pointers — a fresh
C name at that reassignment point may look "more natural" to the
allocator AND is arguably the more truthful spelling: it's a different
object). Do NOT try register-asm pins or scheduling barriers — plain
declaration-order / naming levers only, per the register-alloc-pure-c
technique family.

## H2 (session 2) — CONFIRMED: block-local var split closes the register-mapping gap
**Result:** Fully confirmed. Splitting `v0` into per-tile-draw-block `{ }`
scopes (fresh `s32 v0;` per block) dropped the hard-reg set from
{s0,s1,s2,s3,s4,s5} to exactly {s0,s1,s2,s3,s4} and the per-variable mapping
came out matching target with no additional declaration-order work needed.
Sandbox 137 -> 74. See evidence.md [s2] for the full `.greg` dump citation.
**verdict:** CONFIRMED (kill_scope n/a -- this is a positive/CONFIRMED
result, not a kill).

## H3 (session 2) — KILLED: splitting the AND expression does not fix v0/v1 coloring
**Statement:** rewriting `v0 = D_800A34F8 & 0xF;` as two statements
(`v0 = D_800A34F8; v0 &= 0xF;`) will make cc1 reuse the load's register for
the in-place AND, matching target's `andi $v0,$v0,0xF` (same reg as the
preceding `lw $v0,...`).
**Mechanism (hypothesized):** GCC's `combine`/local-alloc might coalesce a
load-then-immediately-consumed temp into the same hardreg as its single
user when they're separate insns with an obvious single-def/single-use
chain, more readily than when the RHS is a single compound expression.
**Probe:** Applied the 2-statement form at all 4 `D_800A34F8 & 0xF` sites,
re-measured via `sandbox --disable all`.
**Result:** Score went from 68 to 72 (WORSE). Reverted to the single-
expression form (`v0 = D_800A34F8 & 0xF;`).
**verdict:** KILLED
**kill_scope:** instance
**measured_on:** candidate.c s2 final chassis (block-local var split + all
s2 field-offset fixes applied), zero FAKE constructs present.

## Frontier for next session
1. **The systemic v0<->v1 register-coloring swap (68-point residual, ALL
   of it).** Read the `.lreg`/`.combine` dumps (not yet read this session
   -- `pwsh tools/grinder/dump.ps1 func_8006A564` already regenerates them
   in tmp/grind/func_8006A564/dumps/) for the FIRST `D_800A34F8 & 0xF` site
   specifically: does cc1 emit the load and the AND as two RTL insns with
   two DIFFERENT pseudo REGNOs that then get colored to v0/v1 in the
   "wrong" order relative to target, or does it coalesce them into one
   pseudo that then gets colored v1 instead of v0? The two diagnoses need
   different levers: (a) if two pseudos, a decl-order / statement-order
   change might bias local-alloc's processing order (which pseudo is
   "seen first" and gets first pick of the lowest free hardreg); (b) if
   one coalesced pseudo, the issue is a global.c allocno-priority
   tiebreak, and the lever is more likely an unrelated nearby live range
   changing conflict counts. H3 already KILLED the "split the expression
   into two statements" attempt at the SOURCE level, so this needs the
   RTL-level read before the next lever guess, not another blind
   statement-shape experiment (see the PASS ATTRIBUTION contract in the
   session brief -- guessing pass attribution across sessions is exactly
   the failure mode it exists to prevent).
2. **The single block3 scheduling tie** (`sb $v0,0x6($s0)` vs
   `addu $a0,$s0,$zero` ordering) -- low weight, likely secondary to #1;
   revisit after the coloring issue is understood, since fixing #1 may
   shift RTL insn numbering enough to change this tie's resolution for
   free.
3. If #1's RTL-level diagnosis doesn't yield an obvious C lever, run the
   permuter on `asm/funcs/func_8006A564.s` (clean single-function target
   per `difficult-is-not-impossible.md` §3) directed at register-mapping
   variants (`--stop-on-zero`) -- not yet attempted in this ledger's
   2 sessions (R3 in asm-until-matched.md caps this at 2 permuter
   sessions ever for this function; none spent yet).

## [s1] Casting the halved packed-color value through (u32) before >>1 makes GCC emit srl instead of sra, matching target at both color-halving sites (block 3 and the final color-select block).
- mechanism: GCC 2.7.2 picks arithmetic vs logical right-shift codegen from the shifted operand's C signedness; s32 defaults to sra, (u32) forces srl.
- probe: objdump tmp/sandbox/func_8006A564/text1b.o before/after the cast; diff against asm/funcs/func_8006A564.s:101,105,148,149.
- result: Both halving sites now emit `srl s0,s0,0x1` byte-identical to target. Sandbox masked score unchanged at 137 (this was a real but non-dominant instruction-level fix; the register-allocation gap in H2 dominates the residual).
- verdict: CONFIRMED

## [s2] Confining the reused `s32 v0;` local to a fresh block-scoped declaration per tile-draw block (instead of one function-wide declaration) drops the global allocator's hard-reg set from {s0,s1,s2,s3,s4,s5} to exactly {s0,s1,s2,s3,s4}, matching target's callee-save mask and per-variable register mapping with no further declaration-order work.
- mechanism: GCC 2.7.2's global.c allocates registers per-pseudo based on live-range/conflict analysis; a single source-level variable reused across every basic block in the function is treated as one long-lived pseudo whose live range crosses many blocks, forcing promotion to an extra callee-saved register. Block-scoping the declaration gives each instance a short, block-confined live range.
- probe: Read tmp/grind/func_8006A564/dumps/text1b.greg before/after the split; compare `;; Hard regs used:` line and the pseudo->hardreg disposition table against asm/funcs/func_8006A564.s's prologue mask/register moves.
- result: Sandbox --disable all score 137 -> 74 (build_insns 204 -> 202). Hard regs used dropped to exactly {2,3,4,5,6,7,16,17,18,19,20,29,31} (no s5/21), and pseudo->hardreg mapping matched target's arg0->s2, arg1->s1, arg2->s3, tile->s0, flag-local->s4 exactly.
- verdict: CONFIRMED

## [s2] Splitting the combined expression `v0 = D_800A34F8 & 0xF;` into two statements (`v0 = D_800A34F8; v0 &= 0xF;`) will make cc1 color the AND's result into the same hard register as the preceding load, matching target's `andi $v0,$v0,0xF` in-place pattern and closing (part of) the systemic v0<->v1 coloring swap that accounts for the whole 68-point residual.
- mechanism: Hypothesized: GCC's local-alloc/combine might coalesce a load-then-immediately-consumed temp into its single user's hardreg more readily when they are separate RTL insns from separate C statements than when the RHS is one compound expression.
- probe: Applied the 2-statement split at all 4 `D_800A34F8 & 0xF` sites in the s2 candidate (chassis: block-local var split + all field-offset fixes already applied, build_insns==target_insns==199, floor 68), re-measured via `& tools/wteng.ps1 main sandbox func_8006A564 --disable all`.
- result: Score regressed from 68 to 72 (worse). Reverted to the single-expression form, confirmed back at 68.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c s2 final chassis (block-local var split applied, all s1-bug field-offset fixes applied), zero FAKE constructs present
