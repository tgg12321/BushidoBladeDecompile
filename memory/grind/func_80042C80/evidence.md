# Evidence bank - func_80042C80

## s1 (recon, 2026-09-08) - MATCHED at distance 0, oracle ok
- canonical: verdict C, hand_coded_tier LOW, target 122 insns. Ordinary pure-C target.
- OBJECT MODEL: Judge - MATCHES. TU-local `extern s16 Judge[];` (src/text1a_c.c, same
  declaration the matched siblings func_80042874 / func_80042F10 use); no header row, no
  address row, but the s16 element type is proven by the distance-0 body (lh/lhu + sll/sra
  shapes and the `sh` of the raw element are all consistent with a 2-byte signed table). No
  declaration fix needed; no other global is touched (a0/a1 are pointer params).
- Chassis at dispatch: retired-chassis body (u16 rawA stage replacing the retired volatile cast)
  scored 53, frame 56 (vars=48, regs=2); only cosA had the target's lhu+sll+sra shape, the other
  three (sinA/sinB/sinC) folded to lh. -> rejected/rawA-staging-cosA-only-frame56-score53.c
- Target load shape (asm/funcs/func_80042C80.s): cosB (0x334C4) and cosC (0x334D0) are `lh`;
  sinB, sinA, sinC, cosA are `lhu` + `sll 16` + `sra 16`. sinB's raw lhu value ($t3) is stored
  to a1[2] (`sh $t3,4($a1)` at 0x3361C) at the tail; sinB's extension is a separate pseudo.
- Frame gradient via cc1 `.frame` line (tmp/grind/func_80042C80/s1/probe.py, phantom-frame-slots
  instrument): target = `$sp,48` = vars 40 + regs 2/0. Declaring sinA/sinB/sinC/cosA as s16 and
  cosB/cosC as s32 gives vars=40 exactly (5 narrow locals -> 48, 4 -> 40). s16 cosB/cosC also
  gives vars=40 (f4 probe: identical frame and load counts) - not measured in the sandbox.
- Mechanism (measured f1 vs f3, both vars=40, lh 2 / lhu 7 / sll16 7 in cc1 output): a s16
  local whose first arithmetic use (the sign_extend emitted at expand) lies AFTER the
  `a1[2] = sinB;` store cannot have its lhu folded into the extend, because combine.c:914-917
  rejects a MEM src when use_crosses_set_p sees an intervening memory store -> lhu; sll; sra.
  cosB/cosC as s32 extend at the load itself -> lh. This is the same mechanism the sibling
  func_80042874 ledger banked for its rawA stage.
- Store placement matters for sched1, not for combine: with the store BEFORE the cosB/cosC loads
  (f1) sandbox = 83, because the Judge loads may-alias a1[] and the scheduler keeps them below the
  store; with the store AFTER `cosB_cosC = cosB * cosC;` (f3, all six loads above it) sched1 sinks
  the store to the tail as in the target and sandbox = 0. -> rejected/store-before-cosBC-loads-score83.c
- Phantom $s1 save (target saves s1 at 0x2C and never uses it): reproduced by f3 (regs=2/0) and
  f0, absent in f1 (regs=1/0). No `reg 17` appears in the .greg dump of f0, so the save comes from
  reload's spill-register pool marking (reload1.c:2322 `regs_ever_live[spill_regs[..]] = 1`), a
  register-pressure artifact of the body - it needs no construct.
- verify-oracle with the candidate in src/text1a_c.c: ok, build SHA1
  62efab4f73f992798c43e8c730aa43baa10bb4fa == locked oracle.
- Artifacts: tmp/grind/func_80042C80/s1/{f0,f1,f2,f3,f4}.c, probe.py, apply.py,
  pairdiff_f0.txt, pairdiff_f1.txt, f0.s/f1.s/f2.s/f3.s/f4.s (cc1 output),
  tmp/grind/func_80042C80/dumps/ (cc1 -da dumps taken on the f0 body).
