# SELF-VET — func_8003D888

CONSTRUCTS: (1) same-variable split-init of the low mask `m1 = 1 << avail; m1 -= 1;`;
(2) fresh named intermediate `u32 hi;` holding the masked high-bit slice of the newly
loaded word, consumed by the final OR; (3) ordinary block-local `u32 p` carrying the
word pointer and then the loaded word (multi-write, plain program logic); (4) ordinary
block-locals `m2` / `shift` and the function-scope `avail` / `r`.

## T1 semantic purpose
(1) `m1 = 1 << avail; m1 -= 1;` — both statements are LIVE: the first computes a real
value, the second consumes it and produces the mask that `r = s[1] & m1;` reads. There
is no dead write. Semantically it is the same mask as the one-line form; the split is a
decomposition of a real computation, not an inert construct. It IS load-bearing for the
bytes (measured: un-split = 19, split = 0), which is why it is claimed under the
sanctioned split-init family below rather than presented as neutral style.
(2) `hi` — holds `((u32)p >> shift) & m2`, a real value that appears in the target's own
bytes (`and $v0,$v0,$a0` at 0x8003D8D8). Behaviour is byte-identical with or without the
name, so standing alone it FAILS T1: it has no observable effect on the function's output
beyond the un-named form. It is therefore claimed under the SOTN-sanctioned
named-intermediate family and carries a /* FAKE */ annotation.
(3)(4) ordinary locals with real values and real uses; each emits real instructions.

## T2 human-programmer
(1) Yes — computing a mask as "take the bit, then subtract one" is a normal spelling, and
this project already has an accepted instance of the identical shape in a matched
function (`q = v * (...); q = q >> 12;`, rob_life_ctrl_2, src/text1a_c.c).
(2) Partly — naming the high-bit slice of a freshly loaded word `hi` in a bitstream
reader is a spelling a human would plausibly write, but a reader could still legitimately
ask "why is this named rather than inlined?", so I do not rest on T2: the construct is
claimed under a sanctioned family with its prerequisites discharged.
(3)(4) Yes — a bitstream reader with a word pointer, a mask, a shift count and an
accumulator is exactly this code.

## T3 GCC-internals justification
(2) is justified by a named GCC mechanism and I state that plainly rather than hide it:
GCC 2.7.2's RTL expansion (`expr.c` expand_binop) fixes the `iorsi3` source-operand order
from the C expression tree, and `combine` preserves it. Dump-proven this session by
diffing the two forms (tmp/grind/func_8003D888/s4/dumps_v0 vs dumps_w3): the `.combine`
IOR insn is `(ior:SI (reg 88) (reg 89))` un-named vs `(ior:SI (reg 89) (reg/v 81))` named,
while the `.greg` allocation — every conflict line, the `75 preferences: 3` line and the
register dispositions — is byte-identical between the two forms. That is exactly why the
naming works where swapping the operands in the source expression does not: the swap also
moves the statement's LUID, which changes sched1's order, REG_LIVE_LENGTH and
`allocno_compare`, and regresses the build to score 17 (measured).
(1) is justified the same way as its precedent: the split raises the pseudo's
`reg_n_refs` / set count and `combine` folds the pair back into the single emitted
`addiu $v0,$v0,-1`, so it is byte-neutral in instruction count.
(3)(4) need no GCC-internals justification — they are program logic.

## T4 permuter/search provenance
Honest disclosure: this session's modality was `permuter`, and construct (1) came out of a
decomp-permuter campaign (tmp/perm_3D888_s4c, output-10-1, weighted 670 -> 10). It is NOT
banked because a detector missed it. It was (a) re-measured by hand in the cheat-invisible
sandbox (13 -> 1), (b) minimised — the permuter's `m1 = 1 << avail; m1 = m1 - 1;` was
re-spelled to the exact owner-sanctioned shape `m1 -= 1` and re-measured at 0, and
(c) proven load-bearing by removing it (w1 = 19). Construct (2) was NOT a permuter find:
it was hand-derived from the single remaining instruction diff (`or v1,v1,v0` vs
`or v1,v0,v1`) and chosen out of a 7-member hand-written operand-order sweep in which
every other spelling was measured worse (17/18/18/18/19/29). Both survive on their own
measurements and their dumps, not on search provenance.

## T5 family check
(1) same-variable split-init accumulation — owner ruling 2026-06-13, live code, Judge PASS
precedent cited below. Not a dead store (the intermediate is read by the `-=`), not a
constant holder, not a cancellation pair (the operands `1 << avail` and `1` are the real
operands of the real expression `(1 << avail) - 1`, not an invented decomposition of a
constant — this is the distinction the `v1 += 2; v1 -= 1;` FAIL at
docs/grind/decisions.md:1790 turned on).
(2) named-intermediate declaration order, no-new-park-categories.md SOTN-accepted section
as clarified by owner rulings 2026-08-17 and 2026-08-31. Checked against the neighbouring
families and it is none of them: not variable reuse (fresh local, not a borrow — so
staged-value-reused-variable does not apply), not a dead store (it is read), not a
constant holder (it holds a computed value), not a pointer alias, not
duplicated-statement-into-arms, not a dead array/pad, not a do-while(0) wrap, not
volatile of any kind.
No register pins, no `__asm__`, no scheduling barriers, no volatile coercion, no alias
renames, no width-cast padding anywhere in the diff.
(3)(4) no family needed — ordinary C.

## T6 naming-announces-intent
`m1`, `m2`, `shift`, `p`, `hi`, `avail`, `r`, `n`, `s`. None is `pad`/`dummy`/`unused`/
`spill`-style coercion vocabulary; `hi` is the domain word for the high-bit slice of a
bitstream word and it is genuinely read on the next line. Every local is written AND
read; there is no address-of, no discard, no unused declaration.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: same-variable split-init accumulation
  SCOPE: "(3) Sanctioned family: same-variable split-init (owner ruling 2026-06-13, memory/feedback/split-init-accumulation-sanctioned.md, precedent func_80049C24 / commit ad11a8c8, 'var = a; var += b;')."
  PRECEDENT: docs/grind/decisions.md:1075
  FAMILY: named-intermediate declaration order
  SCOPE: "- **Named-intermediate declaration order** ([[narrow-byte-args-packed-call]]"
  PRECEDENT: .claude/rules/no-new-park-categories.md:204
  (The rule file hard-wraps this scope sentence across lines 204-207; unwrapped it reads
  "Named-intermediate declaration order (narrow-byte-args-packed-call hi/lo sub-trick):
  declare a sub-expression as a separately-named local to bias LUID. SOTN's `randy` chain
  in `src/weapon/w_037.c` is the same mechanism." The 2026-08-17 clarification at
  .claude/rules/no-new-park-categories.md:208-237 supplies the six prongs checked below,
  including its statement that a fresh named intermediate qualifies "**whatever GCC
  pass it acts through**", which is what licenses the expand_binop mechanism here.)

ANNOTATION-CONFORMANCE:
  /* FAKE: `hi` names the masked high-bit slice of the freshly loaded word,
     mechanism: GCC 2.7.2 RTL expansion (expr.c expand_binop) fixes the iorsi3
     source-operand order from the C expression tree and combine preserves it --
     naming the slice moves it to operand 1 (`or v1,v1,v0`, target) without
     touching statement order, so sched1's order and the greg allocation are
     byte-identical to the un-named form (dumps diffed:
     tmp/grind/func_8003D888/s4/dumps_v0 vs dumps_w3), whereas swapping the
     operands in the source expression itself also moves the LUID and regresses
     the allocation (score 17), lever-exhaustion: memory/grind/func_8003D888/
     hypotheses.md s4 (7 operand-order spellings measured 1/17/18/18/18/19/29)
     + evidence.md s1-s4. */
  The annotation carries all three required parts: WHAT (`hi` names the masked high-bit
  slice), MECHANISM (a named GCC pass — RTL expansion / expr.c expand_binop, preserved by
  combine, dump-proven), and LEVER-EXHAUSTION (a pointer to hypotheses.md s4's measured
  7-spelling operand-order sweep plus the s1-s4 evidence bank).
  Construct (1) carries no /* FAKE */: the split-init family is live code, and its Judge
  PASS precedent states verbatim "it is live code, so no FAKE annotation is required
  (annotations attach to the dead-code carve-outs, which this is not)"
  (docs/grind/decisions.md:1075).

## Named-intermediate six-prong check (owner rulings 2026-08-17 / 2026-08-31)
(1) once-written: `hi` is written exactly once and read exactly once. PASS.
(2) real value: it holds `((u32)p >> shift) & m2`, which is emitted as the target's own
    `srlv $v0,$a1,$v0` / `and $v0,$v0,$a0` pair — the naming only relocates where the
    value is named, it does not invent a value. PASS.
(3) byte-neutral: sandbox reports build_insns 37 == target_insns 37, score 0. PASS.
(4) fresh local, not a borrow: `hi` is declared inside the if-arm and exists nowhere else
    in the function. PASS.
(5) destination not live-pre-initialized: `hi` has no initializer and no prior value.
    PASS.
(6) standard prerequisites: dump-proven named mechanism (see T3, dumps_v0 vs dumps_w3),
    documented lever exhaustion (hypotheses.md s4's 7-spelling sweep plus the s1-s3
    structural exhaustion of the seating axes), /* FAKE */ annotation present in
    src/code6cac_c2.c, layer-1 + layer-2 review to follow. PASS.
