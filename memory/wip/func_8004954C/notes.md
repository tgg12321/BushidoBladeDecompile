# func_8004954C WIP — triangular-sum leaf in text1b.c

## TL;DR (2026-06-13 s1)

14-insn pure-C target; canonical gate verdict C; clean for-loop form
sits at **sandbox-disable-all 6** (HEAD floor was 9, carrying register-asm
pins + a `volatile pad` + a `(void) var_a3;` no-op — all cheats per
[[inline-asm-policy]]). Build_insns now match target (14 vs 14); the 6
remaining diffs are a pure register-allocation swap (`sum`↔`i` between
$v1 and $a3 across 6 references). All other structure — 8-byte frame,
delay-slot fills, opcode sequence — matches.

The lever that flips the RA priority (parenthesisation
`(sum + arg2) - arg1`) is forbidden as operand-axis reassociation
(sibling of [[or-tree-shape-shift]]); see `rejected/paren_reassoc.c`.

## Resume in one read

1. `cp memory/wip/func_8004954C/candidate.c → src/text1b.c` (function body
   at line ~837; just replace the existing `s32 func_8004954C(...)` block).
2. Confirm with `& tools/eng.ps1 sandbox func_8004954C --disable all` →
   score 6 / build_insns 14 / target_insns 14.
3. Pick a next-hypothesis from `meta.json` and try it. **Do NOT re-derive
   the rejected forms in `meta.json.rejected_forms`** (parenthesisation,
   two-statement split, register-asm pins, `volatile pad`,
   declaration-order swaps, register-hint keyword, block-local i,
   arg0-local-alias, commutative reorder). Each is a verified dead-end.

## What's the gap

Target ASM:
```
addiu sp,-8
addu  a3, $0, $0       ; i=0      (a3 = COUNTER)
blez  a1, end
 addu v1, $0, $0       ; sum=0    (v1 = SUM, delay slot)
LOOP:
addu  v1, v1, a0       ; sum += arg0
addiu a3, a3, 1        ; i++
slt   v0, a3, a1       ; i < arg1
bnez  v0, LOOP
 addiu a0, a0, -1      ; arg0-- (delay slot)
end:
subu  v0, a2, a1       ; v0 = arg2 - arg1
addu  v0, v1, v0       ; v0 = sum + v0
addiu sp, +8
jr    ra
 nop
```

Candidate's ASM (clean for-loop): IDENTICAL opcode order, IDENTICAL
instruction count (14), IDENTICAL frame and delay slots — but `sum` lives
in $a3 and `i` lives in $v1 (target has the opposite). Six register-token
diffs.

## Why the RA flips (the GCC priority calculation)

The greg dump (tmp/iso_8004954C.c.greg, regenerated each session) shows:

  v02 baseline (candidate):  `;; 6 regs to allocate: 72 76 75 73 80 74`
  v26 paren-reassoc lever:   `;; 6 regs to allocate: 72 75 76 73 80 74`

Pseudo 75 = sum (declared first), pseudo 76 = i. In v02, i (76) has
HIGHER priority than sum (75) — allocated first → takes lowest free reg
($v1=3); sum picks next → $a3=7. In v26 the reverse — sum is allocated
first and takes $v1.

GCC's `allocno_compare` priority ~ `(log2(n_refs) * freq * class_size) /
live_length`. With `return sum + (arg2 - arg1)`, sum lives until the last
epilogue insn — long live-length → low priority. With
`return (sum + arg2) - arg1`, sum dies at the first epilogue insn —
short live-length → high priority → allocated first.

The natural pure-C lever to invert this would be to either shorten sum's
live range (forbidden — that's the reassoc cheat above) or extend i's
live range without using `(void)i;`. No semantic-purpose extension was
found this session.

## Ruled out (do not re-derive — see meta.json.rejected_forms)

- Parenthesisation `(sum + arg2) - arg1` — cheat, [[or-tree-shape-shift]] sibling
- Two-statement split `sum += arg2; return sum - arg1;` — same family
- `volatile unsigned int pad;` — dead-vars-local-array cheat
- Register-asm pins on $7/$3 — cheat-asm; DIDN'T even close (HEAD's floor 9)
- Declaration-order swaps (sum-first vs i-first) — no effect
- `register` hint keyword (sum, i, or both) — no effect
- Block-local `i` in nested `{}` — no effect
- `s32 a = arg0; ... sum += a; a--;` local alias for arg0 — no effect
- `(arg2 - arg1) + sum` commutative reorder — same RA as `sum + (arg2 - arg1)`
- `(i - i)` term added to return — folded; no IR-level extension
- `for` vs `while` vs `if + do-while` — `for` is the only form emitting
  the 8-byte frame and the 14-insn structure; the others stop at 11

## Next session

Permuter-directed (`PERM_*` macros) starting from `candidate.c` is the
highest-EV next step. The shape of the gap (single-axis RA swap, 6
identical reg-rename token diffs, all structure-correct) is exactly the
small-search-space the permuter eats. See `meta.json.next_hypotheses`
for the rest.
