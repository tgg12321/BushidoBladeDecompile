# Evidence bank — func_80060544

Function: `func_80060544(s32 arg0, s32 arg1)` in `src/text1b.c` (currently at line 13114).
Target asm: `asm/funcs/func_80060544.s`, 133 instructions.
Canonical gate: **C** (`asm_insns 0`, "pure-C distance 18 <= 50 — pure-C target").
`diagnose` verdict at session start: LARGE (d29 / 27 differing insn) — that number is
computed against the *rules-enabled* reference and is NOT the honest floor; the honest
cheat-free floor is what `sandbox --disable all` prints.

## Floor history
| session | floor (`sandbox --disable all`) | build_insns | note |
|---|---|---|---|
| s1 start | **18** | 134 | 11 regfix rules dropped by the sandbox, 324 cheat-asm insns stripped file-wide |
| s1 end   | **4**  | 134 | three levers landed; edits in place in `src/text1b.c` |

## What the function does (shape, from target asm + the C)
Two sequential loops that fill a 0x28-byte stack struct (`S544 s` at `sp+0x18`) and hand
`&s` to `func_8007352C` each iteration, chaining the returned value into the next
iteration's `s.arg1_field` (a display-primitive linker: each call returns the next
primitive pointer). Loop 1 runs `i = 0..3` over a 3-word-stride geometry table
(`D_8009B770`, stride 0xC) and selects a per-`i` static table
(`i==0 -> D_8009B7D8`, `i in {1,2} -> D_8009B800`, `i==3 -> D_8009B7D0`); the `i==3` arm
additionally calls `func_80073728(&s, 0)` and skips the `func_8007352C` call. A one-off
`func_8007352C` with `D_8009B7A0` / `D_8009B820` follows, then loop 2 runs `j = 0..1`
over walking pointers `D_8009B3B0` (stride 0xC) and `D_8009B840` (stride 8). The tail is
`initTexPage(arg0+0x5DC, 1, 0, saMotionSet(s.p_geom, 0), 0)` then
`ot_Link(D_800A374C + arg1*4, arg0+0x5DC)`, returning `(arg0+0x5F4) - arg0`.

## The 11 regfix rules the sandbox drops (what they were papering over)
- `reorder 17,18,15,16 @ 15-18` — prologue init-order: target emits `$s5 = 3` BETWEEN the
  `i` and `idx` zero-inits; our literal-3 build emitted it after both. **Closed by the
  `last = 3` lever (H3).**
- 6x `subst la $3 -> $2` / `sw $3 -> $2` on `D_8009B770` / `D_8009B7D8` / `D_8009B800` /
  `D_8009B7D0` plus `addu $2,$17,$3 -> $2` — the $v1-instead-of-$v0 address-pseudo
  cluster. **Closed by the geom split-init (H1) and the shared `stat` local (H2).**
- `delete @ 77` — the extra `move $s5,$s7`. **Still open (H4).**
- `subst sll $4,$21,2 -> sll $4,$23,2` — the $s5/$s7 consequence of the same extra copy.
  **Still open (H4).**

## Measured facts (session s1, 2026-08-03)
1. **The address-materialisation pseudo splits from its consumer when the whole address
   expression is written as ONE expression.** `s.p_geom = (s32 *)((s32)&D_8009B770 + idx);`
   emits `lui $v1 / addiu $v1 / addu $v0,$s1,$v1`; target emits `lui $v0 / addiu $v0 /
   addu $v0,$s1,$v0`. Rewriting as split-init accumulation
   (`geom = (s32)&D_8009B770; geom += idx; s.p_geom = (s32 *)geom;`) makes it one
   self-updating pseudo, which local_alloc puts in $v0. Floor 18 -> 15.
   Sanctioned family: [[split-init-accumulation-sanctioned]].
2. **The same mechanism governs the three per-arm static-table stores.** Three direct
   `s.p_static = &D_800...;` stores in three branch arms produce three separate pseudos,
   all allocated $v1. Routing all three through ONE shared named local
   (`stat = (s32)&D_800...; s.p_static = (s32 *)stat;`) collapses them to one pseudo that
   gets $v0, matching target in all three arms. Floor 15 -> 8. This is a *cheap, general*
   finding: it is lever (1) applied to a multi-arm select, and it is worth trying on any
   function whose regfix cluster is a pure `la $3 -> $2` rename family.
3. **Target's `3` was set in the C source, not hoisted by loop.c.** Target's prologue is
   `addu $s0,$zero,$zero` (i=0) / `addiu $s5,$zero,0x3` / `addu $s1,$zero,$zero` (idx=0).
   A CSE'd loop-invariant constant cannot land there: loop.c emits preheader movables
   immediately before the loop start, i.e. after BOTH inits — which is exactly what the
   literal-3 C produced. Writing `last = 3;` between `i = 0;` and `idx = 0;` and using it
   for the two equality tests (`i == last`, `i != last`; the `i < 3` / `i < 4` tests stay
   `slti` immediates and are unaffected) reproduces target's prologue exactly.
   Floor 8 -> 4. See candidate.c's header for the cheat-family note on `last`.
4. **`new_var` (the `arg1` carrier for the `ot_Link` index) is LOAD-BEARING.** Removing it
   regresses 18->48 from the old base and 4->10 from the new base; hoisting its init to
   the top regresses 8->14; additionally routing `s.arg2_field` through it also gives 14.
   The documented [[drop-param-alias-local]] recipe does NOT apply to this function.
   Details + exact edits in `rejected/`.

## Artifacts
- `tmp/grind/func_80060544/s1/pairs.py` — prints the instruction-level diff between the
  cheat-invisible sandbox object (`tmp/sandbox/func_80060544/text1b.o`) and the
  byte-matching reference (`build/src/text1b.o`) via `engine.diagnose.diff_pairs`.
  This is the gradient microscope for this function; re-run it after every edit.
- `tmp/grind/func_80060544/s1/pairs_floor4.txt` — the diff at the floor-4 state.
- `tmp/grind/func_80060544/s1/diff.sh` — an earlier objdump-vs-`.s` text differ. It is
  NOT useful (objdump mnemonics vs splat `.s` mnemonics never line up); use pairs.py.

## Reading the pairs.py output
Every `beqz/bnez/j/beq` hunk in the output is ADDRESS NOISE, not a real diff: the sandbox
object strips 324 cheat-asm instructions file-wide, so `func_80060544` sits at a different
offset than in the reference object and every branch/jump literal differs. The engine's
score masks these. Only non-branch hunks are real.

- [s1] Canonical gate: verdict C, asm_insns 0, 133 target instructions - a pure-C target, no canonical-asm question anywhere in this function.

- [s1] Honest cheat-free floor moved 18 -> 4 this session; build_insns 134 vs target 133 throughout (the single extra insn is the `move $s5,$s7` in H4).

- [s1] The 11 regfix rules the sandbox drops decompose exactly into the three mechanisms found: 1 prologue `reorder` (closed by H3), 7 `la`/`sw`/`addu` $v1->$v0 substs (closed by H1+H2), and `delete @ 77` + the `sll $4,$21,2 -> $4,$23,2` subst (the still-open H4 pair).

- [s1] `engine diagnose` reports LARGE / d29 / 27 differing insns for this function - that figure is measured against the rules-enabled reference and is NOT the honest floor. Do not use it as a gradient; `sandbox --disable all` is the gradient.

- [s1] The gradient microscope for this function is tmp/grind/func_80060544/s1/pairs.py (engine.diagnose.diff_pairs between tmp/sandbox/func_80060544/text1b.o and build/src/text1b.o). Every branch/jump hunk it prints is address noise - the sandbox strips 324 cheat-asm insns file-wide, so the function sits at a different offset in the two objects. Only non-branch hunks are real.

- [s1] A plain objdump-vs-splat-`.s` text diff (tmp/grind/func_80060544/s1/diff.sh) is useless here: objdump mnemonics (`move a1,zero`) never line up with the splat `.s` spelling (`addu $a1, $zero, $zero`). Do not rebuild that wheel.

- [s1] Generalisable finding: a regfix cluster consisting purely of `subst "la $3,SYM" "la $2,SYM"` renames is a signature for the split-address-pseudo problem, and both fixes (split-init accumulation, one shared named carrier across branch arms) are sanctioned pure-C forms. Worth sweeping the rule corpus for that shape.

- [s1] Judge note on the surviving `last = 3;`: it is a LIVE, twice-read local (not a dead store), and reads naturally as `index of the last/special-cased element`, but it is a constant-holder and therefore sits in the [[named-local-fake-exception]] family. It is deliberately un-annotated pending a ruling; if the Judge wants the /* FAKE */ annotation plus documented lever exhaustion, that is a one-comment fix.
