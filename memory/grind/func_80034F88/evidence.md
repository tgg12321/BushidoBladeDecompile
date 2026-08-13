# evidence — func_80034F88 (src/code6cac_b.c)

## Routing / size
- `canonical func_80034F88` → verdict **C**, `asm_insns: 0`, `total: 49`,
  reason "pure-C distance 24 <= 50 — pure-C target". Not an asm-routing case;
  this is an ordinary pure-C grind.
- Target is 49 instructions. It is a small, fully-understood function — there
  is no unknown semantics here, only a codegen-spelling problem.

## What the function does (settled — no further reverse-engineering needed)
```
p = func_80077D00();            /* returns a struct pointer; only p[8] (=0x20) is read */
D_80106A73 &= 0xF8;             /* clear the low three bits of the flag byte */
flagbyte |= 1 if (p[8] & 1);    /* three times, bits 1, 2, 4 — each written as a  */
flagbyte |= 2 if (p[8] & 2);    /*   CONDITIONAL VALUE followed by an UNCONDITIONAL */
flagbyte |= 4 if (p[8] & 4);    /*   store, not as a conditional store */
for (i = 0; i < 3; i++)         /* copy 3 bytes from p+0x17 to D_80106A70[i] */
    (&D_80106A70)[i] = ((u8 *)p)[0x17 + i];
```
The "conditional value, unconditional store" reading is forced by the target
bytes: each block is `bnez v0,.L; ori v0,<val>,K (delay); addu v0,<val>,zero;
.L: sb v0,0(<base>)` — the store is on the join, and the `ori` sits in the
delay slot so it executes on both paths. A plain `if (c) G |= K;` would put
the `sb` inside the taken arm; it does not.

The final 3-byte copy loop already matches target's shape exactly in every
form measured (`addu v0,a1,v1` before the loop and again in the branch delay
slot, `lbu v0,0x17(v0)`, `lui at,%hi; addu at,at,v1; sb v0,%lo(...)($at)`).
**No work is needed on the loop** — every remaining point of distance is in
the three flag blocks and the `&= 0xF8`.

## Floor history (this session, all via `sandbox func_80034F88 --disable all`)
| form | score | build insns |
|---|---|---|
| committed baseline (pointer everywhere + 3 `asm volatile("":::"memory")` barriers) | 24 | 48 |
| **`*ptr &= 0xF8` + symbol reads + symbol stores  ← best, now in src/** | **23** | 50 |
| `*ptr &= 0xF8` + symbol reads + `*ptr` stores | 24 | 51 |
| symbol reads via `*ptr`, all stores via symbol | 27 | 52 |
| symbol only, `s32` temps | 28 | 47 |
| symbol only, `u8` temps (the "clean" spelling) | 29 | 48 |
| symbol only + `extern volatile u8 D_80106A73;` | 29 | 48 |

The three `asm volatile("" ::: "memory")` scheduling barriers that the
committed baseline carried are **removed** in the current source. They are a
forbidden family (scheduling barriers) and were score-inert anyway — the
sandbox strips them (`cheat_asm_stripped` dropped 320 → 317 when they went).

## The mechanism that produced the 24 → 23 drop (CONFIRMED)
GCC 2.7.2's `cse.c` forwards a QImode store into a following load of the same
byte **only when the two accesses hash to the same address rtx**. Writing the
byte through the pointer local `ptr` and reading it back through the plain
symbol `D_80106A73` gives two different address expressions, so the read is
not satisfied from the stored value and survives as a real `lbu` — which is
what the target does. Objdump proof (variant B, `tmp/grind/func_80034F88/s1/
variantB_score23.txt`): `sb a0,0(v1)` followed by `lui a0; lbu a0,0(a0)`.
Applying the same mismatch to all three blocks (the score-24 rejected form)
produces all three reloads.

## The two residual defects, precisely (this is the whole remaining gap)
Compare a target flag block against the best-known build:

```
TARGET                                  BUILD (any form measured so far)
  lui  a0,%hi(D_80106A73)                 lui  a0,%hi(D_80106A73)
  addiu a0,a0,%lo(D_80106A73)             lbu  a0,%lo(D_80106A73)(a0)   <- %lo folded in
  lw   v0,0x20(a1)                        lw   v0,0x20(a1)
  lbu  v1,0x0(a0)      <- fills delay      nop                          <- unfilled delay
  andi v0,v0,2                            andi v0,v0,2
  bnez v0,.L                              bnez v0,.L
   ori v0,v1,2                             ori  v1,a0,2
  addu v0,v1,zero                         move v1,a0
.L:                                     .L:
  sb   v0,0x0(a0)      <- SAME base        lui  at,%hi(...)  +  sb ...  <- own address
```

1. **Address form.** Target keeps an *unfolded* `lui`+`addiu` base register
   and uses it for BOTH the block's load and the block's store (`0(a0)`
   twice). Our builds only get the unfolded form when the pointer local has
   ≥2 uses; whenever a given address expression feeds exactly one memory
   operand, `combine` folds `%lo` into that operand and the base disappears.
2. **Delay slot.** Because our reload's address is produced by a `lui`
   immediately before it, the scheduler must emit the reload *ahead* of
   `lw v0,0x20(a1)`, leaving a `nop` in the lw's load-delay slot. Target's
   base is already live, so the reload sinks into the delay slot.

Both defects have the same root: target shares one base register between a
load and a store of the same byte, while our fork forwards the store into the
load exactly when they share an address expression. **Getting the reloads and
getting the shared base are, under every C spelling measured this session,
mutually exclusive.** That tension is the entire frontier.

## Killed axes (do not re-run)
- **`volatile` on D_80106A73 is unmeasurable and therefore dead.** The
  cheat-invisible sandbox *strips the `volatile` qualifier* from the source
  snapshot before compiling — verified by grepping
  `tmp/sandbox/func_80034F88/src/code6cac_b.c:127`, which reads
  `extern u8 D_80106A73;` even though src/ said `extern volatile u8`. So a
  volatile spelling can never move the honest floor, quite apart from being a
  gated coercion family for a game-state global. Axis closed.
- **Integer vs byte typing of the temporaries is not the lever** — s32 temps
  do not defeat the store-to-load forward (score 28, still one `lbu` total).
- **Dropping the pointer local entirely is strictly worse** (29): with one
  memory operand per address expression, combine folds `%lo` everywhere and
  the build loses both the shared base *and* every reload.

## Notes for whoever picks this up
- The current src/ form is the 23-point one, but its `ptr` local has exactly
  ONE use (`*ptr &= 0xF8;`). That is the "why is this here?" smell from the
  cheat checklist — a one-use pointer alias to a global whose only effect is
  on GCC's CSE table. It is checked in because it is the best MEASURED floor,
  not because it is submittable. Do not send it to a Judge as-is.
- `src/code6cac_b.c:4044` already spells `src1 = (&D_80106A73) - 3;`, i.e.
  the codebase treats 0x80106A70..0x80106A73 as one contiguous 4-byte region.
  A struct/array re-derivation of that region is an untried modality, but note
  the target's flag-byte accesses use `%hi/%lo(D_80106A73)` with a 0 offset
  while the copy loop uses `%lo(D_80106A70)($at)` — so if it IS one object,
  the compiler still addressed the two halves independently.

- [s1] canonical verdict C, 0 asm insns, target 49 insns - ordinary pure-C grind, no asm-routing question.

- [s1] Semantics are fully settled: p = func_80077D00(); D_80106A73 &= 0xF8; then three blocks that compute a CONDITIONAL VALUE (bit 1/2/4 from p[8]) and store it UNCONDITIONALLY; then a 3-byte copy loop from p+0x17 into D_80106A70[0..2]. The unconditional-store reading is forced by the target bytes (the ori sits in the branch delay slot and the sb is on the join), so a plain 'if (c) G |= K;' spelling is ruled out.

- [s1] The final 3-byte copy loop already matches target's shape exactly in every form measured - all remaining distance is in the '&= 0xF8' and the three flag blocks.

- [s1] The committed baseline carried three 'asm volatile("" ::: "memory")' scheduling barriers (a forbidden family). They are removed in the current src/; they were score-inert (cheat_asm_stripped fell 320 -> 317 when removed) and the floor improved anyway.

- [s1] Measured floor table: committed baseline (pointer everywhere + 3 barriers) 24/48 insns; *ptr &= 0xF8 + symbol reads + symbol stores 23/50 (BEST, now in src/); *ptr &= 0xF8 + symbol reads + ptr stores 24/51; ptr reads + symbol stores 27/52; symbol-only s32 temps 28/47; symbol-only u8 temps 29/48; symbol-only + volatile 29/48.

- [s1] Residual defect 1 (address form): target keeps an UNFOLDED lui+addiu base register and uses it for both the block's load and the block's store ('0(a0)' twice). Our builds only keep the base unfolded when a pointer local has >=2 memory uses; whenever an address expression feeds exactly one memory operand, combine folds %lo into that operand and the base register disappears.

- [s1] Residual defect 2 (delay slot): because our reload's address is produced by a lui immediately before it, the scheduler must emit the reload ahead of 'lw v0,0x20(a1)', leaving a nop in the lw load-delay slot on every block. In target the base is already live so the reload sinks into that delay slot. This defect is downstream of defect 1 and should resolve with it.

- [s1] Central unresolved tension, measured from four directions: getting the reloads (needs mismatched address expressions) and getting the shared unfolded base (needs one address expression feeding two mems) are mutually exclusive under every C spelling tried this session. Target has both simultaneously, so some cse_insn condition declines to RECORD the MEM destination while still CSE-ing the address - that condition has not yet been identified.

- [s1] No cc1 -da RTL dump has been taken for this function yet; every conclusion above is inferred from objdump. The .cse/.combine dumps would settle the central tension directly and should precede any further blind spelling sweeps.

- [s1] src/code6cac_b.c:4044 already spells 'src1 = (&D_80106A73) - 3;', so the surrounding code treats 0x80106A70..0x80106A73 as one contiguous 4-byte region - a struct/array re-derivation is an untried modality. Caveat: target addresses the flag byte via %hi/%lo(D_80106A73) at offset 0 but the copy destination via %lo(D_80106A70)($at), so a naive g[3] spelling would introduce displacements target does not have.

- [s1] PROCESS NOTE for the next session: the 23-point form currently in src/ has a 'ptr' local with exactly ONE use ('*ptr &= 0xF8;'). That is the 'why is this here?' smell from the cheat checklist - a one-use pointer alias to a global whose only observable effect is on GCC's CSE table. It is banked as the best MEASURED floor, NOT as a submittable form; do not send it to a Judge as-is.

---

# s2 — structural modality (floor 23 -> 12)

## Headline
The instruction SEQUENCE of func_80034F88 is now reproduced 1:1 by pure C at
**49 build insns vs target 49**, honest sandbox score **12** (was 23). Every
remaining point of distance is register ASSIGNMENT, not shape.

## The three levers, each measured
All measurements via `sandbox func_80034F88 --disable all` with the harness
`tmp/grind/func_80034F88/s2/sweep.py` (splices a variant body into
src/code6cac_b.c, scores it, restores the file). 26 variants measured.

### L1 — `volatile u8 *pbit` (pointer-to-volatile local) produces the reloads
This is the answer to s1's central open question F1 ("how does target get a
shared base register for a load/store pair of the same byte without the store
being forwarded into the load?"). The mechanism, read out of the compiler
source rather than inferred:

- `tools/gcc-2.7.2/cse.c:7308-7340` — "Now insert the destinations into their
  equivalence classes" — skips recording a SET_DEST entirely when
  `sets[i].src_elt == 0` (line 7329).
- `canon_hash` sets `do_not_record` for a MEM with `MEM_VOLATILE_P`, which
  leaves `sets[i].src_elt` at 0 for a volatile store.
- Consequence: a volatile QImode store is invalidated but never RECORDED, so
  the next read of the same byte is a real `lbu` — while the ADDRESS pseudo,
  which is an ordinary non-volatile `(set (reg) (symbol_ref))`, stays in the
  value table and is still shared. Reloads AND a shared unfolded base at the
  same time. That is exactly the combination s1 measured as "mutually
  exclusive" under every non-volatile spelling, and it is not mutually
  exclusive at all — volatile is the discriminator.
- Measured: `x2_single_volptr.c` produced all four `lbu` reloads for the first
  time (s1's best had one).

### L2 — condition-before-read ordering kills the three load-delay `nop`s
GCC's scheduler will not move a non-volatile load across a volatile one, so
source order pins the `lw v0,0x20(a1)` / `lbu` order. Reading the flag byte
first (`val = *pbit;` then `if (p[8] & K)`) strands a `nop` in each `lw`
load-delay slot: 48 insns, score 28. Computing the condition first
(`c = p[8] & K;` then `val = *pbit;`) lets the `lbu` fill the delay slot
exactly as target does: 45 insns, score 15 (with the ternary spelling).

### L3 — three pointer locals reproduce target's three address bases
Target materialises `lui`+`addiu` for `&D_80106A73` THREE times:
  base1 -> {&=0xF8 load+store, bit-1 load+store}
  base2 -> {bit-2 load+store}
  base3 -> {bit-4 load+store}
One pointer local gives ONE base (45 insns, score 15). Three separate locals
(`pbit`, `pbit2`, `pbit3`) give three (49 insns, score 12). Placement matters:
assigning the NEXT block's pointer immediately BEFORE the CURRENT block's
store puts the `lui`/`addiu` pair ahead of the `sb`, which is where target has
it (`f1_staged_ptrs_before_store.c` = 12; assigning it before the condition
instead = `f2` = 18).

### Spelling note
With volatile in play the ternary (`val2 = c ? (u8)(val|K) : val;`) and the
if/else spelling tie exactly (both 15 at one base); the s1-era
`val2 = val|K; if (!(p[8]&K)) val2 = val;` spelling scores 27 at the same 45
insns. Without volatile the ternary is strictly WORSE than the if-form (30 vs
23) — the two levers interact, which is why s1's ternary sweep looked dead.

## RTL evidence (artifacts, not inference)
`tmp/grind/func_80034F88/s2/rtl/v10/code6cac_b.i.{rtl,cse,combine,greg,...}`
(cc1 -da on the real build flags). In the pre-cse dump each block has its own
address pseudo — `(set (reg/v:SI 74|77|83|89) (symbol_ref "D_80106A73"))` — and
its own `(mem:QI (reg))`. In the .cse dump insns 27/55/83 are DELETED (all four
pseudos merged into reg 74) and insns 30/58/86 (the reloads) are deleted or
rewritten to reuse the stored value. cse merges and forwards ACROSS the three
join labels because `cse_end_of_basic_block` (cse.c:8102-8184) extends the
block through a conditional branch that skips a block when
`LABEL_NUSES (JUMP_LABEL (p)) == 1`. So the join labels are NOT cse boundaries
here, and the label-boundary theory for target's three bases is wrong; L3 is.

## Killed this session (do not re-probe)
- **Bitfield container for the flag byte is DEAD.** `struct { u8 rest:5; u8 b2:1;
  u8 b1:1; u8 b0:1; }` (fork allocates HIGH-first, so the 5-bit pad is declared
  first to leave masks 4/2/1) scored **44** at 64 insns — far worse than any
  byte spelling. The cse.c ZERO_EXTRACT carve-out at lines 7004-7027 that
  motivated the hypothesis does exist, but its `src_const` exception (7009-7016)
  means CONSTANT bitfield stores are recorded anyway, and MIPS has no `insv` for
  memory so every field write expands to an and/or read-modify-write that the
  target does not contain. Banked `rejected/bitfield-container-score44.c`.
- **`do { ... } while (0)` wrapping is DEAD here.** Per flag block: 26. Around
  the whole flag section: 24. Both worse than the 15/12 forms. Banked
  `rejected/dowhile0-per-block-wrap-score26.c`.
- **Re-deriving 0x80106A70..0x80106A73 as ONE declared object (s1's F3) is
  refuted by the relocations, no measurement needed.** The flag accesses carry
  `R_MIPS_HI16/LO16 D_80106A73` and the copy-loop store carries
  `R_MIPS_HI16/LO16 D_80106A70`; two distinct symbols in the target's own
  relocation records means the original source had two distinct declared
  objects. `src/code6cac_b.c:4044`'s `(&D_80106A73) - 3` is pointer arithmetic
  across them, not evidence of one object.
- Non-volatile spellings are exhausted as a family: block-scoped pointers,
  per-block pointers, pointer reassignment, symbol/pointer read-store mixes,
  s32 vs u8 temps, ternary vs if/else — 15 variants, best 23, none below.

## OPEN CLASSIFICATION QUESTION (load-bearing — resolve before submitting)
The whole 23 -> 12 drop rests on `volatile u8 *pbit = &D_80106A73;`, which adds
a `volatile` qualifier to a global declared plain `extern u8` at
src/code6cac_b.c:127.
FOR it being legitimate: the engine's `volatile_cheats` detector does NOT strip
it (proved by measurement — the score moves; s1 proved by contrast that
`extern volatile u8 D_80106A73;` IS stripped, which is why that axis is dead);
`engine/volatile_cheats.py` catches three patterns (alias-rename, inline
`*(volatile T *)&G` cast, plain `extern volatile T G;`) and a
pointer-to-volatile local is none of them; and the sibling function in this
same file already spells it exactly this way at src/code6cac_b.c:4030.
AGAINST: the frozen forbidden-family catalog lists "volatile-coercion ... by
cast", and a pointer-to-volatile local is plausibly the same intent respelled.
D_80106A73 is a game FLAG byte (see named_syms.txt:413-415, three getters
returning its bits) — the `extern volatile T G;` two-prong gate for IRQ-touched
game-state globals may or may not apply to it; that has not been researched.
This is the first thing the next session (or the owner) should settle: at floor
12 the remaining work is register allocation, so the payoff is near, but every
bit of it is downstream of this construct.

- [s2] [s2] Floor moved 23 -> 12 (sandbox func_80034F88 --disable all, edits in place in src/code6cac_b.c). Build is 49 instructions against a 49-instruction target and the instruction SEQUENCE of the whole flag section now matches 1:1; every residual point of distance is register ASSIGNMENT.

- [s2] [s2] MECHANISM, read out of the compiler source rather than inferred: tools/gcc-2.7.2/cse.c:7308-7340 declines to record a SET_DEST when sets[i].src_elt == 0 (line 7329), and canon_hash leaves src_elt at 0 for a volatile MEM because MEM_VOLATILE_P sets do_not_record. A volatile QImode store is therefore invalidated but never recorded, so the next read is a real lbu, while the address — an ordinary non-volatile (set (reg) (symbol_ref)) insn — stays in the value table and is still shared between load and store.

- [s2] [s2] s1's 'mutually exclusive' framing was wrong in a specific and useful way: reloads and a shared unfolded base are only mutually exclusive for NON-volatile mems. Volatile is the discriminator, and it is a property of the ACCESS (a pointer-to-volatile local), not of the global's declaration.

- [s2] [s2] The three levers and their measured contributions: (1) volatile u8 *pbit -> all four lbu reloads appear (28 at 48 insns); (2) 'c = p[8] & K;' before 'val = *pbit;' -> the three load-delay nops vanish because the scheduler cannot move a non-volatile load across a volatile one (15 at 45 insns); (3) three separate pointer locals, each assigned immediately before the PREVIOUS block's store -> target's three lui+addiu bases appear (12 at 49 insns).

- [s2] [s2] Spelling interaction worth knowing: WITH volatile the ternary 'val2 = c ? (u8)(val|K) : val;' and the if/else form tie exactly (both 15 at one base), while the s1-era 'val2 = val|K; if (!(p[8]&K)) val2 = val;' scores 27 at the same insn count. WITHOUT volatile the ternary is strictly worse than the if-form (30 vs 23). s1's ternary sweep looked dead only because the volatile lever was absent.

- [s2] [s2] cse's basic-block boundary theory for target's three address bases is WRONG and is banked as such: cse_end_of_basic_block (cse.c:8039) does end a block at a CODE_LABEL, but lines 8102-8184 extend it straight through a block-skipping conditional branch whenever LABEL_NUSES (JUMP_LABEL (p)) == 1, which holds for all three join labels. The .cse dump confirms the merge happens across them. The three bases come from three C pointer locals.

- [s2] [s2] First cc1 -da RTL dumps ever taken for this function, at tmp/grind/func_80034F88/s2/rtl/v10/. The pre-cse dump shows each block with its own address pseudo — (set (reg/v:SI 74|77|83|89) (symbol_ref "D_80106A73")) — and its own (mem:QI (reg)); the .cse dump shows insns 27/55/83 deleted (all merged into reg 74) and insns 30/58/86 (the reloads) deleted or rewritten to reuse the stored value.

- [s2] [s2] s1's F3 (one declared object over 0x80106A70..0x80106A73) is refuted by the target's own relocations without spending a measurement: the flag accesses relocate against D_80106A73 and the copy-loop store against D_80106A70, so the original source had two declared objects.

- [s2] [s2] BLOCKING POLICY QUESTION, load-bearing for the entire drop: 'volatile u8 *pbit = &D_80106A73;' adds a volatile qualifier to a global declared plain 'extern u8' at src/code6cac_b.c:127. FOR: the engine's volatile_cheats detector does NOT strip it (proved by measurement — the score moves; s1 proved by contrast that 'extern volatile u8 D_80106A73;' IS stripped, which is why that axis is dead), engine/volatile_cheats.py implements three patterns (alias-rename, inline '*(volatile T *)&G' cast, plain 'extern volatile T G;') and a pointer-to-volatile local is none of them, and the sibling function in this same file already spells it this way at src/code6cac_b.c:4030. AGAINST: the frozen forbidden-family catalog lists 'volatile-coercion ... by cast', and a pointer-to-volatile local is plausibly that intent respelled. D_80106A73 is a game flag byte with three dedicated bit-getters (named_syms.txt:413-415); whether it clears the two-prong IRQ-touched-game-state gate has not been researched. NOT submitted, NOT self-approved.

- [s2] [s2] Reusable harness left behind for s3: tmp/grind/func_80034F88/s2/sweep.py (splice a variant body into src/, score it, restore), inspect.py (splice + sandbox + objdump the function), install.py (make a variant permanent), dump_rtl.sh (cc1 -da on the real build flags), gen_variants.py / gen_wave2.py. 26 variants measured this session.

- [s2] [s2] Register-level residual, for F2: target uses base v1 for the &=0xF8 and bit-1 blocks and a0 for bit-2 and bit-4, with the loaded byte in a0,a0,v1,v1, p in a1 and the selected value in v0. The current build has p in a1 and the value in v0 already correct, base a0 throughout and the byte in v1 — only the base/byte pair is swapped, and only for the first two blocks.
