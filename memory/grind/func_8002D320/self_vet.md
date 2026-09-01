# SELF-VET — func_8002D320

(Rewritten this session, 2026-08-31, against the applied diff at
src/code6cac_b.c:860 — the Judge-passed ret-form tail from the 2026-08-31
ESCALATE packet (docs/grind/decisions.md:16948). The construct layer-1 FAILed
on 2026-08-31 21:06 — a hardcoded constant routed through a dead borrowed
local into the function's exit value, banked at rejected/layer1-fail-0831-2106.c
— is ABSENT from this diff in every spelling, and the family citation the
Judge ruled misfiled for that tail is not claimed anywhere below.)

CONSTRUCTS: GTE vector-load island (__asm__ volatile: addu-$t4 preamble + lwc2 $0/$1 + 2 nops + mvmva .word 0x4A486012, "r"(vin), "$12"+"memory" clobbers), GTE vector-store island (__asm__ volatile: addu-$t4 preamble + swc2 $25/$26/$27, "r"(vout), "$12"+"memory" clobbers), GTE LZCS island (__asm__ volatile: addu-$t4 + mtc2 $t4,$30 + 2 nops + addu $t4,$sp + swc2 $31, "=m"(sp_var), "r"(x), "$12" clobber), FAKE dead store `ret = 1;` inside the zero arm of the tail if/else (dead-store-fake-exception), sequential reuse of local `x` (rotated X -> squared distance -> discriminant -> sqrt), dedicated result local `ret` set live in both arms and read once at the function exit

## T1 semantic purpose
- The three cop2 islands: real computation with NO C analog (GTE mvmva vector
  transform; LZCS/LZCR leading-zero count feeding the sqrt table walk). The
  bytes they emit ARE the function's behavior; splat tags the target's
  preamble/cop2 instructions "handwritten instruction". Covered by the
  EXECUTED canonical-asm grant (inline_asm_canonical.txt:366).
- `ret = 1;` in the zero arm: NO semantic purpose — it is dead (overwritten by
  `ret = 0;` before any read). That is exactly why it is claimed under the
  dead-store-fake-exception FAKE family with the mandatory annotation, not
  passed off as natural C. Per that rule's model, T1-failure is the family's
  defining property; the rule's prerequisites (exhaustion, named mechanism,
  annotation) substitute for semantic purpose.
- `ret` itself (the variable): real — set live in both arms, read at the
  function exit. It is a dedicated result variable, not a borrowed local, and
  no constant is smuggled through a dead value into the exit — both arms'
  live stores are the values the function actually yields.
- `x` reuse chain: every write is a real value read downstream. No other dead
  code anywhere in the body.

## T2 human-programmer
- Islands: yes — a 1998 PSX programmer hand-wrote exactly these (the target's
  redundant $t4 copy + unfilled cop2 delay nops are shapes GCC never emits).
- `ret = 1; ret = 0;`: a reader would ask why — that is the FAKE smell, which
  is why it carries the mandatory annotation and a sanctioned-family claim.
- Everything else (early rejects, min/max Y envelope, result variable,
  one-variable computation chain): natural hand-written C.

## T3 GCC-internals justification
- Islands: justified by program semantics (cop2 ops), not by any GCC pass.
- The dead store: yes — its justification IS jump.c's store-flag if-conversion
  single-set precondition. That is precisely the sanctioned-FAKE-family
  situation: the mechanism is named, the exhaustion is documented, and the
  annotation is present, per the rule's own prerequisites.
- No other construct's presence is GCC-motivated.

## T4 permuter/search provenance
No permuter or auto-search produced any part of this body. The tail shape is
the documented confirmed closure from .claude/rules/dead-store-fake-exception.md
(func_80078EC0), selected by the Judge's own 2026-08-31 packet; the rest was
hand-derived from the target asm and the twin func_8002EA24's ledger. Measured
in the cheat-invisible sandbox: score 0, 120/120, re-proven 2026-08-31 THIS
session (tmp/grind/func_8002D320/s1/sandbox_0.json).

## T5 family check
- Islands: canonical inline asm (GTE/cop2) — the ALLOWED category of
  inline-asm-policy, in the exact narrow spelling the 2026-07-30 twin Judge
  ruling demanded (address computed in C, bound as "r" operand; template =
  $t4 addressing preamble + cop2 ops only; "$12" clobber; no pins, no
  barriers, no hardcoded-$N operand templates). Cluster membership row:
  cop2-addressing-preamble-cluster.md:73. Grant EXECUTED on the OWNER-CLUSTER
  door (inline_asm_canonical.txt:366).
- The tail: dead-store-fake-exception, the exact func_80078EC0 confirmed-
  closure shape (dead `ret = 1;` INSIDE the arm; two-set arm). No live local
  is borrowed, no constant travels through a dead variable into the exit
  value — `ret` is a dedicated result variable whose live store in EACH arm
  is the value the exit reads. The construct this function's ban list and
  layer-1 FAIL describe (a borrowed dead local carrying a hardcoded constant
  to the exit) is absent, and the family citation the Judge ruled misfiled for
  that construct is not claimed.
- No forbidden family is touched: zero register pins, zero scheduling
  barriers, zero hardcoded-$N operand templates, zero volatile coercion, zero
  dead arrays, zero alias renames, no do-while(0), no build-time rewriting.

## T6 naming-announces-intent
No pad/dummy/unused/spill/slack names. `ret` is the function's real result
value (both arms live into the single exit read). `sp_var` is the LZCR output
slot the island genuinely writes and the C genuinely reads (`lzcr = sp_var;`)
— same name and role as in the authorized siblings func_800274BC/func_8002D518.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: dead-store-fake-exception
  SCOPE: "Ordinary-C assignment statements inside a function body whose stored value is never read (GCC DCEs the store; its existence influences RA / scheduling / flow analysis upstream of DCE)"
  PRECEDENT: .claude/rules/dead-store-fake-exception.md:121

  FAMILY: canonical cop2 inline asm — addu-$t4 addressing-preamble cluster (owner ruling 2026-08-17; grant executed via pipeline 2026-08-31)
  SCOPE: "the 26 queued functions sharing the `addu $t4,$aN,$zero` + cop2 idiom (28 total in the 0x8001-0x8003 band) inherit this disposition subject to the same mechanical per-function check ... which the Judge may apply without re-escalation."
  PRECEDENT: .claude/rules/cop2-addressing-preamble-cluster.md:73

ANNOTATION-CONFORMANCE: the dead store carries, in src/code6cac_b.c directly on the statement:
  /* FAKE: dead store -- overwritten by `ret = 0;` on the
   * next statement, never read.  Mechanism: jump.c's
   * store-flag if-conversion requires SINGLE-SET 0/1 arms;
   * the two-set arm keeps target's unfolded diamond (bnez;
   * move v0,zero delay; addiu v0,1) instead of folding the
   * pair to `slt` + `xori v0,v0,1`.  Family:
   * dead-store-fake-exception (confirmed closure
   * func_80078EC0, .claude/rules/dead-store-fake-exception.md:107-128).
   * Lever-exhaustion: memory/grind/func_8002D320/hypotheses.md
   * sessions 1-2 (five pure-C tail shapes measured: plain
   * early-return 3/118, result-carrier nest 4/119,
   * goto-reject 3/118, inverted sense 3/118, combined-&&
   * 8/119) + the twin func_8002EA24's six-shape tail census
   * on the identical diamond. */
carrying what (the dead `ret = 1;`) + mechanism (jump.c store-flag single-set
precondition) + lever-exhaustion (hypotheses.md s1-s2 five measured shapes +
twin census). The canonical islands are not FAKE-class and carry no FAKE
annotation (none owed — they are grant-covered canonical asm).
