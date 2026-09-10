# SELF-VET — func_80018094  (s9, 2026-09-09 — filed with a `ruling-request`, NOT a candidate-ready)

The body under vet is `memory/grind/func_80018094/candidate.c` (= tmp/grind/func_80018094/s9/f1.c),
which measured `sandbox func_80018094 --disable all` == **0** (153/153, rules_dropped 0) this
session with the body spliced into src/code6cac.c. It is NOT submitted, because one construct
below has no frozen-family home and the first-reach rule makes that a ruling question.

CONSTRUCTS: (1) oversized locals object `s32 sp_tmp[4]` with only element 0 written/read;
(2) `do { ... } while (0);` wrap around the inner if/else + scale tail; (3) a second, tied inline-asm
output on the LZC island (`"=r"(lut)` with the input constrained `"1"(sum_sq)`) whose register the
island template never writes; (4) one variable spelling both `sum_sq` and the arm's log2 result;
(5) a named local `lut` for the LUT byte the arm already computes.

## T1 semantic purpose
(1) no observable effect — it is a frame-size device; sanctioned only under the oversized-locals
carve-out. (2) no observable effect — control flow is identical without it. (3) no observable
effect — `lut`'s value after the asm is never read; the construct exists solely to make reload
emit `move $a0,$a1`. (4) semantic: one variable now carries the value through its two lifetimes;
byte-observable only as register choice, but it is an ordinary refactor a human can motivate
("the distance is replaced by its log"). (5) semantic: `lut` names a real subexpression and is
read on the next line; measured byte-neutral on its own (s9/d5.c == 5).

## T2 human-programmer
(1) no — a programmer writes `s32 sp_tmp;`. (2) no — the wrap has no purpose to a reader.
(3) **no, emphatically** — nobody declares an asm output that the template does not write; a
reader would ask "why is `lut` an output here?" and the honest answer is "so the compiler emits a
copy", which is the cheat smell in its pure form. (4) yes — reusing the variable that holds the
distance for the distance's log is a normal thing to write. (5) yes — naming a repeated
subexpression is normal.

## T3 GCC-internals justification
(1) function.c assign_stack_local / mips.c compute_frame_size. (2) reorg.c delay-slot filling.
(3) reload's tied-operand copy plus global.c find_reg / `preferences: 4`. (4) global.c allocno
merge (the two values become one allocno, which is the `pref_reroute` vector inverse.py returned).
Constructs 1-3 are justified BY a pass, not by program logic — 1 and 2 sit inside sanctioned
families that permit exactly that with a FAKE annotation; **3 does not**.

## T4 permuter/search provenance
No permuter was run this session. The four steps came from the ra_solver models
(`inverse_compose.py classify`, `inverse.py local --block 6`, `inverse.py global --goal 98`) and
each was spelled by hand and measured; none of them passes a detector only by spelling. The tied
operand is inherited from s5, where it also came from hand reasoning, not search.

## T5 family check
(1) dead-vars-local-array, 2026-07-01 written-never-read / oversized-locals carve-out — claimed
below. (2) do-while-zero-exception, owner ruling 2026-07-06 — claimed below. (4) and (5) are
ordinary C (variable reuse of a live local for its own successor value; a named intermediate that
is once-written/once-read and carries a real value present in the target's bytes).
(3) **matches no family.** It is not a register pin, not hardcoded-$N injection, not a barrier,
not volatile coercion, not a dead local — it is an inline-asm OPERAND-CONSTRAINT device, a class
the frozen list does not contain and `docs/reference/sotn-construct-index.md` does not index at
all (searched; s8 recorded the same negative). By the first-reach rule that makes it a
ruling-request subject, and this session files one rather than submitting.

## T6 naming-announces-intent
No `pad` / `dummy` / `unused` / `spill` names. `sp_tmp` is the inherited name for the live LZC
output object; `lut` names the LUT byte it holds; `lw_v1` / `li_v0` are the COMPLETED-C sibling
func_8001A67C's own names for the same statements (src/code6cac.c:911-914).

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: dead-vars-local-array (written-never-read / oversized-locals carve-out)
  SCOPE: "FORBIDDEN as of 2026-05-31 (expanded 2026-06-01) — unused local arrays and (void)&scalar address-coercion. NARROW CARVE-OUT 2026-07-01: a WRITTEN-never-read local array is sanctioned (SOTN dra/62DEC.c ships u8 sp70[4] written 4x/read 0x, twice, in matched core) when the TARGET bytes contain the dead"
  PRECEDENT: .claude/rules/dead-vars-local-array.md:1

  FAMILY: do-while-zero-exception
  SCOPE: "SANCTIONED (owner ruling 2026-07-06, supersedes the 2026-06-04 mechanism-scoping): `do { ... } while (0);` (any body, incl. empty) is an allowed pure-C match device for ANY codegen effect incl. register allocation, with mandatory inline FAKE annotation; nested wraps need a single-level-insufficient"
  PRECEDENT: .claude/rules/do-while-zero-exception.md:1

  FAMILY: (none claimed for the tied asm operand — that is the ruling question)

ANNOTATION-CONFORMANCE:
  /* FAKE: unwritten tail sp_tmp[1..3] on the live LZC-output locals object, mechanism:
     function.c assign_stack_local / mips.c compute_frame_size ... lever-exhaustion:
     memory/grind/func_80018094/hypotheses.md s2 H15, s3 H19-H22, s4 (8,906 permuter iterations) */
  /* FAKE: do{...}while(0) wrap around the whole inner if/else + scale tail, mechanism: reorg.c
     delay-slot filling / jump2 block ordering (dropping the wrap costs 13 insns, measured
     tmp/grind/func_80018094/s9/g4.c), lever-exhaustion: hypotheses.md s5 H26-H28, s6 H29-H32,
     s7 H31-H37, s8 */
  The tied operand carries an explicit UNCLASSIFIED-DEVICE comment naming it as the s9
  ruling-request subject and instructing that the body not be committed until the ruling lands.
