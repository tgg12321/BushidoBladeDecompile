# Hypothesis ledger — func_80049718

## s1 (recon) — floor 11 -> 0

Residual at floor 6 mapped to exactly two real clusters (all other objdump
diffs are unrelocated %hi/%lo placeholders in the standalone sandbox .o):

- **Cluster B (store schedule, KILLED->fixed, 6->4):** block-2 `if(var_s3!=1)`
  emitted `sh v0,4(=6)` before the two zero stores; target emits it after.
  Fix = reorder source stores (obj+6, obj+8, obj+0xA, obj+4). Pure C.
- **Cluster A (prologue addr comp, KILLED->fixed, 4->0):** `&D_800EF980[arg0]`
  — our fork's sched1 hoists the 1-insn `sll` (index) above the 2-insn base
  chain (lui/addiu), so index->$v0, base->$v1, `addu s0,v0,v1`. Target starts
  the base chain first (critical path: s0 feeds the `lh`), base->$v0,
  index->$v1, `addu s0,v1,v0`. Fix = stage base into a named local
  `s16 *tbl = D_800EF980; p_anim = tbl + arg0;` -> base pseudo born first.
  Bare `D_800EF980 + arg0` (no local) = floor 11 (rejected/pointer-add.c).

Result: clean floor 4 (changes 1+2 pass layer-1 cheat-reviewer). A 4->0 close
exists via named base-pointer alias but cheat-reviewer FAILed it (pointer-alias
family; needs lever-exhaustion + /* FAKE */). src left at clean floor 4.

RTL mechanism for Cluster A (text1b.i.greg:7783-7800): fold canonicalizes the
address PLUS so symbol_ref(base) is operand-2 -> INDEX ashift insn emitted first
(insn18 -> $v0), BASE symbol_ref second (insn20 -> $v1) -> `addu s0,v0,v1`.
Target: base first ($v0), index second ($v1), `addu s0,v1,v0`.

NEXT (drill): (1) register-alloc-pure-c A/B/C on the prologue addr comp;
(2) directed permuter from the floor-4 candidate targeting the addu $s0 operand
order; (3) if pure-C exhausted, legitimize the alias with /* FAKE */ + the
mechanism above + the exhaustion ledger, per pointer-alias-fake-exception.

## [s1] The floor-6 residual is exactly two real clusters; all other objdump diffs are unrelocated %hi/%lo placeholders in the standalone sandbox .o.
- mechanism: Relocation-normalized diff of asm/funcs/func_80049718.s vs the sandbox text1b.o shows real diffs only at the prologue address computation (pos 7-11) and the block-2 store schedule (pos 173-175).
- probe: tmp/grind/func_80049718/s1/normdiff.py over mine_objdump.txt
- result: Confirmed two clusters (A prologue, B store schedule); everything else is relocation noise.
- verdict: CONFIRMED

## [s1] Cluster B: target schedules `*(s16*)(obj+4)=6` after the two zero stores (obj+8, obj+0xA); our fork emits it first. A pure source-statement reorder fixes it.
- mechanism: Reordering the four independent halfword stores to distinct offsets in the `if(var_s3!=1)` block so obj+4=6 follows obj+8/obj+0xA matches target's late `sh v0,4(s2)`. Behavior-identical; cheat-reviewer PASS.
- probe: Reorder stores; sandbox func_80049718 --disable all
- result: Floor 6 -> 4.
- verdict: CONFIRMED

## [s1] Cluster A: prologue `&D_800EF980[arg0]` has base/index in swapped registers because `fold` canonicalizes the address PLUS with the symbol_ref as operand-2, so the index (ashift) insn is emitted before the base (symbol_ref) insn and local-alloc gives index->$v0, base->$v1.
- mechanism: RTL .greg dump insn18 `(reg v0)=ashift s6,1` precedes insn20 `(reg v1)=symbol_ref D_800EF980`, yielding `addu s0,v0,v1`. Target emits base first (base->$v0, index->$v1), `addu s0,v1,v0` — s0 feeds the next `lh` (critical path).
- probe: cc1 -O2 -G0 ... -da text1b.i; read text1b.i.greg:7783-7800
- result: Mechanism confirmed. Simple statement reorder (var_s3<->p_anim) does NOT change it (still 4).
- verdict: CONFIRMED

## [s1] A named base-pointer intermediate (`s16 *tbl = D_800EF980; p_anim = tbl + arg0;`) closes Cluster A to distance 0, but is a policy-gated pointer-alias-family construct, not a clean pure-C close.
- mechanism: A live pointer register keeps the base as PLUS operand-1 (not demoted by fold), so the base insn is emitted first -> base->$v0. But `tbl` is behaviorally identical to `&D_800EF980[arg0]`, used once, no observable effect beyond steering RA.
- probe: Apply alias; sandbox=0; relocation-normalized byte diff (clean); layer-1 cheat-reviewer
- result: sandbox 0 (genuine, not masked) BUT cheat-reviewer FAIL: pointer-alias-fake-exception family; requires documented lever-exhaustion + /* FAKE */ annotation (neither present). Reverted from src.
- verdict: KILLED
