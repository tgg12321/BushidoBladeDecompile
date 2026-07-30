# Hypothesis ledger — func_80033D38

## Session s1 (2026-07-29, recon) — 5 hypotheses, all resolved, floor 17 → 0

### H1 — CONFIRMED. The function is a sorted-record insert over an 8-byte-stride array.
Mechanism: `sll $v0,$v1,3` + `lw 0xC($v0)` + `sb 0x8/0x9` + `sw 0xC` off a single
held base `$t1 = &D_80106A50` describe records `{ u8 x; u8 y; s32 t; }` with `t`
at +4; the middle loop's lw/lw/sw/sw at +0/+4 → +8/+0xC is an 8-byte struct copy
shifting `recs[k+1] = recs[k]`.
Probe: hand-decode of asm/funcs/func_80033D38.s, then write the natural C.
Result: the natural form dropped the honest floor 17 → 9 (45 insns vs 47) on the
first build, with the shift loop and the entire tail already byte-exact. The
model is right.

### H2 — KILLED. "A single-variable scan can reach the target."
Mechanism claim: some spelling of `for (i = 3; i > 0; i--) if (recs[i].t <
D_800A3858) break;` would keep the in-loop `sll`/`addu` index math.
Probe: 8 single-variable spellings measured (plain for; array test in the for
condition; `while (i > 0 && ...)`; decrement in the body; `while (1)` + breaks;
in-body `struct HitRec *p = &recs[i]`; `s32 (*)[2]` view; flat `s32 *` view
indexed `slots[i*2+1]`), plus a `goto` variant and a `now`-local variant.
Result: all 45 insns / distance 9 (the goto and `now`-local variants strictly
worse at 19 and 11). KILLED — with one loop variable, loop.c always
strength-reduces the single DEST_ADDR giv to a walking pointer (`addiu v1,t1,24`
/ `addiu v1,v1,-8`), which is 2 insns SHORTER than target and unreachable by
respelling. Do not re-probe this family.
Banked: `rejected/single-var-for-strength-reduced.c`.

### H3 — CONFIRMED (the decisive one). Target carries the scan index and the insertion slot as TWO live variables.
Mechanism: in target loop1 the biv is `$v1` (init `addiu v1,a3,-1`, updated in the
`bgtz` delay slot, counting 2,1,0) and the record examined is `recs[v1+1]`;
`$a3` is updated by `addu a3,v1,zero` — a plain register COPY, i.e. a
source-level `n = j;` — and the invariant at the load is `a3 == v1 + 1`. So `j`
is the loop's own 0-based scan index and `n` is the insertion slot published to
`D_800A38E9`.
Probe: 7 two-variable forms, then a 27-cell shape × read-spelling ×
tail-spelling cross-product.
Result: `while (1) { j = n - 1; ... n = j; if (n <= 0) break; }` reached
distance 1. CONFIRMED.

### H4 — CONFIRMED. `j = n - 1` must be the FIRST statement of the loop body, and the loop must be `while (1)` + breaks rather than `do/while`.
Mechanism: target holds `addiu v1,a3,-1` TWICE — preheader and `bgtz` delay slot
— the signature of reorg.c stealing the loop-TOP instruction into the backedge
delay slot and duplicating it into the preheader. Written last instead, the
preheader copy shares a basic block with `n = 3;`, cse folds it to `addiu
v1,zero,2`, and the form stalls at 4. Spelled `do { ... } while (n > 0);`, GCC
peels the first array test onto a constant-folded address (`lw v0,28(v0)`) and
rotates the array compare onto the backedge: 51 insns, distance 24-25.
Probe: decrement-first vs decrement-last × `while(1)` / `do-while` / `for(;;)`.
Result: `while (1)` + decrement-first = 1; every do/while and `for(;;)` cell =
24-25. Sibling trap measured: the exit test must be `n <= 0` (which becomes the
`bgtz` backedge); the arithmetically identical `n < 1` emits slti+beqz → 25.
Banked: `rejected/dowhile-peels-first-test.c`.

### H5 — CONFIRMED. Element addresses must be materialised in named pointer variables.
Mechanism: a subscript or parenthesised pointer sum used inline builds the
address as plus(scaled_index, base) → `addu v0,v0,t1`; target has base first,
`addu v0,t1,v0`. Assigning the address to a pointer variable first flips the
operand order.
Probe: `recs[j+1].t`, the commuted subscript `(j+1)[recs].t`, `(recs+j+1)->t`,
`p = &recs[j+1]`, `p = recs + j + 1`, `p = recs + j` + `p[1].t`, at function and
block scope; same axis in the tail.
Result: all inline spellings index-first (residual 1 in the loop, 1 in the tail);
`p = recs + j + 1;` / `ins = recs + n + 1;` base-first and byte-exact. `p =
&recs[j+1]` does NOT work (keeps index-first AND costs 8). **Distance 0.**
Banked: `rejected/subscript-address-operand-order.c`.

## Frontier
Empty — the function reached honest pure-C distance 0 with a register-inclusive
47/47 verification. The only work left is outside a grind session's surface:
`retire func_80033D38` to drop the 6 stale regfix rules at regfix.txt:3447-3452,
then a full-build SHA1 verify and `queue done`.
