# Hypothesis ledger — rob_life_ctrl_2

## [s1] HEAD body with regfix rules stripped scores 2 at the honest sandbox --disable all metric (nothing has drifted vs ledger s0 floor=2)
- mechanism: sandbox --disable all: rules_dropped=2, cheat_asm_stripped=78, build_insns=96, target_insns=96, score=2. canonical: verdict=C, distance=2 (pure-C target).
- probe: & tools/wteng.ps1 main sandbox rob_life_ctrl_2 --disable all; & tools/wteng.ps1 main canonical rob_life_ctrl_2
- result: score=2; verdict=C; the 2 stripped rules are the rob_life_ctrl_2 mflo/sra substs at regfix.txt:487-488
- verdict: CONFIRMED

## [s1] The exact residual is the t-mult mflo destination and its dependent sra source; target reuses a1(=4096-constant) for t's mflo after a1's last subu use at 0xB0 dies, our build assigns pseudo 104 to $8/t0 instead
- mechanism: objdump of tmp/sandbox/rob_life_ctrl_2/text1a_c.o vs asm/funcs/rob_life_ctrl_2.s: at 0xC4 target `mflo a1` / ours `mflo t0`; at 0xCC target `sra a1,a1,0xc` / ours `sra a1,t0,0xc`. All other 94 insns match byte-for-byte. Precisely matches the ledger's documented gap.
- probe: mipsel-linux-gnu-objdump -d tmp/sandbox/rob_life_ctrl_2/text1a_c.o | sed -n '/<rob_life_ctrl_2>:/,/^$/p'; compare against asm/funcs/rob_life_ctrl_2.s
- result: diff = { 0xC4: mflo $8 -> $5, 0xCC: sra $5,$8,12 -> sra $5,$5,12 } = 2 insns; 94/96 already byte-identical
- verdict: CONFIRMED
