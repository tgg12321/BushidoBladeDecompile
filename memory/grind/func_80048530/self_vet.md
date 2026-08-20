# SELF-VET — func_80048530

CONSTRUCTS: single commutative operand order `arg0 = off + base;` (off-first,
target-matching) with mandatory FAKE annotation. Nothing else in the diff is a
construct: the rest of the body is the plain floor-1 candidate (fresh-assign
walker + walking-pointer halfword reads, ordinary C, s3-banked), and the diff
REMOVES the rule-era cheat chassis (3 register-asm pins, 3 hardcoded-move
`__asm__` blocks, 2 `""` constraint barriers).

## T1 semantic purpose
The expression `off + base` computes a value the function requires (the
relocated table pointer); the VALUE is fully semantic. The ORDER of the two
commutative operands has no runtime effect — which is precisely the case the
2026-08-20 owner ruling sanctions: a single committed operand order in an
associative+commutative expression chosen to match target, under its five
prerequisites (all met — see SANCTIONED-FAMILY-CLAIMS). Not a freestanding
dead construct; annotated at the site.

## T2 human-programmer
`off + base` is an entirely natural spelling — a human writing "relocate the
offset by the base" writes either order with no preference (SOTN E1/E2
exhibits show shipped code with trial-found term orders). The FAKE annotation
discloses that THIS order was chosen for target provenance, per the ruling's
mandatory-annotation prong.

## T3 GCC-internals justification
Yes — the reason the order matters is expand_binop's source-order preservation
for commutative operands (dump-proven, re-verified THIS session from
regenerated dumps: tmp/grind/func_80048530/dumps/text1b.rtl:1983 shows insn 28
= (plus (reg 83 off) (reg 76 base)) in source order; text1b.greg:2095 shows
the same insn allocated (set v1 (plus v0 v1)) = target's addu $v1,$v0,$v1;
narrative in tmp/grind/func_80048530/s5/mechanism-proof.md). Under the general
policy this is the cheat signal; the 2026-08-20 carve-out makes exactly this
construct sanctionable when the mechanism is NAMED and DUMP-PROVEN rather than
enumeration-derived (prerequisite 2, satisfied). The justification is
disclosed in the annotation, not hidden.

## T4 permuter/search provenance
The order was derived STRUCTURALLY in s3 (hand analysis: objdump residual
`addu $v1,$v1,$v0` vs target `addu $v1,$v0,$v1`; commutative canonicalization
reasoning; six-spelling sweep proving only the off-first source order reaches
the byte with the v1-walker routing). The s4 permuter campaign (13/13 zeros =
this same swap) CORROBORATES the mechanism-derived order — per prerequisite 2
a permuter find may corroborate, never substitute; it does not substitute
here. Additionally the order has verbatim provenance: it is the target's own
operand order (cc1psx also emits base-first from base-first source, s3, so
the ORIGINAL source provably spelled it off-first).

## T5 family check
The construct is the or-tree-shape-shift family — historically forbidden, now
carrying the 2026-08-20 owner carve-out ("single justified target-matching
order") under which this function was explicitly unparked (rule file line 195
names func_80048530; ruling commit a5a5c1ec; unpark commit 04a187a7). All
five prerequisites verified below. No other forbidden family is touched; the
diff net-REMOVES cheat constructs (pins/__asm__/barriers).

## T6 naming-announces-intent
All names (`base`, `off`, `count`, `entry`, `a`-`d`) describe semantic roles.
No pad/dummy/spill naming. The FAKE annotation announces the operand-order
provenance as MANDATED by the carve-out (prong 3).

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: or-tree-shape-shift — single justified target-matching operand order (owner ruling 2026-08-20 carve-out)
  SCOPE: "A SINGLE committed operand order or parenthesization in an associative+commutative expression (`|`, `&`, `^`, `+`), chosen to match target, is sanctioned when ALL of the following hold:"
  PRECEDENT: .claude/rules/or-tree-shape-shift.md:166
  PRECEDENT: a5a5c1ec
  Prerequisite compliance:
    1. Honest orderings measured dead first — ledger s1-s4
       (memory/grind/func_80048530/hypotheses.md): natural `base + off`
       = floor 1 (base-first byte); every non-swap off-first spelling
       misroutes the walker (22/20/12, rejected/offfirst-structural-
       misroutes.c); permuter axis dead (s2 10.6k iters no zero from
       floor-10; s4 13/13 zeros all = this swap); canonical-asm refused
       (scan_hand_coded LOW 1/8); fork-vs-cc1psx settled (cc1psx also
       base-first from natural order).
    2. Named, dump-proven mechanism — expand_binop source-order
       preservation for commutative equal-precedence pseudos; no
       downstream pass (combine/greg/sched) swaps; re-proven THIS
       session from regenerated dumps at insn 28
       (tmp/grind/func_80048530/dumps/text1b.rtl:1983,
       text1b.greg:2095; tmp/grind/func_80048530/s5/mechanism-proof.md).
       Order is mechanism- and provenance-derived, not
       enumeration-derived.
    3. Mandatory E1-style annotation — present at the site in
       src/text1b.c (see ANNOTATION-CONFORMANCE).
    4. Identical runtime semantics (commutative +) and instruction-count
       neutrality — sandbox this session: build_insns 47 == target_insns
       47, score 0, rules_dropped 5.
    5. Layer-1 + layer-2 adversarial review — proceeds via the pipeline
       (this self-vet precedes layer-1; the Judge follows).

ANNOTATION-CONFORMANCE: the following /* FAKE */ block is present in
src/text1b.c immediately above the construct, carrying what + mechanism +
lever-exhaustion:
  /* FAKE: operand order chosen to match target (off + base, not base + off);
   * mechanism: RTL expansion's commutative-operand canonicalization
   * (expand_binop) keeps two equal-precedence pseudos in source order, and
   * no later pass (combine/sched) reorders the addu operands — so only the
   * off-first source spelling emits target's `addu $v1,$v0,$v1`;
   * lever-exhaustion: memory/grind/func_80048530/ s1-s4 — every natural
   * ordering and every non-swap off-first spelling measured dead
   * (natural base+off = 1 insn off; off+=base / mem-inline / fresh-walker
   * misroute the walker, scores 22/20/12; cc1psx also emits base-first from
   * the natural order); sanctioned by the 2026-08-20 owner ruling in
   * .claude/rules/or-tree-shape-shift.md (single justified target-matching
   * operand order). */
