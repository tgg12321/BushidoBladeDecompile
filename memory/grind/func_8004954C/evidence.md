# Evidence bank — func_8004954C

## s1 (2026-07-22, recon) — floor reconfirmed + mechanism nailed
- **Floor = 6 confirmed** with clean for-loop candidate applied to src (sandbox
  --disable all: score 6, build_insns 14, target_insns 14, 0 rules, 375 cheat-asm
  stripped from the FILE but none in this func). HEAD's src had cheats (pins +
  volatile pad + (void) cast); replaced with the clean candidate.c form.
- **No sibling/duplicate analog** — tools/find_duplicates.py present; no
  8004954C entry in tmp/duplicates_leads.txt. This is a lone triangular-sum leaf.
- **m2c reference shape == candidate exactly**: var_v1=sum (accumulator, +=arg0),
  var_a3=i (counter), `return var_v1 + (arg2 - arg1)`; do{}while(a3<a1) guarded by
  if(a1>0). Structurally identical to candidate.c — the ONLY gap is RA.
- **Fresh greg (artifact func_8004954C.greg) reproduces prior v02 exactly:**
  `;; 6 regs to allocate: 72 76 75 73 80 74`; dispositions `75 in 7  76 in 3`
  → sum(pseudo75)→$a3, i(pseudo76)→$v1. Target is the inverse (sum→$v1, i→$a3).
- **Mechanism of the wall (measured, two-axis):** allocno_compare priority ≈
  floor_log2(n_refs)·freq·size / live_length. Counting from target asm: i has
  **3 loop refs** (`addiu a3,1`=use+set, `slt a3,a1`=use) vs sum's **2**
  (`addu v1,v1,a0`=use+set); AND i dies at the loop while sum lives to the
  return `addu v0,v1,v0`. So i wins on BOTH the freq (more loop refs) AND the
  live_length (shorter) axes → i(76) sorts ahead of sum(75) in the alloc order →
  i grabs the lower reg $v1(3). This is NOT a tie (if it were, allocno-number
  tiebreak would pick sum75 first) — i has strictly higher priority.
- **Why trivial restructures can't remove i's 3rd ref:** the byte target
  `slt $v0,$a3,$a1` forces an UP-counter compared to arg1 (arg1 must stay live
  in $a1 for the final `subu v0,a2,a1`). That mandates i's separate test ref.
  Count-down / `i-- > 0` / `arg2 - i` forms all break the `slt vs a1` bytes.
- **Not owner-gated:** the permuter axis is UNTRIED (prior session named it
  highest-EV, never ran it). Floor is grindable.

- WIP rejected_form: {'form': '`(sum + arg2) - arg1` parenthesisation (v26)', 'score': 2, 'reason': "sibling family to or-tree-shape-shift (FORBIDDEN: parenthesization-axis mutations in associative/commutative expressions). Score reaches 2 because the parens flip sum's live range — but the change is pure codegen-control reassociation: a programmer would naturally write `sum + (arg2 - arg1)` for this 'sum + offset' semantic. The mutation only exists to change GCC's allocno priority — a cheat by spelling per no-new-park-categories."}

- WIP rejected_form: {'form': '`sum = sum + arg2; return sum - arg1;` two-statement split (v20)', 'score': 2, 'reason': 'same family as v26 — splitting the return into two statements is statement-axis reassociation. Same codegen-control intent. FORBIDDEN.'}

- WIP rejected_form: {'form': '`volatile unsigned int pad;` (HEAD source carried this)', 'score': 'did not close (HEAD floor 9 with the pad active)', 'reason': 'dead-vars-local-array — unused fixed-size local declared only to force GCC to reserve frame bytes. Listed as cheat-spelling in inline-asm-policy.md expanded cheat catalog (2026-05-31). The HEAD source also carried register-asm pins on $7/$3 and a `(void) var_a3;` no-op extending live range — all source-level cheats.'}

- WIP rejected_form: {'form': '`register s32 sum/i asm(...)` pins (v01-style)', 'score': 9, 'reason': "register-asm pins are cheat-asm per inline-asm-policy. They also DID NOT close — HEAD's score with pins was 9, worse than the clean-C floor of 6. Pins do not bind reliably here."}

- WIP rejected_form: {'form': 'declaration-order swaps, `register` hint keyword, block-local i, register copy of arg0, `(arg2 - arg1) + sum` (commutative reorder)', 'score': 6, 'reason': "no effect on GCC's pseudo-priority computation; all stayed at the same 6-reg-swap floor. Documented as ruled-out so the next agent doesn't re-derive."}

- == imported from memory/wip notes.md ==
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


- [s1] sandbox --disable all = score 6, build_insns 14, target_insns 14, rules_dropped 0 (clean candidate applied to src/text1b.c:837).

- [s1] canonical verdict = C, distance 9 (<=50 pure-C target).

- [s1] m2c(asm/funcs/func_8004954C.s) reconstructs the candidate exactly: sum accumulator (var_v1), i counter (var_a3), return var_v1 + (arg2 - arg1), if(arg1>0){do{...}while(a3<a1)}.

- [s1] greg alloc order 72 76 75 73 80 74; dispositions sum(75)->$a3(7), i(76)->$v1(3) — inverse of target (sum->$v1, i->$a3).

- [s1] Two-axis priority: i has 3 loop refs vs sum 2, AND i.live_length < sum.live_length (i dies at loop, sum lives to return). i strictly outranks sum; not a tie.

- [s1] No sibling/duplicate analog: tools/find_duplicates.py present, no 8004954C entry in tmp/duplicates_leads.txt.

- [s1] HEAD src carried cheats (register asm($7/$3) pins + volatile pad + (void)var_a3) at floor 9; replaced with clean floor-6 candidate.

- [s1] Permuter axis is UNTRIED (prior session named it highest-EV, never launched) — floor is grindable, not owner-gated.

## s2 (2026-07-22, structural) — structural axis measured DEAD; floor 6 holds
- **Floor 6 reconfirmed** with clean candidate applied (score 6, build 14, target 14).
- **The RA-flip mechanism is precisely quantified (greg + isolated-TU dumps in
  tmp/grind/func_8004954C/s2/greg/):** priority formula (tools/gcc-2.7.2/global.c:604
  allocno_compare) = `flog2(nrefs)*nrefs/live_length * 10000 * size`. Baseline iso
  dispositions: sum(75)->$a3(7), i(76)->$v1(3) WRONG, subu at END. To flip, sum must
  outrank i; the ONLY structural way is to SHORTEN sum's live range — which is the
  forbidden reassoc family.
- **off-early form (`s32 off = arg2 - arg1;` hoisted before loop; `return sum + off`)
  reaches score 4** — RA becomes CORRECT (iso dump: sum(76)->$v1, i(77)->$a3, off(75)
  ->$a2). Mechanism: single final `addu v0,sum,off` (vs baseline `subu;addu`) shortens
  sum's live range by one insn -> sum's priority rises above i's. Residual 4 = pure
  delay-slot scheduling (subu carried in $a2 lands in the blez delay slot; target
  computes subu post-loop with sum=0 in the delay slot).
- **off-early REJECTED by cheat-reviewer (s2, FAIL):** same species as paren_reassoc —
  cross-loop live-range steering of sum vs i, re-spelled as a named local. Decisive
  tell: my own `off_temp_late` counter-experiment (offset computed near natural use at
  END) reverts to floor 6 — no programmer-natural placement yields the RA benefit, so
  it is codegen-motivated live-range manipulation, not the SOTN named-intermediate
  mechanism (SOTN E4 names adjacent to use; this relocates across the loop). Saved to
  rejected/off_early_livrange_steer.c.
- **Structural sweep (25 forms, tmp/grind/func_8004954C/s2/sweep*.py) — ALL subu-at-end
  forms stay at 6:** cmp-operand-swap (`arg1 > i`), do-while, postdec (`sum+=arg0--`),
  sum-explicit (`sum = sum + arg0`), pre-increment, all declaration/statement orders of
  {off,sum,i}, off-split, off-late-assign, bound-var (`s32 bound=arg1`), off commuted
  (`off+sum`). NONE flips the RA with subu-at-end.
- **Type-narrowing measured dead:** i as s16 -> score 5 but build_insns 17 (extra
  sign-extends, worse structure); i as u32 -> score 7 (worse). Not viable.
- **Count-down measured dead (confirms s1 reasoning):** separate `n=arg1; while(n>0)`
  -> score 9 build 10; `for(i=arg1;i!=0;i--)` -> score 10 build 10. Break the 14-insn
  structure and the slt-vs-a1 bytes.
- **`sum = arg2 - arg1` seed (accumulate into offset-seeded sum) -> score 6 build 13:**
  collapses the frame, worse structure. Dead.
- **Conclusion:** the structural axis (block-local splits, decl/stmt order, type
  narrowing, re-association) is measured DEAD for reaching below 6 legitimately. The
  only structural form that lowers the score (off-early, 4) is a live-range-steering
  cheat. The floor-6 RA wall is not breakable by legitimate structural C — sum cannot
  be made to outrank i without shortening sum's live range (forbidden). Remaining
  live axis: F1 permuter (different modality, still UNTRIED) may find a byte-neutral
  mutation; that is the next non-structural avenue. NOT owner-gated (permuter untried).

- [s2] Floor 6 reconfirmed (clean candidate: score 6, build_insns 14, target_insns 14, 0 rules).

- [s2] allocno priority formula confirmed from tools/gcc-2.7.2/global.c:604 allocno_compare = (flog2(n_refs)*n_refs / live_length) * 10000 * size; tie broken by lower allocno (pseudo) number.

- [s2] Isolated-TU greg dumps (tmp/grind/func_8004954C/s2/greg/): baseline dispositions sum(75)->$a3, i(76)->$v1 (WRONG, subu at END); off-early sum(76)->$v1, i(77)->$a3, off(75)->$a2 (CORRECT RA, subu in blez delay slot).

- [s2] The RA flip requires shortening sum's live range, which is exactly the forbidden reassoc family (or-tree-shape-shift). Every legitimate structural form leaves i strictly outranking sum -> floor 6.

- [s2] off-early (score 4) cheat-reviewer verdict FAIL: cross-loop live-range steering, sibling of rejected paren_reassoc; saved to rejected/off_early_livrange_steer.c.

- [s2] 25 structural variants swept total (sweep.py/sweep2.py/sweep3.py); only the live-range-steering off-early family drops below 6, and it is a cheat.

- [s2] Structural modality is exhausted for this function; the floor-6 RA wall is not breakable by legitimate structural C. Next live avenue is the F1 permuter campaign (different modality, still UNTRIED) — floor remains grindable, not owner-gated.
