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

---

# s3 — structural modality (honest floor 23 -> 20; the volatile axis ruled out)

## Headline
Two volatile-free structural levers dropped the HONEST floor to **20** at 49
build insns vs a 49-insn target, and the blocking policy question s2 left open
was resolved AGAINST the volatile family with citations and gate research — so
s2's 12 is discarded and 20 is the real state of this function.

## The two new honest levers (measured, `sandbox --disable all`)
| form | score | insns |
|---|---|---|
| s1 best: `*ptr &= 0xF8` + symbol reads/stores, read-before-condition | 23 | 50 |
| + condition BEFORE the flag read (`c = p[8] & K;` first) | **22** | 49 |
| + copy loop spelled `*((u8 *)p + i + 0x17)` (iv before the constant) | **20** | 49 |

1. **Condition-before-read is worth 1 point on the NON-volatile form too.**
   s2 found this lever on the volatile form and never back-ported it; it also
   removes one instruction (50 -> 49, target's exact count).
2. **The copy loop was NOT already matching.** s1 and s2 both recorded "the copy
   loop matches target's shape exactly in every form measured". It did not:
   `*((u8 *)p + 0x17 + i)` emits `addu v0,v1,a1` where target has
   `addu v0,a1,v1` — operand order reversed — and the pair appears twice (loop
   body + branch delay slot), so it was worth 2 points. Writing the induction
   variable before the constant displacement, `*((u8 *)p + i + 0x17)`, fixes it.
   Equivalent-looking spellings do NOT: `((u8 *)p)[i + 0x17]` and
   `(&D_80106A70)[i] = ((u8 *)p)[0x17 + i]` both stay at 22.

## The volatile ruling (s2's blocking F1) — RESOLVED, and resolved AGAINST it
s2 could not classify `volatile u8 *pbit = &D_80106A73;` (the construct the
whole 23 -> 12 drop rested on). s3 resolved it from the project's own artifacts
rather than escalating:
- `engine/volatile_cheats.py` docstring, pattern 2: "Inline
  `*(volatile T *)&D_globalsym` casts on game-RAM symbols. Treats a plain
  game-state global as volatile at the access site, defeating CSE/scheduling."
  A local `volatile u8 *` initialised with `&D_80106A73` is that cast in two
  statements. Same object, same added qualifier, same CSE effect.
- The module's pointer-to-volatile carve-out is explicitly about
  `extern volatile T *name;` declarations whose pointee "is whatever address the
  pointer holds (typically a hardware register address loaded at startup)" — not
  a local aliasing a known plain global. The detector not stripping it is a
  regex gap; cheat-checklist T4/T5 say a gap is not a sanction.
- The sanctioned alternative (`extern volatile u8 D_80106A73;` + an entry in
  `volatile_extern_allowlist.txt`) FAILS BOTH PRONGS of
  `.claude/rules/legitimate-volatile-interrupt-touched.md`, researched this
  session: prong 1 — 0x80106A73 is KSEG0 game RAM (not the 0x1F8xxxxx MMIO
  range) and none of the six functions that touch it (func_800167AC/BC/D4/EC,
  func_80034708, func_80034F88, func_80035280) is installed via
  InterruptCallback / VSyncCallback / irq_EnableInterrupts /
  irq_AcknowledgeVblank anywhere in src/; the only external storer,
  func_800167EC, is called synchronously from src/ings.c:414. prong 2 — the
  use-site shape is store-then-readback in straight-line code, which is not on
  the rule's exact three-shape list (spin-wait / double-read-across-
  sequence-point / IRQ-mutated-loop-bound).

## Why the remaining 20 points need volatile (mechanism, not inference)
Target reads the flag byte back from memory four times, each `lbu`/`sb` pair
sharing ONE unfolded base (`lbu a0,0(v1)` directly after `sb a0,0(v1)` — it
reloads into the very register that already holds the stored value).
- `cse.c:7308-7340` skips recording a SET_DEST when `sets[i].src_elt == 0`
  (line 7329), and `canon_hash` (cse.c:1941-1947) sets `do_not_record` for a MEM
  **only** when `MEM_VOLATILE_P` is set. The other `do_not_record` triggers
  (PRE/POST inc-dec, PC, CC0, CALL, UNSPEC_VOLATILE, volatile ASM_OPERANDS, and
  hard registers under SMALL_REGISTER_CLASSES) are all unreachable from C in
  this function.
- Two C address expressions for the same byte are therefore a dichotomy: hash
  the same (cse merges the pseudos — one base, and the store forwards into the
  load so the reload dies) or hash differently (no forward, but two `lui`s and
  `combine` folds `%lo` into each single-use mem). Every one of the ~30 spellings
  measured across s1-s3 lands on one side or the other.
- The aliasing escape (`(&D_80106A70)[3]` for one side of the pair) is not an
  escape: GCC would treat the two expressions as distinct objects and MISCOMPILE
  the read-back, so it cannot be what the original source did.

## Correction to an s2 conclusion (premise was false)
s2 killed s1's F3 (one declared object over 0x80106A70..0x80106A73) "by the
target's own relocation records". A shipped PS-EXE has **no relocation records**
— splat names `lui/addiu` pairs by resolving the computed ADDRESS to the nearest
symbol (see [[splat-symbol-names-are-not-evidence]]). So the .s file's
`%hi(D_80106A73)` is splat's rendering, not evidence about the original
declaration, and s2's stated reason for the kill is void. F3 stays dead for a
different and stronger reason: whatever the address is spelled as, the base-1
`sb` -> `lbu` pair needs ONE shared address pseudo, which is exactly the case
where cse forwards.

## Forensic scorer built this session (reusable)
`tmp/grind/func_80034F88/s3/rawscore.py` scores src/ as-it-stands with all
regfix/asmfix rules dropped but NO volatile strip and NO cheat-asm strip, using
the same `engine/score.py` distance. It is the only way to see what a
declaration-level `volatile` would compile to, since the sandbox strips it. Its
numbers are FORENSIC ONLY and are never an honest-floor claim. Companion
harnesses: `s3/probe.py` (splice variant -> sandbox score + raw score ->
restore), `s3/sidebyside.py` (difflib-aligned build-vs-target instruction
listing), `s3/apply.py`.

- [s3] Honest floor 23 -> 20 with edits in place in src/code6cac_b.c, 49 build insns vs 49 target insns, zero volatile and zero cheat-asm (the three committed `asm volatile("" ::: "memory")` barriers are gone; cheat_asm_stripped fell 320 -> 317).
- [s3] Lever A (+1 point, -1 insn): computing the block condition into a local BEFORE reading the flag byte works on the NON-volatile form as well as the volatile one. s2 discovered it on the volatile form and never back-ported it, which is why the non-volatile family looked exhausted at 23.
- [s3] Lever B (+2 points): the copy loop was never matching. `*((u8 *)p + 0x17 + i)` emits `addu v0,v1,a1`; target has `addu v0,a1,v1`. Spelling the induction variable before the constant — `*((u8 *)p + i + 0x17)` — flips the operand order, and the insn appears twice (loop body and branch delay slot). `((u8 *)p)[i + 0x17]` and the array-both spelling do NOT fix it (both stay 22).
- [s3] The one-pointer mask alias is load-bearing and is a FROZEN-list family ("C-level pointer alias to a global"): `u8 *ptr = &D_80106A73; *ptr &= 0xF8;` is a read-modify-write, so the address expression has TWO memory uses and combine cannot fold `%lo` into either — target's unfolded lui+addiu base survives and the first lbu reload materialises. Spelling the mask as `D_80106A73 &= 0xF8;` costs 9 points (29).
- [s3] Non-volatile family re-swept with the s2 levers in hand: three pointer locals buy nothing without volatile (cse merges the address pseudos: sym/sym n1 = 22, n3 = 22); ptr reads + sym stores = 27, sym reads + ptr stores = 26, ptr both = 24; ternary spelling is worse than if/else without volatile (26 vs 22), confirming s2's observation that the ternary only wins in the presence of volatile.
- [s3] s2's blocking F1 is RESOLVED AGAINST the construct: `volatile u8 *pbit = &D_80106A73;` is engine/volatile_cheats.py pattern 2 (`*(volatile T *)&D_globalsym` on a game-RAM global) respelled in two statements; the detector's silence is a regex gap, and the module's pointer-to-volatile carve-out is scoped to `extern volatile T *name;` declarations whose pointee is a runtime hardware address, not to a local aliasing a known plain global.
- [s3] The sanctioned volatile path is measured DEAD on both prongs. Prong 1: 0x80106A73 is KSEG0 game RAM, and none of func_800167AC/BC/D4/EC, func_80034708, func_80034F88, func_80035280 is installed via InterruptCallback / VSyncCallback / irq_EnableInterrupts / irq_AcknowledgeVblank; the only external storer func_800167EC is called synchronously at src/ings.c:414. Prong 2: the use-site is store-then-readback in straight-line code, not on the rule's exact three-shape list.
- [s3] Even if the allowlist grant existed it would NOT close the function: forensic raw scores (rawscore.py, no volatile strip) put `extern volatile u8 D_80106A73;` with plain symbol accesses at 28-32 / 51 insns — WORSE than the honest 20 — because with every access volatile, combine cannot fold `%lo` into any of the eight mems. The 12 belongs to the three volatile POINTER LOCALS, not to the declaration.
- [s3] Mechanism for the residual 20: cse.c:7329 skips recording a SET_DEST only when sets[i].src_elt == 0, and canon_hash (cse.c:1941-1947) sets do_not_record for a MEM only under MEM_VOLATILE_P. Every other do_not_record trigger (PRE/POST inc-dec, PC, CC0, CALL, UNSPEC_VOLATILE, volatile ASM_OPERANDS, hard reg under SMALL_REGISTER_CLASSES) is unreachable from C here. Two address expressions for one byte therefore either hash the same (merged -> store forwards -> reload dies) or hash differently (no forward -> two luis, %lo folded into each single-use mem). Aliasing spellings like (&D_80106A70)[3] are not an escape because GCC would treat them as distinct objects and miscompile the read-back.
- [s3] CORRECTION to s2: its F3 kill cited "the target's own relocation records", but a shipped PS-EXE has no relocations — splat renders `%hi(D_80106A73)` by resolving the computed address to the nearest symbol ([[splat-symbol-names-are-not-evidence]]). The premise is void; F3 stays dead for the stronger reason that base-1's sb -> lbu pair needs one shared address pseudo, which is precisely the forwarding case.

- [s3] FLOOR UPDATE: the s3 floor is 18, not 20. `s32 val = D_80106A73;` (int-typed loaded byte, u8 val2) on top of the condition-before-read and copy-loop-operand-order levers scores 18 at 51 build insns. s1 had killed int-typed temporaries at 28, but that measurement predated every lever found since; the type of the loaded byte only pays once the address/ordering shape is right. 16 further variants (declaration order, s32 val2, ternary, positive-sense if, shared condition/value local, pointer stores, hoisted p[8], read-first) are all 18 or worse.

- [s3] Honest floor 23 -> 18 (`sandbox func_80034F88 --disable all`), edits in place in src/code6cac_b.c, 51 build insns vs 49 target insns, zero volatile and zero cheat-asm — the three committed `asm volatile("" ::: "memory")` scheduling barriers are gone (cheat_asm_stripped 320 -> 317).

- [s3] Lever A (+1 point, -1 insn): computing the block condition into a local BEFORE reading the flag byte works on the NON-volatile form as well as the volatile one. s2 found it on the volatile form and never back-ported it, which is why the non-volatile family looked exhausted at 23.

- [s3] Lever B (+2 points): the copy loop was never matching, contrary to an explicit s1 and s2 evidence claim. `*((u8 *)p + 0x17 + i)` emits `addu v0,v1,a1`; the target has `addu v0,a1,v1`. Writing the induction variable before the constant displacement — `*((u8 *)p + i + 0x17)` — flips it, and the insn appears twice (loop body + branch delay slot).

- [s3] Lever C (+2 points): `s32 val = D_80106A73;` with `u8 val2`. Trades 2 extra instructions for 2 points of distance; `s32 val2` cancels it.

- [s3] The one-pointer mask alias is load-bearing and sits in a FROZEN-list family ('C-level pointer alias to a global'): `u8 *ptr = &D_80106A73; *ptr &= 0xF8;` is a read-modify-write, so the address expression has TWO memory uses, combine cannot fold %lo into either, the target's unfolded lui+addiu base survives, and the first lbu reload materialises. Spelling the mask as `D_80106A73 &= 0xF8;` costs 9 points (29).

- [s3] Non-volatile family re-swept with s2's levers in hand: three pointer locals buy nothing without volatile (cse merges the address pseudos — sym/sym n1 = 22, n3 = 22); ptr reads + sym stores = 27; sym reads + ptr stores = 26; ptr both = 24; the ternary value-select is worse than if/else without volatile (26 vs 22), confirming s2's note that the ternary only wins in the presence of volatile.

- [s3] s2's blocking F1 is RESOLVED AGAINST the construct with citations: `volatile u8 *pbit = &D_80106A73;` is engine/volatile_cheats.py pattern 2 respelled in two statements; the detector's silence is a regex gap, and the module's pointer-to-volatile carve-out is scoped to `extern volatile T *name;` declarations whose pointee is a runtime hardware address.

- [s3] The sanctioned volatile path is measured DEAD on both prongs of .claude/rules/legitimate-volatile-interrupt-touched.md. Prong 1: 0x80106A73 is KSEG0 game RAM and none of func_800167AC/BC/D4/EC, func_80034708, func_80034F88, func_80035280 is installed via InterruptCallback / VSyncCallback / irq_EnableInterrupts / irq_AcknowledgeVblank; the only external storer func_800167EC is called synchronously at src/ings.c:414. Prong 2: store-then-readback in straight-line code is not on the rule's exact three-shape list.

- [s3] Even a granted allowlist entry would NOT close the function: forensic raw scores (no volatile strip) put `extern volatile u8 D_80106A73;` with plain symbol accesses at 28-32 / 51 insns, worse than the honest 18, because with every access volatile combine cannot fold %lo into any of the eight mems. The 12 belongs to the three volatile POINTER LOCALS, not to the declaration.

- [s3] Mechanism for the residual 18: cse.c:7329 skips recording a SET_DEST only when sets[i].src_elt == 0, and canon_hash (cse.c:1941-1947) sets do_not_record for a MEM only under MEM_VOLATILE_P; every other trigger is unreachable from C here. Two address expressions for one byte therefore either merge (store forwards, reload dies) or diverge (no forward, but %lo folds into each single-use mem). Aliasing spellings like (&D_80106A70)[3] would be treated as a distinct object and miscompile the read-back.

- [s3] CORRECTION to s2: its F3 kill cited 'the target's own relocation records', but a shipped PS-EXE has no relocations — splat renders %hi(D_80106A73) by resolving the computed address to the nearest symbol ([[splat-symbol-names-are-not-evidence]]). The premise is void; F3 stays dead for the stronger reason above.

- [s3] New reusable instrument: tmp/grind/func_80034F88/s3/rawscore.py scores src/ as it stands with all rules dropped but NO volatile strip and NO cheat-asm strip, using the same engine/score.py distance. It is the only way to see what a declaration-level volatile compiles to. Its numbers are FORENSIC ONLY and are never an honest-floor claim. Companions: probe.py (sandbox + raw per variant), sidebyside.py (difflib-aligned build-vs-target listing), apply.py, gen_g.py, gen_h.py.

---

# s4 — permuter modality (floor unchanged at 18; the permuter axis is measured DEAD)

## Headline
decomp-permuter has now been run on func_80034F88 for the first time —
**50,425 iterations across two structurally different chassis** — and it does
not reach below the honest floor of **18**. Every one of the 97 finds was
re-scored with the honest engine sandbox; the best honest, semantically-correct
find ties the seed at 18. Separately, the four remaining "untried" structural
probes s3 left on frontier F1 were measured and none of them improves the floor.
The floor stands at 18, and it now stands with the permuter axis closed.

## The two campaigns (telemetry in metrics/events.jsonl)
| seed | chassis | insns | base sandbox | iters | finds | best honest find |
|---|---|---|---|---|---|---|
| 1 | s3 `h0_s32val` (`s32 val` / `u8 val2`) | 51 | 18 | 34,003 | 30 | 18 (tie) |
| 2 | s3 `g0_floor20` (`u8 val` / `u8 val2`) | 49 | 20 | 16,422 | 67 | 18 (tie) |

Both were launched via `tools/permuter_campaign.py launch` with `-j 8
--stop-on-zero`, waited for IN-TURN with `permuter_campaign.py wait`, and
`harvest --stop`ped before the session ended. Workspaces:
`tmp/grind/func_80034F88/s4/ws` and `.../ws2`.

Seed 2 is notable for CONVERGING ON SEED 1: several of its 18-point finds are
51-instruction forms, i.e. the permuter independently rediscovered the
`s32`-widened loaded byte starting from the 49-instruction `u8` chassis. Two
structurally distinct starting points, one basin, the same floor.

## THE LOAD-BEARING METHODOLOGICAL FINDING (the reusable part)
**The permuter's weighted score is UNCORRELATED — very slightly NEGATIVELY
correlated — with the honest sandbox distance on this function.** Measured over
all 30 seed-1 finds (`tmp/grind/func_80034F88/s4/eval.json`):

    perm 1070 -> sandbox 18      perm 1225 -> 25, 23
    perm 1100 -> 19              perm 1231 -> 20, 24
    perm 1170 -> 21, 20          perm 1251 -> 24
    perm 1200 -> 18              perm 1255 -> 43
    perm 1205 -> 20, 20          perm 1270 -> 20
    perm 1210 -> 21, 21, 21, 21  perm 1281 -> 19
    perm 1215 -> 32, 20, 20      perm 1290 -> 18, 18, 18, 17*
    perm 1220 -> 41              perm 1295 -> 20, 19
                                 perm 1300 -> 20, 45

The permuter's own BEST find (1070) scores 18; its WORST (1300) scores 20; the
numerically best sandbox result came from a mid-table 1290. So `--stop-on-zero`
can never fire on a form the engine would call closed, and the campaign is a
random SAMPLER of the neighbourhood rather than a descent of the metric that
counts. Consequence for every future permuter session on this function (and a
caution for any function where the two metrics diverge): **do not read the
permuter score as progress; re-score every find through
`sandbox <func> --disable all` and rank on that.** The instrument is
`tmp/grind/func_80034F88/s4/eval.py` (splice each `ws/output-*/source.c` body
into src/, sandbox it, restore src/) — it generalises to any function by
changing three constants at the top.

## The one sub-18 find is a MISCOMPILE, not a floor drop
Seed-1 `output-1290-4` scored **17 at 49 insns** — the only sub-floor number
either campaign produced. It is semantically wrong: the permuter hoisted the
third block's `D_80106A73 = val2;` store INSIDE the `if (!c)` arm, so with bit 4
of `p[8]` set the flag byte is never written at all, and with it clear the byte
is written with `val | 4` rather than `val`. The target stores unconditionally
on the join. It additionally carries a `long new_var = 4;` constant holder and
an `s32 *new_var3 = p;` pointer alias, both of which would need FAKE carve-outs
even had the semantics been right. Banked at
`rejected/permuter-17-store-hoisted-into-arm-MISCOMPILES.c` specifically so no
later session re-finds this 17 and credits it. **The sandbox scores bytes, not
behaviour — read every find for semantics before crediting its score.**

## The four remaining s3-F1 structural probes, all measured
`tmp/grind/func_80034F88/s4/score_files.py`, honest sandbox, floor form = 18/51:

| probe (s3's wording) | spelling | score / insns | verdict |
|---|---|---|---|
| "per-block temporaries (val_a/val_b/val_c)" | `va/vb/vc`, `ra/rb/rc`, `ca/cb/cc` | **30** / 50 | DEAD, -12 |
| "a second natural use of `ptr` late in the function" | copy loop as `*(ptr - 3 + i)` | **31** / 50 | DEAD, -13 |
| (milder form of the same) | third block stores `*ptr = val2;` | **22** / 50 | DEAD, -4 |
| "an explicit `(u8)`-cast staging variable for the truncation" | `val2 = (u8)val \| K;` in both arms | **18** / 51 | NEUTRAL |
| "swap which of c/val is computed adjacent to the delay slot" | block 1 reads the flag first, blocks 2-3 condition-first | **18** / 51 | NEUTRAL |

The two "second use of ptr" results are the informative ones and they point the
opposite way from s3's intuition: lengthening `ptr`'s live range does work
mechanically, and it is exactly what must NOT happen. Each extra use keeps the
one address pseudo live further down the function, which is precisely the case
where cse merges it and the target's per-block `lui`+`addiu` rematerialisations
disappear. The floor-18 form's `ptr` having exactly ONE (read-modify-write) use
is load-bearing in both directions: two memory operands so `combine` cannot fold
`%lo`, and no third operand so the pseudo dies before block 2.

## Reusable instruments left for s5
- `tmp/grind/func_80034F88/s4/mkws.sh` / `mkws2.sh` — build a decomp-permuter
  workspace for this function from a standalone chassis file: minimal-context
  `base.c` (six lines of typedefs/externs plus the function), a `compile.sh`
  that runs the REAL pipeline (cc1 with `-mel` + prologue_fix + maspsx +
  multu_pad, then extracts the `.ent`..`.end` region and assembles it at offset
  0), and a `target.o` built from `asm/funcs/func_80034F88.s` prefixed with a
  `.set gp=64`-stripped `prelude.inc`. **The minimal-context base was validated
  to reproduce the full-TU codegen instruction-for-instruction** (51 insns, same
  diff shape as the sandbox build), so permuter runs need not carry the
  4,000-line translation unit.
- `s4/eval.py` / `eval2.py` — honest-sandbox re-scoring of every `output-*`.
- `s4/score_files.py` — honest-sandbox score for any standalone chassis file.
- `s4/watch.sh` / `watch2.sh` — N blocking `permuter_campaign.py wait` calls in
  ONE turn (the turn-economy pattern; never hand-poll a campaign across turns).

- [s4] decomp-permuter run on func_80034F88 for the first time: 50,425 iterations across two structurally different chassis (51-insn `s32 val` seed, 34,003 iters, 30 finds; 49-insn `u8 val` seed, 16,422 iters, 67 finds). Every find re-scored with the honest sandbox. Best honest, semantically-correct find = 18 on both seeds, i.e. a tie with the seed and NO improvement on the floor. Seed 2 converged onto seed 1's basin (its 18-point finds are 51-insn `s32`-widened forms), so two distinct starting points reach the same floor.
- [s4] METHODOLOGICAL FINDING, reusable beyond this function: the permuter's weighted score is uncorrelated (slightly negatively correlated) with the engine's honest sandbox distance here. Its best find (perm 1070) scores sandbox 18 while its worst (perm 1300) scores 20, and the numerically best sandbox result came from a mid-table perm 1290. `--stop-on-zero` therefore can never fire on a form the engine would call closed, and the campaign is a random sampler of the neighbourhood rather than a descent. Every find must be re-scored through `sandbox --disable all` and ranked on that; `tmp/grind/func_80034F88/s4/eval.py` does it and generalises by changing three constants.
- [s4] The only sub-floor number either campaign produced (seed-1 output-1290-4, sandbox 17 at 49 insns) is a MISCOMPILE: the permuter hoisted the third block's `D_80106A73 = val2;` store inside the `if (!c)` arm, so the byte is never written when bit 4 of p[8] is set and is written with `val | 4` instead of `val` when it is clear. It also carries a constant-holder local and a pointer alias. Banked at rejected/permuter-17-store-hoisted-into-arm-MISCOMPILES.c so no later session credits the 17. The sandbox scores bytes, not behaviour.
- [s4] All four structural probes s3 left "untried" on frontier F1 are now measured and none improves the floor: per-block temporaries (va/vb/vc + ra/rb/rc + ca/cb/cc) = 30 at 50 insns; copy loop addressed through the existing pointer as `*(ptr - 3 + i)` = 31 at 50; third block storing via `*ptr = val2;` = 22 at 50; explicit `(u8)`-cast staging on the selected value = 18 (neutral); block 1 reading the flag before its condition while blocks 2-3 stay condition-first = 18 (neutral).
- [s4] The two "give `ptr` a second use" results invert s3's intuition and sharpen the mechanism: lengthening the pointer's live range is exactly what must NOT happen. Every extra use keeps the single address pseudo live further down the function, which is the case where cse merges it and the target's per-block lui+addiu rematerialisations vanish. The floor-18 form's `ptr` having exactly ONE read-modify-write use is load-bearing in both directions — two memory operands so combine cannot fold %lo, and no third operand so the pseudo dies before block 2.
- [s4] A minimal-context permuter base (six lines of typedefs/externs plus the function) was VALIDATED to reproduce the full-TU codegen instruction-for-instruction for this function, so permuter workspaces need not carry the 4,000-line translation unit. Builder: tmp/grind/func_80034F88/s4/mkws.sh (real pipeline in compile.sh: cc1 -mel + prologue_fix + maspsx + multu_pad, `.ent`..`.end` extraction, assembly at offset 0; target.o from asm/funcs/func_80034F88.s + a `.set gp=64`-stripped prelude.inc).

- [s4] Honest floor is UNCHANGED at 18 (`sandbox func_80034F88 --disable all`, 51 build insns vs a 49-insn target), with s3's candidate form re-installed in src/code6cac_b.c this session — the committed baseline that src/ carried at session start was the 24-point form with three forbidden `asm volatile("" ::: "memory")` scheduling barriers, and it has been replaced by the volatile-free 18.

- [s4] decomp-permuter has now been run on func_80034F88 for the first time in the function's history: 50,425 iterations across two structurally different chassis (51-insn `s32 val` seed = 34,003 iters / 30 finds; 49-insn `u8 val` seed = 16,422 iters / 67 finds). Both campaigns were waited for IN-TURN and `harvest --stop`ped before the session ended; `permuter_campaign.py status` confirms both PIDs dead and no campaign outlives the session.

- [s4] Every one of the 97 finds was re-scored with the honest engine sandbox rather than trusted at its permuter score. The best honest, semantically-correct find on either seed is 18 — a tie with the seed. No permuter find beats hand derivation on this function.

- [s4] The 49-insn `u8 val` chassis CONVERGED onto the 51-insn `s32 val` basin: several of its 18-point finds are 51-instruction s32-widened forms, i.e. the permuter independently rediscovered s3's Lever C from a different starting shape. Two structurally distinct chassis are one basin.

- [s4] METHODOLOGICAL, reusable beyond this function: the permuter's weighted score is uncorrelated (slightly negatively correlated) with the engine's honest sandbox distance here. Best permuter find (1070) = sandbox 18; worst (1300) = sandbox 20; best sandbox result came from a mid-table 1290. `--stop-on-zero` can never fire on a form the engine would call closed, so a campaign is a random sampler, not a descent. Rank finds on the sandbox; tmp/grind/func_80034F88/s4/eval.py does the re-scoring and generalises by changing three constants.

- [s4] The only sub-floor number either campaign produced (17 at 49 insns) is a MISCOMPILE — the third block's unconditional store hoisted inside the `if (!c)` arm — plus a constant-holder local and a pointer alias. Banked so no later session credits it. The sandbox scores bytes, not behaviour; every permuter find must be read for semantics first.

- [s4] All four structural probes s3 listed as untried on frontier F1 are now measured: per-block temporaries 30/50, copy loop via `ptr` 31/50, third-block store via `*ptr` 22/50, `(u8)`-cast staging 18/51 (neutral), block-1-read-first 18/51 (neutral). Frontier F1 as s3 wrote it is spent.

- [s4] The two 'give `ptr` a second use' results INVERT s3's stated intuition and sharpen the mechanism: lengthening the pointer's live range is exactly what must NOT happen, because every extra use keeps the single address pseudo live further down the function — the case where cse merges it and the target's per-block lui+addiu rematerialisations vanish. The floor form's ONE read-modify-write use of `ptr` is load-bearing in both directions (two memory operands so combine cannot fold %lo; no third operand so the pseudo dies before block 2). Do not 'clean up' that pointer and do not give it another use.

- [s4] A minimal-context permuter base (six lines of typedefs/externs plus the function) was VALIDATED to reproduce the full-TU codegen instruction-for-instruction for this function, so permuter workspaces need not carry the 4,000-line translation unit. Builder tmp/grind/func_80034F88/s4/mkws.sh runs the real pipeline in compile.sh (cc1 with -mel, prologue_fix, maspsx, multu_pad), extracts the `.ent`..`.end` region and assembles it at offset 0, and builds target.o from asm/funcs/func_80034F88.s prefixed with a `.set gp=64`-stripped prelude.inc.

- [s4] No cheat construct was proposed or installed this session. The permuter's coercion-shaped proposals (constant-holder `new_var` locals, pointer aliases, a `volatile int pad;`, dead `val2 = val | 2;` stores, a stray `do { } while (0);`) were rejected on sight as unsanctioned-without-exhaustion proposals rather than surfaced; none of them beat 18 on the honest sandbox anyway.

## s5 (permuter modality) — the DIRECTED permuter axis, measured and closed

s4 killed the RANDOM permuter (50,425 iterations, two chassis, best find ties
the floor at 18) and left frontier F2: "directed permuter (`PERM_*` macros) can
still contribute as an exhaustive ENUMERATOR of a specific axis even though
random permuter is dead." s5 executed exactly that, twice, and additionally ran
the same two cross-products deterministically through the honest sandbox so the
axis is CLOSED rather than sampled. Nothing improved on 18; the floor form is
unchanged; src/code6cac_b.c carries the floor-18 candidate.

### How the directed campaigns were run
`decomp-permuter` in MANUAL-MUTATION mode: a base with multi-choice
`PERM_GENERAL(...)` macros and no `PERM_RANDOMIZE`, which disables random
mutation entirely and makes the permuter enumerate the declared cross-product.
Both campaigns went through `tools/permuter_campaign.py launch/wait/harvest
--stop` (telemetry per the 2026-07-07 owner directive) and both TERMINATED BY
THEMSELVES on space exhaustion — the permuter prints "Will run for N
iterations" where N is the size of the declared space, so a directed campaign
has a natural, provable end rather than a fresh-seed stopping heuristic.

| campaign | ws | space | permuter iters | outputs |
|---|---|---|---|---|
| `directed-perm-general` | `s5/ws` | 864 (ordering / select / call order / loop) | 864 | **0** |
| `directed-types-condform` | `s5/ws2` | 144 (types / mask / condition form) | 288 | 1 (`output-1290-1`) |

The single output scored 1290 against a base of 1300 on the permuter's weighted
metric — i.e. the permuter ranked it BETTER than the floor chassis. Re-scored
with `sandbox func_80034F88 --disable all` it is **18: a tie**. Third
independent confirmation of s4-H1 (the permuter metric is not a gradient here).
Banked at `rejected/directed-perm-mixed-condform-score18-TIE.c`.

### Wave A — 864 forms, exhaustively sandbox-scored (`s5/sweep5.py`)
Axes, all INDEPENDENT per flag block (s1-s4 only ever measured UNIFORM
spellings plus one block-1-only variant):
call/ptr order (2) x per-block condition-read-vs-flag-read order (2^3) x
per-block select form neg-if/pos-if/ternary (3^3) x copy-loop spelling (2).

Score distribution over all 864: 18 x8, 19 x8, 20 x24, 21 x56, 22 x48, 23 x88,
24 x80, 25 x56, 26 x48, 27 x16, tail to 38. **Best = 18, floor unchanged.**

Four facts fall straight out of the full grid:

1. **Per-block statement order is COMPLETELY NEUTRAL at floor 18.** The eight
   18-point forms are exactly `call=pc`, `sels=nnn`, `loop=i17` crossed with ALL
   EIGHT per-block orderings. s3's "condition before the flag read" lever was
   worth 1 point at floor 23 and is worth ZERO now, in every per-block
   combination — the lever was absorbed by the `s32 val` widening, not additive
   with it. Any future session may spell that ordering whichever way reads best.
2. **`ptr = &D_80106A73;` must come AFTER `p = func_80077D00();`** — the minimum
   over all 432 forms with the assignment before the call is **29**, +11 over
   the floor. The address pseudo has to be born after the call insn or the whole
   flag section reshapes. This axis had never been probed at all.
3. **The select form is additive and position-weighted, never mixable for gain.**
   Minimum by select triple: nnn 18, pnn 19, npn 20, nnp 20, ntn/tnn/ttn 21,
   ppn/nnt 21, ... ttt 24. A positive-sense `if` costs +1 in block 1 and +2 in
   blocks 2-3; a ternary costs +3 anywhere. Uniform-negated is strictly optimal
   and no mixture recovers anything.
4. **The copy-loop spelling is exactly orthogonal.** Over all 432 matched pairs,
   `*((u8 *)p + 0x17 + i)` scores EXACTLY +2 versus `*((u8 *)p + i + 0x17)` —
   delta set = {2}, no exceptions. s3's lever is fully independent of every flag
   block spelling, which is why it survived every later change.

### Wave B — 144 forms, exhaustively sandbox-scored (`s5/sweep6.py`)
val type (s32/u8/u32) x val2 type (u8/s32/u32) x c type (s32/u8) x c
cardinality (one shared `c` vs three used `c1/c2/c3`) x mask spelling
(`*ptr &= 0xF8;` vs `*ptr = *ptr & 0xF8;`) x condition form
(`c = p[8] & K; if (!c)` vs `c = p[8]; if (!(c & K))`).

Distribution: 18 x4, 19 x28, 20 x14, 21 x98. **Best = 18, floor unchanged.**

1. **`u32 val` ties `s32 val` at 18**; `u8 val` bottoms at 20. What matters is
   that the loaded byte is read into a WORD-sized temporary (the
   `(zero_extend:SI (mem:QI))` of s3-H5), not its signedness.
2. **The mask spelling is neutral**: compound `*ptr &= 0xF8;` and expanded
   `*ptr = *ptr & 0xF8;` both reach 18. This is a real result about lever 1 of
   the candidate: what defeats `combine`'s `%lo` fold is the address expression
   having TWO memory operands, not the compound-assignment syntax.
3. **`u8 c` bottoms at 19, `s32 c` at 18** — the condition temporary must be
   word-sized too.
4. **Splitting `c` into three used locals `c1/c2/c3` bottoms at 19.** s4's
   per-block probe split all three temporaries at once (30) and never isolated
   the condition; isolated, the split costs exactly 1. So the shared `c` is
   load-bearing but only mildly, and the 30 of s4's probe was dominated by
   splitting `val`/`val2`.
5. **Masking inside the `if` (`c = p[8]; if (!(c & K))`) costs 1 and one
   instruction** (19 at 52 insns) — GCC keeps the raw word live across the
   flag-byte read instead of the already-masked condition.

### What s5 spent, and what remains
Directed enumeration: 1,008 distinct forms scored on the honest metric
(864 + 144), plus 1,152 permuter iterations across the same two spaces. Added
to s1-s4 that is roughly 1,070 hand/enumerated spellings and 51,577 permuter
iterations, all bottoming at 18. The permuter axis — random (s4) AND directed
(s5) — is now closed in both of its modes.

The frontier that survives is unchanged and is NOT a permuter question: the
forensic `-da` read of the floor-18 form itself (`.greg` register dispositions
and the conflict list for the pseudo holding `val2`), which is the only
remaining instrument that would explain the v0/v1 swap mechanically instead of
by spelling. s5 deliberately did not spend its budget there — that is a
forensics-modality probe and s5's mandate was permuter.

- [s5] Honest floor is UNCHANGED at 18 (51 build insns vs a 49-insn target); src/code6cac_b.c carries the floor-18 candidate body and `sandbox func_80034F88 --disable all` printed score 18 with it in place at the end of the session.

- [s5] Directed permuter was run on this function for the first time: manual-mutation mode (multi-choice PERM_GENERAL macros, no PERM_RANDOMIZE) makes decomp-permuter enumerate the declared cross-product and TERMINATE BY ITSELF on exhaustion — so a directed campaign has a provable end rather than needing the fresh-seed stopping heuristic. Campaign 1 ran its full 864 iterations in ~30 s; campaign 2 ran 288 iterations in ~44 s. Both harvest --stopped; `permuter_campaign.py status` reports 0 live campaigns and no permuter processes remain.

- [s5] The directed campaigns produced ONE output in total: ws2/output-1290-1, permuter weighted score 1290 against a base of 1300 — the permuter's own metric ranked it BETTER than the floor chassis. Re-scored honestly it is 18, a tie. Third independent confirmation of s4-H1 that the permuter metric is uncorrelated with the sandbox here.

- [s5] 1,008 distinct forms were enumerated deterministically and scored with the honest sandbox this session (864 in wave A, 144 in wave B); cumulative across s1-s5 that is roughly 1,070 measured spellings plus 51,577 permuter iterations, every one bottoming at 18.

- [s5] Wave A score distribution over all 864 forms: 18 x8, 19 x8, 20 x24, 21 x56, 22 x48, 23 x88, 24 x80, 25 x56, 26 x48, 27 x16, tail to 38. Wave B over 144: 18 x4, 19 x28, 20 x14, 21 x98.

- [s5] The eight 18-point wave-A forms are exactly `call-first` x `all-negated-if selects` x `p + i + 0x17` loop crossed with all eight per-block condition/read orderings — so the per-block ordering axis is completely neutral at the floor, and s3's 1-point ordering lever was absorbed by the s32-val widening rather than being additive with it.

- [s5] Assigning `ptr = &D_80106A73;` BEFORE the `func_80077D00()` call costs a minimum of +11 (best 29 over all 432 such forms) — the largest single-axis penalty measured on this function, and an axis never probed in s1-s4.

- [s5] Select-form penalties are additive and position-weighted, never mixable for gain: minimum by triple is nnn 18, pnn 19, npn/nnp 20, nnt/ntn/tnn/ppn/ttn 21, ... ttt 24. A positive-sense `if` costs +1 in block 1 and +2 in blocks 2-3; a ternary costs +3 anywhere.

- [s5] The copy-loop lever is exactly orthogonal: `+ 0x17 + i` costs exactly +2 versus `+ i + 0x17` in all 432 matched pairs, delta set {2}.

- [s5] `u32 val` ties `s32 val` at 18 while `u8 val` is 20 — the loaded byte's lever is WORD WIDTH (the (zero_extend:SI (mem:QI))), not signedness. `u8 val2` is required (s32/u32 val2 = 20), `s32 c` is required (`u8 c` = 19), one shared `c` is required (three used c1/c2/c3 = 19), and pre-masking the condition is required (`c = p[8]; if (!(c & K))` = 19 at 52 insns).

- [s5] `*ptr &= 0xF8;` and `*ptr = *ptr & 0xF8;` both score 18 — what defeats combine's %lo fold is the address expression carrying TWO memory operands, not the compound-assignment syntax. This refines lever 1 of the candidate without changing it.

- [s5] No cheat-family construct was written, proposed or measured this session: every one of the 1,008 forms is ordinary C (statement order, select spelling, integer types, local cardinality), and the single permuter output was vetted and rejected as a strictly-worse tie rather than surfaced.

---

# s6 — forensics modality (floor unchanged at 18; the RTL model that s1-s5 were
# grinding against is WRONG in two specific, load-bearing ways)

## Headline
The first cc1 `-da` dump ever taken OF THE FLOOR-18 FORM ITSELF (s2's dumps were
of a pre-lever volatile variant that no longer resembles the body) refutes two
mechanisms that s1-s5 all reasoned from, and replaces them with what the dumps
actually show. The floor did not move, and no spelling was found below 18, but
the search space this closes is larger than any single spelling: the "combine
folds %lo" model and the "the v0/v1 swap is an RA problem you can steer" model
are both dead.

## CORRECTION 1 — nothing "folds %lo". RTL EXPANSION emits `(mem (symbol_ref))`.
s1 (evidence "Residual defect 1"), s3 (candidate.c lever 1) and s4 all state that
`combine` folds `%lo` into an address expression that feeds exactly ONE memory
operand, and that this is why the unfolded `lui`+`addiu` base disappears. That is
not what happens. In the PRE-CSE dump (`s6/rtl/floor18/fn/rtl.fn`) every plain
`D_80106A73` access is ALREADY `(mem:QI (symbol_ref:SI ("D_80106A73")))` —
lines 52, 87, 102, 137, 152, 187 — while the two `*ptr` accesses are ALREADY
`(mem:QI (reg/v:SI 73))` — lines 29, 37. The address form is decided by RTL
EXPANSION (`expand_expr` on the C expression), because the MIPS backend's
`GO_IF_LEGITIMATE_ADDRESS` accepts a bare `symbol_ref` as an address; the
`lui $at` / `%lo(...)($at)` pair is manufactured downstream at ASSEMBLY OUTPUT
time, not by any optimizer pass. No GCC pass ever converts a register base into a
`%lo` operand, and no pass can be defeated into keeping one.

Consequence for the C: the address form of every single access is a DIRECT,
one-to-one function of how that access is spelled in the source —
`*ptr`/`ptr[0]` gives `(mem (reg))`, `D_80106A73` gives `(mem (symbol_ref))`.
There is no optimizer decision in between to steer, and therefore no lever on
this axis beyond choosing the spelling per access. Target's four flag-byte
accesses are all `0(reg)`, so ALL FOUR were spelled through a pointer in the
original source.

## CORRECTION 2 — the store→load forward is an ADDRESS-RTX identity test in cse,
## and the two address forms never hash together.
`s6/rtl/floor18/fn/cse.fn` shows exactly which reads survive:
  - block 1's read (line 64) survives as `(mem:QI (symbol_ref))` → a real `lbu`.
    The preceding store was `(set (mem:QI (reg 73)) ...)` (line 49) — a DIFFERENT
    address rtx, so `cse_insn` never finds the stored value.
  - blocks 2 and 3's reads (lines 114, 160) are GONE, replaced by
    `(set (reg/v:SI 74) (zero_extend:SI (reg/v:QI 75)))` — the value that was
    just stored. Their stores (lines 99, 145) are `(mem:QI (symbol_ref))`, the
    IDENTICAL rtx, so the forward fires.
  Those two surviving `zero_extend`s from a register are the two `andi a0,v1,0xff`
  truncations, i.e. they are ALSO the entire 51-vs-49 instruction-count excess.
So the dichotomy s3 stated is confirmed, but its cause is `canon_hash` seeing two
structurally different address rtxes — NOT `cse.c:7329` declining to record a
SET_DEST. The 7329 path (`sets[i].src_elt == 0`) is only reached for a volatile
MEM; for these non-volatile stores the destination IS recorded and the forward is
an ordinary hash hit.

## CORRECTION 3 — the v0/v1 swap is NOT steerable by live-range surgery.
The frontier s5 handed to this session was: "the `.greg` dispositions and conflict
list will say WHY the pseudo holding `val2` lands in v1 where target has v0."
Answer, from `s6/rtl/floor18/fn/greg.fn`:

    ;; 8 regs to allocate: 77 75 74 76 82 86 90 72
    ;; 75 conflicts: 72 74 75 76 82 86 90 2 29     <- val2, conflicts HARD REG 2
    ;; 76 conflicts: 72 74 75 76 82 86 90 29       <- c, no hard-reg-2 conflict
    ;; 76 preferences: 2
    ;; Register dispositions: ... 75 in 3   76 in 2 ...

`find_reg` (global.c) hands `$v0` to the condition pseudo 76 — which carries an
explicit `preferences: 2` — and cannot hand it to 75 because 75's conflict set
contains hard reg 2. The `$v0`/`$v1` swap is therefore a HARD-REG conflict, not a
tie-break, not a priority ordering, and not a consequence of 75 conflicting with
76 (the ternary and if/else forms REMOVE the 75-76 conflict and still get `$v1`).

Where hard reg 2 comes from is visible one pass earlier: `local-alloc` assigns the
block-local `p[8]` load pseudos to `$v0` before `global-alloc` ever runs —
`s6/rtl/floor18/fn/lreg.fn` ";; Register 80 in 2. ... 84 in 2. 88 in 2. 92 in 2."
Every pseudo whose allocno survives into `global_alloc` alongside those blocks
inherits the hard-reg-2 conflict.

Six independent forms were built and dumped to test whether ANY live-range
surgery removes that conflict. None does, and every one is worse than the floor:

| form | sandbox / insns | what its .greg showed |
|---|---|---|
| floor-18 control | **18** / 51 | 75 in $v1, conflicts {…,2,29} |
| if/else both arms | 24 / 51 | 75-76 conflict GONE, hard-reg-2 conflict remains, still $v1 |
| ternary + `(u8)` casts | 24 / 51 | identical to if/else |
| positive-sense if | 23 / 49 | unchanged picture |
| store duplicated into arms | 35 / 61 | `j` around the else arm + two symbol-addressed stores per block |
| `val2` split per block | 24 / 49 | all three val2 pseudos still conflict with 2; value moves to `$a0` |
| `val` split per block | 23 / 51 | loaded-byte pseudos gain `preferences: 3`; value to `$a0` |
| `val` + `val2` split | 29 / 49 | monotonic with the above; brackets s4's 30 |

`val2`-only and `val`-only splits had never been isolated (s4 split all three
temporaries at once = 30; s5 isolated only `c` = 19). Both are now measured.

## What this leaves
Every one of the three residual defect classes — the missing `lbu` reloads, the
two extra `andi` truncations, and the `$v0`/`$v1` swap — is now traced to ONE
source decision: **whether each flag-byte access is spelled through a pointer or
through the symbol**, decided at RTL expansion with no intervening pass. Target's
bytes require all four accesses to be pointer-spelled AND require the block-2/3
reads NOT to be forwarded from the block-1/2 stores. Under non-volatile C those
two requirements are contradictory, because two pointer pseudos holding the same
`symbol_ref` are merged by `canon_reg` into one address rtx and the forward fires
(measured by s2 and s3 at 22 for the three-pointer form).

That contradiction is now stated at the level of the RTL the compiler actually
builds, rather than at the level of an optimizer pass that was never involved.

- [s6] Honest floor UNCHANGED at 18 (`sandbox func_80034F88 --disable all`, 51 build insns vs 49 target insns) with the s3/s4/s5 candidate body re-installed in src/code6cac_b.c; the committed baseline that src/ carried at session start was again the 24-point form with three forbidden `asm volatile("" ::: "memory")` scheduling barriers.
- [s6] FIRST cc1 -da dump of the FLOOR-18 form itself (s2's only dumps were of a pre-lever volatile variant). Artifacts: tmp/grind/func_80034F88/s6/rtl/floor18/ plus per-variant dumps; the per-function slices are in each rtl/<tag>/fn/*.fn (rtl, cse, combine, lreg, greg, sched, jump2, dbr).
- [s6] CORRECTION to s1/s3/s4, load-bearing: nothing "folds %lo". The pre-cse dump (rtl.fn lines 52/87/102/137/152/187) shows every plain `D_80106A73` access is ALREADY `(mem:QI (symbol_ref))` straight out of RTL expansion, and the `*ptr` accesses are ALREADY `(mem:QI (reg 73))` (lines 29/37). MIPS `GO_IF_LEGITIMATE_ADDRESS` accepts a bare symbol_ref, so the `lui $at` + `%lo(...)($at)` pair is manufactured at ASSEMBLY OUTPUT time. `combine` is not involved; there is no optimizer decision on this axis to defeat. The address form of each access is a direct function of how that access is spelled in C.
- [s6] CORRECTION to s2/s3: the store-to-load forward that eats the block-2 and block-3 reloads is an ORDINARY cse hash hit on identical address rtxes, not the `cse.c:7329` `src_elt == 0` path (which is reachable only for a volatile MEM). cse.fn shows block 1's read surviving as `(mem:QI (symbol_ref))` because the preceding store was through `(mem:QI (reg 73))`, and blocks 2/3's reads replaced by `(zero_extend:SI (reg/v:QI 75))` because their stores were the identical symbol_ref rtx.
- [s6] The two surviving `zero_extend`-from-register insns ARE the two `andi a0,v1,0xff` truncations in the build, i.e. the forward is also the entire 51-vs-49 instruction-count excess. Killing the forward and getting target's insn count are the same problem, not two.
- [s6] The s5 frontier question is ANSWERED: `val2` lands in $v1 because its allocno's conflict set contains HARD REG 2 (greg.fn `;; 75 conflicts: 72 74 75 76 82 86 90 2 29`), while the condition pseudo 76 carries an explicit `preferences: 2` and no hard-reg-2 conflict, so global.c's find_reg gives it $v0. The hard reg comes from `local-alloc`, which assigns the block-local `p[8]` load pseudos to $v0 before global_alloc runs (lreg.fn `;; Register 80 in 2. ... 84 in 2. 88 in 2. 92 in 2.`).
- [s6] The v0/v1 swap is NOT a 75-vs-76 tie: the if/else-both-arms and ternary forms REMOVE the 75-76 conflict entirely (`75 conflicts: 72 74 75 2 29`) and `val2` still gets $v1. Only the hard-reg-2 conflict binds.
- [s6] Live-range surgery on the temporaries is measured DEAD as a register lever, in six forms, none of which removes the hard-reg-2 conflict: if/else both arms 24/51, ternary with explicit (u8) casts 24/51, positive-sense if 23/49, store duplicated into both arms 35/61 (GCC emits a `j` around the else arm plus two symbol-addressed stores per block), val2-split-only 24/49, val-split-only 23/51, val-and-val2-split 29/49. Splitting val2 or val ALONE had never been isolated before (s4 split all three temporaries at once = 30; s5 isolated only `c` = 19).
- [s6] Reusable instruments: tmp/grind/func_80034F88/s6/dump.sh (cc1 -da on the real build flags for the current src/), slice.py (cut the func_80034F88 region out of every pass dump into rtl/<tag>/fn/*.fn — the ONLY safe way to read these dumps, the raw ones are whole-TU), forensic.py (splice a variant -> honest sandbox -> cc1 -da -> print the .greg allocation header + the flag-block objdump -> restore src/), sbs.py (build-vs-target instruction listing from the sandbox object with no src mutation), gen.py / gen2.py (variant generators), bank.py.
- [s6] Nothing cheat-shaped was written, proposed or measured this session: all seven variants are ordinary C (select form, temporary cardinality), and the session's product is RTL evidence plus seven banked negatives.

- [s6] Honest floor UNCHANGED at 18 (`sandbox func_80034F88 --disable all`, 51 build insns vs a 49-insn target), with the s3/s4/s5 candidate body re-installed in src/code6cac_b.c and re-scored at 18 at the end of the session. The committed baseline that src/ carried at session start was AGAIN the 24-point form with three forbidden `asm volatile("" ::: "memory")` scheduling barriers — every session inherits that and must re-apply candidate.c first.

- [s6] This is the first cc1 -da dump ever taken of the FLOOR-18 form; s2's only dumps (s2/rtl/v10/) are of a pre-lever volatile variant that no longer resembles the body, which is why two wrong mechanisms survived three sessions.

- [s6] CORRECTION to s1/s3/s4: nothing 'folds %lo'. RTL EXPANSION emits `(mem:QI (symbol_ref "D_80106A73"))` for a plain symbol access and `(mem:QI (reg 73))` for a `*ptr` access; the `lui $at` + `%lo(...)($at)` pair is manufactured at assembly-output time because the MIPS backend accepts a bare symbol_ref as a legitimate address. There is no optimizer decision on this axis to steer.

- [s6] CORRECTION to s2/s3: the store-to-load forward that eats the block-2/3 reloads is an ordinary cse hash hit on identical address rtxes, NOT the cse.c:7329 src_elt == 0 path (which remains volatile-only, so s3's volatile ruling stands untouched).

- [s6] The two `zero_extend`-from-register insns that replace the forwarded reloads ARE the two `andi a0,v1,0xff` truncations in the build, so killing the forward and reaching the target's 49-instruction count are the same problem rather than two.

- [s6] The s5 frontier question is answered mechanically: `val2` lands in $v1 because its allocno's conflict set contains hard reg 2 (greg.fn `;; 75 conflicts: 72 74 75 76 82 86 90 2 29`), while the condition pseudo 76 carries `preferences: 2` and no hard-reg-2 conflict. local-alloc gave $v0 to the block-local p[8] load pseudos before global_alloc ran (lreg.fn `;; Register 80 in 2. ... 84 in 2. 88 in 2. 92 in 2.`).

- [s6] Live-range surgery on the temporaries is measured DEAD as a register lever in seven forms, none of which removes the hard-reg-2 conflict: if/else both arms 24/51, ternary with explicit (u8) casts 24/51, positive-sense if 23/49, store duplicated into both arms 35/61 (GCC emits a `j` around the else arm plus two symbol-addressed stores per block — the sanctioned duplicated-statement-into-arms shape is the WORST result of the session), val2-split-only 24/49, val-split-only 23/51, val-and-val2-split 29/49.

- [s6] val2-only and val-only splits had never been isolated before: s4 split all three temporaries at once (30) and s5 isolated only `c` (19). Both are now measured and both are worse than the shared form, so the whole live-range-cardinality axis is monotonically worse than 18.

- [s6] All three residual defect classes (missing lbu reloads, two extra andi truncations, $v0/$v1 swap) are now traced to ONE source decision: pointer-vs-symbol spelling per access, fixed at RTL expansion. Target's bytes need all four flag-byte accesses pointer-spelled AND need the block-2/3 reads not forwarded from the block-1/2 stores; under non-volatile C those are contradictory because two pointer pseudos holding the same symbol_ref are merged by canon_reg into one address rtx (measured by s2/s3 at 22 for the three-pointer form).

- [s6] Nothing cheat-shaped was written, proposed or measured this session: all seven variants are ordinary C differing only in select form and temporary cardinality; the session's product is RTL evidence plus seven banked negatives.

- [s6] New reusable instruments: s6/dump.sh (cc1 -da on the real build flags against the CURRENT src/), s6/slice.py (cut the func_80034F88 region out of every pass dump into rtl/<tag>/fn/*.fn — the only safe way to read these dumps, since the raw ones are whole-TU), s6/forensic.py (splice a variant -> honest sandbox -> cc1 -da -> print the .greg allocation header + the flag-block objdump -> restore src/), s6/sbs.py (build-vs-target instruction listing from the sandbox object with no src mutation), s6/gen.py + gen2.py + bank.py.


---

## s7 — forensics modality (cse basic-block boundaries and qty classes)

s7 took the two axes s6 left on the frontier and closed both. It also produced
the single most consequential measurement made on this function so far: a
NON-VOLATILE C form whose flag section carries the target's exact memory-access
and address signature.

### The measured wave (16 forms, all `sandbox func_80034F88 --disable all`)
| form | score | insns | lbu | sb | lui |
|---|---|---|---|---|---|
| v1 floor-18 chassis (control) | 18 | 51 | 3 | 5 | 6 |
| v2 do-while(0) + three pointer locals | 23 | 47 | 2 | 5 | 2 |
| v3 do-while(0) + one pointer local | 23 | 47 | 2 | 5 | 2 |
| v4 three pointer locals, no boundary | 21 | 47 | 2 | 5 | 2 |
| v5 three distinct symbol+addend pointers, all hoisted | 26 | 49 | 4 | 5 | 4 |
| v6 three distinct symbol+addend pointers, staged per block | 21 | 49 | 4 | 5 | 4 |
| v7 copy-chained pointers (`ptr2 = ptr1;`) | 21 | 47 | 2 | 5 | 2 |
| v8 floor chassis + do-while(0) per block | 20 | 51 | 3 | 5 | 6 |
| v9 bit-index loop, pointer accesses | 31 | 36 | 3 | 3 | 2 |
| v10 bit-index loop, symbol accesses | 31 | 37 | 3 | 3 | 4 |
| w1 four distinct symbol+addend pointers | 28 | 55 | 5 | 5 | 5 |
| w2 floor chassis + distinct pointers for blocks 2/3 | **18 (TIE)** | 51 | 5 | 5 | 6 |
| w3 as w2 but stores through the plain symbol | 20 | 51 | 5 | 5 | 8 |
| p1 one-iteration `for` per block + three pointer locals | 36 | 58 | **5** | **5** | **4** |
| (target) | 0 | 49 | 5 | 5 | 4 |

### Finding 1 — the boundary that works is a CODE_LABEL, and only a CODE_LABEL
`cse_end_of_basic_block` (cse.c:8039) terminates the block at a CODE_LABEL
**unconditionally**, so a label boundary is honoured by both cse passes. The
NOTE_INSN_LOOP_END break at cse.c:8054 is guarded by `! after_loop`, and the
comment there is explicit: "If we are running after loop.c has finished, we can
ignore the NOTE_INSN_LOOP_END." cse2 runs after loop.c.
This is not inference — it is in the dumps. `s7/rtl/v3_dw_one_ptr/fn/cse.fn`
still contains `(set (reg 85) (mem:QI (reg/v:SI 73)))` for blocks 2 and 3 after
the FIRST cse pass (the reloads survive), and `.../cse2.fn` no longer contains
them (the second pass merged the flag section back into one basic block and
forwarded every store into the following load). The do-while(0) family — and
any construct whose only cse effect is a loop note — is therefore dead on this
function for a mechanism reason, not a spelling reason. s2 measured it dead
(26/24/30); s7 explains why no respelling of it can work.

### Finding 2 — a surviving boundary reproduces the target's signature exactly
`p1_loop1_per_block` (each flag block wrapped in `for (j = 0; j < 1; j++)`, three
pointer locals) produces **lbu 5 / sb 5 / lui 4 — the target's counts** — with
three `lui %hi(D_80106A73)` + `addiu ...,%lo(D_80106A73)` bases at addend 0 and
a real `lbu 0(base)` for every flag read. Both target properties that s1-s6
recorded as mutually exclusive in non-volatile C appear together, because a
fresh cse basic block flushes the value table and therefore (a) the store cannot
be forwarded into the next block's read and (b) the next `ptr = &D_80106A73;` is
no longer a redundant set, so it emits its own address materialisation.
The cost is entirely the loop scaffolding: `addiu a1,a1,1` / `blez a1,...` /
`move a1,zero` per block, 9 insns, 58 against target's 49, score 36.
`for (i = 0; i < 1; i++)` is explicitly named as a NOT-sanctioned spelling by the
non-extension clause of `.claude/rules/no-new-park-categories.md`, so this form
is an instrument, never a submission.

### Finding 3 — the qty-class-breaking family works and cannot beat 18
`insert_regs` (cse.c:1006-1042) merges two address pseudos into one quantity
only when their SET_SRC rtxs hash equal, and `canon_reg` (cse.c:2532-2574) then
rewrites the later pseudo to the older one, which is what makes the store and
the read share an address rtx and forward. Spelling the second and third bases
as `(u8 *)&D_80106A70 + 3` / `(u8 *)&D_80106A71 + 2` gives
`(const (plus (symbol_ref ...) (const_int N)))`, which does not hash equal to
`(symbol_ref "D_80106A73")` — the pseudos stay in separate quantities
(`s7/rtl/v6_distinct_syms_staged/fn/cse.fn` shows regs 73/74/75 alive with three
different address values) and every reload survives.
It never beats the floor: w2 ties at 18 with the target's exact access counts,
and the side-by-side shows why — blocks 2 and 3 now match the target
instruction-for-instruction except for register names and the LO16 addend
(`addiu a2,a2,3` against `addiu a0,a0,0`). The two non-zero addends cost exactly
what the two recovered reloads gain. An addend-0 spelling of the same mechanism
would score below 18, but that requires a SECOND declared symbol at 0x80106A73,
which is the forbidden alias-rename family. Copy-chained pointers (`ptr2 = ptr1`)
merge as predicted (21, lui 2), confirming the reading of insert_regs.

### Finding 4 — the loop SHAPE is refuted by the bytes
The flag section as a loop over the bit index (the last untried shape) emits a
real 36-insn loop: GCC 2.7.2 at -O2 without `-funroll-loops` neither unrolls nor
peels. 31 in both the pointer and symbol spellings. Refuted by the objdump
rather than by the score.

### What this does to the F3 escalation packet
The packet's central claim must be REWRITTEN before it is ever filed. It is no
longer true that "honest non-volatile pure C cannot produce the target's four
0(reg) reloads" — p1 produces them, plus the three addend-0 bases, without any
volatile. The correct, much narrower claim is:
> every construct measured so far that creates a cse basic-block boundary
> surviving cse2 also emits instructions of its own, and the target has no
> instructions to spare.
That is a search question about C control flow, not a proof of impossibility,
and it is the frontier this session hands to the next one.

- [s7] Honest floor unchanged at 18 (v1 control re-measured this session: 18 / 51 insns). 16 forms scored with `sandbox func_80034F88 --disable all`; full table in evidence.md.

- [s7] cse.c:8039 ends a basic block at a CODE_LABEL unconditionally, so a label boundary is honoured by BOTH cse passes; the NOTE_INSN_LOOP_END break at cse.c:8054 is guarded by `! after_loop`, so loop-note boundaries work in cse1 and are erased by cse2 (dump-proven: v3's cse.fn keeps the block-2/3 reloads, cse2.fn does not).

- [s7] p1_loop1_per_block reaches lbu 5 / sb 5 / lui 4 — the target's exact memory-access and address signature — in non-volatile C, with three addend-0 %hi/%lo(D_80106A73) bases and a real lbu 0(base) per flag read. Cost: 9 insns of loop scaffolding (58 vs 49), score 36.

- [s7] The qty-class-breaking family works exactly as insert_regs (cse.c:1006-1042) + canon_reg (cse.c:2532-2574) predict: identical SET_SRC rtxs merge (v4 21, v7 copy-chain 21, both lui 2 / lbu 2), distinct symbol+addend rtxs do not (v5 26, v6 21, both lui 4 / lbu 4; regs 73/74/75 alive in v6's cse.fn).

- [s7] Ceiling of the qty-class-breaking family is a TIE at 18 (w2, lbu 5 / sb 5), never an improvement: its blocks 2 and 3 match target instruction-for-instruction except register names and the LO16 addend, and the two non-zero addends cost exactly what the two recovered reloads gain. Four distinct bases (w1) score 28.

- [s7] An addend-0 distinct address rtx for the same byte would beat 18, but it requires a SECOND declared symbol at 0x80106A73, which is the forbidden alias-rename family — not an available axis.

- [s7] The bit-index loop shape is refuted by the bytes, not the score: GCC 2.7.2 at -O2 without -funroll-loops emits a real 36-insn loop (v9 31, v10 31) against a 49-insn unrolled target.

- [s7] The F3 escalation packet's central claim ('honest non-volatile pure C cannot produce the target's four 0(reg) reloads') is now FALSE and must be rewritten before it is ever filed; the surviving claim is only that every cse2-surviving boundary construct measured so far emits instructions the target has no room for.

- [s7] src/code6cac_b.c was left byte-unchanged (every probe splices and restores); the best form remains the floor-18 body in memory/grind/func_80034F88/candidate.c, whose header now carries the s7 correction.


---

## s8 (rederive) - what changed in the model

**The floor did not move: 18, the same body as s3-s7.** What moved is the
mechanism model, in one important direction and one closing direction.

**1. A second, cheaper reload mechanism exists, and it is not a label
boundary.** s7 concluded that only a cse2-surviving CODE_LABEL can stop the
store-to-load forward. That is now known to be incomplete. `cse.c`'s
`invalidate_skipped_block` (7843-7867) / `invalidate_skipped_set` (7810-7836)
INVALIDATE any MEM written inside a conditionally-skipped block instead of
recording it. So a store spelled inside a conditional arm -
`if (p[8] & K) *ptr |= K;` - makes every following read of the byte a real
`lbu`, in non-volatile C, with no label, no loop note and no extra
instruction. Measured directly: `m1_ptr_read_condstore` scores 18 at 42
instructions with `lbu 4 / sb 5`, and its `cse2.fn` still contains
`(mem:QI (reg/v:SI 73))` for every flag read
(`tmp/grind/func_80034F88/s8/rtl/m1_ptr_read_condstore/fn/cse2.fn`).

**2. And it cannot be used here, for a reason internal to the target.** The
target's store is UNCONDITIONAL: `asm/funcs/func_80034F88.s` has
`addu $v0,$a0,$zero` at 0x80034FC4 - the else-value of a select computed on
the fall-through path - and exactly one `sb` per block, placed AFTER the join
label. The AROUND mechanism requires the store to be INSIDE the arm. A
conditional-store body is therefore 7 instructions short of the target by
construction: 3 missing value-select moves plus 4 missing
address-materialisation instructions. Every hybrid that restores part of the
target's shape (block 1 floor + blocks 2/3 conditional, symbol/pointer
crossings, symbol-spelled mask) scores 21-33, all worse.

**3. The two open sub-problems are ONE sub-problem.** s7 listed the missing
reloads and the three addend-0 address bases as separate frontier items. They
are the same item: with three pointer locals staged exactly as the target
materialises its bases, `cse` still deletes the second and third
`ptr = &D_80106A73;` as redundant sets (measured: `lui 2` in both n3 and m5),
because `invalidate_skipped_block` only invalidates registers SET INSIDE the
skipped arm and the pointer locals are set at top level. A cse basic-block
boundary is what produces BOTH.

**4. The zero-cost boundary question is now answered, negatively, from the
compiler source.** `cse_end_of_basic_block` ends the block at a join label
only if the extension at cse.c:8102-8184 declines, and there are exactly
three ways for it to decline: `LABEL_NUSES (JUMP_LABEL (p)) != 1`; a
`CODE_LABEL` between the branch and the insn preceding the label (which
defeats the `no_labels_between_p` test in the AROUND arm); or the preceding
insn being a used `CODE_LABEL` (which defeats both arms). Each needs an
instruction the target does not contain, or a construct in a forbidden
family:
  - a second `LABEL_REF` means a second jump insn. Where the added jump
    targets the immediately following label, `jump.c` deletes it before cse1
    runs and `LABEL_NUSES` falls back to 1 - measured on two spellings
    (n4 22/51 and n8 23/49, both with `lbu 3`, the plain-diamond signature).
  - an unreferenced label inside the arm is a dead-goto label pad, a listed
    forbidden family; a referenced one carries its own jump.
  - the BARRIER (follow_jumps) escape needs the arm to end in an
    unconditional jump, i.e. the if/else shape, where the block is FOLLOWED
    rather than ended, with no invalidation.

**What this means for the F3 escalation packet.** It is closer to filable
than it was, but it is still NOT filable from a non-escalation modality and
the wording s7 proposed must change again. The correct claim is now: *every
construct that ends the cse basic block between two flag blocks costs at
least one instruction the target does not contain, or is a forbidden
no-semantic-purpose construct; and the only zero-cost mechanism that
reproduces the reloads (the AROUND / invalidate_skipped_block path) is
incompatible with the target's unconditional store.* The exhibits are s7's
p1 signature match and this session's m1 (free reloads, wrong store shape).

- [s8] cse.c's invalidate_skipped_block (7843-7867) / invalidate_skipped_set (7810-7836) INVALIDATE a MEM written inside a conditionally-skipped block instead of recording it - a second, label-free, volatile-free way to defeat the store-to-load forward that no prior session had found.

- [s8] m1_ptr_read_condstore ('val = *ptr; if (c) *ptr = val|K;') scores 18 at 42 insns with lbu 4 / sb 5 / lui 2, and its cse2.fn still holds (mem:QI (reg/v:SI 73)) for every flag read - the reloads survive both cse passes.

- [s8] The target's store is UNCONDITIONAL: asm/funcs/func_80034F88.s carries 'addu $v0,$a0,$zero' at 0x80034FC4 on the fall-through path and one 'sb' per block after the join label, so the AROUND mechanism (which needs the store inside the arm) can never produce the target's shape - the conditional-store chassis is 7 insns short by construction.

- [s8] The most natural human spelling of this function (plain symbol, conditional |=) scores 35 at 51 insns with lui 8 - four more lui than the target and no shared base; the target's source was NOT the obvious form.

- [s8] Store duplicated into both arms: find_cross_jump merges only 1 of the 3 duplicate pairs, leaving 7 sb against the target's 5 (19 at 46 insns).

- [s8] An explicit two-label goto diamond does NOT create a cse boundary: jump.c deletes the jump-to-the-next-label before cse1 runs and LABEL_NUSES returns to 1 (n4 22/51, lbu 3 - the plain-diamond signature).

- [s8] Three pointer locals staged exactly where the target materialises its three bases still collapse to lui 2 in the conditional-store chassis, because invalidate_skipped_block only invalidates registers set INSIDE the skipped arm.

- [s8] Consequence for the model: the missing reloads and the missing addend-0 bases are ONE problem (a cse basic-block boundary), not two as s7's frontier had it.


## s9 (rederive) - the boundary trade, priced

The session's numbers, all `sandbox func_80034F88 --disable all` with the
objdump census of the whole function (the copy loop contributes one `lbu`,
one `sb` and one `lui` to every count below; the target totals are
lbu 5 / sb 5 / lui 4 at 49 insns):

| form | score | insns | lbu | sb | lui |
|---|---|---|---|---|---|
| floor (current src) | 18 | 51 | 3 | 5 | 6 |
| b0 staged pointers, no boundary | 21 | 47 | 2 | 5 | 2 |
| c1 one `&&` boundary (block 1) | 23 | 50 | 3 | 5 | 3 |
| c4 two `&&` boundaries (blocks 1+2) | 31 | 53 | 4 | 5 | 4 |
| b1 three `&&` boundaries | 33 | 55 | 4 | 5 | 4 |
| d6 symbol-outer / pointer-arm read | 29 | 47 | 5 | 5 | 6 |
| e2 same + store through the pointer | 24 | 48 | 7 | 5 | 5 |
| f1 two boundaries + block-1 mismatch | 32 | 54 | 5 | 5 | 5 |
| d4 m2c two-read select + floor levers | 20 | 49 | 3 | 5 | 6 |
| d1 m2c verbatim | 29 | 45 | 2 | 5 | 6 |

Read the table as three families that each own PART of the target and cannot
be combined below the floor:

1. **Address materialisation** (lui 4, three addend-0 bases) is owned by the
   cse basic-block BOUNDARY family.  Without a boundary, cse deletes the second
   and third `ptrN = &D_80106A73;` as redundant sets (b0: lui 2).  With one
   boundary per join, each survives and emits its own `lui %hi` + `addiu %lo`
   at addend 0 (c4/b1: lui 4).  Price: one conditional branch per boundary.
2. **Reloads** (lbu 5) are owned by the mismatched-SPELLING family - two reads
   of the same byte written through different address expressions.  Price: each
   symbol-spelled access carries its own `lui $at` + `%lo(...)($at)`.
3. The **floor form** owns neither signature (lbu 3 / lui 6) and wins on
   alignment: at 51 insns it is 2 over the target, where the closest
   signature-correct form (f1) is 5 over.

The two mechanisms are not additive: f1 has both properties and scores 32.
The gap between "the build looks like the target" and "the build scores like
the target" is entirely the branch and materialisation insns the boundary
constructs drag in.

### What this does to the escalation thesis inherited from s8

s8's frontier read: *"every construct that ends the cse basic block between two
flag blocks costs at least one instruction the target does not contain ... the
claim that remains unproven is completeness: that no OTHER C construct reaches
those three states."*  s9 closes both of the two lines of attack that frontier
named, and it does so with a measurement rather than an argument:

- the `&&` boundary (LABEL_NUSES 2) is the cheapest reachable spelling, it is
  measured, and it is net-negative at every count of boundaries (+2, +13, +15);
- `thread_jumps` cannot supply a second LABEL_REF here because the three join
  labels are followed by different code, so no redirection is available.

The remaining escapes from cse_end_of_basic_block's extension test (read
verbatim at cse.c:8102-8184 this session) are: a used CODE_LABEL immediately
preceding the join (needs a jump to it), any CODE_LABEL between the branch and
the arm's last insn (needs a jump, or an unreferenced label = the forbidden
dead-goto label pad), and a BARRIER before the join (the if/else shape, where
the block is FOLLOWED rather than ended).  Every one of them costs at least one
jump insn.  A C construct that emits a CODE_LABEL with no jump does not exist
outside the computed-goto (`&&label`) family, which is a label pad by another
name.

- [s9] cse_end_of_basic_block's extension test was read VERBATIM this session at tools/gcc-2.7.2/cse.c:8092-8184 and its escapes are exactly three: (a) LABEL_NUSES (JUMP_LABEL (p)) != 1; (b) the backward scan at 8109-8114 stopping on a CODE_LABEL with LABEL_NUSES != 0 immediately before the join (the scan skips ordinary NOTEs and NUSES-0 labels, and breaks on NOTE_INSN_LOOP_END / NOTE_INSN_SETJMP); (c) the forward no_labels_between_p check at 8168-8172 finding ANY CODE_LABEL between the branch and the arm's last insn. Each of the three requires at least one extra jump or branch insn from C, or an unreferenced label (the forbidden dead-goto label pad).

- [s9] The `&&` boundary is REAL and works with an unconditional store: staged-pointer chassis, no guard = lbu 2 / sb 5 / lui 2 at 47 insns (score 21); guards on blocks 1+2 = lbu 4 / sb 5 / lui 4 at 53 insns (score 31), i.e. the target's three addend-0 `lui %hi(D_80106A73)` + `addiu %lo(D_80106A73)` bases materialise and the block-2/3 reloads survive.

- [s9] Marginal price of one cse basic-block boundary on this function: +3 insns and +2 score for the first (21/47 -> 23/50), +3 insns and +8 score for the second (23/50 -> 31/53), +2 insns and +2 score for the third (31/53 -> 33/55, and it buys nothing because there is no fourth base to rematerialise).

- [s9] The target's lbu 5 / sb 5 access counts are reachable with NO boundary, NO volatile and NO loop scaffolding by spelling a block's two reads differently - `val2 = D_80106A73 | K;` outside the arm and `val2 = *ptr;` inside it - at 47 insns (score 29). Spelling the store through the pointer as well gives SEVEN surviving reloads at 48 insns (score 24, the best member of the family).

- [s9] The complete target signature (lbu 5 / sb 5) alongside an unconditional store costs 54 insns and scores 32 (f1). Signature is not score: the floor form has neither signature (lbu 3 / lui 6) and wins because it is only 2 insns over the target where every signature-correct form is 5 or more.

- [s9] m2c's own reconstruction of this function (fresh run this session) scores 29 at 45 insns; the best hybrid of m2c's two-reads-per-block select with the floor's levers scores 20 at 49 insns. m2c's structure is NOT the missing one.

- [s9] The three flag blocks' join labels are each followed by different code, so thread_jumps (which runs at toplev.c:2861, immediately before cse1) has no equivalent-destination pair to redirect and cannot raise any join label's LABEL_NUSES.

- [s9] Target census re-established from asm/funcs/func_80034F88.s this session: 49 insns, lbu 5 (mask read + one per flag block + the copy loop), sb 5, lui 4 (three addend-0 D_80106A73 bases + the copy loop's D_80106A70), exactly one conditional branch per flag block, and each block's store emitted AFTER its join label.

---

# s10 — synthesis modality (honest floor 18 -> 9; the model of this function is
# rewritten)

## Headline
The s1-s9 model — a three-way trade in which reloads, addend-0 shared bases and
instruction count can each be bought only at the others' expense — was
INCOMPLETE, and the missing piece is free. A third reload mechanism exists,
costs nothing, needs no volatile / label / branch / loop note, and is compatible
with the target's unconditional store. With it the honest floor moved
**18 -> 13 -> 10 -> 9** in one session, at **49 build instructions against a
49-instruction target**, and from flag block 2 to the end of the function the
build is identical to the target instruction-for-instruction AND
register-for-register.

## THE MECHANISM (register death) — the load-bearing finding
`cse` records a store as an equivalence class containing BOTH the stored MEM and
the REGISTER the value came from. A later read of the same byte is satisfied
from the cheapest LIVE member of that class — the register — and the load is
deleted (that is the `andi a0,v1,0xff` zero-extend-from-register that the s3-s9
floor form emitted where the target has an `lbu`). If that register has been
OVERWRITTEN before the read, the MEM is the only member left and cse must emit a
real `lbu`.
Consequence: **a reload survives exactly when the variable that supplied the
stored value is reassigned before the byte is read back** — no volatile, no
CODE_LABEL, no NOTE_INSN_LOOP_END, no conditional store, and no extra
instruction. This is a THIRD mechanism alongside the two s1-s9 enumerated
(volatile MEM; cse basic-block boundary) and s8's AROUND /
`invalidate_skipped_block` path, and unlike all three it is compatible with the
target's unconditional store. Every "the reloads cost N instructions" conclusion
in the s7/s8/s9 sections is superseded for the case where the stored value's
variable is reused.
In the new floor form the block condition and the selected value are ONE local
(`c`), so each block's `c = p[8] & K;` kills the previous block's stored value
and the following `val = *ptrN;` is a real reload.

## The three levers, each measured (all `sandbox func_80034F88 --disable all`)
| lever | form | score / insns |
|---|---|---|
| s3-s9 floor (control, re-measured) | `f0_s32_floor` | 18 / 51 |
| 1. condition/value variable reuse, symbol-spelled | `v1_reuse_c_ifelse` | 21 / 52 |
| 1+2. same, every access through one pointer local | `w3_one_ptr_rw` | **13 / 45** |
| 1+2+3. + one pointer variable re-assigned per block | `z1_reassign_block_top` | **10 / 49** |
| 1+2+3. + a second pointer variable, re-assigned once | `zz5_two_vars` / `zz6` | **9 / 49** |

1. **Condition/value variable reuse** (`if (c) c = val | K; else c = val;`) —
   the frozen SOTN-accepted "variable reuse for codegen control" family. It buys
   the reloads by the mechanism above AND, independently, the target's register
   assignment for the selected value: the target emits `ori v0,a0,K` /
   `move v0,a0`, reusing the CONDITION's register for the result, which s6 had
   recorded as an unsteerable `local-alloc` tie ("the $v0/$v1 swap is NOT a
   steerable allocation tie"). That s6 conclusion is now REFUTED: with a
   separate `val2` local those insns come out as `ori v1,a0,K` / `move v1,a0`,
   6 points of pure register naming, and the reuse spelling flips all six.
2. **Every flag access through a pointer local**, so each base is an unfolded
   `lui %hi` + `addiu %lo` pair shared by that block's load and store. Mixing
   symbol and pointer spellings on this chassis costs 9-15 points (w4 24, w5 24,
   w8 31, zz4 35).
3. **A second base-bearing pointer, re-assigned**, so all three of the target's
   addend-0 base materialisations appear. One base = 13/45; three bases via one
   re-assigned variable = 10/49; via two variables (one re-assigned) = 9/49.

## What the residual 9 is
Side-by-side `tmp/grind/func_80034F88/s10/sbs_zz5.txt`. Everything from flag
block 2 to the function's end is exact. All 9 points are the mask region plus
block 1 and every one of them is downstream of ONE absent instruction, block 1's
`lbu` reload:
- base/byte hard-register swap in the mask region (target base `v1` / byte `a0`,
  build base `a2` / byte `v1`) — 5;
- a `nop` where the target has `lbu a0,0(v1)` in the `lw` load-delay slot — 1;
- `ori v0,v1,1` / `move v0,v1` against target `ori v0,a0,1` / `move v0,a0`, plus
  block 1's `sb` addressing the swapped base — 3.
The mask's stored value is still live in a register at block 1's read, so the
register-death mechanism does not fire there.

## The block-1 reload IS reachable — and it is mutually exclusive with the
## register assignment (this is the frontier)
Routing the mask's value through the same reused local
(`c = *ptr & 0xF8; *ptr = c;`) makes block 1's `c = p[8] & 1;` kill it and
produces the target's full `lbu 5` count with an unconditional store — the only
non-volatile zero-cost form ever measured to do so. But carrying the mask
through `c` extends that local's live range across the call return and permutes
the entire allocation (`p` into a2, condition and value into v1): **33** on
every base structure it was crossed with (x1/x5/x6 33/45, x3 33/47,
x2/x4/x8/zz1/a1/a2/a3/a4/a8 33/49, a7 34/50). Reading block 1 through the other
pointer variable also produces the reload, at 50 insns / 27 (b1); storing
through it is 13 (b2); both is 23/51 (b3). Banked in `rejected/`.

## The 49-instruction `u8 val` chassis is DEAD at 20 (s9's frontier item 2)
s9 left "nothing in s4-s9 has re-explored the 49-insn chassis" as a live
hypothesis. Thirteen forms crossed it with every lever found since s3
(two-reads-per-block, outer-symbol/arm-pointer reads, staged pointer bases,
pointer/symbol mixes per access, explicit-RMW mask, `s32` selected value,
per-block read pointers): u0/u1/u9/u10 20/49, u4 23/48, u3/u5/u7/u8 24,
u6/u11/u12 25/49, u2 29/47. Best 20; nothing reached even the then-floor of 18.
The chassis was never the axis. **KILLED**, banked as
`rejected/u8-chassis-49insn-plateau-score20.c`.

## Robustness of the 9 (it is a plateau, not a single lucky spelling)
Perturbations that leave it unchanged: declaration order (all four orders
tried), `s32` vs `u32` loaded byte, `*ptr &= 0xF8` vs `*ptr = *ptr & 0xF8`,
arm order (`if (c)` vs `if (!c)`), array vs deref spelling (`ptr[0]`), staged vs
block-top placement of the second pointer's first assignment. Perturbations that
cost: `u8` loaded byte (28), `u8` condition (22), ternary select instead of
if/else (24), assigning the pointer before the call (29).

## Classification note carried into the outcome
The 9-point form's SECOND `ptr2 = &D_80106A73;` re-assigns a variable a value it
already holds. It fails checklist T1 (no observable effect) and T2 (a reader
would ask why it is there); it exists because a redundant address set is not
deleted on this chassis and therefore emits the target's third base. It is
banked as a MEASUREMENT and explicitly NOT submitted or self-approved — see the
CLASSIFICATION section of `candidate.c`. The clean form, in which every
construct is on the frozen SOTN-accepted list (a genuinely-used pointer alias to
a global plus condition/value variable reuse; no dead store, no redundant
assignment, no volatile, no barrier, no label pad), is `candidate_clean_13.c` at
**13 / 45** — still 5 points better than the s1-s9 floor of 18.

- [s10] MECHANISM (the session's load-bearing finding): cse records a store as an equivalence class holding both the stored MEM and the REGISTER the value came from, and satisfies a later read from the live register — so a reload survives, at zero instruction cost and with no volatile / CODE_LABEL / loop note / conditional store, exactly when the variable that supplied the stored value is REASSIGNED before the byte is read back. This is a third reload mechanism beyond the two s1-s9 enumerated and s8's AROUND path, and unlike all of them it is compatible with the target's unconditional store. Every s7/s8/s9 statement pricing the reloads at one or more instructions is superseded for the variable-reuse case.
- [s10] Honest floor 18 -> 9 (`sandbox func_80034F88 --disable all`), edits in place in src/code6cac_b.c, 49 build instructions against a 49-instruction target, zero volatile, zero cheat-asm. Ladder measured this session: 18/51 (s3-s9 floor control) -> 21/52 (variable reuse, symbol-spelled) -> 13/45 (+ all accesses through one pointer local) -> 10/49 (+ the pointer variable re-assigned per block) -> 9/49 (+ a second pointer variable).
- [s10] From flag block 2 to the end of the function the 9-point build matches the target instruction-for-instruction AND register-for-register (tmp/grind/func_80034F88/s10/sbs_zz5.txt).
- [s10] s6's conclusion that the $v0/$v1 swap of the selected value is 'NOT a steerable allocation tie' is REFUTED. Spelling the block condition and the selected value as ONE local flips all six `ori`/`move` register-naming points: target `ori v0,a0,K` / `move v0,a0` reuses the CONDITION's register for the result, and a separate `val2` local is what forced `v1`.
- [s10] s9's frontier item 2 (the 49-insn `u8 val` chassis, never re-explored since s3) is KILLED at 20: thirteen forms crossing it with every lever found in s4-s9 score 20-29, best 20.
- [s10] Three separate pointer LOCALS give the target's instruction shape at exactly 49 insns but score 23 in every placement and declaration order (w1, w2, w9, z6, z7, z8): the allocator gives all three bases the same hard register and evicts `p` from a1. What works is ONE pointer variable re-assigned (10/49), or two variables one of which is re-assigned (9/49).
- [s10] Block 1's missing reload is reachable — routing the mask's stored value through the reused local produces the target's full lbu 5 count with an unconditional store — but it extends that local's live range across the call return and permutes the entire allocation: 33 on every base structure measured (13 variants), 27 for the second-pointer read spelling. On this chassis block 1's reload and the target's register assignment are mutually exclusive; that trade is the whole residual 9 and the next session's frontier.
- [s10] The 9 is a plateau, not a lucky spelling: declaration order (4 orders), s32 vs u32 loaded byte, compound vs explicit mask RMW, arm order, and array vs deref spelling are all neutral; u8 loaded byte (28), u8 condition (22), ternary select (24) and assigning the pointer before the call (29) all cost.
- [s10] CLASSIFICATION CARRIED FORWARD, not self-approved: the 9-point form's second `ptr2 = &D_80106A73;` re-assigns a variable a value it already holds and fails checklist T1/T2; it exists only because a redundant address set is not deleted on this chassis and therefore emits the target's third base. The clean fallback — every construct on the frozen SOTN list — is memory/grind/func_80034F88/candidate_clean_13.c at 13/45.

- [s10] Honest floor 18 -> 9 (`sandbox func_80034F88 --disable all`), edits in place in src/code6cac_b.c, 49 build instructions against a 49-instruction target, zero volatile and zero cheat-asm.

- [s10] THE MECHANISM (reusable beyond this function): cse records a store as an equivalence class holding both the stored MEM and the REGISTER the value came from, and satisfies a later read of the same byte from the live register, deleting the load. Overwriting that register before the read leaves the MEM as the only member of the class, so a real load is emitted. A reload therefore survives at ZERO instruction cost, with no volatile / CODE_LABEL / loop note / conditional store, exactly when the variable that supplied the stored value is reassigned first.

- [s10] This is a THIRD reload mechanism beyond the two s1-s9 enumerated (volatile MEM; cse basic-block boundary) and s8's AROUND / invalidate_skipped_block path, and unlike all of them it is compatible with the target's unconditional store. Every s7/s8/s9 statement pricing the reloads at one or more instructions is superseded for the variable-reuse case, and the s9 escalation thesis must be rewritten before it is ever filed.

- [s10] Measured ladder this session: 18/51 (s3-s9 floor control) -> 21/52 (condition/value variable reuse, symbol-spelled) -> 13/45 (+ every flag access through one pointer local) -> 10/49 (+ the pointer variable re-assigned per block) -> 9/49 (+ a second pointer variable, one re-assignment).

- [s10] From flag block 2 to the end of the function the 9-point build matches the target instruction-for-instruction AND register-for-register (tmp/grind/func_80034F88/s10/sbs_zz5.txt).

- [s10] The residual 9 is entirely the mask region plus block 1 and every point is downstream of ONE absent instruction, block 1's `lbu a0,0(v1)` reload: base/byte hard-register swap in the mask region (5), a nop where the target fills the lw load-delay slot (1), and the ori/move pair plus block 1's sb addressing the swapped base (3).

- [s10] s6's conclusion that the $v0/$v1 swap of the selected value is not steerable is REFUTED: spelling the block condition and the selected value as ONE local flips all six ori/move register-naming points, because the target reuses the condition's register for the result and a separate val2 local is what forced v1.

- [s10] s9's frontier item 2 (the 49-insn `u8 val` chassis, never re-explored since s3) is KILLED at 20 across 13 forms crossing it with every post-s3 lever.

- [s10] Three separate pointer LOCALS give the target's instruction shape at exactly 49 insns but score 23 in every placement and declaration order, because the allocator gives all three bases the same hard register and evicts `p` from a1.

- [s10] The 9 is a plateau, not a lucky spelling: declaration order (4 orders), s32 vs u32 loaded byte, compound vs explicit mask RMW, arm order and array-vs-deref spelling are all neutral; u8 loaded byte 28, u8 condition 22, ternary select 24, pointer assigned before the call 29.

- [s10] CLASSIFICATION CARRIED FORWARD, not self-approved: the 9-point form's SECOND `ptr2 = &D_80106A73;` re-assigns a variable a value it already holds. It fails checklist T1 (no observable effect) and T2 (a reader would ask why it is there) and exists only because a redundant address set is not deleted on this chassis and therefore emits the target's third base. It is banked as a MEASUREMENT and must not be submitted as it stands.

- [s10] The CLEAN fallback is memory/grind/func_80034F88/candidate_clean_13.c at 13 / 45 insns: one pointer local carrying all traffic on the byte plus condition/value variable reuse - both on the frozen SOTN-accepted list, with no dead store, no redundant assignment, no volatile, no barrier and no label pad. Even if the classification question is resolved against the 9, the function's floor is 13, not the 18 of s1-s9.

- [s10] Instruments left for s11: s10/probe.py (splice + honest sandbox + objdump lbu/sb/lui census; --install to make a form permanent), s10/sbs.py (difflib-aligned build-vs-target listing - on this function the score IS the count of misaligned positions, so the listing names the exact instruction to chase), and gen_u/gen_v/gen_w/gen_x/gen_y/gen_z/gen_zz/gen_a2/gen_b.py (68 forms measured this session).

- [s11] The target's shared addend-0 base CANNOT be produced by spelling the flag access through the symbol. A direct `D_80106A73` read/write pair always folds `%lo` into each access separately (`lui at` + `lbu %lo(at)` ... `lui at` + `sb %lo(at)`), never `lui`+`addiu`+`lbu 0(base)`+`sb 0(base)`: a1 24/49, a2 24/49, a3 32/50, a4 21/52, a5 32/50, a6 24/47, a7 24/49, b8 24/49. The cse-extended-basic-block model s11 opened with ("each if/else join clears cse's table, so a symbol reference must re-materialise the address by itself") is REFUTED as a route to the base: the join does end the block, but what comes out is two `lui`s with folded `%lo`, not a shared base. Therefore an addend-0 base requires the address to sit in a POINTER LOCAL, and an ADDITIONAL base requires an additional ASSIGNMENT of a pointer local - there is no third option.

- [s11] The target's flag-block tail (`bnez; ori` in the delay slot; `addu`; `LABEL: sb 0(base)`) is NOT a jump2 cross-jump merge of a store duplicated into both arms. Writing the store inside each arm (the SOTN-sanctioned duplicated-statement-into-arms shape) emits 48 instructions against the target's 49 - the arms' stores merge but the value copy does not - and scores 18 (c1/c2, pointer chassis), 30 (c3/c4, symbol blocks 2/3), 34-35 (c5/c6, symbol everywhere), 19/46 (c9, one base). The label-shared spelling s1-s10 have always used is the correct reading of that tail.

- [s11] **A POINTER COPY defeats cse's fold of block 1's reload at zero instruction cost.** `ptr2 = ptr;` after the mask store, with block 1 reading and storing through `ptr2`, makes block 1's `val = *ptr2;` a real `lbu` (lbu census 176, the target's count) while keeping 49 instructions and, unlike s10's register-death mechanism, WITHOUT permuting the allocation. This is a FOURTH reload mechanism, distinct from volatile MEM (forbidden), the cse basic-block boundary (costs a branch, s7/s9), invalidate_skipped_block (needs a conditional store, s8) and register death (s10, costs the whole allocation at 33).

- [s11] h1 (`tmp/grind/func_80034F88/s11/variants/h1_copy_ptr2_blk1.c`, banked as memory/grind/func_80034F88/candidate_h1_13.c, score 13 / 49 insns) is the first form in eleven sessions that is INSTRUCTION- AND REGISTER-IDENTICAL to the target from the prologue through block 1's if/else: `lui v1` / `addiu v1` / `lbu a0,0(v1)` / `move a1,v0` / `andi a0,a0,0xf8` / `sb a0,0(v1)` / `lw v0,32(a1)` / `lbu a0,0(v1)` / `andi v0,v0,1` / `bnez` / `ori v0,a0,1` / `move v0,a0` / `sb v0,0(v1)` - base 1 in v1, the mask in a0, the surviving reload in the `lw`'s load-delay slot (no nop), condition and result in v0. Side-by-side: tmp/grind/func_80034F88/s11/sbs_h1.txt.

- [s11] h1's ENTIRE residual 13 is one register swap and its scheduling consequence: the build materialises blocks 2/3's base into v1 - base 1's register, dead the instant block 1 stores - and puts their byte in a0; the target does the opposite (base a0, byte v1), and because a0 is free there its scheduler hoists the `lui`/`addiu` pair ABOVE block 1's `sb`. Both allocations are legal for the identical instruction stream, so this is an allocno-priority tie, not a shape difference.

- [s11] That tie is robust to every structural perturbation measured: byte local split one/two/three ways (j1/j2/j3 13), separate condition local for blocks 2/3 (j7 13), mask carried by `val`/`m`/`c` (h1 13, j4 13, j5 28), block 1 storing through the original pointer instead of the copy (h2 15, j6 15), the copy made after the condition (j8 13), a dedicated third pointer local for blocks 2/3 (i2/i3/i6 18), four single-assignment pointer locals (i5 13), blocks 2/3's base materialised before block 1's store (k1 18, k2 13, k3 15, k5 23/50) and block 3 re-using the mask's pointer (i4 14, k4 26).

- [s11] Declaration order is completely INERT on this function's allocation: all 24 permutations of the four working locals plus 6 orders moving `p` and `i` score exactly 33 on the b2 chassis, at 49 instructions with an identical census. The frozen SOTN "named-intermediate declaration order" lever has no gradient here and future sessions should not spend measurements on it.

- [s11] Splitting the byte local costs an instruction on the register-death (mask-in-`c`) chassis - f1/f2/f5/f8 all 50 insns / 34, f3 50/28 - but is FREE on the plain-mask chassis (f4 9/49, identical to the baseline). The split is only worth carrying on a chassis that already has the reload from elsewhere.

- [s11] Type narrowing of the value locals is dead across the board: u8 byte + s32 cond 24/51, 22/50, 33/49; s32 byte + u8 cond 18/49, 18/49, 24/49; u8 both 23/51, 21/50, 26/49; signed-char byte 23/58 (sign-extension insns). None reaches the 9.

- [s11] b2 (`c = *ptr & 0xF8; *ptr = c;` on the s10 two-pointer base structure) emits the target's COMPLETE 49-instruction stream - same opcodes, same order, same delay slots, no nop - with a pure hard-register permutation (build base1 a2 / p a3 / base2 a1 / mask+byte v1 / cond v1 against target base1 v1 / p a1 / base2 a0 / byte a0 / cond v0), score 33. Its greg dump (tmp/grind/func_80034F88/s11/rtl/b2/fn/greg.fn) shows 6 pseudos allocated v1..a3 with 72 conflicting with all five others; the target's stream only ever uses v0/v1/a0/a1, which is only possible if block 1's byte and blocks 2/3's byte are DIFFERENT pseudos (block 1's byte is a0, sharing with base 2/3; blocks 2/3's byte is v1, sharing with base 1). h1 supersedes b2 as the route to that shape.

- [s11] The session floor is UNCHANGED at 9 (b0/zz5, b1, b6, f4, f7 all 9/49). s10's classification question is unchanged with it, and s11 adds a datum that sharpens it: since an extra addend-0 base can only come from an extra pointer-local ASSIGNMENT (first bullet), the only redundancy-free spellings of three bases are three or four SINGLE-assignment pointer locals - measured at 23 (s10, three plain locals), 18 (i2/i3/i6) and 13 (i5, four locals, one of them a copy). A redundancy-free 9 does not exist on any structure measured in s1-s11.

- [s11] Instruments added: s11/probe.py + s11/sbs.py (s10's, re-pointed at s11/variants), s11/dump.sh + s11/slice.py (s6's cc1 -da harness re-pointed), and gen_a/gen_c/gen_d/gen_f/gen_g/gen_h/gen_i/gen_j/gen_k.py - 90 forms measured this session.

- [s11] The target's shared addend-0 base CANNOT be produced by a direct symbol reference — `D_80106A73` read+write always folds `%lo` into each access separately (two `lui`s per block). An addend-0 base requires a pointer LOCAL holding the address; an ADDITIONAL base requires an ADDITIONAL pointer-local assignment. There is no third option, which is the constraint underneath s10's classification problem.

- [s11] A POINTER COPY is a fourth reload mechanism, distinct from volatile MEM (forbidden), a cse basic-block boundary (costs a branch, s7/s9), invalidate_skipped_block (needs a conditional store, s8) and register death (s10, costs 33 in allocation). It is free: no instruction, no register, no barrier, no label, no volatile.

- [s11] h1 (memory/grind/func_80034F88/candidate_h1_13.c, 13 / 49 insns, lbu census 176 = the target's 5) is the first form in eleven sessions that is instruction- AND register-identical to the target from the prologue through block 1's if/else. Side-by-side: tmp/grind/func_80034F88/s11/sbs_h1.txt.

- [s11] h1's ENTIRE residual is one register swap plus its scheduling consequence: build gives blocks 2/3's base v1 (base 1's just-vacated register) and their byte a0; the target does the opposite, and because a0 is free there its scheduler hoists the base's `lui`/`addiu` above block 1's `sb`. Both allocations are legal for an identical instruction stream.

- [s11] That tie is robust to 19 measured structural perturbations: byte splits one/two/three ways, a separate condition local, mask carried by `val`/`m`/`c`, storing through the original pointer, the copy made after the condition, a dedicated third pointer local, four single-assignment locals, the base materialised before block 1's store and before its if/else, and block 3 re-using the mask's pointer.

- [s11] Declaration order is completely inert here — 30 orders, all exactly 33 with an identical census. The frozen SOTN declaration-order lever has no gradient on this function.

- [s11] b2 (s10's register-death mask on the two-pointer base structure) emits the target's COMPLETE 49-instruction stream — same opcodes, order and delay slots, no nop — with a pure hard-register permutation, score 33. Its greg dump (tmp/grind/func_80034F88/s11/rtl/b2/fn/greg.fn) shows 6 pseudos allocated v1..a3 where the target's stream only ever uses v0/v1/a0/a1.

- [s11] Type narrowing of the value locals is dead across the board: u8 byte 24/51, 22/50, 33/49; u8 condition 18/49, 18/49, 24/49; u8 both 23/51, 21/50, 26/49; signed char 23/58.

- [s11] Splitting the byte local costs an instruction on the register-death chassis (50 insns) but is free on the plain-mask chassis (9/49) — the split is only worth carrying on a chassis that already has the reload from elsewhere.

- [s11] Floor UNCHANGED at 9 (b0/zz5, b1, b6, f4, f7 all 9/49). s10's classification block on the 9's redundant `ptr2 = &D_80106A73;` is unchanged, and s11 sharpened it: the only redundancy-free spellings of three bases are three or four single-assignment pointer locals, measured at 23, 18 and 13. A redundancy-free 9 does not exist on any structure measured in s1-s11.

- [s11] src/code6cac_b.c was restored to HEAD after every probe; the session leaves no source edits (probe.py splices and restores, and the one --install for the RTL dump was reverted with git checkout).

## s12 (structural) — MATCHED. Honest floor 9 -> **0** at 49/49 insns.

- [s12] `sandbox func_80034F88 --disable all` prints `score 0`, `build_insns 49`,
  `target_insns 49`, `rules_dropped 30`, `cheat_asm_stripped 317` with the body
  now installed at src/code6cac_b.c:3899. The form is banked verbatim at
  memory/grind/func_80034F88/candidate.c and vetted at
  memory/grind/func_80034F88/self_vet.md.

- [s12] THE ELEVEN-SESSION WALL WAS THE BYTE LOCAL, NOT THE POINTER STRUCTURE.
  s1-s11 all used ONE C local for the loaded flag byte across all three flag
  blocks, and every one of ~150 measured forms varied only the pointer/base
  structure around it. With a single byte local the allocno's live range spans
  block 2's base materialisation, the two conflict, the base is evicted from a0
  onto a2, and the byte is then pinned to a0 in every block — which is exactly
  the "blocks 2/3 base/byte swap" the ledger recorded from s3 onward as an
  unsteerable allocno-priority tie. It was never a tie; it was a conflict, and
  the conflict was created by the source using one variable where the original
  used two.

- [s12] The target's own stream states the requirement plainly (read off
  tmp/grind/func_80034F88/s12/sbs_d3.txt, target side):
      ori v0,a0,1 / move v0,a0 / lui a0 / addiu a0 / sb v0,0(v1)
  Block 1's loaded byte is in a0 and dies at the `move`; block 2's base is
  materialised into THAT SAME a0 one instruction later, still ahead of block 1's
  `sb`. a0 therefore has to be free between block 1's `move` and block 2's
  `lui`, which is only possible if block 1's byte and block 2's byte are
  different pseudos. Everything downstream (block 2/3's byte on v1, block 3's
  base reusing a0) follows from that one reuse.

- [s12] The split POINT matters and is measurable: mask+block 1 sharing one
  local and blocks 2/3 sharing another scores 0; one local per block also
  scores 0 (h5, f_d1_s2/s4); one local for everything scores 13 (h6); and the
  "wrong" split — mask alone against the three blocks together — is also 13
  (f_d1_s7). Byte-local splitting is worth 12-13 points on any chassis whose
  instruction stream is already right, and 0 points on one that is not
  (f_a2_s1..s8 are all 9 or 21 — the split cannot rescue a chassis missing
  block 1's reload).

- [s12] Once the split is present the pointer structure stops being delicate.
  SEVEN structurally different bodies reach 0: four single-assignment pointer
  locals with no copy (g4), the same with a copy handle (g7 = f_d1_s1), the
  same with purely sequential assignment placement (g9, h3), per-block-scoped
  handles with the mask outside (h1) and with the mask as its own block (h2),
  and h5's per-block byte locals. The submitted form is h2/i5 — the one with
  the fewest constructs to vet.

- [s12] The two constructs s10 and s11 both flagged as classification-blocked
  are GONE from the matched form: there is no re-assignment of a pointer to a
  value it already holds (every handle is assigned exactly once, in its own
  block) and no pointer copy. The three `asm volatile("" ::: "memory")`
  scheduling barriers that the pre-existing src body carried are also deleted.
  The matched body adds no inline asm, no pin, no volatile, no cast, no dead
  store and no unused declaration.

- [s12] KILLED — the s11 frontier's item 2 (cross the pointer-COPY handle with
  every recorded base structure). Measured across all of them and the copy
  handle strictly COSTS on a single-byte-local chassis: zz5's two variables 13
  (a1), z1's single re-assigned handle 12 (a5/a6), w3's single base 12 (a7/a8),
  three copy handles one per block 27 (a9); the 9-point chassis with the h1
  mask spelling and no copy stays 9 (a2). The copy buys block 1's reload
  (objdump lbu 176 vs 175) and pays for it in blocks 2/3 — the two failure
  regions were entangled, not independent as the frontier assumed.

- [s12] KILLED — duplicated-read-into-arms as a ref-count lever on this
  function. Spelling the select as `if (c) { c = *q | K; } else { c = *q; }`
  adds two lbu (178) and four instructions (53) and scores 20-22 (b1/b2/b3),
  against 13 for the same chassis with a shared read. The frontier's "change
  the REF COUNTS" probe is answered negatively for this shape.

- [s12] Load-bearing spellings inside the matched form, each measured against
  the otherwise-identical body: reusing `c` for the condition and then for the
  stored value is required (a separate result local collapses the if/else
  diamond to 44 insns and scores 30 — j1; a compound `v |= K` with no else is
  also 30 — j3; a ternary is 25 — j5; testing `p[8] & K` directly in the `if`
  is 30 — i1/i6). `s32` for the byte local is required (`u8` scores 10 —
  i3/i7). Hoisting all four pointer assignments to the top of the function
  scores 25 (h4); assigning each handle inside its own block is 0.

- [s12] Wave sizes: 13 forms (wave A, the s11-frontier cross), 11 (wave C/D/E,
  base-register alternation), 24 (wave F, byte-local splits x 3 chassis),
  10 (wave G, construct minimisation), 7 (wave H, naturalisation), 8 (wave I,
  spelling strip-down), 6 (wave J, is the `c` reuse avoidable). 79 forms, all
  sandbox-scored; results in tmp/grind/func_80034F88/s12/results.json.

## s12b (permuter modality — re-dispatch after the layer-1 FAIL)

Context: the preceding session reached honest sandbox **0** at 49/49 insns, and
its body was FAILed by the layer-1 cheat-reviewer for carrying four textually
repeated `u8 *q = &D_80106A73;` declarations. The driver has since BANNED that
construct under BOTH classifications (pointer-alias-fake-exception family, and
"ordinary program logic"). This session therefore treated the byte-local-split
finding as inherited and attacked the ONE remaining open question: is there a C
spelling that produces the target's FOUR separate materialisations of
`&D_80106A73` WITHOUT four repeated declarations?

- [s12b] The four separate address materialisations are LOAD-BEARING, and their
  price is now measured exactly (wave K, 8 forms, byte-local split held fixed
  in every one). One natural handle for the whole function scores **23** at 47
  insns (lbu 173); two handles **27**/47; three handles **23**/49 (lbu 175);
  one handle with the mask through the plain symbol **28**; one handle with
  blocks 2/3 through the plain symbol **28**; no handle at all (plain symbol
  everywhere) **29**; two handles where the second is a pointer copy **23**; a
  single handle with three function-scope byte locals and no inner scopes
  **23**. Against 0 for four handles. So the byte-local split — the s12
  breakthrough — is worth nothing on its own: it only pays on a chassis that
  already has four address materialisations. The reload census tracks it
  monotonically (lbu 173 at one handle, 175 at three, 176 at four).

- [s12b] NEW AXIS, and the first structurally different route to the four
  materialisations ever found: a `static inline` helper. A single
  `static inline void bb2_set_flag(s32 c, s32 bit)` whose body declares
  `u8 *q = &D_80106A73;` ONCE, called three times, plus one handle in the
  caller for the `&= 0xF8` mask, scores **13 at 50 insns with lbu 176** — all
  four reloads present, the same access signature as the matched form, from
  ONE textual declaration of the handle. Three spellings of it tie at 13 (m1
  caller-scope mask block, m3 caller function-scope mask handle, m4 helper
  takes `p` and computes its own condition); `u8` instead of `s32` for the
  helper's byte local costs 3 (m5 = 16). Precedent for `static inline` in this
  codebase: src/main.c:2396 (`_memcpy`). CONTROL: the same helper without the
  `inline` keyword emits a real `jal` (m6 = 35 at 37 insns), so the inlining is
  what produces the four pseudos, not the factoring per se.

- [s12b] WHY the inline chassis is 13 and not 0 — read off the side-by-side
  (tmp/grind/func_80034F88/s12/sbs_m1.txt). The inlined copies do NOT keep an
  unfolded shared base: each instantiation's access folds `%lo` into its own
  mem, so where the target has `lbu a0,0(v1)` / `lui a0; addiu a0,a0,0` /
  `sb v0,0(v1)` the build has `lui a0,0x0; lbu a0,0(a0)` and `lui at,0x0;
  sb v0,0(at)`. The mask's base does survive in v1 and is reused for block 1's
  STORE, but not for block 1's load. The mechanism is GCC 2.7.2's inliner
  (`integrate.c` `copy_rtx_and_substitute` + its const-equivalence map)
  substituting the constant address at each use inside the copied body, which
  leaves every mem with a single-use address that `combine` then folds. This is
  the exact inverse of the textual form, where the pointer local survives to
  cse as a shared pseudo. Passing the address as an ARGUMENT does not escape it
  (m7 = 22 at 52 insns, lbu 174) — the actual argument is itself a constant, so
  the same substitution happens.

- [s12b] KILLED (third independent confirmation) — the permuter axis, now also
  on the inline chassis. Campaign `inline-helper-chassis`, workspace
  tmp/grind/func_80034F88/s12/ws, 8 jobs, base permuter score 1885, **6,450
  iterations**, best permuter score 450 (a 4x improvement on its own metric).
  Every one of the 16 lowest-scoring finds sandbox-scores **32-43**, against
  **13** for the seed the campaign started from — and each carries sb 160-161
  against the correct 164, i.e. the mutations delete the flag stores outright.
  The permuter cannot preserve a `static inline` chassis (it un-inlines or
  guts the helper) and its metric remains uncorrelated with the honest sandbox
  on this function. Harvested and stopped in-session.

- [s12b] The function-like MACRO spelling reaches **0** (m9, 49 insns, lbu 176)
  — one textual declaration site, expanding to exactly the banned four-handle
  body. It is recorded and banked deliberately as
  `rejected/macro-respelling-of-banned-four-handle-score0-DO-NOT-SUBMIT.c`:
  under cheat-checklist T5 it is the banned construct respelled, not a new
  attack, and the brief is explicit that respelling a banned construct is the
  same construct. It is banked as EVIDENCE for the ruling question below, not
  as a candidate. Do not submit it.

- [s12] Wave K price table for the pointer-handle count, byte-local split held fixed in all 8 forms: one handle 23/47 insns (lbu 173), two 27/47 (174), three 23/49 (175), two-with-a-copy 23/46 (174), one-with-mask-on-symbol 28/49 (174), one-with-blocks-2/3-on-symbol 28/48 (174), no handle at all 29/47 (173) - against 0/49 (lbu 176) for four handles. The four address materialisations of &D_80106A73 are load-bearing and the honest floor without them is 23.

- [s12] NEW AXIS: `static inline void bb2_set_flag(s32 c, s32 bit)` declaring `u8 *q = &D_80106A73;` ONCE and called three times, plus one handle in the caller for the `&= 0xF8` mask, scores 13 at 50 insns with lbu 176 - the matched form's exact access signature (all four reloads) from a single textual declaration. Three spellings tie at 13 (m1/m3/m4). `static inline` precedent in this tree: src/main.c:2396 (`_memcpy`).

- [s12] CONTROL for that axis: the same helper WITHOUT the `inline` keyword emits a real `jal` and scores 35 at 37 insns (m6), so the inlining - not the factoring - is what creates the four pointer pseudos.

- [s12] The inline chassis's residual 13 is entirely %lo folding, read off the side-by-side at tmp/grind/func_80034F88/s12/sbs_m1.txt: build emits `lui a0,0x0; lbu a0,0(a0)` and `lui at,0x0; sb v0,0(at)` where target has one shared unfolded `lui`/`addiu` base serving both the reload and the store. Mechanism: GCC 2.7.2's inliner (integrate.c copy_rtx_and_substitute + const-equivalence map) substitutes the constant address at each use inside the copied body, so every mem reaches combine with a single-use address. Passing the address as an ARGUMENT does not escape it (m7 = 22 at 52 insns), because the actual argument is itself a constant.

- [s12] PERMUTER, third independent kill: campaign `inline-helper-chassis` (tmp/grind/func_80034F88/s12/ws, 8 jobs) ran 6,450 iterations, base permuter score 1885 -> best 450, and all 16 lowest-scoring finds sandbox-score 32-43 against the seed's 13. Every find carries sb 160-161 vs the correct 164 - the randomizer deletes flag stores and cannot preserve a `static inline` chassis. Harvested and stopped in-session with a reason string; campaign confirmed dead (alive: false).

- [s12] The function-like macro spelling reaches 0 at 49 insns (m9) but is the banned four-handle construct respelled (identical post-preprocessing declarations), so it was banked as evidence only and NOT submitted. src/code6cac_b.c was left at its committed state; `git status` shows no src change.

- [s12] CLASSIFICATION CONTRADICTION now the real blocker: the target bytes are reachable (0 measured twice), wave K shows the four materialisations are forced, and the layer-1 reviewer's own prescribed remedy was to add the `/* FAKE: ... */` annotation to the four `u8 *q = &D_80106A73;` declarations - but the driver's BANNED-CONSTRUCTS list for this function bans that construct both 'claimed under the C-level pointer alias to a global / pointer-alias-fake-exception family' AND 'treated as ordinary program logic', i.e. it bans the remedy along with its alternative.

## s13 (permuter) — the inline-helper 13 is a two-stage codegen fact, and the permuter is dead a fourth time

- **The stall is located exactly.** A cc1 `-da` dump of the m1 inline-helper
  chassis (`tmp/grind/func_80034F88/s11/rtl/s13m1`, sliced by
  `tmp/grind/func_80034F88/s13/rtlslice.py`) shows that already at `.rtl` —
  before any optimisation pass — the three INLINED reads of the flag byte are
  `(mem:QI (symbol_ref "D_80106A73"))` while the caller's own non-inlined mask
  block reads `(mem:QI (reg/v:SI 74))`. All four stores, including the inlined
  ones, are still `(mem:QI (reg/v:SI N))` at `.rtl` and at `.combine`. Counts
  are flat across rtl/jump/cse/loop/cse2/combine, so **cse and combine are not
  the folding agents** — the substitution is done by integrate.c at inline time
  (`copy_rtx_and_substitute` + its const-equivalence map), and the stores escape
  it only because they sit after the if/else merge label where integrate bumps
  `map->const_age` and drops every equivalence.
- **The store folding is a downstream consequence.** With the read substituted,
  each inlined copy's pointer pseudo has one reference left, so local-alloc's
  `update_equiv_regs` records `reg_equiv_constant` and substitutes the address
  into the store as well — which is why the final asm shows `lui at,0x0 / sb
  v0,0(at)` despite `.combine` RTL that still stores through a register. The
  four-handle form escapes it because each of its pseudos carries three
  references (set + read + store) and stays in a hard register.
- **Wave N killed the lever.** Eight forms, all designed to give the pointer
  pseudo a second reference: read duplicated into both arms 35 (60 insns) and
  38 (66) with the arms swapped; select performed on the mask so the read is a
  single post-label RMW 34 (38 insns, branch structure lost); read through a
  pointer copy 13 (the const equivalence propagates straight through a
  pseudo-to-pseudo set — ties m1 exactly at 50 insns / lbu 176); handle assigned
  inside both arms 42 (68); nested inline read accessor 29 (56); three
  materialisations with the mask and flag block 1 sharing one caller-scope
  handle 24 at the target's exact 49 insns and lbu 175; that last crossed with
  the duplicated read 36 (55). **Nothing beats 13.** The label route does keep
  the pseudo, exactly as the mechanism predicts — it just materialises real
  extra loads (lbu census 177-179 vs the target's 176) that cost far more than
  the one or two instructions an unfolded base saves.
- **n7 is the instructive negative.** It reproduces the target's own base
  structure (`sbs_m1.txt` shows the target reloading block 1 through the mask's
  `v1`), lands on the target's exact instruction count, and still scores 24 with
  lbu 175: three address materialisations cannot produce the fourth reload. It
  is independent confirmation of wave K's price table from a different chassis.
- **Permuter, fourth kill.** Seeded from n7 (a structurally different chassis
  from s12b's m1, and the closest-by-instruction-count seed ever used):
  4,544 iterations drove the permuter's metric 1670 -> 675 while the six best
  finds sandbox-scored 26/27/33/33/33/34 against the seed's 24, every one of
  them carrying `sb` 162 against the correct 164 — the randomizer again buys its
  metric by deleting two flag stores. Harvested and stopped in-session
  (`tmp/grind/func_80034F88/s13/harvest.json`). Four kills now span every seed
  quality there is; seed structure is not the variable.
- **Net.** The honest floor for a form carrying no banned construct is unchanged
  at 13 (m1, tied by n4). F1 is closed. The only live item is F2 — the
  classification contradiction — which is a ruling-request, not a sweep.

- [s13] cc1 -da on the m1 inline-helper chassis: at .rtl the three inlined flag-byte reads are already (mem:QI (symbol_ref "D_80106A73")) while the caller's own mask read is (mem:QI (reg/v:SI 74)); the counts do not move through jump/cse/loop/cse2/combine, so integrate.c — not cse and not combine — performs the folding.

- [s13] All four stores, including the three inlined ones, are still (mem:QI (reg/v:SI N)) at .combine, yet the final asm emits `lui at,0x0 / sb v0,0(at)`; the only pass left is local-alloc's update_equiv_regs, so the store folding is a downstream consequence of the read folding leaving each pseudo with one reference.

- [s13] Wave N price table (score / build insns / lbu census, target = 49 insns / lbu 176): n1 35/60/179, n2 38/66/179, n3 34/38/173, n4 13/50/176, n5 42/68/179, n6 29/56/176, n7 24/49/175, n8 36/55/177, m1 baseline 13/50/176.

- [s13] n4 proves the integrate const equivalence propagates through a pseudo-to-pseudo copy: `u8 *r = q; v = *r;` scores exactly the same 13/50/176 as reading through q directly.

- [s13] n7 is independent confirmation of wave K from a different chassis: it reproduces the target's own base structure (target reloads flag block 1 through the mask's v1, per s12/sbs_m1.txt) and hits the target's exact 49-instruction count, and still scores 24 with lbu 175 — three address materialisations cannot produce the fourth reload.

- [s13] Permuter campaign s13 (n7 seed, 4,544 iterations, 73 finds): metric 1670 -> 675, every sandbox-scored find 26-34 against the seed's 24, all with sb 162 against the correct 164 (two flag stores deleted). Harvested and stopped in-session; no campaign left running.

- [s13] The honest floor for a form carrying no banned construct is unchanged at 13 (m1, tied by n4). src/code6cac_b.c was restored to its committed state; the only working-tree change is the engine's own metrics/events.jsonl append.

---

# s14 — forensics modality (best NON-BANNED floor 13 -> 10; the cse-flush mechanism named)

## Headline
The instrumented-cc1 `-da` dumps answer, for the first time, WHY the banned
four-declaration form reaches 0 and every other shape does not — and the answer
turns out not to be about declarations at all.  What produces the target's
"four address materialisations + surviving reloads" signature is a FRESH SET of
the address rtx after a cse value-table flush, and a plain RE-ASSIGNMENT of one
pointer local does that just as well as a new declaration.  A single `u8 *q`
re-assigned before the bit-2 and bit-4 blocks scores **10 at 49/49 insns**,
against 13 for s12b/s13's `static inline` helper — a real drop of the best
non-banned floor, reached by a construct that is not the banned one.

## The mechanism (RTL, not inference)
Artifacts: `tmp/grind/func_80034F88/s14/rtl/{w4,w1,r1,r12}/code6cac_b.i.*`
(cpp + the INSTRUMENTED `tools/gcc-2.7.2/cc1` on the real build flags, via
`s14/dump.sh`; sliced with `s14/slice.py` and `s14/flat.py`).

1. **The join labels ARE cse basic-block boundaries on this chassis.**  Each
   flag block is a TWO-ARMED `if/else`, so it emits a conditional branch, an
   unconditional jump, a barrier and two CODE_LABELs.  `cse_end_of_basic_block`
   ends the block at the join and the value table is flushed.  This CORRECTS an
   s2 conclusion that has been carried forward unchallenged for eleven sessions:
   s2 argued the join labels are not boundaries because cse.c:8102-8184 extends
   a block through a conditional branch when `LABEL_NUSES (JUMP_LABEL (p)) == 1`.
   That extension applies to a branch that SKIPS a block and falls through —
   true of s2's one-armed `val2 = val|K; if (!c) val2 = val;` chassis, false of
   the two-armed `if/else` chassis every form since s10 has used.  The dumps show
   the flush directly.
2. **A pseudo re-used across the flush loses the reloads.**  In `w1` (one handle,
   no re-assignment) the previous block's store and the next block's load are the
   identical address rtx `(mem:QI (reg 74))` inside the same post-flush block, so
   cse records the store and forwards it.  `w1`'s `.cse` dump has ONE
   `(set (reg 74) (symbol_ref "D_80106A73"))` and only FIVE QI mems (three loads
   gone).  Honest score 29.
3. **A fresh set after the flush restores BOTH halves at once.**  The new
   `(set (reg N) (symbol_ref "D_80106A73"))` finds nothing in the flushed table,
   so it survives as a real `lui`/`addiu` materialisation; and the previous
   block's store, addressed through a pseudo whose value the table no longer
   knows, hashes differently from the new block's load, so the load survives as a
   real `lbu`.  Reloads AND an unfolded shared base simultaneously, with zero
   volatile.  This is the concrete resolution of the tension s1-s3 recorded as
   "mutually exclusive" and s2 attributed to `MEM_VOLATILE_P`.
4. **The fresh set does not need a fresh DECLARATION** — this is the new result.
   `r1`'s `.cse` dump retains THREE `(set (reg/v:SI 74) (symbol_ref
   "D_80106A73"))` insns (14, 59, 93) and ALL EIGHT QI mems, from a single
   declared `u8 *q` re-assigned per block.  cse keys on whether the symbol_ref is
   in the (flushed) table, not on how many C objects exist.
5. **Corollary that explains a measurement.**  An assignment placed BEFORE a
   flush is deleted by cse as redundant: the bit-1 block's assignment in `r1` is
   absent from the `.cse` dump, which is why `r1` (assign in all three blocks)
   and `r8` (assign only in the bit-2 and bit-4 blocks) compile to byte-identical
   code and both score 10.

## The residual 10 points — the FIRST SEGMENT's register assignment only
Side-by-side (`s14/sbs.py r1.c`): the bit-2 block, the bit-4 block and the whole
copy loop are instruction- AND register-identical to target.  Everything that
differs is the mask + bit-1 segment — target puts the base in `a0` and the loaded
byte in `v1`, this build swaps them, and target's bit-1 block carries a
load-delay `nop` where this build has a memory op.  With ONE pseudo re-set three
times, local-alloc gives the pointer one hard register for the whole function, so
the first segment cannot be allocated differently from the rest.  FOUR DISTINCT
pseudos (the banned form) is what produces the target allocation.

## Measured this session (honest `sandbox --disable all`, 49-insn target)
| form | score | insns | note |
|---|---|---|---|
| `w4` four block-scoped declarations (BANNED — control only) | 0 | 49 | reproduces s12's result |
| **`r8` one `u8 *q`, re-assigned before the bit-2 and bit-4 blocks** | **10** | **49** | **new best non-banned** |
| `r1` same, re-assigned in all three blocks | 10 | 49 | byte-identical to `r8` (cse deletes the bit-1 set) |
| `r12` mask leaves the byte in a local, bit-1 reuses it, 2/4 re-assign | 10 | 49 | same code |
| `r15`/`r16` declaration-order permutations | 10 | 49 | neutral |
| `r17` mask split into read / and / write | 10 | 49 | neutral |
| `r22` mask as `*q = *q & 0xF8` · `r23` via a `u8 m` · `r24` read-before-condition | 10 | 49 | neutral — a hard plateau |
| `r11` re-assign before the bit-2 block only | 20 | 47 | |
| `r19` TWO pointer locals (q for mask+bit-1, q2 re-assigned for 2 and 4) | 21 | 49 | |
| `r6` `u8` byte local on the re-assign chassis | 21 | 51 | |
| `r20` THREE pointer locals | 23 | 49 | |
| `r3` re-assignment placed before the CURRENT block's store | 25 | 48 | |
| `r10` re-assign before the bit-4 block only | 27 | 49 | |
| `r4` mask on the plain symbol + three re-assignments | 16 | 51 | keeps all four reloads but costs 2 insns |
| `r13` handle initialised in its declaration, before the call | 29 | 51 | |
| `w1` one handle, no re-assignment | 29 | 49 | the forwarding baseline |
Note the axis is NOT monotone in handle count: 1 re-set handle = 10, two
declarations = 21, three = 23, four = 0.  "More handles is better" is false.

## Classification caveat (stated plainly, not self-approved)
`r8`'s two later `q = &D_80106A73;` assignments are value-redundant — `q`
already holds that address.  As written they are dead self-assigns to a LOCAL,
which is on the frozen SOTN-sanctioned list; but they are unmistakably in the
same INTENT family as the four repeated block-scoped declarations the driver
banned for this function, and the driver's standing rule is that a banned
construct respelled is the same construct.  s14 therefore does NOT submit it,
does NOT install it in src/, and banks it in candidate.c with this caveat
attached.  The ruling question is folded into the F2 escalation the ledger has
been carrying since s12.

- [s14] [s14] Best NON-BANNED honest floor moved 13 -> 10 (`sandbox func_80034F88 --disable all`, 49 build insns vs a 49-insn target). The form is ONE `u8 *q` declared once and ASSIGNED `&D_80106A73` three times — before the mask, and at the top of the bit-2 and bit-4 blocks. It is banked at memory/grind/func_80034F88/candidate.c and deliberately NOT installed in src/ (src/code6cac_b.c is left at its committed state, as s12b and s13 also left it).

- [s14] [s14] MECHANISM, read out of cc1 -da dumps rather than inferred: each flag block is a TWO-ARMED if/else, so it emits a conditional branch, an unconditional jump, a barrier and two CODE_LABELs; cse_end_of_basic_block ends the cse basic block at the join and FLUSHES the value table. A pointer pseudo re-used across the flush keeps an identical address rtx, so the previous store is recorded and forwarded into the next load (reload dies, one lui/addiu for the function — w1, score 29). A FRESH SET of the address after the flush finds nothing in the table, so it survives as a real lui/addiu materialisation AND the previous store no longer hashes equal to the next load, so the reload survives. Both halves at once, with zero volatile.

- [s14] [s14] THE NEW RESULT: that fresh set does not need a fresh DECLARATION. tmp/grind/func_80034F88/s14/rtl/r1/code6cac_b.i.cse retains THREE `(set (reg/v:SI 74) (symbol_ref "D_80106A73"))` insns (14, 59, 93) and all eight QI mems from a single declared `u8 *q` re-assigned per block, while s14/rtl/w1/code6cac_b.i.cse has one such set and only five QI mems. Eleven sessions of handle-COUNT sweeps missed this because they varied how many pointers were declared, never whether one pointer was re-set after a flush.

- [s14] [s14] CORRECTION to an s2 conclusion that has been carried forward since session 2: the if/else join labels ARE cse basic-block boundaries on the chassis in use since s10. s2's citation of the LABEL_NUSES == 1 block-extension at cse.c:8102-8184 is correct only for its own ONE-ARMED `if` chassis, where the branch skips a block and falls through. On the two-armed if/else the table is flushed at every join, and the .cse dumps show it.

- [s14] [s14] Corollary that explains an otherwise puzzling measurement: an address assignment placed BEFORE a flush is deleted by cse as redundant. The bit-1 block's assignment in r1 is absent from the .cse dump, which is why r1 (assign in all three blocks) and r8 (assign only in the bit-2 and bit-4 blocks) compile to byte-identical code and both score 10.

- [s14] [s14] The residual 10 points are ENTIRELY the first segment's register assignment. Side-by-side: the bit-2 block, the bit-4 block and the copy loop are instruction- and register-identical to target; the mask + bit-1 segment has target's base in a0 and byte in v1 where the build has base v1 and byte a0, and target's bit-1 block carries a load-delay nop where the build has a memory op. With ONE pseudo re-set three times, local-alloc gives the pointer one hard register for the whole function, so the first segment cannot be allocated differently from the rest; four DISTINCT pseudos (the banned form) is what produces the target allocation.

- [s14] [s14] The handle axis is NOT monotone: one re-set handle = 10, two declared pointer locals = 21, three = 23, four (banned) = 0, one handle with no re-assignment = 29. Any future session that reasons 'more handles is closer' is reasoning from a false premise.

- [s14] [s14] Full measurement set on the re-assign chassis (honest sandbox / build insns): r8 and r1 and r12 and r15 and r16 and r17 and r22 and r23 and r24 all 10/49; r4 (mask on the plain symbol + three re-assignments) 16/51 with all four reloads; r11 (re-assign before the bit-2 block only) 20/47; r19 (two pointer locals) 21/49; r6 (u8 byte local) 21/51; r20 (three pointer locals) 23/49; r3 (re-assignment before the CURRENT block's store) 25/48; r10 (re-assign before the bit-4 block only) 27/49; r13 (handle initialised in its declaration, before the call) 29/51; r7 (separate result local) 30/44; w1 29/49; w4 (banned control) 0/49.

- [s14] [s14] CLASSIFICATION CAVEAT, stated and not self-approved: r8's two later `q = &D_80106A73;` assignments are value-redundant — q already holds that address. As written they are dead self-assigns to a LOCAL, which is on the frozen SOTN-sanctioned list, but they are in the same INTENT family as the four repeated block-scoped declarations the driver banned for this function, and a banned construct respelled is the same construct. s14 therefore did not submit, did not install in src/, and folded the question into the standing F2 escalation.

- [s14] [s14] Reusable instruments left behind: tmp/grind/func_80034F88/s14/dump.sh (splice a variant into src/, cpp + the INSTRUMENTED tools/gcc-2.7.2/cc1 with -da, collect every RTL dump plus BB2_* debug stderr, restore src/), splice.py, slice.py (per-pass census of address pseudos / symbol-addressed mems / QI mems), flat.py (one compact line per insn from any -da dump), probe.py (honest sandbox + lbu/sb/lui census per variant), sbs.py. The instrumented cc1 exposes BB2_ALLOC_DEBUG / BB2_PRIO_DEBUG / BB2_FINDREG_DEBUG / BB2_RANK_DEBUG / BB2_QTY_DEBUG / BB2_SCHED_DEBUG / BB2_RELOAD_DEBUG and nine more env-gated hooks; s14 used none of them and they are the obvious instrument for the remaining allocation question.


## s15 (forensics) — the register allocation is now fully explained, and a THREE-object form measures 0

### Harness / artifacts
`tmp/grind/func_80034F88/s15/dump.sh <variant> [ENV=1 ...]` is s14's dump
harness re-pointed at `s15/rtl/`; it splices a variant from
`tmp/grind/func_80034F88/s14/variants/` into src/, runs cpp + the INSTRUMENTED
`tools/gcc-2.7.2/cc1` with `-da`, and restores src/. `s15/flatten.py <v> <pass>`
prints one line per insn of func_80034F88 out of any RTL dump. `s15/probe.py`
and `s15/sbs.py` are s14's, re-pointed at `s15/variants/`. Dumps taken this
session: r8, w4, r19, t1.

### VALIDITY CHECK, done first (bank this — it was never checked before)
`engine/buildconfig.py:19` compiles with `tools/gcc-2.7.2/build/cc1`, while the
INSTRUMENTED compiler is `tools/gcc-2.7.2/cc1` — two different binaries (md5
8837b7da… vs 29b10d86…). Every forensics session since s11 has read dumps from
the instrumented one and compared them against sandbox scores from the other.
`tmp/grind/func_80034F88/s15/xcheck.sh` compiles the same .i with BOTH and diffs
func_80034F88's asm: **IDENTICAL**. The instrumentation is print-only and the
dumps are representative. Re-run xcheck.sh if either binary is ever rebuilt.

### LEDGER CORRECTION — s14 recorded the residual's orientation backwards
s14's frontier says "target has base a0 / byte v1 there, the build has base v1 /
byte a0". It is the other way round. From `asm/funcs/func_80034F88.s` directly:
the mask + bit-1 segment is `lui $v1 / addiu $v1 / lbu $a0,0($v1) / andi $a0 /
sb $a0,0($v1) / lw $v0 / lbu $a0,0($v1)` — **target base $v1, byte $a0** — and
the bit-2 / bit-4 segments are base $a0, byte $v1. The floor-10 build has base
$a0 / byte $v1 in ALL THREE segments. (In `s15/sbs_t1.txt`, i.e. sbs.py's
output, the LEFT column is the TARGET and the RIGHT column is the build.)

### THE MECHANISM, named end to end
Three GCC decisions, all read out of the dumps, produce the target's register
assignment:

1. **cse.c value-table flush at the two-armed if/else join labels** (s14's
   finding, re-confirmed). A pointer SET after a flush survives as a real
   lui/addiu; the previous block's store then no longer hashes equal to the next
   block's load, so the reload survives. This is what moves lbu 173 to 176.

2. **cse.c rewrites a pointer set that is NOT after a flush into a register
   COPY.** In the banned four-declaration form w4, `s15/rtl/w4/code6cac_b.i.lreg`
   insn 27 is `(set (reg/v:SI 77) (reg/v:SI 74))` — the bit-1 block's handle is a
   copy of the mask block's handle, not a fresh symbol_ref. reg 74 is confined to
   one basic block, so **local-alloc.c** gives it a hard register before global
   allocation runs, and the copy gives allocno 77 a preference for that register.

3. **global.c `allocno_compare` + `find_reg`'s `regs_someone_prefers`.**
   Priority is `floor_log2(n_refs) * n_refs * 10000 * size / live_length`
   (global.c:643-648; the ALLOCDBG line prints exactly this). Measured, for
   func_80034F88:

   | form | pointer allocnos | pri | alloc order | result |
   |---|---|---|---|---|
   | r8 (one object re-set 3x) | one pseudo, 10 refs / len 29 | 10344 | ord 4, BEFORE the bytes | pointer takes $a0 for the whole function, all bytes $v1 — score 10 |
   | r19 = t2 (two objects, pointers declared first) | 6 refs / len 16 and 4 / 26 | 7500, 3076 | the 7500 ties the bytes and wins on allocno number | 21 |
   | t1 (two objects, values declared first) | same | same | the tie now goes to the BYTES | 10 |
   | w4 (banned, four declarations) | 3 refs / len 14 each | 2142 | ord 8-10, AFTER the bytes and after `p` | bit-1 byte pushed off $v1 onto $a0, pointer then takes $v1 = TARGET, score 0 |

   `find_reg` does `IOR_HARD_REG_SET (used, regs_someone_prefers[allocno])`
   (global.c:1001) — an allocno avoids registers that a conflicting,
   not-yet-allocated allocno prefers. That is the step that puts the bit-1 byte
   in $a0 in w4 and leaves $v1 for the copy-linked pointer.

### THE STRUCTURAL CONSEQUENCE (kills s14's frontier hypothesis outright)
GCC 2.7.2's global.c has **no live-range splitting**: one C pointer object gives
one DECL_RTL pseudo, one allocno and ONE hard register for the whole function.
The target uses TWO different hard registers for the flag-byte base ($v1 for the
mask + bit-1 segment, $a0 for bit-2 / bit-4). Therefore **no single-pointer-
object form can ever score below 10, whatever its live-range shape** — the s14
frontier's question ("which other live-range shape produces two allocnos without
two pointer objects") has the answer: none exists. Do not spend another session
on live-range spellings of one handle.

### THE DECLARATION-ORDER LEVER (new, and it is worth 11-13 points)
When the pointer allocno and the byte allocnos tie at pri 7500,
`allocno_compare` falls through to `return *v1 - *v2;` — the allocno number,
which follows pseudo number, which follows DECLARATION ORDER. Declaring the six
value locals before the pointer locals flips that tie:
  * two-object chassis: t2 (pointers first) **21** → t1 (values first) **10**
  * three-object chassis: v3 (pointers first) **13** → v1 (values first) **0**
Declaration order is on the frozen sanctioned list ("named-intermediate
declaration order"), but note the choice here is made for an allocator reason.
s3 measured "declaration order is neutral" — true on the one-object chassis,
where the single pointer allocno prices at 10344 and there is no tie to break.

### THE 0
`s15/variants/v1.c` — THREE function-scope pointer objects (`qm` for the mask,
`q1` set inside the bit-1 block, `q2` set inside the bit-2 block and re-set
inside the bit-4 block) plus the declaration-order lever — measures **score 0 at
49/49 insns, lbu 176 / sb 164 / lui 456**. `v2.c`, identical except that the
bit-1 handle is spelled `q1 = qm;` (an explicit pointer copy, which is what cse
produces from `q1 = &D_80106A73;` anyway), also measures **0**. Both are banked:
v1 at `memory/grind/func_80034F88/candidate.c`, v2 at `s15/variants/v2.c`.

**Neither was submitted.** The driver's ban covers "four separate
`u8 *q = &D_80106A73;` local pointer declarations (one per flag block)" under
both the pointer-alias family and the ordinary-program-logic reading. Three
function-scope objects with four assignments is not that construct literally,
but it is the same intent — extra C handles on one global whose only purpose is
to give the allocator more allocnos — and cheat-checklist T5 forbids a session
from self-approving a respelling. s15 therefore returned `ruling-request`.

### Forms measured this session (honest sandbox, `--disable all`)

    t1  two objects, values declared first .......... 10  (49 insns, lbu 175)
    t2  two objects, pointers declared first ........ 21  (= s14's r19)
    t3  values split around the pointers ............ 10
    t5  two objects, next handle staged pre-store ... 20  (lbu 174)
    t6  three objects, staged pre-store ............. 23
    t7  t6 with pointers declared first ............. 23
    t8  two objects, one staged one in-block ........ 10
    u1  symbol-spelled mask + two objects ........... 16  (51 insns, lbu 176)
    u2  symbol mask + three objects ................. 27
    u3  u1 with pointers declared first ............. 27
    u4  u1 with the mask written out longhand ....... 16
    v1  three objects, values declared first ........  0  (49 insns, lbu 176)
    v2  v1 with `q1 = qm;` ..........................  0
    v3  v1 with pointers declared first ............. 13
    v4  v2 with pointers declared first ............. 13

### Harness gotcha worth banking
`BB2_FINDREG_DEBUG` takes a **pseudo number**, not a boolean — `=1` silently
matches pseudo 1 and prints nothing. Use `BB2_FINDREG_DEBUG=<pseudo>` (or
`BB2_RELOAD_DEBUG=1`, which dumps every retrying call).

# ==================== s16 (forensics) ====================
Session-numbering note: the driver dispatched this session as "session 15", but
two earlier attempts already occupied the s15 scratch namespace — `tmp/grind/
func_80034F88/s15/` (the ruling-request session whose three-object score-0 form
the Judge FAILed on 2026-08-13 16:36) and `tmp/grind/func_80034F88/s15b/` (a
forensics session the driver DISCARDED for a scope violation, so nothing it found
was ever banked). This session's scratch is `s15c/`. Everything below was
measured or verified first-hand here; s15b material is cited only where this
session independently reproduced it.

## The target's own register assignment, read off asm/funcs/func_80034F88.s
The datum every earlier session inferred but the ledger never wrote down
literally. The flag section of the target is:

    lui  $v1,%hi(D_80106A73) ; addiu $v1,$v1,%lo(D_80106A73)   <- materialisation #1
    lbu  $a0,0x0($v1) ; andi $a0,$a0,0xF8 ; sb $a0,0x0($v1)    <- the 0xF8 mask
    lw   $v0,0x20($a1) ; lbu $a0,0x0($v1) ; andi $v0,$v0,0x1
    ori  $v0,$a0,0x1
    lui  $a0,%hi(D_80106A73) ; addiu $a0,$a0,%lo(D_80106A73)   <- materialisation #2
    sb   $v0,0x0($v1)                                          <- bit-1 store, base $v1
    lw   $v0,0x20($a1) ; lbu $v1,0x0($a0) ; andi $v0,$v0,0x2
    ori  $v0,$v1,0x2 ; sb $v0,0x0($a0)                         <- bit-2, base $a0
    lui  $a0,%hi(D_80106A73) ; addiu $a0,$a0,%lo(D_80106A73)   <- materialisation #3
    lw   $v0,0x20($a1) ; lbu $v1,0x0($a0) ; andi $v0,$v0,0x4
    ori  $v0,$v1,0x4 ; sb $v0,0x0($a0)                         <- bit-4, base $a0
    ... lbu $v0,0x17($v0) ; lui $at,%hi(D_80106A70) ; sb $v0,%lo(D_80106A70)($at)

Read structurally: the target holds &D_80106A73 in EXACTLY TWO live ranges — one
in $v1 spanning the mask block and the whole bit-1 block, and one in $a0 that is
SET TWICE (once before the bit-2 block, once before the bit-4 block, byte-
identical materialisations). The trailing 3-byte copy loop uses no base local at
all: `%lo(D_80106A70)($at)`, the assembler's own scratch. `p` lives in $a1, and
the loaded byte alternating $a0 / $v1 between the first segment and the other two
follows from the base choice, it is not an independent degree of freedom.

Two live ranges of one address, each with its own hard register, is the entire
10-point residual.

## The single-object ceiling is a structural fact of GCC 2.7.2, verified in source
Verified first-hand this session (not inherited):
* `tools/gcc-2.7.2/global.c:426` — `reg_allocno[i] = max_allocno++;`. One allocno
  per pseudo. The only aliasing path is the `reg_may_share` branch two lines
  above, which MERGES two pseudos into one allocno; there is no split anywhere.
  `find_reg` then gives each allocno a single hard register for its whole live
  range, and 2.7.2 has no live-range-splitting pass at all.
* `tools/gcc-2.7.2/reload.c:4128-4137` — the one escape (spill the pointer and let
  reload rematerialise its `REG_EQUIV (symbol_ref "D_80106A73")` into a different
  reload register per reference) is closed in source:

      if (GET_CODE (ad) == REG) { regno = REGNO (ad);
        if (reg_equiv_constant[regno] != 0
            && strict_memory_address_p (mode, reg_equiv_constant[regno]))
          { *loc = ad = reg_equiv_constant[regno]; return 1; }

  On MIPS a bare symbol_ref IS a legitimate memory address, so the MEM's address
  is replaced by the constant instead of being reloaded into a register: the
  access degenerates to the plain-symbol form (`lbu $r,SYM`), the measured 28-29
  family, never an addend-0 base.

Therefore two hard registers for this address require two allocnos, which require
two pseudos, which — given the kill below — require two C pointer objects. That
is the banned construct by construction, not by spelling.

## NEW MEASUREMENT — the anonymous-reference escape is dead (4 forms)
The only route to a second base that is neither a DECLARED nor an ASSIGNED C
pointer object is an ANONYMOUS reference: a direct symbol access, or pointer
arithmetic yielding an unnamed temporary. Four spellings measured on the r8
single-object chassis (`tmp/grind/func_80034F88/s15c/variants/*.c`,
`s15c/results.json`; honest `sandbox func_80034F88 --disable all`; target census
49 insns / lbu 176 / sb 164 / lui 456):

    r8.c  single object `q`, re-assigned before the bit-2 and bit-4 blocks
                                            score 10   49 insns  lbu 175 lui 456
    a1.c  q covers mask+bit-1; bit-2 and bit-4 use `D_80106A73` directly
                                            score 28   48 insns  lbu 174 lui 457
    a2.c  mask+bit-1 use `D_80106A73` directly; q covers bit-2 and bit-4
                                            score 14   49 insns  lbu 175 lui 458
    a3.c  r8 chassis with only the bit-4 block spelled as a direct reference
                                            score 16   49 insns  lbu 175 lui 457
    a4.c  bit-2/bit-4 spelled `*((u8 *)&D_80106A70 + 3)` — arithmetic off the
          neighbouring symbol, hoping for an unfoldable temporary
                                            score 28   49 insns  lbu 174 lui 457

Every anonymous spelling is WORSE than the 10-point single-object floor, and the
lui census RISES (456 -> 457/458) instead of holding the target's 456: each
anonymous access pays its own materialisation rather than sharing a base.

## Forensic cause, from the instrumented cc1 (not inferred from the score)
`tmp/grind/func_80034F88/s15c/rtl/a2/cc1.err:637-646` (BB2_ALLOC_DEBUG on a2, the
best anonymous form) lists this function's allocnos:

    ord=0 pseudo=73 hardreg=3  nrefs=11 livelen=7  pri=47142
    ord=1 pseudo=82 hardreg=2  nrefs=5  livelen=7  pri=14285
    ord=2 pseudo=86 hardreg=2  nrefs=5  livelen=7  pri=14285
    ord=3 pseudo=78 hardreg=3  nrefs=5  livelen=11 pri=9090
    ord=4 pseudo=77 hardreg=2  nrefs=3  livelen=4  pri=7500
    ord=5 pseudo=81 hardreg=3  nrefs=3  livelen=4  pri=7500
    ord=6 pseudo=85 hardreg=3  nrefs=3  livelen=4  pri=7500
    ord=7 pseudo=74 hardreg=4  nrefs=6  livelen=17 pri=7058   <- the ONE pointer q
    ord=8 pseudo=72 hardreg=5  nrefs=6  livelen=37 pri=3243   <- p

It is the SAME pseudo set as the single-object control r8 (73/78/82/86/77/81/85/
74/72 — nine allocnos, one pointer pseudo): replacing three of q's uses with
anonymous references created NO new pseudo. GCC 2.7.2 expands a direct symbol MEM
with the symbol_ref inline in the address, so an anonymous reference never
occupies a register — no pseudo, no allocno, no second hard register. The
generated asm confirms it (`s15c/rtl/a2/code6cac_b.s`): `lbu $2,D_80106A73` /
`sb $2,D_80106A73` / `sb $3,D_80106A73` for the direct segment, then `0($4)` for
the bit-2 and bit-4 segments — one base register, not two.

Net: the pointer-OBJECT count is not a spelling choice. It is the only dial GCC
2.7.2 exposes for the number of base registers, and the Judge has banned turning
it past one.

## Gate-1 datum (re-run and re-verified this session)
`python3 tools/scan_hand_coded.py --single func_80034F88` ->
`HAND_CODED: tier=LOW  score=0/8  ("no strong hand-coded indicators")`, all eight
signals unchecked (0 multu/mflo pairs, no empty-body branches, no BIOS jumptable
pattern, no high-similarity siblings, all callee-save uses saved, no redundant
mask-before-shift, max load burst 3 in any 8-insn window, 1 spill / 4 distinct
registers over 49 instructions). Banked here because the session that previously
ran it was discarded, so the result never reached the ledger. It is dispositive
against a canonical-asm disposition under endgame-lock-disposition gate 1 — as
one should expect, since the residual is an ordinary GCC register-allocation
tie-break, the opposite of a hand-coded signature.

## Ledger hygiene performed this session
`memory/grind/func_80034F88/candidate.c` had been left holding the THREE-object
score-0 body the Judge FAILed on 2026-08-13 16:36 (its own header says "DO NOT
INSTALL AND SUBMIT"). That is a trap for a future session, and candidate.c is
specified as the best ADMISSIBLE form. candidate.c now holds the single-pointer-
object r8 body at honest 10; the FAILed three-object body remains banked at
`memory/grind/func_80034F88/rejected/three-pointer-objects-judge-FAIL-score0.c`.

- [s15] Target register assignment, read literally off asm/funcs/func_80034F88.s for the first time in this ledger: &D_80106A73 lives in EXACTLY TWO live ranges — $v1 spanning the 0xF8 mask and the whole bit-1 block (lui/addiu $v1 once), and $a0 SET TWICE with byte-identical lui/addiu materialisations, once before the bit-2 block and once before the bit-4 block. The trailing 3-byte copy loop uses no base local at all (sb $v0,%lo(D_80106A70)($at)); p lives in $a1.

- [s15] tools/gcc-2.7.2/global.c:426 is `reg_allocno[i] = max_allocno++;` — one allocno per pseudo. The only aliasing path is the reg_may_share branch two lines above, which MERGES allocnos; there is no split anywhere, and GCC 2.7.2 has no live-range-splitting pass. find_reg gives each allocno one hard register for its whole live range.

- [s15] tools/gcc-2.7.2/reload.c:4128-4137 closes the spill/rematerialise escape in source: a REG address whose pseudo carries a reg_equiv_constant that is itself a legal memory address has the MEM ADDRESS replaced by the constant (*loc = ad = reg_equiv_constant[regno]; return 1;) rather than being reloaded into a register. On MIPS a bare symbol_ref is a legal address, so the access degenerates to the plain-symbol form.

- [s15] Anonymous-reference sweep on the r8 single-object chassis, honest sandbox (target census 49 insns / lbu 176 / sb 164 / lui 456): r8 = 10 (49, lbu 175, lui 456); a1 = 28 (48, lbu 174, lui 457); a2 = 14 (49, lbu 175, lui 458); a3 = 16 (49, lbu 175, lui 457); a4 = 28 (49, lbu 174, lui 457). Every anonymous spelling is worse than the single-object floor AND raises the lui census above the target's 456.

- [s15] BB2_ALLOC_DEBUG trace for a2 (tmp/grind/func_80034F88/s15c/rtl/a2/cc1.err:637-646) lists nine allocnos — 73(hr3) 82(hr2) 86(hr2) 78(hr3) 77(hr2) 81(hr3) 85(hr3) 74(hr4, the one pointer) 72(hr5, p) — the identical pseudo set to the single-object control. Replacing three pointer uses with anonymous references created NO new pseudo: GCC 2.7.2 never puts a direct symbol_ref into a register on MIPS, so an anonymous reference cannot be a base register.

- [s15] Generated asm for a2 (s15c/rtl/a2/code6cac_b.s) confirms it at instruction level: lbu $2,D_80106A73 / sb $2,D_80106A73 / sb $3,D_80106A73 for the direct segment, then 0($4) for the bit-2 and bit-4 segments — one base register, not two.

- [s15] Gate-1 datum re-run and banked this session (the prior session that ran it was discarded, so it never reached the ledger): python3 tools/scan_hand_coded.py --single func_80034F88 -> HAND_CODED: tier=LOW score=0/8, all eight signals unchecked (0 multu/mflo pairs, no empty-body branches, no BIOS jumptable pattern, no high-similarity siblings, all callee-save uses saved, no redundant mask-before-shift, max load burst 3 in any 8-insn window, 1 spill / 4 distinct registers over 49 insns). Dispositive against a canonical-asm disposition under endgame-lock-disposition gate 1.

- [s15] Ledger hygiene: memory/grind/func_80034F88/candidate.c had been left holding the THREE-pointer-object score-0 body that the layer-1 reviewer FAILed on 2026-08-13 15:24 and the Judge FAILed on 2026-08-13 16:36 (its own header reads 'DO NOT INSTALL AND SUBMIT') — a trap in the slot reserved for the best ADMISSIBLE form. candidate.c now holds the single-pointer-object r8 body at honest 10; the FAILed body remains banked in rejected/.

- [s15] src/code6cac_b.c was left byte-identical to its committed state this session (the probe harness restores it after every splice); no build-pipeline file, rule file, or engine file was touched.

==== s16 (rederive) ====

Modality: rederive (fresh m2c decompile, matched-corpus sibling transplant,
structurally different C shapes). Floor unchanged at 10; the session's product
is four independent kills plus the first CORPUS-LEVEL evidence about how this
codegen shape is spelled in already-matched BB2 code.

**1. The 10-point residual is entirely in BLOCK 1, and it is TWO defects, not
one.** Full instruction-aligned side-by-side of the banked candidate against
target (tmp/grind/func_80034F88/s16/, `sbs.py base.c`; both 49 insns):

    target                        our build
    lui  v1,%hi(D_80106A73)   |   lui  a0,%hi(D_80106A73)
    addiu v1,v1,%lo           |   addiu a0,a0,%lo
    lbu  a0,0(v1)             |   lbu  v1,0(a0)
    move a1,v0                |   move a1,v0
    andi a0,a0,0xf8           |   andi v1,v1,0xf8
    sb   a0,0(v1)             |   sb   v1,0(a0)
    lw   v0,32(a1)            |   lw   v0,32(a1)
    lbu  a0,0(v1)   <-RELOAD  |   nop             <-RELOAD MISSING
    andi v0,v0,0x1            |   andi v0,v0,0x1
    bnez v0,.L                |   bnez v0,.L
    ori  v0,a0,0x1            |   ori  v0,v1,0x1
    move v0,a0                |   move v0,v1
    (join) lui a0 / addiu a0  |   sb   v0,0(a0)
    sb   v0,0(v1)             |   lui  a0 / addiu a0

Blocks 2 and 4 and the whole trailing 3-byte copy loop are INSTRUCTION- AND
REGISTER-IDENTICAL. The residual decomposes as: (a) block 1's base register is
$a0 in our build and $v1 in the target, with the byte value swapped the other
way (5 lines); (b) our build has no post-store reload in block 1 (the masked
value stays live in a register), so a maspsx load-delay nop lands where the
target's second `lbu` is (lbu census 175 vs the target's 176); (c) the block-1
store then floats to the other side of block 2's address materialisation.

**2. A SECOND, independent derivation of the ceiling — from cse.c, not from
global.c.** s15 closed the ceiling from the register-allocator side (one allocno
per pseudo, no live-range splitting). The missing block-1 reload closes it again
from the CSE side, and the two are the same wall seen from two directions:
GCC 2.7.2's cse hashes a MEM on its ADDRESS RTX. A store and a later load whose
addresses are the same pseudo hash to the same entry, so the load is always
satisfied from the stored register and the reload is folded away. Nothing
between the block-1 store and the block-1 read invalidates that entry — the only
intervening insn is a LOAD (`lw v0,32(a1)`), and loads do not invalidate. Blocks
2 and 4 get their reloads for free only because a two-armed if/else JOIN LABEL
sits between their store and their read and flushes the value table (the s14
finding). Block 1 has no preceding join, so the ONLY way to miss the hash entry
is a DIFFERENT address rtx — i.e. a second address pseudo — i.e. a second C
object, which is exactly the banned construct. One C local is one DECL_RTL is
one pseudo in GCC 2.7.2; restating `q = &D_80106A73;` does not create a second
one (measured: identical output).

**3. The register dial ($v1 vs $a0 for the single pointer allocno) is KILLED
three independent ways.** This was the last unmeasured intra-constraint dial on
the s15 frontier.
  - Declaration order is INERT. Pointer declared first / middle / last, and all
    block temporaries hoisted to function scope: d1 / d2 / d3 / f1 all score 10
    with an identical instruction stream (tmp/grind/func_80034F88/s16/results.json).
  - Live-range length does not free $v1. Confining the pointer to block 1 alone
    makes it win $a1 (not $v1), score 28; confining it to blocks 2+4 leaves it on
    $a0, score 14. Banked as rejected/pointer-live-block1-only-score28.c and
    rejected/pointer-live-blocks23-only-score14.c.
  - Even the ILLEGITIMATE route fails. `register u8 *q asm("$3")` (diagnostic
    only, never committable) is IGNORED by GCC 2.7.2 for a local that never
    appears as an asm operand — the emitted stream is instruction-identical to
    the unpinned form, score 10, same census. So there is no shape, legal or
    otherwise, that puts the single pointer allocno on $v1.
  The counterfactual is also now arithmetic rather than speculation: $v1 would
  repair block 1's five swapped lines and BREAK the five equivalent lines in each
  of blocks 2 and 4, which currently match exactly — net +5. One register cannot
  serve both segments in either direction.

**4. m2c's fresh reconstruction is NOT the original shape.** m2c (mipsel-gcc-c
target) reconstructs the function with no pointer object at all and with the OR
computed first and the else-arm overriding it (`v = D_80106A73 | 1; if (!(p[8] &
1)) v = D_80106A73; D_80106A73 = v;`). Transcribed literally it scores 26 at 45
build insns; carried onto the single-pointer chassis it scores 21, also at 45
insns (u8 or s32 temporaries alike). In every spelling GCC folds the two reads
of the byte together and then collapses the diamond, so the build comes out FOUR
instructions SHORT of the target's 49. The symmetric `if (c) c = v | bit; else
c = v;` in candidate.c is the shape that reproduces the target's diamond; m2c's
is not a lever, it is a worse shape.

**5. CORPUS EVIDENCE — how already-matched BB2 code spells "one global in two
base registers".** New tool: tmp/grind/func_80034F88/s16/sibling_scan.py and
sibling_scan2.py sweep all 1,437 asm/funcs/*.s for functions that materialise
ONE symbol into TWO OR MORE distinct registers via lui/addiu, then keep only
those where both registers are genuinely used as a memory BASE (offset($reg)),
excluding address-as-a-value cases. Result: 34 functions in the whole binary have
the construct; 9 of them are NOT in engine/queue.json, i.e. already COMPLETED
with zero regfix/asmfix rules — SpuSetKey, SpuSetReverbModeParam, func_8002C0DC,
func_80037F40, func_8004A940 (x2 symbols), func_8008241C, func_80082D34,
startIntrDMA, startIntrVSync.
  The closest and most instructive is **func_80037F40** (src/code6cac_c_mid.c,
zero rules, matched in commit 89bfc882): its target asm puts D_80106A50 in BOTH
$v1 and $a2 as base registers, and its accepted pure-C body reaches that with
TWO C pointer objects on the one global —
    src/code6cac_c_mid.c:196   p   = (u8 *)&g_file_disc_size;      /* checksum byte walker */
    src/code6cac_c_mid.c:211   src = (Quad *)&g_file_disc_size;    /* block-copy source */
  Two handles, but of DIFFERENT TYPES doing DIFFERENT JOBS, and the commit
message does not even mention them — they were ordinary program logic, not a
technique. That is the distinction the Judge drew for func_80034F88: four
identical `u8 *q = &D_80106A73;` handles with identical jobs have no such
semantic differentiation and read as a register-allocation lever. The census
result to carry forward is that in the ENTIRE matched corpus there is no example
of the construct being reached WITHOUT multiple C handles; the two-base-register
codegen and the multiple-C-object source are one-to-one.

- [s16] The 10-point residual is entirely inside BLOCK 1. Instruction-aligned side-by-side (49 build insns vs 49 target): blocks 2 and 4 and the whole trailing 3-byte copy loop are INSTRUCTION- AND REGISTER-IDENTICAL to target. Block 1 contributes (a) 5 lines where our base register is $a0 and the loaded byte $v1 while the target has them the other way round, (b) one missing post-store reload -- a maspsx load-delay nop sits where the target has a second `lbu a0,0(v1)` -- and (c) the block-1 store floating to the other side of block 2's address materialisation.

- [s16] lbu census 175 against the target's 176 pins the missing instruction exactly: it is block 1's reload, and only block 1's. sb 164 and lui 456 already match target exactly.

- [s16] Declaration order of the pointer relative to the other locals is completely INERT for this function: pointer-first, pointer-middle, pointer-last, and all-temporaries-at-function-scope produce an identical instruction stream at score 10.

- [s16] `register u8 *q asm("$3")` is IGNORED by GCC 2.7.2 here -- the emitted stream is instruction-identical to the unpinned candidate. (Diagnostic only; the pin is a forbidden family and is banked purely as the recorded measurement in rejected/register-asm-pin-v1-ignored-diagnostic-score10.c.) Even the illegitimate route cannot move the allocno.

- [s16] Confining the pointer's live range to block 1 alone makes it win $a1, not the target's $v1 (score 28); confining it to blocks 2+4 leaves it on $a0 (score 14). Live-range length does not free $v1 either.

- [s16] $a0 is the BETTER of the two choices, by arithmetic on the measured side-by-side: it makes blocks 2 and 4 exact (0 diffs) at a cost of 5 lines in block 1, whereas $v1 would repair 5 and break 10.

- [s16] cse.c mechanism for the missing reload: GCC 2.7.2 hashes a MEM on its address rtx, so a store and a later load through the SAME pseudo hit one entry and the load is folded to the stored register. Blocks 2 and 4 keep their reloads only because a two-armed if/else join label flushes the value table first (s14); block 1 has no preceding join and the one intervening insn is a load, which does not invalidate. A second address pseudo -- hence a second C object -- is the only escape.

- [s16] New reusable tooling: tmp/grind/func_80034F88/s16/sibling_scan.py and sibling_scan2.py census the whole binary for 'one symbol materialised into N distinct base registers', filtered to zero-cheat COMPLETED functions. Useful for any future function with this shape.

- [s16] Corpus result: 34 functions in the binary have one symbol in 2+ base registers; 9 are COMPLETED with zero rules; every one of those that was inspected reaches it with MULTIPLE C handles on the global. func_80037F40 (src/code6cac_c_mid.c:196 and :211, commit 89bfc882) is the cleanest in-tree precedent -- two handles of different types doing different jobs (a checksum byte-walker and a block-copy source), incidental enough that the match commit never mentions them. That contrast is exactly the Judge's distinction: func_80034F88's four handles are the same type doing the same job, which is what makes them read as an allocation lever rather than program logic.

- [s16] m2c's fresh reconstruction (tools/m2c/m2c.py --target mipsel-gcc-c) uses no pointer object and an asymmetric value shape; every transcription of it lands 4 instructions short of the target at scores 21-26. It is not a lever.


==== s17 (rederive) ====

Modality: rederive. Baseline re-measured this session: candidate.c's
single-pointer-object body scores 10 at 49 build insns, lbu 175 / sb 164 /
lui 456 (variant a0_base, tmp/grind/func_80034F88/s17/results.json). Thirteen
bodies measured in total, all with AT MOST ONE C pointer object aliasing
D_80106A73 (the Judge's binding constraint).

-- 1. s2's kill of the "one declared 4-byte object" model (s1's F3) rested on
      evidence that does not exist, and the real answer is "codegen-neutral".

s2 recorded F3 as "Refuted directly by the shipped relocations: the flag
accesses carry R_MIPS_HI16/LO16 against D_80106A73 while the copy-loop store
carries R_MIPS_HI16/LO16 against D_80106A70. Two distinct symbols in the
relocation records means the original source declared two distinct objects. No
measurement needed."

There are no relocation records. disc/SLUS_006.63 is a PS-X EXE (verified this
session from the file magic), a flat absolutely-linked image with no reloc
table; the %hi/%lo forms in asm/funcs/func_80034F88.s are splat's
RECONSTRUCTION, pairing lui/addiu immediates and naming the sum from its own
symbol table, which carries D_80106A70..D_80106A73 as four consecutive one-byte
auto-names in undefined_syms_auto.txt:981-984. This is exactly the failure mode
memory/splat-symbol-names-are-not-evidence.md warns about. F3 was therefore
never actually answered; it is UNFALSIFIABLE from the binary.

Answered by measurement instead. Variant a1_arr_all spells every flag address
`&D_80106A70 + 3`:

    a1_arr_all   score 12   49 insns   lbu 175  sb 164  lui 456
    a0_base      score 10   49 insns   lbu 175  sb 164  lui 456

Identical instruction count and identical census. tmp/grind/func_80034F88/s17/
reloc.py disassembles the sandbox object with relocations: a1's three base pairs
are R_MIPS_HI16/LO16 against D_80106A70 with an in-field addend of 3, base's are
the same relocs against D_80106A73 with addend 0. 0x80106A70 + 3 == 0x80106A73
and %hi is 0x8010 either way, so the LINKED bytes are byte-identical. The +2 is a
FALSE distance -- engine/score.py masks branch/jump targets but not R_MIPS_LO16
addends (memory/sandbox-lo16-text-addend-false-distance.md).

CONCLUSION, on real evidence: the one-4-byte-object model and the four-scalars
model are indistinguishable in linked bytes for this function. GCC folds the +3
into the pointer's own materialisation (`la reg, D_80106A70+3`), not into a MEM
displacement, so the pointer still holds 0x80106A73 and every access is at
displacement 0 exactly as in the target. The model cannot help and cannot hurt;
F3 is closed for the right reason. Mixed-addend spellings on ONE object
(a2_arr_first 10, a3_arr_last 12) confirm it: the addend is invisible once the
address is in a pointer, because the MEM is (mem:QI (reg q)) either way.

-- 2. The missing block-1 reload is repriced from +5 to +1, and the reprice is
      itself the proof that the ceiling is the address-pseudo count.

The 10-point residual is (a) block 1's base register being $a0 where the target
uses $v1, and (b) one missing post-store reload (our lbu 175 vs the target's
176). Buying (b) needs block 1's flag READ to have an address rtx that differs
from the mask store's, so cse's value table misses. With one pointer object the
only such rtx available is the bare symbol, and this session priced every
placement of it:

    e1  block 1 READ via symbol only      11   50 insns  lbu 176  sb 164  lui 457
    e2  mask STORE via symbol only        15   50        lbu 176  sb 164  lui 457
    e3  block 1 STORE via symbol only     18   50        lbu 175  sb 164  lui 457
    d1  whole mask via symbol             16   51        lbu 176  sb 164  lui 458
    d2  whole block 1 via symbol          15   52        lbu 176  sb 164  lui 458
    d3  whole mask via symbol, addend 3   16   51        lbu 176  sb 164  lui 457
    d4  whole block 3 via symbol          16   49        lbu 175  sb 164  lui 457

e1 is the new cheapest: it reproduces the target's lbu 176 AND sb 164 exactly --
the first form on this chassis to carry the target's full access census -- and
its entire cost is one `lui $at` (50 insns vs 49, lui 457 vs 456). The
side-by-side (s17 sbs.py e1_b1read_sym.c) shows the difference is precisely
`lui v1,%hi / lbu v1,%lo(v1)` where the target has a single `lbu a0,0($v1)`.

That is a clean statement of the ceiling. Base spends exactly 1 point on the
absent reload; e1 spends exactly 1 point on the lui that buys it. To get the
reload for free the block-1 read must go through a REGISTER whose address rtx
differs from the mask store's -- i.e. a second address pseudo -- i.e. a second C
pointer object, which is the banned construct. The trade is exactly balanced
because the compiler charges one address materialisation either way.

-- 3. The unrolled three-if/else shape is confirmed as the original source shape.

r1_bitcopy `*q = (*q & 0xF8) | (p[8] & 7);` scores 34 at 27 insns; r2_accum
(three ifs ORing into one value, one store) scores 30 at 35 insns. The target is
49 insns with four lbu / four sb on the flag byte and three `bnez` selects
(`andi $v0,$v0,K / bnez / ori $v0,$a0,K / addu $v0,$a0,$zero`). No branchless or
single-store spelling can produce that; the source really was three separate
read-modify-write blocks. This closes the "maybe the semantics are simpler than
we assume" line of re-derivation.

-- 4. Frontier bullet 2 (a second address object derived from a genuinely
      different program value) is KILLED.

The only non-constant value in the function is p = func_80077D00(). Read at
src/text1b_b.c:898, that function is `s32* func_80077D00(void) { return
&D_8009BD24; }` -- it returns 0x8009BD24. D_80106A73 - 0x8009BD24 = 0x6AD4F, an
arbitrary distance across a segment boundary (0x8009BD24 is in .data, which ends
~0x800A3800; 0x80106A73 is bss). There is no fixed structural offset a human
would ever have written, so there is no non-constant expression in this function
that yields &D_80106A73. The bullet is closed as predicted, without
manufacturing an opaque zero.

-- 5. Sibling context (not a lever, recorded for the next session).

The next function in the file, func_8003504C (src/code6cac_b.c:3938, still in
the queue with 10 regfix rules, so NOT a proven-spelling precedent), reads the
SAME status word: `D_80102786 = ((u32)p[8] >> 3) & 1;`. So p[8] is a bitfield
and func_80034F88 mirrors its bits 0..2 into D_80106A73's bits 0..2. It also
carries `base = &D_80102785; ptr = (u8 *)(base - 9);` -- two pointer objects
derived from one global region -- but as an unmatched, rule-carrying function it
is evidence of nothing.

- [s17] Baseline re-measured: candidate.c's single-pointer-object body (variant a0_base) scores 10 at 49 build insns, lbu 175 / sb 164 / lui 456, against the target's 49 / 176 / 164 / 456. Floor unchanged at 10.

- [s17] Thirteen bodies measured this session, every one with AT MOST ONE C pointer object aliasing D_80106A73 (the Judge's binding constraint). Full table in memory/grind/func_80034F88/evidence.md under '==== s17 (rederive) ===='.

- [s17] NEW BEST-CENSUS FORM: e1 (block 1's flag READ alone spelled as the direct symbol D_80106A73, everything else through the single pointer q) scores 11 at 50 insns with lbu 176 and sb 164 - the target's EXACT access census, the first single-object form on this chassis to reach it. Its entire excess is one 'lui $at' (lui 457 vs 456).

- [s17] The reload/lui trade is exactly one-for-one: the floor-10 base loses exactly 1 point to the absent reload, and e1 pays exactly 1 point for the lui that buys the reload back. That balance is not a coincidence - the compiler charges one address materialisation either way - so it is an arithmetic restatement of the ceiling rather than a plateau observation.

- [s17] s2's kill of the 4-byte-single-object model (s1's F3) cited 'the shipped relocation records'. disc/SLUS_006.63 is a PS-X EXE with no relocation table; asm/funcs/*.s %hi/%lo symbol names are splat's reconstruction from absolute immediates against undefined_syms_auto.txt:981-984, which auto-names D_80106A70..73 as four consecutive single bytes. The premise was false and the hypothesis had never been tested.

- [s17] Answered properly: spelling every flag address '&D_80106A70 + 3' gives 49 insns and lbu 175 / sb 164 / lui 456 - identical to base in every respect - and objdump -r shows the difference is only R_MIPS_HI16/LO16 against D_80106A70 with an in-field addend of 3 vs against D_80106A73 with addend 0. Since 0x80106A70+3 == 0x80106A73 and %hi is 0x8010 either way, the LINKED bytes are identical; the sandbox's +2 is a false distance from the unmasked LO16 addend (memory/sandbox-lo16-text-addend-false-distance.md). The declaration model is codegen-neutral and cannot close the function.

- [s17] GCC folds a constant addend into the pointer's own materialisation ('la reg, D_80106A70+3'), NOT into the MEM displacement, so every access remains at displacement 0 exactly as in the target. This also means the addend is invisible to cse once the address is in a pointer, because the MEM is (mem:QI (reg q)) regardless of how q was initialised - which is why mixed-addend spellings on one object (a2 10, a3 12) buy nothing.

- [s17] The register dial was re-tested on the new e1 chassis and is inert there too: f2 (q declared first), f4 (read hoisted above the condition) and f5 (u8 read temporary) all score 11 at 50 insns with lbu 176 / sb 164 / lui 457, bit-for-bit e1's own result. f1 (q assigned before the call) 29 / 52, f3 (mask via symbol) 15 / 50.

- [s17] The target's three-if/else shape is confirmed as the original source shape, not an artefact of the chassis: the branchless bit-copy is 34 at 27 insns and the OR-accumulation form is 30 at 35, against the target's 49 insns with four lbu / four sb on the flag byte and three bnez selects.

- [s17] func_80077D00 is 's32* func_80077D00(void) { return &D_8009BD24; }' (src/text1b_b.c:898), so the function's only non-constant value is 0x8009BD24 - 0x6AD4F away from D_80106A73 across a segment boundary. There is no non-constant expression in this function that yields the flag address.

- [s17] Sibling context (not a lever): the next function in the file, func_8003504C (src/code6cac_b.c:3938), reads the same status word with 'D_80102786 = ((u32)p[8] >> 3) & 1;', so p[8] is a bitfield and func_80034F88 mirrors its bits 0..2 into D_80106A73's bits 0..2. func_8003504C still carries 10 regfix rules, so its own two-pointers-from-one-global spelling is evidence of nothing.

- [s17] src/ was restored clean after every probe (git status shows only memory/grind ledger files, the three new rejected/ forms, and metrics/events.jsonl).

==== s18 (synthesis) ====

MODALITY BRIEF: re-read the whole ledger, write the best MERGED attack, reset
the frontier. Two things came out of the merge: one previously-unmeasured hole
in the ceiling proof (now closed by measurement, in both directions), and one
proof that no session had stated, which materially changes what the escalation
packet is asking the owner.

--- 1. THE HOLE IN THE CEILING PROOF, AND ITS CLOSURE.

The ceiling argument the ledger has rested on since s15/s16 is a chain:
  (a) the target holds &D_80106A73 in TWO hard registers;
  (b) hard registers are handed out per allocno, global.c:426 makes exactly one
      allocno per pseudo, find_reg gives one hard register per allocno for the
      whole live range, and GCC 2.7.2 has no live-range splitting;
  (c) therefore two hard registers require two pseudos;
  (d) "one C object is one DECL_RTL is one pseudo", so two pseudos require two
      C pointer objects -- the construct the Judge banned.

Step (d) had only ever been verified for SCALAR pointer locals. It is not a
theorem: a C object of AGGREGATE type is not one DECL_RTL pseudo at all. A
frame-resident aggregate is a MEM, and every read of a member loads into a
FRESH pseudo -- which is precisely the "two pseudos from one declared C object"
that the whole argument says is impossible. Eighteen sessions in, no aggregate
address holder had ever been measured, so the load-bearing step of the ceiling
argument had a live hole.

Measured this session (tmp/grind/func_80034F88/s18/{probe.py,probe2.py,
variants/,results.json,asm_*.txt}); all four bodies are the banked floor-10
chassis with the pointer local replaced by an aggregate:

  variant        shape                                     score  insns  lbu/sb/lui
  a0_base.c      control: scalar `u8 *q` (candidate.c)        10     49   175/164/456
  g1_arr1.c      `u8 *qa[1];` one slot, all four accesses     10     49   175/164/456
  g2_struct1.c   `struct { u8 *b; } s;` one member            10     49   175/164/456
  g3_arr2.c      `u8 *qa[2];` mask+blk1 on [0], blk2/4 on [1] 35     58   175/164/456
  g4_struct2.c   `struct { u8 *a, *b; } s;` same split        35     58   175/164/456

The kill is stronger than the scores: g1 and g2 are byte-for-byte the SAME
instruction stream as the scalar control (`diff asm_a0_base.txt asm_g1_arr1.txt`
and the same against asm_g2_struct1.txt are both empty). A single-slot,
address-never-taken aggregate is scalarised into exactly ONE pseudo by GCC
2.7.2, so it is not a route to a second address register -- it is the scalar
form spelled longer.

The two-slot aggregates DO create two address pseudos, and they pay for it in
the frame: the prologue grows from `addiu sp,sp,-24` to `addiu sp,sp,-32`, the
first address is SPILLED with `sw v1,16(sp)` and reloaded per use, and the body
lands at 58 instructions against the target's 49 (score 35). That is strictly
worse than the two-SCALAR-object forms the Judge banned (10 and 21), and
nowhere near 0. The aggregate family therefore neither rescues the
single-object floor nor hides an unexplored route to the target.

Step (d) now reads, in its repaired form: two address pseudos require either
two scalar C pointer objects (the banned construct, reaches 0) or one aggregate
C object with two slots (frame-resident, 58 insns, score 35, cannot reach 0).
The ceiling stands, and it now stands on a measurement instead of on an
untested premise.

--- 2. THE ORDERING PROOF: THE TARGET'S OWN INSTRUCTION ORDER SHOWS THE
--- ORIGINAL SOURCE HELD TWO SIMULTANEOUSLY-LIVE ADDRESS OBJECTS.

Read straight off asm/funcs/func_80034F88.s, three consecutive instructions:

    /* 80034FC8 */  lui    $a0, %hi(D_80106A73)     <- block 2's base materialised
    /* 80034FCC */  addiu  $a0, $a0, %lo(D_80106A73)
    /* 80034FD0 */  sb     $v0, 0x0($v1)            <- block 1's store, base $v1

The second base is materialised while the FIRST base is still the live base
register of a store that has not been issued yet. The two values overlap.

A single C pointer object cannot produce that order, and not for allocator
reasons -- for dataflow reasons that hold at every optimisation level. One C
pointer local is one DECL_RTL pseudo; re-assigning it emits `(set (reg q)
(symbol_ref))` on the SAME pseudo; and that set is a def of the very register
the pending `sb` reads as its base. No GCC pass will move a def of a pseudo
above a use of that pseudo, because doing so changes the program. So with one
object the store must precede the re-materialisation, always.

Our own build confirms the forced order empirically. s18/asm_a0_base.txt,
block 1:

    sb    v0,0(a0)          <- block 1's store FIRST
    lui   a0,0x0            <- only then the next base
    addiu a0,a0,0

i.e. exactly the target's two instructions, in the only order a single object
permits. Every single-object form measured across eighteen sessions has this
order; no source-level dial (declaration order, live-range shape, statement
order, staging, mask spelling, temporary width, register pins, 6,450+ permuter
iterations) has ever flipped it, and by the argument above none can.

This upgrades the ledger's central claim in kind, not merely in strength. Until
now the statement was "OUR reproduction cannot get two base registers from one
C object". The statement now available is "the ORIGINAL SOURCE necessarily
contained at least two distinct address objects for D_80106A73", because the
shipped instruction order is unreachable from any one-object C program compiled
by any compiler that preserves dataflow.

That is the most decision-relevant fact this function has produced, and it
belongs at the top of the escalation packet, because it changes the question
the owner is being asked. The Judge's ban was reasoned as "four identical
`u8 *q = &D_80106A73;` declarations treated as a register-allocation lever
rather than as ordinary program logic". The ordering proof says a multi-object
source is not a lever invented to steer the allocator -- it is a property of the
code that was actually compiled in 1998. Whether that makes a two- or
three-object reconstruction admissible is still NOT a session's call (checklist
T5 forbids self-approving a respelling of a banned construct, and the standing
2026-07-27 ruling wants a cited SOTN-master precedent for a coercion/spelling
family). But the escalation entry should ask the sharpened question -- "is
reconstructing a provably-multi-object original a coercion at all?" -- rather
than the old one, "may we add handles to steer the allocator?".

--- 3. THE MERGED POSITION, FOR THE NEXT LADDER PASS.

Stated once so no future session re-derives it:

  * Floor 10 at 49/49 insns, census lbu 175 / sb 164 / lui 456 against the
    target's 176 / 164 / 456. The entire residual is block 1: the base/byte
    register pair is swapped against the target, and block 1's post-store
    reload is folded by cse (our maspsx nop where the target has the second
    `lbu`).
  * The reload is priced at exactly +1 (s17's e1: buy the reload with a direct
    symbol read, pay one extra `lui $at`; 11 at 50 insns with the target's
    EXACT access census). The trade is exactly balanced -- getting the reload
    without the lui needs a second address pseudo.
  * A second address pseudo is reachable only by a second scalar C pointer
    object (banned; reaches 0) or a two-slot aggregate (new this session;
    frame-resident, 58 insns, 35). There is no third route: anonymous symbol
    references never occupy a register (s16), reload rematerialisation is
    short-circuited by reload.c:4128-4137 (s16), no non-constant expression in
    the function yields the address (s17), the declaration model is
    codegen-neutral (s17), and the semantic simplification is four instructions
    short of the target's shape (s17).
  * The target's instruction ORDER proves the original held two objects (this
    session), so the gap between the admissible floor and the target is a
    SOURCE-MODEL gap, not a search gap.

The correct next step is the escalation-modality pass the frontier already
names, carrying the ordering proof. The pure-C ladder should not be re-run.

- [s18] s18 measured the aggregate address-holder family for the first time in eighteen sessions: a0_base (scalar control) 10 / 49 insns / lbu 175 sb 164 lui 456; g1_arr1 (`u8 *qa[1];`) 10 / 49 / same census; g2_struct1 (`struct { u8 *b; } s;`) 10 / 49 / same census; g3_arr2 (`u8 *qa[2];`) 35 / 58; g4_struct2 (two-member struct) 35 / 58. Results at tmp/grind/func_80034F88/s18/results.json.

- [s18] The single-slot aggregate forms are not merely equal-scoring — their disassembly is byte-for-byte identical to the scalar candidate's (empty diffs of asm_g1_arr1.txt and asm_g2_struct1.txt against asm_a0_base.txt). GCC 2.7.2 scalarises a single-slot, address-never-taken aggregate into exactly one pseudo, so aggregate spelling is inert on this function.

- [s18] The two-slot aggregate forms do produce two address pseudos, but the aggregate goes to the frame: prologue `addiu sp,sp,-24` becomes `addiu sp,sp,-32`, the first address is stored with `sw v1,16(sp)` and reloaded per use, and the body costs 9 extra instructions (58 vs the target's 49). Two address pseudos are therefore obtainable from ONE C object, but only at a price that puts the form further from the target than any two-scalar-object form.

- [s18] The ceiling proof's step (d) is repaired and now measured rather than assumed: two address pseudos require either two scalar C pointer objects (the banned construct, reaches 0) or one two-slot aggregate (58 insns, score 35, cannot reach 0). No third route survives — anonymous symbol references never occupy a register (s16), reload rematerialisation is short-circuited by reload.c:4128-4137 (s16), no non-constant expression in the function yields the flag address (s17), and the 4-byte declaration model is codegen-neutral (s17).

- [s18] ORDERING PROOF (new, and the most decision-relevant fact this function has produced): the target emits `lui $a0,%hi(D_80106A73)` at 80034FC8 and `addiu $a0,$a0,%lo` at 80034FCC BEFORE `sb $v0,0x0($v1)` at 80034FD0 — the second base is materialised while the first is still the live base of a pending store. With one C pointer object those are the same pseudo, so the second materialisation is a def of the register the store reads, and no dataflow-preserving compiler may emit it first. The original source therefore necessarily contained at least two distinct address objects for D_80106A73.

- [s18] The forced opposite order is visible in our own floor-10 build (s18/asm_a0_base.txt): `sb v0,0(a0)` then `lui a0` / `addiu a0`. Eighteen sessions of source-level dials have never flipped it and, by the dataflow argument, none can.

- [s18] Consequence for the escalation packet: the Judge's ban was reasoned as 'four identical `u8 *q = &D_80106A73;` declarations treated as a register-allocation lever rather than as ordinary program logic'. The ordering proof shows a multi-object source is not an invented lever but a property of the code that was actually compiled in 1998, so the gate-2 question the owner should be asked is 'is reconstructing a provably-multi-object original a coercion at all?' rather than 'may we add handles to steer the allocator?'. A session may NOT self-answer that (checklist T5; the standing 2026-07-27 ruling's citation requirement), and no multi-object body was installed or submitted this session.

- [s18] src/code6cac_b.c was left exactly as committed (the probe harness splices and restores; `git status` shows src clean). The session's only tracked changes are the two ledger files, two new rejected/ banks, and the candidate.c header.

- [s18] Housekeeping note for future sessions: the body committed in src/code6cac_b.c is NOT candidate.c — it is an older form carrying `asm volatile("" ::: "memory")` scheduling barriers, which the honest sandbox strips, and which coincidentally also scores 10 (with a different instruction stream: one base in $a2 plus `move` copies). Any 'base' measurement taken without splicing candidate.c is measuring that body, not the banked candidate.

==== s19 (structural) ====

HEADLINE. Floor unchanged at 10. Eighteen further structurally distinct forms
measured on the score-10 single-pointer-object chassis (candidate.c), chosen to
sweep the structural dimensions that had never been varied ON THIS CHASSIS: the
type of the OTHER live pointer, the spelling and staging of the 0xF8 mask store,
condition hoisting/re-association, scope flattening, declaration order inside
block 1, and three allocno-ordering perturbations. TEN of the eighteen land on
exactly 10 / 49 insns / lbu 175 / sb 164 / lui 456 — bit-for-bit the same score
and census as the base — and the other eight are strictly worse. The structural
axis is now dead by saturation as well as by the s15-s18 mechanism proof: the
residual is invariant under every source-level dimension that does not add a
second address object.

HARNESS / ARTIFACTS (all under tmp/grind/func_80034F88/s19/)
  gen.py / gen2.py           variant generators (waves 1 and 2)
  variants/ variants2/ variants3/   the 18 bodies, one dimension changed each
  probe.py / probe2.py / probe3.py  splice -> `sandbox --disable all` -> objdump
                                    census (lbu / sb / lui) -> restore src
  results.json / results2.json / results3.json   the raw measurements

THE MEASUREMENTS (honest `sandbox func_80034F88 --disable all`; target 49 insns,
census lbu 176 / sb 164 / lui 456)

  score 10, 49 insns, lbu 175 / sb 164 / lui 456  — INDISTINGUISHABLE FROM BASE
    a0_base                candidate.c re-measured (sanity anchor)
    a_p_as_u8ptr           `p` declared `u8 *`, flag conditions read as
                           `*(s32 *)(p + 0x20) & K`, loop as `p + i + 0x17`
    d_flat_scope_reuse     v/c hoisted to function scope, reused by all 3 blocks
    i_block1_read_first    block 1 reads the flag BEFORE the condition
    n1_mask_via_s32_temp   `m = *q; *q = m & 0xF8;`
    n2_mask_longhand       `*q = *q & 0xF8;`
    n3_mask_via_u8_temp    `u8 m = *q; *q = m & 0xF8;`
    n5_q_declared_first    `u8 *q;` declared before `s32 *p;`
    n7_mask_complement     `*q &= ~7;`
    o3_block1_decl_order   block 1 declares `c` before `v`
    o4_block1_flat         block 1 loses its inner scope (locals at fn top)

  worse
    b_block1_shift_cond    11 / 49  block 1's test re-associated as `p[8] << 31`
                                    (sll+bgez where target has andi+beq)
    m_block1_u8_cond       11 / 49  block 1's cond/result local narrowed to u8
    o2_loop_ne_cond        13 / 49  copy loop `i != 3` instead of `i < 3`
    n6_cond_above_mask     13 / 45  block 1's condition read hoisted above the
                                    0xF8 store: cse then forwards that store into
                                    block 1's read, lbu 175 -> 174, -4 insns
    o1_i_live_early        24 / 53  `i = 0;` before the call (loop index live
                                    across the whole flag section)
    c_conditions_hoisted   29 / 39  all three `p[8] & K` conditions computed
                                    before the mask store: with no memory read
                                    between the flag stores cse forwards TWO of
                                    them, lbu 175 -> 173, and the function
                                    collapses to 39 insns (10 under target)
    n4_q_before_call       29 / 51  `q = &D_80106A73;` before `func_80077D00()`:
                                    the pointer is live across the jal, so it
                                    takes a callee-saved register and the
                                    prologue grows a save/restore pair

WHAT THE TEN-WAY TIE MEANS (the load-bearing part)

The ten tied forms are not near-duplicates. They differ in the type of the
function's other live pointer, in whether the flag temporaries live in three
inner scopes / one inner scope / function scope, in declaration order both
between the two pointer objects and between block 1's own two locals, in the
statement order of block 1's two reads, and in four different spellings of the
0xF8 read-modify-write (compound, long-hand, s32-staged, u8-staged, complement
constant). Every one of those is a dimension the codegen-technique-index lists
as a structural lever, and every one is EXACTLY codegen-neutral here: same
instruction count, same lbu/sb/lui census, same score.

That is the empirical face of the s15-s18 mechanism result. Block 1's defect is
(a) the base register being $a0 where target uses $v1 and (b) one missing
post-store reload, and both are decided by there being ONE address pseudo. All
of the above vary the SHAPE of the code around that pseudo without changing how
many address objects exist, so none of them can move either half. The three
forms that DO move the numbers move them the wrong way and for a legible reason:
hoisting a condition (n6) or all three conditions (c) removes the memory
reference that was separating a flag store from the next flag read, so cse
forwards the store and DELETES a reload the target has (census lbu 175 -> 174 ->
173, i.e. moving further from the target's 176, not closer); and making a value
live across the call (n4, o1) buys callee-saved registers and prologue growth.

Two smaller facts worth banking on their own:
  * The type of `p` is codegen-neutral on this function. `s32 *p` with `p[8]`
    and `(u8 *)p + i + 0x17`, and `u8 *p` with `*(s32 *)(p + 0x20)` and
    `p + i + 0x17`, produce the same count and census at the same score. Future
    sessions do not need to re-sweep the other pointer's type.
  * The 0xF8 mask store has no spelling degrees of freedom at all — five
    spellings, one stream. It is not a lever, it is a fixed point.

- [s19] Floor unchanged at 10 (49 build insns vs the target's 49; census lbu 175 / sb 164 / lui 456 against the target's lbu 176 / sb 164 / lui 456). candidate.c re-measured this session as variant a0_base and is unchanged as the best admissible form; src/code6cac_b.c is byte-clean at session end (the probe harness restores it).

- [s19] TEN structurally distinct forms tie the base at 10 on every counter simultaneously (score, insn count, lbu, sb, lui). They differ in the type of the function's other live pointer, in whether the flag temporaries live in three inner scopes / one inner scope / function scope, in declaration order both between the two pointer objects and between block 1's own two locals, in the statement order of block 1's two reads, and in five spellings of the 0xF8 read-modify-write. Every one of those is a codegen-technique-index structural lever and every one is exactly codegen-neutral here.

- [s19] The type of `p` is codegen-neutral on this function: `s32 *p` with p[8] and (u8 *)p + i + 0x17, and `u8 *p` with *(s32 *)(p + 0x20) and p + i + 0x17, give the same instruction count and the same census at the same score. Future sessions do not need to re-sweep the other pointer's type.

- [s19] The 0xF8 mask store has no spelling degrees of freedom at all: compound assignment, long-hand `*q = *q & 0xF8;`, s32-staged temp, u8-staged temp and `&= ~7` all produce one stream. It is a fixed point, not a lever.

- [s19] Condition hoisting is measurably counter-productive and explains why: with the separating p[8] load removed, cse forwards the flag store into the following read and deletes a reload the target has, driving the lbu census 175 -> 174 (one condition hoisted, score 13 at 45 insns) -> 173 (all three hoisted, score 29 at 39 insns) against the target's 176.

- [s19] Any value made live across the func_80077D00() call takes a callee-saved register and grows the prologue: q materialised before the call scores 29 at 51 insns, the loop index defined at the top scores 24 at 53 insns.

- [s19] Taken with s15-s18, the residual is now shown invariant under every source-level structural dimension that does not change how many address objects exist -- the empirical face of the one-allocno-per-pseudo ceiling proof, reached independently of it.

==== s20 (structural) ====

HEADLINE. Floor unchanged at 10, but the residual is RE-DECOMPOSED and one half of
it is now closed for good. 35 further forms measured. The session's central
result: the block-1 reload that the target has and we lack (our lbu census 175
against the target's 176) is recoverable at EXACTLY ZERO instruction cost from a
SINGLE C pointer object -- eight different forms reach 49 instructions with the
target's exact lbu 176 / sb 164 / lui 456 census -- and the score does not move.
That kills s17's central pricing claim ("the reload costs one extra lui, so 10 is
an exactly balanced ceiling") and it narrows the whole 10-point residual to ONE
thing: block 1's register naming plus the placement of the second base
materialisation, both of which need two SIMULTANEOUSLY LIVE address values.

HARNESS / ARTIFACTS (all under tmp/grind/func_80034F88/s20/)
  gen.py gen2.py gen3.py gen4.py               variant generators (waves 1-4)
  variants/ variants2/ variants3/ variants4/   the 35 bodies, one dimension each
  probe.py probe2.py probe3.py probe4.py       splice -> `sandbox --disable all`
                                    -> objdump census (lbu/sb/lui) -> restore src
  results.json results2.json results3.json results4.json   raw measurements

THE INSTRUCTION-LEVEL PICTURE (dumped fresh this session, not inherited)

Our score-10 base, block 1 (objdump of tmp/sandbox/.../code6cac_b.o):
    lui a0 ; addiu a0 ; lbu v1,0(a0) ; move a1,v0 ; andi v1,0xf8 ; sb v1,0(a0)
    lw v0,32(a1) ; NOP ; andi v0,v0,1 ; bnez ; ori v0,v1,1 ; move v0,v1
    sb v0,0(a0) ; lui a0 ; addiu a0
Target (asm/funcs/func_80034F88.s):
    lui v1 ; addiu v1 ; lbu a0,0(v1) ; move a1,v0 ; andi a0,0xf8 ; sb a0,0(v1)
    lw v0,0x20(a1) ; LBU a0,0(v1) ; andi v0,v0,1 ; bnez ; ori v0,a0,1 ; addu v0,a0,0
    lui a0 ; addiu a0 ; sb v0,0(v1)

Two facts fall straight out of that alignment and neither was in the ledger:
  (1) The "missing reload" and the "maspsx nop" are the SAME position: the
      target's second lbu sits in the load-delay slot of `lw p[8]`. We emit a nop
      there because cse forwarded the mask store into block 1's read and deleted
      the load. The instruction COUNT is 49 either way.
  (2) Our build already re-materialises the base THREE times from ONE C object
      (lui/addiu at the top of blocks 2 and 4), exactly as the target does. The
      single-object chassis is not short of address materialisations; it is short
      of two materialisations that are LIVE AT THE SAME TIME.

RESULT 1 -- THE RELOAD IS FREE (the s17 pricing claim is dead)

`q = q + 3; q = q - 3;` inserted between the mask store and block 1's read
(x1_roundtrip_true_symbol) scores 10 at 49 insns with lbu 176 / sb 164 / lui 456 --
the target's exact access census at the target's exact instruction count, with the
true `&D_80106A73` symbol spelling and therefore no LO16-addend artefact. Seven
sibling forms do the same: delta 3 in either order (y1), delta 1 (y2), the pair
inside block 1's scope (y3), the pair after the condition load (y4), two round
trips (y5), the pair replicated in every block (y8, 45 insns / score 13 -- worse,
because it also frees blocks 2 and 4), and the `&D_80106A70 + 3` spelling (w06).

MECHANISM (why it is free, and why the cheaper spellings are not equivalent).
GCC 2.7.2's cse.c keys its memory table on the ADDRESS EXPRESSION's equivalence
class. It cannot re-associate `(q + 3) - 3` back to `q`, so the pseudo that
reaches block 1's read carries no known equivalence to the pseudo the mask store
wrote through; the recorded MEM value is not matched and the load survives.
local-alloc then coalesces the whole copy chain onto one hard register, so the
arithmetic itself emits nothing. Contrast the two nearby spellings, both measured:
  * a redundant CONSTANT re-assignment in the same place (`q = &D_80106A73;`
    again -- v01_reassign_b1) is score-, count- AND census-neutral (10 / 49 /
    lbu 175): cse copy-propagates the second set onto the first pseudo, the
    equivalence survives, and the store is still forwarded.
  * the same round trip placed BEFORE the mask store (y6) leaves the census at
    lbu 175: the fresh pseudo has to be created BETWEEN the store and the read.

ADMISSIBILITY. The round trip is dead pointer arithmetic with no semantic purpose;
it fails checklist T1/T2/T3 and is NOT proposed and NOT installed. It is banked at
rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c as a DIAGNOSTIC
chassis only. (Noted for a future ruling, not acted on here: `x + K - K` opaque
arithmetic is the shape SOTN's wiki endorses for defeating bit-test transforms.
The question is moot at present because the construct does not move the score --
it buys census fidelity and nothing else.)

RESULT 2 -- THE RESIDUAL IS NOW EXACTLY ONE THING

With the reload recovered the two streams agree instruction-for-instruction in
block 1 except: (a) the base register is $a0 where the target uses $v1 and the
loaded value is $v1 where the target uses $a0, and (b) the second base's lui/addiu
pair sits AFTER block 1's store where the target has it BEFORE. Every one of the
eight reload-recovering forms scores exactly 10, so the $a0/$v1 dial does not move
on this chassis either -- a fourth independent kill after s16's three.

(b) is not reachable by moving the existing re-materialisation statement earlier,
which is the obvious source-side attack and is now measured dead:
`q = &D_80106A73;` hoisted to just before block 1's store (x4) scores 21 at 51
insns; with block 2 keeping its own re-materialisation as well (x5) also 21 / 51;
done in every block (x6) 25 at 48 insns with lbu 173. The reason is exactly the
s18 ordering proof seen from the source side: if the store goes through the NEW
pseudo then the old pseudo dies early, so the two share a register and nothing is
gained, and cse then forwards that store into the next block's read and destroys
two further reloads. The target's order requires the store to use the OLD value
while the NEW base is already materialised -- two live values, which one C object
cannot provide.

THE OTHER 25 FORMS (all worse or neutral; one dimension changed each)

  10 / 49  v01 redundant constant re-assignment at the top of block 1 (above)
  10 / 49  w07 block 1 addressing spelled `q[0]` instead of `*q` (same rtx)
  10 / 49  x2  first materialisation spelled `&D_80106A70 + 3`, nothing else --
               so a SINGLE addend-spelled materialisation carries NO phantom
               distance; s17's +2 R_MIPS_LO16 artefact needs the whole body
               spelled that way
  10 / 49  x3  walk to the flag byte before the mask store (pseudo folded away)
  12 / 46  v07 block 1's store duplicated into both arms
  13 / 47  v08 block 1 as one local, compound `|=`, no else arm
  13 / 47  v09 block 1 tests p[8] inline (no staged condition local)
  13 / 48  v06 the 0xF8 mask folded into block 1's arms (one store fewer)
  13 / 52  w04 single object re-assigned to the neighbouring symbol for block 1
  13 / 45  y8  round trip replicated in every block
  14 / 52  v02 block 1 reads the flag byte SIGNED (`*(s8 *)q`)
  14 / 52  w02 object walked to `&D_80106A70` after the mask, block 1 uses q[3]
  16 / 52  v03 all three blocks read signed
  16 / 51  w05 mask applied to the symbol, pointer materialised for block 1
  17 / 52  v05 object based at D_80106A70 with displacement 3, symbol copy loop
  19 / 47  v11 the flag word p[8] read ONCE into a local for all three blocks
  20 / 51  w01 object based at D_80106A70, mask via q[3], then walked +3
  20 / 51  w08 mask via q[3] off the 0x70 base, blocks re-materialise the flag ptr
  20 / 54  w03 every access displaced +3 off a D_80106A70 base
  21 / 51  x4  re-materialisation hoisted before block 1's store
  21 / 51  x5  same, blocks 2 and 4 keeping their own as well
  22 / 50  v04 as v05 but the copy loop shares the object (loses the loop's lui)
  25 / 48  x6  re-materialisation before every store
  27 / 49  v10 block 1 stages the whole flag word and masks at the test
  32 / 50  y7  round trip but blocks 2 and 4 lose their re-materialisations

- [s20] Floor unchanged at 10 (49 build insns vs the target's 49). src/code6cac_b.c is byte-clean at session end (the probe harness restores it); candidate.c keeps the same body and gains an s20 header note.

- [s20] The block-1 defect is TWO things at ONE position, not two separate costs: the target's second `lbu a0,0(v1)` sits in the load-delay slot of `lw p[8]`, and our build emits a maspsx nop there because cse forwarded the mask store into block 1's read and deleted the load. Both streams are 49 instructions.

- [s20] The missing reload is recoverable at ZERO instruction cost from ONE C pointer object: eight forms that create a fresh address pseudo between the mask store and block 1's read (`q = q + 3; q = q - 3;` and siblings) reach 49 insns with the target's exact lbu 176 / sb 164 / lui 456 census. This KILLS s17's claim that the reload costs one extra lui and that 10 is therefore an exactly balanced ceiling -- the reload half is free, and the ceiling is entirely the register half.

- [s20] Mechanism: cse.c cannot re-associate `(q + 3) - 3` back to `q`, so the pseudo reaching the read has no equivalence to the store's address pseudo and the recorded MEM value is not matched; local-alloc coalesces the copies afterwards, so the arithmetic emits nothing. A redundant CONSTANT re-assignment in the same place is copy-propagated instead and is exactly neutral (10 / 49 / lbu 175), and the same round trip placed BEFORE the mask store does not recover the reload -- the fresh pseudo must be created between the store and the read.

- [s20] Corollary that corrects an inherited assumption: the per-block `q = &D_80106A73;` re-assignments emit lui/addiu because they follow the if/else JOIN LABEL (cse table flush, the s14 finding), not because a re-assignment is inherently a fresh materialisation. Our single-object build already makes three address materialisations, exactly as the target does; what it cannot make is two that are live at the same time.

- [s20] The dead-arithmetic chassis is a DIAGNOSTIC ONLY and was never installed or proposed: dead pointer arithmetic has no semantic purpose (T1/T2) and its mechanism is a named GCC pass (T3). Banked at rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c. It is also moot for disposition -- it does not move the score, only the census.

- [s20] With the reload recovered, the entire 10-point residual is (a) block 1's $a0/$v1 register naming and (b) the second base materialisation sitting after block 1's store instead of before it. All eight reload-recovering forms score exactly 10, so the $a0/$v1 dial is dead a fourth independent way.

- [s20] Hoisting the existing re-materialisation statement to just before block 1's store -- the obvious source-side attack on the ordering half -- is measured dead: 21 at 51 insns (x4, x5) and 25 at 48 insns with lbu 173 (x6). If the store goes through the new pseudo the old pseudo dies early, so nothing is gained, and cse then forwards that store into the next block's read and deletes two further reloads. This is the s18 ordering proof reproduced from the source side.

- [s20] A single addend-spelled materialisation (`&D_80106A70 + 3` for the first one only) carries NO phantom distance (10 / 49, x2). s17's +2 R_MIPS_LO16 artefact requires the whole body spelled that way; the caveat should be applied per-materialisation, not per-form.

- [s20] Floor unchanged at 10 (49 build insns vs the target's 49; census lbu 175 / sb 164 / lui 456 against the target's lbu 176 / sb 164 / lui 456). src/code6cac_b.c is byte-clean at session end -- the probe harness splices and restores.

- [s20] Fresh instruction-level alignment (dumped this session, not inherited): our block 1 is `lui a0; addiu a0; lbu v1,0(a0); move a1,v0; andi v1,0xf8; sb v1,0(a0); lw v0,32(a1); NOP; andi v0,v0,1; bnez; ori v0,v1,1; move v0,v1; sb v0,0(a0); lui a0; addiu a0` against target's `lui v1; addiu v1; lbu a0,0(v1); move a1,v0; andi a0,0xf8; sb a0,0(v1); lw v0,0x20(a1); LBU a0,0(v1); andi v0,v0,1; bnez; ori v0,a0,1; addu v0,a0,0; lui a0; addiu a0; sb v0,0(v1)`.

- [s20] The 'missing reload' and the 'maspsx nop' are the SAME position: the target's second lbu sits in the load-delay slot of `lw p[8]`. Both streams are 49 instructions.

- [s20] Our single-object build ALREADY makes three address materialisations (lui/addiu at the tops of blocks 2 and 4), exactly as the target does. The chassis is not short of materialisations; it is short of two that are live at the same time.

- [s20] Corollary that corrects an inherited assumption: those per-block re-assignments emit lui/addiu because they follow the if/else JOIN LABEL (cse table flush, the s14 finding), not because a re-assignment is inherently a fresh materialisation -- v01 proves it, since the same re-assignment placed inside the mask's basic block is copy-propagated away and is exactly neutral.

- [s20] The reload half of the residual is FREE from one C object: eight fresh-address-pseudo forms reach 49 insns with the target's exact lbu 176 / sb 164 / lui 456 census. s17's 'the reload costs one extra lui, so 10 is an exactly balanced ceiling' is therefore dead as stated -- the balance was an artefact of the particular (direct-symbol) spelling s17 measured.

- [s20] Mechanism for that: cse.c cannot re-associate (q + 3) - 3 back to q, so the pseudo reaching the read has no equivalence to the store's address pseudo and the recorded MEM value is not matched; local-alloc coalesces the copies afterwards so the arithmetic emits nothing. Position matters -- the same round trip placed before the mask store (y6) does not recover the load.

- [s20] The construct that achieves it is dead pointer arithmetic with no semantic purpose (fails checklist T1/T2, and its only explanation is a named GCC pass -- T3). It was NEVER installed as a candidate and is NOT proposed; it is banked as a diagnostic chassis at memory/grind/func_80034F88/rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c. It is moot for disposition in any case: it does not move the score, only the census.

- [s20] After that recovery the entire 10-point residual is exactly two things: (a) block 1's $a0/$v1 register naming and (b) the second base's lui/addiu sitting after block 1's store instead of before it -- both requiring two SIMULTANEOUSLY LIVE address values.

- [s20] The obvious source-side attack on (b) -- hoisting the chassis's existing re-materialisation statement to just before block 1's store -- is measured dead at 21 / 51 insns (x4, x5) and 25 / 48 with lbu 173 (x6). This reproduces s18's instruction-ordering proof from the source side rather than from the target's bytes.

- [s20] The $a0/$v1 dial is now dead a fourth independent way: all eight forms that change block 1's pseudo structure keep the identical register assignment at score 10.

- [s20] A SINGLE addend-spelled materialisation carries no phantom distance (x2 = 10 / 49). s17's +2 R_MIPS_LO16 artefact applies per-materialisation, not per-form -- future sessions should subtract it proportionally, not wholesale.

- [s20] Seventeen further block-1 / addressing spellings measured at 12-32 (signed read 14/16, store into arms 12, single local no else 13, inline condition 13, mask folded into arms 13, flag-word staged 27, flag word read once 19, D_80106A70-based displaced addressing 17/20/20/22/54-insn variants, neighbour-symbol re-assignment 13, mask on the symbol 16).


==== s21 (permuter) ====

MODALITY: permuter (5th permuter session on this function; s4 random, s5
directed, s12b, s13, now s21). Two campaigns, both launched, waited for
IN-TURN, harvested and --stopped before the session ended. 81,779 iterations
total. Artifacts under tmp/grind/func_80034F88/s21/.

WHAT WAS NEW ABOUT THIS SESSION'S CAMPAIGNS. Every prior campaign was seeded
ABOVE the honest floor: s4's two seeds were floor-18 chassis, s5 was a directed
run on 18, s13 seeded the inline-helper chassis at 13. Seed 1 here is the
score-10 single-object body that has been candidate.c since s16 — the honest
floor itself, never permuted. Seed 2 is e1
(rejected/b1-read-direct-symbol-reload-priced-at-1-score11.c, score 11 at 50
insns), the only other chassis carrying the target's exact lbu 176 access
census, also never permuted.

Workspaces are built by tmp/grind/func_80034F88/s21/mkws.sh and mkws2.sh, both
derived from s13's validated minimal-context builder (the base reproduces
full-TU codegen instruction-for-instruction; base_insns.txt / base2_insns.txt
vs target_insns.txt confirm it again this session). Finds are re-scored with
the HONEST sandbox by tmp/grind/func_80034F88/s21/eval.py, which also censuses
each find for ADMISSIBILITY: how many distinct C objects hold an address of
D_80106A73, counting pointer COPIES (`q2 = q;`) as second objects, plus
new_var constant holders / volatile / __asm__. Ranking is on the sandbox score,
never the permuter score (s4-H1).

SEED 1 — the floor chassis. 44,626 iterations, permuter metric 455 -> 340, six
finds. Honest sandbox re-scores (eval_seed1.json), best first:

    find             perm   sandbox  insns  addr-objects
    output-455-1     455    10       49     1     (a re-spelling of the seed)
    output-380-1     380    12       49     1
    output-440-1     440    13       49     1
    output-450-2     450    13       49     1
    output-340-1     340    14       49     2
    output-450-1     450    21       50     2

Nothing beats the seed. The floor chassis is a local minimum of the RANDOM
STRUCTURAL neighbourhood as well as of nineteen sessions of hand enumeration —
this is the first time that has been tested, because no campaign had ever been
seeded there. It also reconfirms s4-H1 in the sharpest possible form: the
permuter's BEST metric find (340) is the sandbox's SECOND-WORST (14), and the
permuter's WORST metric find (455) is the sandbox's best (10). The two metrics
are anti-correlated on this function, so --stop-on-zero can never fire here and
campaign progress cannot be read off the permuter's numbers.

SEED 2 — the e1 chassis. 37,153 iterations, permuter metric 365 -> 255, six
finds. Honest sandbox re-scores (eval_seed2.json):

    find             perm   sandbox  insns  lbu  addr-objects
    output-255-1     255    8        51     176  2   (BANNED — pointer copy)
    output-355-1     355    8        50     176  2   (MISCOMPILE + copy)
    output-350-2     350    13       51     176  1
    output-350-1     350    14       50     176  1
    output-360-1     360    14       50     176  1
    output-275-1     275    22       52     176  2

This is the first time any search or any hand form on this function has
produced a semantically correct score below 10. It does not move the floor,
because it is the BANNED construct:

  * output-255-1 keeps the program's meaning (`c = p[8] & 1;` intact) and
    scores 8. Its delta from its own seed is exactly one line: `new_var = q;`
    in block 1 with `*new_var = c;` for the store. That is a pointer COPY, a
    SECOND C pointer object aliasing D_80106A73, which the Judge's binding
    constraint names by that exact spelling ("`q1 = qm;` copies ... are all
    out"). It is also the lever s11 found by hand and recorded as the
    zero-cost reload handle. Banked at
    rejected/permuter-s21-pointer-copy-second-object-score8-BANNED.c.
  * output-355-1 also scores 8 but DELETED block 1's `& 1`, so the bit-0 flag
    is set whenever ANY bit of p[8] is set. Banked at
    rejected/permuter-s21-block1-mask-dropped-MISCOMPILES-score8.c. Two of the
    three sub-floor finds in this function's entire history are semantic
    breaks (the other is s4's 17), which is the standing reason every
    sub-floor find here is read for meaning before its score is believed.
  * every SINGLE-OBJECT find on seed 2 scored 13 or 14 — worse than the floor.

PRICING THE BANNED OBJECT, AND THE FACT THAT MATTERS. e1 is output-255-1 minus
the copy and scores 11 at 50 insns. So a second address object is worth
exactly 3 points and costs 1 instruction: 11/50 -> 8/51.

And it still does not reach 0. The instruction-level side-by-side of
output-255-1 against asm/funcs/func_80034F88.s
(tmp/grind/func_80034F88/s21/sbs.sh; output-255-1_insns.txt vs
target2_insns.txt) is 51 insns against 49, and the whole difference is block 1:

    ours                              target
    lui a0 / addiu a0,a0,0            lui v1 / addiu v1,v1,0
    lbu v1,0(a0)                      lbu a0,0(v1)
    andi v1,v1,0xf8                   andi a0,a0,0xf8
    sb v1,0(a0)                       sb a0,0(v1)
    move v1,a0                        (absent)
    lui a0 / lbu a0,0(a0)             lbu a0,0(v1)

The copy produces a second pseudo whose VALUE COMES FROM THE FIRST — hence the
`move`, and hence a fresh `lui` for the direct-symbol read — where the target
materialises its second base from the symbol itself, before block 1's store
(s18's ordering proof). Reaching 0 previously required THREE objects
(rejected/three-pointer-objects-judge-FAIL-score0.c). So the banned two-object
family is not one construct away from the match either, and the ledger's "two
SIMULTANEOUSLY LIVE address values, materialised independently" statement is
now confirmed from a third independent direction: random search descending onto
the ceiling from ABOVE (seed 1 cannot get under 10 with one object) and from
BELOW (seed 2's two-object forms stop at 8, not 0).

TELEMETRY. Both campaigns are in metrics/events.jsonl with
permuter-launch/permuter-harvest events and stop reasons. Seed 1's fresh-seed
window closed cleanly: no novel find for 546s after 38,015 iterations. Seed 2
was still emitting novel finds when it was stopped, but every novel find after
the first two was the same copy family, and its single-object finds never got
below 13.

- [s21] Two permuter campaigns, 81,779 iterations total, both launched via tools/permuter_campaign.py with telemetry, waited for IN-TURN, and harvested with --stop before the session ended; both stop reasons are recorded in metrics/events.jsonl.

- [s21] Seed 1 (tmp/grind/func_80034F88/s21/ws, label s21-floor10-single-object) is the score-10 single-object candidate body -- the honest floor -- which no prior campaign had ever seeded (s4 seeded floor-18 chassis, s5 was directed at 18, s13 seeded the inline-helper 13). 44,626 iterations, permuter metric 455 -> 340, six finds, honest sandbox re-scores 10 / 12 / 13 / 13 / 14 / 21. Fresh-seed window closed: no novel find for 546s after 38,015 iterations.

- [s21] Seed 2 (ws2, label s21-e1-direct-symbol-b1read) is e1, the only other chassis with the target's exact lbu 176 census, also never permuted. 37,153 iterations, permuter metric 365 -> 255, six finds; single-object finds 13 / 14 / 14, two-object finds 8 / 8 / 22.

- [s21] output-255-1 is the first semantically correct sub-floor form in this function's history: sandbox 8 at 51 insns with lbu 176 / sb 164 (the target's exact access census). It is INADMISSIBLE -- `new_var = q;` is a pointer COPY, a second C pointer object aliasing D_80106A73, named verbatim by the Judge's binding constraint ('`q1 = qm;` copies ... are all out'). Banked at memory/grind/func_80034F88/rejected/permuter-s21-pointer-copy-second-object-score8-BANNED.c.

- [s21] output-355-1 also scores 8 but DELETED block 1's `& 1`, so the bit-0 flag would be set whenever any bit of p[8] is set. Two of the three sub-floor finds in this function's entire history are semantic breaks (the other is s4's 17), which is the standing reason every sub-floor find here is read for meaning before its score is believed. Banked at rejected/permuter-s21-block1-mask-dropped-MISCOMPILES-score8.c.

- [s21] Pricing: the second address object is worth exactly 3 points and costs 1 instruction on the e1 chassis (11 at 50 insns -> 8 at 51). It does NOT reach 0; the three-object body was needed for that.

- [s21] Instruction-level side-by-side of the score-8 two-object form against asm/funcs/func_80034F88.s (s21/sbs.sh): 51 insns vs 49, and the whole difference is block 1 -- ours `lui a0 / addiu a0 / lbu v1,0(a0) / andi v1 / sb v1,0(a0) / move v1,a0 / lui a0 / lbu a0,0(a0)` against target `lui v1 / addiu v1 / lbu a0,0(v1) / andi a0 / sb a0,0(v1) / lbu a0,0(v1)`. A copied pointer is a dependent value, not an independent materialisation, so it buys the reload at the price of a move plus a lui and leaves the register naming inverted.

- [s21] s4-H1 (permuter score vs honest sandbox distance are uncorrelated on this function) reconfirmed at maximum contrast on seed 1: best metric find 340 = sandbox 14; worst metric find 455 = sandbox 10.

- [s21] src/ was left untouched: eval.py splices each find in, sandboxes it, and restores the file in a finally block; `git status` shows only ledger files and metrics/events.jsonl modified. Bare src/code6cac_b.c (the cheat-carrying body still in the tree) sandboxes at 24 / 48 insns as it did at session start; the honest floor of 10 is the candidate.c body, re-measured this session as find output-455-1.

==== s22 (escalation) ====

DISPOSITION SESSION. No new pure-C form was proposed and none was needed: the
driver had declared exhaustion (floor FLAT AT 10 across s14..s21, five distinct
modalities), and s21 closed the last open search axis. This session's product is
the disposition itself, filed in docs/grind/decisions.md as the entry
"2026-08-13 — func_80034F88 — OWNER-ESCALATION — RESOLVED BY STANDING RULING
(2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE".

1. BASELINE RE-MEASURED (both bodies, this session).
   * The in-tree cheat-carrying body (src/code6cac_b.c:3899, three
     `asm volatile("" ::: "memory")` barriers + 31 regfix rules) sandboxes at
     24 with cheats stripped: 49 target insns vs 48 build insns.
   * candidate.c (single C pointer object) installed into src and sandboxed:
     10 at 49/49 insns. Floor confirmed unchanged for the ninth session.
   * src/ was then restored with `git checkout -- src/code6cac_b.c` so main
     keeps the cheat that holds the oracle, exactly as the standing ruling
     directs. `git status` at session end shows only ledger/doc files and
     metrics/events.jsonl.

2. GATE 1 (canonical-asm / hand-coded evidence) — FAILS, measured, not asserted.
   `python3 tools/scan_hand_coded.py --single func_80034F88` returns
   tier=LOW score=0/8 ("no strong hand-coded indicators"); all eight boxes are
   unchecked, including the whole STRONG tier (S1 multu pacing, S2 empty branch,
   S6 BIOS jumptable). Saved at tmp/grind/func_80034F88/s22/scan_hand_coded.txt.
   This is the FIRST time the gate has actually been run on this function — the
   2026-08-13 16:36 Judge ruling explicitly noted "Gate 1 is unevaluated — no
   scan_hand_coded run appears anywhere in this ledger". It is now evaluated and
   it fails, so canonical-asm authorization is refused by policy
   (.claude/rules/endgame-lock-disposition.md: a LOW score is dispositive).

3. GATE 2 (SOTN-master precedent for the closing construct) — FAILS, and this
   is a FIRST-HAND CENSUS, not an absence-of-memory argument. A local SOTN
   master checkout exists at C:\Users\Trenton\Desktop\sotn-decomp, commit
   db41b28eee52969244a52cc269c8163d1ed8826a (branch master, 2026-07-01). Two
   scripted passes over its 1,675 src/**/*.c files:

   * Permissive (s22/sotn_census.py -> sotn_census.txt): 65 functions assign
     `&SYM` of ONE symbol to two or more distinct local handles. Reading them,
     essentially all are distinct SUB-OBJECTS — `&g_CurrentEntity->ext.azaghal.base`
     vs `.pos` vs `.offset`; `&g_Entities[UNK_ENTITY_8]` vs `[UNK_ENTITY_5]`;
     `&g_CurrentBuffer->drawModes[..]` vs `->sprite[..]` — i.e. ordinary program
     logic, and not the construct at issue.
   * Strict (s22/sotn_census2.py -> sotn_census2.txt, keyed on the IDENTICAL
     address-expression text): only 21 functions, every one disqualified:
       - non-matching ports: src/dra_psp/, src/main_psp/, src/sel_psp/, src/pc/,
         src/saturn/ (these are not the PSX matching target);
       - global-to-global stores (`D_80138FB4 = &D_psp_09236838;` etc.), not two
         live local handles;
       - two genuinely different walkers that merely START at the same node —
         the strongest-looking hit, src/ric/pl_blueprints.c:2219 (and the
         identical shape in src/maria/pl_blueprints.c:1887), is
         `prim2 = prim1 = &g_PrimBuf[self->primIndex];` after which `prim1` is
         advanced 16 nodes in its own loop and the two pointers then write
         DIFFERENT vertices of DIFFERENT primitives each iteration. Real logic;
         not a redundant handle retained for codegen.
   * A targeted grep for fake/match/required-ANNOTATED address-taking lines in
     the matched PSX trees (dra, ric, st, main, weapon, servant, boss) returns
     only the FakePrim / VertexFake TYPE-PUNNING family — a cast of ONE handle to
     a different struct type — plus `_svm_tn[...field_7_fake_program...]`. No
     instance anywhere of a second, redundant, simultaneously-live C handle on
     one address kept because the match needs it.

   The sanctioned pointer-alias-fake-exception family's own SOTN evidence
   (`tilemap = &g_Tilemap; // n.b.! unused, required for PSP`) is a SINGLE and
   UNUSED alias. What this function needs is two (in the only form that reaches
   0, three) simultaneously live, independently materialised, load-bearing
   handles. Extending the family to that shape is precisely the "novel extension
   of an existing sanctioned family to a shape that family's evidence does not
   cover" the endgame-lock rule refuses, and under the 2026-07-27 ruling a
   census that comes back negative is a FAILED gate, not an open question.

4. ONE CANDIDATE UN-TRIED LEVER WAS CONSIDERED AND FOUND ALREADY DEAD. The tail
   copy loop needs a base at D_80106A70 anyway, so declaring `u8 *r = &D_80106A70;`
   for the loop and letting block 1 ride on it is the one shape that could add a
   second address pseudo out of ORDINARY logic rather than as a lever. It is
   already banked: rejected/base70-disp3-loop-shares-object-score22.c, score 22.
   No other un-tried lever was derivable from the ledger.

5. DISPOSITION FILED. Both AND-gates fail, so the owner's 2026-07-27 standing
   ruling is APPLIED, not requested: keep the cheat on main (oracle stays green),
   classify INCOMPLETE — OWNER-ACCEPTED (not COMPLETED-C, not
   COMPLETED-INLINE-ASM-CANONICAL), park out of active grind but eligible for
   re-attempt if a genuinely new pure-C lever or new tooling appears. Nothing is
   pending on the owner.

- [s22] Floor re-measured and unchanged at 10 (candidate.c body, 49/49 insns); in-tree cheat-carrying body measures 24 at 48 insns with cheats stripped; src/ restored to HEAD after measurement.
- [s22] What holds the byte-match: 31 regfix.txt rules for func_80034F88, 0 asmfix rules, plus three `asm volatile("" ::: "memory")` cheat-asm barriers in the in-tree body; the function is not in inline_asm_canonical.txt.
- [s22] GATE 1 EVALUATED FOR THE FIRST TIME: scan_hand_coded --single func_80034F88 = tier LOW, score 0/8, no STRONG (S1/S2/S6) signal. Canonical-asm refused by policy. Artifact: tmp/grind/func_80034F88/s22/scan_hand_coded.txt.
- [s22] GATE 2 EVALUATED BY FIRST-HAND CENSUS of the local SOTN master checkout (commit db41b28eee52969244a52cc269c8163d1ed8826a, 1,675 src .c files): 65 permissive hits are distinct sub-objects; only 21 strict hits (identical address expression) and every one is a non-matching port, a global store, or two genuinely different walkers (ric/maria pl_blueprints `prim2 = prim1 = &g_PrimBuf[...]`, where prim1 is then advanced 16 nodes). NO SOTN precedent exists for a redundant, simultaneously-live second C handle on one global. Artifacts: s22/sotn_census.py|.txt, s22/sotn_census2.py|.txt, s22/sotn_checkout.txt.
- [s22] The `fake`-annotated address-taking lines in SOTN's matched PSX trees are the FakePrim/VertexFake TYPE-PUNNING family (one handle cast to another struct type), not second handles — so they do not support the construct even by analogy.
- [s22] The last plausibly-untried lever (a `u8 *r = &D_80106A70;` object owned by the tail copy loop, shared with block 1) was already measured dead at 22: rejected/base70-disp3-loop-shares-object-score22.c.
- [s22] DISPOSITION FILED in docs/grind/decisions.md as OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE. Terminal; nothing pending on the owner.

- [s22] [s22] Floor re-measured and unchanged at 10: candidate.c's single-C-pointer-object body installed into src/code6cac_b.c sandboxes at 10 with 49 build insns vs 49 target insns. src/ was restored with 'git checkout -- src/code6cac_b.c' immediately afterwards, per the standing ruling that the cheat stays on main to hold the oracle.

- [s22] [s22] What holds the byte-match: 31 regfix.txt rules for func_80034F88, 0 asmfix rules, plus three asm volatile("" ::: "memory") cheat-asm scheduling barriers in the in-tree body; the function is not in inline_asm_canonical.txt. The bare in-tree body sandboxes at 24 (48 build insns) with cheats stripped.

- [s22] [s22] GATE 1 EVALUATED FOR THE FIRST TIME IN 22 SESSIONS: scan_hand_coded --single func_80034F88 = tier LOW, score 0/8, no STRONG (S1/S2/S6) signal. Canonical-asm authorization is refused by policy (a LOW score is dispositive per .claude/rules/endgame-lock-disposition.md).

- [s22] [s22] GATE 2 EVALUATED BY FIRST-HAND CENSUS of the local SOTN master checkout (db41b28eee52969244a52cc269c8163d1ed8826a, 1,675 src .c files): NO instance anywhere of a redundant, simultaneously-live second C handle on one global address kept because the match needs it. 65 permissive hits are distinct sub-objects; the 21 strict hits are non-matching ports, global stores, or two genuinely different walkers.

- [s22] [s22] The fake/match-annotated address-taking lines in SOTN's matched PSX trees (dra, ric, st, main, weapon, servant, boss) are the FakePrim/VertexFake TYPE-PUNNING family — one handle cast to a different struct type — so they do not support the construct even by analogy.

- [s22] [s22] The last plausibly-untried lever (a 'u8 *r = &D_80106A70;' object owned by the tail copy loop and shared with block 1) was already measured dead at 22 in the rejected bank (base70-disp3-loop-shares-object-score22.c), so the escalation was not filed over an unexplored axis.

- [s22] [s22] Exhaustion, as inherited and re-stated for the entry: floor FLAT AT 10 for nine consecutive sessions (s14..s22) across five distinct modalities (forensics, rederive, synthesis, structural, permuter), thirteen earlier sessions before those, 128 disproven forms banked in memory/grind/func_80034F88/rejected/, and ~170,000 permuter iterations over five campaigns including the first two ever seeded AT the floor (s21).

- [s22] [s22] DISPOSITION FILED THIS SESSION in docs/grind/decisions.md: '2026-08-13 — func_80034F88 — OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE'. It states both gates' evidence, the mechanism of the 10-point residual, and the exhaustion record. Terminal — nothing is pending on the owner.

- [s22] [s22] Session scope: no src/ change survives (git status shows only docs/grind/decisions.md, the two ledger files, memory/grind/func_80034F88/candidate.c's header, and metrics/events.jsonl). No commit, no queue/retire command, no touch of regfix.txt/asmfix.txt/.claude/rules/engine/tools/Makefile/*.ld.

==== s23 (escalation / owner-directive: SOLVER) ====

- [s23] CHASSIS RE-MEASURED ON THE NEW MAIN. Since s22 the tree went through the
  asm-until-matched sweep-3 migration (commit 3a5882b2): `src/code6cac_b.c:2389`
  is now `INCLUDE_ASM("asm/funcs", func_80034F88);` and the function carries
  **ZERO regfix rules, ZERO asmfix rules and ZERO cheat-asm**. s22's disposition
  text ("main keeps the 31 regfix rules + three scheduling barriers that hold the
  byte-match") is STALE and must not be quoted again: there is no retained cheat
  and no accepted debt on main for this function any more. Installing
  `memory/grind/func_80034F88/candidate.c` (the single-pointer-object body) plus
  one `extern u8 D_80106A70;` declaration and running
  `sandbox func_80034F88 --disable all` gives **score 10, 49 target insns / 49
  build insns, rules_dropped 0** — the ledger floor of 10 reproduces exactly on
  the new chassis, so every banked spelling conclusion remains chassis-valid.

- [s23] OWNER DIRECTIVE EXECUTED FOR THE FIRST TIME IN 23 SESSIONS. The queue
  item's 2026-08-24 directive ("solver modality (ra_solver/sched_solver)
  recommended before deep re-grind of RA/scheduler-tiebreak residuals") had never
  been acknowledged in this ledger (the driver's consistency audit flagged it).
  It is now executed end-to-end; full console output is banked at
  `tmp/grind/func_80034F88/s23/ra_solver_report.txt`, the extracted models at
  `f88.model.json` (floor-10 chassis) and `f88.diag.model.json` (s20's
  exact-census diagnostic chassis).

- [s23] THE FORWARD MODEL IS EXACT FOR THIS FUNCTION — the solver's verdicts here
  are not extrapolation. `tools/ra_solver/extract.py func_80034F88 code6cac_b`
  yields 9 allocnos; `simulate.py` reproduces **sort order: MATCH** and
  **dispositions 9/9 match** against the instrumented-cc1 ALLOCDBG ground truth.
  The measured allocation is
  `{72:$a1, 73:$v1, 74:$a0, 77:$v1, 78:$v0, 81:$v1, 82:$v0, 85:$v1, 86:$v0}`,
  with pseudo 74 = the `&D_80106A73` address object (nrefs 10, livelen 29,
  pri 10344, conflicts with every other allocno) and pseudo 72 = `p`
  (`func_80077D00()`'s result, $a1).

- [s23] THE RA HALF OF THE RESIDUAL IS TYPED **FORECLOSED**, twice, by the
  inverse solver — a first-class validated negative, not a search timeout.
  * `inverse.py global --swap 74,73` (i.e. block 1's base↔value flip that the
    target exhibits: target holds the base in `$v1` and the flag byte in `$a0`,
    we hold them the other way round) → FORECLOSED, "no perturbation of any
    modelled input, up to depth 2, reaches the target assignment", with 18
    preference atoms reported NOT EMITTABLE.
  * `inverse.py global --goal '{"74": 3}'` (the one-sided move: address object
    into `$v1`) → FORECLOSED, 8 non-emittable preference atoms.
  The atom space searched was 218 single perturbations over 8 classes (refs,
  live length, birth order, conflicts, preferences, calls-crossed, ...), i.e.
  exactly the classes a C spelling can move.

- [s23] THE MECHANICAL REASON, in the allocator's own vocabulary and new to this
  ledger: **`$v1` and `$a0` never appear as hard registers in this function's
  pre-RA RTL**, so `global.c set_preference` can never record a preference for
  either of them, for any pseudo. The preference lever — the strongest dial the
  allocator exposes to C — is structurally unavailable HERE, because the function
  has no call argument setup, no hard-register-returning intrinsic and no
  two-address idiom that would put those registers into the RTL before allocation.
  This is why 22 sessions of live-range, ref-count and declaration-order surgery
  were all inert: they perturb inputs the solver has now enumerated, and none of
  them reaches the goal.

- [s23] THE SOURCE-MODEL CEILING IS CONFIRMED FROM THE ALLOCATOR'S INPUT SIDE.
  s20's diagnostic chassis (`rejected/roundtrip-fresh-pseudo-target-census-
  score10-DEAD-ARITH.c`, `q = q + 3; q = q - 3;`) reproduces the target's EXACT
  memory-access census (lbu 176 / sb 164 / lui 456 at 49 insns) and was the one
  form that might have created a second address pseudo. Extracted and modelled
  this session: it does **not**. The model still has exactly **9 allocnos**, and
  the address object is still the single pseudo 74 — merely with nrefs 10 → 15
  and livelen 29 → 30. The extra `lbu` is extra REFS on one allocno, not a second
  allocno. `inverse.py global --goal '{"74": 3}'` on that model is FORECLOSED as
  well. So on the only chassis where the instruction multiset matches the target,
  the residual is purely RA and RA is foreclosed.

- [s23] Combining that with the pipeline funnel `inverse_compose classify` uses
  (multiset differs → PRE-RA; multiset matches, texts differ → RA; texts match,
  order differs → SCHED): the floor-10 body classifies PRE-RA (our maspsx nop vs
  target's second `lbu`, census 175 vs 176), and the census-exact diagnostic body
  classifies RA — and RA is FORECLOSED. Both halves of the residual are therefore
  typed dead by the solver, from opposite sides. The only remaining producer of
  target's assignment is a SECOND ADDRESS ALLOCNO, which needs a second pseudo,
  which needs a second C object aliasing `D_80106A73` — the construct the Judge
  banned on 2026-08-13 and the construct s22's SOTN-master census found no
  precedent for.

- [s23] GATE 1 RE-RUN THIS SESSION (`tools/scan_hand_coded.py --single
  func_80034F88`, output `tmp/grind/func_80034F88/s23/scan_hand_coded.txt`):
  **tier=LOW score=0/8**, all eight signals unset (S1 0 multu/mflo pairs, S2 no
  empty-body branches, S3 49 insns/1 spill/4 distinct regs, S4 max load burst 3,
  S5 jaccard < 0.5, S6 no BIOS jumptable, S7 all callee-save uses saved, S8 no
  redundant mask-before-shift). Unchanged from s22; canonical-asm refused.

- [s23] THE SOLVER NAMES ITS OWN NEXT MOVE, and it is TOOLING, not C: "the
  mechanism is outside the current model — the local-alloc SUGGESTED-REGISTER
  pass (`qty_phys_copy_sugg` / `qty_phys_sugg`, reported-not-scored today),
  `qty_size` for DImode, or reload's spill-retry. Extend the instrumentation
  before spending another spelling search." Two of those three are inapplicable
  here on inspection (no DImode quantity in this function; `reload_sim` needs a
  retry block and this function's allocation is validated 9/9 pre-reload with one
  spill, the `$ra` save). The suggested-register pass is the one unmodelled
  mechanism that is not excluded by the measurements — and extending it means
  adding an env-gated fprintf to `tools/gcc-2.7.2/local-alloc.c` `block_alloc`
  plus a parser, i.e. work in `tools/`, a surface a grind session may not touch.
  That is the one decidable, non-standard-lowering question this residual poses.

- [s23] Session scope: `src/code6cac_b.c` was edited only to install the banked
  candidate and (temporarily) the s20 diagnostic body for model extraction, and
  is reverted to HEAD (`INCLUDE_ASM`) at end of session — `git diff` over the
  tree shows only the ledger files, docs/grind/decisions.md and
  metrics/events.jsonl. No commit, no queue/retire command, no touch of
  regfix.txt / asmfix.txt / .claude/rules/ / engine/ / tools/ / Makefile / *.ld.

- [s23] CHASSIS: since s22 the asm-until-matched sweep-3 migration (commit 3a5882b2) made src/code6cac_b.c:2389 `INCLUDE_ASM("asm/funcs", func_80034F88);`. The function now carries ZERO regfix rules, ZERO asmfix rules and ZERO cheat-asm. The 2026-08-13 disposition entry's claim that 'main keeps the 31 regfix rules + three scheduling barriers that hold the byte-match' is STALE and must not be quoted again — there is no retained cheat and no accepted debt for this function, so the disposition costs nothing and lowers no standard.

- [s23] FLOOR REPRODUCED ON THE MIGRATED TREE: installing memory/grind/func_80034F88/candidate.c plus one `extern u8 D_80106A70;` and running `sandbox func_80034F88 --disable all` gives score 10, 49 target insns / 49 build insns, rules_dropped 0. Every banked spelling conclusion remains chassis-valid. src/ reverted to HEAD at end of session.

- [s23] SOLVER FORWARD MODEL IS EXACT FOR THIS FUNCTION (so its negatives are not extrapolation): 9 allocnos; simulate.py reports 'sort order: MATCH' and 'dispositions: 9/9 match' against the instrumented-cc1 ALLOCDBG ground truth.

- [s23] TYPED FORECLOSED, TWICE: inverse.py global --swap 74,73 and --goal {"74": 3} both return 'NEGATIVE RESULT: no perturbation of any modelled input, up to depth 2, reaches the target assignment', over an atom space of 218 single perturbations across the eight input classes a C spelling can move.

- [s23] THE MECHANICAL REASON, new to this function's record: $v1 and $a0 never appear as hard registers in this function's pre-RA RTL, so global.c set_preference can never record a preference for either. The preference lever — the strongest dial the allocator exposes to C — is structurally absent here. This retires the 22-session description of the residual as an 'RA tie-break': it is not a tie the allocator could have broken the other way.

- [s23] CEILING NOW PROVEN FROM THE ALLOCATOR'S INPUT SIDE, not inferred from the emitted stream: s20's census-exact chassis (rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c) does NOT produce a second address allocno — 9 allocnos unchanged, pseudo 74 only gains refs (10 -> 15) and one unit of live length. Target's two simultaneously live base registers therefore require a second address ALLOCNO => a second pseudo => a second C object aliasing D_80106A73, which is the construct the Judge banned on 2026-08-13 16:36.

- [s23] GATE 1 (canonical-asm) RE-RUN THIS SESSION AND FAILS: tools/scan_hand_coded.py --single func_80034F88 => tier=LOW score=0/8, all eight signals unset (S1 0 multu/mflo pairs, S2 no empty-body branches, S3 49 insns / 1 spill / 4 distinct regs, S4 max load burst 3, S5 jaccard < 0.5, S6 no BIOS jumptable, S7 all callee-save uses saved, S8 no redundant mask-before-shift). No STRONG S1/S2/S6 signal.

- [s23] GATE 2 (SOTN-master precedent for two simultaneously live C handles on one global) FAILS, unchanged from s22's first-hand census of sotn-decomp master db41b28eee52969244a52cc269c8163d1ed8826a (1,675 src/*.c): 21 candidate functions, every one disqualified as a non-matching port, a global-to-global store, or two genuinely different walkers starting at the same node. A negative census is a failed gate, not an open question.

- [s23] NO AUTO-REJECT PACKET FILED: per the owner's second 2026-08-24 ruling, a packet whose YES would lower a standard (sanctioning the no-precedent multi-handle family, overriding the canonical evidence bar, or 'accepting the debt') is pre-decided NO. The filed entry asks for none of those; it records the refusal of both gates, which is the standing ruling applied.

- [s23] THE ONE DECIDABLE, NON-STANDARD-LOWERING QUESTION is routing/tooling: build the local-alloc suggested-register instrumentation (one env-gated fprintf in tools/gcc-2.7.2/local-alloc.c block_alloc dumping qty_phys_copy_sugg/qty_phys_sugg — already listed in the ra_solver README as a known hook gap — plus the matching parse in tools/ra_solver/local_extract.py), or do not. YES gives this function a typed verdict on its only unmodelled mechanism and every future RA-seat residual in the queue inherits the extension; NO leaves the FORECLOSED verdict conditional on that single pass and closes the item at floor 10.

- [s23] EXHAUSTION RECORD: floor FLAT AT 10 for ten consecutive sessions (s14..s23) across six distinct modalities (forensics, rederive, synthesis, structural, permuter, escalation/solver), on top of thirteen earlier sessions, 128 disproven forms banked in memory/grind/func_80034F88/rejected/, ~170,000 permuter iterations over five campaigns including two seeded at the floor itself.

- [s23] SESSION SCOPE: src/code6cac_b.c was edited only to install the banked candidate and (temporarily) the s20 diagnostic body for model extraction, and is reverted to HEAD. git diff shows only docs/grind/decisions.md, the two ledger files, memory/grind/func_80034F88/candidate.c's header and metrics/events.jsonl. No commit, no queue/retire command, no touch of regfix.txt / asmfix.txt / .claude/rules/ / engine/ / tools/ / Makefile / *.ld.

==== s24 (escalation — owner ruling 1 EXECUTED: the local-alloc SUGGESTED-REGISTER pass) ====

CHASSIS. candidate.c installed at src/code6cac_b.c:2546 (replacing the
INCLUDE_ASM line); both `extern u8 D_80106A70;` (include/code6cac.h:469) and
`extern u8 D_80106A73;` (src/code6cac_b.c:127) already exist on main, so no
declaration was added this session. `sandbox func_80034F88 --disable all` =
**score 10, 49 target insns / 49 build insns, rules_dropped 0**. The floor is
reproduced on today's tree; src/ was reverted to HEAD before the session ended.

WHAT THIS SESSION COULD DO THAT s23 COULD NOT. s23's entire frontier was one
item: the local-alloc SUGGESTED-REGISTER pass (`qty_phys_copy_sugg` /
`qty_phys_sugg`), "reported-not-scored" by ra_solver, the single mechanism its
FORECLOSED verdicts were conditional on. It named the work — one env-gated
fprintf in tools/gcc-2.7.2/local-alloc.c block_alloc plus a parse in
tools/ra_solver/local_extract.py — and correctly noted that both files are
outside a grind session's surface. The owner GRANTED exactly that work on
2026-08-30 (decisions.md, "OWNER RULINGS — escalation batch resolved", ruling 1:
"Local-alloc instrumentation — GRANTED (func_8002EA24, func_80034F88 ...);
sessions on them route via the solver modality once the instruments exist").
**The instruments now exist**: commit 70d6c905 ("engine: ra_solver — the
suggested-register pass, modelled EXACTLY (Phase 7)") added BB2_SUGG_DEBUG
(SUGGDBG-QTY in block_alloc, SUGGDBG-FFR in find_free_reg) and
`local_extract.py --suggest`, with the model validated corpus-wide at
preference 1578/1578 and assignment 947/947 across all 32 TUs. Phase 7 ran the
probe for camera_set_zoom and DispPracticeMenuTex_A but NOT for this function.
This session is the first that could run it here, and it is therefore the first
session since s14 with a genuinely un-tried lever rather than another spelling.

THE MEASUREMENT (full console record: tmp/grind/func_80034F88/s24/sugg_report.txt;
raw tables tmp/grind/func_80034F88/s24/sugg_summary.txt; models
tmp/ra_solver_work/code6cac_b.{local,sugg}.json).

  python3 tools/ra_solver/local_extract.py code6cac_b --func func_80034F88 --suggest

(1) THE PASS IS INERT HERE — MEASURED, NOT ASSUMED. func_80034F88 has seven
local-alloc quantities and they are ALL "main"-pass rows; the stream contains
not one QTYDBG-SUGG line. The complete SUGGDBG-QTY input table reads
ncopysugg=0, nsugg=0, copysugg=[], sugg=[] on all seven:

  blk=0  qty=0 reg1=76 size=1 mode=1      blk=0  qty=1 reg1=79 size=1 mode=4
  blk=3  qty=0 reg1=83 size=1 mode=4      blk=6  qty=0 reg1=87 size=1 mode=4
  blk=10 qty=0 reg1=91 size=1 mode=4      blk=10 qty=1 reg1=93 size=1 mode=1
  blk=10 qty=2 reg1=89 size=1 mode=4

Stronger, from the SUGGDBG-FFR side: all seven find_free_reg calls have
`used == first_used`. local-alloc.c:2205-2213 is the only place the suggestion
sets enter allocation — `if (just_try_suggested) IOR_COMPL_HARD_REG_SET
(first_used, qty_phys_{copy_,}sugg[qty])`. With the sets empty that restriction
is the identity map, so the pass CANNOT change a seat in this function. Not
"did not on this chassis" — cannot, for any chassis with these inputs.

(2) THE MECHANISM, READ IN COMPILER SOURCE. tools/gcc-2.7.2/local-alloc.c:1859-
1899 (combine_regs) sets qty_phys_copy_sugg / qty_phys_sugg on exactly two
branches, guarded by `ureg < FIRST_PSEUDO_REGISTER` and
`sreg < FIRST_PSEUDO_REGISTER`: a suggestion exists only where one side of a
tieable copy is a HARD register. s23 established that $v1 and $a0 never appear
as hard registers in this function's pre-RA RTL (no call-argument setup — the
sole call `func_80077D00()` takes none — and no hard-reg-returning idiom
besides $v0). So the suggestion pass is dead for precisely the same structural
reason global.c set_preference is dead: it was never an independent second
chance at the seat, it is the same missing hard register seen from another pass.

(3) THE CONTESTED PSEUDO IS NOT A LOCAL-ALLOC OBJECT AT ALL. Local-alloc's
pseudo set here is {76,79,83,87,89,91,93}; s23's global allocno set is
{72,73,74,77,78,81,82,85,86}. They are DISJOINT. Pseudo 74 — the `&D_80106A73`
address object whose seat IS the residual — is a global allocno, so local_alloc
never assigned it and no local-alloc pass sits in its causal chain. This is the
same structural refutation Phase 7 recorded for DispPracticeMenuTex_A, and it
is a stronger statement than "no suggestion on the contested pseudo".

(4) THE OTHER NAMED HOOK GAP IS ALSO MEASURED ABSENT. qty_size is 1 for every
one of the seven quantities, so the DImode mispricing gap cannot be operating
here either — independently reconfirming s23's "all 9 allocnos are mode SI".

(5) THE SOLVER'S OWN VERDICT. `inverse.py local ... --sugg ... --goal {"74": 3}`
run for all four blocks (0, 3, 6, 10) returns NEGATIVE RESULT / FORECLOSED over
55, 24, 24 and 55 single perturbations, each reporting the goal unit as absent
from the block (`qty 74: - -> $v1`), which is (3) restated by the tool.

CONSEQUENCE FOR THE FUNCTION'S RECORD. s23's FORECLOSED verdict carried an
explicit condition — "conditional on that one unmodelled pass". That condition
is now discharged. The RA residual of func_80034F88 is FORECLOSED
UNCONDITIONALLY: every modelled input class (refs / live length / birth order /
conflicts / preferences / calls-crossed) was searched to depth 2 by s23 and the
one unmodelled mechanism is now measured inert three independent ways. There is
no remaining pass through which a C spelling could reach the target's block-1
base-vs-value naming, and the target's two simultaneously live base registers
still require a second address allocno = a second pseudo = a second C object
aliasing D_80106A73, the construct banned on 2026-08-13 for this function.

GATE RE-RUNS (both fail, both re-measured this session).
  Gate 1 — `python3 tools/scan_hand_coded.py --single func_80034F88`
  (tmp/grind/func_80034F88/s24/scan_hand_coded.txt): tier=LOW score=0/8, all
  eight signals unset. No STRONG (S1/S2/S6) signal => canonical asm refused.
  Gate 2 — SOTN-master precedent for two simultaneously live C handles on ONE
  global address. s22 censused this first-hand (sotn-decomp master
  db41b28eee52969244a52cc269c8163d1ed8826a) and got NEGATIVE. Reconfirmed this
  session from a SECOND, INDEPENDENT source: the machine-generated
  docs/reference/sotn-construct-index.md (commit aa53500226ee84be763f3e8702b27de06456b3a7)
  carries 206 `pointer_alias` rows, 163 of them PSX/GCC-2.7.2. Grouping them by
  (file, identical RHS address expression) yields 33 groups with more than one
  alias — and every one is one alias per FUNCTION repeated across sibling
  functions in the same file (line spans 12 to 3373). The two tightest were read
  first-hand in the checkout and both are ordinary logic in DIFFERENT functions:
  src/dra/4DA70.c:30 and :42 are `func_800EDAE4` and `func_800EDB08`, and
  src/st/rare/e_azaghal.c:475-477 / :489-491 are `InitPositionLerp` and
  `ApplyPositionLerp`, each taking three aliases to three DIFFERENT members
  (base / pos / offset). Zero PSX functions in SOTN master hold two live handles
  on one address. (Script: tmp/grind/func_80034F88/s24/sotn_index_gate2.txt.)

NO NEW SPELLING WAS TRIED THIS SESSION and nothing was added to rejected/ — the
mandated modality was disposition, the un-tried lever was the granted instrument,
and it measured inert. The floor is unchanged at 10 and candidate.c is unchanged.

- [s24] CHASSIS: candidate.c installed at src/code6cac_b.c:2546 (both extern decls already on main); `sandbox func_80034F88 --disable all` = score 10, 49 target insns / 49 build insns, rules_dropped 0. Ledger floor reproduced on today's tree; src/ reverted to HEAD before session end (git status shows no src change).

- [s24] The owner's 2026-08-30 ruling 1 granted exactly the instrumentation s23's frontier named, and it EXISTS on main as of commit 70d6c905 ('engine: ra_solver -- the suggested-register pass, modelled EXACTLY (Phase 7)'). s23 could not run this probe; s24 is the first session that could. Phase 7 itself ran it only for camera_set_zoom and DispPracticeMenuTex_A.

- [s24] MEASURED INERT (1): all 7 local-alloc quantities carry ncopysugg=0 / nsugg=0 / empty copysugg and sugg sets; the stream contains zero QTYDBG-SUGG rows.

- [s24] MEASURED INERT (2): all 7 find_free_reg calls have used == first_used. local-alloc.c:2205-2213 is the sole consumer of the suggestion sets (`if (just_try_suggested) IOR_COMPL_HARD_REG_SET (first_used, ...)`), so with empty sets the suggestion preference is the identity map -- the pass cannot change a seat here, as opposed to merely not having done so.

- [s24] MECHANISM (source-read, not inferred): tools/gcc-2.7.2/local-alloc.c:1859-1899 (combine_regs) sets qty_phys_copy_sugg / qty_phys_sugg only on the two branches guarded by `ureg < FIRST_PSEUDO_REGISTER` / `sreg < FIRST_PSEUDO_REGISTER` -- a suggestion exists only where one side of a tieable copy is a HARD register. $v1 and $a0 never appear as hard regs in this function's pre-RA RTL (s23), and the sole call func_80077D00() takes no arguments, so the suggestion pass is dead for the same structural reason global.c set_preference is dead. It was never an independent second chance.

- [s24] STRUCTURAL: local-alloc pseudo set {76,79,83,87,89,91,93} is DISJOINT from the global allocno set {72,73,74,77,78,81,82,85,86}; the contested address pseudo 74 is a GLOBAL allocno that local_alloc never assigned.

- [s24] qty_size == 1 on all seven quantities: the other named Phase-5 hook gap (DImode mispricing) is measured absent too.

- [s24] inverse.py local --sugg --goal {"74": 3} returns NEGATIVE RESULT / FORECLOSED on all four blocks (0, 3, 6, 10) over 55/24/24/55 single perturbations.

- [s24] CONSEQUENCE: s23's FORECLOSED verdict was explicitly conditional on this one unmodelled pass. The condition is discharged; the RA residual of func_80034F88 is now FORECLOSED UNCONDITIONALLY. Reaching the target's two simultaneously live base registers still requires a second address allocno = a second pseudo = a second C object aliasing D_80106A73, banned for this function on 2026-08-13.

- [s24] GATE 1 (re-run this session): `python3 tools/scan_hand_coded.py --single func_80034F88` = tier=LOW score=0/8, all eight signals unset. No STRONG (S1/S2/S6) signal => canonical asm refused.

- [s24] GATE 2 (re-confirmed from a second independent source this session): NEGATIVE. See hypothesis 4.

- [s24] No packet is filed and none is available: the 2026-08-26 entry's single decidable question was the routing/tooling question 'build the hook extension?', the owner answered YES, it was built, and it measures inert. The only conceivable remaining ask (sanction a no-precedent multi-handle family) is the AUTO-REJECT class under the owner's second ruling of 2026-08-24.

- [s24] DISPOSITION FILED: docs/grind/decisions.md, entry '## 2026-08-30 - func_80034F88 - OWNER-ESCALATION - RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE (final: the owner's 2026-08-30 ruling-1 instrument was BUILT, RUN, and measures INERT)'. It supersedes the 2026-08-26 entry, whose refusal was conditional on exactly the pass measured here.

- [s24] No new spelling was tried and nothing was added to rejected/ this session -- the mandated modality was disposition, the un-tried lever was the granted instrument, and it measured inert. Floor unchanged at 10; candidate.c body unchanged (header updated with the s24 record).

- [operator 2026-09-02] owner ruling 2026-09-02 (decisions.md 'foreclosure mechanics'): re-activated — ledger floor 10 > ENDGAME_LOCK_MAX_FLOOR=5, so the 2026-07-27 standing ruling was never its subject; the ladder runs a second full cycle (20 flat sessions, >= 6 modalities) before any disposition. All standing banned_constructs remain in force. exhaustion_base=24

==== s25 (synthesis) ====

- [s25] CHASSIS RE-MEASURED ON HEAD. `src/code6cac_b.c:3420` is
  `INCLUDE_ASM("asm/funcs", func_80034F88);` (asm-until-matched). Installing
  `memory/grind/func_80034F88/candidate.c` verbatim needs NO extra extern
  (`include/code6cac.h:472` already declares `extern u8 D_80106A70;` and
  `src/code6cac_b.c:128` declares `extern u8 D_80106A73;`). Measured this
  session: `sandbox func_80034F88 --disable all` = **score 10, 49 target insns /
  49 build insns, rules_dropped 0, scorable true**. The dispatch brief's
  "measurement unavailable" chassis line is resolved: floor 10 is chassis-valid
  on HEAD and every banked conclusion carries over unchanged.

- [s25] THE FULL INSTRUCTION-ALIGNED DIFF, re-derived from scratch this session
  (build stream `tmp/grind/func_80034F88/s25/build.txt`, target
  `asm/funcs/func_80034F88.s`). This is the first time the ledger carries the
  whole 49-vs-49 alignment in one place rather than in prose:

    idx  TARGET                     BUILD (candidate.c)
    4    lui   $v1,%hi(A73)         lui   $a0,%hi(A73)        DIFF (reg)
    5    addiu $v1,%lo(A73)         addiu $a0,%lo(A73)        DIFF (reg)
    6    lbu   $a0,0($v1)           lbu   $v1,0($a0)          DIFF (reg swap)
    7    move  $a1,$v0              move  $a1,$v0             same
    8    andi  $a0,$a0,0xF8         andi  $v1,$v1,0xf8        DIFF (reg)
    9    sb    $a0,0($v1)           sb    $v1,0($a0)          DIFF (reg swap)
    10   lw    $v0,0x20($a1)        lw    $v0,0x20($a1)       same
    11   lbu   $a0,0($v1)           nop                       DIFF (reload)
    12   andi  $v0,$v0,0x1          andi  $v0,$v0,0x1         same
    13   bnez  $v0,L                bnez  $v0,L               same
    14   ori   $v0,$a0,0x1          ori   $v0,$v1,0x1         DIFF (reg)
    15   addu  $v0,$a0,$zero        move  $v0,$v1             DIFF (reg)
    16   lui   $a0,%hi(A73)         sb    $v0,0($a0)          DIFF (order)
    17   addiu $a0,%lo(A73)         lui   $a0,%hi(A73)        DIFF (order)
    18   sb    $v0,0($v1)           addiu $a0,%lo(A73)        DIFF (order)
    19..48                          IDENTICAL (blocks 2 and 3, the whole copy
                                    loop, and the epilogue match instruction
                                    for instruction AND register for register)

  Read as an allocation statement: **our build is the target with blocks 0 and 1
  re-spelled in blocks 2/3's register convention.** The target uses base=$v1 /
  value=$a0 for blocks 0-1 and base=$a0 / value=$v1 for blocks 2-3; our single
  address object uses base=$a0 / value=$v1 EVERYWHERE, so blocks 2-3 come out
  exact and blocks 0-1 come out mirrored.

- [s25] THE SEAT SWAP IS A NET LOSS, priced. This closes s16's "$v1-vs-$a0 dial
  would be a net loss" as arithmetic rather than as a remark. If a single-object
  form could be made to seat the address in $v1 for the whole function (the
  target's blocks-0/1 convention), blocks 0 and 1 would gain the seven register
  positions above (idx 4,5,6,8,9,14,15), but blocks 2 and 3 - currently 100%
  exact, nine scored instructions each - would each acquire the mirror image of
  the same six register differences (lui/addiu reg, `lbu $a0,0($v1)`,
  `ori $v0,$a0`, `addu $v0,$a0`, `sb $v0,0($v1)`), i.e. about twelve new points
  against about eight recovered. **No single-seat assignment of one address
  allocno beats the current one.** The current chassis is the argmax over the
  whole one-address-object seat space, not merely a point in it. This also
  explains why 24 sessions of live-range / ref-count / declaration-order surgery
  aimed at the "$v1 dial" were not merely inert but pointed the wrong way.

- [s25] THE FRESH-PSEUDO ESCAPE IS POSITION-SPECIFIC AND DOES NOT REACH THE
  RESIDUAL. s20 established that a dead round-trip (`q = q + 3; q = q - 3;`)
  placed BEFORE block 1 creates a fresh address pseudo and recovers the target's
  missing `lbu` at zero instruction cost (still score 10, target's exact
  lbu 176 / sb 164 / lui 456 census). The open reading of that result was that
  the "one C object = one pseudo" barrier is false, and hence that the same
  trick moved to the block-1/block-2 boundary might give blocks 2-3 a different
  seat from blocks 0-1 without a second C object. **Measured this session: it
  does not.** Four placements, all on the score-10 chassis, all diagnostics
  never intended for installation (dead pointer arithmetic fails checklist
  T1/T2/T3):
    * round-trip ADDED after block 1's store, blocks 2/3 keeping their
      `q = &D_80106A73;` re-materialisations -> score 10, 49 insns, and the
      emitted stream is **byte-for-byte identical** to the base chassis
      (`diff tmp/grind/func_80034F88/s25/build.txt tmp/grind/func_80034F88/s25/v3.txt`
      is empty). The round-trip is folded away entirely at that position: no
      fresh pseudo, no extra insn, no seat change.
      Banked rejected/s25-roundtrip-at-b1b2-boundary-IDENTICAL-STREAM-score10-DEAD-ARITH.c
    * round-trip REPLACING block 2's re-materialisation -> score 11 at **47**
      insns: the address never dies, so the second lui/addiu pair is never
      emitted and the function comes out two instructions short.
      Banked rejected/s25-roundtrip-replaces-b2-remat-folds-away-score11-47insn.c
    * round-trip replacing block 2's AND block 3's -> score 13 at **45** insns
      (both re-materialisation pairs gone).
      Banked rejected/s25-roundtrip-replaces-b2b3-remat-folds-away-score13-45insn.c
    * round-trip before block 1 (s20's own form) re-measured on HEAD -> score 10
      at 49 insns, reconfirmed.
      Banked rejected/s25-roundtrip-before-b1-reconfirms-s20-score10-DEAD-ARITH.c
  Mechanism the four together name: the round-trip's only observable effect is
  to make an otherwise-redundant `q = &D_80106A73;` set NON-redundant to cse, so
  that set survives as its own address pair. It is a re-materialisation ENABLER,
  not an allocno splitter. Where the set is already non-redundant (blocks 2 and
  3, which sit after a CODE_LABEL that flushes cse's value table - the s7
  mechanism recorded at hypotheses.md:892) it is inert; where it replaces the
  set it deletes the pair. **A fresh RTL temp is not a fresh allocno for the C
  variable it feeds: the temp is copy-propagated into `q`'s pseudo, and `q`
  still has exactly one allocno (tools/gcc-2.7.2/global.c:426), hence exactly
  one hard register for its whole live range.** s20's "one C object = one pseudo
  is not a theorem" reading is hereby narrowed to its true scope: extra address
  temps can appear, but they cannot carry a second SIMULTANEOUSLY LIVE base.

- [s25] `&D_80106A70 + 3` AS THE BLOCK-2/3 RE-MATERIALISATION EXPRESSION ->
  score 12 at 49 insns. Two different address rtxs for the same byte
  (symbol_ref(A73)+0 for blocks 0-1, symbol_ref(A70)+3 for blocks 2-3) do NOT
  buy a second allocno either - still one `q`, one seat - and the differing
  %hi/%lo symbol costs 2 points. It is also the declaration pun the dispatch
  brief's auto-scan flags (`extern u8 D_80106A70;` indexed out of bounds), so it
  is inadmissible independently of its score.
  Banked rejected/s25-a70-plus-3-remat-b2b3-score12-DECL-PUN.c

- [s25] SIBLING func_80034708 (same TU, same global) - MANDATED TRANSPLANT
  DISCHARGED, NOTHING TO SPEND, ONE EVIDENTIARY GAIN. Its ledger has no
  candidate.c and its floor is 542 (one session, s1 2026-07-07), so there is no
  spelling to transplant onto this chassis. What it DOES carry is first-hand
  target evidence about this exact byte: func_80034708's jump-table cases 8 and
  9 spell `D_80106A73 ^= 1` / `^= 2` through a single long-lived pointer local
  (`s5 = &D_80106A73`, held across the whole phase-B loop). The original
  codebase's idiom for this byte is a **pointer object, not a direct symbol
  access**, which independently corroborates candidate.c's object model and
  independently corroborates s16's measurement that the plain-symbol family is
  far worse (28/29). It supplies no second handle: func_80034708 holds exactly
  one, in a function long enough to have wanted more.

- [s25] KILL RE-AUDIT (2026-09-01 protocol) - VACUOUS BY CONSTRUCTION, and
  discharged by substitution. `memory/grind/func_80034F88/state.json` has NO
  `kills[]` array at all (keys present: func, file, session_count,
  current_modality, floor_history, frontier, judge_constraints,
  banned_constructs, ladder_skip, pending_fixup, origin, created,
  exhaustion_base, last_unpark_reason, last_unpark_at). There is therefore no
  instance kill whose `measured_on` can be stale and `tools/fake_ablate.py` has
  nothing to ablate. The re-audit was instead discharged against the two
  closest-to-target BANKED FORMS, both re-measured on HEAD this session: the
  base chassis (candidate.c, 10 / 49 insns) and s20's census-exact fresh-pseudo
  diagnostic (10 / 49 insns). Both hold at their recorded scores.

- [s25] CONTRADICTION-RULE AUDIT (the brief's standing instruction). The weakest
  foreclosure in the ledger was s20's, because it was the only one whose own
  measurement contradicted the ceiling argument it sat under: s20 showed a fresh
  address pseudo IS creatable from one C object, which if it generalised would
  have voided s23/s24's "second base needs a second allocno needs a second C
  object" chain. That is the foreclosure this session re-opened and re-measured,
  and it closes cleanly in the ceiling's favour (position-specific, temp not
  allocno, stream byte-identical at the boundary that mattered). The remaining
  foreclosures - RA modelled exactly and FORECLOSED over 218 perturbations
  (s23), local-alloc suggested-register pass measured inert three ways on a
  DISJOINT pseudo set (s24) - are not weakened by anything measured here, and
  s25's seat-swap pricing supplies the argument they were missing: even a
  granted swap would make the function WORSE.

- [s25] THE ARRAY DECLARATION MEASURED FOR THE FIRST TIME (the dispatch brief's
  DATA MODEL signal, and F3 of this session's frontier). Every prior session
  varied the address SPELLING at the use site (`&D_80106A70 + 3`,
  `*(&D_80106A70 + i)`); none had ever changed the DECLARATION. Patched
  `include/code6cac.h:472-474` from three `extern u8 D_80106A70/71/72;` scalars
  to a single `extern u8 D_80106A70[4];` (the sandbox compiles only
  src/code6cac_b.c, so the two out-of-TU consumers - src/code6cac.c:340-342 and
  src/code6cac_c_mid.c:205 - do not affect the measurement; they WOULD have to
  be adapted in a real integration handoff). Header and src both restored to
  HEAD at the end of the session. Two forms:

    w2  array decl + copy loop spelled `D_80106A70[i] = *((u8 *)p + i + 0x17);`
        (flags still via `q = &D_80106A73;`)
        -> score 10, 49 insns. **EXACTLY the base chassis.**
        Banked rejected/s25-arraydecl-loop-depunned-score10-NEUTRAL-needs-header-handoff.c

    w1  array decl + flags ALSO via the array: `q = &D_80106A70[3];` in all
        three blocks
        -> score 12, 49 insns. Disassembled and reloc-dumped: the ONLY
        differences from the base chassis are three `addiu a0,a0,3` where the
        base emits `addiu a0,a0,0`, and `objdump -r` shows all six sites are
        R_MIPS_HI16/R_MIPS_LO16 against D_80106A70. After linking,
        %lo(D_80106A70)+3 == 0x6A73 == the target's own
        `addiu $v1,$v1,%lo(D_80106A73)` immediate. So the +2 is the known FALSE
        distance from engine/score.py not masking section-relative R_MIPS_LO16
        addends ([[sandbox-lo16-text-addend-false-distance]]), exactly as s17
        found for `&D_80106A70 + 3`. **w1 is byte-identical to the base chassis
        after link.**
        Banked rejected/s25-arraydecl-flags-via-index3-score12-FALSE-LO16-ADDEND.c

  Three consequences, all new:
  (a) The declaration pun the dispatch brief flags on candidate.c:374
      (`*(&D_80106A70 + i)` against a scalar `extern u8 D_80106A70;`) is
      REMOVABLE AT ZERO CODEGEN COST. The sanctioned fix is exactly the one the
      brief names - fix the DECLARATION, not the use site - and w2 proves it is
      free. It is not a single-file grind edit: `extern u8 D_80106A70[4];`
      requires src/code6cac.c:340-342 (`D_80106A70 = 0x11;` ->
      `D_80106A70[0] = 0x11;` etc, and the read at :345) and
      src/code6cac_c_mid.c:205 (`(Quad *)&D_80106A70` -> `(Quad *)D_80106A70`)
      to move with it, i.e. an integration handoff. It should be executed
      whenever this function next has a submittable body, and it removes a
      standing layer-1 finding for free.
  (b) The array/aggregate model is byte-COMPATIBLE with the target for the flag
      byte as well as for the colour bytes (w1). The disassembly cannot
      distinguish `%lo(D_80106A70)+3` from `%lo(D_80106A73)` in a linked PS-EXE,
      so the target's asm is NOT evidence against the one-object model.
  (c) But the aggregate model does NOT create a second live base. w1 still has
      exactly one `q`, one pseudo, one seat; blocks 2-3 keep the register they
      had and blocks 0-1 stay mirrored. Merging A70..A73 into one declared
      object changes the symbol the relocation names and nothing else. The
      aggregate-merge family is therefore a HYGIENE fix here, not a lever: F3 as
      posed in this session's frontier reset is measured and CLOSED in the same
      session that opened it.

- [s25] CHASSIS: HEAD carries `INCLUDE_ASM("asm/funcs", func_80034F88);` at src/code6cac_b.c:3420. Installing memory/grind/func_80034F88/candidate.c verbatim needs NO extra extern (include/code6cac.h:472 already declares D_80106A70, src/code6cac_b.c:128 declares D_80106A73). Measured this session: score 10, 49 target insns / 49 build insns, rules_dropped 0, scorable true. The dispatch brief's 'measurement unavailable' chassis line is resolved and floor 10 is chassis-valid.

- [s25] FULL 49-vs-49 ALIGNMENT re-derived from scratch and banked in evidence.md for the first time. Positions 0-3 and 7,10,12,13 and 19-48 are identical instruction-for-instruction AND register-for-register; the 10 points are positions 4,5,6,8,9 (block 0 register mirror), 11 (nop where the target reloads), 14,15 (block 1 register mirror) and the 16/17/18 ordering triple (target emits the second base's lui/addiu BEFORE block 1's store; we emit the store first, because with one C object the second set is a def of the register the store reads).

- [s25] SEAT SWAP PRICED AS A NET LOSS. The target runs base=$v1 / value=$a0 in blocks 0-1 and base=$a0 / value=$v1 in blocks 2-3; our single address object runs base=$a0 / value=$v1 everywhere, so blocks 2-3 are 100% exact and blocks 0-1 are mirrored. Granting the $v1 seat for the whole function would recover the seven register positions in blocks 0-1 and give back the mirror image of six register differences in EACH of blocks 2 and 3 (~+8 against ~-12). candidate.c is the ARGMAX of the one-address-object seat space, not a point in it we failed to improve -- which retires the whole class of live-range / ref-count / declaration-order probes that 22 of the first 24 sessions ran and explains why they were not merely inert but aimed the wrong way.

- [s25] s20's FRESH-PSEUDO ESCAPE IS POSITION-SPECIFIC. Round-trip at the block-1/block-2 boundary: score 10, 49 insns, stream BYTE-FOR-BYTE identical to the base chassis (empty diff). Replacing block 2's re-materialisation: 11 at 47 insns. Replacing block 2's and block 3's: 13 at 45 insns. Before block 1 (s20's form): 10 at 49, reconfirmed on HEAD. The construct makes a cse-redundant `q = &D_80106A73;` set survive as its own address pair; it does not split an allocno.

- [s25] ARRAY DECLARATION MEASURED FOR THE FIRST TIME IN 25 SESSIONS. `extern u8 D_80106A70[4];` + copy loop spelled `D_80106A70[i] = ...` scores 10 at 49 insns -- EXACTLY the base chassis. So candidate.c's flagged declaration pun is removable at ZERO codegen cost via the sanctioned declaration-level fix; it needs src/code6cac.c:340-342/:345 and src/code6cac_c_mid.c:205 to move with the header, i.e. an integration handoff, and it should be executed whenever this function next has a submittable body.

- [s25] THE AGGREGATE MODEL IS BYTE-COMPATIBLE BUT NOT A LEVER. Reaching the flag byte as `&D_80106A70[3]` scores 12, and objdump -d + objdump -r show the only differences are three `addiu a0,a0,3` against R_MIPS_LO16 D_80106A70, linking to the target's own 0x6A73 immediate -- the known false LO16-addend distance. So the target's disassembly cannot distinguish %lo(D_80106A70)+3 from %lo(D_80106A73) and is not evidence against the one-object model; but the merge creates no second live base.

- [s25] SIBLING func_80034708: no candidate.c, floor 542, nothing to transplant. Its target holds D_80106A73 through ONE long-lived pointer local (s5, jtbl cases 8/9 `^= 1` / `^= 2`) -- corroborating candidate.c's pointer-object model and holding exactly one handle itself.

- [s25] KILL RE-AUDIT VACUOUS BY CONSTRUCTION: state.json has no kills[] key at all, so no instance kill has a stale measured_on and tools/fake_ablate.py has nothing to ablate. Discharged by substitution instead -- the two closest-to-target banked forms (candidate.c and s20's census-exact fresh-pseudo diagnostic) were both re-measured on HEAD at 10 / 49 and both hold.

- [s25] CONTRADICTION-RULE AUDIT: the weakest foreclosure was s20's, because its own measurement (a fresh address pseudo IS creatable from one C object) contradicted the ceiling argument above it. Re-opened and re-measured this session; it closes in the ceiling's favour. The s23 (RA modelled exactly, FORECLOSED over 218 perturbations) and s24 (local-alloc suggested-register pass inert three ways on a DISJOINT pseudo set) foreclosures are not weakened by anything measured here, and s25's seat-swap pricing supplies the argument they were missing.

## s26 (solver) — the residual is PRE-RA, and cse.c is the pass

**Headline.** The first thing the solver playbook mandates —
`inverse_compose.py classify` — had never been run on this function in 25
sessions. It says **FIRST DIVERGENCE: PRE-RA**. The whole RA framing the ledger
carried from s16 to s25 was aimed one stage too late.

### The residual, restated exactly

`classify code6cac_b func_80034F88 --target-object build/src/code6cac_b.o
--ours-object tmp/sandbox/func_80034F88/code6cac_b.o`, on candidate.c installed
at HEAD (sandbox re-measured: score 10, 49 target insns, 49 build insns,
rules_dropped 0):

```
FIRST DIVERGENCE: PRE-RA
  ours only  : nop
  target only: lbu #,0(#)
```

One instruction differs in KIND, not in register: the target has a byte load
where we have a load-delay `nop`. Everything else in the 10-point score is
register naming *downstream* of that. Two independent defects, which every
prior session treated as one:

* **D1 — PRE-RA.** Block 1's reload of the flag byte is deleted. Target
  `/* 257B4 */ lbu $a0, 0x0($v1)` has no counterpart in our stream; our
  `4f64: nop` sits in its place.
* **D2 — RA.** Blocks 0-1 wear blocks 2/3's convention (base `$a0` + value
  `$v1`; target has base `$v1` + value `$a0`). This is the seat question s16-s25
  ground on, and it is only askable once D1 is fixed.

### Pass attribution, from the dumps (not guessed)

`pwsh tools/grinder/dump.ps1 func_80034F88`, then the count of `(mem:QI` inside
the `;; Function func_80034F88` region of each dump:

| dump | `(mem:QI` refs |
|---|---|
| `.rtl` | 8 |
| `.jump` | 8 |
| `.cse` | **7** |
| `.loop` / `.cse2` / `.combine` | 7 |

**cse.c, the first CSE pass, is the pass that deletes the load.** The RTL names
the insns:

```
(insn 14 (set (reg/v:SI 74) (symbol_ref:SI ("D_80106A73"))))
(insn 17 (set (reg:QI 75) (mem:QI (reg/v:SI 74))))
(insn 18 (set (reg:SI 76) (and:SI (subreg:SI (reg:QI 75) 0) (const_int 248))))
(insn 20 (set (mem:QI (reg/v:SI 74)) (subreg:QI (reg:SI 76) 0)))     <- block 0 store
(insn 25 (set (reg:SI 79) (mem/s:SI (plus:SI (reg/v:SI 72) (const_int 32)))))
(insn 29 (set (reg:QI 80) (mem:QI (reg/v:SI 74))))                   <- DELETED by cse
(insn 30 (set (reg/v:SI 77) (zero_extend:SI (reg:QI 80))))
```

cse forwards insn 20's stored value into insn 29/30 and folds the
`zero_extend` away because reg 76 is `and(...,248)` — high bits provably zero.
Note also insn 56: `(set (reg/v:SI 74) (symbol_ref ...))` — the ONE C object is
re-set per block, so all four materialisations share pseudo 74, hence one
allocno (`global.c:426`), hence one hard register. That is the D2 chain, and it
is intact; it is just not the whole residual.

### D1 has no one-object honest fix (eight shapes, all measured)

`tmp/grind/func_80034F88/s26/sweep1.txt` + `probe1.txt`:

| form | score / insns | classify |
|---|---|---|
| w00 base (candidate.c) | 10 / 49 | PRE-RA |
| w01 read duplicated into both arms | 10 / 49 | **still PRE-RA** |
| w02 same, all three flag blocks | 25 / 53 | — |
| w03 block 1 as `*q = c ? (*q\|1) : *q;` | 12 / 46 | — |
| w04 `do { ... } while (0);` around block 1 | 10 / 49 | still PRE-RA |
| w05 `v = *q;` before `c = p[8] & M;` | 10 / 49 | still PRE-RA |
| w06 value local typed `u8` | 19 / 51 | — |
| w07 `do { *q &= 0xF8; } while (0);` (block 0) | 15 / 50 | PRE-RA, **two** nops |
| w08 explicit `q = &D_80106A73;` in block 1 too | 10 / 49 | still PRE-RA |

The classifier itself names `split-read-defeats-hoist` as the honest lever for a
cse-merge residual; w01/w02 are that lever, and it does not work here, because
cse follows jumps at -O2 (the equivalence propagates into both arms) and jump2
re-merges the copies. w08 fails for a different and important reason: cse
recognises a re-set of the SAME pseudo to a value it already holds, rewrites the
source to the register and keeps the quantity — so no memory-table
invalidation happens and the load stays dead.

### What DOES restore the load — and a multiset-matching chassis exists

Two constructs restore it, both inadmissible, and between them they name the
mechanism exactly:

1. **Dead round-trip on the one object** (`q = q + 3; q = q - 3;` before
   block 1 — s20's diagnostic, x09/x10 here): score 10 / 49 insns, and
   `classify` flips to **RA**. So a chassis whose instruction multiset matches
   the target IS reachable; the residual on it is purely register naming.
2. **A second C pointer object read by block 1** (y02: `r = &D_80106A73;`
   inside block 1, `r` also serving blocks 2-3): score 13 / 49, `classify`
   **RA**, and — decisively — *no extra `lui`/`addiu`*. Still three %hi/%lo
   pairs, exactly as the target. cse turns the second handle's set into a copy
   of the first and flow propagates it away, while keeping `mem(r)` as its own
   table entry, so the load survives for free.

And the control: **y01** — two objects, but block 1 still reading through the
first pointer — is score 21 and **still PRE-RA**. So it is not object *count*
that restores the target's instruction; it is *block 1's read going through a
different pointer PSEUDO*.

### The RA foreclosure, re-derived on the corrected model

The brief's re-audit rule applies to the model, not just to a spelling: if the
base chassis is PRE-RA-divergent, then s23's and s24's `inverse.py` FORECLOSED
verdicts were computed on a model with the wrong insn set — the func_80072CD4
defect the tool exists to prevent. So the search was re-run on x09 (the
multiset-matching chassis):

* `extract.py func_80034F88 code6cac_b` → 9 allocnos, dispositions 17;
  `simulate.py` → **sort order MATCH, dispositions 9/9**. The model is
  genuinely different from s23's: pseudo 74's priority is 15000 here vs 10344
  there, moving it from 5th to 2nd in the allocation order.
* Baseline disposition (2=`$v0`, 3=`$v1`, 4=`$a0`, 5=`$a1`):
  `{72: $a1, 73: $v1, 74: $a0, 77: $v1, 78: $v0, 81: $v1, 82: $v0, 85: $v1, 86: $v0}`.
* `inverse.py global --goal '{"74": 3, "73": 4, "77": 4}'` (the FULL blocks-0/1
  disposition, not a subset) → **FORECLOSED**. `--goal '{"74": 3}'` →
  **FORECLOSED**. Depth 2, 260-atom space, 18 preference atoms mechanically
  non-emittable because neither `$v1` nor `$a0` ever appears as a hard register
  in this function's pre-RA RTL.

So the RA foreclosure is **upgraded, not voided**: it now stands on a chassis
whose instruction multiset matches the target, where the classifier's
"searching this produces fiction" objection does not apply.

### Why this matters for the standing ban

The ban and the 2026-08-13 Judge FAIL both classify the repeated
`u8 *q = &D_80106A73;` handles as a *register-allocation lever* — a construct
with no observable effect on the function's output. D1 is not a seat. It is one
extra `lbu` in the target's instruction stream, upstream of RA, and this session
measured that a second pointer pseudo read by block 1 is the only structure
short of dead arithmetic that produces it, at zero instruction cost. That is a
different factual claim from the one the ban rests on, and it is the question
s26 hands forward.

### Artifacts

`tmp/grind/func_80034F88/s26/`: `classify.txt`, `sweep1.txt`, `probe1.txt`,
`probe2.txt`, `probe3.txt`, `inverse_x09.txt`, `build.txt`, `x09_rt3.txt`,
`variants/`, `variants2/`, `variants3/`, plus the dumps at
`tmp/grind/func_80034F88/dumps/`. Nine forms banked to
`memory/grind/func_80034F88/rejected/`.

## s26 (solver, continuation run) — the scheduler axis is opened for the first time, and it closes on one dependence edge

**Context.** This is the second dispatch of session 26. The first s26 run wrote
and committed the section above but never produced an outcome JSON, so the
driver discarded it and re-dispatched; `state.json` still reads
`session_count: 25`. Everything in the section above was re-checked against
HEAD here and stands. This section is the NEW work.

- [s26b] CHASSIS RE-CONFIRMED ON HEAD. `src/code6cac_b.c:3420` carries
  `INCLUDE_ASM("asm/funcs", func_80034F88);`; installing
  `memory/grind/func_80034F88/candidate.c` verbatim measures **score 10,
  49 target insns / 49 build insns, rules_dropped 0, scorable true**. The
  dispatch brief's "measurement unavailable" is resolved; floor 10 is
  chassis-valid for everything below.

- [s26b] `tools/sched_solver` HAD NEVER BEEN RUN ON THIS FUNCTION in 25
  sessions (`grep sched_solver hypotheses.md` = 0 hits; the single hit in
  evidence.md is the queue item's 2026-08-24 directive text). The ledger's
  "16/17/18 ordering triple" residual — the target emits the second base's
  `lui`/`addiu` BEFORE block 1's store, we emit the store first — is an
  emission-order residual, exactly the class `sched_solver` exists to type.
  `extract.py code6cac_b` reports **parity=True** (the README's stale
  `parity=False` note for this TU no longer applies), funcs=128, blocks=1464,
  picks=7270.

- [s26b] TARGET PINNED PROPERLY. `mkasm.sh` refuses to carry the target for an
  INCLUDE_ASM-routed function (it copies `.hon.s` to `.tgt.s`), so the target
  stream was built by splicing `asm/funcs/func_80034F88.s` into
  `code6cac_b.hon.s` in place of our body, with register names normalised to
  cc1's numeric form (`tmp/grind/func_80034F88/s26/splice_tgt.py` ->
  `tmp/sched_map/code6cac_b.tgt.head.s`, 53 target lines against our 58).

- [s26b] GOALMAP: **all 11 basic blocks report `GOAL == OURS (identity)` in
  BOTH scheduler passes**, and difflib finds `moved: 0` on the honest->target
  alignment. Caveat recorded honestly: the alignment is weak
  (`equal 3, replace 8, delete 33, insert 38`) because register naming differs
  almost everywhere, so most goals are interpolated rather than resolved. The
  identity verdict is therefore corroborating, not decisive on its own — the
  decisive result is the dependence structure below.

- [s26b] THE ORDERING TRIPLE IS A DEPENDENCE EDGE, NOT A TIE. In the extracted
  model, the block that begins at block 1's join label is block 3, insns
  `{49 = block-1 store, 56 = the re-materialisation set of the address pseudo,
  59 = lw p[8], 60/64 = lbu/andi, 67 = the compare/jump}`. Its dependence list,
  identically in both passes, is:

  ```
  pass 2 block 3 deps: {"59": [[49,0]], "56": [[49,14]], "60": [[59,0]],
                        "64": [[49,0],[56,0]], "67": [[64,14],[60,0]]}
  ```

  `"56": [[49, 14]]` is an anti/output dependence from insn 49 to insn 56 —
  emitted by `tools/gcc-2.7.2/sched.c:1738`
  (`add_dependence (insn, XEXP (u, 0), REG_DEP_ANTI)` over `reg_last_uses`)
  because insn 49 USES the address pseudo and insn 56 SETS it. Block 6 (block
  2's store + block 3's re-materialisation) carries the mirror edge
  `"90": [[83,14]]` — and there the TARGET also emits the store first, which is
  why block 6 is already exact. The model reproduces both facts.

- [s26b] `perturb.py` TYPED VERDICT: **every** vector that reaches the target's
  block-3 order starts with `del_dep 56 <- 49`. Searching 95-96 single atoms
  plus depth-2 pairs, pass 1 finds 5 vectors and pass 2 finds 3 for goal
  `67,64,60,59,49,56`; for the goal that matches our own intra-block tie
  (`67,60,64,59,49,56`) pass 2's **minimal vector is the single atom
  `del_dep 56 <- 49` and nothing else**. No priority, LUID, cost or
  function-unit atom reaches the goal, at any depth searched, without it —
  `schedule_block` never releases an insn with an unsatisfied `LOG_LINKS`
  entry, so a dependence edge is not perturbable by scheduler inputs. The
  scheduler axis is therefore CLOSED on a one-address-pseudo chassis, and the
  ONLY C-level way to delete that edge is for the store and the
  re-materialisation to touch DIFFERENT pseudos.

- [s26b] THE "SECOND PSEUDO WITHOUT A SECOND POINTER OBJECT" ROUTE IS CLOSED AT
  THE MECHANISM, NOT JUST BY SCORE. A direct global access (`D_80106A73 |= 2;`)
  looked like the one construct that creates a fresh address pseudo per use
  while declaring no second C object. It does not: `CONSTANT_ADDRESS_P`
  (`tools/gcc-2.7.2/config/mips/mips.h:2369`) accepts `SYMBOL_REF`, so the
  access stays `(mem (symbol_ref))` and never allocates an address pseudo at
  all. Measured in the emitted bytes of `h1_ptr01_sym23`: block 2 becomes
  `lui $v0,%hi(sym)` + `lbu $v0,%lo(sym)($v0)` — **one `lui`, no `addiu`**,
  the %lo folded into the memory operand, 48 insns instead of 49. The target
  has three UNFOLDED `lui`+`addiu` pairs. So a plain-symbol access can never
  produce the target's address shape, and this single fact explains every
  plain-symbol and hybrid score in the bank (14, 16, 18, 20, 22, 26, 28, 29,
  30, 35) without re-deriving any of them.

- [s26b] FIVE NEW HYBRID FORMS MEASURED (pointer object for some blocks, plain
  symbol for the rest), all banked to `rejected/hybrid-*`:
  `h1` pointer blocks 0-1 / symbol 2-3 = **28** at 48 insns; `h2` pointer for
  the mask only = **22** at 52; `h3` pointer blocks 0-2 / symbol 3 = **16** at
  49; `h4` (h1 with the block-1 re-materialisation restored) = **28** at 48;
  `h5` pointer 0-1, symbol 2, pointer 3 = **20** at 50. None approaches 10.

- [s26b] CONVERGENCE. The first s26 run reached "block 1 reading through a
  different pointer pseudo is the only structure that restores the target's
  `lbu`" from `cse.c`. This run reaches "the store and the re-materialisation
  must sit on different pseudos" from `sched.c` dependence construction, and
  then shows the only pseudo-creating construct other than a pointer object
  (a direct symbol access) cannot make the target's address shape. Two
  independent passes, one conclusion: on this toolchain the target's
  instruction stream requires more than one pointer OBJECT on `D_80106A73`,
  which is the standing banned construct.

### Artifacts (this run)

`tmp/grind/func_80034F88/s26/`: `sched1.sh`, `sched2.sh`, `sched3.sh`,
`sched4.sh`, `sched5.sh`, `sched6.sh`, `splice_tgt.py`, `h1.dis`,
`variants4/` (5 forms), `code6cac_b.c.orig2`; plus
`tmp/sched_solver_work/code6cac_b.sched.json` and
`tmp/sched_map/code6cac_b.tgt.head.s`.

- [s26] This is the SECOND dispatch of session 26: the first s26 run committed its evidence.md/hypotheses.md sections and nine rejected forms but never wrote an outcome JSON, so the driver discarded it and state.json still reads session_count 25. Everything in that section was re-checked against HEAD here and stands; this session's work is recorded as s26b.

- [s26] tools/sched_solver had never been run on func_80034F88 in 25 sessions (0 hits in hypotheses.md; the single evidence.md hit is the queue item's own 2026-08-24 directive text), despite the ledger carrying an emission-order residual since s25.

- [s26] extract.py reports parity=True for code6cac_b (128 funcs, 1464 blocks, 7270 picks) -- the sched_solver README's stale parity=False note for this TU no longer applies.

- [s26] mkasm.sh cannot carry the target for an INCLUDE_ASM-routed function (it copies .hon.s to .tgt.s by design, owner ruling 2026-08-25); the target stream here was built by splicing asm/funcs/func_80034F88.s into code6cac_b.hon.s with register names normalised to cc1's numeric form.

- [s26] Extracted dependence lists, identical in both scheduler passes: block 3 = {"59":[[49,0]], "56":[[49,14]], "60":[[59,0]], "64":[[49,0],[56,0]], "67":[[64,14],[60,0]]}; block 6 = {"93":[[83,0]], "90":[[83,14]], "94":[[93,0]], "98":[[83,0],[90,0]], "101":[[98,14],[94,0]]}.

- [s26] h1's emitted bytes are the direct evidence that a plain-symbol access folds %lo into the memory operand: `lui $v0,%hi(sym)` + `lbu $v0,%lo(sym)($v0)`, one lui and no addiu, versus the target's three unfolded lui+addiu pairs.

- [s26] The two class kills converge with the first s26 run's cse.c finding from a completely different pass: cse.c says block 1's reload survives only if block 1 reads through a different pointer pseudo; sched.c says the ordering triple resolves only if the store and the re-materialisation sit on different pseudos; mips.h says the only construct that makes an address pseudo for a global is a pointer-typed value. All three point at a second pointer OBJECT, which is the standing banned construct.

- [s26] Scope note for a future escalation: unbanning the two-object family reaches 8 (s21), not 0. Only the three-object family measures 0, and the Judge FAILed that body at final call on 2026-08-13, so it cannot be resubmitted without a later Judge ruling.

## s27 (forensics) -- cse.c read at the predicate level; the one-object cost law measured

**Chassis.** `memory/grind/func_80034F88/candidate.c` installed at
`src/code6cac_b.c:3420`; `sandbox func_80034F88 --disable all` = **score 10,
49 target / 49 build insns, rules_dropped 0**. Floor 10 re-confirmed on HEAD.
`src/` restored to HEAD at session end.

**D1 located in the RTL, not inferred.** s26 named cse.c from a per-pass
`(mem:QI` count. This session read the two dumps side by side
(`tmp/grind/func_80034F88/s27/f88.jump` vs `f88.cse`, sliced out of
`tmp/grind/func_80034F88/dumps/`) and the rewrite is literal:

    .jump  (insn 29 (set (reg:QI 80) (mem:QI (reg/v:SI 74))))
    .cse   (insn 29 (set (reg:QI 80) (subreg:QI (reg:SI 76) 0)))

cse does NOT delete the insn -- it retargets the load's source to the register
block 0 stored (insn 20 `(set (mem:QI (reg 74)) (subreg:QI (reg:SI 76) 0))`),
turning a `lbu` into a QI register copy; the copy and its `zero_extend`
(insn 30) are then folded away downstream, which is where the instruction
actually disappears. The target's `lbu $a0, 0($v1)` at 0x80034FB4 is that
insn.

**The gate, quoted.** `canon_hash` hashes a REG on its QUANTITY, not its
number -- `hash += ((unsigned) REG << 7) + (unsigned) reg_qty[regno];`
(tools/gcc-2.7.2/cse.c:1905). So every `(mem:QI (reg 74))` in the function
hashes to the same slot for as long as reg 74 keeps one quantity, and the
store's table entry (recorded at cse.c:7338 `insert (dest, sets[i].src_elt,
...)`, gated only by `sets[i].src_elt == 0` at cse.c:7328) is found by every
later load.

**PASS-INPUT ENUMERATION (the complete set of source-side inputs that stop
the forwarding), each checked against the target's own instruction stream:**

1. *Volatile mem* -- `canon_hash` sets `do_not_record` for `MEM_VOLATILE_P`
   (cse.c:1943), which leaves `src_elt == 0` and skips the record. Measured
   dead as a family (12/15/28, `rejected/vol*`).
2. *A CALL / UNSPEC_VOLATILE inside the stored value* -- `do_not_record`
   (cse.c:1967). No call exists between the target's `sb` at 0x80034FAC and
   its `lbu` at 0x80034FB4, so this was not the original's input.
3. *A non-fixed hard register in the value* (cse.c:1902) -- only under
   `SMALL_REGISTER_CLASSES`, which MIPS does not define.
4. *Memory invalidated between the store and the load* -- `invalidate_memory`
   (cse.c:7599), driven by a CALL_INSN or a store through an address cse
   cannot disambiguate. The target's stream contains neither: `sb`, `lw`,
   `lbu`, nothing else.
5. *A cse basic-block-path boundary between them* -- paths break at code
   labels. `tools/label_census.py --func func_80034F88`
   (`tmp/grind/func_80034F88/s27/label_census.txt`) reports the target's FOUR
   labels at insns 16 / 25 / 34 / 37, all `preds=2`; the earliest is at
   insn 16, i.e. AFTER block 1's compare arms. There is no label between the
   store and the reload in the target either.
6. *A different address QUANTITY for the load* -- cse.c:1905. This is the only
   input left standing.

**The one-object cost law (six measured points, this session).** Ten bodies
were built varying only WHICH of the four byte-access groups re-assigns the
single `u8 *q`, and with which of two spellings (`&D_80106A73`, or
`(u8 *)((u8 *)&D_80106A70 + 3)` -- the same address, an rtx cse cannot unify
with the first). Driver `tmp/grind/func_80034F88/s27/sweep.ps1`, bodies in
`tmp/grind/func_80034F88/s27/variants/`, per-form disassembly `a*.txt`,
`inverse_compose.py classify` run on each:

| sets of `q` | form | insns | surviving `lbu` | score |
|---|---|---|---|---|
| 1 | a9  (A,-,-,-)   | 47 | 1 | 23 |
| 2 | a10 (A,-,A,-)   | 47 | 2 | 20 |
| 3 | **candidate.c** | 49 | 3 | **10** |
| 3 | a1/a2/a5/a8 (mixed spellings) | 49 | 3 | 22/14/14/22 |
| 4 | a3 (A,B,A,A)    | 51 | **4** | 12 |
| 4 | a4 (B,A,A,A)    | 51 | **4** | 12 |
| 4 | a11 (A,B,B,B)   | 51 | **4** | 14 |
| 4 | a12 (A,B,A,B)   | 51 | **4** | 13 |

Two exact identities across the series:
**surviving `lbu` == number of assignments to `q`**, and
**build insns == 41 + 2 x (number of assignments to `q`)** (a9's 47 is off the
line because a single set also flips the branch polarity of all three arms;
its `lbu` count obeys the first identity exactly). Every assignment buys
exactly one surviving byte load and costs exactly one `lui`+`addiu` pair,
because a new quantity is only created by setting the pseudo to an rtx cse
cannot unify with what it already holds -- and such an rtx is, by
construction, a fresh address materialisation.

**The target sits off that line.** It needs FOUR surviving `lbu` (0x80034FA0,
0x80034FB4, 0x80034FD8, 0x80034FFC) with THREE `lui`/`addiu` pairs
(0x80034F98, 0x80034FC8, 0x80034FF0) at 49 insns. The law gives 4 loads only
at 51 insns and 49 insns only at 3 loads. **The single-pointer-object family
cannot produce the target's instruction multiset, for arithmetic reasons, not
search reasons.** This closes the ledger's longest-standing open question
(s16-s26: "which spelling of one `q`") with a mechanism rather than
saturation.

**What a zero-cost fourth quantity requires.** Exactly one construct produces
a new quantity for zero instructions: a set whose SOURCE cse *can* unify
(so cse rewrites it to a register copy, which flow then propagates away) while
its DESTINATION is a different pseudo. In C that is a second declared pointer
object -- s26's y02, measured 49 insns / four `lbu` / three pairs / score 13 /
classify RA -- and the three-object body that reaches 0. Both are the standing
ban.

**s26 frontier item answered by inspection + measurement.** "Can the address be
carried by a pointer-typed value that is not a second object aliasing
D_80106A73 -- e.g. derived from `p` or from another live object that reaches
the byte?" The DATA MODEL records exactly one record covering the byte
(0x80106A70..0x80106A73: three default-colour nibbles plus the flag byte); `p`
(the `func_80077D00()` return) is unrelated memory. The one derived spelling
that exists, `&D_80106A70[3]`, was built in eight of the ten forms above: it
does create a distinct quantity, and it costs the same full `lui`+`addiu`
pair as any other materialisation, plus 2-12 points of `addiu #,#,3` vs
`addiu #,#,0` distance. The route is closed.

**FAKE re-audit (mandated).** `tools/fake_ablate.py --func func_80034F88
--file code6cac_b --candidate memory/grind/func_80034F88/candidate.c` reports
"no FAKE-annotated constructs found". The candidate's only construct of a
FAKE-requiring family is its single `u8 *q` pointer alias, so it was ablated
by hand: a13 spells all four groups as direct `D_80106A73` accesses and
measures **score 29 at 47 insns** with three `lbu` missing
(`rejected/pointer-alias-ablated-plain-symbol-47insn-score29.c`). The alias is
worth 19 points and is load-bearing; no lever is being masked behind it.

**Loop axis (mandated tool).** `tools/loop_movables.py --func func_80034F88
--file code6cac_b` (`tmp/grind/func_80034F88/s27/loop_movables.txt`): the copy
loop at insns 123..154 has `insn_count=6`, `loop_has_call=False`, threshold
122, and **an empty movable table** -- loop.c hoists nothing out of it, so
there is no LICM input to perturb. `tools/nrefs_census.py` could not run (its
`extract.py run_dumps` cpp step exits 1 on this TU); the .lreg/.greg dumps
under `tmp/grind/func_80034F88/dumps/` carry the same allocation data and
s23/s24 already mined them.

- [s27] Chassis re-confirmed: candidate.c at src/code6cac_b.c:3420 measures score 10, 49 target / 49 build insns, rules_dropped 0 on HEAD; src/ restored to HEAD at session end.

- [s27] D1 is now located in the RTL rather than inferred from a per-pass count: .jump insn 29 (set (reg:QI 80) (mem:QI (reg/v:SI 74))) becomes .cse insn 29 (set (reg:QI 80) (subreg:QI (reg:SI 76) 0)). cse RETARGETS the load's source to the register block 0 stored (insn 20), turning a lbu into a QI register copy; the copy and its zero_extend (insn 30) fold away downstream, which is where the instruction actually disappears.

- [s27] The gate is cse.c:1905 -- canon_hash hashes a REG on reg_qty[regno], not on the register number -- so all four byte accesses share one hash slot while `q` holds one quantity. The store's table entry is recorded at cse.c:7338 with the only relevant guard being sets[i].src_elt == 0 at cse.c:7328.

- [s27] Cost law measured at four points on the one-object family: assignments to q = 1/2/3/4 give surviving lbu = 1/2/3/4 and build insns = 47/47/49/51 (the 1-set point also flips all three arms' branch polarity, which is why its insn count is off the 41+2n line while its lbu count is not).

- [s27] Four independent 4-assignment forms (a3, a4, a11, a12) all restore EVERY byte load at 51 insns, scores 12/12/14/13, with classify showing only the surplus lui/addiu as 'ours only' -- the nop-for-lbu delta that has defined this residual since s26 is gone. That is the first time any admissible single-object form has matched the target's lbu count.

- [s27] The target sits off the law: four lbu (0x80034FA0/FB4/FD8/FFC) at three lui+addiu pairs (0x80034F98/FC8/FF0) in 49 insns.

- [s27] label_census on the target: 49 insns, four labels (0x80034FC8 @16, 0x80034FEC @25, 0x80035010 @34, 0x8003501C @37), all preds=2, callee-saved refs all zero. The earliest label is after block 1's compare arms, so the target's own compilation had no cse path boundary between block 0's store and block 1's reload either.

- [s27] loop_movables on the copy loop (insns 123..154): insn_count=6, loop_has_call=False, threshold 122, and an EMPTY movable table -- loop.c hoists nothing, so there is no LICM input to perturb on this function.

- [s27] tools/nrefs_census.py cannot run on this TU: its extract.py run_dumps cpp step exits 1 on src/code6cac_b.c. The .lreg/.greg dumps under tmp/grind/func_80034F88/dumps/ carry the same allocation data and s23/s24 already mined them.

- [s27] A zero-cost fourth address quantity requires a set whose SOURCE cse can unify (so cse rewrites it to a register copy that flow propagates away) into a DIFFERENT destination pseudo. In C that is a second declared pointer object -- s26's y02 (49 insns, four lbu, three pairs, score 13, classify RA) and the three-object body that measured 0 -- i.e. the standing ban.

## s28 (forensics) -- the cse input enumeration COMPLETED, and the round-trip's gate located in the RTL

**Chassis.** `candidate.c` (variant `b0`) installed at `src/code6cac_b.c:3420`;
`sandbox func_80034F88 --disable all` = **score 10, 49 build / 49 target insns**,
classify FIRST DIVERGENCE: PRE-RA with `ours only: nop` / `target only: lbu #,0(#)`.
The dispatch brief carried "measurement unavailable" for the HEAD floor -- it is
**10**, re-measured this session. `src/` restored to HEAD at session end.

**The residual restated arithmetically (this makes the remaining search finite).**
Our 49 insns and the target's 49 insns are the SAME COUNT with a 1:1 shape swap:
we emit a `nop` exactly where the target emits `lbu $a0,0($v1)` at 0x80034FB4 --
the load fills the load-delay slot of `lw $v0,0x20($a1)`. So restoring block 1's
byte reload is **free**: a construct that restores it without adding a second
instruction lands on the target's multiset. Every route measured so far adds two
(a fourth address materialisation: 51 insns; the call gate: 51 insns).

**PASS-INPUT ENUMERATION -- two gates s27 did not list, both closed this session.**
s27 enumerated five cse inputs (volatile mem cse.c:1943, CALL/UNSPEC_VOLATILE
cse.c:1967, hard reg cse.c:1902, invalidate_memory cse.c:7599, path boundary at a
code label) plus the address quantity. Reading `cse_insn`'s dest-recording loop
end to end (cse.c:7310-7360) turns up two more entry points to
`sets[i].src_elt == 0`, the condition at cse.c:7329 that skips
`insert (dest, ...)` and therefore leaves the stored byte unrecorded:

6. **`in_libcall_block`** (cse.c:7326). Reachable only if the STORE insn sits
   between a REG_LIBCALL and its REG_RETVAL. `emit_libcall_block` wraps only the
   call sequence, and `*q &= 0xF8` contains no libcall operation (no division, no
   float), so no ordinary-C spelling of block 0 puts the `sb` inside one.
7. **A bitfield destination** (cse.c:7004-7027): when `SET_DEST` is a
   `ZERO_EXTRACT` / `SIGN_EXTRACT`, cse sets `sets[i].src_elt = 0` and
   `sets[i].src_volatile = 1` *specifically so the destination is invalidated but
   no value is recorded* -- exactly the behaviour the target's stream needs.
   **MEASURED AND DEAD.** `b1`/`b1b` spell block 0 as a 3-bit bitfield store
   (`((struct FB *)q)->f3 = 0;`), both 51 insns, score 13/13, reload still
   forwarded. The mechanism: `mips.md:2901`'s `insv` expander `FAIL`s for every
   field that is not a 32-bit byte-aligned field, so `store_bit_field` falls back
   to `store_fixed_bit_field`'s explicit load/and/store RTL. Direct proof --
   `grep -c zero_extract` over `.rtl`, `.jump` and `.cse` is **0/0/0**, and the
   dumped block 0 is insn 17 `(set (reg:QI 75) (mem/s:QI (reg 74)))`, insn 20
   `and 248`, insn 22, insn 24 `(set (mem/s:QI (reg 74)) (reg:QI 75))`
   (`tmp/grind/func_80034F88/s28/b1b.jump.block0`). No `ZERO_EXTRACT` SET_DEST
   ever reaches cse on this target, so gate 7 is unreachable, not merely unlucky.

**Gate 8, also measured: MEM_IN_STRUCT_P asymmetry does not help.** In `b1b` the
store is `(mem/s:QI (reg 74))` and block 1's reload is `(mem:QI (reg 74))` --
different `MEM_IN_STRUCT_P` -- and the reload is STILL forwarded, because
`canon_hash` does not hash that flag and `exp_equiv_p` does not compare it. So
reaching the byte through a struct-typed lvalue in one block and a plain
`u8 *` in another is not a cse-defeating input.

**Gate 5 (invalidate_memory) measured for the first time, and priced.** `b2`
hoists block 0's read-modify-write ABOVE `p = func_80077D00()` so the CALL_INSN
invalidates the byte's table entry (cse.c:7599). **The reload IS restored** --
classify's only remaining shape difference is an extra `lw #,0x20(#)` /
`sw #,0x20(#)` pair (the pointer now has to survive the call) -- at 51 insns,
score 29. So the call gate costs +2, the same price as a fourth address
materialisation. The target's own stream has no call between its `sb` at
0x80034FAC and its `lbu` at 0x80034FB4, so this was not the original's input.

**MANDATED KILL RE-AUDIT -- the s25/s20 dead round-trip, re-measured on HEAD and
located in the RTL for the first time.** `b4` re-installs
`q = q + 3; q = q - 3;` between block 0 and block 1 on the current chassis:
**49 insns, score 10, and classify now reports FIRST DIVERGENCE: RA with a
MATCHING instruction multiset** (`ours: lbu v1,0(a0)` x2 vs
`target: lbu a0,0(v1)` x2 -- the whole residual is the $v1/$a0 seat swap).
The instance kill stands (it is a dead-arithmetic construct, and s26's RA inverse
FORECLOSES the swap on this very chassis), but the MECHANISM is now proven rather
than inferred. Side-by-side dumps
(`tmp/grind/func_80034F88/s28/b4.jump`, `b4.cse`):

    .jump  (insn 20 (set (mem:QI (reg/v:SI 74)) (subreg:QI (reg:SI 76) 0)))
           (insn 23 (set (reg/v:SI 74) (plus (reg/v:SI 74) (const_int 3))))
           (insn 26 (set (reg/v:SI 74) (plus (reg/v:SI 74) (const_int -3))))
           (insn 35 (set (reg:QI 80) (mem:QI (reg/v:SI 74))))
    .cse   insn 35 SURVIVES UNCHANGED  (in candidate.c it becomes
           (set (reg:QI 80) (subreg:QI (reg:SI 76) 0)) -- s27's D1)

Insns 23 and 26 SET the address pseudo, so `cse_insn` calls `invalidate` on
reg 74, which does `reg_tick[regno]++` at **cse.c:1539**; every table entry whose
expression mentions reg 74 -- including the `(mem:QI (reg 74))` that insn 20
recorded -- stops validating, and block 1's load is a real load again. cse still
folds the arithmetic (both insns carry REG_EQUAL notes: `D_80106A73 + 3`, then
`D_80106A73`), which is why the round-trip costs zero instructions. **That is the
only zero-cost invalidator measured in 28 sessions, and its C spelling is a dead
pointer round-trip.**

**Toolchain fact worth carrying off this function: bitfield direction flipped
with `-mel`.** `.claude/rules/bitfield-direction-divergence.md` (dated
2026-06-11, i.e. BEFORE the 2026-08-04 `-mel` adoption) states our cc1 allocates
the FIRST-declared bitfield at the HIGH bits and advises declaring SDK bitfield
structs in flipped field order. Measured here on the live toolchain:
`struct { u8 f5:5; u8 f3:3; }` with `f3 = 0` emits `andi #,#,0x1f` (so `f5`
occupies bits 0-4), and `struct { u8 f3:3; u8 f5:5; }` with `f3 = 0` emits
`andi #,#,0xf8`. **First-declared now takes the LOW bits** -- the rule's advice
is stale post-`-mel` and re-flipping field order would be wrong. Recorded here,
not acted on (rule files are outside a grind session's surface).

- [s28] HEAD chassis re-measured: candidate.c = score 10, 49/49 insns, classify PRE-RA with ours-only nop / target-only lbu. The dispatch brief's "measurement unavailable" resolves to floor 10.
- [s28] The nop-for-lbu swap is 1:1, so restoring block 1's reload is instruction-free; every measured route to it costs +2 (fourth materialisation 51 insns, call gate 51 insns).
- [s28] cse.c:7326 in_libcall_block is the sixth src_elt==0 gate; unreachable because `*q &= 0xF8` contains no libcall operation and emit_libcall_block wraps only the call sequence.
- [s28] cse.c:7004-7027 (ZERO_EXTRACT/SIGN_EXTRACT SET_DEST) is the seventh gate and is UNREACHABLE on MIPS: mips.md:2901's insv expander FAILs for non-32-bit fields, so store_bit_field emits explicit load/and/store RTL. zero_extract count is 0 in .rtl, .jump and .cse for the bitfield body b1b.
- [s28] Bitfield bodies b1 (f5 first) and b1b (f3 first) both measure 51 insns / score 13 with the reload still forwarded; b1 additionally pins the field order (andi 0x1f vs andi 0xf8).
- [s28] MEM_IN_STRUCT_P asymmetry between the store (mem/s:QI) and the reload (mem:QI) does not defeat forwarding -- canon_hash does not hash the flag and exp_equiv_p does not compare it.
- [s28] b2 (block-0 RMW hoisted above the call) restores the reload via cse.c:7599 invalidate_memory but costs +2 (an lw/sw pair keeping the pointer alive across the call): 51 insns, score 29.
- [s28] KILL RE-AUDIT: the s25/s20 dead round-trip re-measures at 49 insns / score 10 on HEAD with a MATCHING multiset and an RA-only residual ($v1/$a0 swap). Its cse mechanism is now RTL-proven: setting the address pseudo bumps reg_tick at cse.c:1539, invalidating the (mem:QI (reg 74)) entry insn 20 recorded, so insn 35 survives cse unchanged.
- [s28] Bitfield direction on the live toolchain: FIRST-declared field takes the LOW bits (measured andi 0x1f / andi 0xf8). .claude/rules/bitfield-direction-divergence.md predates the 2026-08-04 -mel adoption and its "declare in FLIPPED field order" advice is stale.

### Artifacts (s28)

`tmp/grind/func_80034F88/s28/`: `variants/b0.c`, `variants/b1.c`, `variants/b1b.c`,
`variants/b2.c`, `variants/b4.c`, `sweep.ps1`, `install.py`, `dis.sh`, `cls.sh`,
`b0.txt`, `b1.txt`, `b1b.txt`, `b2.txt`, `b4.txt`, `b4.jump`, `b4.cse`,
`b1b.jump.block0`, `code6cac_b.c.orig`; plus the refreshed pass dumps in
`tmp/grind/func_80034F88/dumps/`.

- [s28] HEAD chassis re-measured (the dispatch brief said 'measurement unavailable'): candidate.c installed at src/code6cac_b.c:3420 gives sandbox --disable all = score 10, 49 build / 49 target insns, rules_dropped 0. Floor is 10.

- [s28] The residual is a 1:1 shape swap, not a size gap: classify reports exactly 'ours only: nop' and 'target only: lbu #,0(#)'. The target's lbu $a0,0($v1) at 0x80034FB4 fills the load-delay slot of lw $v0,0x20($a1); we fill that slot with a nop. Restoring block 1's byte reload is therefore INSTRUCTION-FREE, and any construct that restores it without adding a second instruction lands on the target's multiset.

- [s28] Target address geometry, read off asm/funcs/func_80034F88.s: three lui+addiu pairs (0x80034F98 -> $v1, 0x80034FC8 -> $a0, 0x80034FF0 -> $a0) serving four lbu and four sb. Group A ($v1) carries TWO loads and TWO stores (lbu 0x80034FA0, sb 0x80034FAC, lbu 0x80034FB4, sb 0x80034FD0); groups B and C carry one load and one store each. So the entire deficit is one read-after-write inside a SINGLE address quantity.

- [s28] cse input enumeration completed. s27 listed five gates plus the address quantity; s28 adds cse.c:7326 in_libcall_block (unreachable -- emit_libcall_block wraps only the call sequence and *q &= 0xF8 contains no libcall operation) and cse.c:7004-7027, the bitfield ZERO_EXTRACT/SIGN_EXTRACT SET_DEST gate.

- [s28] The bitfield gate is unreachable on MIPS, not merely unlucky: mips.md:2901's insv expander FAILs for any field that is not 32 bits wide and byte-aligned, so store_bit_field falls back to explicit load/and/store RTL. Measured: grep -c zero_extract = 0 in .rtl, .jump and .cse for the bitfield body b1b.

- [s28] MEM_IN_STRUCT_P asymmetry (mem/s:QI store vs mem:QI reload, both on reg 74) does not defeat forwarding -- canon_hash does not hash that flag and exp_equiv_p does not compare it. Measured in b1b's dumped RTL (insn 24 vs insn 33) with the reload still deleted.

- [s28] The CALL gate (cse.c:7599 invalidate_memory) DOES restore the reload -- b2 measures 51 insns / score 29 with the lbu multiset matching and only an lw/sw spill pair as the shape difference -- but costs +2, the same price as a fourth address materialisation.

- [s28] The dead round-trip's cse gate is now located in the RTL rather than inferred: .jump insn 23 (set (reg 74) (plus (reg 74) 3)) and insn 26 (set (reg 74) (plus (reg 74) -3)) make cse_insn invalidate reg 74, which bumps reg_tick at cse.c:1539 and un-validates the (mem:QI (reg 74)) entry insn 20 recorded; insn 35's load therefore survives .cse verbatim. Both round-trip insns carry REG_EQUAL notes (D_80106A73 + 3, then D_80106A73), so cse folds the arithmetic away and the construct costs zero instructions.

- [s28] Re-audited kill result: that round-trip chassis is 49 insns / score 10 with FIRST DIVERGENCE: RA and a MATCHING multiset -- the closest chassis this function has ever reached -- and the whole residual is the $v1/$a0 seat swap that s26's RA inverse already FORECLOSES on this exact chassis. The construct is dead arithmetic with no semantic purpose (fails cheat-checklist T1/T2/T6), so it is not a submission route.

- [s28] Bitfield direction under -mel: FIRST-declared field takes the LOW bits (b1 andi 0x1f vs b1b andi 0xf8). .claude/rules/bitfield-direction-divergence.md (2026-06-11) predates the 2026-08-04 -mel adoption and its flipped-field-order advice is stale; this is a cross-function toolchain fact, recorded but not acted on.


==== s29 (rederive) ====

CHASSIS RE-MEASURE. `memory/grind/func_80034F88/candidate.c` re-installed at
`src/code6cac_b.c:3420` and measured on HEAD this session: **score 10,
target_insns 49, build_insns 49, rules_dropped 0**. The dispatch brief's
CHASSIS CHECK printed "measurement unavailable"; the ledger's floor of 10 is
therefore re-confirmed by direct measurement, not inherited.

THE RESIDUAL, INSTRUCTION BY INSTRUCTION (first full objdump alignment banked
in the ledger). Blocks 2 and 3, the copy loop, the prologue and the epilogue are
byte-exact. All ten points sit in blocks 0-1:

    #   target                          candidate.c
    1   lui   $v1,%hi(D_80106A73)       lui   $a0,%hi(D_80106A73)
    2   addiu $v1,$v1,%lo              addiu $a0,$a0,%lo
    3   lbu   $a0,0($v1)                lbu   $v1,0($a0)
    4   sb    $a0,0($v1)                sb    $v1,0($a0)
    5   lbu   $a0,0($v1)                nop
    6   ori   $v0,$a0,0x1               ori   $v0,$v1,0x1
    7   addu  $v0,$a0,$zero             addu  $v0,$v1,$zero
    8   [L] lui   $a0,%hi               [L] sb    $v0,0($a0)
    9       addiu $a0,$a0,%lo               lui   $a0,%hi
    10      sb    $v0,0($v1)                addiu $a0,$a0,%lo

MANDATED KILL RE-AUDIT (floor flat >= 3 sessions). The instance-killed form that
sits closest to the target is the s20/s25 dead round-trip
(`rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c`, the only
banked body whose instruction MULTISET matches the target). Re-measured on the
current HEAD chassis this session: **score 10 at 49 insns** -- the kill holds.
candidate.c carries no /* FAKE */-annotated construct (`fake_ablate.py` had
nothing to strip in s27), so the (b) ablation arm is the s27 hand ablation,
already banked at 29.

NEW AND SHARPER: the round-trip body's objdump was aligned instruction by
instruction for the first time. With block 1's reload RESTORED (its `lbu
$v1,0($a0)` sits exactly where the target's `lbu $a0,0($v1)` sits) the score is
STILL 10, and the ten differing positions are rows 1-4 and 6-10 of the table
above with row 5 now a register-naming difference instead of a missing
instruction. **Restoring block 1's byte reload is worth ZERO score points.**
That corrects the s25/s28 pricing (which carried the reload at ~2 of the 10):
the ENTIRE residual is the blocks-0/1 seat convention (7 instructions) plus the
3-instruction rotation at the block-1 join, and both are consequences of one and
the same missing fact -- a SECOND address allocno covering blocks 0-1. Future
probes must target the allocno, not the reload.

CROSS-FUNCTION EVIDENCE -- func_80035280, never examined in 28 sessions.
`asm/funcs/func_80035280.s` is this function's INVERSE (same TU, same
`func_80077D00()` handle, same `p[8]` flag word, same D_80106A70[0..2] triple,
copied the other way). It is direct evidence about the original source's object
model for this byte:

  * 0x80035294/98  `lui $a1,%hi(D_80106A73)` / `addiu $a1,$a1,%lo` -- ONE named
    pointer local holding &D_80106A73, materialised once and live across the
    whole flag section (three `lbu 0($a1)` reads at 0x800352AC/C8/DC, no
    re-materialisation).
  * 0x8003529C  `addiu $a2,$a1,-0x3` -- the pointer to D_80106A70 is DERIVED
    FROM IT by pointer arithmetic (`r = q - 3`), and that derived pointer is
    what its copy loop walks (`lbu 0($a2)` / `addiu $a2,$a2,1`).

So this code family's idiom for 0x80106A73 is a named `u8 *` local, exactly the
object model candidate.c uses -- independent corroboration (alongside sibling
func_80034708's single long-lived `s5`) that the plain-symbol family is not what
the original wrote. It also explains why 35280 keeps every reload with ONE
pointer where F88 cannot: 35280 writes `sw $v0,0x20($t0)` BETWEEN its byte
reads, and that store fires cse.c:7599 `invalidate_memory`; F88's target stream
between its block-0 `sb` and its block-1 `lbu` is `sb`, `lw`, `lbu` -- no store,
so no invalidation (the s27/s28 enumeration stands, and 35280 is the positive
control for it).

Finally, 35280 sharpens what the target's THREE materialisations mean for F88's
original source. F88's pairs partition the four byte-access groups as
{mask + bit1} / {bit2} / {bit4} -- pair #1 serves the mask AND the bit-1 block,
pairs #2 and #3 serve one block each. That is precisely the partition a source
with three separately-scoped pointer locals produces, and it is the partition of
the three-object body that measured score 0 in s13-s16. It is NOT reachable from
one C object: one C object is one pseudo (global.c:426), one allocno, one hard
register, and the seat is then forced to $a0 by blocks 2-3 (which are exact).

REDERIVE PROBES (all four banked in rejected/):
  * v1 `u8 *r = q - 3;` live from the top, copy loop spelled `r[i] = ...` (the
    35280 idiom transplanted onto this chassis): **score 19 at 50 insns**. The
    second address allocno is real -- `r` is seated in $a2 and the loop drops its
    `lui $at,%hi` re-materialisation -- but blocks 0-1 come out
    BYTE-FOR-BYTE IDENTICAL to the base chassis (base $a0 / value $v1). An extra
    live address allocno that does NOT alias the flag byte does not perturb the
    contested seat at all.
  * v2 the same derivation placed just before the loop: **30 at 48 insns**.
  * v3 the block-2 re-materialisation moved INTO block 1, between the value
    select and the store (the position the target emits it at): **21 at 51
    insns** -- crossing the branch join costs a fourth lui/addiu pair.
  * v4 the same move applied to blocks 1 and 2 both: **22 at 49 insns**; the
    join order is still ours, not the target's, consistent with the s26b
    REG_DEP_ANTI finding (sched.c:1738).

- [s29] Chassis re-measured this session (the brief's CHASSIS CHECK printed 'measurement unavailable'): candidate.c at src/code6cac_b.c:3420 = score 10, target_insns 49, build_insns 49, rules_dropped 0.

- [s29] First full instruction alignment banked: blocks 2 and 3, the copy loop, the prologue and the epilogue are byte-exact; all ten points are in blocks 0-1 (target base $v1 / value $a0, ours base $a0 / value $v1, plus the lui/addiu-vs-sb rotation at the block-1 join).

- [s29] The dead round-trip chassis (matching instruction multiset) re-measures 10 at 49 on HEAD, and with the reload restored the score does not move -- the reload is score-neutral and the residual is entirely allocno-driven.

- [s29] func_80035280 (0x80035294-0x8003529C) shows the original source's idiom for 0x80106A73: one named u8* local, with the D_80106A70 pointer derived as `q - 3`.

- [s29] Transplanting that idiom onto this chassis (u8 *r = q - 3 for the copy loop) scores 19 at 50 insns and leaves blocks 0-1 byte-for-byte unchanged: a non-aliasing address allocno does not perturb the contested seat.

- [s29] Moving an address re-assignment across a branch join costs a fourth lui/addiu pair (51 insns, score 21), re-confirming the s27 cost law from a new direction.


==== s30 (rederive) ====

CHASSIS RE-MEASURE (dispatch printed "measurement unavailable"). candidate.c
re-installed at src/code6cac_b.c:3420 and measured on HEAD this session:
**score 10, target_insns 49, build_insns 49, rules_dropped 0**. The floor of 10
is confirmed by direct measurement for the second consecutive session.

--- 1. THE LAST UNSPENT SIBLING: CD_sync's F1 combine-foldable chain-extender ---

The dispatch brief listed CD_sync (src/system.c, foreclosed, floor 2, 116
sessions) and CD_datasync as siblings never spent on this ledger. CD_sync's
candidate.c carries the one construct this function has never seen: the
FAKE-annotated F1 combine-foldable chain-extender (sanctioned by owner ruling
2026-07-01), spelled there as

    idx_1495 = (u8 *)((u8 *)tbl_125c
                      + ((s32)&D_800A1494 - (s32)D_800A125C) + 1);

whose header claims it "folds to &D_800A1494 + 1 with ZERO emitted bytes" while
"flow.c records the extra reg_n_refs before combine.c folds the SYMBOL_REF
difference". That is exactly the shape of lever this function's frontier wants
(a perturbation of the address quantity that costs no instruction), and it is
NOT the standing multi-handle ban: it is assigned to the SINGLE existing object
`q`, declaring no second pointer object.

Transplanted onto this chassis as

    q = (u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70));

in three placements -- all three assignments (a_chainext_all), the first only
(a2_chainext_first), and blocks 2+3 only (a3_chainext_b2b3). ALL THREE measure
**score 14 at 49 build insns**.

MECHANISM, read off the dumps rather than inferred (pwsh tools/grinder/dump.ps1
func_80034F88 on the a2 body, dumps under tmp/grind/func_80034F88/dumps/):

  * .combine holds THREE `(set (reg) (symbol_ref:SI ("D_80106A73")))` insns --
    the SYMBOL_REF difference is folded away completely and the chain-extender
    leaves no arithmetic behind. The two later sets carry
    `(expr_list:REG_EQUAL (symbol_ref:SI ("D_80106A73")))`, the first does not.
  * .greg assigns ALL THREE of those sets to `(reg/v:SI 4 a0)` -- one hard reg,
    one allocno, identical to the base body. The chain-extender creates NO
    second address allocno. `global.c:426` (one C object = one pseudo = one
    allocno) is untouched by it.
  * The emitted function bodies are BIT-IDENTICAL. `diff base.s a2.s`
    (tmp/grind/func_80034F88/s30/{base,a2}.s) is FOUR lines and all four are the
    frame:  `subu $sp,$sp,24` -> `subu $sp,$sp,32`, `sw $31,16($sp)` ->
    `sw $31,24($sp)`, and the matching epilogue pair. Every instruction between
    the prologue and the epilogue is unchanged.

So the +4 points are ENTIRELY a phantom frame slot: the folded-away intermediate
still reserves 8 bytes of locals frame (the known GCC 2.7.2 behaviour recorded in
the auto-memory `project/phantom-frame-slots-gcc272`). The lever's reg_n_refs
perturbation, which is load-bearing in CD_sync, is INERT here -- it does not
reach allocation at all on this chassis, because the address pseudo's seat is
already decided by the blocks-2/3 uses that are byte-exact.

CONCLUSION: the F1 chain-extender cannot move this residual. It is byte-neutral
in the body by construction (it folds pre-RA), and the residual is a body
defect, not a priority defect. This closes the sibling-inheritance axis
completely: func_80034708 (s25), func_80035280 (s29), CD_sync/CD_datasync (s30).

--- 2. do-while(0) JOINT WRAPS (the un-banked placements) ---

The bank held do-while(0) around block 0 (15), around block 1 (10, reload lost),
per-block (26) and at the block boundary (23), but never a JOINT wrap of
adjacent blocks. Measured:

    mask-stmt + block 1 wrapped together   score 12, 50 insns
    block 2 + block 3 wrapped together     score 10, 49 insns (INERT)
    all three blocks wrapped together      score 12, 50 insns

Any wrap that spans the mask store adds an instruction (50 vs the target's 49);
the blocks-2/3 wrap is a no-op on a region that is already byte-exact. The
sanctioned do-while(0) family is now exhausted in every placement on this
chassis.

--- 3. FRESH m2c RE-DECOMPILE (the mandated rederive tool, re-run) ---

`python3 tools/m2c/m2c.py --target mipsel-gcc-c --valid-syntax -f func_80034F88`
run fresh this session. m2c's shape is the INVERTED diamond -- the positive
value is computed first and overwritten in the negated arm:

    var_v0 = D_80106A73 | 1;
    if (!(p[8] & 1)) { var_v0 = D_80106A73; }
    D_80106A73 = var_v0;

The bank holds that shape only on the PLAIN-SYMBOL chassis (m2c-verbatim 29,
posif-copy-first 23, m2c-tworead-condfirst 20, m2c-value-shape-s32-temps 21).
It had never been crossed with the pointer chassis. Measured on the `q` chassis
with the three re-assignments intact, with an s32 temp and with m2c's own u8
temp: **both score 21 at 45 build insns**. The diamond COLLAPSES -- four
instructions short of the target's 49 -- because reading `*q` in both the
computation and the negated arm lets cse fold the whole select into a single
arm. The m2c shape is dead on this chassis in both spellings.

--- 4. WHAT s30 DOES NOT CHANGE ---

candidate.c is unchanged: no form measured this session reaches 10, and the two
that tie it (the blocks-2/3 do-while wrap; the s25/s20 round-trip) emit the same
stream. The residual is still exactly what s29 priced: one missing address
allocno, expressed as the blocks-0/1 seat convention plus the 3-insn lui/addiu/sb
rotation at the block-1 join.

- [s30] Chassis re-measured on HEAD this session: candidate.c = score 10, target_insns 49, build_insns 49, rules_dropped 0. The dispatch brief's CHASSIS CHECK was 'measurement unavailable'; the floor of 10 is now measured, not inherited.

- [s30] The CD_sync F1 chain-extender folds COMPLETELY before RA on this chassis: .combine holds three plain (set (reg) (symbol_ref "D_80106A73")) insns (the two later ones carrying REG_EQUAL symbol_ref notes), and .greg assigns all three to (reg/v:SI 4 a0). One pseudo, one allocno, identical to the base body.

- [s30] diff tmp/grind/func_80034F88/s30/base.s tmp/grind/func_80034F88/s30/a2.s is exactly four lines, all of them frame: subu $sp,$sp,24 -> subu $sp,$sp,32, sw $31,16($sp) -> sw $31,24($sp), and the two matching epilogue lines. Every instruction between prologue and epilogue is unchanged, so the F1 chain-extender is byte-neutral IN THE BODY and its entire 4-point cost is a phantom frame slot for the folded intermediate.

- [s30] Sibling inheritance is now fully spent for this function: func_80034708 (s25), func_80035280 (s29), and CD_sync / CD_datasync (s30, the F1 chain-extender). No sibling ledger holds a construct that has not been transplanted and measured here.

- [s30] do-while(0) is exhausted in every placement on this chassis: block 0 = 15, block 1 = 10 (reload lost), per-block = 26, block boundary = 23, mask+block1 joint = 12 at 50 insns, block2+block3 joint = 10 at 49 (inert), all-three joint = 12 at 50. Any wrap spanning the mask store costs one instruction against the target's 49.

- [s30] m2c's inverted diamond on the pointer chassis (measured for the first time, both s32 and u8 temps) is 21 at 45 build insns: cse forwards the duplicated *q read and the select collapses, so the shape LOSES four instructions instead of restoring block 1's lbu.

- [s30] Seven new forms banked under memory/grind/func_80034F88/rejected/ (s30-*), bringing the disproven bank to 166. candidate.c is unchanged as the best admissible form at floor 10, with its header updated with the s30 note (10).

- [s30] The residual is unchanged from s29's pricing: one missing address allocno, expressed as the blocks-0/1 seat convention plus the 3-insn lui/addiu/sb rotation at the block-1 join. src/code6cac_b.c is back at HEAD (INCLUDE_ASM) and the tree carries no src dirt.


==== s31 (structural) ====

CHASSIS RE-MEASURE (dispatch printed "measurement unavailable" again).
candidate.c re-installed at src/code6cac_b.c:3420 and measured on HEAD this
session: **score 10, target_insns 49, build_insns 49, rules_dropped 0**. Third
consecutive session in which the floor of 10 is confirmed by direct measurement.

MANDATED KILL RE-AUDIT. (a) The closest banked form to the target -- the s20/s25
dead round-trip `q = q + 3; q = q - 3;`
(rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c), the only
body whose instruction multiset matches the target's -- re-measures **10 at 49
insns** on today's chassis, unchanged. (b) `python3 tools/fake_ablate.py --func
func_80034F88 --file code6cac_b --candidate memory/grind/func_80034F88/
candidate.c` reports "no FAKE-annotated constructs found ... nothing to
ablate": the floor-10 body is FAKE-free, so no banked kill on this chassis was
measured with a FAKE carrier occupying the contested pseudo.

--- 1. THE STRUCTURAL LEVER SET, MEASURED WITH DUMPS (not inferred) ---

Eleven bodies measured (tmp/grind/func_80034F88/s31/variants/, scores from
`sandbox --disable all`):

    base (candidate.c)                                  10   49
    v1  mask value reused as block 1's value            10   49   asm IDENTICAL
    v2  `v = *q` read before `c = p[8] & K` in all 3    10   49   asm IDENTICAL
    v3  p narrowed to `u8 *`, flag word via *(s32*)     12   49
    v4  init-then-conditional-or (empty else arm)       30   44
    v5  named intermediate for the mask statement       10   49   asm IDENTICAL
    v6  flag word split into its own local (block 1)    10   49   asm IDENTICAL
    v7  v5 + v6 together                                10   49   asm IDENTICAL
    v8  ONE function-scope v/c pair shared by 3 blocks  10   49   asm IDENTICAL
    v9  v8 with the classic `*q &= 0xF8;` mask          10   49
    v10 value carried between blocks (`v = c;`)         33   49
    v11 block 1's flag-word read hoisted above the mask 14   49

"asm IDENTICAL" is not a score tie: `diff base.s vN.s` on the cc1 output
(tmp/grind/func_80034F88/s31/*.s, produced with `pwsh tools/grinder/dump.ps1
func_80034F88` per variant) is EMPTY. Every block-local split, declaration-order
and statement-re-association form in the modality's catalogue that stays at 49
insns emits the SAME instruction stream, byte for byte.

The .greg dumps say why, and this is the new datum: the extra C objects do not
survive to allocation. base, v5 and v7 all print the identical allocator input:

    ;; 9 regs to allocate: 73 78 82 86 74 77 81 85 72
    ;; 74 conflicts: 72 74 77 78 81 82 85 86 2 3 29
    ;; 77 preferences: 3      ;; 78 preferences: 2   (82, 86 likewise 2)
    ;; Register dispositions: ... 74 in 4 ...

-- same allocno list, same conflict graph, same preferences, same seats. Adding
`s32 m;` for the mask value or `s32 f;` for the flag word adds NO allocno.

v8 is the strongest form of the experiment: collapsing the three block-scoped
`v`/`c` pairs into ONE function-scope pair takes the allocator input from NINE
global allocnos to FIVE (`;; 5 regs to allocate: 73 76 75 74 72`) -- a different
conflict graph, a different allocation order, a different priority ranking -- and
the emitted function is still BIT-IDENTICAL to base, with `74 in 4` ($a0) again.
The address object's seat is invariant under the entire structural axis measured
here.

--- 2. WHY $v1 IS UNREACHABLE FOR THE ADDRESS OBJECT, IN THE ALLOCATOR'S OWN
       VOCABULARY (new to this ledger) ---

Every previous session states the residual as "one missing address allocno".
The .greg header states it one level lower, as a HARD-REG CONFLICT:

    ;; 74 conflicts: ... 2 3 29

Allocno 74 (the `&D_80106A73` object) conflicts with hard registers $v0 (2) and
$v1 (3) -- and 29 ($sp), which every allocno does. It is therefore not merely
outranked for $v1; it is INELIGIBLE for it before find_reg ever runs. The holder
is a LOCAL-ALLOC pseudo: dispositions show `75 in 3` / `77 in 3` for pseudos
outside s23's global allocno set {72,73,74,77,78,81,82,85,86} -- the block-0
mask read/`and` value (QImode; s24's local-alloc table row
`blk=0 qty=0 reg1=76 size=1 mode=1`). It takes $v1 because $v0 is unavailable:
the RTL shows the call's return copy `(set (reg/v:SI 5 a1) (reg:SI 2 v0))`
(insn 11) SCHEDULED AFTER the whole mask sequence (insns 14/17/18/20), so $v0
is live across block 0. Local-alloc runs before global-alloc, so it wins the
seat unconditionally.

v11 is the direct test of that chain: hoisting block 1's `p[8]` read above the
mask statement gives `p` a use before block 0 and does free $v0 for the block-0
value (`75 in 2` now, `76 preferences: 3`). It does NOT free $v1 -- another
local-alloc pseudo (`77 in 3`) takes it -- `74 conflicts: ... 2 3 29` is
unchanged, 74 is still in $a0, and the body costs 4 points (14 at 49 insns).

An s23 CLAIM IS CORRECTED HERE, with a file:line. s23 wrote: "`$v1` and `$a0`
never appear as hard registers in this function's pre-RA RTL, so
`global.c set_preference` can never record a preference for either, for any
pseudo. The preference lever ... is structurally unavailable HERE." The dumps
falsify the conclusion: pseudo 77 carries `preferences: 3` ($v1) and pseudos
78/82/86 carry `preferences: 2` ($v0), in EVERY variant measured this session.
The mechanism is `tools/gcc-2.7.2/global.c:1709-1713`, inside set_preference:

    if (reg_renumber[src_regno] >= 0)  src_regno  = reg_renumber[src_regno];
    if (reg_renumber[dest_regno] >= 0) dest_regno = reg_renumber[dest_regno];

-- a copy between a global allocno and a pseudo that LOCAL-ALLOC already seated
is converted to a hard-reg preference. Preferences for $v0/$v1 therefore exist
in this function without any call-argument setup. What is true, and is the
sharper statement, is that allocno 74 carries NO preference line in any of the
eleven bodies: it is never the operand of a copy at all (it is set from a bare
`(symbol_ref "D_80106A73")`), so no preference can be recorded for it. Supplying
one requires a copy from an object local-alloc has already seated in $v1 -- i.e.
a second C object aliasing the byte, the standing banned construct -- and even
then the hard-reg conflict on 3 would make find_reg reject it.

--- 3. WHAT THIS SESSION DOES NOT CHANGE ---

candidate.c is unchanged: nothing measured this session beats 10, and the seven
forms that tie it emit base's exact stream. The structural modality is now
banked with dumps rather than with scores: block-local splits, declaration
order/scope, statement re-association, value-object sharing and type narrowing
are each measured, and the four that change the emitted code (v3 12, v4 30,
v10 33, v11 14) all move away from the target.

LADDER STATUS after s31: cycle 2 of the modality ladder (owner directive
2026-09-02: 20 flat sessions and at least 6 distinct modalities before any
disposition) stands at TEN sessions and SIX distinct modalities -- escalation
(s23, s24), synthesis (s25), solver (s26), forensics (s27, s28), rederive (s29,
s30), structural (s31). The modality count condition is now met; the session
count is not.

- [s31] Chassis re-measured on HEAD this session: candidate.c = score 10, 49 target insns / 49 build insns, rules_dropped 0. Third consecutive session confirming the floor by direct measurement.

- [s31] Eleven structural bodies measured: base 10/49; v1 10/49; v2 10/49; v3 12/49; v4 30/44; v5 10/49; v6 10/49; v7 10/49; v8 10/49; v9 10/49; v10 33/49; v11 14/49.

- [s31] Seven of the ties are not merely score-equal: `diff base.s vN.s` on the cc1 output is EMPTY for v1, v2, v5, v6, v7 and v8 -- the structural rewrites emit base's instruction stream byte for byte.

- [s31] base/v5/v7 .greg print the identical allocator input: `;; 9 regs to allocate: 73 78 82 86 74 77 81 85 72`, `;; 74 conflicts: 72 74 77 78 81 82 85 86 2 3 29`, `;; 77 preferences: 3`, dispositions `74 in 4`. Adding a named mask intermediate or a flag-word local creates NO allocno.

- [s31] v8 (one function-scope v/c pair) reduces the global allocno set from nine to five (`;; 5 regs to allocate: 73 76 75 74 72`) with a different conflict graph and allocation order, and still emits a bit-identical function with the address object in $a0.

- [s31] The address allocno's exclusion from $v1 is a HARD-REG conflict (`74 conflicts: ... 2 3 29`), created by a local-alloc-seated pseudo holding $v1 across q's live range; local-alloc runs before global-alloc, so it wins the seat unconditionally.

- [s31] v11 tests that chain directly: hoisting block 1's p[8] read above the mask frees $v0 for the block-0 value (`75 in 2`) but not $v1 (`77 in 3`); 74's hard-reg conflicts are unchanged and the body costs 4 points.

- [s31] s23's statement that '$v1 and $a0 never appear as hard registers in this function's pre-RA RTL, so set_preference can never record a preference for either, for any pseudo' is corrected: preferences for hard regs 2 and 3 are printed for pseudos 77/78/82/86 in every body, via global.c:1709-1713 (reg_renumber mapping). Allocno 74 alone carries none, because it is never a copy operand.

- [s31] fake_ablate on candidate.c reports no FAKE-annotated constructs, so no banked kill on this chassis was measured with a FAKE carrier on the contested pseudo.

- [s31] Ladder cycle 2 (owner directive 2026-09-02) now stands at TEN sessions and SIX distinct modalities -- escalation (s23, s24), synthesis (s25), solver (s26), forensics (s27, s28), rederive (s29, s30), structural (s31). The >= 6 modality condition is met; the 20-session condition is not.

==== s32 (structural) ====

The residual's mechanism is now stated with BOTH of its barriers measured, and
the frontier's one-barrier statement from s31 is corrected.

CHASSIS. candidate.c installed at src/code6cac_b.c:3420 measures 10 at 49 build
insns / 49 target insns, rules_dropped 0, cheat_asm_stripped 27. fake_ablate
reports no FAKE construct in the body. Two banked forms re-measured on this
chassis reproduce their banked scores exactly (mask-on-symbol 16/51,
pointer-live-blocks23-only 14/49), so every s31 instance kill's basis holds.

THE TWO BARRIERS. The 10 points are all in block 0 + block 1: the target puts
the address in $v1 and the byte value in $a0, we put the address in $a0 and the
value in $v1, and where the target reloads the byte (`lbu $a0, 0($v1)`) we emit
the load-delay nop of `lw $v0, 0x20($a1)`. The address is pseudo 74 in the .greg
dumps.

  (1) HARD-REG CONFLICT. On every body where `q` is live across block 0, .greg
      prints `74 conflicts: ... 2 3 29`: 74 is ineligible for $v0 and $v1 before
      find_reg runs. tools/gcc-2.7.2/config/mips/mips.h defines no
      REG_ALLOC_ORDER, so local-alloc (which runs first) hands its block-0
      quantities out in raw register number order -- $v0, then $v1 -- and every
      such quantity overlapping 74's range becomes a hard-reg conflict for it.
      Block 0 always holds at least TWO of them (the mask's QI load temp and SI
      and-result, plus the flag-word temp). This is invariant under respelling:
      base seats 75,76 in $v1 and 79 in $v0; the mask-on-symbol body seats
      75,76 in $v0 and 79 in $v1; the q-materialised-late body seats 76 in $v1
      and 75 in $v0. All three print the same conflict set and all three print
      `74 in 4` ($a0).

  (2) ALLOCNO PRIORITY. Shortening 74's live range to blocks 2/3 only
      (pointer-live-blocks23-only) DOES delete barrier (1): 74 becomes 6 refs
      across 17 insns and its conflict set drops to `72 74 78 81 82 85 86 2 29`
      -- hard reg 3 is gone. The seat still does not move. global.c:635-655
      orders allocnos by floor_log2(n_refs)*n_refs/live_length; the printed
      order is `73 82 86 78 77 81 85 74 72`, so 74 is allocated EIGHTH, after
      78 (0.91), 81 and 85 (0.75) -- all three conflict with 74 and all three
      take $v1 -- against 74's own 0.71.

  The two barriers trade against each other: the only structural lever that
  removes the hard-reg conflict (shortening the address's live range) is
  exactly the lever that lowers its priority below the allocnos that then take
  $v1. On the 17-insn range the threshold is floor_log2(n)*n > 15.45, i.e. 74
  would need EIGHT refs instead of six to be ordered ahead of 78, and each
  extra ref through `q` is an extra memory access on a body already at the
  target's 49 instructions.

  Note also that priority is NOT a barrier on the base body: `73 conflicts:
  72 73 2 29` does not list 74, so the $v1-holding allocno 73 and the address 74
  do not conflict at all there, and 74 (10345) already outranks 77/81/85 (7500).
  s31's reading of the base body was right; what it did not measure is that the
  conflict is removable and that removing it merely swaps in barrier (2).

WHAT THIS SAYS ABOUT THE TARGET. In the target, block 0's mask temp is in $a0,
which under raw allocation order means BOTH $v0 (the live call return, still
live at the `lbu` on line 8 -- `addu $a1,$v0,$zero` is line 9) and $v1 were
unavailable when local-alloc reached it. $v1 being unavailable at that moment
means the address was already seated there, i.e. the target's block-0/1 address
was resolved before local-alloc's block-0 quantities -- which no single-pointer
body reproduces, because a single `q` spanning blocks 0-3 is necessarily a
global allocno and global-alloc runs second.

ARTIFACTS. tmp/grind/func_80034F88/s32/{greg_base,greg_v1,greg_v2,greg_v3,
lreg_base}.txt, base.txt (objdump of the floor body), base.c, v1.c, v2.c, v3.c,
install.py, ext.py. New rejected form:
rejected/s32-q-materialised-after-flagread-score14.c.

- [s32] Chassis re-established: candidate.c = score 10, 49 build insns / 49 target insns, rules_dropped 0, cheat_asm_stripped 27; fake_ablate finds no FAKE construct in the floor body.

- [s32] objdump of the floor body (tmp/grind/func_80034F88/s32/base.txt) against asm/funcs/func_80034F88.s localises all 10 points to block 0 + block 1: target holds the address in $v1 and the byte value in $a0, we hold the address in $a0 and the value in $v1, and target's `lbu $a0, 0($v1)` sits where we emit the load-delay nop of `lw $v0, 0x20($a1)`. Blocks 2, 3 and the trailing copy loop are register-exact.

- [s32] tools/gcc-2.7.2/config/mips/mips.h defines no REG_ALLOC_ORDER, so local-alloc allocates in raw register number order: $v0 first, $v1 second. That is why block-0 local quantities land on $v1 and become a hard-reg conflict for the address allocno.

- [s32] global.c:635-655 (allocno_compare) prioritises by floor_log2(n_refs)*n_refs/live_length*10000*size. Measured priorities on the base body: address allocno 74 = 1.03, the $v1-holding allocno 73 = 4.71 -- but 73 does NOT appear in 74's conflict list, so priority is not the barrier there; the hard-reg conflict on register 3 is.

- [s32] On the pointer-live-blocks23-only body the hard-reg conflict on register 3 disappears from `74 conflicts` entirely (`72 74 78 81 82 85 86 2 29`), the first measured body in 32 sessions where it does, and the seat still does not move -- the printed allocation order `73 82 86 78 77 81 85 74 72` puts 74 eighth of nine.

- [s32] The two barriers trade against each other on this function: the live-range shortening that deletes the hard-reg conflict is the same change that lowers the address allocno's priority below allocnos 78/81/85, which conflict with it and take $v1.

- [s32] In the target, block 0's mask temp sits in $a0 while the call return is still live in $v0 (`addu $a1,$v0,$zero` follows the first lbu), which under raw allocation order means $v1 was already occupied when local-alloc reached that quantity -- i.e. the target's block-0/1 address was seated before local-alloc's block-0 quantities, which a single q spanning blocks 0-3 cannot be, since it is necessarily a global allocno and global-alloc runs second.

==== s33 (synthesis) ====

CHASSIS RE-MEASURE. `memory/grind/func_80034F88/candidate.c` installed at
`src/code6cac_b.c:3420` and measured on HEAD this session: **score 10,
target_insns 49, build_insns 49, rules_dropped 0, cheat_asm_stripped 27**. The
dispatch brief printed "measurement unavailable" for the fifth consecutive
session; the floor of 10 is measured, not inherited. `src/` restored to HEAD at
session end (`git status --porcelain src/` empty).

MANDATED KILL RE-AUDIT. (a) The banked form closest to the target -- the s20/s25
dead round-trip `q = q + 3; q = q - 3;`
(`rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c`, the only
body whose instruction multiset matches the target's) -- re-measures **10 at 49
insns** on today's chassis. The kill holds for the fifth consecutive session.
(b) `python3 tools/fake_ablate.py --func func_80034F88 --file code6cac_b
--candidate memory/grind/func_80034F88/candidate.c` again reports "no
FAKE-annotated constructs found ... nothing to ablate", so no banked kill on
this chassis was measured with a FAKE carrier occupying the contested pseudo.

--- 1. THE FIRST .greg OF THE SCORE-0 BANNED BODY, AND WHAT IT ACTUALLY SAYS ---

Thirty-two sessions have described the score-0 three-object body
(`rejected/three-pointer-objects-judge-FAIL-score0.c`, Judge FAIL 2026-08-13) as
"three allocnos beat one". Nobody had ever dumped its allocator input. Installed
and re-measured this session: **score 0 at 49/49 on HEAD** (the banned body still
matches). `pwsh tools/grinder/dump.ps1 func_80034F88` on it, sliced to
`tmp/grind/func_80034F88/s33/greg_t3.txt` / `lreg_t3.txt`:

  * The three surviving `(set (reg) (symbol_ref "D_80106A73"))` sets go to
    pseudos **80, 82, 82**. The FOURTH source-level assignment (`q1`) does NOT
    survive as a symbol_ref set -- cse rewrote it into a register COPY from 80.
  * Pseudo **80 is NOT in the allocno list** (`;; 10 regs to allocate: 73 78 79
    77 74 75 76 82 72 81`). It is a LOCAL-ALLOC quantity: born and dead inside
    block 0's basic block. Local-alloc seats it in **$v1** -- `$v0` is taken by
    the live `func_80077D00` return and `tools/gcc-2.7.2/config/mips/mips.h`
    defines no `REG_ALLOC_ORDER`, so allocation is raw register order.
  * Pseudo **81** (`q1`, the blocks-0/1 handle) is a global allocno carrying
    `;; 81 preferences: 3` and `;; 81 conflicts: 72 74 77 81 29` -- **no hard-reg
    conflict on register 3**. It is allocated LAST of the ten and still takes
    $v1, because the preference exists and nothing conflicting holds $v1.

So the score-0 body's mechanism is not "more allocnos". It is exactly two facts:
**(i) a block-0-confined LOCAL-ALLOC pseudo seats $v1; (ii) the blocks-0/1
address allocno is the DESTINATION of a cse-created copy from it, which
`tools/gcc-2.7.2/global.c:1709-1713` (`set_preference`, reg_renumber mapping)
turns into a hard-reg preference for $v1 and which carries no hard-reg-3
conflict.** s31 named that preference mechanism from the base body's dumps; this
is the first time it has been seen doing the actual work.

--- 2. THE MECHANISM REPRODUCED WITH **ONE** DECLARED POINTER OBJECT (c1) ---

Fact (i) needs a pseudo, not a C object. An address expression that is not
`CONSTANT_ADDRESS_P` at expand time is forced into a compiler temp; cse folds it
afterwards, so the temp costs nothing. Spelled as an anonymous symbol difference
in the mask statement, with `q` first assigned in block 1:

    *(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70)) &= 0xF8;

**c1 measures score 13 at 49 build insns** and its `.greg`
(`tmp/grind/func_80034F88/s33/greg_c1.txt`) is the result this session exists
for:

    base   ;; 74 conflicts: 72 74 77 78 81 82 85 86 2 3 29     74 in 4   ($a0)
    c1     ;; 74 conflicts: 72 74 79 80 83 84 87 88 2 29
           ;; 74 preferences: 3                                74 in 3   ($v1)

The hard-reg conflict on register 3 that s31/s32 measured on **every** body with
`q` live across block 0 is GONE, and the single declared pointer object is seated
in **$v1** -- the target's seat, reached for the first time in 33 sessions
without a second named handle. Pseudo 75 (the anonymous temp) is local-alloc'd to
$v1 exactly as pseudo 80 is in the banned body. The prologue stays
`addiu sp,sp,-24`, so unlike s30's chain-extender there is no phantom frame slot.

**Blocks 0 and 1 are now BYTE-EXACT with the target.** Disassembly
`tmp/grind/func_80034F88/s33/c1.txt` against `asm/funcs/func_80034F88.s`:

    lui v1 / addiu v1 / lbu a0,0(v1) / move a1,v0 / andi a0,0xf8 / sb a0,0(v1)
    lw v0,0x20(a1) / lbu a0,0(v1) / andi v0,1 / bnez / ori v0,a0,1 / move v0,a0
    [L] sb v0,0(v1)

-- identical to the target through 0x80034FD0, including the block-1 `lbu` that
has been missing since s1 and the `$v1` base / `$a0` value convention.

--- 3. WHY c1 IS 13 AND NOT 0 -- THE RESIDUAL MOVED, IT DID NOT CLOSE ---

`q` is one C object, hence one pseudo, hence one allocno, hence ONE hard register
(`tools/gcc-2.7.2/global.c:426`). Its $v1 preference therefore also drags blocks
2 and 3 -- which are register-exact on candidate.c -- onto $v1, where the target
uses $a0. All 13 points are now in blocks 2/3, and the 3-instruction
lui/addiu-vs-sb rotation at the block-1 join is a CONSEQUENCE of that, not a
separate defect: with block 2 sharing $v1, its address materialisation cannot be
hoisted above block 1's store.

Four follow-ups pin the boundaries of the new family (all banked in `rejected/`):

    c2  block 0 AND block 1 anonymous, q for blocks 2/3      21   50 insns
    c3  q in block 1 only, blocks 0/2/3 anonymous            14   48 insns
    c4  block 0 anonymous, q in blocks 1+2, block 3 anon     13   49 insns
    c5  `q = &D_80106A73;` placed BEFORE the anonymous mask  10   49 insns

c5 is the control that isolates the lever: moving q's materialisation above the
anonymous expression makes the TEMP the copy destination instead of q, and the
body collapses back to the base chassis exactly. Declaration/materialisation
ORDER, not the expression, is what assigns the seat.

c2 is the one that closes the route. Block 1's store sits after the diamond's
join label, and a code label is a cse path boundary (s27 gate 5), so an anonymous
address re-materialises there (+1 insn, 50 vs the target's 49). **An anonymous
address temp cannot survive a code label; only a named C object can.** The
target needs a $v1 value spanning blocks 0-1 (two labels) and a separate $a0
value spanning blocks 2-3, i.e. TWO pseudos that each survive a cse path
boundary -- which is two named C objects, the standing ban.

--- 4. THE RESIDUAL, RESTATED FOR THE NEXT SESSION ---

The 10-point residual is no longer "one missing address allocno" as a single
lump. It factors into two independent seats, and s33 measured that they are
individually reachable but not simultaneously reachable from one object:

    blocks 0-1 address in $v1   -- REACHED (c1), one object, zero-instruction cost
    blocks 2-3 address in $a0   -- REACHED (candidate.c), one object
    both at once                -- needs two pseudos surviving two cse path
                                   boundaries = two named objects (banned)

- [s33] Chassis re-measured on HEAD: candidate.c = score 10, 49 target / 49 build insns, rules_dropped 0, cheat_asm_stripped 27. Fifth consecutive session confirming the floor by direct measurement; the dispatch brief again printed "measurement unavailable".

- [s33] KILL RE-AUDIT: the s20/s25 dead round-trip re-measures 10 at 49 insns on today's chassis (kill holds), and fake_ablate again finds no FAKE-annotated construct in candidate.c.

- [s33] The Judge-FAILed three-object body still measures score 0 at 49/49 on HEAD, and its .greg was dumped for the first time in 33 sessions (tmp/grind/func_80034F88/s33/greg_t3.txt).

- [s33] The score-0 body's mechanism is NOT "three allocnos beat one": its qm (pseudo 80) is absent from the allocno list, i.e. a block-0-confined LOCAL-ALLOC quantity that local-alloc seats in $v1 (raw register order, $v0 held by the live call return, no REG_ALLOC_ORDER in mips.h); cse rewrites q1's `= &D_80106A73` into a register copy from it; global.c:1709-1713 turns that copy into `;; 81 preferences: 3`; and 81 carries NO hard-reg conflict on register 3, so it takes $v1 even though it is allocated last of ten.

- [s33] That mechanism is reproducible with ONE declared pointer object. c1 addresses the mask through an anonymous symbol difference `*(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70))`, which is not CONSTANT_ADDRESS_P at expand and is forced into a compiler temp (pseudo 75) that dies inside block 0.

- [s33] c1's .greg is the first in 33 sessions to print `74 preferences: 3` and `74 in 3` for the address object, with the hard-reg-3 conflict absent (`74 conflicts: 72 74 79 80 83 84 87 88 2 29` vs base's `... 2 3 29`). The construct costs zero instructions and zero frame bytes (prologue stays addiu sp,sp,-24; contrast s30's chain-extender phantom slot).

- [s33] c1 makes blocks 0 and 1 BYTE-EXACT with the target through 0x80034FD0, including the block-1 lbu missing since s1 and the $v1-base/$a0-value convention. Score 13 at 49 build insns; all 13 differing instructions are in blocks 2/3.

- [s33] The seat swap MOVED rather than closed: q is one C object = one pseudo = one allocno = one hard register (global.c:426), so its $v1 preference also pulls blocks 2/3 off the target's $a0. The block-1-join lui/addiu-vs-sb rotation is a consequence of block 2 sharing $v1, not an independent defect.

- [s33] c5 is the isolating control: placing `q = &D_80106A73;` BEFORE the anonymous mask expression returns the body to exactly 10 at 49 -- the temp becomes the copy destination and q keeps $a0. Materialisation ORDER is the lever, not the expression.

- [s33] c2 closes the route: with block 1 also anonymous, its post-join store re-materialises the address (21 at 50 insns) because a code label is a cse path boundary (s27 gate 5). An anonymous address temp cannot survive a label; only a named C object can.

- [s33] Family boundary measured: c1 13/49, c2 21/50, c3 14/48, c4 13/49, c5 10/49. The admissible floor is unchanged at 10 and candidate.c is unchanged.

### Artifacts (s33)

`tmp/grind/func_80034F88/s33/`: `variants/{base,t3,c1,c2,c3,c4,c5,rt}.c`,
`greg_base.txt`, `greg_t3.txt`, `greg_c1.txt`, `lreg_base.txt`, `lreg_t3.txt`,
`lreg_c1.txt`, `c1.txt` (objdump), `install.py`, `ext.py`, `dis.sh`,
`code6cac_b.c.orig`. New rejected forms:
`rejected/s33-anon-symdiff-block0-blocks01-EXACT-score13.c`, `rejected/s33-c2.c`,
`rejected/s33-c3.c`, `rejected/s33-c4.c`, `rejected/s33-c5.c`.

- [s33] Chassis re-measured on HEAD (the dispatch brief printed 'measurement unavailable' for the fifth consecutive session): candidate.c at src/code6cac_b.c:3420 = score 10, target_insns 49, build_insns 49, rules_dropped 0, cheat_asm_stripped 27.

- [s33] The Judge-FAILed three-object body still measures score 0 at 49/49 on HEAD, and its allocator input was dumped for the first time in 33 sessions (tmp/grind/func_80034F88/s33/greg_t3.txt, lreg_t3.txt).

- [s33] In that body only THREE (set (reg) (symbol_ref "D_80106A73")) sets survive, to pseudos 80, 82, 82; the fourth source-level assignment was rewritten by cse into a register copy from pseudo 80 -- the zero-cost fourth quantity s27 predicted.

- [s33] Pseudo 80 is absent from ';; 10 regs to allocate: 73 78 79 77 74 75 76 82 72 81', i.e. it is a block-0-confined LOCAL-ALLOC quantity, and local-alloc seats it in $v1 because $v0 holds the live func_80077D00 return and tools/gcc-2.7.2/config/mips/mips.h defines no REG_ALLOC_ORDER.

- [s33] Allocno 81 (the blocks-0/1 handle) carries ';; 81 preferences: 3' via global.c:1709-1713 and ';; 81 conflicts: 72 74 77 81 29' with NO hard-reg 3; it is allocated last of ten and still takes $v1. The winning mechanism is the local-alloc seat plus the copy preference, not the allocno count.

- [s33] That mechanism is reproducible with ONE declared pointer object: c1 addresses the mask through the anonymous symbol difference *(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70)), which is not CONSTANT_ADDRESS_P at expand and is forced into compiler temp pseudo 75, dying inside block 0.

- [s33] c1's .greg is the first in 33 sessions to print '74 preferences: 3' and '74 in 3' for the declared address object, with the hard-reg-3 conflict gone ('74 conflicts: 72 74 79 80 83 84 87 88 2 29' vs base's '... 2 3 29').

- [s33] c1's blocks 0 and 1 are BYTE-EXACT with the target through 0x80034FD0 (tmp/grind/func_80034F88/s33/c1.txt vs asm/funcs/func_80034F88.s), including the block-1 lbu that has been missing since s1 and the $v1-base/$a0-value convention.

- [s33] The construct costs zero instructions and zero frame bytes -- c1's prologue stays addiu sp,sp,-24 -- unlike the s30 CD_sync F1 chain-extender, whose entire 4-point cost was a phantom frame slot.

- [s33] c1 scores 13 at 49 build insns because q is one C object = one pseudo = one allocno = one hard register (global.c:426): the $v1 preference also pulls blocks 2 and 3, register-exact on candidate.c, off the target's $a0. All 13 differing instructions are in blocks 2/3.

- [s33] The lui/addiu-vs-sb rotation at the block-1 join is a CONSEQUENCE of block 2 sharing $v1 (its address materialisation cannot be hoisted above block 1's store), not an independent defect -- correcting the s29/s32 framing that treated it as a separate 3-instruction gap.

- [s33] c5 isolates the lever: placing q = &D_80106A73; BEFORE the anonymous mask expression returns the body to exactly 10 at 49, because the temp becomes the copy destination and q keeps $a0. Materialisation ORDER, not the expression, assigns the seat.

- [s33] c2 (21 at 50 insns) and c3 (14 at 48 insns) close the anonymous route for the later blocks: their accesses sit after diamond join labels, which are cse path boundaries, so an anonymous address re-materialises there instead of being reused.

- [s33] Residual restated: blocks 0-1 in $v1 is REACHED (c1) and blocks 2-3 in $a0 is REACHED (candidate.c), each from a single declared object; both at once needs two pseudos each surviving a cse path boundary, i.e. two named C objects, the standing ban.

- [s33] KILL RE-AUDIT: the s20/s25 dead round-trip re-measures 10 at 49 on today's chassis (fifth consecutive confirmation) and fake_ablate again finds no FAKE-annotated construct in candidate.c.

- [s33] The s32 frontier item 'raise the address allocno's ref count to eight on a 17-insn live range' is RETIRED -- s33 reached the $v1 seat without touching ref counts, and the two-barrier framing is superseded: the hard-reg conflict is deletable at zero cost and priority never fires once a preference exists.

- [s33] Ladder accounting: s33 is session TWELVE of cycle 2 and the sixth distinct modality was already reached at s31 (escalation s23/s24, synthesis s25/s33, solver s26, forensics s27/s28, rederive s29/s30, structural s31/s32). Eight sessions remain before the owner directive 2026-09-02 permits any disposition.

- [s33] src/code6cac_b.c restored to HEAD (INCLUDE_ASM) at session end; git status --porcelain src/ is empty. candidate.c is unchanged apart from an s33 header note; the disproven bank is now 178 forms.

==== s34 (synthesis) ====

CHASSIS RE-MEASURE. `memory/grind/func_80034F88/candidate.c` installed at
`src/code6cac_b.c:3420` and measured on HEAD this session: **score 10,
target_insns 49, build_insns 49, rules_dropped 0, cheat_asm_stripped 28**. The
dispatch brief printed "measurement unavailable" for the sixth consecutive
session; the floor of 10 is measured, not inherited. `src/` restored to HEAD at
session end.

MANDATED KILL RE-AUDIT (the two closest-to-target banked forms, both re-measured
on today's chassis):
  * `rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c` (the
    s20/s25 dead round-trip, the only banked body whose instruction multiset
    matches the target's) -- **10 at 49 insns**, unchanged for the sixth
    consecutive session.
  * `rejected/s33-anon-symdiff-block0-blocks01-EXACT-score13.c` (c1, the
    structurally closest body: blocks 0 and 1 byte-exact) -- **13 at 49 insns**,
    reproducing s33 exactly.
  `tools/fake_ablate.py` was not re-run: s31/s32/s33 all report "no
  FAKE-annotated constructs found" for candidate.c and the body measured this
  session is byte-identical to the one they ablated, so no banked kill on this
  chassis was measured with a FAKE carrier occupying the contested pseudo.

--- 1. THE SYNTHESIS: WHAT THE TARGET ACTUALLY REQUIRES, IN PSEUDOS ---

Read straight off `asm/funcs/func_80034F88.s`, the flag byte's address is
materialised THREE times and lives in TWO hard registers:

    P1  $v1  lui/addiu at 80034F98; used by block 0's lbu+sb, block 1's lbu,
             and block 1's store `sb $v0,0($v1)` at 80034FD0 -- which sits
             AFTER the join label .L80034FC8.
    P2  $a0  lui/addiu at 80034FC8 (immediately after that same label, BEFORE
             block 1's store); used by block 2's lbu and its store at 80034FEC,
             again after a join label.
    P2' $a0  lui/addiu at 80034FF0; block 3's lbu and its store at 80035010,
             after .L80035010.

P2 and P2' are disjoint, so ONE pseudo assigned twice reproduces them -- that is
exactly what `candidate.c`'s `q` does, and blocks 2/3 are register-exact there.
P1 needs a SECOND pseudo, and its live range crosses a code label, i.e. a cse
path boundary (s27 gate 5) and a basic-block boundary, so it can be neither a
local-alloc quantity nor a cse-recovered anonymous temp. `global.c:426` gives one
pseudo one allocno one hard register, so P1 and P2 cannot be the same C object.
s33 measured both halves individually reachable from a single named object
(c1 = P1 correct, base = P2 correct); this session attacked the only remaining
question: **can the P1 carrier be an anonymous pseudo that survives the join
label?**

--- 2. ROUTE A -- TERNARY STORE (address expanded before the branch) ---

`expand_assignment` evaluates the destination MEM before the RHS, so writing
block 1 as `E = c ? (v | 1) : v;` puts the address into a pseudo in the SAME
basic block as the read, before the conditional -- an anonymous pseudo whose
live range then crosses the join with no named object. Measured (all with the
s33 anonymous symbol-difference `*(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 -
(s32)&D_80106A70))` as the spelling of E):

    d1  block0 anon, block1 anon ternary, blocks 2/3 q      22   48 insns
    d5  all four blocks anon, ternary stores                18   48 insns
    d6  block0 anon ternary, blocks 2/3 q ternary           26   50 insns
    d7  all-q chassis, block 1 ternary only                 12   46 insns
    d4  CONTROL: block0+block1 anon, statement form         21   50 insns
        (reproduces s33's c2 exactly, so the harness is sound)

Every ternary body LOSES instructions rather than gaining the seat.

--- 3. ROUTE B -- STORE DUPLICATED INTO BOTH ARMS ---

The sanctioned duplicated-statement-into-arms shape puts the store inside each
arm, so no address value has to cross the join at all:

    e1  block0 anon, block1 `if (c) { E = v|1; } else { E = v; }`, 2/3 q   22  48
    e4  e1 with the arms inverted (`if (!c)` first)                        22  48
    e5  all four blocks anon with duplicated arm stores                    18  48
    e6  all-q chassis, block 1 duplicated arm stores                       12  46

Identical scores and instruction counts to the matching ternary bodies -- the two
routes converge on the same RTL.

--- 4. WHY BOTH ROUTES LOSE AN INSTRUCTION -- PASS ATTRIBUTION FROM DUMPS ---

`pwsh tools/grinder/dump.ps1 func_80034F88` on e1, dumps kept at
`tmp/grind/func_80034F88/s34/dumps_e1/`. Counting `(set (mem:QI` in the
func_80034F88 slice of each dump (the trailing copy loop's store is `mem/s:QI`
and is not counted):

    .rtl .jump .cse .loop .cse2 .combine .flow .jump2 .lreg .greg .sched2 -> 5
    .dbr                                                                 -> 4
    (insns 21, 45, 55, 91, 125  ->  insns 21, 45, 91, 125;
     `(note 51 45 57 "" NOTE_INSN_DELETED)` stands where insn 55 was)

So BOTH arm stores survive expand, cse, combine, cross-jump, allocation and both
scheduler passes, and the else-arm store (insn 55) is deleted in the **delay-slot
reorg pass**, `tools/gcc-2.7.2/reorg.c:3442` (`prior_insn = redundant_insn
(trial, insn, delay_list)`; the matcher itself is `redundant_insn` at
reorg.c:1996). The emitted assembly
(`tmp/grind/func_80034F88/s34/dumps_e1/code6cac_b.s`) confirms it: `beq $2,$0,.L737`
with `ori` in the delay slot, ONE `sb`, and `.L737` placed after it -- the false
path stores nothing. The same shape also drops the block-1 reload (block 0's
masked value is still live in $v1), so the target's `lbu $a0,0($v1)` at 80034FB4
is gone too: a 2-instruction loss against the target's 49.

This corrects the natural guess (that cse deletes a store of a value it knows is
already in memory). cse does not touch these insns; reorg does, and only because
an arm-local store is a delay-slot thread candidate.

CONSEQUENCE. The target's block-1 store is unconditional at the join with a
merged value in $v0 -- exactly the `if/else` + `*q = c;` shape `candidate.c`
already has. Any spelling that moves that store inside an arm (to spare the
address from crossing the label) is deleted by reorg; any spelling that leaves it
at the join needs the address live across the label, and an anonymous carrier
re-materialises there for +1 instruction (d4/c2 = 50 vs the target's 49).

--- 5. TWO MIXED BODIES (anonymous read, named store) ---

    f1  block0 anon, block1 reads via E then `q = &D_80106A73;`, `*q = c;`   14  49
    f2  f1 with `q = &D_80106A73;` placed before the anonymous read          13  49

f2 collapses onto c1's score, confirming s33's c5 finding from the other side:
once `q` is materialised anywhere in the block-0/1 region, cse rewrites its
symbol set into a copy from the anonymous temp, `global.c:1709-1713` records
`74 preferences: 3`, and `q` takes $v1 for its whole range -- blocks 2/3 included.

--- 6. BOTH s33 FRONTIER ITEMS CLOSED WITH THE c1 .greg ---

`tmp/grind/func_80034F88/s34/dumps_c1/code6cac_b.greg`:

    ;; 9 regs to allocate: 73 80 84 88 74 79 83 87 72
    ;; 74 conflicts: 72 74 79 80 83 84 87 88 2 29
    ;; 74 preferences: 3
    ;; Register dispositions: 72 in 5  73 in 3  74 in 3 ... 79 in 4  83 in 4  87 in 4

  F1 ("is there a pseudo that already crosses the block-1 join that blocks 2/3
  could consume as an address base?") -- **ANSWERED: exactly one, pseudo 72,
  which is `p` in $a1** (`72 in 5`), the func_80077D00 return, live across the
  entire body. Consuming it as an address base for blocks 2/3 requires runtime
  arithmetic between an unrelated data pointer and a static symbol, which costs
  instructions and is not ordinary C. No other pseudo spans the join. F1 CLOSED.

  F2 ("is any pseudo seated in $a0 before block 2, to source an $a0 copy
  preference?") -- **its stated premise is FALSE on the c1 chassis**: 79, 83 and
  87 are all `in 4` ($a0), and 79 is block 1's loaded byte value, seated in $a0
  before block 2 exactly as the target's `lbu $a0,0($v1)` is. What does not exist
  is a legal C copy from a byte VALUE to an address pseudo, so the preference
  source cannot be spent. F2 CLOSED on the correct ground (no expressible copy),
  not on the ground s33 recorded (no $a0-seated pseudo).

--- 7. STATE OF THE SEARCH AFTER s34 ---

`candidate.c` is unchanged at 10/49. The admissible-form ceiling is still 10, and
the residual is stated in its sharpest form yet: the target needs a second
address pseudo whose live range crosses the block-1 join label; a named C object
is the only construct measured to produce one; a second named pointer object
aliasing `&D_80106A73` is the standing Judge ban. Ten new disproven forms banked
(bank size 188).

- [s34] Chassis re-measured on HEAD (dispatch printed 'measurement unavailable' for the sixth consecutive session): candidate.c at src/code6cac_b.c:3420 = score 10, target_insns 49, build_insns 49, rules_dropped 0, cheat_asm_stripped 28.
- [s34] KILL RE-AUDIT: the s20/s25 dead round-trip re-measures 10 at 49 insns and s33's c1 re-measures 13 at 49 insns on today's chassis; both banked kills hold unchanged.
- [s34] The target's address requirement restated in pseudos: P1 in $v1 spans block 0 through block 1's post-label store; P2 in $a0 is materialised twice (blocks 2 and 3), each crossing its own join label. candidate.c's single q reproduces P2 exactly; P1 needs a second pseudo whose live range crosses a code label.
- [s34] ROUTE A (ternary store, destination address expanded pre-branch by expand_assignment): d1 22/48, d5 18/48, d6 26/50, d7 12/46. None reaches 49 instructions; all lose the target's block-1 reload and its unconditional join store.
- [s34] ROUTE B (store duplicated into both arms, the sanctioned duplicated-statement-into-arms shape): e1 22/48, e4 22/48, e5 18/48, e6 12/46 -- scores and instruction counts identical to the matching ternary bodies, i.e. the two routes converge on the same RTL.
- [s34] CONTROL d4 (block 0 + block 1 anonymous, statement form) reproduces s33's c2 at 21/50 exactly, so the s34 harness measures the same thing s33's did.
- [s34] PASS ATTRIBUTION FROM DUMPS, not inference: in e1 both arm stores (insns 45 and 55) survive .rtl, .jump, .cse, .loop, .cse2, .combine, .flow, .jump2, .lreg, .greg and .sched2 (5 QI stores throughout) and insn 55 becomes '(note 51 45 57 "" NOTE_INSN_DELETED)' at .dbr -- the else-arm store is deleted by the delay-slot reorg pass, tools/gcc-2.7.2/reorg.c:3442 (redundant_insn, defined at reorg.c:1996), NOT by cse.
- [s34] Because reorg deletes an arm-local store as a redundant delay-slot thread insn, the target's unconditional join store cannot be spelled inside the arms; and leaving it at the join forces an anonymous address to re-materialise there (d4/c2 = 50 insns vs the target's 49). No anonymous carrier for P1 measured this session reaches 49 instructions AND the $v1 seat.
- [s34] f2 (q materialised before the anonymous read in block 1) scores 13 at 49, the same as c1, confirming s33's c5 result from the other side: any materialisation of q inside the block-0/1 region becomes the copy destination of the anonymous temp and drags q's whole range, blocks 2/3 included, onto $v1.
- [s34] s33 frontier F1 CLOSED with c1's .greg: the ONLY pseudo whose live range crosses the block-1 join for its own reasons is 72 = p in $a1 (';; Register dispositions: 72 in 5'), the func_80077D00 return; using it as an address base for blocks 2/3 needs runtime arithmetic between an unrelated data pointer and a static symbol.
- [s34] s33 frontier F2's premise is measured FALSE on the c1 chassis: pseudos 79, 83 and 87 are all seated in $a0 ('79 in 4'), and 79 is block 1's loaded byte value, seated in $a0 before block 2. F2 is closed instead by the absence of any legal C copy from a byte value to an address pseudo.
- [s34] Ladder accounting: s34 is session THIRTEEN of cycle 2; the six-modality condition was met at s31. Seven sessions remain before the owner directive 2026-09-02 permits any disposition.
- [s34] HARNESS TRAP (cost the first measurement batch): `bash tools/wsl.sh ...` invoked from the PowerShell tool fails with "wsl: command not found", so the s33 install-and-measure loop silently no-ops the install and every variant re-measures the previously installed body. The s34 runner does the source splice in PowerShell with explicit LF joins instead; tmp/grind/func_80034F88/s34/run.ps1.
- [s34] src/code6cac_b.c restored to HEAD (INCLUDE_ASM) at session end. candidate.c unchanged apart from an s34 header note; the disproven bank is now 188 forms.

### Artifacts (s34)

`tmp/grind/func_80034F88/s34/`: `variants/{cand,base,c1,c3,rt,d1,d4,d5,d6,d7,e1,e4,e5,e6,f1,f2}.c`,
`run.ps1`, `install.py`, `ext.py`, `dis.sh`, `dumps_e1/`, `dumps_c1/`, `c1.o`,
`e1.o`, `code6cac_b.c.orig`.

- [s34] Chassis re-measured on HEAD (dispatch printed 'measurement unavailable' for the sixth consecutive session): candidate.c at src/code6cac_b.c:3420 = score 10, target_insns 49, build_insns 49, rules_dropped 0, cheat_asm_stripped 28.

- [s34] The target's address requirement restated in pseudos, read off asm/funcs/func_80034F88.s: P1 in $v1 (lui/addiu at 80034F98) spans block 0, block 1's lbu and block 1's store at 80034FD0, which sits AFTER the join label .L80034FC8; P2 in $a0 is materialised twice (80034FC8 for block 2, 80034FF0 for block 3), each range also crossing a join label. candidate.c's single q reproduces P2 exactly; P1 requires a SECOND pseudo whose live range crosses a code label.

- [s34] ROUTE A (ternary store, destination address expanded pre-branch): d1 22/48, d5 18/48, d6 26/50, d7 12/46 -- all below the target's 49 instructions.

- [s34] ROUTE B (store duplicated into both arms): e1 22/48, e4 22/48, e5 18/48, e6 12/46 -- identical to the matching ternary bodies, the two routes converge on the same RTL.

- [s34] CONTROL d4 (block 0 + block 1 anonymous, statement form) reproduces s33's c2 at 21/50 exactly, so the s34 harness measures the same thing s33's did.

- [s34] PASS ATTRIBUTION FROM DUMPS: in e1 both arm stores (insns 45, 55) survive .rtl/.jump/.cse/.loop/.cse2/.combine/.flow/.jump2/.lreg/.greg/.sched2 (5 QI stores throughout) and insn 55 is '(note 51 45 57 "" NOTE_INSN_DELETED)' at .dbr -- the else-arm store is deleted by the delay-slot reorg pass, tools/gcc-2.7.2/reorg.c:3442 (redundant_insn at reorg.c:1996), NOT by cse. The same shape also drops the target's block-1 reload, a 2-instruction loss.

- [s34] Consequence: the target's block-1 store is unconditional at the join with a merged value in $v0 (exactly candidate.c's if/else + `*q = c;` shape). Moving it into the arms is deleted by reorg; leaving it at the join forces an anonymous address to re-materialise there for +1 instruction (d4/c2 = 50).

- [s34] f2 (q materialised before the anonymous read in block 1) scores 13 at 49, the same as c1, confirming s33's c5 from the other side: any materialisation of q inside the block-0/1 region becomes the copy destination of the anonymous temp and drags q's whole range, blocks 2/3 included, onto $v1 (global.c:1709-1713, '74 preferences: 3').

- [s34] c1's .greg: ';; 9 regs to allocate: 73 80 84 88 74 79 83 87 72'; ';; 74 preferences: 3'; ';; Register dispositions: 72 in 5  73 in 3  74 in 3 ... 79 in 4  83 in 4  87 in 4'. Pseudo 72 (p, in $a1) is the only allocno spanning the block-1 join; 79/83/87 are all seated in $a0.

- [s34] HARNESS TRAP worth inheriting: `bash tools/wsl.sh ...` invoked from the PowerShell tool fails with 'wsl: command not found', so an s33-style install-and-measure loop silently no-ops every install and re-measures the previously installed body. tmp/grind/func_80034F88/s34/run.ps1 does the splice in PowerShell with explicit LF joins instead.

- [s34] Ladder accounting: s34 is session THIRTEEN of cycle 2; the six-modality condition was met at s31. Seven sessions remain before the owner directive 2026-09-02 permits any disposition.

- [s34] src/code6cac_b.c restored to HEAD (INCLUDE_ASM) at session end; candidate.c unchanged apart from an s34 header note; ten new disproven forms banked (bank size 188).

==== s35 (synthesis) ====

CHASSIS RE-MEASURE. `memory/grind/func_80034F88/candidate.c` installed at
`src/code6cac_b.c:3420` and measured on HEAD this session (variant `g0`, a
byte-for-byte regeneration of the candidate body): **score 10, build_insns 49**.
The dispatch brief printed "measurement unavailable" for the SEVENTH consecutive
session; the floor of 10 is measured, not inherited. `src/` restored to HEAD at
session end (`git status --porcelain src/` empty).

MANDATED KILL RE-AUDIT (the two closest-to-target banked forms, both re-measured
on today's chassis):
  * `rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c` (the
    s20/s25 dead round-trip, the only banked body whose instruction multiset
    matches the target's) -- **10 at 49 insns**, seventh consecutive session
    unchanged.
  * `rejected/s33-anon-symdiff-block0-blocks01-EXACT-score13.c` (c1, blocks 0/1
    byte-exact) -- **13 at 49 insns**, reproducing s33/s34 exactly.
  * `python3 tools/fake_ablate.py --func func_80034F88 --file code6cac_b
    --candidate memory/grind/func_80034F88/candidate.c` -> "no FAKE-annotated
    constructs found ... nothing to ablate". No banked kill on this chassis was
    measured with a FAKE carrier occupying the contested pseudo.

--- 1. THE MERGED ATTACK, AND THE ONE HOLE LEFT IN IT ---

Merging s16/s17 (residual localisation), s27/s28 (cse input enumeration),
s29 (reload pricing), s31/s32 (allocator restatement), s33 (the local-alloc /
preference mechanism, c1-c5) and s34 (ternary / arm-duplication / mixed bodies,
reorg.c:3442) leaves ONE search axis with a finite parameter: which of the four
flag-byte access sites (block 0's mask, block 1, block 2, block 3) go through the
single declared pointer object `q`, and which go through the anonymous
symbol-difference expression
`*(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70))`.
The ledger already held five points of that lattice:

    q:0123  anon:none   base / candidate.c   10   49    <- floor
    q:123   anon:0      s33 c1               13   49    blocks 0/1 byte-exact
    q:23    anon:01     s33 c2               21   50
    q:1     anon:023    s33 c3               14   48
    q:12    anon:03     s33 c4               13   49

s35 measured the REMAINING quadrants, closing the lattice:

    q:01    anon:23                                     g1   28   48
    q:0     anon:123                                    g5   25   48
    q:01    anon:23, q initialised through the pun      g6   27   48
    q:0123  q initialised AND re-assigned through it    g7   14   49
    q:01    anon:23, two DIFFERENT anon spellings       g8   28   48

Every quadrant is >= the base's 10. The q-coverage lattice is enumerated end to
end and its minimum is `q:0123` = candidate.c = 10.

--- 2. TWO SUBSTANTIVE CORRECTIONS TO THE INHERITED FRAMING ---

(a) **"An anonymous address costs +1 instruction per label crossing" is FALSE as
a general statement.** It was generalised from s33's c2 (50 insns). g1's
disassembly (`tmp/grind/func_80034F88/s35/g1.txt`) shows blocks 2 and 3, spelled
anonymously, emit

    lui  v0,%hi   /  lbu v0,%lo(v0)        (read)
    lui  at,%hi   /  sb  v1,%lo(at)        (store)

= FOUR instructions per block, exactly the count of the target's
`lui / addiu / lbu / sb` for the same block. The anonymous carrier is
instruction-count-NEUTRAL in blocks 2/3; it is wrong only in FORM (two separate
%hi materialisations instead of one reused full address) and therefore in
register assignment. g1's single missing instruction (48 vs 49) is block 1's
reload, the same one candidate.c is missing -- not a label re-materialisation.
The +1 in c2/d4 is specific to block 1's post-join store, where the address must
be produced a second time INSIDE the join block.

(b) **`q`'s hard register is not a two-valued dial ($v1 vs $a0).** g1 shows a
third seat: with `q` covering only blocks 0+1, `q` takes **$a1** and `p` is
displaced to **$a0** -- both pseudos move, and the target's `p` in $a1 is lost as
well. This is the first body in the bank where shortening `q`'s range displaces
`p`. It confirms s16's stray note ("a block-1-only live range wins $a1 not $v1")
and generalises it: the address object and `p` compete for the same two argument
registers, so live-range surgery on `q` can cost the seat twice.

--- 3. THE "DISTINCT ANONYMOUS SPELLINGS" IDEA, KILLED ---

The one mechanism-level idea s33/s34 left implicit was that two anonymous address
expressions written DIFFERENTLY might escape cse's canonicalisation and each get
its own full-address pseudo -- which is what the target's blocks 2 and 3 have.
g8 spells block 2 as `(s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70)`
and block 3 as `(s32)&D_80106A73 + ((s32)&D_80106A70 - (s32)&D_80106A70)`.
g8 measures **28 at 48**, the same outcome as g1's single spelling: cse
canonicalises both to the same address rtx. Distinct spelling is not a lever.

--- 4. THE BLOCK-1 RELOAD FRONTIER ITEM, ANSWERED FROM THE BANK ---

The inherited frontier asked for "a spelling of block 0's mask that defeats cse's
memory-value tracking WITHOUT changing the address pseudo -- if it lands below 10
the one-missing-address-allocno factorisation is wrong". Every measured body that
actually restores the reload is now in hand and all are ABOVE 10:
`rejected/b1-read-direct-symbol-reload-priced-at-1-score11.c` (s17) = 11 at 50;
s34's f1 (anonymous read, named store) = 14 at 49; and g1 shows the reload is the
single instruction separating 48 from 49 in the anonymous quadrants. The
factorisation stands: the reload is not separable value from the address seat.

--- 5. STATE OF THE SEARCH AFTER s35 ---

`candidate.c` unchanged at 10/49. The residual is unchanged in substance and
sharper in statement: the target needs a full-address pseudo for blocks 0/1 in
$v1 AND a second full-address pseudo for blocks 2/3 in $a0; the anonymous carrier
can supply the instruction COUNT but never the reused-full-address FORM, and the
only construct measured to supply that form is a named C pointer object, of which
exactly one is permitted. Five new disproven forms banked (bank size 193).

- [s35] Chassis re-measured on HEAD (dispatch printed 'measurement unavailable' for the seventh consecutive session): the candidate body = score 10, build_insns 49.
- [s35] KILL RE-AUDIT: the s20/s25 dead round-trip re-measures 10 at 49 and s33's c1 re-measures 13 at 49 on today's chassis; both banked kills hold unchanged. fake_ablate again reports no FAKE-annotated construct in candidate.c.
- [s35] The q-coverage lattice (which of the four flag-byte access sites go through the declared pointer `q` versus the anonymous symbol-difference expression) is now ENUMERATED END TO END. New quadrants: g1 q:01/anon:23 = 28 at 48; g5 q:0/anon:123 = 25 at 48; g6 (punned q initialiser, q:01) = 27 at 48; g7 (punned q initialiser, full coverage) = 14 at 49; g8 (two distinct anon spellings) = 28 at 48. Minimum over the whole lattice is the base q:0123 = 10.
- [s35] CORRECTION to the inherited framing: an anonymous address is NOT '+1 instruction per label crossing'. g1's disassembly shows blocks 2/3 spelled anonymously emit lui+lbu for the read and lui+sb for the store = 4 insns per block, exactly the target's lui/addiu/lbu/sb count. The anonymous carrier is instruction-count-neutral there and wrong only in FORM (two %hi materialisations instead of one reused full address). The +1 seen in c2/d4 is specific to block 1's post-join store.
- [s35] CORRECTION: `q`'s seat is not a two-valued $v1/$a0 dial. In g1 (q covering blocks 0+1 only) `q` takes $a1 and `p` is displaced to $a0, so live-range surgery on the address object can cost the target's `p` seat as well.
- [s35] The 'two DIFFERENT anonymous spellings each get their own pseudo' idea is KILLED: g8 spells blocks 2 and 3 with different symbol-difference nestings and reproduces g1's 28 at 48 -- cse canonicalises both to the same address rtx.
- [s35] The inherited frontier item 'find a mask spelling that restores block 1's reload without a second address pseudo, and if it lands below 10 the factorisation is wrong' is ANSWERED from the bank: every measured body that restores the reload is above 10 (s17 direct-symbol read = 11 at 50; s34 f1 = 14 at 49), and g1 shows the reload is the single instruction separating 48 from 49 in the anonymous quadrants. The factorisation stands.
- [s35] Ladder accounting: s35 is session FOURTEEN of cycle 2; the six-modality condition was met at s31. Six sessions remain before the owner directive 2026-09-02 permits any disposition.
- [s35] src/code6cac_b.c restored to HEAD (INCLUDE_ASM) at session end; `git status --porcelain src/` empty. candidate.c unchanged apart from an s35 header note; five new disproven forms banked (bank size 193).

### Artifacts (s35)

`tmp/grind/func_80034F88/s35/`: `variants/{g0,g1,g5,g6,g7,g8,rt,c1}.c`, `gen.py`,
`run.ps1`, `g1.txt` (g1 disassembly), `code6cac_b.c.orig`, and the per-variant `.o` files.

- [s36] Chassis re-measured on HEAD this session (the dispatch brief printed 'measurement unavailable' for the seventh consecutive session): the candidate body = score 10, build_insns 49, rules_dropped 0. src/code6cac_b.c restored to HEAD (INCLUDE_ASM) at session end; `git status --porcelain src/` is empty.

- [s36] KILL RE-AUDIT: the s20/s25 dead round-trip re-measures 10 at 49 and s33's c1 re-measures 13 at 49 on today's chassis; both banked kills hold unchanged. tools/fake_ablate.py again reports no FAKE-annotated construct in candidate.c, so no banked kill on this chassis was measured with a FAKE carrier occupying the contested pseudo.

- [s36] The q-coverage lattice -- which of the four flag-byte access sites (block 0's mask, blocks 1, 2, 3) go through the single declared pointer `q` versus the anonymous symbol-difference expression *(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70)) -- is now enumerated END TO END. Inherited points: q:0123 = 10/49 (base), q:123 = 13/49 (c1), q:23 = 21/50 (c2), q:1 = 14/48 (c3), q:12 = 13/49 (c4). New s35 points: q:01 = 28/48 (g1), q:0 = 25/48 (g5), q:01 with punned q initialiser = 27/48 (g6), q:0123 with punned initialiser at every materialisation = 14/49 (g7), q:01 with two distinct anonymous spellings = 28/48 (g8). The minimum over the whole lattice is the base setting, candidate.c, at 10.

- [s36] CORRECTION to the framing s33/s34 handed forward: an anonymous address carrier is NOT '+1 instruction per label crossing'. g1's disassembly shows the anonymously-addressed blocks 2 and 3 emit lui + lbu %lo (read) and lui + sb %lo (store) = 4 instructions per block, exactly the count of the target's lui/addiu/lbu/sb. The carrier is instruction-count-neutral there and is wrong only in FORM -- two separate %hi materialisations instead of one reused full address -- hence in register assignment. The +1 measured in c2/d4 belongs specifically to block 1's post-join store, where the address must be produced a second time inside the join block.

- [s36] CORRECTION: `q`'s seat is not a two-valued $v1/$a0 dial. In g1 (q covering blocks 0+1 only) q takes $a1 and p is displaced from the target's $a1 into $a0 -- the address object and p compete for the same two argument registers, so live-range surgery on q can cost the target's p seat as well as failing to win the address seat.

- [s36] The 'two DIFFERENT anonymous spellings each get their own full-address pseudo' idea is killed: g8 spells blocks 2 and 3 with different symbol-difference nestings and reproduces g1's 28 at 48 exactly -- cse canonicalises both to the same address rtx.

- [s36] The inherited frontier item 'find a mask spelling that restores block 1's reload without a second address pseudo; if it lands below 10 the factorisation is wrong' is answered from the bank: every measured body that emits the reload is above the floor (s17 direct-symbol read = 11 at 50; s34 f1 = 14 at 49), and g1 shows the reload is the single instruction separating 48 from 49 in the anonymous quadrants.

- [s36] Residual restated after s35: the target needs a full-address pseudo for blocks 0/1 seated in $v1 AND a second full-address pseudo for blocks 2/3 seated in $a0. The anonymous carrier can supply the instruction COUNT (g1) but never the reused-full-address FORM; the only construct measured to supply that form is a named C pointer object, of which exactly one is permitted under the standing Judge constraint.

- [s36] Ladder accounting: s35 is session FOURTEEN of cycle 2 and the six-modality condition was met at s31 (escalation s23/s24, synthesis s25/s33/s34/s35, solver s26, forensics s27/s28, rederive s29/s30, structural s31/s32). Six sessions remain before the owner directive 2026-09-02 permits any disposition.

- [s36] Five new disproven forms banked in memory/grind/func_80034F88/rejected/ (s35-q-blocks01-anon-blocks23-score28.c, s35-q-block0-only-anon-123-score25.c, s35-qinit-punned-blocks01-anon23-score27.c, s35-qinit-punned-full-coverage-score14.c, s35-two-distinct-anon-spellings-score28-IDENTICAL-TO-g1.c); bank size is now 193.

==== s37 (synthesis) ====

(Session-numbering note: `state.json` recorded `session_count` 36 at dispatch and
the previous session tagged its bullets both `[s35]` and `[s36]`. This session is
tagged **s37** and its scratch is `tmp/grind/func_80034F88/s37/`, so nothing
overwrites the earlier directory.)

CHASSIS RE-MEASURE. `memory/grind/func_80034F88/candidate.c` regenerated as
variant `h0` and spliced into `src/code6cac_b.c:3420`: **score 10, build_insns
49**. The dispatch brief again printed "measurement unavailable", so the floor of
10 is measured this session, not inherited. `src/code6cac_b.c` restored to HEAD at
session end (`git status --porcelain src/` empty).

--- 1. THE 10 POINTS, READ OFF THE DISASSEMBLY INSTRUCTION BY INSTRUCTION ---

The base body's own objdump (`tmp/grind/func_80034F88/s37/h0.txt`) against
`asm/funcs/func_80034F88.s`, both 49 instructions, prologue through block 1:

    target                          h0 (candidate.c)
    lui   v1,%hi(D_80106A73)        lui   a0,%hi(D_80106A73)
    addiu v1,v1,%lo                 addiu a0,a0,%lo
    lbu   a0,0(v1)                  lbu   v1,0(a0)
    move  a1,v0                     move  a1,v0
    andi  a0,a0,0xf8                andi  v1,v1,0xf8
    sb    a0,0(v1)                  sb    v1,0(a0)
    lw    v0,0x20(a1)               lw    v0,32(a1)
    lbu   a0,0(v1)      <- reload   nop                 <- load-delay filler
    andi  v0,v0,0x1                 andi  v0,v0,0x1
    bnez  v0,.L80034FC8             bnez  v0,L
     ori  v0,a0,0x1                  ori  v0,v1,0x1
    addu  v0,a0,$zero               move  v0,v1
    lui   a0,%hi ; addiu a0         (materialised after the sb instead)
    sb    v0,0(v1)                  sb    v0,0(a0)

Blocks 2 and 3 are register-exact in the base body (the ledger's standing claim,
re-confirmed here). So the entire 10-point residual sits in blocks 0/1 and is
exactly three facts:

  (i)   the blocks-0/1 base is `$a0` where the target has `$v1`;
  (ii)  the blocks-0/1 byte value is `$v1` where the target has `$a0` -- the
        residual is a pure $v1 <-> $a0 ROTATION, not a missing computation;
  (iii) the target's block-1 reload `lbu $a0,0($v1)` occupies the load-delay slot
        of `lw $v0,0x20($a1)`, where the base body emits a bare `nop`.

(iii) sharpens s29's pricing from the other side: the reload is not merely "worth
zero points", it is worth zero INSTRUCTIONS -- the slot is already paid for by a
nop. It is also the fourth reference to the blocks-0/1 address pseudo, which
matters for (i) and (ii) through the priority formula below.

--- 2. WHY THE ROTATION HAPPENS, FROM THE BASE BODY'S OWN .greg ---

`pwsh tools/grinder/dump.ps1 func_80034F88` with h0 installed;
`tmp/grind/func_80034F88/dumps/code6cac_b.greg`, func_80034F88 block:

    ;; 9 regs to allocate: 73 78 82 86 74 77 81 85 72
    ;; 73 conflicts: 72 73 2 29
    ;; 74 conflicts: 72 74 77 78 81 82 85 86 2 3 29
    ;; Register dispositions: 72 in 5  73 in 3  74 in 4  ...

`74` is `q` (the single declared pointer), seated in **4 = $a0**; `73` is block 0's
byte value, seated in **3 = $v1**. The allocation ORDER is the whole story: 73
heads the sorted list, finds `$v0` blocked by the live call return and takes
`$v1`; when `74` is reached fifth it conflicts with both 2 and 3 and takes `$a0`.
The sort key is `allocno_compare` (`tools/gcc-2.7.2/global.c:635`):

    pri = (floor_log2(allocno_n_refs) * allocno_n_refs / allocno_live_length)
          * 10000 * allocno_size

so the short-lived block-0 value outranks the long-lived pointer, and the pointer
loses `$v1` by ORDER, not by conflict -- 73 and 74 do not conflict with each other.

CONSEQUENCE, and an independent re-derivation of the ledger's standing
factorisation (from the priority formula and the dispositions, not from the
earlier conflict-graph argument): the base chassis needs its address in `$v1` for
blocks 0/1 and in `$a0` for blocks 2/3. `global.c:426` gives one pseudo exactly
one hard register, so no re-ordering, re-prioritising or live-range surgery on the
single `q` can produce both seats. This also explains WHY the address loses the
seat, which s31/s32 left implicit.

--- 3. THE NEW AXIS: A `static inline` HELPER FUNCTION ---

The one construct that multiplies address pseudos WITHOUT the source declaring
more than one pointer object is a helper function inlined at each call site:
GCC 2.7.2's `integrate.c` copies the callee's RTL with fresh pseudos per call
site, so three calls to a helper holding `u8 *q = &D_80106A73;` yield three
address pseudos from ONE textual declaration. It is also the spelling a human
would reach for given three identical flag-set blocks, and the project already
ships the idiom (`src/main.c:1069` `static inline void vmSetStartAddr`, and
`src/main.c:2187` `static inline void _memcpy`). This axis had never been measured
on this function; the bank's
`macro-respelling-of-banned-four-handle-score0-DO-NOT-SUBMIT.c` is a TEXTUAL macro
expanding to four block-scoped declarations, which is a different construct.

Eleven bodies measured (`tmp/grind/func_80034F88/s37/variants/h1..h11.c`, scored
with `sandbox func_80034F88 --disable all`):

    h1   ptr-helper, blocks 1/2/3, fresh result local            31   48
    h2   ptr-helper, blocks 2/3 (blocks 0/1 keep caller q)       30   49
    h3   ptr-helper for all four sites (mask has its own helper) 31   48
    h4   symbol-helper, blocks 1/2/3, caller q for block 0       26   47
    h5   symbol-helper, no pointer object anywhere               30   44
    h6   CONTROL: no helper, all four sites on the plain symbol  29   47
    h7   symbol-helper blocks 2/3, caller q for blocks 0/1       29   46
    h8   ptr-helper (candidate.c's reuse shape), blocks 2/3      24   49
    h9   ptr-helper (reuse shape), blocks 1/2/3                  13   50
    h10  symbol-helper (reuse shape), blocks 2/3                 28   48
    h11  ptr-helper (reuse shape), block 3 only                  16   49

The best is h9 at 13 -- three points ABOVE the base floor and one instruction long.
The axis does not reach 10.

WHY IT FAILS, from h2's disassembly (`tmp/grind/func_80034F88/s37/h2.txt`, the
49-instruction member): inlining fires (no `jal` to the helper in the object and no
out-of-line copy emitted), and it does produce a SECOND full-address
materialisation -- `lui a1 / addiu a1` at +0x44, after block 1's store. But the two
inlined copies are then merged by cse into a single STORE base in `$a1`, while each
inlined READ is folded by combine into a bare `lui vX` + `lbu 0(vX)` pair, losing
the target's reused-full-address read base. Worse, the helper's address pseudo
takes `$a1` and displaces `p` into `$a0` -- the same double loss s35 measured for
the shortened-`q` quadrant (g1). The helper multiplies pseudos, but not into the
seats the target uses.

Side result worth inheriting: h9's helper mirrors candidate.c's block shape
exactly, i.e. the condition PARAMETER is reused as the merged value, which is what
emits the target's `addu v0,a0,$zero` select move. That reuse is worth 11-18 points
inside the family (h1 31 -> h9 13; h2 30 -> h8 24), so the reuse idiom is confirmed
load-bearing independently of the helper question.

--- 4. TWO SMALLER RESULTS ---

(a) **Duplicate-read-into-arms is byte-neutral on block 1.** The frozen sanctioned
family "duplicate-read into branch arms"
(`.claude/rules/split-read-defeats-hoist.md`) applied to block 1 only with no
pre-read (`if (c) { c = *q | 1; } else { c = *q; }`) measures **10 at 49** -- and
its objdump is BIT-IDENTICAL to the base body's (`diff h0.txt h15.txt` empty). cse
collapses the arm reads back onto block 0's stored value, so the family cannot
restore the reload here. The pre-read variant (h14, read kept for the else arm) is
19 at 51; all three blocks at once is 29 at 55 (h16) / 25 at 53 (h17).

(b) **Feeding the trailing copy loop off `q` does not lift its priority usefully.**
`q[i - 3]` (h13) and `*(q - 3 + i)` (h12) -- the idiom the file itself already uses
at `src/code6cac_b.c:4044` (`(&D_80106A73) - 3`) -- both measure **30 at 48**: the
loop loses its own `lui %hi(D_80106A70) / addu / sb %lo` materialisation, which
costs more than the extra references to `q` buy.

--- 5. STATE OF THE SEARCH AFTER s37 ---

`candidate.c` unchanged at 10/49. The residual is now stated at instruction
granularity (blocks 0/1 only: a $v1<->$a0 rotation plus a free reload slot), its
allocator cause is derived from the base body's own dispositions plus
`global.c:635`'s priority formula, and the strongest remaining ordinary-C
construct for producing a second address pseudo from a single declaration -- a
`static inline` helper -- is measured across eleven bodies and does not reach the
floor. Six new disproven forms banked (bank size 199).

- [s37] Chassis re-measured on HEAD (the dispatch brief printed 'measurement unavailable' for the eighth consecutive session): the candidate body, regenerated as variant h0, = score 10, build_insns 49. src/code6cac_b.c restored to HEAD at session end; `git status --porcelain src/` empty.
- [s37] The 10-point residual is localised to blocks 0/1 at instruction granularity by diffing h0's own objdump against asm/funcs/func_80034F88.s: (i) the blocks-0/1 base is $a0 where the target has $v1; (ii) the blocks-0/1 byte value is $v1 where the target has $a0 -- a pure $v1<->$a0 rotation; (iii) the target's block-1 reload `lbu $a0,0($v1)` sits in the load-delay slot of `lw $v0,0x20($a1)`, where the base body emits a bare `nop`. Blocks 2/3 are register-exact.
- [s37] The reload therefore costs ZERO instructions (its slot is already spent on a nop), a stronger statement than s29's arithmetic pricing; it is also the fourth reference to the blocks-0/1 address pseudo.
- [s37] The base body's own .greg (tmp/grind/func_80034F88/dumps/code6cac_b.greg): ';; 9 regs to allocate: 73 78 82 86 74 77 81 85 72'; ';; 73 conflicts: 72 73 2 29'; ';; Register dispositions: 72 in 5  73 in 3  74 in 4'. q is pseudo 74 seated in $a0; block 0's byte value is pseudo 73 seated in $v1. 73 and 74 do NOT conflict -- q loses $v1 purely to allocation ORDER, because allocno_compare (tools/gcc-2.7.2/global.c:635) sorts by floor_log2(n_refs)*n_refs/live_length and the short-lived value outranks the long-lived pointer.
- [s37] Independent re-derivation of the standing factorisation: the base chassis needs its address in $v1 for blocks 0/1 and in $a0 for blocks 2/3; global.c:426 gives one pseudo one hard register, so no re-ordering or live-range surgery on the single q can produce both seats.
- [s37] NEW AXIS MEASURED AND KILLED -- a `static inline` helper function. GCC 2.7.2's integrate.c copies the callee RTL with fresh pseudos per call site, so N calls to a helper holding `u8 *q = &D_80106A73;` give N address pseudos from ONE textual declaration; the idiom is ordinary C and already ships in this project (src/main.c:1069, src/main.c:2187). Eleven bodies: h1 31/48, h2 30/49, h3 31/48, h4 26/47, h5 30/44, h6 (control, plain symbol, no helper) 29/47, h7 29/46, h8 24/49, h9 13/50, h10 28/48, h11 16/49. Best is 13, three points above the floor.
- [s37] Mechanism of that failure, from h2's disassembly: inlining fires (no jal, no out-of-line copy) and does emit a second full-address materialisation, but cse merges the inlined copies into ONE store base in $a1 while combine folds each inlined READ into a bare lui + lbu %lo pair, losing the target's reused-full-address read base; and the helper's address pseudo takes $a1, displacing p out of the target's $a1 into $a0.
- [s37] The condition-parameter-reused-as-merged-value shape (candidate.c's idiom, which emits the target's `addu v0,a0,$zero` select move) is worth 11-18 points inside the helper family (h1 31 -> h9 13; h2 30 -> h8 24), confirming the reuse idiom is load-bearing independently of the helper question.
- [s37] The frozen sanctioned family 'duplicate-read into branch arms' is BYTE-NEUTRAL on block 1 here: `if (c) { c = *q | 1; } else { c = *q; }` (h15) measures 10 at 49 and its objdump is bit-identical to the base body's (empty diff). cse collapses the arm reads onto block 0's stored value, so the family cannot restore the reload. Pre-read variant h14 19/51; all three blocks h16 29/55, h17 25/53.
- [s37] Feeding the trailing copy loop off q -- `q[i - 3]` (h13) and `*(q - 3 + i)` (h12), the idiom src/code6cac_b.c:4044 already uses -- measures 30 at 48 for both: the loop loses its own lui %hi(D_80106A70) / addu / sb %lo materialisation, which costs more than the extra references to q buy.
- [s37] Ladder accounting: this is session FIFTEEN of cycle 2 (state.json session_count 36 at dispatch); the six-modality condition was met at s31. Five sessions remain before the owner directive 2026-09-02 permits any disposition.
- [s37] Six new disproven forms banked in memory/grind/func_80034F88/rejected/ (s37-inline-helper-ptr-blocks23-score30.c, s37-inline-helper-symbol-no-pointer-anywhere-score30.c, s37-inline-helper-ptr-blocks123-reuse-shape-score13.c, s37-inline-helper-ptr-block3-only-score16.c, s37-q-as-copy-loop-base-score30.c, s37-dupread-both-arms-all-three-blocks-score29.c); bank size is now 199.

### Artifacts (s37)

`tmp/grind/func_80034F88/s37/`: `variants/{h0..h18}.c`, `gen.py`, `gen2.py`,
`gen3.py`, `run.ps1`, `dis.sh`, `h0.txt`, `h2.txt`, `h15.txt`, per-variant `.o`
files, `code6cac_b.c.orig`. Allocation dump:
`tmp/grind/func_80034F88/dumps/code6cac_b.greg` (base body installed).

--- 6. LATE ADDITION: THE ALLOCNO-ORDER LEVER, PROBED AND MEASURED BYTE-INERT ---

Section 2 identified the ordering of block 0's byte-value allocno (73) ahead of
the pointer allocno (74) as the proximate cause of the `$v1 <-> $a0` rotation, and
`allocno_compare`'s formula
`floor_log2(n_refs)*n_refs/live_length` as the only handle on it. Reference count
cannot be raised for free (every reference is an instruction), so the free
parameter is the VALUE allocno's LIVE LENGTH: lengthen it and its priority falls
below the pointer's. Five bodies (`tmp/grind/func_80034F88/s37/gen4.py`,
variants `k1`-`k5`):

    k1  block 0's masked value kept in a local and merged by block 1
        (block 1 does not re-read the byte at all)                    10   49
    k2  the same shared value carried on into block 2                 13   49
    k3  p[8] hoisted into a local before block 0 (F2 scheduling probe) 20   47
    k4  block 1's byte read moved ahead of its p[8] condition read    10   49
    k5  block 0's mask split into `m = *q; m = m & 0xF8; *q = m;`     10   49

k1, k4 and k5 all TIE the floor at 10/49 -- and all three disassemble
BIT-IDENTICALLY to the base body (`diff h0.txt k1.txt` / `k4.txt` / `k5.txt` all
empty). Lengthening the value pseudo's C-level live range does not lengthen its
RTL live range: cse forwards block 0's stored value into block 1 in the base body
anyway, so k1's "shared value" and the base's "re-read that gets forwarded" are
the same RTL. The allocno-order lever is therefore not reachable from statement
structure at zero instruction cost; the two bodies that DO change 73's range
(k2, k3) change the instruction stream and score worse.

This extends s31's "seven bodies tie at 10 and emit a bit-identical stream" to
TEN bodies (s31's seven, plus s37's h15, k1, k4, k5), and it is the sharpest
statement of the base basin's rigidity so far: three structurally different
spellings of block 0/1's value flow, plus one sanctioned duplicate-read arm
rewrite, all collapse to the identical 49 instructions.

- [s37] The allocno-ORDER lever (demote block 0's byte-value allocno below the pointer allocno in allocno_compare by lengthening its live range) is measured BYTE-INERT: k1 (value kept in a local and merged by block 1, no re-read) 10/49, k4 (byte read moved ahead of the p[8] condition read) 10/49 and k5 (mask split into `m = *q; m = m & 0xF8; *q = m;`) 10/49 all disassemble bit-identically to the base body. cse forwards block 0's stored value into block 1 in the base body anyway, so the C-level "longer live range" is the same RTL. k2 (shared value carried into block 2) 13/49 and k3 (p[8] hoisted before block 0) 20/47 do change the stream, and both score worse.
- [s37] Bit-identical-tie count for this basin is now TEN bodies: s31's seven, plus s37's h15 (duplicate-read arms), k1, k4 and k5.
- [s37] Three further disproven forms banked (s37-block0-value-lives-across-block1-score10-BIT-IDENTICAL.c, s37-p8-hoisted-before-block0-score20.c, s37-block0-mask-split-two-statements-score10-BIT-IDENTICAL.c); bank size is 202. Artifacts add `s37/gen4.py`, `s37/variants/{k1..k5}.c`, `s37/{k1,k4,k5}.txt`.

- [s37] Chassis re-measured on HEAD (the dispatch brief printed 'measurement unavailable' for the eighth consecutive session): the candidate body, regenerated as variant h0, scores 10 at 49 build_insns, rules_dropped 0. src/code6cac_b.c restored to HEAD at session end; `git status --porcelain src/` empty.

- [s37] KILL RE-AUDIT: the base body itself is the closest-to-target banked form (49/49 instructions) and re-measures 10; `python3 tools/fake_ablate.py --func func_80034F88 --file code6cac_b --candidate memory/grind/func_80034F88/candidate.c` again reports no FAKE-annotated construct, so no banked kill on this chassis was measured with a FAKE carrier occupying the contested pseudo.

- [s37] The 10-point residual is localised at instruction granularity to blocks 0/1: (i) their base is $a0 where the target has $v1; (ii) their byte value is $v1 where the target has $a0 -- a pure rotation, no missing computation; (iii) the target's block-1 reload `lbu $a0,0($v1)` fills the load-delay slot of `lw $v0,0x20($a1)` that the base body wastes on a `nop`, so the reload costs ZERO instructions. Blocks 2/3 are register-exact.

- [s37] From the base body's own .greg: ';; 9 regs to allocate: 73 78 82 86 74 77 81 85 72'; ';; 73 conflicts: 72 73 2 29'; ';; Register dispositions: 72 in 5  73 in 3  74 in 4'. q (74) loses $v1 to allocation ORDER, not conflict: allocno_compare (tools/gcc-2.7.2/global.c:635) sorts by floor_log2(n_refs)*n_refs/live_length, so the short-lived block-0 value is allocated first, finds $v0 blocked by the live call return and takes $v1.

- [s37] NEW AXIS KILLED -- the `static inline` helper. Eleven bodies: h1 31/48, h2 30/49, h3 31/48, h4 26/47, h5 30/44, h6 (control, plain symbol, no helper) 29/47, h7 29/46, h8 24/49, h9 13/50, h10 28/48, h11 16/49. Inlining fires (no jal, no out-of-line copy) and emits a second full-address materialisation, but cse merges the inlined copies into one store base in $a1, combine folds each inlined read into a bare lui + lbu %lo pair, and the helper's address pseudo displaces p out of the target's $a1.

- [s37] The condition-parameter-reused-as-merged-value shape (candidate.c's idiom, which emits the target's `addu v0,a0,$zero` select move) is worth 11-18 points inside the helper family (h1 31 -> h9 13; h2 30 -> h8 24), so the reuse idiom is load-bearing independently of the helper question.

- [s37] The allocno-ORDER lever is byte-inert from statement structure: k1 (block 0's value merged by block 1 with no re-read) 10/49, k4 (byte read ahead of the p[8] condition read) 10/49 and k5 (mask split into two statements) 10/49 all disassemble BIT-IDENTICALLY to the base body. k2 13/49 and k3 (p[8] hoisted before block 0) 20/47 change the stream and score worse.

- [s37] The frozen sanctioned family 'duplicate-read into branch arms' is byte-neutral on block 1 here: h15 measures 10 at 49 with an objdump bit-identical to the base body's; cse collapses the arm reads onto block 0's stored value.

- [s37] Feeding the trailing copy loop off q (`q[i - 3]` h13, `*(q - 3 + i)` h12 -- the idiom src/code6cac_b.c:4044 already uses) measures 30 at 48 for both: the loop loses its own lui %hi(D_80106A70) / addu / sb %lo materialisation.

- [s37] The bit-identical-tie count for this basin is now TEN bodies (s31's seven, plus s37's h15, k1, k4, k5) -- the sharpest statement so far of how rigid the base basin is under statement restructuring.

- [s37] Ladder accounting: this is session fifteen of cycle 2 (state.json session_count 36 at dispatch); the six-modality condition was met at s31. Five sessions remain before the owner directive 2026-09-02 permits any disposition.

- [s37] Nine new disproven forms banked in memory/grind/func_80034F88/rejected/ (four s37-inline-helper-*, s37-q-as-copy-loop-base-score30, s37-dupread-both-arms-all-three-blocks-score29, s37-block0-value-lives-across-block1-score10-BIT-IDENTICAL, s37-p8-hoisted-before-block0-score20, s37-block0-mask-split-two-statements-score10-BIT-IDENTICAL); bank size is now 202.

==== s38 (synthesis) ====

(Session-numbering note: the dispatch brief called this "session 35", but the
ledger already carries `==== s35 ====` and `==== s37 ====` blocks from earlier
runs and `state.json` records a higher `session_count`. This session is tagged
**s38** and its scratch is `tmp/grind/func_80034F88/s38/`, so nothing overwrites
an earlier directory.)

CHASSIS RE-MEASURE. `memory/grind/func_80034F88/candidate.c` spliced into
`src/code6cac_b.c` as variant `b0`: **score 10, build_insns 49**. The dispatch
brief printed "measurement unavailable" for the NINTH consecutive session, so the
floor of 10 is measured this session, not inherited. `src/code6cac_b.c` and
`include/code6cac.h` are both restored to HEAD at session end
(`git status --porcelain src/ include/` empty).

MANDATED KILL RE-AUDIT (the two closest-to-target banked forms, re-measured on
today's chassis):
  * `rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c` (the
    s20/s25 dead round-trip, the only banked body whose instruction multiset
    matches the target's) -- **10 at 49 insns**, unchanged.  Its disassembly
    differs from the base body by exactly ONE instruction
    (`diff s38/b0.txt s38/rt.txt`: our `nop` at +0x4f64 becomes
    `lbu $v1,0($a0)`), confirming for the first time by direct diff that the
    reload is free and lands in the WRONG register ($v1/$a0 rotated).
  * `rejected/s33-anon-symdiff-block0-blocks01-EXACT-score13.c` (c1) --
    **13 at 49**, reproducing s33/s34/s35 exactly.
  * `tools/fake_ablate.py --func func_80034F88 --file code6cac_b --candidate
    memory/grind/func_80034F88/candidate.c` -> "no FAKE-annotated constructs
    found ... nothing to ablate".  No banked kill on this chassis was measured
    with a FAKE carrier occupying the contested pseudo.

--- 1. THE INHERITED CAUSAL STORY WAS WRONG: PSEUDO 73 IS THE LOOP COUNTER ---

s37 read the base body's `.greg` header line
`;; 9 regs to allocate: 73 78 82 86 74 77 81 85 72` and identified **pseudo 73
(seated in $v1, the top-priority allocno) as "block 0's byte value"**, then built
its whole causal account on that: "q loses $v1 to allocation ORDER, because
allocno_compare sorts the short-lived block-0 value ahead of the long-lived
pointer".  That attribution is FALSE, and this session read it off the RTL rather
than inferring it.

`pwsh tools/grinder/dump.ps1 func_80034F88` with `b0` installed; the function's
segment of `tmp/grind/func_80034F88/dumps/code6cac_b.lreg` mentions
`(reg/v:SI 73)` in exactly these insns:

    (insn 122 ... (set (reg/v:SI 73) ...))                      ; i = 0
    (insn 142 ... (set (mem/s:QI (plus:SI (reg/v:SI 73) ...     ; D_80106A70[i] store
    (insn 148 ... (set (reg/v:SI 73) (plus:SI (reg/v:SI 73) ... ; i++
    ...            (lt:SI (reg/v:SI 73) ...                     ; i < 3

**Pseudo 73 is `i`, the induction variable of the trailing three-byte copy
loop** -- not block 0's byte value.  Its nrefs of 11 over a live length of 7 is
the loop-depth weighting flow.c applies to loop-body references, which is why it
tops the priority sort.  And it is CORRECT: the target seats `i` in $v1 too
(`addu $v1,$zero,$zero` / `addiu $v1,$v1,0x1` at 80035014 / 8003502C).  The loop
counter competes with nothing; s37's "short-lived block-0 value outranks the
long-lived pointer" describes a competition that does not exist.

--- 2. THE REAL BARRIER, WITH EXACT NUMBERS: LOCAL-ALLOC, NOT GLOBAL PRIORITY ---

The instrumented cc1 (`tools/gcc-2.7.2/cc1`, BB2_ALLOC_DEBUG=1, granted for this
function by decisions.md:14784) prints the full allocno table.  Base body
(`tmp/grind/func_80034F88/s38/allocdbg_b0.txt`):

    ord=0 pseudo=73 hardreg=3  nrefs=11 livelen=7  pri=47142   <- i (loop)
    ord=1 pseudo=78 hardreg=2  nrefs=5  livelen=7  pri=14285   <- p[8]&1 cond
    ord=2 pseudo=82 hardreg=2  nrefs=5  livelen=7  pri=14285
    ord=3 pseudo=86 hardreg=2  nrefs=5  livelen=7  pri=14285
    ord=4 pseudo=74 hardreg=4  nrefs=10 livelen=29 pri=10344   <- q
    ord=5 pseudo=77 hardreg=3  nrefs=3  livelen=4  pri=7500
    ord=6 pseudo=81 hardreg=3  nrefs=3  livelen=4  pri=7500
    ord=7 pseudo=85 hardreg=3  nrefs=3  livelen=4  pri=7500
    ord=8 pseudo=72 hardreg=5  nrefs=6  livelen=34 pri=3428    <- p

and the `.greg` conflict rows are

    ;; 73 conflicts: 72 73 2 29
    ;; 74 conflicts: 72 74 77 78 81 82 85 86 2 3 29

`73` and `74` do NOT conflict.  `74` (q) carries a HARD-REG conflict with reg 3
($v1) which `73` does not.  The source of that hard conflict is visible in the
same `.greg` dump's RTL, BEFORE global allocation runs:

    (insn 14 (set (reg/v:SI 4 a0) (symbol_ref:SI ("D_80106A73"))))
    (insn 17 (set (reg:QI 3 v1) (mem:QI (reg/v:SI 4 a0))))
    (insn 18 (set (reg:SI 3 v1) (and:SI (subreg:SI (reg:QI 3 v1) 0) (const_int 248))))
    (insn 20 (set (mem:QI (reg/v:SI 4 a0)) (subreg:QI (reg:SI 3 v1) 0)))

Block 0's masked byte is already a HARD register, `$v1`, at that point: it is a
single-basic-block quantity, so **local-alloc seats it in $v1 before global-alloc
ever looks at `q`**, and `q` -- live across it -- therefore hard-conflicts with
$v1 and can never be given that seat by `find_reg`.  ($v0 is unavailable to that
quantity because the call return is still live: insn 11, `a1 = v0`, is scheduled
AFTER insn 20.)

**This replaces the priority story with a stronger one, and the difference is
operational:** priority is a dial a session can push on from C (reference count,
live length); a hard-register conflict is not.  The proof that priority is NOT
the barrier is already in the bank: in the round-trip body `rt`, q's own numbers
rise to `nrefs=15 livelen=30 pri=15000` and it moves from **ord=4 to ord=1** --
allocated before every allocno except the loop counter -- and it STILL takes
`hardreg=4` ($a0).  Winning the sort does not win the seat.

--- 3. WHAT THE TARGET DOES, RE-READ AT INSTRUCTION LEVEL ---

`asm/funcs/func_80034F88.s` (49 insns) holds &D_80106A73 in $v1 for blocks 0+1
(80034F98..80034FD0) and materialises it AGAIN into $a0 for block 2 (80034FC8)
and a third time into $a0 for block 3 (80034FF0); block 0/1's byte value is $a0
and blocks 2/3's is $v1; `i` is $v1.  So the target's blocks-0/1 address is a
global pseudo that WON $v1, which means in the original compile block 0's byte
quantity was NOT local-allocated to $v1 -- it took $a0.  With a single C object
that seat is unreachable for the reason in section 2, and the second seat needs a
second pseudo (global.c:426), i.e. the banned second object.  The residual
factorisation the ledger has carried since s16 is therefore re-derived a fifth
time, now from local-alloc rather than from global priority.

--- 4. THE DECLARATION PUN IS REMOVABLE AT ZERO CODEGEN COST ---

The dispatch brief's auto-scan flags `candidate.c`'s trailing loop
`*(&D_80106A70 + i) = *((u8 *)p + i + 0x17);` as a DECLARATION PUN against
`extern u8 D_80106A70;` (include/code6cac.h:472) and states that a candidate
carrying it FAILs layer-1; the sanctioned fix is at the declaration.  Measured
this session for the first time:

    include/code6cac.h:472  extern u8 D_80106A70;  ->  extern u8 D_80106A70[3];
    loop body               *(&D_80106A70 + i) = ...  ->  D_80106A70[i] = ...

    a1  score 10  build_insns 49   and  `diff s38/b0.txt s38/a1.txt` is EMPTY

The pun-free body is **bit-identical** to the punned one.  D_80106A70 is
referenced in exactly two places project-wide (`grep -rn D_80106A70 include src`
= the header line and this loop), so the header change is a one-line, one-symbol
INTEGRATION HANDOFF with no other build-file impact.  The pun-free body is banked
at `memory/grind/func_80034F88/candidate_arraydecl_pun_free.c`; `candidate.c`
keeps the punned spelling only because it is the form that compiles against
HEAD's header unmodified.

The array spelling must stay on the DESTINATION operand only: writing the source
byte as `((u8 *)p)[i + 0x17]` measures **12 at 49** (banked as
`rejected/s38-loop-source-byte-array-indexed-score12.c`).

--- 5. STATE OF THE SEARCH AFTER s38 ---

`candidate.c` unchanged at 10/49.  The causal account of the residual is
corrected and sharpened: the blocking fact is a LOCAL-ALLOC hard-register seat
($v1 taken by block 0's block-local masked byte before global runs), not a
global-allocno priority ordering, and the priority dial is measured impotent even
when it is won outright (rt: ord 4 -> 1, seat unchanged).  The layer-1
declaration-pun blocker is closed at zero cost, contingent on a one-line header
handoff.  One new disproven form banked (bank size 203).

- [s38] Chassis re-measured on HEAD (dispatch printed 'measurement unavailable' for the ninth consecutive session): the candidate body, spliced as variant b0, = score 10, build_insns 49. src/code6cac_b.c and include/code6cac.h restored to HEAD at session end; `git status --porcelain src/ include/` empty.
- [s38] KILL RE-AUDIT: rt (the s20/s25 dead round-trip) re-measures 10 at 49 and c1 (s33's anon-symdiff block 0) re-measures 13 at 49; fake_ablate again reports no FAKE-annotated construct in candidate.c. NEW: `diff s38/b0.txt s38/rt.txt` shows the two bodies differ by EXACTLY ONE instruction -- our `nop` becomes `lbu $v1,0($a0)` -- the first direct-diff confirmation that the block-1 reload is instruction-free and lands in the rotated register.
- [s38] CORRECTION TO s37: pseudo 73 -- the top-priority allocno (nrefs=11, livelen=7, pri=47142) that takes $v1 -- is NOT 'block 0's byte value'. The .lreg RTL shows reg 73 in `(set (reg/v:SI 73) ...)`, `(mem/s:QI (plus:SI (reg/v:SI 73) ...))`, `(set (reg/v:SI 73) (plus:SI (reg/v:SI 73) ...))` and `(lt:SI (reg/v:SI 73) ...)`: it is `i`, the induction variable of the trailing three-byte copy loop, and its nrefs of 11 is flow.c's loop-depth weighting. The target seats `i` in $v1 as well (80035014 / 8003502C), so it is CORRECT and competes with nothing. s37's mechanism ('the short-lived block-0 value outranks the long-lived pointer in allocno_compare') describes a competition that does not exist.
- [s38] THE REAL BARRIER, read from the .greg RTL before global allocation: block 0's masked byte is already the HARD register $v1 -- `(insn 17 (set (reg:QI 3 v1) (mem:QI (reg/v:SI 4 a0))))`, `(insn 18 (set (reg:SI 3 v1) (and:SI (subreg:SI (reg:QI 3 v1) 0) (const_int 248))))`, `(insn 20 (set (mem:QI (reg/v:SI 4 a0)) (subreg:QI (reg:SI 3 v1) 0)))`. It is a single-basic-block quantity, so LOCAL-alloc seats it in $v1 before global-alloc considers q; q is live across it and therefore carries the hard-reg-3 conflict listed in `;; 74 conflicts: 72 74 77 78 81 82 85 86 2 3 29`. $v0 is not available to that quantity because the call return is still live (insn 11, `a1 = v0`, is scheduled after insn 20).
- [s38] The global-priority dial is measured IMPOTENT for the seat: in the round-trip body rt, q's allocno rises to nrefs=15 livelen=30 pri=15000 and moves from ord=4 to ord=1 (ahead of every allocno except the loop counter), and it still receives hardreg=4 ($a0). Winning allocno_compare does not win $v1, because the obstruction is a hard-register conflict created by local-alloc, not a sort order.
- [s38] Base-body allocno table (instrumented cc1, BB2_ALLOC_DEBUG, tmp/grind/func_80034F88/s38/allocdbg_b0.txt): 73/$v1 nrefs=11 livelen=7 pri=47142 (i); 78,82,86/$v0 nrefs=5 livelen=7 pri=14285 (the three p[8]&mask conditions); 74/$a0 nrefs=10 livelen=29 pri=10344 (q); 77,81,85/$v1 nrefs=3 livelen=4 pri=7500 (the three block values); 72/$a1 nrefs=6 livelen=34 pri=3428 (p). c1's table is also banked (allocdbg_rt_c1.txt): there q reaches hardreg=3 ($v1) and the per-block values move to $a0, which is exactly why c1's blocks 0/1 are byte-exact and its blocks 2/3 are not.
- [s38] THE DECLARATION PUN IS REMOVABLE AT ZERO COST. With include/code6cac.h:472 changed from `extern u8 D_80106A70;` to `extern u8 D_80106A70[3];` and the trailing loop spelled `D_80106A70[i] = *((u8 *)p + i + 0x17);`, variant a1 measures 10 at 49 and its objdump is BIT-IDENTICAL to the punned base (`diff s38/b0.txt s38/a1.txt` empty). D_80106A70 is referenced in exactly two places project-wide (the header line and this loop), so the fix is a one-line INTEGRATION HANDOFF. Banked as memory/grind/func_80034F88/candidate_arraydecl_pun_free.c; any future candidate-ready should be that body plus the header line.
- [s38] The array spelling is destination-only: writing the loop's SOURCE byte as `((u8 *)p)[i + 0x17]` instead of `*((u8 *)p + i + 0x17)` measures 12 at 49 (banked as rejected/s38-loop-source-byte-array-indexed-score12.c).
- [s38] Ladder accounting: this is session SIXTEEN of cycle 2; the six-modality condition was met at s31. Four sessions remain before the owner directive 2026-09-02 permits any disposition.
- [s38] One new disproven form banked; bank size is now 203.

### Artifacts (s38)

`tmp/grind/func_80034F88/s38/`: `variants/{b0,rt,c1,a1,a2}.c`, `run.ps1`,
`splice.py`, `allocdbg.sh`, `ad.sh`, `dis.sh`, `allocdbg_b0.txt`,
`allocdbg_rt_c1.txt`, `qtydbg_b0.txt`, `b0.txt`, `a1.txt`, `rt.txt`,
`code6cac_b.c.orig`, per-variant `.o` files.  Allocation dump:
`tmp/grind/func_80034F88/dumps/code6cac_b.{lreg,greg}` (base body installed).

- [s38] Chassis re-measured this session (the dispatch brief printed 'measurement unavailable' for the ninth consecutive session): candidate.c spliced as variant b0 = score 10, build_insns 49, rules_dropped 0. src/code6cac_b.c and include/code6cac.h both restored to HEAD at session end; `git status --porcelain src/ include/` empty.

- [s38] KILL RE-AUDIT: rt (the s20/s25 dead round-trip, the banked form whose instruction multiset matches the target's) re-measures 10 at 49; c1 (s33's anon-symdiff block 0, blocks 0/1 byte-exact) re-measures 13 at 49; `tools/fake_ablate.py --func func_80034F88 --file code6cac_b --candidate memory/grind/func_80034F88/candidate.c` again reports no FAKE-annotated construct, so no banked kill on this chassis was measured with a FAKE carrier occupying the contested pseudo.

- [s38] Base-body allocno table from the instrumented cc1 (BB2_ALLOC_DEBUG=1, tools/gcc-2.7.2/cc1; the local-alloc/allocation instrumentation is granted for this function by docs/grind/decisions.md:14784): ord0 pseudo73 hardreg3 nrefs11 livelen7 pri47142 (i); ord1-3 pseudos 78/82/86 hardreg2 nrefs5 livelen7 pri14285 (the three p[8]&mask conditions); ord4 pseudo74 hardreg4 nrefs10 livelen29 pri10344 (q); ord5-7 pseudos 77/81/85 hardreg3 nrefs3 livelen4 pri7500 (the three block values); ord8 pseudo72 hardreg5 nrefs6 livelen34 pri3428 (p).

- [s38] The .greg conflict rows show `;; 73 conflicts: 72 73 2 29` and `;; 74 conflicts: 72 74 77 78 81 82 85 86 2 3 29`: pseudos 73 and 74 do NOT conflict with each other, and only 74 carries the hard-reg-3 ($v1) conflict.

- [s38] In c1's allocno table q does reach hardreg=3 ($v1) and the per-block values move to $a0 -- which is exactly why c1's blocks 0/1 are byte-exact and its blocks 2/3 (which the base gets right) are not. One pseudo, one hard register (global.c:426), so the two seats the target uses are not simultaneously reachable from one C object.

- [s38] Direct-diff result new this session: `diff s38/b0.txt s38/rt.txt` is a single hunk -- our `nop` at +0x4f64 versus `lbu $v1,0($a0)`. The target's block-1 reload is instruction-free (it fills a load-delay slot the base wastes) and differs from the target only in the rotated register.

- [s38] The array-declared, pun-free body measures 10 at 49 with an objdump bit-identical to the punned base (`diff s38/b0.txt s38/a1.txt` empty); `grep -rn D_80106A70 include src` returns exactly two lines (include/code6cac.h:472 and this function's loop), so the declaration fix has no other build-file impact.

- [s38] Ladder accounting: this is session sixteen of cycle 2 (the ledger's own numbering; the dispatch brief called it 'session 35' while the ledger already carried s35 and s37 blocks, so this session is tagged s38 and its scratch is tmp/grind/func_80034F88/s38/). The six-modality condition was met at s31; four flat sessions remain before the owner directive 2026-09-02 permits any disposition.

- [s38] One new disproven form banked (rejected/s38-loop-source-byte-array-indexed-score12.c); bank size is now 203.

==== s39 (synthesis) ====

(Session-numbering note: the dispatch brief again called this "session 35"; the
ledger already carries s35/s37/s38 blocks, so this session is tagged **s39** and
its scratch is `tmp/grind/func_80034F88/s39/`.)

CHASSIS RE-MEASURE. `memory/grind/func_80034F88/candidate.c` (the s38 body)
spliced as variant `b0`: **score 10, build_insns 49**. The dispatch brief printed
"measurement unavailable" for the TENTH consecutive session, so the floor is
measured here, not inherited. `src/code6cac_b.c` and `include/code6cac.h` are
both back at HEAD at session end (`git status --porcelain src/ include/` empty).

MANDATED KILL RE-AUDIT. The instance kill re-measured this session is s37's k1
(`rejected/s37-block0-value-lives-across-block1-score10-BIT-IDENTICAL.c`, the
banked form claimed to make block 0's value live across the block-1 branch):
**10 at 49**, unchanged. But its .greg dump shows the s37/s38 kill NARRATION was
incomplete -- see section 1 -- and re-reading it is what produced this session's
result. `fake_ablate.py` again reports no FAKE-annotated construct in
`candidate.c`, so no banked kill on this chassis was measured with a FAKE carrier
on the contested pseudo.

--- 1. THE HARD-REGISTER CONFLICT ON q IS REMOVABLE, AND IT IS REMOVED ---

The ledger has carried, since s31 and in sharper form since s38, the statement
that the `&D_80106A73` object `q` is INELIGIBLE for $v1 before `find_reg` even
runs, because `.greg` prints a hard-register conflict with reg 3:

    ;; 75 conflicts: 72 74 75 77 79 80 83 84 2 3 29

s38 attributed that hard conflict to local-alloc seating "block 0's masked byte"
in $v1 before global-alloc looks at q, and treated it as a fact of the function
rather than a fact of the SPELLING. This session read the pre-allocation RTL for
three different spellings of block 0, and the attribution is spelling-dependent:

  * `*q &= 0xF8;` (the body every session since s20 has measured) expands to TWO
    pseudos -- a QImode load destination and an SImode AND through a subreg:

        (insn 17 (set (reg:QI 76) (mem:QI (reg/v:SI 75))))
        (insn 19 (set (reg/v:SI 74) (and:SI (subreg:SI (reg:QI 76) 0) (const_int 248))))
        (insn 22 (set (mem:QI (reg/v:SI 75)) (subreg:QI (reg/v:SI 74) 0)))

    Pseudo 76 is referenced in ONE basic block, so it never enters the global
    allocno table at all: local-alloc seats it, and (with $v0 held by the call
    return, and no REG_ALLOC_ORDER defined for MIPS in
    `tools/gcc-2.7.2/config/mips/mips.h`, so the scan is ascending) the lowest
    free hard register is $v1. THAT is the source of q's hard-reg-3 conflict.

  * `m = *q; m = m & 0xF8; *q = m;` -- one named SImode variable, loaded and
    masked in place -- expands to ONE pseudo and a `zero_extend`:

        (insn 14 (set (reg/v:SI 75) (symbol_ref:SI ("D_80106A73"))))
        (insn 18 (set (reg/v:SI 74) (zero_extend:SI (mem:QI (reg/v:SI 75)))))
        (insn 20 (set (reg/v:SI 74) (and:SI (reg/v:SI 74) (const_int 248))))
        (insn 23 (set (mem:QI (reg/v:SI 75)) (subreg:QI (reg/v:SI 74) 0)))

    The entry block now contains NO local-alloc quantity. `.greg` prints

        ;; 75 conflicts: 72 74 75 77 79 80 83 84 2 29

    -- hard reg 3 is GONE from q's conflict row. q is ELIGIBLE for $v1.

Measured, not inferred: `tmp/grind/func_80034F88/s39/v2.greg` and `v4.greg`
versus `k1.greg`. This retires the s31/s38 formulation of the residual.

--- 2. THE RESIDUAL IS NOW A TWO-NUMBER PRIORITY GAP ---

Eligible is not seated. With the round-trip spelling the instrumented cc1
(`tools/gcc-2.7.2/cc1`, BB2_ALLOC_DEBUG=1, granted by decisions.md:14784;
driver `tmp/grind/func_80034F88/s39/ad.sh`) prints:

    ord=0 pseudo=73 hardreg=3 nrefs=11 livelen=7  pri=47142   <- i (loop counter)
    ord=1 pseudo=77 hardreg=2 nrefs=5  livelen=6  pri=16666
    ord=2 pseudo=80 hardreg=2 nrefs=5  livelen=7  pri=14285
    ord=3 pseudo=84 hardreg=2 nrefs=5  livelen=7  pri=14285
    ord=4 pseudo=74 hardreg=3 nrefs=6  livelen=9  pri=13333   <- m  (takes $v1)
    ord=5 pseudo=75 hardreg=4 nrefs=10 livelen=28 pri=10714   <- q  (gets $a0)
    ord=6 pseudo=79 hardreg=3 nrefs=3  livelen=4  pri=7500
    ord=7 pseudo=83 hardreg=3 nrefs=3  livelen=4  pri=7500
    ord=8 pseudo=72 hardreg=5 nrefs=6  livelen=33 pri=3636    <- p

`allocno_compare` (tools/gcc-2.7.2/global.c:635) sorts on
`floor_log2(nrefs) * nrefs / live_length`; m at 13333 is allocated before q at
10714 and takes the seat. The target wants the opposite pairing (address in $v1,
byte value in $a0), so the whole 10-point residual is now the 2619-point gap
between those two numbers. Thresholds that would flip it, from the formula:
q nrefs >= 13 at livelen 28 (13928); or q livelen <= 22 at nrefs 10 (13636); or
m nrefs <= 4 at livelen 9 (8888); or m livelen >= 12 at nrefs 6 (10000).

--- 3. ALL FOUR INPUTS TO THAT GAP MEASURE PINNED FROM C ---

Every dial was pushed on directly this session and each is held by a different
pass:

  (a) q's nrefs is pinned at 10. Adding a redundant `q = &D_80106A73;`
      re-materialisation in block 1 (variant `w1`) leaves the allocno table
      numerically IDENTICAL -- q still nrefs=10 livelen=28 pri=10714 -- because
      cse deletes the redundant set. Score 10 at 49, unchanged. Extra
      re-materialisations therefore cannot buy references; only genuinely new
      uses could, and the candidate new use (q as the trailing copy loop's store
      base) was measured at 30 in s37.
  (b) m's nrefs is pinned at 6 whenever m is global. Interposing a copy
      (`v = m;`, block 1's arms reading v -- variant `z2`) is coalesced away:
      identical table, identical score. Removing block 1's use of m entirely
      (variant `t1`: block 1 reads `D_80106A73` by symbol so cse cannot forward)
      does drop m out of the global table -- but only by making it a
      single-basic-block quantity again, which restores exactly the local-alloc
      $v1 seat and the hard-reg-3 conflict section 1 removed. 11 at 50. That is
      the dichotomy: m consumed by block 1 => global, pri 13333, takes $v1;
      m not consumed by block 1 => local, local-alloc takes $v1.
  (c) m's livelen is pinned at 9 and moves the WRONG way under reordering.
      Computing block 1's condition between m's load and m's mask (variant `x1`,
      the direct attempt at livelen >= 12) gives livelen 7, pri 17142 -- the
      scheduler compacts the range rather than stretching it. 14 at 49. Hoisting
      the whole flag word first (`x3`) is 27 at 48.
  (d) q's livelen is pinned at 28 without dropping q from a block, and the hybrid
      that drops it (q for blocks 0/1, symbol for blocks 2/3) is the
      already-banked `hybrid-h1_ptr01_sym23-score28.c`.

--- 4. THE BIT-IDENTICAL BASIN IS NOW FIFTEEN BODIES ---

`b0` (the s38 candidate), `k1`, `v1`, `v2`, `v3`, `v4`, `w1`, `z2` and the
array-declared `a1` all measure 10 at 49 and disassemble BIT-IDENTICALLY
(`diff s39/b0.txt s39/<n>.txt` empty except the objdump filename line). Added to
s31's seven and s37's h15/k1/k4/k5 this is the widest measured statement of the
basin's rigidity: three structurally distinct spellings of block 0's
read-modify-write, two of block 1's value flow, and the destination-array
declaration change all collapse to the same 49 instructions -- while, per section
1, they do NOT all produce the same conflict graph. Score-identity is not
chassis-identity, and the ledger's practice of ranking bodies only by score is
what hid this for 19 sessions.

--- 5. CANDIDATE.C IS REPLACED (SAME BYTES, BETTER CHASSIS) ---

`memory/grind/func_80034F88/candidate.c` and its pun-free twin
`candidate_arraydecl_pun_free.c` now carry the round-trip spelling of block 0
(`m = *q; m = m & 0xF8; *q = m;`). Both measure 10 at 49 (`a1` verified this
session with `include/code6cac.h:472` temporarily reading
`extern u8 D_80106A70[3];`; disassembly bit-identical to `b0`; header restored).
The body emits byte-for-byte the same output as the previous candidate; the
reason to prefer it is that q is no longer hard-conflicted out of $v1, so the
next session inherits a live arithmetic target instead of a foreclosure.

- [s39] Chassis re-measured on HEAD (dispatch printed 'measurement unavailable' for the tenth consecutive session): candidate body = score 10, build_insns 49, rules_dropped 0. src/ and include/ restored to HEAD; `git status --porcelain src/ include/` empty.
- [s39] KILL RE-AUDIT: s37's k1 re-measures 10 at 49 on today's chassis; fake_ablate reports no FAKE construct in candidate.c. Re-reading k1's .greg is what exposed the s38 mis-attribution below.
- [s39] THE HARD-REG-3 CONFLICT ON q IS A PROPERTY OF THE SPELLING, NOT OF THE FUNCTION. `*q &= 0xF8;` expands to a QImode load pseudo plus an SImode AND-through-subreg; the QImode pseudo is single-basic-block, so local-alloc seats it in $v1 (ascending scan, no REG_ALLOC_ORDER in tools/gcc-2.7.2/config/mips/mips.h, $v0 held by the call return) and q inherits `;; conflicts: ... 2 3 29`. Spelling block 0 as `m = *q; m = m & 0xF8; *q = m;` emits a single `zero_extend:SI (mem:QI)` into one named pseudo, leaves the entry block with NO local quantity, and q's conflict row loses hard reg 3 entirely. Measured in tmp/grind/func_80034F88/s39/{k1,v2,v4}.greg.
- [s39] The residual is therefore no longer an ineligibility but a priority gap in global.c:635 allocno_compare: m nrefs=6 livelen=9 pri=13333 is ordered ahead of q nrefs=10 livelen=28 pri=10714 and takes $v1 first. Flip thresholds from floor_log2(n)*n/livelen: q nrefs>=13, or q livelen<=22, or m nrefs<=4, or m livelen>=12.
- [s39] q's nrefs is pinned at 10: a redundant `q = &D_80106A73;` in block 1 (w1) is deleted by cse and leaves the allocno table numerically identical (10 at 49).
- [s39] m's nrefs is pinned at 6 while m is global: an interposed copy (z2) is coalesced away (identical table); removing block 1's use of m (t1, block 1 reads the symbol so cse cannot forward) drops m from the global table only by making it a single-BB quantity, which restores the local-alloc $v1 seat and the hard-reg-3 conflict. 11 at 50.
- [s39] m's livelen moves the wrong way under statement reordering: computing block 1's condition between the load and the mask (x1) gives livelen 7 / pri 17142, not >=12. 14 at 49. Hoisting the whole flag word (x3) is 27 at 48.
- [s39] The block-1 two-arm select is load-bearing: replacing it with `c = m; if (cond) c = m|1;` (w2) or `c |= 1` (w3) is 13 at 47 -- the branch-around form loses two instructions.
- [s39] Bit-identical-tie count is now FIFTEEN bodies. Score-identity is NOT chassis-identity: bodies with identical bytes have different conflict graphs, which is why 19 sessions of score-only ranking missed section 1.
- [s39] candidate.c and candidate_arraydecl_pun_free.c both REPLACED with the round-trip spelling; both measure 10 at 49 and disassemble bit-identically to the previous candidate.
- [s39] Ladder accounting: this is session SEVENTEEN of cycle 2; the six-modality condition was met at s31. Three flat sessions remain before the owner directive 2026-09-02 permits any disposition.
- [s39] Five new disproven forms banked; bank size is now 208.

### Artifacts (s39)
`tmp/grind/func_80034F88/s39/`: `variants/{b0,k1,v1,v2,v3,v4,w1,w2,w3,x1,x2,x3,z2,t1,a1}.c`,
`run.ps1`, `ad.sh`, `{b0,k1,v1,v2,v3,v4,w1,x1,a1}.txt` (objdumps), `{k1,v2,v4}.greg`,
`{v2,v4}.lreg`, `code6cac_b.c.orig`.

- [s39] Chassis re-measured on HEAD this session (the dispatch brief printed 'measurement unavailable' for the tenth consecutive session): the candidate body = score 10, build_insns 49, rules_dropped 0. src/code6cac_b.c and include/code6cac.h are both restored to HEAD at session end; `git status --porcelain src/ include/` is empty.

- [s39] The ledger's standing statement since s31 -- that allocno q is INELIGIBLE for $v1 before find_reg runs -- is spelling-dependent and is removed by an ordinary-C change: `*q &= 0xF8;` expands to a QImode load pseudo plus an SImode AND through a subreg, and the QImode pseudo is single-basic-block so local-alloc seats it in $v1; `m = *q; m = m & 0xF8; *q = m;` emits one zero_extend:SI (mem:QI) into one named pseudo and leaves the entry block with no local quantity. Measured in tmp/grind/func_80034F88/s39/{k1,v2,v4}.greg.

- [s39] The reason local-alloc picks $v1 specifically: tools/gcc-2.7.2/config/mips/mips.h defines no REG_ALLOC_ORDER, so the free-register scan is ascending, and $v0 is unavailable because the call return is still live (the `a1 = v0` copy is scheduled after block 0's store).

- [s39] On the new chassis the entire 10-point residual is a priority gap in global.c:635 allocno_compare (floor_log2(nrefs) * nrefs / live_length): m nrefs=6 livelen=9 pri=13333 is allocated before q nrefs=10 livelen=28 pri=10714 and takes $v1. Full table in evidence.md; source tmp/grind/func_80034F88/s39/ad.sh (instrumented cc1, BB2_ALLOC_DEBUG=1, granted by docs/grind/decisions.md:14784).

- [s39] The four numeric ways the formula allows the flip: q nrefs >= 13 at livelen 28 (13928); q livelen <= 22 at nrefs 10 (13636); m nrefs <= 4 at livelen 9 (8888); m livelen >= 12 at nrefs 6 (10000). Each was probed this session and each is held by a different pass -- cse deletes redundant q re-materialisations (w1), copy coalescing absorbs interposed copies (z2), local-alloc reclaims m the moment block 1 stops consuming it (t1), and the first scheduling pass compacts rather than stretches m's range (x1).

- [s39] Score-identity is not chassis-identity. Fifteen bodies now measure 10 at 49 with bit-identical disassembly (s31's seven, s37's h15/k1/k4/k5, and s39's v1/v2/v3/v4/w1/z2/a1), yet they do NOT all produce the same conflict graph -- b0/k1 carry the hard-reg-3 conflict and v2/v4 do not. Ranking bodies by score alone is what hid this for 19 sessions.

- [s39] candidate.c and candidate_arraydecl_pun_free.c are both REPLACED with the round-trip spelling. Both measure 10 at 49. The pun-free twin was verified this session with include/code6cac.h:472 temporarily reading `extern u8 D_80106A70[3];` and its objdump is bit-identical to the punned body; the header was restored immediately afterwards.

- [s39] Ladder accounting: this is session seventeen of cycle 2 (the ledger's own numbering; the dispatch brief again said 'session 35' while the ledger already carried s35/s37/s38, so this session is tagged s39). The six-modality condition was met at s31; three flat sessions remain before owner directive 2026-09-02 permits any disposition.

- [s39] Five new disproven forms banked in memory/grind/func_80034F88/rejected/; bank size is now 208.

==== s40 (synthesis) ====

(Session-numbering note: the dispatch brief again called this "session 35"; the
ledger already carries s35/s37/s38/s39 blocks, so this session is tagged **s40**
and its scratch is `tmp/grind/func_80034F88/s40/`.)

CHASSIS RE-MEASURE. `memory/grind/func_80034F88/candidate.c` (the s39 round-trip
body) spliced as variant `b0`: **score 10, build_insns 49**. The dispatch brief
printed "measurement unavailable" for the ELEVENTH consecutive session, so the
floor is measured here, not inherited. `src/code6cac_b.c` is restored to HEAD at
session end (`git status --porcelain src/ include/` empty).

MANDATED KILL RE-AUDIT. The instance kill re-measured this session is s39's `t1`
(`rejected/s39-block1-reads-symbol-directly-score11.c`) -- the banked form that is
STRUCTURALLY closest to the target, because it is the only one whose block 1
reloads the flag byte from memory the way the target's `lbu $a0,0($v1)` at
80034FB4 does. It re-measures **11 at 50**, unchanged from s39.
`fake_ablate.py` again reports no FAKE-annotated construct in `candidate.c`, so
no banked kill on this chassis was measured with a FAKE carrier on the contested
pseudo.

--- 1. THE S39 FRONTIER IS ARITHMETICALLY INCOMPLETE: FLIPPING q INTO $v1
    CANNOT REACH ZERO ---

s39 restated the whole residual as "make pri(q) > pri(m) in
`global.c:635 allocno_compare`, and q takes $v1". That is a correct description
of why blocks 0/1 are register-swapped, but it is NOT a route to a byte match,
and this session's first job was to read the target's register geometry per
block instead of per function. Read straight off `asm/funcs/func_80034F88.s`:

    block 0   lui/addiu $v1 = &D_80106A73 ; lbu $a0,0($v1) ; andi $a0 ; sb $a0,0($v1)
    block 1   lbu $a0,0($v1) ; ... ; sb $v0,0($v1)          <- SAME address register
    block 2   lui/addiu $a0 = &D_80106A73 ; lbu $v1,0($a0) ; sb $v0,0($a0)
    block 3   lui/addiu $a0 = &D_80106A73 ; lbu $v1,0($a0) ; sb $v0,0($a0)

The target therefore holds `&D_80106A73` in **two different general registers**:
`$v1` for blocks 0-1 and `$a0` for blocks 2-3. They are simultaneously live: at
`.L80034FC8` the block-2 materialisation (`lui $a0` / `addiu $a0`, 80034FC8 and
80034FCC) is scheduled ABOVE block 1's store `sb $v0,0($v1)` (80034FD0), so both
registers hold the address across those three instructions.

GCC 2.7.2 gives one pseudo exactly one hard register
(`tools/gcc-2.7.2/global.c:1275`, `reg_renumber[allocno_reg[allocno]] = best_reg;`;
one allocno per pseudo at `global.c:426`) and has no live-range splitting. A
single C pointer object is a single pseudo, so on the single-object chassis the
address occupies ONE register for all four blocks. Our build puts it in `$a0`,
which is right for blocks 2/3 and wrong for blocks 0/1 -- exactly the 10-point
residual. Winning the priority fight s39 described would put it in `$v1`, which
is right for blocks 0/1 and wrong for blocks 2/3: the same two register-swapped
blocks, relocated, not removed. The single-object chassis has a FLOOR, and 10 is
it. s39's frontier items 1 and 2 are retired as routes to zero (they remain
correct descriptions of the allocator's behaviour).

Direct corroboration, measured this session rather than inferred: variant `qL`
(round-trip chassis, trailing copy loop addressed off `q[i-3]` to buy q
references) moves the allocno table exactly the way s39's formula predicts --
q rises to `nrefs=12 livelen=36 pri=10000` and displaces the loop counter
(pseudo 73) from `$v1` to `$a0` -- yet the score goes UP to **30 at 48**, and the
objdump shows q seated in `$a1`, with blocks 2/3 now materialising `$a1` instead
of `$a0`. Table in `tmp/grind/func_80034F88/s40/ad_qL.txt`.

--- 2. AN ADDRESS ONLY LANDS IN AN ALLOCATABLE REGISTER IF THE C SOURCE NAMES A
    POINTER OBJECT ---

The obvious repair to section 1 is to let q cover blocks 0/1 only and let blocks
2/3 get their own address WITHOUT a second declared pointer (which the standing
Judge constraint forbids). Two spellings were measured on the round-trip
chassis:

  * `a1` -- blocks 2/3 through the anonymous symbol-difference expression
    `*(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70))`:
    **28 at 48**.
  * `a2` -- blocks 2/3 through the bare symbol (`v = D_80106A73; ... D_80106A73 = c;`):
    **28 at 48**, and byte-for-byte the same stream as `a1`, which proves the
    symbol-difference expression constant-folds to the plain `symbol_ref` before
    address selection.

Their disassembly (`s40/a1.txt`) is the point: blocks 2/3 emit

    lui $v0,%hi ; lbu $v0,%lo($v0)      and      lui $at,%hi ; sb $v1,%lo($at)

-- four instructions per block, the same COUNT as the target's
lui/addiu/lbu/sb, but with the address folded into each memory operand and
expanded through the assembler temporary `$at`, never held in an allocatable
register. That is the general behaviour, not an accident of this spelling: a
`symbol_ref` is a legitimate MIPS address, so a mem whose address is a bare
symbol never forces the address into a pseudo; the address becomes a
register-held VALUE (the `lui`+`addiu` pair) only when the C source uses
`&D_80106A73` as a value -- i.e. assigns it to a pointer object. The target does
that twice.

`h1` closes the last non-declaration route: blocks 2/3 replaced by two expansions
of a `static inline` helper that declares its OWN `u8 *q` (so the caller still
declares only one pointer object). **30 at 49.** Inlining fires (no call in the
`.o`), but the two expansions SHARE one store base -- and it is `$a1`, the same
register the caller's q gets -- while both reads fold back to bare `lui`+%lo; the
block shape also degrades to `beqz`+`nop`. Inlining does not buy a second
register-held address. This reproduces s37's result on the new chassis, where it
had been measured only on the retired `*q &= 0xF8` one.

--- 3. WHAT THE RESIDUAL ACTUALLY IS, RESTATED ---

Not a priority gap and not an ineligibility: a PSEUDO-COUNT requirement. The
target's byte stream requires two pseudos holding `&D_80106A73`, live
simultaneously at `.L80034FC8`; every measured ordinary-C way to create the
second one either (a) declares a second pointer object -- the score-0 body
(`rejected/three-pointer-objects-judge-FAIL-score0.c`), which the Judge has ruled
a cheat and the standing constraint bans -- or (b) produces no register-held
address at all (a1/a2, and the whole anonymous-carrier family measured in
s33/s34/s35), or (c) costs instructions (s33 c1 at 13, s34's +1-insn carriers).
The 10-point floor of the single-object chassis is therefore a property of the
chassis, and the ladder's remaining question is not "which C lever moves the
allocator" but "does any sanctioned family supply a second address pseudo".

- [s40] Chassis re-measured on HEAD (dispatch printed 'measurement unavailable' for the eleventh consecutive session): candidate body b0 = score 10, build_insns 49. src/ restored to HEAD; `git status --porcelain src/ include/` empty.
- [s40] KILL RE-AUDIT: s39's t1 (`rejected/s39-block1-reads-symbol-directly-score11.c`, the only banked form whose block 1 reloads the byte from memory like the target's 80034FB4 `lbu $a0,0($v1)`) re-measures 11 at 50, unchanged. fake_ablate reports no FAKE construct in candidate.c.
- [s40] READ OFF THE TARGET ASM: `&D_80106A73` lives in TWO general registers -- $v1 across blocks 0-1 (lui/addiu at 80034F98/80034F9C) and $a0 across blocks 2-3 (lui/addiu at 80034FC8/80034FCC and 80034FF0/80034FF4) -- and they are simultaneously live, because the block-2 materialisation is scheduled above block 1's store `sb $v0,0($v1)` at 80034FD0.
- [s40] Therefore the s39 frontier is not a route to zero: GCC 2.7.2 assigns one hard register per pseudo (global.c:1275) with no live-range splitting, so a single C pointer object supplies ONE address register for all four blocks. Our build puts it in $a0 (right for blocks 2/3, wrong for 0/1); flipping it to $v1 would be right for 0/1 and wrong for 2/3. Same two swapped blocks, relocated.
- [s40] Measured corroboration: variant qL (loop addressed off q[i-3]) moves the table exactly as s39's formula predicts (q nrefs 12, livelen 36, pri 10000; loop counter displaced from $v1 to $a0) and the score RISES to 30 at 48, with q seated in $a1.
- [s40] a1 (blocks 2/3 anonymous symbol-difference) and a2 (blocks 2/3 bare symbol) both measure 28 at 48 and emit an IDENTICAL stream: the symbol-difference expression constant-folds to the plain symbol_ref. Both spell blocks 2/3 as `lui $v0/$at` + %lo-folded lbu/sb -- four insns per block, the same count as the target's lui/addiu/lbu/sb, but with no address in an allocatable register.
- [s40] Generalisation supported by a1/a2 and by the whole s33-s35 anonymous-carrier family: a bare symbol_ref is a legitimate MIPS address, so a mem addressed by it never forces the address into a pseudo; `&D_80106A73` becomes a register-held value (lui+addiu) only where the C source uses it AS a value, i.e. assigns it to a pointer object.
- [s40] h1 (blocks 2/3 as two expansions of a static inline helper declaring its own `u8 *q`) = 30 at 49 on the round-trip chassis: inlining fires but the two expansions share ONE store base, and it is $a1 -- the same register the caller's q takes -- while both reads fold to bare lui+%lo. Inlining does not buy a second register-held address. Reproduces s37's old-chassis result on the new chassis.
- [s40] RESTATEMENT: the residual is a pseudo-COUNT requirement (two address pseudos live at .L80034FC8), not a priority gap and not an ineligibility. 10 is the floor of the single-object chassis, not a plateau within it.
- [s40] Ladder accounting: session EIGHTEEN of cycle 2; the six-modality condition was met at s31. Two flat sessions remain before owner directive 2026-09-02 permits any disposition.
- [s40] Four new disproven forms banked; bank size is now 212.

### Artifacts (s40)
`tmp/grind/func_80034F88/s40/`: `variants/{b0,t1,a1,a2,qL,h1}.c`, `run.ps1`, `dis.sh`,
`ad.sh`, `{b0,t1,a1,qL,h1}.txt` (objdumps), `ad_qL.txt` (instrumented-cc1 allocno table
for qL), `code6cac_b.c.orig`.

- [s40] [s40] Chassis re-measured on HEAD (the dispatch brief printed 'measurement unavailable' for the eleventh consecutive session): the candidate body b0 = score 10, build_insns 49. src/code6cac_b.c restored to HEAD at session end; `git status --porcelain src/ include/` empty.

- [s40] [s40] Target register geometry, read block by block off asm/funcs/func_80034F88.s: blocks 0-1 address D_80106A73 through $v1, blocks 2-3 through $a0, and both registers hold the address simultaneously at .L80034FC8 because the block-2 lui/addiu pair is scheduled above block 1's store `sb $v0,0($v1)` (80034FD0).

- [s40] [s40] The residual is a PSEUDO-COUNT requirement, not a priority gap (s39) and not an ineligibility (s31/s38): two pseudos must hold &D_80106A73 simultaneously, and GCC 2.7.2 gives one allocno per pseudo (global.c:426) exactly one hard register (global.c:1275) with no live-range splitting.

- [s40] [s40] Measured corroboration that the priority route cannot reach zero: variant qL (loop addressed off q[i-3]) moves the allocno table exactly as s39's formula predicts (q nrefs 12, livelen 36, pri 10000; loop counter displaced from $v1 to $a0) and the score RISES to 30 at 48, with q seated in $a1 and blocks 2/3 materialising $a1.

- [s40] [s40] a1 (blocks 2/3 anonymous symbol-difference) and a2 (blocks 2/3 bare symbol) both measure 28 at 48 and emit an IDENTICAL stream: the symbol-difference expression constant-folds to the plain symbol_ref. Both spell blocks 2/3 as lui + %lo-folded lbu/sb through $at/$v0 -- four insns per block, the same count as the target's lui/addiu/lbu/sb, but with no address in an allocatable register.

- [s40] [s40] Generalisation supported by a1/a2 and by the s33-s35 anonymous-carrier family: a bare symbol_ref is a legitimate MIPS address, so a mem addressed by it never forces the address into a pseudo; &D_80106A73 becomes a register-held value (lui+addiu) only where the C source uses it AS a value, i.e. assigns it to a pointer object. The target does that twice.

- [s40] [s40] h1 (blocks 2/3 as two expansions of a static inline helper declaring its own pointer) = 30 at 49 on the round-trip chassis: inlining fires but the two expansions share ONE store base -- $a1, the same register the caller's q takes -- and both reads fold to bare lui+%lo.

- [s40] [s40] Kill re-audit: s39's t1 re-measures 11 at 50, unchanged; fake_ablate.py finds no FAKE-annotated construct in candidate.c.

- [s40] [s40] candidate.c body is UNCHANGED (still the s39 round-trip spelling at 10/49); only its header comment was updated with the two-address-register finding. Four new disproven forms banked; bank size is now 212.

- [s40] [s40] Ladder accounting: this is session EIGHTEEN of cycle 2 (the ledger's own numbering -- the dispatch brief again said 'session 35' while the ledger already carried s35/s37/s38/s39). The six-modality condition was met at s31; two flat sessions remain before owner directive 2026-09-02 permits any disposition.

==== s41 (synthesis) ====

(Session-numbering note: the dispatch brief again called this "session 35" and its
floor history stopped at s34, while the ledger already carried s35/s37/s38/s39/s40.
This session is tagged **s41**; scratch is `tmp/grind/func_80034F88/s41/`. The
digest is stale by six sessions - a reader who trusts the brief's "Live frontier"
block will re-derive s37-s40. Read the ledger tail first.)

CHASSIS RE-MEASURE. `memory/grind/func_80034F88/candidate.c`, regenerated as
variant `b0` and spliced at `src/code6cac_b.c:3420`: **score 10, build_insns 49**.
The dispatch brief printed "measurement unavailable" for the TWELFTH consecutive
session, so the floor is measured here, not inherited. `src/` restored to HEAD at
session end (`git status --porcelain src/ include/` empty).

MANDATED KILL RE-AUDIT (both closest-to-target banked forms, re-measured today):
  * `rejected/s39-block1-reads-symbol-directly-score11.c` (t1 - the only banked
    form whose block 1 reloads the flag byte from memory the way the target's
    `lbu $a0,0($v1)` at 80034FB4 does): **11 at 50**, unchanged from s39/s40.
  * `rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c` (rt -
    the only banked body whose instruction multiset matches the target's):
    **10 at 49**, unchanged.
  * `tools/fake_ablate.py` again reports "no FAKE-annotated constructs found ...
    nothing to ablate" for candidate.c, so no banked kill on this chassis was
    measured with a FAKE carrier occupying the contested pseudo.

--- 1. S40's FRONTIER 1 IS CLOSED BY MEASUREMENT: THE p-DERIVED ADDRESS CANCELS ---

s40 left exactly one prescribed probe: "an address whose displacement or base
comes from a runtime quantity that provably equals a constant (e.g. derived from
`p`)", to be judged on INSTRUCTION COUNT first. Three bodies were built on the
candidate chassis around

    #define PDER ((u8 *)((s32)p + ((s32)&D_80106A73 - (s32)p)))

  * `n1` - blocks 2/3 addressed through `PDER`, `q` keeps blocks 0/1:
    **28 at 48**, and its objdump is **byte-identical to s40's `a1`/`a2`**
    (the symbol-difference and bare-symbol bodies). The runtime base buys
    nothing: the address is folded back to a bare `symbol_ref` and expanded as
    `lui $v0` + `%lo`-folded `lbu`, `lui $at` + `%lo`-folded `sb`.
  * `n2` - the single declared `q` RE-ASSIGNED from `PDER` before blocks 2 and 3
    (one C object, non-constant-looking re-set): **10 at 49**, and its objdump is
    **byte-identical to the base body `b0`**. The re-set is cancelled to the
    constant and then deleted as redundant; it does not even cost an instruction.
  * `n3` - the mirror (blocks 0/1 through `PDER`, `q` on blocks 2/3):
    **16 at 50**.

PASS ATTRIBUTION FROM DUMPS, not inference (`pwsh tools/grinder/dump.ps1` with n1
installed; dumps in `tmp/grind/func_80034F88/dumps/`). The expression is NOT
folded at the tree level: `.rtl` still contains, per access,

    (set (reg 82) (symbol_ref "D_80106A73"))   REG_EQUAL symbol_ref
    (set (reg 83) (plus (reg/v 72 = p) (reg 82)))
    (set (reg 84) (minus (reg 83) (reg/v 72)))
    (set (reg:QI 85) (mem:QI (reg 84)))

i.e. tree reassociation turns `p + (SYM - p)` into `(p + SYM) - p`, and the
address really is a runtime value at expand. Counting `(minus` in the function's
section across passes: `.rtl` 4, `.jump` 4, `.cse` 3, `.loop` 3, `.cse2` 3,
`.combine` **0**, `.greg` 0 - the cancellation completes in **combine**, which
reaches `simplify_binary_operation` -> `simplify_plus_minus`
(`tools/gcc-2.7.2/cse.c:4250`), the brute-force PLUS/MINUS reassociator that
cancels the `+p` against the `-p` and leaves the bare symbol. That is a general
property of ADDITIVE spellings, not of this one: any chain of `+`/`-` whose terms
cancel to `&D_80106A73` is reassembled by the same routine.

Consequence for the frontier: the class of "addresses forced into a pseudo
because a runtime quantity makes them non-constant" is empty for additive
spellings. Combined with s40's a1/a2 (bare symbol, symbol difference), s40's h1
(static inline helper with its own pointer), and the whole s33/s34/s35
anonymous-carrier family, **every measured way of naming the flag byte's address
that is not an assignment to a declared C pointer object either folds to a bare
`symbol_ref` (no register-held address at all) or costs an instruction.**

--- 2. WHAT THE RESIDUAL IS, IN ONE PARAGRAPH (for the record) ---

Read off `asm/funcs/func_80034F88.s`, the target materialises `&D_80106A73` into
an allocatable register THREE times - `lui/addiu $v1` at 80034F98 (blocks 0-1),
`lui/addiu $a0` at 80034FC8 (block 2), `lui/addiu $a0` at 80034FF0 (block 3) -
with the $v1 and $a0 ranges simultaneously live at `.L80034FC8`. GCC 2.7.2 gives
one allocno per pseudo (`global.c:426`) exactly one hard register
(`global.c:1275`) and has no live-range splitting, so a single C pointer object
cannot supply two of them (s40); a re-assignment of that one object to the same
constant is deleted by cse (s25, and n2 again this session); and nothing but an
assignment of `&D_80106A73` to a pointer object emits the `lui`+`addiu` pair at
all (s40 a1/a2, s41 n1). The target byte stream therefore implies the original C
named the flag-byte address more than once. That construct - a second/third
`u8 *q = &D_80106A73;` - is exactly what the standing Judge constraint bans, and
it is what the score-0 body (`rejected/three-pointer-objects-judge-FAIL-score0.c`)
spells. The residual is an EMISSION requirement, not a register-allocation lever;
that distinction is what this session escalates as a ruling question.

- [s41] Chassis re-measured on HEAD (dispatch printed 'measurement unavailable' for the twelfth consecutive session): candidate body b0 = score 10, build_insns 49. src/ restored to HEAD; `git status --porcelain src/ include/` empty.
- [s41] KILL RE-AUDIT: t1 (`rejected/s39-block1-reads-symbol-directly-score11.c`) re-measures 11 at 50 and rt (`rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c`) re-measures 10 at 49, both unchanged; fake_ablate reports no FAKE construct in candidate.c.
- [s41] n1 (blocks 2/3 addressed through the p-derived expression `(u8 *)((s32)p + ((s32)&D_80106A73 - (s32)p))`) = 28 at 48 and is BYTE-IDENTICAL to s40's a1/a2, so a runtime base does not keep the address out of the folder.
- [s41] n2 (the single declared q re-assigned from that same p-derived expression before blocks 2 and 3) = 10 at 49 and is BYTE-IDENTICAL to the base body b0: the re-set cancels to the constant and is then deleted as redundant, costing nothing and buying nothing.
- [s41] n3 (mirror: blocks 0/1 p-derived, q on blocks 2/3) = 16 at 50.
- [s41] PASS ATTRIBUTION FROM DUMPS: the p-derived address is NOT folded at the tree level -- .rtl carries `(set (reg 83) (plus p sym))` + `(set (reg 84) (minus (reg 83) p))` per access, i.e. tree reassociation produced `(p + SYM) - p`. `(minus` counts by pass in the function's section: .rtl 4, .jump 4, .cse 3, .loop 3, .cse2 3, .combine 0, .greg 0 -- the cancellation completes in COMBINE via simplify_binary_operation -> simplify_plus_minus (tools/gcc-2.7.2/cse.c:4250), the brute-force PLUS/MINUS reassociator.
- [s41] Therefore s40's Frontier 1 is closed for additive spellings: no ordinary-C additive expression of the flag byte's address survives to become a second address pseudo, whatever runtime quantity it is written in terms of.
- [s41] Residual restated for the ruling record: the target materialises &D_80106A73 into an allocatable register THREE times (80034F98 $v1, 80034FC8 $a0, 80034FF0 $a0); only an assignment of the address to a declared C pointer object emits that lui+addiu pair; one object gives one register (global.c:426/1275) and its redundant re-sets are deleted by cse. The residual is an EMISSION requirement, not an RA lever.
- [s41] Ladder accounting: session NINETEEN of cycle 2; the six-modality condition was met at s31. One flat session remains before owner directive 2026-09-02 permits any disposition.
- [s41] Three new disproven forms banked; bank size is now 215. candidate.c body unchanged at 10/49.
- [s41] DIGEST DRIFT worth inheriting: the dispatch brief has now been six sessions stale twice running (it printed 'session 35' and a s34-era frontier). Sessions must read the ledger tail (`grep -n '^==== s' evidence.md | tail`) before spending a probe on a brief-listed 'next probe'.

### Artifacts (s41)
`tmp/grind/func_80034F88/s41/`: `variants/{b0,t1,rt,n1,n2,n3}.c`, `run.ps1`, `dis.sh`,
`{b0,n1,n2,n3}.txt` (objdumps), `{b0,t1,rt,n1,n2,n3}.o`, `code6cac_b.c.orig`;
`tmp/grind/func_80034F88/dumps/` (cc1 -da pass dumps for the n1 body).

==== s42 (synthesis) ====

MERGED ATTACK AFTER s42 -- this is the terminal statement of the function's
search space, written to be read WITHOUT the preceding 41 sessions.

Chassis measured this session (`sandbox func_80034F88 --disable all`, target 49
insns): b0 (memory/grind/func_80034F88/candidate.c, the s39 round-trip body)
= **10 at 49**, rules_dropped 0.  The dispatch digest reported "measurement
unavailable"; the ledger floor of 10 is re-confirmed by direct measurement.
`tools/fake_ablate.py` reports "no FAKE-annotated constructs found ... nothing
to ablate" -- the floor body is ordinary C carrying no coercion of any kind.

THE RESIDUAL, STATED ONCE AND COMPLETELY.
Read off asm/funcs/func_80034F88.s block by block, the target materialises
`&D_80106A73` into an allocatable register THREE times:
  80034F98/9C  lui/addiu $v1   -- serves block 0 (the mask) and block 1
  80034FC8/CC  lui/addiu $a0   -- serves block 2, and is scheduled ABOVE block
                                  1's `sb $v0,0($v1)` at 80034FD0, so $v1 and
                                  $a0 hold the same address simultaneously
  80034FF0/F4  lui/addiu $a0   -- serves block 3 (a SECOND materialisation into
                                  the same hard register, not a reuse)
The floor body has ONE address pseudo (`q`), seated in $a0.  That seat is right
for blocks 2/3 -- which are register-exact -- and wrong for blocks 0/1, where
the target wants the address in $v1 and the byte value in $a0.  The whole
10-point residual is that $v1<->$a0 rotation plus a nop/lbu swap in an
already-paid-for load-delay slot (s37, priced at zero).

WHY ONE OBJECT CANNOT SUPPLY BOTH SEATS: one pseudo is one allocno
(tools/gcc-2.7.2/global.c:426) and one allocno gets exactly one hard register
(global.c:1275); GCC 2.7.2 has no live-range splitting.  s39 removed the
LOCAL-alloc hard-reg-3 conflict that used to make `q` ineligible for $v1 (the
round-trip mask spelling), leaving a pure `allocno_compare` priority gap
(global.c:635, key = floor_log2(n_refs)*n_refs*size/live_length): m 6 refs /
9 live / pri 13333 versus q 10 refs / 28 live / pri 10714.  Both sides of that
gap have now been attacked to exhaustion (s37 k1-k5, s39's five bodies, s42
v1-v3 below): every parameter change that is byte-neutral leaves the ORDER
unchanged, and every parameter change that moves the order changes the
instruction stream for more than it buys.  And even winning $v1 for `q` only
TRADES the two seats (s32/s35), because one register cannot be two.

MEASURED THIS SESSION (all `sandbox func_80034F88 --disable all`):
  b0  the floor body (round-trip chassis)                        10 at 49
  v1  RE-AUDIT of s37 h12/h13 on the post-s39 chassis: trailing
      copy loop fed off `q` (`*(q - 3 + i) = ...`)               30 at 48
  v2  value CHAINING: every flag block consumes the previous
      block's computed value instead of re-reading the byte,
      spelled with two alternating named value locals            24 at 45
  v3  the same chain over blocks 0-2 only, block 3 re-reading    24 at 47
  rt  RE-AUDIT rejected/roundtrip-fresh-pseudo-target-census-
      score10-DEAD-ARITH.c (the only banked body whose
      instruction MULTISET matches the target's)                 10 at 49
  t1  RE-AUDIT rejected/s39-block1-reads-symbol-directly-
      score11.c (the only banked body whose block 1 reloads the
      flag byte the way the target's `lbu $a0,0($v1)` does)      11 at 50
rt and t1 are bit-for-bit the values the ledger banked; the mandated kill
re-audit therefore finds NO verdict that was measured on a stale chassis or
with a FAKE carrier occupying the contested pseudo.

WHAT v1-v3 ADD TO THE MAP.  v1 closes the last chassis-relative doubt about the
reference-count side of allocno_compare: the ONLY free source of extra `q`
references in this function is the trailing copy loop, and consuming it costs
the loop its own `lui %hi(D_80106A70) / addu $at,$at,$v1 / sb %lo(D_80106A70)($at)`
address form -- three instructions the target contains -- for a priority lift
worth at most one register seat.  v2/v3 close the live-LENGTH side from the
value's end: lengthening the value allocnos by chaining block N's result into
block N+1 does lengthen them, but it also deletes the target's `lbu` reloads in
blocks 2/3, because there is then no memory read left for cse to forward FROM.
The target reads the flag byte four times (80034FA0, 80034FB4, 80034FD8,
80034FFC) and is 49 instructions; the chained bodies have two or zero reloads
and 45-47 instructions.  Emission cost dominates every allocation lever on this
function -- that is the single sentence this session adds to the map.

THE AXIS THAT WOULD CLOSE IT, AND WHY IT IS SHUT.  Exactly one C construct has
ever been measured to emit the target's second and third lui+addiu
materialisations: a further `u8 *q = &D_80106A73;` pointer object (the score-0
body, rejected/three-pointer-objects-judge-FAIL-score0.c).  Every non-pointer
spelling measured over s33-s41 -- bare symbol, symbol-difference, p-derived
additive arithmetic (cancelled in combine via simplify_plus_minus,
tools/gcc-2.7.2/cse.c:4250), static-inline helpers, anonymous carriers, one-
and two-slot aggregates, `&D_80106A70[3]` array-model addressing -- either
folds to a bare symbol_ref and is expanded through `$at` with %lo folded into
the memory operand, or costs instructions.  The Judge closed that axis twice:
2026-09-04 15:09 (docs/grind/decisions.md:22578) and again 2026-09-05 06:11
(docs/grind/decisions.md:23322), the second time answering s41's
emission-vs-RA reframing directly -- "that ruling's ground was the C TEXT, not
the pass name ... this ledger's own s22/s24 SOTN-master census returns ZERO
instances, so it is a family extension beyond pointer-alias-fake-exception =
FAIL(CONSTRUCT), never an escalate" -- and directing the record: "file it; do
not re-open this axis."

DISPOSITION FILED THIS SESSION.  s41 was session nineteen of cycle 2; s42 is
session TWENTY, and the modality condition (>= 6 distinct modalities) was met at
s31, so the owner's 2026-09-02 directive is satisfied.  The honest floor is 10,
above ENDGAME_LOCK_MAX_FLOOR = 5, so the 2026-07-27 standing ruling is not this
function's subject and the record is titled LADDER EXHAUSTED (non-endgame
residual, floor 10).  Filed in docs/grind/decisions.md this session; outcome
returned owner-gated.  candidate.c is unchanged (10 at 49) and src/ is back at
HEAD carrying `INCLUDE_ASM("asm/funcs", func_80034F88);`.

==== s43 (synthesis) -- the seat condition is a two-clause conjunction, and both clauses are now measured from both sides ====

Chassis re-measured first: `memory/grind/func_80034F88/candidate.c` (the s39
round-trip body) spliced into src/code6cac_b.c scores **10 at 49 insns**,
rules_dropped 0, on HEAD 2026-09-05. The ALLOCDBG allocno table
(`wsl bash tmp/grind/func_80034F88/s39/ad.sh`, BB2_ALLOC_DEBUG=1) reproduces
s39's numbers exactly:

    ord=0 pseudo=73 (i)  hardreg=3 nrefs=11 livelen=7  pri=47142
    ord=1 pseudo=77      hardreg=2 nrefs=5  livelen=6  pri=16666
    ord=2 pseudo=80      hardreg=2 nrefs=5  livelen=7  pri=14285
    ord=3 pseudo=84      hardreg=2 nrefs=5  livelen=7  pri=14285
    ord=4 pseudo=74 (m)  hardreg=3 nrefs=6  livelen=9  pri=13333
    ord=5 pseudo=75 (q)  hardreg=4 nrefs=10 livelen=28 pri=10714
    ord=6 pseudo=79      hardreg=3 nrefs=3  livelen=4  pri=7500
    ord=7 pseudo=83      hardreg=3 nrefs=3  livelen=4  pri=7500
    ord=8 pseudo=72 (p)  hardreg=5 nrefs=6  livelen=33 pri=3636

The target's seats are q in $v1 (blocks 0/1, sharing $v1 with the loop
induction variable `i`, which our body also puts in $v1) and the block-0 flag
value in $a0. Our body has them swapped. `i` and `q` do NOT conflict in either
body (73 is absent from 75's conflict row), so the target's sharing of $v1
between the address and the induction variable is reachable; the only question
is which of `m` and `q` reaches $v1 first.

**The seat condition, stated as a conjunction.** q takes $v1 iff
  (a) q's .greg conflict row contains no hard reg 3, AND
  (b) q precedes the block-0 value in `allocno_compare`
      (tools/gcc-2.7.2/global.c:635, key = floor_log2(n_refs)*n_refs*size/live_length).
The base body satisfies (a) -- its row is `72 74 75 77 79 80 83 84 2 29`, hard
regs 2 and 29 only -- and fails (b) by 2619 points.

**Both clauses measured from both sides this session (seven bodies).**

| body | block-0 spelling | score/insns | allocate order | q's hard-reg conflicts | q's seat |
|---|---|---|---|---|---|
| b0 | `m = *q; m = m & 0xF8; *q = m;` (base) | 10 / 49 | 73 77 80 84 **74 75** 79 83 72 | 2 29 | $a0 |
| d1 | b0 + redundant `q = &D_80106A73;` in block 1 | 10 / 49 | identical to b0 | 2 29 | $a0 |
| m1 | `m = *q & 0xF8; *q = m;` | 10 / 49 | 73 78 81 85 **75 74** 80 84 72 | 2 **3** 29 | $a0 |
| m4 | `m = *q; v0 = m & 0xF8; *q = v0;` | 10 / 49 | 73 78 81 85 **76 75** 80 84 72 | 2 **3** 29 | $a0 |
| m3 | raw byte named, mask recomputed in both block-1 arms | 17 / 49 | 73 80 84 88 **75 74** 83 87 72 | 2 **3** 29 | $a0 (p displaced to $a2) |
| m5 | masked value named inside block 1's scope | 27 / 52 | -- | -- | -- |
| m2 | block-1 store duplicated into the arms, no named c | 12 / 46 | -- | -- | -- |
| e1 | b0 + block-1 condition hoisted above `q = &...` | 13 / 49 | 73 79 81 85 **74 76** 80 84 72 | 2 29 | $a0 |

Reading of the table:

1. **Clause (b) is winnable and the win is worthless on its own.** m1 and m4
   both cut the block-0 value's reference count and both flip the sort so that
   q precedes it -- and both still score 10 with q in $a0, because cutting the
   count requires splitting block 0 into two pseudos, and whichever pseudo is
   confined to block 0 is a local-alloc quantity. local_alloc runs before
   global_alloc and takes the first free hard register in allocation order;
   $v0 is occupied across block 0 by the call's return value (the v0 -> $a1
   copy is scheduled AFTER block 0's read-modify-write in every body measured,
   including the target, whose copy sits at 0x80034FA4), so the quantity takes
   $v1. It shows up in .greg as a pseudo dispositioned `in 3` that is absent
   from the "regs to allocate" list, and as hard reg 3 appearing in q's
   conflict row. m1's is pseudo 76, m4's is pseudo 74, m3's is pseudo 78.

2. **Clause (a) is what the base body already buys, and it is fragile.** The
   base body's single named `m` carrying both the load and the mask is the ONLY
   measured spelling with no block-0-confined pseudo -- which is exactly why
   s39 adopted it -- and the price of that is m's 6 references.

3. **A same-constant re-assignment of `q` contributes zero references.** d1's
   .greg section is character-for-character identical to b0's. s41's n2 had
   shown such a re-set produces identical BYTES; d1 shows it does not even
   reach flow's reference count, so the cheapest imaginable route to clause (b)
   from q's side -- more pointer assignments -- is arithmetically inert, not
   merely byte-neutral.

4. **A source-level hoist does not shorten q's live range.** e1 moves block 1's
   condition above q's materialisation; q comes out with nrefs=10 livelen=28
   pri=10714, digit for digit the base body's, while p's live length grows
   33 -> 35 and the score rises to 13. The first scheduling pass re-places the
   hoisted computation inside q's range before live lengths are counted, the
   same compaction s39's x1/x3 measured from the middle of the range.

**The inequality, with thresholds.** With the block-0 value at 6 refs / 9 live
(13333) and q at 10 refs / 28 live (10714), q wins clause (b) iff any of:
  (i)   q reaches nrefs >= 13 at livelen 28 (3*13/28 = 13928);
  (ii)  q reaches livelen <= 21 at nrefs 10 (3*10/21 = 14285);
  (iii) the block-0 value reaches livelen >= 12 at 6 refs (2*6/12 = 10000),
        or >= 10 at 5 refs, or >= 8 at 4 refs,
and it must do so WITHOUT splitting block 0 into a confined pseudo (clause (a)).
Exit (i)'s known ref sources are all dead: same-constant re-set (zero refs,
d1/n2), copy-loop destination `q[i-3]` (30, s37/s42), derived pointer
`u8 *r = q - 3;` (19 at 50, s29). Exit (ii) is unmoved by hoisting (e1). Exit
(iii) has been attacked from the middle (s39 x1/x3) and the front (e1) and never
from the END -- no measurement yet exists for a body in which the block-0 masked
value has a real, ordinary consumer AFTER block 1's store while remaining one
multi-block pseudo. That is frontier A.

Artifacts: tmp/grind/func_80034F88/s43/variants/*.c (b0, d1, m1..m5, e1),
tmp/grind/func_80034F88/s43/run.ps1 (splice + score + save .o),
tmp/grind/func_80034F88/dumps/code6cac_b.greg (per-body, overwritten between
bodies; the conflict rows quoted above are transcribed in this section).

- [s41] Chassis: memory/grind/func_80034F88/candidate.c re-measured 10 at 49 insns, rules_dropped 0, on HEAD 2026-09-05.

- [s41] ALLOCDBG allocno table for the base body (BB2_ALLOC_DEBUG=1 via tmp/grind/func_80034F88/s39/ad.sh): ord=0 pseudo=73 (i) hardreg=3 nrefs=11 livelen=7 pri=47142; ord=4 pseudo=74 (m) hardreg=3 nrefs=6 livelen=9 pri=13333; ord=5 pseudo=75 (q) hardreg=4 nrefs=10 livelen=28 pri=10714; ord=8 pseudo=72 (p) hardreg=5 nrefs=6 livelen=33 pri=3636.

- [s41] The target seats the flag-byte address in $v1 for blocks 0/1 and the block-0 value in $a0; our bodies have them swapped. Pseudo 73 (i, the copy-loop induction variable) is absent from q's conflict row in every body measured, so the target's sharing of $v1 between the address and i is not a barrier.

- [s41] The base body is the only measured spelling whose q conflict row is free of hard reg 3 (`72 74 75 77 79 80 83 84 2 29`); m1, m3 and m4 all read `... 2 3 29`.

- [s41] A block-0-confined pseudo is identifiable in .greg as a register dispositioned `in 3` that does not appear in the `regs to allocate` list (m1: 76, m3: 78, m4: 74) -- this is local_alloc's seat, taken before global allocation runs.

- [s41] $v0 is unavailable to that local quantity because the call's v0 -> $a1 copy is scheduled after block 0's read-modify-write in every body measured, and in the target as well (its copy sits at 0x80034FA4, after the lbu at 0x80034FA0).

- [s41] A redundant same-constant `q = &D_80106A73;` is deleted before flow counts references: d1's .greg section is character-for-character identical to the base body's and it scores 10 at 49.

- [s41] Score table this session: b0 10/49, d1 10/49, m1 10/49, m4 10/49, m2 12/46, e1 13/49, m3 17/49, m5 27/52.

- [s41] Threshold arithmetic on this chassis: q wins allocno_compare at nrefs >= 13 (livelen 28) or livelen <= 21 (nrefs 10); the block-0 value loses at livelen >= 12 (6 refs), >= 10 (5 refs), >= 8 (4 refs).

- [s41] Process note: the dispatch digest lagged the ledger by eight sessions again (dispatched as 'session 35' with an s34-era frontier while the ledger is at s42); the previous session's owner-gated foreclosure was discarded because a foreclosure disposition requires escalation modality, so the LADDER EXHAUSTED record it drafted still needs re-filing by an escalation-modality session.

## s44 (synthesis, 2026-09-05; dispatched as "session 35" -- the digest lags the ledger by nine sessions) -- the residual re-derived from the OBJDUMP, not from the allocno tables

Floor unchanged: **10 at 49 insns**.  Every measurement below is
`sandbox func_80034F88 --disable all` with the body spliced over
`INCLUDE_ASM("asm/funcs", func_80034F88);` in `src/code6cac_b.c`; `src/` was
restored to HEAD before the outcome was written.

### KILL RE-AUDIT (mandated -- all four re-measure at their banked values)
| body | this session | banked |
|---|---|---|
| b0 (= `memory/grind/func_80034F88/candidate.c`) | 10 at 49 | 10 at 49 |
| rt (`rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c`) | 10 at 49 | 10 at 49 |
| t1 (`rejected/s39-block1-reads-symbol-directly-score11.c`) | 11 at 50 | 11 at 50 |
| m1 (`rejected/s43-mask-folded-single-stmt-localalloc-v1-score10.c`) | 10 at 49 | 10 at 49 |

s41/s42 already established `fake_ablate.py` finds nothing to ablate on this
chassis (the body carries no FAKE construct), so no banked kill on it was
measured with a FAKE carrier occupying the contested pseudo.

### THE CORRECTION s44 MAKES TO THE LEDGER (read this before re-reading s41/s42)
s41 and s42 recorded the residual as an **emission requirement**: "the target
materialises `&D_80106A73` into an allocatable register THREE times and the
only C construct that emits that lui+addiu pair is an assignment of the
address to a declared pointer object", concluding that three materialisations
need three pointer objects.  **The first half of that is true; the conclusion
is false, and the base body already disproves it.**  Disassembly of `b0.o`
(`tmp/grind/func_80034F88/s44/b0.o`, banked; produced this session, never read
in s41/s42, which reasoned from allocno tables and score arithmetic):

```
b0 (score 10, 49 insns)                     TARGET (asm/funcs/func_80034F88.s)
 4f48 lui   a0,%hi(SYM)                     80034F98 lui   v1,%hi(SYM)
 4f4c addiu a0,a0,%lo(SYM)      <-- mat1    80034F9C addiu v1,v1,%lo(SYM)   <-- mat1
 4f50 lbu   v1,0(a0)                        80034FA0 lbu   a0,0(v1)
 4f54 move  a1,v0                           80034FA4 addu  a1,v0,zero
 4f58 andi  v1,v1,0xf8                      80034FA8 andi  a0,a0,0xF8
 4f5c sb    v1,0(a0)                        80034FAC sb    a0,0(v1)
 4f60 lw    v0,32(a1)                       80034FB0 lw    v0,0x20(a1)
 4f64 nop                       <-- (1)     80034FB4 lbu   a0,0(v1)      <-- reload
 4f68 andi  v0,v0,0x1                       80034FB8 andi  v0,v0,0x1
 4f6c bnez  v0,.L                           80034FBC bnez  v0,.L
 4f70  ori  v0,v1,0x1                       80034FC0  ori  v0,a0,0x1
 4f74 move  v0,v1                           80034FC4 addu  v0,a0,zero
.L:                                        .L:
 4f78 sb    v0,0(a0)            <-- (2)     80034FC8 lui   a0,%hi(SYM)
 4f7c lui   a0,%hi(SYM)                     80034FCC addiu a0,a0,%lo(SYM)  <-- mat2
 4f80 addiu a0,a0,%lo(SYM)      <-- mat2    80034FD0 sb    v0,0(v1)
 ... blocks 2 and 3 REGISTER-EXACT and instruction-exact in both, with mat3 ...
 ... the trailing copy loop and epilogue REGISTER-EXACT in both ...
```

Facts this establishes, none of which were on the ledger:
- **The base body already emits all three `lui`+`addiu` materialisations of
  `&D_80106A73`** (4f48, 4f7c, 4fa0), from ONE declared `u8 *q` that is
  re-assigned the same constant at the head of blocks 2 and 3.  cse does not
  unify the re-sets into one register-held value; it re-materialises the
  address at each one.  The "three materialisations require three pointer
  objects" step in s41/s42 is therefore wrong, and any future foreclosure
  record must not rest on it.
- **The entire 10-point residual is one register swap in blocks 0/1**: the
  target puts the ADDRESS in `$v1` and the block-0/1 flag-byte VALUE in `$a0`;
  the base body puts the address in `$a0` and the value in `$v1`.  Blocks 2
  and 3, the copy loop, the prologue and the epilogue are already
  byte-identical.
- **The ordering difference at `.L` is a consequence of the swap, not an
  independent defect.**  In the target mat2 (into `$a0`) can be scheduled
  ahead of block 1's store because that store uses `$v1`; in b0 the store uses
  `$a0`, so mat2 must follow it.  Fix the seats and the order follows.
- **The block-1 reload is worth exactly one instruction slot and it is free.**
  The target's `lbu a0,0(v1)` at 80034FB4 sits in the load-delay slot of the
  preceding `lw`; b0 emits a `nop` there.  Both bodies are 49 insns.  This
  confirms s29's arithmetic pricing of the reload at ZERO by direct
  measurement for the first time: `rt` (which HAS the reload) and `b0` (which
  does not) both score 10.  The reload is not a lever; it is a passenger of
  the seat assignment.

### Pass census by QI memory reference (new instrument -- cheaper than the allocno tables)
`pwsh tools/grinder/dump.ps1 func_80034F88` on the p1 body, then counting
`(mem:QI` inside the `;; Function func_80034F88` slice of each dump
(`tmp/grind/func_80034F88/dumps/`):

| pass | QI stores | QI loads | insns in slice |
|---|---|---|---|
| .rtl / .jump | 4 | **4** | 37 |
| .cse / .loop / .cse2 | 4 | **3** | 34 |
| .combine / .greg | 4 | 3 | 30 |

The target contains four flag-byte loads (80034FA0, 80034FB4, 80034FD8,
80034FFC).  A source with four loads reaches `.rtl` with four and **cse
deletes one**: block 1's load is forwarded from block 0's store because both
mems hash to the same address rtx.  This is the load-count statement of the
same wall the ledger has been describing in allocation terms.

### Measurements (all new this session)
| body | what it changes vs b0 | score | insns |
|---|---|---|---|
| p1 | block 1's THEN arm reads `*q` (else arm keeps `m`) -- duplicate read into arms | 10 | 49 |
| p2 | mirror: block 1's ELSE arm reads `*q` | 10 | 49 |
| p3 | q covers blocks 0/1/2; block 3 addressed through the bare symbol | 16 | 49 |
| p4 | q covers blocks 0/1/3; block 2 addressed through the bare symbol | 20 | 50 |
| p5 | block 0 wholly through the bare symbol; q first assigned inside block 1 | 12 | 50 |
| p6 | block 0's LOAD through the symbol; q assigned just before block 0's store | 11 | 50 |

- **p1 and p2 are BYTE-IDENTICAL to b0** (`cmp` on the .o files).  cse forwards
  the arm read back to `m`, so the body collapses onto the base body before
  allocation: the duplicate-read-into-arms spelling cannot stretch the block-0
  value's live range, because after forwarding it IS the base body.  Frontier A
  of s43 ("stretch `m` from the END with a real ordinary use") is closed for
  every consumer that is a re-read of the same byte, which is the only ordinary
  consumer this function has -- the flag byte's value after block 1's store is
  `m | bit`, not `m`, so no later block can legitimately consume `m` itself.
- **p5/p6 close exit (ii) from a direction e1 did not test.**  s43's e1 tried to
  SHORTEN q's live range by hoisting a computation out of it; p5/p6 shorten it
  by deleting q's uses from block 0 entirely.  The disassembly shows the
  materialisation is still emitted at the top of the function (p5: `lui a0` /
  `addiu a0` at 4f48-4f4c, immediately after the call, before the
  symbol-addressed load at 4f54-4f58), q still receives `$a0`, and the
  symbol-addressed block-0 load costs one extra `lui` (50 insns).  Source-level
  placement of the pointer's definition does not move the allocno: the address
  constant is materialised at the top of the function regardless.
- **p3/p4 price the "give a block back to the symbol" axis.**  s40 measured
  BOTH blocks 2 and 3 on the symbol at 28.  Dropping only block 3 costs 6
  points (16), only block 2 costs 10 (20).  The axis is monotone and has no
  minimum below 10.

### What s44 hands the next session
The residual is ONE seat swap in blocks 0/1 and nothing else.  Both sides of
`allocno_compare` have now been attacked from every measured direction, but
the OBJDUMP framing opens a question the allocation framing never asked: the
base body and the target differ in which of two pseudos (address, value) gets
`$v1`, and every body measured so far leaves the value pseudo ranked first.
The untried lever is the VALUE pseudo's hard-register PREFERENCE rather than
its rank -- global.c gives an allocno a preference from copy insns
(`reg_preferred_class` / the `regs_may_share` and copy-propagation machinery in
`local-alloc.c`), and the target's block-0 value shares `$a0` with the blocks
2/3 ADDRESS materialisations, which is exactly the sharing pattern a copy
preference produces.  No session has yet read the `.lreg` preference records
for the base body; that read, not another spelling, is the next cheap step.

- [s42] Kill re-audit on today's chassis: b0 = 10 at 49, rt = 10 at 49, t1 = 11 at 50, m1 = 10 at 49 -- all equal to their banked values; the chassis carries no FAKE construct (fake_ablate has nothing to ablate, per s41/s42).

- [s42] LEDGER CORRECTION: the base body (one declared `u8 *q`, re-assigned the same constant at the heads of blocks 2 and 3) already emits all THREE of the target's lui+addiu materialisations of &D_80106A73, at 4f48, 4f7c and 4fa0 of tmp/grind/func_80034F88/s44/b0.o. The s41/s42 conclusion that three materialisations require three pointer objects is false and must not carry a future foreclosure record.

- [s42] The entire 10-point residual is one register swap in blocks 0/1: the target seats the flag-byte address in $v1 and the block-0/1 value in $a0, the base body the reverse. Blocks 2 and 3, the copy loop, the prologue and the epilogue are already register-exact and instruction-exact.

- [s42] The `.L` ordering difference (the target schedules the blocks-2/3 materialisation ahead of block 1's store) is a consequence of that swap, because the target's block-1 store uses $v1 and therefore does not conflict with the materialisation's $a0 destination.

- [s42] The target's block-1 reload at 80034FB4 occupies the load-delay slot that the base body fills with a nop, so it costs nothing: rt (which has the reload) and b0 (which does not) are both 49 insns and both score 10. This is the first direct measurement of s29's arithmetic pricing of the reload at zero.

- [s42] Per-pass (mem:QI census of the function slice of the cc1 dumps: .rtl and .jump carry 4 flag-byte loads (the target's count), .cse leaves 3, and .loop/.cse2/.combine/.greg never change it -- cse is the pass that forwards block 1's load out of a four-load source.

- [s42] p1/p2 (duplicate read into block 1's arms) produce objects BYTE-IDENTICAL to b0.o -- a score of 10 does not imply a distinct body, so any future write-up must cmp against b0.o first.

- [s42] .lreg for the base body: pseudo 72 = p (6 refs / 33 insns), 73 = i (11 / 7), 74 = block-0 value (6 / 9), 75 = q (10 / 28); the only local-alloc quantities (78, 83, 87, 89, 91, 93) are ALL seated in hard reg 2 ($v0), so q's conflict row is clean in this body and the seat is lost on sort order alone.

- [s42] New measurements: p1 10/49, p2 10/49, p3 16/49, p4 20/50, p5 12/50, p6 11/50, p7 24/53, p8 13/47.

- [s42] src/code6cac_b.c was restored to HEAD (INCLUDE_ASM) before this outcome was written; no build-pipeline file was modified.

## s45 (synthesis, 2026-09-05; dispatched as "session 35" -- the digest lags the ledger by ten sessions) -- Frontier A executed: the residual is a LOCAL-ALLOC seat, not a global-alloc rank

Floor unchanged: **10 at 49 insns**.  Every measurement is
`sandbox func_80034F88 --disable all` with the body spliced over
`INCLUDE_ASM("asm/funcs", func_80034F88);` in `src/code6cac_b.c`; `src/` was
restored to HEAD before the outcome was written.  Splice/measure harness:
`tmp/grind/func_80034F88/s45/{splice.py,run.ps1,slice.py}`; FINDREGDBG harness
`tmp/grind/func_80034F88/s45/fr.sh` (instrumented cc1 `tools/gcc-2.7.2/cc1`,
instrumentation granted by docs/grind/decisions.md:14784).

### The preference records, read for the first time (this was s44's Frontier A)
`.greg` prints `;; NN preferences: R` lines, and the instrumented cc1 prints the
full pass-0/pass-1 exclusion sets under `BB2_FINDREG_DEBUG=<pseudo>`.  For the
base body `b0` (= `memory/grind/func_80034F88/candidate.c`):

```
;; 9 regs to allocate: 73 77 80 84 74 75 79 83 72
;; 74 conflicts: 72 74 75 77 2 29                 (NO preferences line)
;; 75 conflicts: 72 74 75 77 79 80 83 84 2 29     (NO preferences line)
;; 77/80/84 preferences: 2
;; dispositions: 72 in 5   73 in 3   74 in 3   75 in 4  ...

FINDREGDBG func=func_80034F88 pseudo=74   conflicts: 2 29
  someone_prefers: (empty)  own_copy_prefs: (empty)  own_full_prefs: (empty)
  used_so_far: 0..15 24..29 31
  pass0_used:  0 1 2 16..23 26..31         -> lowest free = 3
FINDREGDBG func=func_80034F88 pseudo=75   conflicts: 2 3 29
  someone_prefers: (empty)  own_copy_prefs: (empty)  own_full_prefs: (empty)
  pass0_used:  0 1 2 3 16..23 26..31       -> lowest free = 4
```

So in `b0` neither contested allocno carries any preference at all: 74 (the
block-0 flag-byte value) takes `$v1` simply as the lowest-numbered register free
in `find_reg`'s pass 0 (global.c:1052), and 75 (`q`) then takes `$a0`.  Nineteen
sessions of `allocno_compare` analysis were reading only half of the selection
rule.

### NEW EXIT #5, which s39's four-exit enumeration structurally could not see
`find_reg`'s pass 0 excludes `regs_someone_prefers[allocno]` (global.c:1001; the
set is built in `prune_preferences`, global.c:911-931).
`regs_someone_prefers[74]` is the union of `hard_reg_full_preferences` over
LOWER-priority allocnos that conflict with 74 -- here 75 (`q`) and 72 (`p`).  If
either carried a full preference for hard reg 3, pass 0 would skip `$v1` for 74,
find `$a0` free, and succeed there; `q` would then take `$v1`.  That is the
target's disposition, reached **with the allocno order left exactly as it is**.
This is a fifth numeric exit alongside s39's four (q nrefs>=13, q livelen<=22,
m nrefs<=4, m livelen>=12), and unlike those four it does not require winning
the priority race at all.  Its only generator is `set_preference`
(global.c:1700-1754), which fires on a copy insn where one side is a hard
register **or a pseudo that local-alloc has already renumbered** -- so the
question it poses is: which hard register does local-alloc give block 0's
quantities?

### Why every order-race winner still loses -- read off .greg, not inferred
| body | q allocno | order | q's conflict row | disposition |
|---|---|---|---|---|
| b0  | 75, 10 refs / 28 insns | 74 **before** 75 | `... 2 29` (clean) | q in 4, value in 3 |
| m1  | 75, 10 refs / 28 insns | 75 **before** 74 | `... 2 3 29` | q in 4, value in 3 |
| rt  | 74, 15 refs / 30 insns | 74 second overall | `... 2 3 29` | q in 4, value in 3 |
| z0 (score-0, BANNED) | 77, 3 refs / 14 insns | -- | clean, **`77 preferences: 3`** | q in 3, value in 4 |

`m1` and `rt` DO win the priority race (m1: folding the mask into one statement
cuts the block-0 value to 4 refs, key 10000 against q's 10714, and the `.greg`
order really does become `73 78 81 85 75 74 ...` with q first; rt: the dead
round-trip lifts q to 15 refs / key 15000 and it is allocated second overall).
Both then lose on the conflict row, and the cause is identical in both: the
spelling that cuts the value's references also makes block 0's byte value a
**block-0-confined quantity**, local-alloc seats that quantity in `$v1`
(m1 `Register 76 ... in block 0`, disposition 3; rt `Register 75`/`76 ... in
block 0`, dispositions 3/3), and it is live while `q` is live, so hard reg 3
enters `q`'s conflict row.  The "two-clause conjunction" s41 recorded now has a
mechanism instead of a description.

### What the score-0 (banned) body actually does -- and it is NOT an allocno-rank story
`z0` = `rejected/macro-respelling-of-banned-four-handle-score0-DO-NOT-SUBMIT.c`.
Its `.lreg`/`.greg` (dumped for the first time as a preference read):
- `Register 74 used 4 times across 16 insns in block 0; pointer` -> **block 0's
  POINTER is itself a block-confined quantity**, and local-alloc seats it in
  hard reg **3**.
- `Register 75 ... 2 times across 4 insns in block 0; 1 bytes` -> block 0's byte
  VALUE is seated in hard reg **4**.
- `Register 77 ... 3 refs / 14 insns; pointer` (the blocks-1 global pointer)
  carries `;; 77 preferences: 3` and is allocated into 3.  The copy from the
  locally-allocated block-0 pointer is what created that preference
  (global.c:1725 `set_preference`, whose `reg_renumber[src] >= 0` branch treats
  a locally-allocated pseudo exactly like a hard register); the copy then
  coalesces away.

### The target re-derived through that lens
In the target the block-0 value is DEAD after `sb a0,0(v1)` at 80034FAC --
block 1 RELOADS at 80034FB4.  Its allocno therefore carries ~3 references all
confined to block 0, i.e. in the original compilation block 0's byte value was a
block-confined quantity seated in **`$a0`** while block 0's address was seated in
**`$v1`** -- the same disposition pair `z0` produces, and the exact opposite of
the pair local-alloc produces in every single-object body measured across 45
sessions.  **The residual is a local-alloc seat question.**  Sessions s32-s44
attacked `global.c` exclusively; `local-alloc.c`'s quantity ordering and its
`qty_phys_copy_sugg` / `find_free_reg` path have never been read for this
function.

### Measurements (all new this session)
| body | what it changes vs b0 | score | insns |
|---|---|---|---|
| x1 | block-1 condition hoisted to function scope (to stretch pseudo 74) | 13 | 45 |
| x2 | block-0 store moved after the hoisted condition | 13 | 45 |
| x3 | block-1's `p[8]` load hoisted between the value's def and the mask | 13 | 49 |
| x4 | block-0 store moved inside block 1, after the condition | 13 | 49 |
| x5 | block-1 AND block-2 conditions hoisted | 25 | 40 |
| y1 | block 0 through the BARE SYMBOL (no pointer object at all), q first assigned at block 1's head | 16 | 51 |
| y2 | as y1 but block 0's mask through a named `u8` local | 16 | 51 |

- **x3/x4 kill the "stretch the value's live range by source statement order"
  exit outright.**  x4's `.lreg` still reads `Register 74 used 6 times across 9
  insns` -- byte-for-byte the same allocno geometry as b0 -- and its `.greg`
  allocation order (`73 77 80 84 74 75 79 83 72`), conflict rows and
  dispositions are IDENTICAL to b0's.  `reg_live_length` is computed after
  combine has re-normalised the insn order, so moving statements between the
  value's definition and its uses in the C source does not reach it.  s39's exit
  "m livelen >= 12" is therefore not addressable by statement order at all; only
  by inserting real instructions, which x1/x2/x5 do at the cost of collapsing
  the body (45 / 45 / 40 insns).
- **y1/y2 close the anonymous-block-0-address route to the preference exit.**
  Spelling block 0 with no pointer object does make block 0's address an
  anonymous pseudo, but y1's `.greg` shows `q` (74) still in `$a0`, still
  carrying hard reg 3 in its conflict row, and still with NO preference for 3:
  GCC re-materialises `&D_80106A73` at the top of the function for `q` instead
  of copying block 0's temp into it, so `set_preference` never fires.  The copy
  edge that `z0` gets for free needs block 0's address to be a *named* object
  whose value cse can forward -- which is the banned multi-handle shape.

### Housekeeping
- `src/code6cac_b.c` was restored to HEAD (`INCLUDE_ASM`) before this outcome was
  written; no build-pipeline file was modified.
- The dispatch digest again lagged badly (dispatched as "session 35" with an
  s34-era frontier while the ledger stood at s44).  Run
  `grep -n '^## \[s' memory/grind/func_80034F88/hypotheses.md | tail` before
  spending a probe on a brief-listed "next probe".
- Ladder accounting: the six-modality condition was met at s31; the LADDER
  EXHAUSTED record drafted by s42 still needs filing by an `escalation`-modality
  session (s42's owner-gated outcome was discarded for filing one from
  `structural`; s44 and s45 are `synthesis`, so neither files one).

- [s43] [s45] Kill re-audit on today's chassis: b0 (= memory/grind/func_80034F88/candidate.c) = 10 at 49/49, rules_dropped 0; m1 and rt reproduce their banked allocno geometry. No FAKE construct is present for fake_ablate.py to ablate (as s41/s42 recorded).

- [s43] [s45] The .greg preference lines and the BB2_FINDREG_DEBUG exclusion sets were read for the first time (this was s44's Frontier A). In the base body pseudo 74 (block-0 flag-byte value) has conflicts {2,29}, empty someone_prefers, empty copy/full preferences, and pass0_used {0,1,2,16..23,26..31} -- it takes $v1 as the lowest free register in find_reg's pass 0 (global.c:1052). Pseudo 75 (q) then has 3 in its pass-0 mask and takes $a0. .greg prints preference lines only for 77/80/84, all for hard reg 2.

- [s43] [s45] NEW EXIT #5 that s39's four-exit allocno_compare enumeration structurally could not see: regs_someone_prefers[74] containing reg 3 makes pass 0 skip $v1 for 74, succeed on $a0, and leave $v1 for q -- with the allocno ORDER unchanged. Built at global.c:911-931 (prune_preferences) from the hard_reg_full_preferences of LOWER-priority conflicting allocnos (q=75, p=72); consumed at global.c:1001. Sole generator: set_preference, global.c:1700-1754, whose reg_renumber[src] >= 0 branch treats a locally-allocated pseudo as a hard register.

- [s43] [s45] The two-clause conjunction s41 described now has a mechanism read off the dumps. m1's allocation order is '73 78 81 85 75 74 80 84 72' (q FIRST -- the priority race IS winnable) but '75 conflicts: ... 2 3 29'; rt's q is allocated second overall but '74 conflicts: ... 2 3 29'; b0's q row is clean ('... 2 29') and loses the order race. In m1 and rt the hard-reg-3 entry comes from a block-0-CONFINED byte quantity that local-alloc seats in $v1 (m1 reg 76; rt regs 75 and 76) while q is live.

- [s43] [s45] The score-0 banned body's advantage is NOT allocno rank. Its .lreg/.greg show block 0's POINTER is itself a block-confined quantity ('Register 74 used 4 times across 16 insns in block 0; pointer') seated by local-alloc in hard reg 3, block 0's byte VALUE seated in hard reg 4, and the blocks-1 global pointer allocno carrying ';; 77 preferences: 3' -- a copy preference inherited from that locally-allocated block-0 pointer -- so it takes $v1 and the copy coalesces away.

- [s43] [s45] TARGET RE-DERIVATION: the target's block-0 value is dead after `sb a0,0(v1)` at 80034FAC because block 1 reloads at 80034FB4, so its allocno carries ~3 references confined to block 0. In the original compilation block 0's byte value was therefore a block-confined quantity seated in $a0 while block 0's address sat in $v1 -- the same disposition pair the score-0 body produces, and the exact opposite of the pair local-alloc produces in every single-object body measured in 45 sessions. The residual is a LOCAL-ALLOC seat question; sessions s32-s44 attacked global.c exclusively and local-alloc.c's quantity ordering and qty_phys_copy_sugg/find_free_reg path have never been read for this function.

- [s43] [s45] reg_live_length for the block-0 value is invariant under source statement order: x4 (block-0 store moved inside block 1 after the condition) still dumps 'Register 74 used 6 times across 9 insns' and an allocation order, conflict-row set and disposition set identical to b0's, because flow.c recomputes it after combine re-normalises the insn order. s39's 'm livelen >= 12' exit is therefore not addressable by statement order.

- [s43] [s45] New measurements: x1 13/45, x2 13/45, x3 13/49, x4 13/49, x5 25/40, y1 16/51, y2 16/51. All seven banked in memory/grind/func_80034F88/rejected/ with s45- prefixes.

- [s43] [s45] src/code6cac_b.c was restored to HEAD (INCLUDE_ASM) before this outcome was written; no build-pipeline file was modified. Ladder accounting: the six-modality condition was met at s31; the LADDER EXHAUSTED record drafted by s42 still needs filing by an escalation-modality session (s42's owner-gated outcome was discarded for filing one from structural; s44 and s45 are synthesis).

## s46 (synthesis, 2026-09-05; dispatched as "session 44" -- the digest still lags the ledger) -- the LOCAL-ALLOC SEAT LAW, measured both ways, and a second find_reg exit read from source

Floor unchanged: **10 at 49 insns**.  All measurements are
`sandbox func_80034F88 --disable all` with the body spliced over
`INCLUDE_ASM("asm/funcs", func_80034F88);` in `src/code6cac_b.c`; `src/` was
restored to HEAD before this was written.  Harness copied from s45 into
`tmp/grind/func_80034F88/s46/{splice.py,slice.py,run.ps1,cmp.sh}`.

### KILL RE-AUDIT (mandated; both prongs run)
- `run.ps1 memory/grind/func_80034F88/candidate.c` -> **score 10, build_insns 49**
  on today's HEAD.  The chassis has not moved; every banked instance kill is
  still measured against a live 10/49 base.
- `python3 tools/fake_ablate.py --func func_80034F88 --file code6cac_b
  --candidate memory/grind/func_80034F88/candidate.c` -> *"no FAKE-annotated
  constructs found ... nothing to ablate"*.  Third consecutive session (s41,
  s42, s46) confirming the ledger's kills were all measured FAKE-free, so no
  banked kill is contaminated by a FAKE carrier occupying a contested pseudo.

### a1 -- the plainest human spelling ties the floor and is BYTE-IDENTICAL to b0
`a1` deletes the named `m` entirely and spells all four flag blocks
identically (`*q = *q & 0xF8;` then, per bit, `c = p[8] & K; v = *q;
if (c) c = v | K; else c = v; *q = c;`).  Measured **10 at 49**, and
`mipsel-linux-gnu-objdump -d` of its object against `s44/b0.o` (script
`tmp/grind/func_80034F88/s46/cmp.sh`) prints **IDENTICAL** -- the two bodies
emit the same 49 instructions.  This matters twice: (i) `candidate.c`'s `m`
variable is codegen-inert, so the submittable spelling of this function is the
uniform one (fewer constructs, better human-programmer-test answer, same
bytes) -- banked as
`rejected/s46-uniform-four-block-reload-no-m-BYTE-IDENTICAL-TO-b0-score10.c`
and recommended as the body to submit if this function ever becomes
submittable; (ii) a1's `.lreg`/`.greg` geometry is NOT b0's (a1 has THREE
block-0-confined quantities 75/76/79 and allocates q FIRST -- order
`73 78 82 86 74 77 81 85 72` -- while b0 has fewer and allocates q fourth), so
two materially different allocations can converge on the same emitted stream.

### THE LOCAL-ALLOC SEAT LAW (new; the mechanism behind 45 sessions of "q's row has a 3")
`find_free_reg` (local-alloc.c:2135) builds its exclusion set as
`for (ins = born_index; ins < dead_index; ins++) IOR_HARD_REG_SET (used,
regs_live_at[ins]);` (**local-alloc.c:2169-2171**) and then scans hard registers
**numerically** (local-alloc.c:2247, no `REG_ALLOC_ORDER` on this target).  At
local-alloc time NO global pseudo has a hard register yet, so among the low
allocatable registers `regs_live_at` can only contain **$v0 (2)** -- the return
value of `func_80077D00`, live from the call until the copy insn
`(set (reg p) (reg:SI 2 v0))`.  Therefore a block-0-confined quantity takes
**reg 2 if $v0 is dead across its span and reg 3 otherwise**, and reg 3 is
exactly what then enters `q`'s `hard_reg_conflicts` row and denies `$v1` to
`q` in `find_reg` regardless of allocno rank.
Measured both ways this session:
- **a1** (no use of `p` inside block 0): sched1 gives the `$v0 -> p` copy
  priority 0 *within block 0* (p has no consumer in that basic block, and
  sched.c computes priority over the block only), so the copy is emitted last,
  $v0 is live across the byte load, and block-0 quantities 75 and 76 are both
  seated in **3**.  `.greg`: `74 conflicts: ... 2 3 29`, dispositions
  `72 in 5  73 in 3  74 in 4 (q)  75 in 3  76 in 3  77 in 3` -- q is allocated
  FIRST among the contenders and still loses `$v1`, purely on the conflict row.
- **a2** (`c0 = p[8] & 1;` hoisted to the top so `p` IS consumed inside block 0,
  otherwise m1's mask-folded body): the copy moves up, $v0 dies early, and
  block-0 quantity 77 is seated in **2** for the first time in the ledger.
  Score **14 at 49**.  Dispositions: `72 in 5  73 in 3  75 in 3  76 in 4 (q)
  77 in 2  78 in 3`.  The law's converse holds -- but the *byte value*
  quantity 78 then takes 3 (2 is occupied by 77, 3 is the next free number),
  so q still lands in 4.  Freeing $v0 relocates the problem one register; it
  does not solve it.

### SECOND find_reg EXIT, read from source (call it exit #6)
After the two-pass scan picks `best_reg`, `find_reg` **overrides** it with a
register from `hard_reg_copy_preferences[allocno]` if that register is free and
class-compatible (**global.c:1096-1127**).  This is a second, independent route
to a seat that no session had read: it needs no priority win and no conflict-row
change, only a copy preference on the allocno itself.  a1 shows the machinery
live: `;; 77 preferences: 3` and `77 in 3` -- a global allocno inheriting reg 3
from a copy off a locally-allocated block-0 quantity, via `set_preference`
(global.c:1709-1735, the `reg_renumber[src] >= 0` branch).  For the residual it
is inert for the same reason exit #5 is: the only copy edge that could hand
`q` a preference for 3 is a copy from a block-0-confined POINTER temp, i.e. a
second C object aliasing `&D_80106A73` -- the axis the Judge closed.

### The target re-derived once more, now with the seat law
In the target block 0's byte value sits in **$a0 (4)**, which under the seat law
requires BOTH 2 and 3 to be excluded across its span: 2 by $v0 (the `addu $a1,
$v0, $zero` copy at 80034FA4 is inside the value's span, exactly as in our
bodies), and 3 by ANOTHER block-0-confined quantity allocated before it.  The
only candidate for that quantity is block 0's ADDRESS -- which in the target is
also read in block 1 (`sb $v0, 0($v1)` at 80034FD0) and therefore cannot be
block-confined unless block 1's pointer is a DIFFERENT pseudo that global alloc
later seats in 3 by copy preference.  That is precisely the score-0 banned
body's shape (`;; 77 preferences: 3`, block-0 pointer local at 3, block-0 value
at 4).  So the seat law converts s40's "two registers hold &D_80106A73
simultaneously at .L80034FC8" from an observation into a mechanism: the target
needs a block-0-confined pointer quantity, a single C pointer object cannot
supply one (its materialisation is hoisted to function entry -- s44 p5, s45 y1 --
and its live range spans all four blocks), and every remaining exit in find_reg
-- rank (s39), conflicts (s41/s43), someone_prefers (s45 exit #5), copy
preference (s46 exit #6) -- routes through that same missing quantity.

- [s46] Kill re-audit on today's chassis: candidate.c = 10 at 49; fake_ablate reports no FAKE-annotated construct to ablate. No banked kill is FAKE-contaminated.
- [s46] The uniform four-block reload body with NO named `m` (a1) scores 10 at 49 and its object is BYTE-IDENTICAL to s44/b0.o (objdump diff empty, tmp/grind/func_80034F88/s46/cmp.sh). candidate.c's `m` is codegen-inert; a1 is the preferred submission spelling. a1's .greg geometry nevertheless differs from b0's (three block-0 quantities, q allocated first), so identical score AND identical bytes can come from different allocations.
- [s46] SEAT LAW: local-alloc's find_free_reg excludes only regs_live_at over the quantity's span (local-alloc.c:2169-2171) and scans numerically (local-alloc.c:2247); at that point no global pseudo is renumbered, so the only low register that can be excluded in this function is $v0 (2), live from the call to the `$v0 -> p` copy. A block-0-confined quantity therefore takes 2 if $v0 is dead across its span and 3 otherwise -- and 3 is what poisons q's conflict row.
- [s46] The seat law was measured in BOTH directions: a1 (no use of p in block 0; sched1 sinks the copy because p has no consumer in that basic block) -> quantities 75/76 in reg 3, q in 4, score 10; a2 (`c0 = p[8] & 1;` hoisted so p IS consumed in block 0) -> quantity 77 in reg 2 (first time in the ledger), but the byte-value quantity 78 then takes 3 and q still lands in 4, score 14 at 49.
- [s46] NEW EXIT #6, read from source: after the two-pass scan, find_reg overrides best_reg with a register from hard_reg_copy_preferences if it is free and class-compatible (global.c:1096-1127). a1 exhibits it live (`;; 77 preferences: 3`, disposition 3). Like exit #5 its only generator for q would be a copy from a block-0-confined pointer temp = the Judge-banned second handle.
- [s46] Consequence for the record: the target's block-0 value at $a0 requires TWO exclusions (2 from $v0, 3 from a prior block-0 quantity), so the target's block 0 must contain a block-confined POINTER quantity. A single C pointer object cannot supply one, so all four find_reg exits (rank, conflicts, someone_prefers, copy preference) route through the same missing quantity.
- [s46] src/code6cac_b.c was restored to HEAD (INCLUDE_ASM) before this outcome was written; no build-pipeline file was modified. Ladder accounting unchanged: the six-modality condition was met at s31; the LADDER EXHAUSTED (non-endgame residual, floor 10) record still needs filing by an `escalation`-modality session.

- [s44] KILL RE-AUDIT (both prongs, mandated): memory/grind/func_80034F88/candidate.c re-measures 10 at 49 on today's HEAD, and tools/fake_ablate.py --func func_80034F88 --file code6cac_b --candidate memory/grind/func_80034F88/candidate.c reports 'no FAKE-annotated constructs found ... nothing to ablate'. Third consecutive session (s41, s42, s46) confirming no banked kill is FAKE-contaminated.

- [s44] New measurements: a1 (uniform four-block reload, no named m) = 10 at 49 and BYTE-IDENTICAL to s44/b0.o; a2 (m1 mask-fold with c0 = p[8] & 1 hoisted) = 14 at 49.

- [s44] SEAT LAW (new, source-cited): local-alloc's find_free_reg excludes only regs_live_at over the quantity's span (local-alloc.c:2169-2171) and scans hard registers numerically (local-alloc.c:2247). At local-alloc time no global pseudo is renumbered, so $v0 (2) -- live from the call to the $v0 -> p copy -- is the only low register that can be excluded in this function; a block-0-confined quantity therefore takes 2 if $v0 is dead across its span and 3 otherwise.

- [s44] sched1 explains why $v0 stays live: INSN_PRIORITY is computed within a basic block, and p has no consumer inside block 0, so the $v0 -> p copy is emitted last in that block (as in the target, addu $a1,$v0,$zero at 80034FA4, after the lbu at 80034FA0).

- [s44] The seat law was measured in both directions -- a1's block-0 quantities 75/76 seat in 3 (q in 4), a2's block-0 quantity 77 seats in 2 for the first time in the ledger, but its byte-value quantity 78 then takes 3 and q still lands in 4.

- [s44] NEW EXIT #6: find_reg overrides best_reg with a copy-preferred register after the two-pass scan (global.c:1096-1127); a1 exhibits it live (';; 77 preferences: 3', disposition 3), preference generated by set_preference's reg_renumber[src] >= 0 branch (global.c:1709-1735).

- [s44] Enumeration of find_reg's seat routes is now complete: allocno rank (s39), hard_reg_conflicts (s41/s43), regs_someone_prefers (s45 exit #5), hard_reg_copy_preferences override (s46 exit #6). All four route through a block-0-confined pointer quantity that a single C pointer object cannot supply.

- [s44] candidate.c's `m` local is codegen-inert; the uniform spelling banked at rejected/s46-uniform-four-block-reload-no-m-BYTE-IDENTICAL-TO-b0-score10.c emits the same 49 instructions and is the recommended submission body. A note recording this was added to candidate.c's header comment (the body is unchanged, so the review body key is unchanged).

- [s44] src/code6cac_b.c was restored to HEAD (INCLUDE_ASM) before this outcome was written; no build-pipeline file was modified. Ladder accounting unchanged: the six-modality condition was met at s31 and the LADDER EXHAUSTED (non-endgame residual, floor 10) record still awaits an `escalation`-modality session -- this session was dispatched as `synthesis`, so it does not file one.

## s47 (synthesis, 2026-09-05; dispatched as "session 45") -- THE TWO-PSEUDO PROOF: the single-C-pointer-object chassis is arithmetically incapable of the target's register geometry, and the last unread splitting mechanism (update_equiv_regs / reload rematerialisation) is read and closed

Floor unchanged: **10 at 49 insns**.  All measurements are
`sandbox func_80034F88 --disable all` with the body spliced over
`INCLUDE_ASM("asm/funcs", func_80034F88);` in `src/code6cac_b.c`; `src/` was
restored to HEAD before this was written.  Harness in
`tmp/grind/func_80034F88/s45x/{splice.py,run.ps1,od.sh,od.py}`.

### KILL RE-AUDIT (mandated; both prongs run)
- `run.ps1 memory/grind/func_80034F88/candidate.c` -> **score 10, build_insns 49**
  on today's HEAD.  Chassis unmoved.
- `python3 tools/fake_ablate.py --func func_80034F88 --file code6cac_b
  --candidate memory/grind/func_80034F88/candidate.c` -> *"no FAKE-annotated
  constructs found ... nothing to ablate"* (fourth consecutive confirmation:
  s41, s42, s46, s47).
- The two closest-to-target banked instance kills re-measured at their banked
  values: `rejected/s44-block0-load-symbol-q-before-store-score11.c` ->
  **11 at 50**; `rejected/s39-block1-reads-symbol-directly-score11.c` ->
  **11 at 50**.  No banked kill is chassis-stale or FAKE-contaminated.

### THE OBJDUMP FACT THE LEDGER NEVER STATED: the target uses TWO different hard registers for the flag-byte ADDRESS

Read directly off `asm/funcs/func_80034F88.s`:

- `80034F98/80034F9C  lui/addiu $v1, D_80106A73` -- base for the block-0 `lbu`
  (80034FA0), the block-0 `sb` (80034FAC), the block-1 reload `lbu` (80034FB4)
  and the block-1 `sb` (80034FD0).  It DIES at 80034FD0.
- `80034FC8/80034FCC  lui/addiu $a0, D_80106A73` -- base for block 2's `lbu`
  (80034FD8) and `sb` (80034FEC).
- `80034FF0/80034FF4  lui/addiu $a0, D_80106A73` -- re-materialised for block 3.

So the target holds the flag-byte address in **$v1 across blocks 0-1** and in
**$a0 across blocks 2-3**, and the two ranges overlap by two insns (the $a0
materialisation at 80034FC8/FCC is scheduled ahead of the block-1 store at
80034FD0, which still uses $v1).  The VALUE registers mirror the swap: block
0/1 value in `$a0`, block 2/3 value in `$v1`.

`tmp/grind/func_80034F88/s44/b0.o.txt` shows the base body doing the same thing
with ONE register: `lui/addiu $a0` three times, value in `$v1` throughout.
That is why blocks 2 and 3 are already register-exact, blocks 0 and 1 are
exactly reversed, and the residual is exactly 10 rather than 5 or 20.

**A pseudo gets exactly one hard register.**  GCC 2.7.2 has no live-range
splitting: `global_alloc` sets `reg_renumber[]` once per allocno and
`local_alloc` seats a quantity once.  The target's address therefore lives in
**two distinct pseudos**, not one.  Every body this ledger has measured on the
single-declared-`u8 *q` chassis has exactly one, so its best possible outcome is
"one half of the flag blocks in the right register" -- which is precisely the
observed 10.

### WHERE A SECOND POINTER PSEUDO CAN COME FROM IN C -- enumerated and measured

1. **A second declared/assigned C pointer object.**  Judge-CLOSED for this
   function (multi-handle axis; three separate constraints in the brief).
2. **The bare symbol, hoisted into a pseudo by cse/loop.**  MEASURED DEAD this
   session and explained from compiler source:
   - `z1a` (blocks 0 AND 1 spelled through the bare symbol `D_80106A73`, with
     `q` declared and first assigned at block 2's head and reassigned at block
     3 -- i.e. the exact C shape of the target's two-range geometry, using only
     ONE declared pointer object): **25 at 49**.
   - `z1b` (same, but block 1 re-reads `D_80106A73` instead of reusing the
     masked `m`, matching the target's 80034FB4 reload): **25 at 49**.
   - The objdump of `z1a` (`tmp/grind/func_80034F88/s45x/z1a.txt`) shows WHY:
     every bare-symbol access is emitted as its own `lui $at` + memory-op macro
     pair (`lui at,0x0 ; sb v1,0(at)`, four times), never as a shared base
     register.  cse never hoists the address because on this target a
     `SYMBOL_REF` **is** a legitimate memory address --
     `GO_IF_LEGITIMATE_ADDRESS` accepts it unconditionally at
     **`tools/gcc-2.7.2/config/mips/mips.h:2300`** (`if (CONSTANT_ADDRESS_P
     (xinsn)) goto ADDR;`, and `CONSTANT_ADDRESS_P` accepts `SYMBOL_REF`) -- so
     the address expression carries no cost for cse to eliminate and no pseudo
     is ever created for it.  This is a structural property of the port, not of
     any one spelling.
   - Consistent with the older banked forms:
     `symbol-only-no-pointer-score29.c` (29),
     `s40-roundtrip-q-blocks01-directsymbol23-score28.c` (28, the mirror split),
     `s44-block2-via-bare-symbol-score20.c` (20),
     `s44-block3-via-bare-symbol-score16.c` (16).
3. **A pointer derived from a DIFFERENT symbol with a non-zero addend** (e.g.
   declaring `D_80106A70` as an array and reaching the flag byte as
   `D_80106A70[3]`).  This forecloses on bytes, not on allocation: the target's
   accesses are all `0($reg)` with `%lo(D_80106A73)` folded into the base,
   whereas an array-base pointer emits `3($reg)` with `%lo(D_80106A70)`.
   Already banked in shape as `distinct-symbol-addend-four-bases-score28.c` (28).
4. **A compiler-side split of one pseudo into two hard registers.**  The last
   unread mechanism; READ THIS SESSION and CLOSED.  `update_equiv_regs`
   (local-alloc.c:947-1116) is the only pass between flow and reload that
   touches a pseudo with a constant equivalence, and its only action is
   *deletion*: when `reg_n_refs[regno] == 2 && reg_basic_block[regno] < 0` it
   records `reg_equiv_replacement[regno] = SET_SRC (set)`
   (**local-alloc.c:1080-1082**), then substitutes the constant back into the
   single use and turns the initialising insn into a `NOTE_INSN_DELETED`
   (**local-alloc.c:1102-1115**).  It never creates a second register; it
   removes one.  Its other effect, `reg_live_length[regno] *= 2`
   (local-alloc.c:1064), only reweights global-alloc priority and is
   inapplicable here: `q` is set three times, so it carries no single REG_EQUIV.
   Reload rematerialisation of an equivalent constant is the same substitution,
   and `q` is never spilled on any measured body.

### CONSEQUENCE

The residual is no longer a heuristic story about allocno rank, conflict rows,
`regs_someone_prefers` or copy preferences (s39/s41/s43/s45/s46 enumerated all
six `find_reg` exits, and each one routes through a missing block-0-confined
pointer quantity).  It is an **arity** fact: the target needs two pointer
pseudos; C creates a pointer pseudo aliasing `&D_80106A73` only by naming a
pointer object (route 2 disproven by measurement plus mips.h:2300, route 3
forecloses on bytes, route 4 disproven from local-alloc source); and the second
pointer object is the axis the Judge closed three times.  The floor of 10 on the
single-object chassis is therefore structural, and the non-alias ladder for this
function has no remaining generator that this session can name.

- [s47] Kill re-audit on today's chassis: candidate.c = 10 at 49; the two closest-to-target banked kills (s44-block0-load-symbol-q-before-store, s39-block1-reads-symbol-directly) both re-measure at 11 at 50; fake_ablate reports no FAKE-annotated construct to ablate (fourth consecutive session).
- [s47] Read off asm/funcs/func_80034F88.s: the target materialises &D_80106A73 into $v1 at 80034F98/F9C (used by blocks 0 and 1, dead at 80034FD0) and into $a0 at 80034FC8/FCC and again at 80034FF0/FF4 (used by blocks 2 and 3). The address occupies TWO distinct hard registers, and the value registers mirror the swap ($a0 in blocks 0/1, $v1 in blocks 2/3).
- [s47] GCC 2.7.2 has no live-range splitting: one pseudo receives exactly one hard register. The target's two address registers therefore require TWO pointer pseudos. The base body (s44/b0.o) has one, which is why blocks 2/3 are already register-exact, blocks 0/1 are exactly reversed, and the residual is exactly 10.
- [s47] z1a (blocks 0 AND 1 through the bare symbol, single declared `u8 *q` first assigned at block 2's head and reassigned at block 3 -- the exact C shape of the target's two-range geometry with one declared object) measures 25 at 49; z1b (same with block 1 re-reading the symbol, matching the target's 80034FB4 reload) also measures 25 at 49.
- [s47] z1a's objdump shows every bare-symbol access emitted as its own `lui $at` + memory-op macro pair, never a shared base register. Mechanism read from source: GO_IF_LEGITIMATE_ADDRESS accepts a SYMBOL_REF address unconditionally (tools/gcc-2.7.2/config/mips/mips.h:2300), so the address costs nothing for cse to eliminate and no pseudo is created for it. Bare-symbol spelling cannot supply a pointer pseudo on this target.
- [s47] The last unread splitting mechanism is closed from source: update_equiv_regs (local-alloc.c:947-1116) only ever DELETES a constant-equivalent pseudo (reg_equiv_replacement at local-alloc.c:1080-1082; substitution + NOTE_INSN_DELETED at local-alloc.c:1102-1115); it never creates a second hard register. `q` is never spilled, so reload rematerialisation cannot split it either.
- [s47] Synthesis: the residual is an ARITY fact, not an allocation-heuristic fact. Two pointer pseudos are required; C generates a pointer pseudo aliasing &D_80106A73 only by naming a pointer object; the second object is Judge-closed. Frontier reset to (i) a ruling-request asking whether the two-register geometry reopens the multi-handle axis, (ii) the header-declaration axis (aggregate-merge) as the only untried non-alias surface, (iii) the LADDER EXHAUSTED record for an escalation-modality session.
- [s47] src/code6cac_b.c was restored to HEAD (INCLUDE_ASM) before this outcome was written; no build-pipeline file was modified.

- [s45] Kill re-audit on today's chassis: candidate.c = 10 at 49; rejected/s44-block0-load-symbol-q-before-store-score11.c = 11 at 50; rejected/s39-block1-reads-symbol-directly-score11.c = 11 at 50; tools/fake_ablate.py reports no FAKE-annotated construct to ablate (fourth consecutive session after s41, s42, s46).

- [s45] Read off asm/funcs/func_80034F88.s: the target materialises &D_80106A73 into $v1 at 80034F98/F9C (base for the block-0 lbu at 80034FA0, the block-0 sb at 80034FAC, the block-1 reload lbu at 80034FB4 and the block-1 sb at 80034FD0, where it dies) and into $a0 at 80034FC8/FCC and again at 80034FF0/FF4 (blocks 2 and 3). The two ranges overlap by two instructions.

- [s45] The target's value registers mirror the address swap: block 0/1 value in $a0, block 2/3 value in $v1. The base body (tmp/grind/func_80034F88/s44/b0.o.txt) uses $a0 for all three address materialisations and $v1 for every value, so blocks 2/3 are register-exact and blocks 0/1 are exactly reversed.

- [s45] GCC 2.7.2 assigns exactly one hard register per pseudo (no live-range splitting), so the target's two address registers require two pointer pseudos. A chassis with one declared pointer object can therefore seat at most half the flag blocks correctly, which is a quantitative explanation of the 10-point floor that has been flat since s26.

- [s45] z1a (blocks 0 AND 1 through the bare symbol, single declared `u8 *q` first assigned at block 2's head and reassigned at block 3) = 25 at 49; z1b (same, with block 1 re-reading the symbol to match the target's 80034FB4 reload) = 25 at 49. This is the first body measured with the target's exact two-range C shape under the one-object constraint.

- [s45] z1a's disassembly (tmp/grind/func_80034F88/s45x/z1a.txt) shows every bare-symbol access emitted as its own lui $at + memory-op macro pair, never a shared base register; a shared base (a1) appears only in blocks 2/3 where the declared pointer lives. Mechanism from source: GO_IF_LEGITIMATE_ADDRESS accepts a SYMBOL_REF address unconditionally (tools/gcc-2.7.2/config/mips/mips.h:2300), so the address costs nothing for cse to eliminate and no pseudo is created for it.

- [s45] update_equiv_regs (tools/gcc-2.7.2/local-alloc.c:947-1116) only ever DELETES a constant-equivalent pseudo: reg_equiv_replacement is set at local-alloc.c:1080-1082 and the initialiser is turned into NOTE_INSN_DELETED at local-alloc.c:1102-1115. Nothing between flow and reload creates a second hard register for one pseudo, and q is never spilled on any measured body.

- [s45] Synthesis: the residual is an ARITY fact rather than an allocation-heuristic fact. The six find_reg exits enumerated by s39/s41/s43/s45/s46 all route through a missing block-0-confined pointer quantity; s47 shows why that quantity cannot exist -- C creates a pointer pseudo aliasing &D_80106A73 only by naming a pointer object, and the second object is the Judge-closed multi-handle axis.

- [s45] A pointer derived from a different symbol with a non-zero addend (declaring D_80106A70 as an array and reaching the flag byte as D_80106A70[3]) forecloses on bytes, not on allocation: the target's accesses are all 0($reg) with %lo(D_80106A73) folded into the base, whereas an array-base pointer emits 3($reg) with %lo(D_80106A70). Already banked in shape as rejected/distinct-symbol-addend-four-bases-score28.c (28).

- [s45] candidate.c's BODY is unchanged this session (only the header comment gained the s47 note), so its review body key is unchanged. The preferred submission spelling remains rejected/s46-uniform-four-block-reload-no-m-BYTE-IDENTICAL-TO-b0-score10.c, and the `*(&D_80106A70 + i)` loop line remains a declaration pun that layer-1 will FAIL until the header declaration is fixed.

- [s45] src/code6cac_b.c was restored to HEAD (INCLUDE_ASM) before this outcome was written; no build-pipeline file, rule file, engine file or tool was modified. Ladder accounting unchanged: the six-modality condition was met at s31 and the floor has been flat at 10 since s26.

## s48 (solver, 2026-09-05; dispatched as "session 46") -- THE RESIDUAL IS PRE-RA, NOT RA: cse.c forwards the block-0 mask store into block-1's reload, and the missing `lbu` is the whole story

Floor unchanged: **10 at 49 insns** (candidate.c re-measured on today's HEAD).
All measurements are `sandbox func_80034F88 --disable all` with the body
spliced over `INCLUDE_ASM("asm/funcs", func_80034F88);`; `src/` was restored to
HEAD before this was written.  Harness: `tmp/grind/func_80034F88/s45x/splice.py`
plus a new runner `tmp/grind/func_80034F88/s46/{run.ps1,cnt.sh,slice.py}`.

### KILL RE-AUDIT (mandated)
- `run.ps1 memory/grind/func_80034F88/candidate.c` -> **score 10, build_insns 49**.
- `tools/fake_ablate.py` was already confirmed clean four sessions running
  (s41/s42/s46/s47); no FAKE construct exists on the chassis, so no banked kill
  can be FAKE-contaminated.
- The two closest banked kills (s44-block0-load-symbol-q-before-store,
  s39-block1-reads-symbol-directly) were re-measured at 11/50 by s47 on this
  same chassis and are not stale.

### THE CLASSIFIER VERDICT THAT OVERTURNS TEN SESSIONS OF FRAMING

`python3 tools/ra_solver/inverse_compose.py classify code6cac_b func_80034F88
--target-object build/src/code6cac_b.o --ours-object
tmp/grind/func_80034F88/s46/ours_cand.o`
(report: `tmp/grind/func_80034F88/s46/classify.txt`):

    func_80034F88 (code6cac_b): honest 49 insns, target 49 insns
    FIRST DIVERGENCE: PRE-RA
      instruction shapes present in ONE stream only (registers blanked):
        ours only  : nop
        target only: lbu #,0(#)

The two streams are the same LENGTH but not the same MULTISET.  The target
contains one `lbu` that our body does not emit, and our body contains one `nop`
where the target does not.  Sessions s38-s47 all reasoned as though the residual
were a pure register-seat problem on a fixed instruction multiset; it is not,
and no RA or scheduler model can express it.  **The s47 "arity proof" (two
pointer pseudos required, therefore the non-alias ladder is exhausted) is
withdrawn**: it was derived from an RA-only reading of a residual whose first
divergence is upstream of RA.

### WHICH INSTRUCTION, AND WHICH PASS EATS IT (read from the dumps, not guessed)

The target reloads the flag byte at the head of flag block 1:

    80034FAC  sb   $a0, 0($v1)      <- block-0 mask store
    80034FB0  lw   $v0, 0x20($a1)
    80034FB4  lbu  $a0, 0($v1)      <- BLOCK-1 RELOAD (the missing insn)

Our body does not, and the vacated load-delay slot after the `lw` becomes a
`nop`.  `pwsh tools/grinder/dump.ps1 func_80034F88` on the a1 body (slices at
`tmp/grind/func_80034F88/s46/{rtl,cse,combine}_slice.txt`):

- `.rtl` insn 29 is the honest reload:
  `(set (reg:QI 80) (mem:QI (reg/v:SI 74)))`, consumed by insn 30
  `(set (reg/v:SI 77) (zero_extend:SI (reg:QI 80)))`.
- `.cse` insn 29 has already been rewritten to
  `(set (reg:QI 80) (subreg:QI (reg:SI 76) 0))` -- **cse.c store-forwarded the
  block-0 mask store (insn 20,
  `(set (mem:QI (reg 74)) (subreg:QI (reg:SI 76) 0))`) into the block-1 load.**
- `.combine` folds 29+30 into `(set (reg 77) (reg 76))` and turns insn 29 into
  `NOTE_INSN_DELETED`; the copy is later coalesced away by the allocator.

The pass is **cse.c**, and the block-1 value pseudo therefore never becomes an
independent quantity -- which is exactly why every RA study since s38 found the
block-0 value and the block-1 value fighting over one seat.

### WHY BLOCKS 2 AND 3 KEEP THEIR RELOADS AND BLOCK 1 DOES NOT

The `.cse` dump prints its extended-basic-block partition:

    ;; Processing block from 2 to 46, 13 sets.     <- block-0 store AND block-1 load
    ;; Processing block from 49 to 80, 8 sets.     <- block 2
    ;; Processing block from 83 to 114, 8 sets.    <- block 3

Blocks 2 and 3 begin after the join `code_label` of the preceding flag test, so
cse restarts with an empty table and their `lbu`s survive.  Block 1's load sits
inside the same EBB as the block-0 store, so cse has the stored value in hand.
The residual is a cse extended-basic-block boundary problem, not an allocator
problem.

### SEVEN NEW MEASUREMENTS THAT LOCATE THE LEVER

| body | shape | score / insns | lbu / nop |
|---|---|---|---|
| a1 (base) | uniform four-block reload, one `u8 *q` | 10 / 49 | 4 / 3 |
| v1 | + `q = &D_80106A73;` re-assigned at block-1's head | 10 / 49 | 4 / 3 |
| v3 | block-1 read duplicated into both arms (split-read shape) | 10 / 49 | 4 / 3 |
| v4 | block-0 mask through a NON-UNIFIABLE address (`*(&D_80106A78 - 5)`, diagnostic only) | 15 / 51 | **5 / 2** |
| v5 | whole block-0 mask duplicated into both arms of `if (p[8] & 1)` | 18 / 50 | **5 / 3** |
| v6 | mask load+`&0xF8` unconditional, only the STORE duplicated into the arms | **12 / 50** | **5 / 3** |
| v8 | v6 with the splitting condition `if (m)` | 14 / 53 | -- |
| v9 | mask fully inside the arms, condition on the raw byte | 19 / 53 | -- |

Readings:

1. **Ordinary reorderings do not defeat the forwarding.**  Re-assigning `q`
   (v1) and duplicating the block-1 READ into the arms (v3) both leave 4 `lbu`
   and score exactly 10 -- cse's table is keyed on the address VALUE, and every
   spelling that keeps one pointer pseudo hands it the same value class.
2. **Address-value non-unification restores the target multiset.**  v4 reaches
   5 `lbu` / 2 `nop` because cse cannot prove
   `(plus (symbol_ref D_80106A78) (const_int -5))` equal to
   `(symbol_ref D_80106A73)`.  It costs +2 insns (a second `lui`/`addiu`), so it
   is diagnostic only, but it is the first body in 48 sessions to reproduce the
   target's instruction multiset.
3. **v4 and v5 reproduce the target's block-0/1 REGISTERS exactly, with no
   allocator lever at all.**  v5's opening is
   `lui $v1 / addiu $v1 / lbu $a0,0($v1) / andi $a0 / sb $a0,0($v1) / lw /
   lbu $a0,0($v1) / andi / bnez / ori $v0,$a0 / move $v0,$a0 / sb $v0,0($v1)`
   -- address in `$v1`, value in `$a0`, i.e. the target's disposition, obtained
   purely by restoring the reload.  The 10-session hunt for a
   "block-0-confined pointer quantity" was chasing a consequence, not a cause.
4. **A duplicated-into-arms STORE gives the EBB break for almost free.**  v6
   puts only `*q = m;` in the two arms of `if (p[8] & 1)`; the branch, its `lw`
   and its `andi` all disappear (cse2 unifies the condition with flag block 1's
   own `p[8] & 1`, and jump2 cross-jumps the identical arms).  Net cost is
   **one insn**, and that insn is the load-delay `nop`, not the branch.
5. **The remaining insn is a sched1 priority artefact.**  In the base body BB0
   holds the chain `lbu -> andi -> sb -> lw -> andi -> bnez` (the `sb` blocks
   the `lw` by memory dependence), so the `lbu` is the longest-path insn and the
   `$v0 -> p` copy sinks into its load-delay slot -- which is exactly what the
   target does (`move $a1,$v0` at 80034FA4).  Moving the `sb` into the arms
   breaks that chain, the copy's path to the branch becomes the longest one, and
   sched1 emits `move $a1,$v0` BEFORE the `lbu`, leaving the slot empty.
6. **The splitting condition is load-bearing.**  v8 (`if (m)`) and v9 (condition
   on the raw byte) are not absorbed by cse2/jump2 and their branches survive at
   +4 insns (53).  Only a condition textually identical to flag block 1's own
   `p[8] & 1` collapses to zero cost.

- [s48] classify (object-level, target=build/src/code6cac_b.o) reports FIRST DIVERGENCE: **PRE-RA** at 49 vs 49 insns -- ours has an extra `nop`, the target an extra `lbu #,0(#)`. The residual is an instruction-multiset difference, so RA and scheduler models cannot express it and the s47 arity/exhaustion conclusion is withdrawn.
- [s48] Pass attribution from the dumps: cse.c store-forwards the block-0 mask store (`.rtl` insn 20) into block-1's reload (`.rtl` insn 29 becomes `(set (reg:QI 80) (subreg:QI (reg:SI 76) 0))` in `.cse`), and combine folds 29+30 into a copy and deletes 29 (`.combine`). The target keeps that `lbu` at 80034FB4.
- [s48] The `.cse` EBB partition explains the asymmetry: block-1's load is inside the same extended basic block as the block-0 store (`;; Processing block from 2 to 46`), while blocks 2 and 3 start after their join code_labels (`from 49 to 80`, `from 83 to 114`) and therefore keep their reloads.
- [s48] v1 (q re-assigned at block-1's head) and v3 (block-1 read duplicated into both arms) both measure 10 at 49 with 4 lbu / 3 nop: ordinary reorderings that keep ONE pointer pseudo cannot defeat cse's store forwarding, because cse keys on the address value class.
- [s48] v4 (block-0 mask through the non-unifiable address `*(&D_80106A78 - 5)`, diagnostic only) measures 15 at 51 with **5 lbu / 2 nop** -- the target's exact instruction multiset, and the first body in 48 sessions to reach it.
- [s48] v4 and v5 reproduce the target's block-0/1 register disposition exactly (address `$v1`, value `$a0`, `move $v0,$a0`, `sb $v0,0($v1)`) with no allocator lever present. Restoring the reload is sufficient to produce the seat the ledger spent s38-s47 trying to buy from find_reg.
- [s48] v5 (whole mask duplicated into both arms of `if (p[8] & 1)`) = 18 at 50, 5 lbu / 3 nop. v6 (only the STORE `*q = m;` duplicated into the arms) = **12 at 50**, 5 lbu / 3 nop -- the branch, its `lw` and its `andi` are all removed because the condition is textually flag block 1's own `p[8] & 1`; the ONLY surviving cost is one load-delay nop.
- [s48] v8 (`if (m)`) = 14 at 53 and v9 (condition on the raw byte) = 19 at 53: a splitting condition that is not the same expression as flag block 1's own test is NOT absorbed, and the branch survives at +4 insns.
- [s48] The v6 residual is a sched1 priority artefact, not an allocation one: with the `sb` moved into the arms, BB0 loses the memory dependence `sb -> lw` that made the `lbu` the longest-path insn, so `move $a1,$v0` outranks the `lbu` and no longer sinks into its load-delay slot.
- [s48] src/code6cac_b.c was restored to HEAD (INCLUDE_ASM) before this outcome was written; no build-pipeline file, rule file, engine file or tool was modified.

- [s46] Kill re-audit on today's chassis: memory/grind/func_80034F88/candidate.c re-measures score 10 at 49 build_insns; fake_ablate has reported no FAKE-annotated construct to ablate for four consecutive sessions (s41, s42, s46, s47), so no banked kill is FAKE-contaminated.

- [s46] inverse_compose.py classify (object-level path, target build/src/code6cac_b.o) reports FIRST DIVERGENCE: PRE-RA at 49 vs 49 instructions: ours only 'nop', target only 'lbu #,0(#)'. RA and scheduler models cannot express an instruction-multiset difference.

- [s46] The missing instruction is identified off asm/funcs/func_80034F88.s as the block-1 reload lbu $a0,0($v1) at 80034FB4, sitting in the load-delay slot of lw $v0,0x20($a1) at 80034FB0; our body emits a nop there.

- [s46] Pass attribution read from cc1 -da dumps, not guessed: .rtl insn 29 (set (reg:QI 80) (mem:QI (reg 74))) has already become (set (reg:QI 80) (subreg:QI (reg:SI 76) 0)) in .cse -- cse.c store-forwarded the block-0 mask store (insn 20) -- and .combine folds 29+30 into (set (reg 77) (reg 76)) with insn 29 marked NOTE_INSN_DELETED.

- [s46] The .cse dump's extended-basic-block partition shows block-1's load inside the same EBB as the block-0 store (';; Processing block from 2 to 46'), while flag blocks 2 and 3 start after their join code_labels ('from 49 to 80', 'from 83 to 114') and therefore keep their reloads. The asymmetry the ledger attributed to allocation is a cse EBB effect.

- [s46] v1 (q re-assigned at block-1's head) = 10 at 49 with 4 lbu / 3 nop, and v3 (block-1 read duplicated into both arms) = 10 at 49 with 4 lbu / 3 nop: ordinary reorderings that keep one pointer pseudo cannot defeat cse store forwarding, because cse keys on the address value class.

- [s46] v4 (mask through the non-unifiable address *(&D_80106A78 - 5), diagnostic only) = 15 at 51 with 5 lbu / 2 nop -- the target's exact instruction multiset, reached for the first time in 48 sessions.

- [s46] v4 and v5 reproduce the target's flag block 0/1 registers exactly (address $v1, value $a0, ori $v0,$a0, move $v0,$a0, sb $v0,0($v1)) with no allocator lever, no second declared pointer object and no FAKE construct. Restoring the reload alone produces the seat that s38-s47 tried to buy from find_reg.

- [s46] v5 (whole mask duplicated into both arms of if (p[8] & 1)) = 18 at 50 with 5 lbu / 3 nop; v6 (only the store *q = m; duplicated into the arms) = 12 at 50 with 5 lbu / 3 nop, where the branch, its lw and its andi are all removed because the condition is textually flag block 1's own p[8] & 1.

- [s46] v8 (condition if (m)) = 14 at 53 and v9 (condition on the raw byte) = 19 at 53: the splitting condition must duplicate an expression the following code already computes, or the branch survives at +4 insns.

- [s46] v6's single extra instruction is a load-delay nop caused by sched1 priority: moving the sb into the arms removes the sb -> lw memory dependence that made the lbu BB0's longest-path insn, so move $a1,$v0 is emitted before the lbu instead of sinking into its delay slot.

- [s46] src/code6cac_b.c was restored to HEAD (INCLUDE_ASM) before this outcome was written; git status shows no modified build-pipeline file, rule file, engine file or tool. The only ledger writes are memory/grind/func_80034F88/{evidence.md,hypotheses.md,candidate.c header comment} and six new rejected/ forms.

## s47 (forensics) — the residual is now thirteen instructions wide and its remaining lever is a local-alloc seat

**Chassis re-audit (mandated).** The brief's CHASSIS CHECK printed "measurement
unavailable", so both anchors were re-measured before any new probe:
`s46/a1.c` = **score 10 at 49 insns**, 4 `lbu` / 3 `nop`; `s46/v6.c` = **score
12 at 50 insns**, 5 `lbu` / 3 `nop`. Both reproduce their banked values exactly.
`tools/fake_ablate.py` on the closest-to-target body reports *no FAKE-annotated
constructs found ... nothing to ablate* — no FAKE carrier occupies any contested
pseudo, so every s46/s47 seat measurement is a clean one.

**The session's result.** Sharing ONE `c0 = p[8] & 1` between the mask block's
splitting conditional and flag block 1's own conditional (`s2.c`, and the
equivalent `t2.c` with `c0` computed before the pointer) yields a body that is
**49 instructions with the target's exact multiset — 5 `lbu`, 2 `nop`**. That
is the first time in this ledger that an ordinary-C body has matched the
target's instruction COUNT and MULTISET simultaneously; s46's v4 reached the
multiset only at 51 instructions and only through a magic-offset pun that was
never submittable. Flag blocks 2 and 3, the copy loop and the epilogue are
byte-identical to the target. The entire residual is the thirteen instructions
of blocks 0 and 1.

**What is left, precisely.** Two things, and they are the same thing:

    ours (s2)   lui a0 ; addiu a0 ; move a1,v0 ; lw v0,0x20(a1) ; lbu v1,0(a0)
                andi v0,1 ; andi v1,0xf8 ; sb v1,0(a0) ; lbu v1,0(a0) ; bnez v0
                ori v0,v1,1 / move v0,v1 ; sb v0,0(a0)
    target      lui v1 ; addiu v1 ; lbu a0,0(v1) ; addu a1,v0,0 ; andi a0,0xf8
                sb a0,0(v1) ; lw v0,0x20(a1) ; lbu a0,0(v1) ; andi v0,1 ; bnez v0
                ori v0,a0,1 / addu v0,a0,0 ; [lui a0 ; addiu a0] ; sb v0,0(v1)

The address is in `$a0` and the values in `$v1`; the target has them the other
way round. `tools/ra_solver/local_extract.py --suggest` on the s2 body
(`s47/ra_local_s2.txt`) shows why: BB0 carries exactly TWO block-local
quantities — the `p[8]` load (reg 77, span 8–12) and the mask's QImode load
(reg 78, span 10–14) — and they OVERLAP, so `find_free_reg`'s numeric scan
(`local-alloc.c:2169-2171`, `:2247`) hands hard reg 2 to 77 and hard reg 3 to
78. Hard reg 3 is therefore already in the address pseudo's conflict row by the
time global alloc reaches it (`nrefs_census`: address pseudo 76, nrefs 12,
livelen 33, priority 10909, allocno rank 4 → hard 4). They overlap only because
sched1 emits the `lw p[8]` between the mask's `lbu` and its `andi`. **De-overlap
the two BB0 locals and both take hard reg 2, hard reg 3 falls free, and the
address takes it.** That is one lever, it is a scheduling lever, and
`tools/sched_solver` models it exactly.

**Two axes closed with measurements.**
- `extern volatile u8 D_80106A73;` DOES reproduce the target's 5-`lbu` / 2-`nop`
  multiset — and costs six instructions (19 at 55 on the uniform body, 21 at 56
  on v6), because `MEM_VOLATILE_P` → `do_not_record` (`cse.c:1943`) also blocks
  cse from unifying the three address materialisations. The volatile axis is
  dead on cost, before the two-prong gate is even consulted (and it would fail:
  no `volatile_extern_allowlist.txt` grant, no IRQ/MMIO writer in the census).
- s39's SImode round-trip (`m = *q; m = m & 0xF8;`) is codegen-INERT on the v6
  chassis — byte-identical output. It only ever moved anything while the store
  sat in BB0.

**The cse guard, enumerated once so no session re-derives it.** A QImode store
to a global escapes cse's hash table only via `flag_float_store` (FP only),
`in_libcall_block`, `sets[i].src_elt == 0`, a paradoxical-SUBREG dest, or a
`ZERO_EXTRACT` dest (`cse.c:7311-7340`). `src_elt == 0` needs `do_not_record`
from `canon_hash`: volatile MEM, PRE/POST_INC/DEC, CALL, `UNSPEC_VOLATILE`,
volatile `ASM_OPERANDS`, or a non-fixed hard reg under `SMALL_REGISTER_CLASSES`
(undefined on MIPS). `ZERO_EXTRACT` is unreachable because MIPS's `insv`
expander FAILs on any field that is not 32 bits wide and byte-aligned
(`mips.md:2901-2912`), so a 3-bit flag-field write lowers to a plain MEM SET.
Of the reachable spellings, volatile costs +6 and the value-class break costs +2
(s46 v4). **The only free route is the EBB boundary** — store inside a
conditional arm, read after the join — which is what v6/s2 already do.

**Chassis note for the next session.** `candidate.c` (score 10, 49 insns) is
still the lowest-SCORING body, but it has the WRONG multiset (4 `lbu`; the
reload is cse'd away). `rejected/s47-shared-cond-mask-store-in-arms-EXACT-TARGET-MULTISET-49insn-score13.c`
scores 13 and is far closer to the target's shape. Score-distance and
shape-distance are not the same ordering here; do not read "13 > 10" as "worse".
Work the s2 body.

- [s47] Chassis confirmed on HEAD 2026-09-05: a1 = score 10 at 49 insns (4 lbu / 3 nop); v6 = score 12 at 50 insns (5 lbu / 3 nop); honest floor unchanged at 10.

- [s47] s2/t2 (one shared c0 = p[8] & 1 driving both the mask's splitting conditional and flag block 1) = score 13 at 49 insns with 5 lbu and 2 nop -- the target's exact instruction count AND multiset, from ordinary C, one declared pointer object, no FAKE construct, no pun.

- [s47] In s2 the target and our body agree instruction-for-instruction from flag block 2 onward; the residual is the thirteen instructions of blocks 0/1, where address and value registers are swapped ($a0/$v1 instead of $v1/$a0) and sched1 orders the copy and lw ahead of the mask's lbu.

- [s47] local_extract --suggest on s2: BB0 carries two block-local quantities, reg 77 (p[8] load, span 8-12) -> hard 2 and reg 78 (mask QImode load, span 10-14) -> hard 3; their spans overlap, which is why hard 3 is consumed.

- [s47] nrefs_census on s2: the address pseudo is 76 (nrefs 12, livelen 33, priority 10909, allocno rank 4) and takes hard 4 because hard 3 is already in its conflict row; the .greg row for the equivalent v6 pseudo 75 reads ';; 75 conflicts: 72 74 75 80 81 84 85 88 89 2 3 29'.

- [s47] extern volatile u8 D_80106A73 reaches the target multiset but at 55/56 instructions (score 19/21) -- the volatile axis is closed on measured cost, before its two-prong gate is even reached.

- [s47] s39's SImode round-trip is byte-identical to the inline mask on the v6 chassis (diff of the disassembled objects is empty), so that confirmed s39 finding is chassis-specific and carries no lever here.

- [s47] cse.c's dest-recording guard (cse.c:7311-7340) plus canon_hash (cse.c:1880-1990) leave only two store-side routes to a surviving reload, both priced (+6 volatile, +2 value-class break); MIPS's insv predicate (mips.md:2901-2912) rules out the ZERO_EXTRACT/bitfield route entirely.

- [s47] fake_ablate on the closest-to-target body reports no FAKE-annotated constructs, so none of this session's seat measurements are confounded by a FAKE carrier.

## s48 (forensics) — the order is solved; the whole residual is ONE local-alloc seat

**Chassis re-measure (mandated).** `s47/s2.c` re-measures **13 at 49** on HEAD
2026-09-05, reproducing its banked value exactly. `fake_ablate` still reports no
FAKE-annotated construct anywhere in the bank, so no seat measurement in this
ledger is FAKE-contaminated (fifth consecutive session).

**The session's result, in one line.** Making the block-0 mask a PLAIN
straight-line statement (`m = *q & 0xF8; *q = m;`) instead of s39–s47's
duplicated-into-the-arms store gives blocks 0/1 the target's exact instruction
ORDER for free, and the entire remaining residual is the `$a0`/`$v1` seat
exchange caused by one local-alloc decision in BB0.

1. **`str1` — plain straight-line mask, one declared pointer object, no FAKE,
   no volatile — is 49 insns at score 10 with the target's block-0 order.**
   `lui/addiu, lbu, move $a1,$v0, andi 0xf8, sb, lw 0x20($a1), <nop>, andi 1,
   bnez, ori/move, sb`. The target is the same sequence with `lbu $a0,0($v1)`
   where we carry the `nop` and with the two registers exchanged. This is
   strictly closer in shape than `candidate.c`'s s39–s47 chassis at the same
   score, so `candidate.c` was replaced by it.

2. **s47's "volatile costs +6" kill is WRONG and is hereby corrected.** With
   `extern volatile u8 D_80106A73;`, `volatile u8 *q`, **and u8-typed value and
   result locals** (`vol3`), the body is **49 instructions, 5 `lbu` / 2 `nop`,
   score 10** — the target's exact count, multiset AND order, end to end. The
   +6 s47 measured was three `andi $x,$x,0xff` zero-extends (forced by *s32*
   value locals: a volatile QI load cannot be folded into `zero_extendqisi2`,
   so combine leaves the widening as its own insn) plus the extra `j` each of
   them pushes out of the if/else. `vol1` (s32 locals) = 19 at 55, `vol2`
   (u8 value, s32 result) = 12 at 49, `vol3` (u8 value + u8 result) = 10 at 49.
   volatile itself costs ZERO instructions here. Its only effect is to stop
   cse.c store-forwarding the block-0 store into block 1's read.
   (The volatile GATE is still unsatisfied — no `volatile_extern_allowlist.txt`
   grant, no IRQ/MMIO writer in the census — so `vol3` is diagnostic evidence,
   banked as `memory/grind/func_80034F88/candidate_vol3_volatile_exact_order.c`,
   not a candidate.)

3. **The residual is named exactly, from the dumps and the models, not
   inferred.** `local_extract --suggest` on `vol3`: BB0 holds two block-local
   quantities — the mask value (reg 74, birth 6, death 10, 4 refs) takes hard 3,
   and the p[8] value (reg 80, birth 14, death 20) takes hard 2. They do NOT
   overlap; s47's "the two BB0 locals overlap" was an artefact of the
   arms chassis and is superseded. `find_free_reg`
   (`tools/gcc-2.7.2/local-alloc.c:2169-2247`) builds
   `used = fixed_reg_set | regs_live_at[birth..death) | ~reg_class_contents`;
   hard 2 is in it because the call return `$v0` is still live over [6,10)
   (the `move $a1,$v0` copy is scheduled after the mask `lbu` — exactly as in
   the target), MIPS defines **no** `REG_ALLOC_ORDER`
   (`grep REG_ALLOC_ORDER tools/gcc-2.7.2/config/mips/mips.h` returns no hit), so the
   scan is numeric from 0 and the mask value takes **hard 3**. That single
   choice puts hard 3 into the blocks-0/1 address allocno's conflict row
   (`.greg`: `;; 75 conflicts: 72 75 78 79 83 84 88 89 2 3 29` — 75 is the only
   allocno with a hard-3 conflict), so `global.c` `find_reg` seats the address
   at hard 4 = `$a0`. The target seats it at hard 3 = `$v1`.

4. **The allocno order is not the lever.** `nrefs_census` on `vol3`:
   ord 0 = reg 73 (loop counter, pri 47142) takes hard 3, but it does NOT conflict
   with the address; ord 1 = reg 75 (address, nrefs 11, livelen 29, pri 11379)
   takes hard 4; ords 2–7 = the six flag-block value pseudos (nrefs 3, livelen 4,
   pri 7500 each); ord 8 = reg 72 (`p`, pri 3529) takes hard 5. The address is
   already the first allocno that wants hard 3; it loses it to a hard-reg
   conflict written by local-alloc, not to another allocno.

5. **The bare-symbol route to a second address register is dead on this
   chassis too (kill re-audit of the s47 instance kill).** On this port a bare
   global access never acquires a `lui/addiu` base register: it lowers to
   `lui $at` + `%lo(sym)($at)`. Measured: `str2`/`vol5` (bare everywhere)
   = 29/25 at 47 (cse unifies the addresses and two insns vanish);
   `str3`/`vol6` (pointer in 0/1, bare in 2/3) = 28/27 at 48/47;
   `str4`/`vol7` (pointer in block 0 only) = 22/24 at 52/48;
   `str5`/`vol8` (bare in 0/1, pointer in 2/3) = 29/15 at 47/49, and `vol8`'s
   block 0 is visibly `lui $v1 / lbu $v1,0($v1) / ... / lui $at / sb $v1,0($at)`
   — the wrong addressing FORM, not merely the wrong register.

6. **The BANNED multi-handle axis does not close the function either.**
   Measured (diagnostically, reverted immediately, never proposed):
   `tw_vol` = two pointer objects on the vol3 chassis = **21 at 49**;
   `tw_str` = the same without volatile = 21 at 49. `tw_vol` reproduces the
   target's STRUCTURE exactly — including the second `lui/addiu` pair emitted
   *before* the block-1 `sb`, which no single-object body can do — but the two
   address allocnos come out **exchanged**: `nrefs_census` on `tw_vol` gives
   ord 1 = reg 76 (blocks 2/3 address, nrefs 6, livelen 16, pri 7500) taking hard 3
   and ord 8 = reg 75 (blocks 0/1 address, nrefs 5, livelen 26, pri 3846) taking
   hard 4, while the target wants 75 at hard 3 and 76 at hard 4. And reg 75 still carries
   the same hard-3 conflict from the same BB0 local-alloc decision. So the
   ban is NOT what is holding this function at 10: the lever is upstream of the
   handle count, and a ruling-request to reopen the multi-handle axis would
   have been spent on a body that measures 21.

- [s48] s47/s2.c re-measures 13 at 49 on HEAD 2026-09-05; fake_ablate reports no FAKE-annotated construct to ablate (fifth consecutive session).
- [s48] str1 (plain straight-line mask `m = *q & 0xF8; *q = m;`, one declared pointer object, no volatile, no FAKE) = score 10 at 49 insns, 4 lbu / 3 nop, and reproduces the target's block-0 instruction ORDER exactly; it replaces the s39-s47 arms chassis as candidate.c.
- [s48] vol3 (extern volatile u8 D_80106A73 + volatile u8 *q + u8-typed value and result locals) = score 10 at 49 insns with 5 lbu / 2 nop -- the target's exact instruction count, multiset and order end to end; the only difference is the $a0/$v1 seat exchange in blocks 0/1.
- [s48] s47's banked kill "extern volatile costs +6 instructions (55/56)" is CORRECTED: the +6 was three `andi 0xff` zero-extends forced by s32-typed value locals plus the extra `j` each pushes out of the if/else. vol1 (s32) 19@55, vol2 (u8 value/s32 result) 12@49, vol3 (u8 value+result) 10@49. volatile costs zero instructions on this chassis.
- [s48] local_extract --suggest on vol3: BB0's two block-local quantities are the mask value (reg 74, birth 6, death 10) taking hard 3 and the p[8] value (reg 80, birth 14, death 20) taking hard 2, and they do NOT overlap; s47's overlap finding was an artefact of the arms chassis.
- [s48] find_free_reg (local-alloc.c:2169-2247) excludes hard 2 over [6,10) because the call return $v0 is live there, and MIPS defines no REG_ALLOC_ORDER (no hit in tools/gcc-2.7.2/config/mips/mips.h), so the numeric scan hands the mask value hard 3 -- which is the sole source of the hard-3 entry in the address allocno's conflict row (.greg: ";; 75 conflicts: ... 2 3 29").
- [s48] nrefs_census on vol3: the address allocno (75, nrefs 11, livelen 29, pri 11379) is ord 1 and is already the first allocno that wants hard 3; it loses hard 3 to a local-alloc hard-reg conflict, not to another allocno. Allocno ORDER is therefore not the lever.
- [s48] Bare-symbol accesses never acquire a lui/addiu base register on this port -- they lower to "lui $at" + "%lo(sym)($at)". Re-measured on the new chassis: str2 29@47, vol5 25@47, str3 28@48, vol6 27@47, str4 22@52, vol7 24@48, str5 29@47, vol8 15@49.
- [s48] The BANNED two-pointer-object body, measured diagnostically and reverted, is 21 at 49 (tw_vol, volatile) and 21 at 49 (tw_str, non-volatile). It reproduces the target's structure exactly but comes out with the two address allocnos EXCHANGED (nrefs_census: reg 76 pri 7500 to hard 3, reg 75 pri 3846 to hard 4; target wants the reverse), and reg 75 still carries the same hard-3 conflict from the same BB0 local-alloc decision. The multi-handle ban is not what holds this function at 10.
- [s48] src/code6cac_b.c was restored to HEAD (INCLUDE_ASM) before this outcome was written; git status shows only metrics/events.jsonl modified. No build-pipeline file, rule file, engine file or tool was touched.

- [s48] s47/s2.c re-measures 13 at 49 on HEAD 2026-09-05, reproducing its banked value; fake_ablate reports no FAKE-annotated construct to ablate (fifth consecutive session), so no seat measurement in this ledger is FAKE-contaminated.

- [s48] str1 (plain straight-line `m = *q & 0xF8; *q = m;`, one declared pointer object, no volatile, no FAKE) = score 10 at 49 insns, 4 lbu / 3 nop, and reproduces the target's block-0 instruction ORDER exactly; it is the new candidate.c.

- [s48] vol3 (extern volatile u8 D_80106A73 + volatile u8 *q + u8-typed value and result locals) = score 10 at 49 insns with 5 lbu / 2 nop - the target's exact instruction count, multiset AND order end to end; the only difference is the $a0/$v1 seat exchange in blocks 0/1.

- [s48] s47's kill 'extern volatile costs +6 instructions (55/56)' is corrected: the +6 was three `andi 0xff` zero-extends forced by s32-typed value locals plus the extra `j` each pushes out of the if/else (vol1 19@55, vol2 12@49, vol3 10@49).

- [s48] local_extract --suggest on vol3: BB0's two block-local quantities are the mask value (reg 74, birth 6, death 10, refs 4) at hard 3 and the p[8] value (reg 80, birth 14, death 20) at hard 2, and their spans are disjoint - s47's overlap finding was an artefact of the arms chassis.

- [s48] find_free_reg (tools/gcc-2.7.2/local-alloc.c:2169-2247) excludes hard 2 over [6,10) because the call return $v0 is live there, and MIPS defines no REG_ALLOC_ORDER (no hit for REG_ALLOC_ORDER in tools/gcc-2.7.2/config/mips/mips.h), so the numeric scan hands the mask value hard 3.

- [s48] .greg on vol3: `;; 75 conflicts: 72 75 78 79 83 84 88 89 2 3 29` - reg 75 (the blocks-0/1 address) is the only allocno in the function carrying a hard-3 conflict, and that conflict is written by local-alloc.

- [s48] nrefs_census on vol3: ord 0 reg 73 (loop counter, pri 47142) at hard 3 but no conflict with the address; ord 1 reg 75 (address, nrefs 11, livelen 29, pri 11379) at hard 4; ords 2-7 the six flag-block value pseudos (nrefs 3, livelen 4, pri 7500); ord 8 reg 72 (p, pri 3529) at hard 5. Allocno ORDER is not the lever.

- [s48] Bare-symbol accesses never acquire a lui/addiu base register on this port - they lower to `lui $at` + `%lo(sym)($at)`: str2 29@47, vol5 25@47, str3 28@48, vol6 27@47, str4 22@52, vol7 24@48, str5 29@47, vol8 15@49.

- [s48] The banned two-pointer-object body, measured diagnostically and reverted, is 21 at 49 on both the volatile and non-volatile chassis, with the two address allocnos exchanged relative to the target - the multi-handle ban is not what holds this function at 10.

- [s48] src/code6cac_b.c was restored to HEAD (INCLUDE_ASM) before this outcome was written; `git status --porcelain src/` is empty. No build-pipeline file, rule file, engine file or tool was modified; the only writes are under memory/grind/func_80034F88/ and tmp/grind/func_80034F88/s48/.

## s49 (rederive, 2026-09-05) -- the residual is now a single global.c priority comparison

Baseline re-measured at dispatch on HEAD with the s48 candidate installed:
`sandbox func_80034F88 --disable all` = **score 10, 49 target insns / 49 build
insns**.  The dispatch brief reported "measurement unavailable"; the ledger's
recorded floor of 10 is correct.

### 1. The block-1 reload is NOT the cause of the seat swap (kills the s46/s47/s48 premise)

Frontier item 2 of the s48 ledger asserted that the missing block-1 reload
(`lbu $a0,0($v1)` at 80034FB4) was the remaining lever, on the theory that it
adds a fourth reference to the blocks-0/1 address pseudo and splits the value
range.  Both banked escapes were re-priced on the CURRENT (straight-line-mask)
chassis this session and BOTH restore the reload without moving a single
register:

| body | spelling of the escape | insns | score | blocks-0/1 address | blocks-0/1 value |
|---|---|---|---|---|---|
| candidate (s48) | none -- cse forwards the store | 49 | 10 | `$a0` | `$v1` |
| `d_volread` | `v = *(volatile u8 *)q;` | 51 | 13 | `$a0` | `$v1` |
| `d_addrbreak` | `v = *(&D_80106A78 - 5);` | 50 | 11 | `$a0` | `$v1` |

`d_volread` costs +2 (`andi $v1,$v1,0xff` because the volatile QI read is not
folded, plus a `j` because the branch structure changes).  `d_addrbreak` costs
+1 (its own `lui`, because the reload goes through a second address).  Neither
changes the disposition of the address pseudo.  The reload and the seat are two
INDEPENDENT residuals, and the seat is the larger one.  (Both spellings are
inadmissible anyway -- volatile-coercion-by-cast and a declaration pun -- they
were run as diagnosis only and are banked in `rejected/` for that value.)

### 2. What actually blocks hard 3, read out of the allocator dumps

`pwsh tools/grinder/dump.ps1 func_80034F88`, dumps in
`tmp/grind/func_80034F88/dumps/`.  Reg 75 is the `&D_80106A73` pointer pseudo;
the target seats it at hard 3 (`$v1`) across blocks 0/1, we seat it at hard 4
(`$a0`).

On the s48 body the `.greg` conflict row is

    ;; 75 conflicts: 72 75 78 79 82 83 86 87 2 3 29

i.e. **hard 3 is a CONFLICT**, and `.lreg` says why:

    Register 74 used 3 times across 6 insns in block 0
    Register 76 used 2 times across 4 insns in block 0; 1 bytes
    ;; Register 74 in 3.   ;; Register 76 in 3.

Two block-0-LOCAL quantities -- reg 76, the QImode temp holding the raw `lbu`
result, and reg 74, the masked value -- are seated by local-alloc at hard 3
before global_alloc ever runs.  `find_free_reg` (local-alloc.c:2169-2247) builds
`used` = fixed_reg_set | regs_live_at[born..dead) | ~GR_REGS, scans hard regs
NUMERICALLY (mips.h defines no `REG_ALLOC_ORDER`; verified by grep this
session), and hard 2 is excluded because the call return `$v0` is still live at
the mask `lbu`.  So a block-0-local quantity ALWAYS lands on hard 3, and hard 3
always ends up in reg 75's conflict row.

### 3. Both block-0-local quantities can be deleted by ordinary C

Two source edits, each ordinary C, remove them:

  1. split the mask into two statements -- `m = *q; m &= 0xF8;` instead of
     `m = *q & 0xF8;`.  The single-statement form is narrowed by `fold` to a
     QImode AND (the constant fits in QI), which is what materialises the
     QImode temp reg 76.  Two statements keep one SImode pseudo.
  2. let block 0's arms consume `m` directly instead of re-reading `v = *q;`.
     The re-read was cse-forwarded to `m` anyway and survived only as
     `(set (reg 78) (reg 74))`, a copy that kept `m` block-0-local.

With both edits (the new `candidate.c`) the conflict row becomes

    ;; 75 conflicts: 72 74 75 77 79 80 83 84 2 29

**hard 3 is gone from reg 75's conflicts.**  This is the first body in 49
sessions where the target's seat is legal for the address allocno.  Emitted
instruction order, count (49) and multiset are unchanged from s48; score is
still 10.

### 4. What is left: allocno_compare, with exact numbers

`global.c allocno_compare` sorts by

    pri = floor_log2(n_refs) * n_refs / live_length * 10000

On the new candidate (`.lreg` counts):

    reg 74 (masked value m):    refs=6   len=9    pri=13333
    reg 75 (&D_80106A73 ptr):   refs=10  len=28   pri=10714

so `;; 9 regs to allocate: 73 77 80 84 74 75 79 83 72` -- 74 is allocated
BEFORE 75, takes the lowest free hard reg (2 excluded by the live `$v0`), i.e.
hard 3, and 75 falls to hard 4.  The target's compile must have had the
opposite order.

There is no third-allocno route: any allocno that conflicts with 74 also
conflicts with 75 (74's live range is contained in 75's), so nothing can be
introduced that pushes 74 off hard 3 without also pushing 75 off it.  The only
route is the priority comparison itself.

Thresholds, from the formula (all measured, not inferred):
  - lower 74: `refs <= 4` at len 9 (pri 8888), or `len >= 12` at refs 6 (10000).
    `refs = 5` at len 9 gives 11111 and is still NOT enough.
  - raise 75: `refs >= 13` at len 28 (13928), or `len <= 22` at refs 10 (13636).

### 5. The dilemma both mask spellings sit in

  (a) ONE-STATEMENT mask -- `m = *q & 0xF8;` and every equivalent tried this
      session (`& ~7`, `& -8`, `& 0x1F8`, `& 0xFFF8`, `(*q >> 3) << 3`): the
      allocno ORDER is already correct (`... 75 74 ...`), 74 drops to 4 refs,
      but fold still narrows the load and the 2-ref block-0-local QImode temp
      takes hard 3, so 75 is blocked.  All five spellings measured score 10 with
      `75 in 4` and a block-0-local at hard 3.
  (b) TWO-STATEMENT mask (the new candidate): no block-0-local at all, but the
      masked value is a 6-ref/9-length global that outranks the pointer.

A winning form needs BOTH: no block-0-local quantity AND pri(75) > pri(74).

### 6. Things measured this session that do NOT move either number

  - `rd7`: computing `c = p[8] & N;` before each `q = &D_80106A73;`
    re-assignment.  sched1 normalises the order back; reg 75 stays refs=10
    len=28.
  - `rd8`: an extra `q = &D_80106A73;` immediately before block 0's store, to
    split reg 75's block-0 live interval.  cse deletes the redundant def; refs
    and length are unchanged.
  - `fake_ablate` on the candidate: "no FAKE-annotated constructs found" -- the
    kill re-audit has nothing to ablate, the chassis carries no FAKE carrier.

Artifacts: `tmp/grind/func_80034F88/s49/bodies/*.c`,
`tmp/grind/func_80034F88/s49/ra.py` (prints allocno refs/length/priority and
the .greg conflict rows for the installed body -- reusable),
`tmp/grind/func_80034F88/s49/install.py` + `m.sh` (install-and-score harness),
`tmp/grind/func_80034F88/s49/fake_ablate.txt`,
`tmp/grind/func_80034F88/dumps/code6cac_b.{lreg,greg}`.

- [s49] Dispatch chassis re-measured: sandbox func_80034F88 --disable all = score 10, 49 target insns / 49 build insns on HEAD with the s48 candidate installed (the brief reported 'measurement unavailable'; the ledger floor of 10 is correct).

- [s49] The whole residual is two facts: (i) the blocks-0/1 &D_80106A73 pointer allocno (reg 75) must be seated at hard 3 ($v1) instead of hard 4 ($a0), and (ii) the block-1 reload must survive cse. This session proves (ii) does not cause (i).

- [s49] mips.h defines no REG_ALLOC_ORDER (grep over tools/gcc-2.7.2/config/mips/mips.h returns nothing), so both find_free_reg (local-alloc.c:2251) and global.c scan hard registers numerically from 0.

- [s49] Any block-0-local quantity in this function is seated at hard 3 by local-alloc, because hard 2 is excluded by the call return $v0 still being live at the mask lbu and the scan is numeric.

- [s49] The s48 candidate carried TWO block-0-local quantities (.lreg: reg 74 '3 times across 6 insns in block 0', reg 76 '2 times across 4 insns in block 0; 1 bytes'; tail: ';; Register 74 in 3.' and ';; Register 76 in 3.'), which is what put hard 3 into reg 75's conflict row.

- [s49] The new candidate's .greg conflict row for the pointer allocno is ';; 75 conflicts: 72 74 75 77 79 80 83 84 2 29' -- hard 3 absent. The s48 row was ';; 75 conflicts: 72 75 78 79 82 83 86 87 2 3 29'.

- [s49] global.c allocno_compare priority is floor_log2(n_refs) * n_refs / live_length * 10000; on the new candidate reg 74 = 6 refs / 9 length / 13333 and reg 75 = 10 refs / 28 length / 10714, giving the allocation order '73 77 80 84 74 75 79 83 72'.

- [s49] No third-allocno route exists on this body: reg 74's live range is contained in reg 75's, so every allocno that conflicts with 74 also conflicts with 75.

- [s49] The candidate emits the target's exact block-0/1 instruction order, count (49) and multiset; the diff is register names plus the missing block-1 reload and the cascaded position of block 2's lui/addiu pair.

- [s49] tools/fake_ablate.py on the candidate reports 'no FAKE-annotated constructs found' -- the mandated kill re-audit had no FAKE carrier to ablate, so it was discharged instead by re-pricing the s46/s47 reload escapes on the current chassis (first hypothesis above).

- [s49] tmp/grind/func_80034F88/s49/ra.py is a reusable one-command reader: it prints the allocation order, every .greg conflict/preference row, the register dispositions, and refs/live_length/allocno priority for every pseudo of the installed body.

## s50 (rederive, 2026-09-05) -- the seat mechanism is solved; the residual is re-scoped

- `global.c:635-656 allocno_compare` is the exact gate, and its formula includes
  `allocno_size` (words): `pri = floor_log2(n_refs) * n_refs / live_length *
  10000 * size`.  All allocnos here are SImode (size 1), so only refs and
  live_length are movable.
- **The pointer allocno's seat IS movable by ordinary C.**  Duplicating the
  flag-block store into both arms of flag blocks 1 and 2 lifts reg 75 from
  refs=10/len=28/pri=10714 to refs=12/len=26/pri=13846, above reg 74's 13333,
  and the dispositions become `75 in 3  74 in 4`.  Block 0 then emits the
  target's registers exactly.  Duplicating in all three flag blocks gives
  refs=13/len=25/pri=15600 and the same seat.  Cross-jump (`jump2`) runs after
  reload, so the duplicated stores are counted by global_alloc and merged
  afterwards -- the mechanism the duplicated-statement-into-arms rule names.
- **The target needs TWO allocnos for `&D_80106A73`**, not one seat: `$v1` for
  the mask + flag block 0 (materialised at 80034F98, last used by the store at
  80034FD0) and `$a0` for flag blocks 1 and 2 (80034FC8, 80034FF0).  With one
  declared pointer object there is one pseudo, one allocno, one hard register,
  and GCC 2.7.2's global_alloc has no live-range splitting.  Both seats were
  measured: hard 4 -> score 10 (flag blocks 1/2 exact), hard 3 -> score 16..18
  (block 0 exact).  The floor-10 residual is therefore "a second address
  allocno over the block-0 region", not "the wrong allocation priority".
- **Direct symbol references cannot supply that second allocno.**  A plain
  `D_80106A73` lvalue compiles to the `lbu $x,D_80106A73` / `sb $x,D_80106A73`
  assembler macro (2 insns via `$at`), never to `lui/addiu` + `0($reg)`; it also
  pushes `p` from `$a1` to `$a2`.  Scores 25..30.
- **The two-object axis (BANNED; diagnosed only) does not reach 0 either.**
  Eight two-object bodies measure 21..26.  The blocker there is the same
  priority race one level down: `pri(t)` = 3333..4545 against `pri(m)` = 13333,
  so the masked value still wins hard 3.  A two-object body would need
  `refs(t) >= 13` at live_length 22 to reach the target seat; duplicated stores
  and duplicated reads got it to 5.  Conclusion for the record: the standing
  multi-handle ban is NOT demonstrably what costs the 10 points.
- Register roles on the current chassis, for the next session (from `.lreg`/
  `.greg` via `tmp/grind/func_80034F88/s50/ra.py`): reg72 = `p` (refs 6, len 33,
  hard 5 = `$a1`, matches target), reg73 = loop counter `i` (refs 11, len 7,
  pri 47142, hard 3, matches target), reg74 = block-0 masked value, reg75 =
  `&D_80106A73`, reg77/80/84 = the three `p[8] & N` conditions (hard 2 = `$v0`,
  match target), reg79/83 = flag block 1/2 values (hard 3 = `$v1`, match
  target).  Everything except reg74/reg75 already matches the target.

- [s50] global.c:635 allocno_compare's priority formula includes allocno_size (words); every allocno in this function is SImode (size 1), so only n_refs and live_length are movable by C.

- [s50] The duplicated-statement-into-arms family is a working, measured priority lever on THIS function: +1 reg_n_refs per duplicated store on the pointer pseudo, counted at global_alloc, merged away later by jump2 cross-jumping which runs after reload.

- [s50] Threshold arithmetic, re-derived and now half-spent: the pointer allocno beats the masked value at refs>=12 with live_length<=26 (13846) or refs=13 at 25 (15600); the live_length route (>=12 at refs 6) is measured shut because sched1 shortens rather than lengthens that range.

- [s50] The target holds &D_80106A73 in two hard registers: $v1 for the mask + flag block 0 (materialised 80034F98, last used 80034FD0) and $a0 for flag blocks 1 and 2 (80034FC8, 80034FF0).

- [s50] Everything except reg74 (block-0 masked value) and reg75 (&D_80106A73) already matches the target on the current chassis: reg72 p in $a1, reg73 loop counter in $v1, reg77/80/84 conditions in $v0, reg79/83 flag block 1/2 values in $v1.

- [s50] A direct D_80106A73 lvalue compiles to the two-insn assembler macro through $at, never to the target's lui/addiu + 0($reg) form, and additionally moves p from $a1 to $a2.

- [s50] Two-object bodies (banned; diagnosis only) bottom out at 21, so the multi-handle ban is not demonstrably the thing costing the 10 points.

## s51 (structural, 2026-09-05) -- allocation facts measured this session

Chassis check: `sandbox func_80034F88 --disable all` on HEAD with
`memory/grind/func_80034F88/candidate.c` installed = score 10, 49 build insns
(target 49). Confirmed twice this session (`p_base_recheck.c` is a
character-for-character regeneration of the same body and also measures 10/49).

Tooling added: `tmp/grind/func_80034F88/s51/rep.sh` -- installs a body, prints
sandbox score/insns, then prints the .greg allocation order, the conflict and
preference rows for the two contested allocnos, the full register dispositions,
and refs/live_length/allocno_compare priority for every pseudo from .lreg. This
makes the allocation race readable in one call per body and should be reused.

### The block-0 allocation race, measured on eight shapes
The target seats the &D_80106A73 address in $v1 (hard 3) for the mask and flag
block 0, and the masked value in $a0 (hard 4). This body does the inverse. Two
independent conditions gate the flip and no measured shape satisfies both:

| block-0 shape | masked value refs/len/pri | pointer sorted first? | hard 3 in pointer's conflict row? | seat | score/insns |
|---|---|---|---|---|---|
| two-statement mask (candidate.c) | 6 / 9 / 13333 | no | no | 74 in 3, 75 in 4 | 10 / 49 |
| single-statement mask (r1) | 4 / 8 / 10000 | YES | YES (QI temp reg76 in 3) | 74 in 3, 75 in 4 | 10 / 49 |
| named raw s32 (w1) | 4 / 8 / 10000 | YES | YES (raw reg75 in 3) | 76 in 4 | 10 / 49 |
| named raw u8 (w2) | 4 / 8 / 10000 | YES | YES | 76 in 4 | 10 / 49 |
| raw live across the branch (x2) | -- | -- | YES, in both rows | 74 in 5, 75 in 4 | 17 / 49 |

Pointer allocno constant across all of these: refs=10 live_length=28 pri=10714.
The relevant local-alloc fact is s44's seat law: the short-lived block-0
quantity takes $v0 when $v0 is dead across its span and $v1 otherwise. $v0 is
live across it here because it still carries func_80077D00's return value until
the `move $a1,$v0`, so the temp always lands on $v1 and always poisons the
pointer's conflict row. local-alloc runs before global_alloc, so this decision
is made before the priority sort the single-statement mask wins.

### The block-1 reload is a cse address-equality effect (first reproduction)
Target 80034FB4 is `lbu $a0,0($v1)` -- a re-read of the flag byte after the
mask store. Every body in 50 prior sessions emitted a load-delay `nop` there.
Spelling the mask's address `(&D_80106A70 + 3)` and flag block 0's
`&D_80106A73` (same byte, two SYMBOL_REFs cse cannot equate) emits the reload
and gives block 0 the target's exact instruction sequence:
lui/addiu, lbu, move a1 v0, andi 0xf8, sb, lui/addiu, lw 32(a1), lbu, andi 1,
bnez, ori, move, sb. Cost: one extra address materialisation (51 insns vs 49)
and the address/value registers still swapped. Objdump:
`tmp/grind/func_80034F88/s51/dis_split.txt`. Score 12.

Consequences: the reload needs no `volatile`, no aliasing story and no
read-duplication -- duplicating the read into both arms of flag block 0, in all
four spellings, is folded away by cse (the stored value is provably 8-bit clean,
so the zero_extend folds to the live masked-value pseudo) and measures 10/49 or
12/46. The open composition problem is that sharing ONE address materialisation
between the mask and flag block 0 is exactly what lets cse forward the store.

### Address spelling is inert on this chassis (kill re-audit)
`(&D_80106A70 + 3)` substituted for `&D_80106A73` measures 10/49 in the mask
alone, in flag blocks 1 and 2 alone, and in both -- identical to the base. The
s16/s17/s25 measurements of 12-14 for the same substitution were chassis
artifacts; those kills are re-audited and re-scoped to "neutral, not harmful".
The practical consequence for the DATA MODEL signal: declaring D_80106A70 as a
4-byte array is worth carrying only as the pun-free spelling of the trailing
copy loop (s38's measured-neutral integration handoff), not as a codegen lever.

### Shapes measured and banked this session
p_base_recheck 10/49, p_mix0_a70 10/49, p_mix12_a70 10/49, p_mix_alt_a70_b1
10/49, p_mix0_a70_reload 10/49, p_mix0_a70_dup12 18/51, p_mix0_a70_dupall
20/50, p_mix0_a70_rel_dup12 16/51, q_split_A70mask_A73rest 12/51,
q_split_A73mask_A70b0 12/51, q_split_A73mask_A70b0_A73b12 12/51,
q_split_*_noreassign12 32/51, q_split_A70mask_A73b0_dup12 18/53,
r1_mask1stmt 10/49, r1_dup0 12/46, r2_anon_mask 17/49, r3_two_dup0 12/46,
r4_mask1stmt_u8 17/50, r5_mask1stmt_reread 10/49, s1 12/46, s2 12/46, s3 10/49,
s4 10/49, s5 10/49, u1 13/49, u2 13/49, u3 13/49, u4 10/49, u5 13/45,
v_c 12/46, v_d 10/49, v_e 10/49, v_f 13/45, v_g 10/49, v_h 13/47, v_i 10/49,
w1 10/49, w2 10/49, w3 10/49, w4 27/52, x1 10/49, x2 17/49, x3 10/49,
y1_split_dup0 14/52, y2_split_dupall 20/54, y3_split_rev_dup0 14/52,
y4_split_nodup 12/51. Sources under
`tmp/grind/func_80034F88/s51/bodies/`; the load-bearing ones are copied into
`memory/grind/func_80034F88/rejected/` with s51- prefixes.

- [s51] Chassis re-measured at dispatch: candidate.c installed on HEAD gives sandbox score 10, 49 build insns against 49 target insns; a regenerated character-equivalent body measures the same, so the ledger floor of 10 is current.

- [s51] Pointer allocno for &D_80106A73 is invariant across every block-0 shape measured this session: refs=10, live_length=28, allocno_compare priority 10714.

- [s51] Two-statement mask gives the masked value refs=6 len=9 pri=13333 and a pointer conflict row clean of hard 3; single-statement mask and named-raw forms give refs=4 len=8 pri=10000 and a pointer conflict row containing hard 3.

- [s51] In the refs=4 shapes the block-0 temp is seated at hard 3 by local-alloc, not by global_alloc: r1's QI temp reg76 (refs=2 len=4) is '76 in 3' and w1's named raw reg75 (refs=2 len=2) is '75 in 3', in both cases while the pointer allocno is listed ahead of the masked value in ';; regs to allocate'.

- [s51] The target's 80034FB4 lbu is a cse address-equality artifact: giving the mask and flag block 0 two non-equatable SYMBOL_REFs for the same byte emits it, and the resulting block 0 matches the target's instruction sequence at 51 insns / score 12 (tmp/grind/func_80034F88/s51/dis_split.txt).

- [s51] Address spelling ((&D_80106A70 + 3) versus &D_80106A73) is codegen-neutral on this chassis in every placement, voiding the s16/s17/s25 spelling kills as chassis artifacts.

- [s51] 47 distinct bodies were compiled and scored this session; none went below 10 and none reached 49 insns with the target's block-0 registers. Full table in evidence.md under the s51 heading.

- [s51] src/code6cac_b.c was restored to HEAD (INCLUDE_ASM) at end of session; no build-file dirt.

- [s51] New reusable tool: tmp/grind/func_80034F88/s51/rep.sh prints score, .greg allocation order, conflict and preference rows, dispositions and per-pseudo refs/live_length/priority in one call per body.

## s52 (structural, 2026-09-05) -- measured facts

- HEAD chassis re-measured at session start: candidate.c installed unchanged =
  **score 10, 49 build insns**, dispositions `74 in 3  75 in 4`, reg74 (block-0
  masked value) refs=6 len=9 pri=13333, reg75 (&D_80106A73 pointer) refs=10
  len=28 pri=10714. Identical to the s49/s50/s51 ledger figures, so every s51
  conclusion is still chassis-valid.
- The base body's own objdump was captured for the first time in several
  sessions and aligned insn-for-insn against asm/funcs/func_80034F88.s. The
  base and the target agree on the number of address materialisations (THREE
  lui/addiu pairs each) and on the whole flag-block-1/2 body. Every one of the
  ten scored differences is downstream of two facts: block 0's address sits in
  $a0 with the masked value in $v1 (the target has them exchanged), and the
  target's 80034FB4 slot holds `lbu $a0,0($v1)` where the base holds a
  load-delay nop.
- PASS ATTRIBUTION (dumps, not inference): with an explicit `w = *q;` re-read
  in flag block 0 the count of `(mem:QI` in the cc1 dumps is 8 in .rtl and
  .jump and 7 in .cse, .loop, .cse2, .flow, .combine, .sched, .lreg and .greg.
  cse is the deleting pass. `(zero_extend:SI (mem:QI` appears only from
  .combine onward, i.e. the surviving QI loads are recombined into
  zero_extends after cse has already removed the redundant one.
- cse.c source read (tools/gcc-2.7.2/cse.c:7308-7376): the store's MEM
  destination is inserted into the value table unless `sets[i].src_elt == 0`,
  which happens for an in_libcall_block, for a ZERO_EXTRACT / SIGN_EXTRACT
  SET_DEST (the bit-field carve-out, cse.c:7004-7027), or for a volatile
  source. No value-side spelling of the stored byte reaches any of the three.
- A bit-field declaration of D_80106A73 produces NO `zero_extract` in .rtl or
  .cse and NO `mem:QI` at all (GCC 2.7.2 MIPS has no insv, so store_bit_field
  falls back to word arithmetic): 47 insns/score 26 and 40 insns/score 35.
- Seat-race table (refs / live_length / allocno_compare priority read from
  .lreg + .greg BEFORE the score; the masked value is pinned at 6/9/13333 in
  every join-form block 0):
      base / f2 hoisted b12 cond   pointer 10/28/10714   no flip   49/10
      b0 dup only                  pointer 11/27/12222   no flip   46/14
      q reused as loop base        pointer 12/28/12857   no flip   49/17
      b0 one-arm + b1b2 dup        pointer 11/24/13750   FLIP      48/21
      b0 + b1 dup                  pointer 12/26/13846   FLIP      47/19
      all three blocks dup         pointer 13/25/15600   FLIP      48/21
- cse DELETES `else { *q = m; }` in flag block 0 as a store of the value
  already known to be in memory: d2_b0dup.c emits `beqz / ori / sb` (one arm,
  46 insns), not the intended two-armed form. This is why block 0 cannot
  supply its own reference lift.
- A redundant `q = &D_80106A73;` inserted between the mask and flag block 0 is
  deleted outright: 49 insns, score 10, pointer refs unchanged at 10.
- Hoisting flag blocks 1 and 2's condition above their pointer assignment is
  byte-neutral (49/10, identical dispositions and identical refs/len). Hoisting
  flag block 1's condition into block 0 lengthens the masked value 9 -> 10
  (pri 12000) but evicts `p` from $a1 to $a2 (49/25).
- Artifacts: tmp/grind/func_80034F88/s52/{rep.sh,rd.py,cnt.py,gen.py,gen2.py,
  gen3.py,inst_bf.py,append.py,bodies/}. Twenty-one bodies measured; sixteen
  banked under memory/grind/func_80034F88/rejected/s52-*.

- [s52] HEAD chassis re-measured at session start: candidate.c installed unchanged = score 10, 49 build insns, dispositions '74 in 3  75 in 4', reg74 (block-0 masked value) refs=6 len=9 pri=13333, reg75 (&D_80106A73 pointer) refs=10 len=28 pri=10714 -- identical to the s49/s50/s51 ledger figures, so every s51 conclusion remains chassis-valid.

- [s52] The base body's objdump was aligned insn-for-insn against asm/funcs/func_80034F88.s: base and target agree on the number of address materialisations (THREE lui/addiu pairs each) and on the whole of flag blocks 1 and 2. All ten scored differences are downstream of exactly two facts -- block 0's address sits in $a0 with the masked value in $v1 (the target has them exchanged), and the target's 80034FB4 slot holds 'lbu $a0,0($v1)' where the base holds a load-delay nop.

- [s52] Pass attribution by dump, not inference: with an explicit re-read in flag block 0 the count of '(mem:QI' in the cc1 dumps is 8 in .rtl and .jump and 7 in .cse and every later dump. cse is the deleting pass; '(zero_extend:SI (mem:QI' first appears in .combine.

- [s52] cse.c source read: the store's MEM destination is inserted into the value table at cse.c:7308-7376 unless sets[i].src_elt == 0 (cse.c:7327) -- reachable only via an in_libcall_block, a ZERO_EXTRACT/SIGN_EXTRACT SET_DEST (cse.c:7004-7027), or a volatile source.

- [s52] A bit-field declaration of D_80106A73 yields zero 'zero_extract' and zero '(mem:QI' in the RTL (GCC 2.7.2 MIPS has no insv, so store_bit_field falls back to word arithmetic): 47 insns / score 26 and 40 insns / score 35.

- [s52] Seat-race table, priorities read from .lreg/.greg before the score, masked value pinned at 6/9/13333 throughout: base and hoisted-condition variant pointer 10/28/10714 no flip 49/10; b0-dup-only 11/27/12222 no flip 46/14; q-reused-as-loop-base 12/28/12857 no flip 49/17; b0-one-arm + b1b2-dup 11/24/13750 FLIP 48/21; b0+b1-dup 12/26/13846 FLIP 47/19; all-three-dup 13/25/15600 FLIP 48/21.

- [s52] cse deletes 'else { *q = m; }' in flag block 0 as a store of the value already known to be in memory: d2_b0dup.c emits a one-armed 'beqz / ori / sb' at 46 insns, which is why block 0 cannot supply its own reference lift.

- [s52] A redundant 'q = &D_80106A73;' inserted between the mask and flag block 0 is deleted outright: 49 insns, score 10, pointer refs unchanged at 10.

- [s52] Twenty-one bodies measured this session; sixteen banked to memory/grind/func_80034F88/rejected/s52-*. src/code6cac_b.c restored to HEAD (git status clean apart from the ledger and metrics).

## [s53] synthesis (2026-09-05) -- HEAD floor re-measured 10 (49 target / 49 build insns)

### Chassis check
`sandbox func_80034F88 --disable all` with `memory/grind/func_80034F88/candidate.c`
installed at src/code6cac_b.c:3420 => score 10, target_insns 49, build_insns 49.
Identical to the ledger's recorded floor, so every s45-s52 spelling conclusion is
still chassis-valid and was NOT re-derived.

### Kill re-audit (mandated; floor flat since s44)
The instance kill whose form sits closest to the target is s51's split-spelling
body (`rejected/s51-splitspell-mask-A70p3-EMITS-TARGET-RELOAD-51insn-score12.c`),
the only banked form that emits the target's 80034FB4 re-read.  Re-measured on
HEAD this session: **51 insns, score 12** -- byte-for-byte the ledger's number.
`tools/fake_ablate.py --func func_80034F88 --file code6cac_b --candidate <that
file>` reports "no FAKE-annotated constructs found; nothing to ablate", so the
kill was not measured behind a FAKE carrier and stands as recorded.

### The exact 10-point residual, read off the objdump (tmp/grind/func_80034F88/s53/build.txt)
Baseline build vs target, aligned:

    target                          this body
    lui   $v1,%hi(D_80106A73)       lui   $a0,%hi(D_80106A73)
    addiu $v1,$v1,%lo(D_80106A73)   addiu $a0,$a0,%lo(D_80106A73)
    lbu   $a0,0($v1)                lbu   $v1,0($a0)
    addu  $a1,$v0,$zero             move  $a1,$v0
    andi  $a0,$a0,0xF8              andi  $v1,$v1,0xF8
    sb    $a0,0($v1)                sb    $v1,0($a0)
    lw    $v0,0x20($a1)             lw    $v0,0x20($a1)
    lbu   $a0,0($v1)   <-- re-read  nop                <-- load-delay nop
    andi  $v0,$v0,1                 andi  $v0,$v0,1
    bnez  $v0,.L                    bnez  $v0,.L
     ori  $v0,$a0,1                  ori  $v0,$v1,1
    addu  $v0,$a0,$zero             move  $v0,$v1
    lui   $a0,%hi / addiu $a0,%lo   sb    $v0,0($a0)
    sb    $v0,0($v1)                lui   $a0,%hi / addiu $a0,%lo

Blocks 1 and 2 and the trailing loop are already byte-exact.  The whole residual
is (i) the $v1/$a0 swap in block 0 and (ii) the missing re-read, and (ii) costs
nothing in instruction count because the slot it belongs in is currently a
load-delay nop.  The target's block-1 `la` is scheduled ABOVE the block-0 store
only because block 0's store goes through $v1 while the la writes $a0; that
ordering difference is a consequence of (i), not an independent problem.

### FINDING 1 (new; corrects the s46-s52 pass attribution): the surviving re-reads
### in flag blocks 1 and 2 are produced by cse invalidating REG q, not by the join
Every session since s46 has explained blocks 1/2's surviving `lbu`s as "cse's
store-forwarding does not cross the control-flow join".  That explanation is
wrong.  The reads survive because each of those blocks re-executes
`q = &D_80106A73;`, and a SET of reg q makes cse invalidate every hash-table
entry that contains reg q -- which includes the `(mem:QI (reg q))` entry the
preceding `*q = <value>;` store recorded at cse.c:7310-7360.

PROOF (measured this session, not argued): take s51's split-spelling body and
delete ONLY block 1's `q = &D_80106A73;` statement, leaving the join label, the
arms and the store untouched.  Block 1's `lbu` vanishes and a load-delay nop
appears in its place (build offset 4f88), exactly the way block 0's does in the
plateau body.  49 insns, score 14.  Banked as
`rejected/s53-splitspell-drop-b1-reassign-b1-read-FORWARDED-49insn-score14.c`.
Deleting block 2's re-assignment instead gives 49 insns / score 22
(`rejected/s53-splitspell-drop-b2-reassign-49insn-score22.c`).

That 49/14 body is the first 49-INSN body in this ledger that carries a genuine
non-forwarded flag-byte re-read: it is off the plateau by register assignment
and la placement, not by instruction count.

### FINDING 2 (the residual, sharply stated): re-read and `la` are CO-LOCATED,
### and the target's block 0 is the one site where they are not
Under Finding 1 the invalidation and the address materialisation are the SAME C
statement, so on every form measured in 53 sessions a surviving re-read at site
S is accompanied by a `lui/addiu` pair at site S.  The target has FOUR
non-forwarded `lbu`s of D_80106A73 --

    80034FA0  mask read      through $v1
    80034FB4  block 0 read   through $v1   <-- no materialisation of its own
    80034FD8  block 1 read   through $a0
    80034FFC  block 2 read   through $a0

-- but only THREE `la` pairs (80034F98 -> $v1, 80034FC8 -> $a0, 80034FF0 ->
$a0).  So the target's block-0 read is invalidated at ZERO instruction cost,
sharing the mask's address register.  The three generators known to this ledger
all cost something:

  * address-value change (split spelling `&D_80106A70 + 3` vs `&D_80106A73`):
    emits an extra `la` pair -- 51 insns, score 12 (s51, re-measured s53);
  * same-value re-assignment in straight-line code: deleted by cse, invalidates
    nothing -- 49 insns, score 10 (s52-redundant-q-assign-deleted-by-cse);
  * cse's own non-recording escapes at cse.c:7327 (`sets[i].src_elt == 0`):
    in_libcall_block, ZERO_EXTRACT/SIGN_EXTRACT SET_DEST, volatile source --
    all three measured shut on this target by s52 (no insv on GCC 2.7.2 MIPS,
    so a bit-field declaration produces SImode word arithmetic and zero
    `zero_extract`; volatile fails both prongs of
    legitimate-volatile-interrupt-touched and is stripped by the sandbox).

Read directly this session: cse.c:7310-7340 gives the complete list of
conditions under which a store's destination is NOT entered into the value
table (flag_float_store on a float mode, in_libcall_block,
`sets[i].src_elt == 0`, paradoxical-SUBREG dest with an extend source), and
cse.c:7182 shows the one further path to `src_elt == 0` that s52 did not name:
`rtx_equal_p (SET_SRC (sets[i].rtl), SET_DEST (sets[i].rtl))`, i.e. a literal
self-store `*q = *q;`, which GCC removes as a no-op before it can help.
`exp_equiv_p` (cse.c:2051) has NO MEM case: it compares mode and operands only,
so MEM_IN_STRUCT_P and RTX_UNCHANGING_P cannot make two reads of the same
address pseudo inequivalent.  That closes the "spell one side as a struct/array
member" idea at the RTL level rather than by measurement.

### Byte fact worth keeping: `&D_80106A70 + 3` is a BYTE-IDENTICAL spelling
%hi(D_80106A70) == %hi(D_80106A73) == 0x8010 and %lo(D_80106A70) + 3 ==
%lo(D_80106A73) == 0x6A73, so `lui $x,%hi(D_80106A70); addiu $x,$x,%lo(...)+3`
links to the same two words as the target's `la` of D_80106A73.  The
split-spelling penalty is entirely the EXTRA la, never its contents -- so a
future form that needs two cse-distinct address values but only three
materialisations is not blocked by the spelling, only by the count.

### FINDING 3: the last unmeasured combination of s52's priority model is dead
s52's frontier proposed pairing g1 (q re-used as the trailing loop's base
pointer: pointer allocno refs 12, live length 28, priority 12857) with h1
(block 1's condition hoisted into block 0: masked value length 9 -> 10,
priority 13333 -> 12000), on the arithmetic that 12857 > 12000 flips the
block-0 seat without any duplicated store in flag blocks 1 or 2 -- which is
what wrecked every previously measured flip.  MEASURED: 49 insns, score 26, and
the seat did NOT flip (block 0 is still `lui $a0 / addiu $a0 / lbu $a1,0($a0)`).
h1 does not merely lengthen the masked value's live range: it restructures
block 0 into `beqz ... / j ...` with the arms split across an unconditional
jump, and it pushes `p` out of $a1 into $a2, so the allocno set global_alloc
sorts is not the set the priority model assumed.  Banked as
`rejected/s53-g1-plus-h1-no-seat-flip-49insn-score26.c`.

### Session end state
src/code6cac_b.c carries candidate.c for measurement only and is restored to
HEAD (`INCLUDE_ASM("asm/funcs", func_80034F88);`) before the outcome is written.
Artifacts: tmp/grind/func_80034F88/s53/.

## s53 (synthesis, 2026-09-05) -- SECOND PASS: the residual is exactly TWO instructions, and the seat and the reload are separately controllable levers

The first s53 pass (its findings are recorded above and in the candidate.c
header) was discarded by the driver validator on an outcome-wording violation,
not on its measurements; the ledger text, the candidate and the three
`rejected/s53-*` forms it banked are intact and are NOT re-derived here.  This
pass re-measured the chassis, then added four measurements that the ledger did
not have.

Chassis re-confirmed on HEAD 2026-09-05 with `memory/grind/func_80034F88/candidate.c`
installed: `sandbox func_80034F88 --disable all` = **score 10, 49 target insns /
49 build insns**.  src/code6cac_b.c is restored to
`INCLUDE_ASM("asm/funcs", func_80034F88);` before this entry was written.

### FINDING A (new, and it re-sizes the whole residual): the target's block-0 reload is INSTRUCTION-COUNT-FREE, and the split-spelling body's ONLY excess is the block-0 `la` pair
Objdump of the plateau body (tmp/grind/func_80034F88/s53/build.txt, captured
this session) against the target, insn for insn:

    target 80034FB0  lw   $v0,0x20($a1)      build 4f60  lw   v0,32(a1)
    target 80034FB4  lbu  $a0,0($v1)         build 4f64  nop        <-- HERE
    target 80034FB8  andi $v0,$v0,1          build 4f68  andi v0,v0,0x1

The plateau body emits a LOAD-DELAY NOP at exactly the site where the target
emits its block-0 reload.  Both bodies are 49 insns.  So the reload does not
cost an instruction on this chassis -- it consumes a slot the plateau body is
already wasting.  Confirmed from the other side: the s51 split-spelling body
(`tmp/grind/func_80034F88/s53/base_split.c`, re-measured this session at 51
insns / score 12) has **no nop there** -- its reload fills the same slot --

    build 4f60 lui a0 / 4f64 addiu a0   <-- the ONLY excess, 2 insns
    build 4f68 lw v0,32(a1)
    build 4f6c lbu v1,0(a0)             <-- the reload, in the delay slot

and every other instruction of that body, including the whole of flag blocks 1
and 2 (`lbu v1,0(a0)` / `ori v0,v1,K` / `move v0,v1` / `sb v0,0(a0)`), the
trailing loop and the epilogue, matches the target register-for-register.  The
residual of the closest-to-target admissible body is therefore EXACTLY ONE
ADDRESS MATERIALISATION -- 2 instructions, `lui`+`addiu` -- and nothing else.
This supersedes the vaguer "co-location law" framing of the first s53 pass with
a number: the target performs FOUR non-forwarded flag-byte reads with THREE
`la` pairs; every admissible body measured in 53 sessions needs one `la` per
non-forwarded read.

### FINDING B: an explicit block-0 read is codegen-INERT on the plateau chassis
`v = *q;` inserted between the mask store and block 0's arms, with both arms
consuming `v` instead of `m` (banked as
`rejected/s53b-explicit-b0-read-INERT-49insn-score10.c`): **49 insns, score 10,
build byte-identical to the plateau** -- no `move`, no extra live value, no seat
change.  cse folds the read onto the masked pseudo and combine erases the
`zero_extend(subreg:QI)` wrapper, exactly as s52 predicted.  So the plateau body
and the explicit-read body are the same program to the back end; asking for the
read costs nothing and buys nothing.

### FINDING C: `do { <mask> } while (0);` costs exactly one nop on this chassis and does not produce the reload
Wrapping the three mask statements in the sanctioned `do { ... } while (0);`
form, arms unchanged (`rejected/s53b-dowhile0-mask-wrap-costs-one-nop-50insn-score12.c`):
**50 insns, score 12**.  The one extra instruction is a load-delay nop at the
mask's own `lbu` (build 4f58): the wrap displaces `move a1,v0` out of that slot,
where the plateau body had scheduled it.  The block-0 read is still forwarded.
This reproduces s30's "any wrap spanning the mask store costs one instruction"
on the CURRENT (s49/s50 two-statement-mask) chassis, and it re-confirms s7's
dump finding that cse2 -- which runs with `after_loop` non-zero and therefore
ignores the `NOTE_INSN_LOOP_END` break at cse.c:8054 -- undoes the cse1 boundary
the wrap creates.

### FINDING D (the useful one): the block-0 SEAT and the surviving RELOAD are SEPARATELY controllable, and no measured body holds both
The same wrap WITH the explicit block-0 read (`rejected/s53b-dowhile0-mask-plus-b0read-SEAT-v1-no-reload-51insn-score20.c`),
51 insns / score 20, emits:

    4f48 lui  v1 / 4f4c addiu v1        <-- pointer seated in $v1
    4f54 lbu  v0,0(v1)
    4f60 sb   v0,0(v1)
    4f64 move a0,v0                     <-- the read, still FORWARDED
    4f80 sb   v0,0(v1)                  <-- block 0's store through $v1

i.e. the address pointer takes **hard 3 ($v1) -- the target's block-0 seat** --
and block 0's store is `sb $v0,0($v1)` exactly as at target 80034FD0.  The read
is nevertheless forwarded (materialised as `move $a0,$v0`, not an `lbu`).  The
seat flip is caused by the wrap AND the read jointly: Finding B shows the read
alone is inert, and Finding C shows the wrap alone leaves the pointer in $a0.

So this ledger now has two independent levers with disjoint effects:

    split spelling (s51)      -> the reload SURVIVES,  seat stays $a0, 51/12
    wrap + explicit read      -> the seat becomes $v1, read forwarded, 51/20
    plateau                   -> neither,                              49/10

and no measured body holds both at once, let alone at 49 insns.  That is the
sharpest statement of the residual this ledger has: it is no longer "one
inequality in global.c allocno_compare" (s49-s52's framing) -- the $v1 seat is
now reachable WITHOUT any duplicated store and without a priority lift, by a
sanctioned construct.  What is missing is holding it together with the reload
inside 49 instructions.

### Session end state
src/code6cac_b.c restored to HEAD.  Artifacts: tmp/grind/func_80034F88/s53/
(build.txt disassemblies, base_split.c) and tmp/grind/func_80034F88/s53b/
(the three probe bodies).

- [s53] Chassis re-confirmed this session: candidate.c installed at src/code6cac_b.c:3420 gives sandbox --disable all = score 10, 49 target insns / 49 build insns. src restored to INCLUDE_ASM before the outcome was written.

- [s53] The plateau body emits a load-delay nop (build 4f64) at exactly the site of the target's block-0 reload (80034FB4 lbu $a0,0($v1)); both bodies are 49 insns, so the reload is instruction-count-free on this chassis.

- [s53] The s51 split-spelling body (51 insns, score 12, re-measured this session) differs from the target by exactly one lui/addiu pair: its reload occupies the same delay slot, and its flag blocks 1 and 2, trailing loop and epilogue match the target register-for-register.

- [s53] The target performs FOUR non-forwarded flag-byte reads (80034FA0, 80034FB4, 80034FD8, 80034FFC) with THREE la pairs (80034F98 -> $v1, 80034FC8 -> $a0, 80034FF0 -> $a0); every admissible body measured in this ledger needs one la per non-forwarded read.

- [s53] An explicit block-0 read is codegen-inert on the plateau chassis: 49 insns, score 10, build byte-identical (rejected/s53b-explicit-b0-read-INERT-49insn-score10.c).

- [s53] do { <mask> } while (0); costs exactly one instruction (a load-delay nop at the mask's lbu, displacing `move a1,v0`) and does not defeat the forwarding, because cse2 runs with after_loop non-zero and the cse.c:8054 NOTE_INSN_LOOP_END break does not apply to it (50 insns, score 12).

- [s53] The wrap plus an explicit block-0 read seats the address pointer on hard 3 ($v1) -- the target's block-0 seat, with sb $v0,0($v1) as at 80034FD0 -- with no duplicated store and no reg_n_refs priority lift (51 insns, score 20). This is the first seat flip in this ledger that does not require the duplicated-store lift s50/s52 measured.

- [s53] cse.c read this session at 8008-8130: cse_end_of_basic_block scans to a CODE_LABEL unconditionally (8039), breaks at NOTE_INSN_LOOP_END only while !after_loop (8054), and extends across a conditional jump only when LABEL_NUSES (JUMP_LABEL) == 1 and a BARRIER precedes the target (8092-8114).

- [s53] Target insn budget for the closest admissible bodies: split spelling 51 = 49 + 2 (block-0 la); wrap 50 = 49 + 1 (nop); wrap + read 51 = 49 + 1 (nop) + 1 (move). No measured body reaches 49 while carrying either the reload or the $v1 seat.

## [s54] synthesis (2026-09-05) -- the co-location law is BROKEN: the target's block-0 reload is reproduced with THREE la pairs, at 49 instructions

### Chassis check
`memory/grind/func_80034F88/candidate.c` installed at src/code6cac_b.c:3420 =>
`sandbox func_80034F88 --disable all` = **score 10, 49 target insns / 49 build
insns**, identical to the ledger floor.  Every s45-s53 spelling conclusion is
therefore still chassis-valid and was not re-derived.

### Kill re-audit (mandated)
The two closest-to-target banked instance kills were re-measured on HEAD:
  * `rejected/s51-splitspell-mask-A70p3-EMITS-TARGET-RELOAD-51insn-score12.c`
    -> **51 insns / score 12**, byte-for-byte the ledger value.  Its full
    disassembly is `tmp/grind/func_80034F88/s54/build_split.txt`: pointer on
    $a0 for all four sites, values on $v1, FOUR la pairs, flag blocks 1/2 and
    the trailing loop exact.  All 12 differing insns are inside block 0.
  * `tools/fake_ablate.py` on that body and on this session's best body:
    "no FAKE-annotated constructs found; nothing to ablate" -- neither kill was
    measured behind a FAKE carrier.

### FINDING 1 (the session's result): cse's forwarding is defeated by invalidating the STORED VALUE'S pseudo, not the ADDRESS pseudo -- and that costs no `la`
s53 stated a "co-location law": every non-forwarded re-read of D_80106A73 in 53
sessions was accompanied by its own `lui/addiu` pair, because the only known
invalidator was re-executing `q = &D_80106A73;` (a SET of reg q, which
invalidates every table entry containing q -- including `(mem:QI (reg q))`).
The target has FOUR non-forwarded reads but only THREE la pairs, so that law
said the target's block-0 reload had no ordinary-C generator.

The law is wrong, and the missing generator is a SECOND cse invalidation route.
`*q = m;` records the store's memory destination with the value class of the
SOURCE register (cse.c:7308-7376).  Re-setting that source register -- i.e.
re-using the C variable `m` for the next value block 0 needs -- calls
`invalidate (reg m)`, which removes reg m from that class.  The `(mem:QI (reg
q))` entry survives, but its class no longer contains a register, so cse cannot
replace the following read of `*q` with anything cheaper than the MEM itself:
**the load stays**.  reg q is untouched, so NO new address materialisation is
emitted.

Measured, both mask spellings, this session:

    m = *q & 0xF8;  *q = m;
    m = p[8];          /* re-use of m: invalidates the stored value's class */
    v = *q;            /* REAL lbu, through the mask's own la */
    c = p[8] & 1;      /* cse folds this read onto m */
    if (c) c = v | 1; else c = v;
    *q = c;

gives (`rejected/s54-mreuse-invalidation-TARGET-BLOCK0-STRUCTURE-49insn-score13.c`,
scratch `bodies/t3_dead_single.c`):

    build (49 insns)                  target
    la    $4,D_80106A73               lui/addiu $v1,D_80106A73
    lbu   $3,0($4)                    lbu  $a0,0($v1)
    move  $5,$2                       addu $a1,$v0,$zero
    andi  $3,$3,0x00f8                andi $a0,$a0,0xF8
    sb    $3,0($4)                    sb   $a0,0($v1)
    lw    $3,32($5)                   lw   $v0,0x20($a1)
    lbu   $2,0($4)   <-- THE RELOAD   lbu  $a0,0($v1)   <-- 80034FB4
    andi  $3,$3,0x0001                andi $v0,$v0,1
    bne   $3,$0,.L737                 bnez $v0,.L80034FC8
    ori   $3,$2,0x0001                 ori $v0,$a0,1
    move  $3,$2                       addu $v0,$a0,$zero
    sb    $3,0($4)                    sb   $v0,0($v1)

**49 instructions, score 13, three la pairs, four non-forwarded reads, no nop.**
This is the first body in 54 sessions that carries the target's block-0 reload
at the target's instruction count and in the target's instruction ORDER; flag
blocks 1 and 2, the trailing loop and the epilogue are byte-exact.  Its whole
13-point residual is the block-0 REGISTER SEAT (build q=$a0 / values $v1,$v0;
target q=$v1 / values $a0,$v0) plus the resulting placement of block 1's `la`
(the target emits it BEFORE block 0's store; a single address pseudo cannot).
The two-statement mask spelling of the same trick is one nop worse (50 insns,
score 13, `rejected/s54-mreuse-twostmt-mask-reload-plus-nop-50insn-score13.c`):
there the reload lands in the same hard register as the `lw` of p[8] and cannot
fill the load-delay slot.

### FINDING 2: the invalidating re-set must carry a REAL value; a constant dead store does nothing
`m = 0;` / `m = 1;` in the same slot (both mask spellings, both orderings,
`rejected/s54-constant-dead-reset-NO-INVALIDATION-49insn-score10.c`) build
BYTE-IDENTICALLY to the plateau: 49 insns, score 10, no reload.  So the
sanctioned dead-store family buys nothing here; the working construct is
ordinary variable re-use whose value block 0 actually consumes.

### FINDING 3: the residual is now a single numeric inequality in global.c, with both sides measured
`tools/ra_solver/extract.py` (ALLOCDBG) on the 49/13 body
(`tmp/grind/func_80034F88/s54/model_t3.json`), allocation order
`73 74 79 83 87 75 78 82 86 72`:

    pseudo  role                      nrefs  livelen   pri     hard
    73      loop counter i              11      7     47142     3   (no conflict with 75)
    74      m (mask value + p[8])        4      4     20000     3   <-- takes $v1 first
    79/83/87 block store values          5      7     14285     3/2
    75      q (the address)             11     29     11379     4   <-- wants $v1
    78/82/86 block re-read values        3      4      7500     2/3
    72      p                            6     34      3529     5

allocno_compare is `floor_log2(nrefs)*nrefs/livelen*10000` (global.c).  Pseudo
74 is the ONLY allocno that conflicts with 75 and outranks it, so the flip needs
either pri(75) > 20000 (nrefs >= 15 at len 29) or pri(74) < 11379 (nrefs 4 at
len >= 8, or nrefs 3 at any len).  Everything measured this session moves one
side and breaks something else:

    else-arm consumes m (u1/u4/u6/u7)   m -> nrefs 5 len 7 pri 14285, but p is
                                        pushed out of $a1 into $a2       49/17
    condition tested inline `if (m & 1)` (u3)                            49/27
    duplicated store, block 2 only (e3)                                  50/16
    duplicated store, blocks 1+2 (e1)                                    51/19
    duplicated store + else-arm m (e2/e4)                            51/23, 50/20
    q re-used as the trailing loop base (s52 g1, old chassis)         no flip

No measured lift reaches the threshold, and each duplicated store costs an
instruction (the arms differ, so cross-jump does not re-merge them).

### FINDING 4: the BANNED two-handle axis is still not the thing that costs the residual
Diagnosis only (never a candidate): mask+block0 on `t`, flag blocks 1/2 on `q`,
with this session's reload mechanism
(`rejected/s54-BANNED-twohandle-on-reload-chassis-50insn-score24.c`): **50
insns, score 24** -- worse than the single-handle 49/13 on the same chassis.
This re-confirms s50 finding 3 on the NEW chassis: the standing multi-handle
ban is not what is costing the 10 points.

### Session end state
src/code6cac_b.c restored to `INCLUDE_ASM("asm/funcs", func_80034F88);`.
candidate.c is UNCHANGED (the plateau body is still the lowest-scoring form at
10; the new 49/13 body is banked in rejected/ as the structural chassis).
Artifacts: tmp/grind/func_80034F88/s54/ (bodies/, model_v2.json, model_t3.json,
model_u1.json, build_split.txt, build_v2_reuse_m_raw.txt).

- [s54] Chassis re-confirmed: candidate.c = score 10, 49/49 on HEAD 2026-09-05.
- [s54] Kill re-audit: s51 split-spelling body re-measured 51/12 unchanged; fake_ablate reports no FAKE constructs in it or in this session's best body.
- [s54] cse's store-forwarding of D_80106A73 is defeated by re-setting the C variable that held the STORED VALUE (`m = p[8];` after `*q = m;`), which drops the register from the stored MEM's equivalence class while leaving reg q untouched -- so the re-read survives with NO extra la. This is the generator the s53 "co-location law" said did not exist.
- [s54] The m-re-use body with a single-statement mask is 49 insns / score 13 and reproduces the target's block-0 instruction sequence and ORDER exactly (3 la pairs, 4 non-forwarded lbu, no load-delay nop); its entire residual is the block-0 register seat.
- [s54] A constant re-set (`m = 0;`) in the same slot does NOT invalidate: 49 insns, score 10, byte-identical to the plateau -- the invalidating re-set must carry a value the block actually consumes.
- [s54] global.c numbers on the 49/13 chassis: q = nrefs 11, len 29, pri 11379, hard 4; m = nrefs 4, len 4, pri 20000, hard 3; m is the ONLY conflicting allocno that outranks q. Flip needs pri(q) > 20000 (nrefs >= 15) or pri(m) < 11379 (nrefs 4 at len >= 8, or nrefs 3).
- [s54] Two pointer objects on the reload chassis (BANNED, diagnosis only) = 50 insns / score 24, worse than the single-handle 49/13.

- [s54] Chassis re-confirmed: memory/grind/func_80034F88/candidate.c installed at src/code6cac_b.c:3420 gives sandbox --disable all = score 10, 49 target insns / 49 build insns on HEAD 2026-09-05.

- [s54] cse records a store's MEM destination in the value class of the SOURCE register (cse.c:7308-7376); re-setting that source pseudo strips the register out of the class, leaving the MEM entry with no register member, so the next read of the same address stays a load while reg q is untouched and no extra la is emitted.

- [s54] The m-re-use body with a single-statement mask is 49 insns / score 13 and reproduces the target's block-0 instruction sequence and order exactly: three la pairs, four non-forwarded lbu, the reload in the lw's shadow with no load-delay nop, and flag blocks 1/2, the trailing loop and the epilogue byte-exact.

- [s54] The two-statement mask spelling of the same trick is 50 insns / score 13: the reload lands in the same hard register as the lw of p[8] and therefore cannot fill the load-delay slot.

- [s54] A constant re-set (m = 0; / m = 1;) in that slot invalidates nothing -- six bodies build byte-identically to the plateau (49/10) -- so the sanctioned dead-store family is inert on this residual.

- [s54] global.c allocno_compare is floor_log2(nrefs)*nrefs/livelen*10000; on the 49/13 chassis q = nrefs 11, len 29, pri 11379 (hard 4) and m = nrefs 4, len 4, pri 20000 (hard 3), with m the only conflicting allocno that outranks q (tmp/grind/func_80034F88/s54/model_t3.json).

- [s54] The loop counter i (pri 47142, hard 3) does NOT conflict with the address allocno, so it is not a blocker; only the m allocno stands between q and hard 3.

- [s54] Nine priority lifts measured on that chassis all miss the flip: else-arm-consumes-m 49/17 (m to pri 14285 but p pushed into $a2), if (m & 1) inline 49/27, duplicated stores 50/16, 51/19, 51/23, 50/20 (each costs an instruction because the arms differ and cross_jump does not re-merge them).

- [s54] Two pointer objects on the reload chassis (BANNED, diagnosis only) measure 50 insns / score 24, worse than the single-handle 49/13 -- the multi-handle ban is not what costs the residual.

- [s54] Kill re-audit: the s51 split-spelling body re-measures 51/12 unchanged and fake_ablate finds no FAKE construct in it or in this session's best body.

## s55 (synthesis, 2026-09-05) -- floor 10 (unchanged); the residual is re-typed from a priority race to an object-model contradiction

**Chassis re-audit.** The plateau body (`memory/grind/func_80034F88/candidate.c`)
measures 49 build insns / 49 target insns / score 10 on HEAD 2026-09-05, and
s54's closest-to-target body
(`rejected/s54-mreuse-invalidation-TARGET-BLOCK0-STRUCTURE-49insn-score13.c`)
measures 49/49 / score 13. Both are byte-for-byte their recorded values, so no
banked kill in this ledger is void on chassis grounds.

**The 49/13 chassis is now ordinary C.** s54 reached the target's block-0
instruction sequence only through a dead store, `m = p[8];`, whose value nothing
consumed. Splitting the condition across two ordinary statements supplies the
identical cse invalidation with every value consumed:

    q = &D_80106A73;
    m = *q & 0xF8;
    *q = m;
    m = p[8];        /* re-set of the stored value's variable: invalidates */
    v = *q;          /* survives as a real lbu, no la of its own */
    m &= 1;          /* ordinary split-init / compound assignment */
    if (m) { m = v | 1; } else { m = v; }
    *q = m;

49 build insns, score 13, build byte-identical to s54's dead-store body. Four
further spellings (separate result variable; two-statement mask; function-scoped
reload variable; blocks 1/2 sharing one pair of value variables) all measure
49/13, so 13 is a hard floor for the whole family. Banked as
`rejected/s55-ordinaryC-mreuse-invalidation-NO-DEAD-STORE-49insn-score13.c`.
This matters for admissibility rather than for the floor: the closest-to-target
chassis now carries no dead store, no FAKE construct and no pun beyond the
pre-existing trailing-loop one.

**Statement order inside block 0 is load-delay-slot critical.** The invalidating
SET must precede the read, but the condition's `andi` must follow it. Fusing
them (`m = p[8] & 1; v = *q;`) is 50 insns / score 13 in all three spellings
measured, because the andi takes the lw's load-delay slot and the reload can no
longer fill it. Splitting the condition is exactly what buys the 49th
instruction.

**Where the 13 points sit.** Aligned objdump of the 49/13 body against
`asm/funcs/func_80034F88.s` (`tmp/grind/func_80034F88/s55/build_c1.txt`): flag
blocks 1 and 2, the trailing loop and the epilogue are byte-exact. Twelve of the
thirteen points are the register naming of build 4f48..4f78 --

    build:   q = $a0    mask / cond / result = $v1    reload = $v0
    target:  q = $v1    mask = $a0, reload = $a0      cond / result = $v0

-- and the thirteenth is that the target materialises flag block 1's address at
80034FC8, BEFORE flag block 0's store at 80034FD0, where every body in this
ledger emits the store first.

**THE STRUCTURAL FINDING.** The target holds `&D_80106A73` in two hard registers
at once. Its three `la` pairs land on $v1 (80034F98), $a0 (80034FC8) and $a0
(80034FF0), and at 80034FC8..80034FD0 the $a0 value and the $v1 value are both
live. A C local whose address is never taken gets exactly one pseudo
(`tools/gcc-2.7.2/stmt.c:3387`) and a pseudo receives exactly one hard register
(`tools/gcc-2.7.2/global.c:1275`); GCC 2.7.2 has no live-range splitting.
Therefore no body with a single C pointer object aliasing that address can emit
the target's bytes, at any allocation priority. The same fact settles s54's
frontier item 3 without a scheduler dump: with one pseudo the block-1 `la` is a
SET of the register block 0's store still reads, and `sched.c:1720` makes it
anti-dependent on that store, so it can never be scheduled above it.

**Consequence for the standing ban.** s50 finding 3 and s54 concluded from
SCORES (two-object bodies at 21..26, then 50/24) that "the standing multi-handle
ban is not what is costing the residual". That inference is now contradicted by
structure: the ban forbids the only object model the target's bytes admit. The
two-object bodies measured badly because their priorities were wrong on the
chassis they were measured on, not because the axis is wrong -- and none of them
was built on the s55 ordinary-C invalidation chassis, which is the first chassis
in this ledger whose flag blocks 1/2, loop and epilogue are all byte-exact with
`p` in $a1. s55 therefore files a **ruling-request** instead of another spelling
pass. If the ruling refuses the second handle, the honest disposition is a
LADDER EXHAUSTED (non-endgame residual, floor 10) foreclosure record, because
the byte-match is then unreachable under the constraint set.

**Also killed this session (all instance kills, all on the s55 chassis, HEAD
2026-09-05, one declared pointer object, no FAKE construct):** sharing the
condition/result and reload variables across two or three flag blocks (five
bodies, every one 49 insns / score 33; ALLOCDBG on the shared body shows the
value allocno at nrefs 17 / livelen 20 / pri 34000, far ABOVE the pointer's
10645, and `p` evicted from $a1 into $a2); one-armed block 0 (`if (m) v |= 1;`
with no else) at 47 insns / score 33, cse deleting the missing arm's re-store;
and the mask variable used as block 0's condition only, with the result in a
separate variable, at 49 insns / score 27 in both mask spellings.

## s55 SECOND PASS (synthesis, 2026-09-05) -- the two-alias grant, spent

Chassis at dispatch: HEAD `INCLUDE_ASM`, ledger floor 10, candidate.c the
single-object 49-insn/score-10 plateau. This pass is the first to spend the
2026-09-05 11:01 Judge PASS (docs/grind/decisions.md:23437), which narrowed the
standing multi-handle closure to permit EXACTLY TWO annotated
`u8 *X = &D_80106A73;` alias objects (one for the mask + flag block 0, one for
flag blocks 1 and 2) on the s55 ordinary-C invalidation chassis.

### E55b.1 -- two block-scoped handles reproduce the target's whole instruction stream
Body: `p = func_80077D00();` first, then an inner block declaring
`u8 *t = &D_80106A73;` carrying the mask + flag block 0, then a second inner
block declaring `u8 *q = &D_80106A73;` carrying flag blocks 1 and 2 (with the
existing `q = &D_80106A73;` re-assignment before block 2). Measured
`sandbox --disable all` = **49 insns / score 17**, and the aligned objdump
(tmp/grind/func_80034F88/s55/cmp.py output) shows EVERY opcode in the target's
order, including the two things 55 sessions of single-object bodies could never
produce simultaneously: the block-0 reload at 80034FB4 AND flag block 1's
`lui/addiu` at 80034FC8 emitted BEFORE flag block 0's store at 80034FD0. The
only differences are register names.
Banked: rejected/s55b-twoalias-blockscoped-EXACT-INSN-ORDER-49insn-score17.c.

### E55b.2 -- the declaration SITE is load-bearing (function scope costs 4 insns)
The same two handles declared with their initializers at FUNCTION scope are
live across `func_80077D00()`, so both are allocated to callee-saved $s0/$s1
and the prologue grows two `sw`/`lw` pairs: 53 insns, score 35
(tmp/grind/func_80034F88/s55/t1.c). The grant is only usable with the
initializers inside blocks that begin after the call.

### E55b.3 -- the two-statement mask clears hard 3 from the pointer's conflict row here too
On the two-handle chassis, `m = *t; m &= 0xF8;` (two statements) removes the
block-0-confined QI temp that the single-statement mask leaves behind (reg 77
in the t2 model, local-alloc-seated at $v1). Pointer conflict row goes from
`72 74 75 76 80 2 3 29` to `72 74 75 76 79 2 29` -- no hard 3. Score unchanged
at 49/17, i.e. s51's finding (ii) reproduces on the two-handle chassis, and the
hard-3 conflict was never the binding constraint.
Banked: rejected/s55b-twoalias-twostmt-mask-no-hard3-conflict-49insn-score17.c.

### E55b.4 -- the target's VALUE PAIRING is reachable, and it needs a dead store to a local
The target pairs {masked value, block-0 reload} in one register ($a0) and
{p[8], condition, result} in another ($v0). The s54/s55 cse-invalidation
chassis pairs them the other way round, because the invalidator must re-set the
STORED value's variable and that variable then carries the condition. Adding a
consumed-then-overwritten re-set -- `c = p[8]; m = c; m = *t;` -- moves the
reload into the masked value's variable and gives the target's pairing:
49 insns, score 15, with pseudo 74 (masked+reload, nrefs 7 len 8) and pseudo 76
(cond+result, nrefs 7 len 9, hard 2 = $v0, the target's register).
`m = c;` is a dead store to a LOCAL (sanctioned dead-store family, FAKE
required); it is coalesced away and costs no instruction.
Banked: rejected/s55b-twoalias-TARGET-VALUE-PAIRING-49insn-score15.c.

### E55b.5 -- u8-typing the block-0 value puts the masked value on the target's $a0
Declaring the block-0 value local as `u8 m` (instead of the function-scope
`s32 m`) splits it into two allocnos: the raw/reload pseudo 75 (nrefs 6 len 6,
hard 3) and the masked pseudo 77 (nrefs 3 len 8, **hard 4 = $a0**, the target's
register for the masked value). Score **14** at 49 insns -- the session best
and the closest body in the ledger. Its residual cost is the u8 truncation in
the else arm: build `andi $v0,$v1,0xff` where the target has
`addu $v0,$a0,$zero`.
Banked: rejected/s55b-twoalias-u8-masked-value-in-a0-49insn-score14.c.

### E55b.6 -- the residual is a three-cycle rotation, and it is one allocation-ORDER fact
Aligned objdump of the score-14/15 bodies:
    build : t=$a1   masked=$v1(or $a0)  p=$a2
    target: t=$v1   masked=$a0          p=$a1
Measured with tools/ra_solver/extract.py (models
tmp/grind/func_80034F88/s55/model_t6.json, model_t7.json):
  * Every block-0 VALUE allocno carries a hard-2 conflict, because sched1 sinks
    the `p = $v0` call-result copy BELOW the mask store (verified in the .lreg
    RTL, tmp/grind/func_80034F88/dumps/code6cac_b.lreg: insn 11
    `(set (reg 72) (reg:SI 2 v0))` appears after insn 23, the mask store). $v0
    being unavailable, the first block-0 value takes $v1 -- the seat the target
    gives the pointer.
  * The pointer allocno is nrefs 5 / livelen 28 / pri 3571 and is allocated
    second-to-last of eight, behind every block-0 value (pri 12000..27500).
    global.c's priority is scale-invariant (floor_log2(n)*n/len), so compacting
    block 0 cannot reorder them. Beating pri 17500 needs nrefs(t) >= 16 at
    len 28; pushing a masked+reload value below pri 3571 needs nrefs 3 at
    len >= 9, which a two-value chain cannot have.
  * The pointer's live range STRICTLY CONTAINS every block-0 value's range, so
    no third allocno can be made to conflict with a block-0 value without also
    conflicting with the pointer. That forecloses the "block $v1 for the value"
    route from the value side.

### E55b.7 -- the inverse solver's minimal atom, spelled and measured
`python3 tools/ra_solver/inverse.py global model_t6.json --goal
'{"75": 3, "74": 4, "72": 5}' --depth 2` reports a **1-atom** solution
(6 distinct vectors), the cheapest being `[conflict_add] pseudo 73 <-> 74`:
make the loop index (pri 47142, hard 3) conflict with the block-0 value, so the
value is pushed off $v1. Spelled as `i = 0;` before block 0 with
`for (; i < 3; i++)`: the index becomes live across the whole body, costs three
instructions and lands at 52 insns / score 31.
Banked: rejected/s55b-loop-index-init-hoist-conflict-atom-52insn-score31.c.
The remaining atoms are refs_up(pointer) 5 -> 16/17 (duplicated-statement
family; every duplicate measured in s50/s54 cost an instruction because the
arms differ) and refs_down(block-0 value) 7 -> 2 (requires splitting masked
from reload, which loses the pairing of E55b.4).

### E55b.8 -- what did NOT move the copy
Reading `p[8]` into a separate variable BEFORE the mask (`c = p[8];` first,
then `m = c;` as the invalidator) does not pull the `p = $v0` copy above the
mask: the copy is still sunk, hard 2 stays in every block-0 value's conflict
row, and the body measures 49/17.
Banked: rejected/s55b-twoalias-early-p8-read-copy-does-not-rise-49insn-score17.c.
Splitting the block-0 RESULT into its own variable (`r`) on the two-handle
chassis deletes two instructions (47 insns, score 23) -- the reload no longer
survives.
Banked: rejected/s55b-twoalias-split-result-var-47insn-score23.c.

- [s55] The 2026-09-05 11:01 Judge PASS (docs/grind/decisions.md:23437) permitting exactly two annotated = &D_80106A73 alias objects is PRODUCTIVE, not merely permissive: the two-handle body is the first in 55 sessions to emit all 49 target instructions in the target's order.

- [s55] Register residual, aligned objdump: build t=$a1 / masked=$v1 or $a0 / p=$a2 against target t=$v1 / masked=$a0 / p=$a1 -- a three-cycle rotation, nothing else.

- [s55] Cause, measured in tmp/grind/func_80034F88/dumps/code6cac_b.lreg: sched1 sinks insn 11 (set (reg 72) (reg:SI 2 v0)) -- the call-result copy -- BELOW the mask store, so $v0 is live across the mask and every block-0 value allocno carries a hard-2 conflict and takes $v1. In the target that copy sits at 80034FA4, in the mask lbu's load-delay slot.

- [s55] The pointer allocno is nrefs 5 / livelen 28 / pri 3571 and is allocated second-to-last of eight, behind every block-0 value (pri 12000..27500). global.c's priority floor_log2(n)*n/len is scale-invariant, so compacting block 0 cannot reorder them: the flip needs nrefs(pointer) >= 16 at len 28, or a masked+reload value at nrefs 3 with len >= 9 (a two-value chain cannot have 3 refs).

- [s55] The pointer's live range STRICTLY CONTAINS every block-0 value's range, so no third allocno can be made to conflict with a block-0 value without also conflicting with the pointer -- the 'block $v1 from the value side' route is closed on this chassis.

- [s55] s51's finding (ii) reproduces on the two-handle chassis: the two-statement mask removes the block-0-confined QI temp and clears hard 3 from the pointer's conflict row (row goes from 72 74 75 76 80 2 3 29 to 72 74 75 76 79 2 29), with no score change -- so the hard-3 conflict was never the binding constraint.

- [s55] Best score this session is 14 (49 insns) on the two-alias u8 body; the honest floor is unchanged at 10 (the single-object plateau, still candidate.c).


## s56 (solver, 2026-09-05) -- the residual is one inequality in global.c

CHASSIS RE-MEASURED ON HEAD.  `candidate.c` installed at src/code6cac_b.c:3420,
`sandbox func_80034F88 --disable all` = score 10, 49 target insns / 49 build
insns.  The dispatch brief's "measurement unavailable" is resolved: the floor is
10 and the ledger figure was correct.

SOLVER CHASSIS SELECTION (a reusable fact, not a one-off).
`goal_from_tgt.py classify code6cac_b func_80034F88` on each banked body:
  - candidate.c (score 10)                  -> FIRST DIVERGENCE: PRE-RA
        ours-only `nop` x1, target-only `lbu #,0(#)` x1.  No RA model of this
        body can be aimed at the target: it does not contain the target's
        block-0 reload at all.
  - rejected/s55b-twoalias-u8-...-score14.c -> FIRST DIVERGENCE: PRE-RA
        ours-only `andi #,#,0xff` x1, target-only `move #,#` x1 (the u8
        else-arm zero-extend).
  - rejected/s55b-twoalias-TARGET-VALUE-PAIRING-49insn-score15.c -> **RA**
        `$a1 -> $v1 x7`, `$v1 -> $a0 x7`, `$a2 -> $a1 x6`, no shape difference.
So the score-15 body -- NOT the score-10 candidate and NOT the score-14 u8 body
-- is the only banked chassis on which the RA solver is meaningful.  Its score is
worse; its structure is the target's.  (`inverse_compose.py classify` refuses
this function outright -- zero-rule guard -- and prints the `goal_from_tgt.py`
route to use instead.)

THE MODEL (tools/ra_solver/extract.py on the score-15 body; model saved as
tmp/grind/func_80034F88/s56/model_s15.json).  Eight allocnos, with GCC's own
`.lreg` numbers cross-checked in tmp/grind/func_80034F88/s56/lreg.txt:
    ord0 73 loop index         refs 11 len  7 pri 47142 -> $v1   target $v1  OK
    ord1 81 loop address temp  refs 10 len 12 pri 25000 -> $v0   target $v0  OK
    ord2 74 mask + reload      refs  7 len  8 pri 17500 -> $v1   target $a0  XX
    ord3 76 block-0 condition  refs  7 len  9 pri 15555 -> $v0   target $v0  OK
    ord4 80 block-1/2 value    refs  6 len 10 pri 12000 -> $v1   target $v1  OK
    ord5 79 handle B (q)       refs  6 len 19 pri  6315 -> $a0   target $a0  OK
    ord6 75 handle A (t)       refs  5 len 28 pri  3571 -> $a1   target $v1  XX
    ord7 72 p                  refs  6 len 34 pri  3529 -> $a2   target $a1  XX
Five of the eight allocnos are ALREADY on their target seats.  The priority
formula fits all eight rows exactly:
    pri = floor_log2(nrefs) * nrefs * size * 10000 / live_length
(73 and 81 are the loop allocnos and carry the same formula; earlier ledger
entries that fitted a bare nrefs*20000/len were fitting the floor_log2==2 rows
only.  The floor_log2 step is load-bearing: nrefs 15 -> factor 3, nrefs 16 ->
factor 4.)

THE WHOLE RESIDUAL, STATED EXACTLY.  find_reg hands 74 the register $v1 because
$v1 is still free when 74 is reached: 74's hard conflicts are {$v0,$ra} (so $v0
is out) and its allocno conflicts are {72,75,76} -- it does NOT conflict with 73,
which already holds $v1.  Make $v1 unavailable to 74 and the rest is automatic:
74 falls to $a0, 75 (which DOES conflict with 74) takes the vacated $v1, and 72
takes $a1.  Two and only two mechanisms make $v1 unavailable:
    (a) a conflict between 74 and 73, or
    (b) pri(75) > pri(74) = 17500, so 75 is allocated first and takes $v1.
inverse.py global model_s15.json with the FULL eight-pseudo disposition as the
goal (per the solver modality's rule 3) and --depth 2 returns minimal solution
size 1 atom / six vectors, and every one of them is a spelling of (a) or (b):
    #1/#2 conflict_add 73<->74                     cost 2
    #3    refs_down 74: 7 -> 2                     cost 6
    #4    refs_down 74: 7 -> 1                     cost 7
    #5/#6 refs_up   75: 5 -> 16 / 17               cost 12/13
No live-length vector is reported because the search bound is +/- 8 and (b) via
length needs len(75) <= 5.  The report also FORECLOSES all 24 preference atoms
mechanically: $v1/$a0/$a1 never appear as hard regs in this function's pre-RA
RTL, so global.c's set_preference can never record a preference for them from
any C.  Report: tmp/grind/func_80034F88/s56/inverse_full.txt.

VECTOR-BY-VECTOR DISPOSITION (this is the session's product).
  - refs_down(74) to 2 or 1 -- BYTE-FORECLOSED BY COUNTING.  Allocno 74's seven
    references map one-for-one onto operands the TARGET's own stream contains:
    lbu $a0,0($v1) (1), andi $a0,$a0,0xF8 (2), sb $a0,0($v1) (1), the reload
    lbu $a0,0($v1) (1), ori $v0,$a0,0x1 (1), addu $v0,$a0,$zero (1).  Removing a
    reference removes one of the target's operands.
  - refs_up(75) to 16/17 -- MEASURED, NOT ARGUED.  The only byte-neutral
    reference-adder in the sanctioned families is duplicated-statement-into-arms
    relying on a jump2 cross_jump re-merge.  It does not re-merge here.
    Duplicating block 0's `*t = c;` into both arms of the score-15 body gives
    **50 insns / score 12**: ours emits `sb; beqz; j` where the target emits
    `bnez`.  It buys exactly +1 reference (5 -> 6) and -4 live length
    (28 -> 24), pri 3571 -> 5000 against the 17500 needed.  Reaching 16 refs
    therefore costs ten more instructions.  Banked as
    rejected/s56-twoalias-dup-store-into-arms-NOT-MERGED-50insn-score12.c;
    model tmp/grind/func_80034F88/s56/model_v1.json.
  - conflict_add(73,74) -- costs 3 instructions.  It needs the trailing copy
    loop's index live across block 0; s55 spelled it (`i = 0;` hoisted, then
    `for (; i < 3; i++)`) and measured 52 insns / score 31.  The .lreg dump
    shows why nothing cheaper exists on this shape: 74's last reference is its
    arm read, and sched1 places block 0's store (insn 55) AFTER block 1's `la`
    (insn 61), so 74 is already dead before any later value is born.
  - pri(75) > 17500 by shortening 75's live range -- needs len <= 5 at nrefs 5.
    75 is live from its `la` (insn 15, immediately after the call) to block 0's
    store (insn 55); the measured length is 28 and the shortest any s55/s56
    shape produced is 24.

THE ONE PLACE THE WALL MOVED.  On the 50-insn duplicated-arm shape the
disposition is {72:$a1, 73:$v1, 74:$v1, 75:$a0, 76:$v0, 79:$a0, 80:$v1,
81:$v0}: **p (72) reaches its target seat $a1 for the first time**, handle B
keeps $a0, and the conflict graph changes -- 75 no longer conflicts with 79,
because block 0's store no longer sinks below block 1's `la`.  The three-cycle
rotation collapses to the single 74<->75 swap, and on that model inverse.py
reports a strictly cheaper bar: refs_down(74) 7 -> 3 suffices (pri 3750 < 75's
5000) instead of 7 -> 2.  Report: tmp/grind/func_80034F88/s56/inverse_v1.txt.
The shape itself is one instruction over, so it is not the answer -- but "get
the 50-insn shape's conflict graph at 49 instructions" is a strictly smaller
question than the one this ledger has been asking for 56 sessions.

- [s56] Chassis re-measured on HEAD with candidate.c installed: sandbox func_80034F88 --disable all = score 10, 49 target insns / 49 build insns. The dispatch brief's 'measurement unavailable' is resolved and the ledger floor of 10 is correct.

- [s56] inverse_compose.py classify refuses this function by design (zero-rule guard: with no regfix/asmfix rules the src-derived tgt.s cannot carry the target stream, and a text-stream classify would report a fictitious PRE-RA verdict). The supported route is goal_from_tgt.py classify / goal, on the OBJECTS.

- [s56] goal_from_tgt.py goal on the score-15 body: 15 renamed pairs, 0 skipped; $a1 -> $v1 x7, $v1 -> $a0 x7, $a2 -> $a1 x6.

- [s56] Measured allocation model of the score-15 body (model_s15.json, agreeing with the .lreg dump): ord0 73 loop index refs 11 len 7 pri 47142 -> $v1 (target seat); ord1 81 loop address temp refs 10 len 12 pri 25000 -> $v0 (target seat); ord2 74 mask+reload refs 7 len 8 pri 17500 -> $v1 (target wants $a0); ord3 76 block-0 condition refs 7 len 9 pri 15555 -> $v0 (target seat); ord4 80 block-1/2 value refs 6 len 10 pri 12000 -> $v1 (target seat); ord5 79 handle B refs 6 len 19 pri 6315 -> $a0 (target seat); ord6 75 handle A refs 5 len 28 pri 3571 -> $a1 (target wants $v1); ord7 72 p refs 6 len 34 pri 3529 -> $a2 (target wants $a1).

- [s56] GCC 2.7.2 global.c allocno priority on this function is exactly floor_log2(nrefs) * nrefs * size * 10000 / live_length -- it fits all eight allocnos with no residual. Earlier ledger fits of the form nrefs*20000/len were fitting only the floor_log2==2 rows and mispredict the loop allocnos and any nrefs>=16 lever.

- [s56] The .lreg RTL slice (tmp/grind/func_80034F88/s56/lreg.txt) shows handle A (reg 75) set at insn 15 immediately after the call and last used by block 0's store at insn 55, which sched1 has already placed AFTER block 1's `la` at insn 61 -- so on the two-alias chassis the target's block-1-la-before-block-0-store order is reproduced, and allocno 74 is dead before any later value is born.

- [s56] inverse.py reports all 24 preference atoms mechanically FORECLOSED for this function: $v1, $a0 and $a1 never appear as hard registers in its pre-RA RTL, so global.c set_preference can never record a preference for them from any C form. Copy-preference levers are therefore off the table permanently for this function's residual, independent of chassis.

- [s56] Duplicated-statement-into-arms measured on this function: one duplication of block 0's store = +1 reg_n_refs, -4 reg_live_length, +1 instruction, no jump2 cross_jump re-merge (50 insns / score 12).

- [s56] src/code6cac_b.c was restored to HEAD (INCLUDE_ASM) at the end of the session; the only tree changes are the ledger files and one new rejected form.

## s57 (forensics, 2026-09-05) -- pass attribution for block 0's reload, and a new chassis

FACT 1. The reload at 80034FB4 is deleted by **cse2**, GCC 2.7.2's second cse
run (-frerun-cse-after-loop), and NOT by cse1 or combine. Traced insn by insn
through the sliced pass dumps in tmp/grind/func_80034F88/s57/:
f88.vB.rtl.txt / f88.vB.jump.txt / f88.vB.cse.txt / f88.vB.loop.txt all carry
`(insn 31 (set (reg:QI 79) (mem:QI (reg/v:SI 75))))`; f88.vB.cse2.txt has it as
`(set (reg:QI 79) (subreg:QI (reg/v:SI 74) 0))`; f88.vB.combine.txt has
`(set (reg/v:SI 78) (reg/v:SI 74))` with insn 31 a NOTE_INSN_DELETED. The
s52/s54 ledger attribution ("cse forwards the store") named the wrong run.

FACT 2. cse records a store's MEM in the value class of the store's SOURCE
pseudo. cse.c:7310-7327 skips recording the destination entirely when
`sets[i].src_elt == 0`, and cse.c:7182 only inserts the source when
`! rtx_equal_p (SET_SRC, SET_DEST)`. So the MEM's recorded value is reachable
from two sides: the ADDRESS register (invalidated by any set of it) and the
VALUE register (invalidated by any set that changes its quantity).

FACT 3. The address side is not free. A second `q = &D_80106A73;` between the
mask store and the read does invalidate -- the reload is alive in f88.vB.cse.txt --
but cse1 deletes that set as redundant (it is gone by f88.vB.cse2.txt), so cse2
re-forwards and the reload dies. Any address set cse1 CANNOT delete materialises
a second la pair; that is s51's 51-instruction split spelling. Measured:
49 insns / score 10, byte-identical to the s56 candidate.

FACT 4. The value side IS free, and it does not have to touch the mask.
Splitting block 0 into `raw = *q; mv = raw & 0xF8; *q = mv; mv = raw; v = *q;`
keys the MEM on `mv`; the dead `mv = raw;` changes that quantity, survives cse1
and loop into the cse2 stream, and is then deleted as trivially dead by
flow/combine. Result: 49 instructions with `lbu` at the target's 80034FB4 slot
(previously a load-delay nop). `mv = 0;` works identically; `mv = raw & 0xF8;`
(same value) and omitting the re-set both lose the reload.

FACT 5. The new chassis is strictly closer than anything banked.
`goal_from_tgt.py classify` on it: FIRST DIVERGENCE **RA**, `$v1 -> $a0 x6`,
`$a0 -> $v1 x5`, ZERO instruction-shape differences. Before s57 the only
RA-class body was the two-alias score-15 form (20 substitutions, three-cycle
rotation, two /* FAKE */ pointer-alias objects spending the Judge's two-handle
grant). The s57 body uses ONE pointer object and one dead store to a local.

FACT 6. ra_solver model tmp/grind/func_80034F88/s57/model_vD.json:
  73 block-0 value chain  nrefs 11 len  7 pri 47142 -> hard 3 ($v1)
  74 &D_80106A73 pointer  nrefs 11 len 29 pri 11379 -> hard 4 ($a0)
  72 p                    nrefs  6 len 34 pri  3529 -> hard 5 ($a1)  [target seat]
  78/83/87 (flag-block values) pri 14285 -> $v0; 77/82/86 pri 7500 -> $v1
The residual is the same seat race the ledger has carried since s49, but every
s49-s56 numeric conclusion was measured on a chassis that either lacked the
reload or carried two FAKE alias objects, so they are chassis-void here.

FACT 7 (tooling). tools/fake_ablate.py reported an ABLATION WIN on the score-15
two-alias body (drop handle A: score 11 vs keep-all 15). It is an artifact: the
ablation removes the `u8 *t = &D_80106A73;` declaration, the build drops flag
block 0 entirely and emits 38 instructions against 49 target instructions, and
the distance metric prices eleven deletions below eleven register substitutions.
Read build_insns before believing an ablation win on this function.

- [s57] HEAD chassis re-measured this session: memory/grind/func_80034F88/candidate.c (s56 body) = score 10, 49 target insns / 49 build insns, confirming the ledger floor of 10.

- [s57] Pass attribution, from sliced dumps: the block-0 reload is a live (mem:QI (reg 75)) load in .rtl, .jump, .cse and .loop, is rewritten to (subreg:QI (reg 74) 0) in .cse2, and is deleted by combine. cse2 is -frerun-cse-after-loop's second cse run.

- [s57] cse.c:7310-7327 skips recording a store's destination when sets[i].src_elt == 0, and cse.c:7182 inserts the source only when ! rtx_equal_p (SET_SRC, SET_DEST) -- so the stored MEM's recorded value is reachable from the address register and from the value register, and invalidating either kills the forwarding.

- [s57] On the two-alias score-15 body the same slices show the mechanism from the other side: cse2 keeps `(insn 33 (set (reg:QI 78) (mem:QI (reg 75))))` alive precisely because `(insn 30 (set (reg 74) (reg 76)))` -- the s55 `m = c;` re-set -- changes reg 74's quantity between the store and the load.

- [s57] New chassis measurement: single `u8 *q`, block 0 split into raw/mv/v with a dead `mv = raw;`, 49 insns / score 10, `lbu v1,0(a0)` present at the target's 80034FB4 slot; goal_from_tgt.py classify => FIRST DIVERGENCE: RA, `$v1 -> $a0 x6`, `$a0 -> $v1 x5`, no instruction-shape difference.

- [s57] ra_solver model tmp/grind/func_80034F88/s57/model_vD.json: 73 (block-0 value chain) nrefs 11 len 7 pri 47142 -> hard 3 ($v1); 74 (&D_80106A73 pointer) nrefs 11 len 29 pri 11379 -> hard 4 ($a0); 72 (p) nrefs 6 len 34 pri 3529 -> hard 5 ($a1, the target seat); 78/83/87 pri 14285 -> $v0; 77/82/86 pri 7500 -> $v1.

- [s57] Consequence for the ledger: every s49-s56 numeric RA conclusion was measured on a chassis that either lacked the target's block-0 reload (PRE-RA) or carried two FAKE pointer-alias objects. They are chassis-relative and must be re-measured on the s57 body before they are spent.

- [s57] src/code6cac_b.c was restored to HEAD (INCLUDE_ASM) before this session ended; only memory/grind/ and tmp/ carry this session's work.

## s58 evidence (forensics, 2026-09-05) — the residual is now a two-gate register race with measured numbers

**E58.1 — There exists a 49-instruction body that matches the target's whole
instruction stream in shape AND order.** `rejected/s58-twoalias-ORDER-EXACT-3cycle-rotation-49insn-score15.c`
(= tmp/grind/func_80034F88/s58/vA.c). It is the s55 two-annotated-alias scaffold
with block 0 spelled as the s57 raw/mv/v split plus the `mv = raw;` value-side
cse2 invalidator. Build vs target, instruction by instruction:

    build 0x4f48 lui/addiu $a1,D_80106A73   target 80034F98 lui/addiu $v1
    build 0x4f50 lbu   $v1,0($a1)           target 80034FA0 lbu   $a0,0($v1)
    build 0x4f54 move  $a2,$v0              target 80034FA4 addu  $a1,$v0,$zero
    build 0x4f58 andi  $v1,$v1,0xf8         target 80034FA8 andi  $a0,$a0,0xF8
    build 0x4f5c sb    $v1,0($a1)           target 80034FAC sb    $a0,0($v1)
    build 0x4f60 lw    $v0,32($a2)          target 80034FB0 lw    $v0,0x20($a1)
    build 0x4f64 lbu   $v1,0($a1)           target 80034FB4 lbu   $a0,0($v1)   <- the reload
    build 0x4f68 andi/bnez/ori/move         target 80034FB8..FC4              identical
    build 0x4f78 lui/addiu $a0,D_80106A73   target 80034FC8 lui/addiu $a0     <- la BEFORE the store
    build 0x4f80 sb    $v0,0($a1)           target 80034FD0 sb    $v0,0($v1)
    ... blocks 1, 2 and the trailing loop are register-for-register identical ...

Score 15. The ONLY divergence is a 3-cycle register rotation:
$a1 -> $v1 (handle A, x7), $v1 -> $a0 (block-0 values, x7), $a2 -> $a1 (p, x6).
`goal_from_tgt.py classify` = FIRST DIVERGENCE: RA, zero shape differences.
The two levers are orthogonal: two pointer pseudos buy the ORDER (a single
pseudo makes block 1's `la` a set of the register block 0's store reads, so the
dependence forbids hoisting it), the value-side dead re-set buys the RELOAD.

**E58.2 — The target's register goal, as pseudos.** `goal_from_tgt.py goal
--model` on vA: `{"74": 3, "72": 5}` = handle A -> $v1, p -> $a1; the block-0
value chain must follow to $a0, where it shares a hard register with handle B
(the two do not conflict: `82 conflicts: 72 74 77 78 82 2 29`).

**E58.3 — GATE 1 is local-alloc, not global.c** (this corrects the ledger's
long-running "global.c priority race" framing for the order-exact chassis).
On vA the block-0 raw and mask values are block-local quantities (2 refs / 2
insns each, `block_alloc` priority 10000), local-alloc's ascending `find_free_reg`
scan gives them $v1 ($0/$1 fixed, $2 live with the call return, $3 free), and
global.c's `dump_conflicts` — printed BEFORE the assignment loop — already shows
`74 conflicts: ... 2 3 29` on the handle-A allocno. Dumps:
tmp/grind/func_80034F88/s58/f88.vA.lreg.txt (`;; Register 75 in 3.`,
`;; Register 76 in 3.`) and f88.vA.greg.txt. Reproduced at renumbered pseudos on
vB (values hoisted to function scope), bytes unchanged.

**E58.4 — GATE 2 is global.c's priority order, and it survives the removal of
gate 1.** Two ways of making every block-0 value pseudo span >1 basic block
(vC: variable reuse into blocks 1/2; vD: one `v` with `v = 0;` as the
invalidator, no raw/mask locals at all) both delete hard reg 3 from the pointer
allocno's conflict set and both still emit the same bytes: the value chain
(7 refs / 8 insns, priority 17500) is allocated far ahead of the pointer
(5 refs / 28 insns, priority 3571) and takes $v1 by ordinary conflict.
vC is strictly worse structurally — it makes the block-1/2 value pseudos
overlap handle A, a conflict the target's disposition forbids.

**E58.5 — The two gates cannot be separated by any blocker.** Handle A is born
at the `la` feeding 80034FA0 and dies at the store 80034FD0; the block-0 value
chain is born at the reload 80034FB4 and dies at 80034FC4, strictly inside.
A hard-reg-3 conflict can only come from a range that overlaps, and every range
overlapping the value chain overlaps the pointer. So the target's assignment
cannot be produced by blocking $v1 — only by allocation ORDER.

**E58.6 — The order route, priced.** GCC 2.7.2 priority (verified by hand
against the greg allocation order on all three variants: vA `73 84 78 83 77 82
74 72`, vC `73 77 74 75 76 82 78 72`, vD `73 82 75 76 81 80 74 72`) is
`floor_log2(refs) * refs * size * 10000 / live_length`. For handle A
(live_length 28) to outrank the block-0 value chain (17500) it needs
`floor_log2(r)*r > 49`, i.e. **r >= 16 references against the 5 the target's
instruction stream contains**; or live_length <= 5 against a pointer spanning
all of block 0. On the vA (local-alloc) gate the equivalent bar is a
block-0-local quantity with `floor_log2(r)*r > 4` (r >= 4) over the mask's
4-insn window, seated at $v1 ahead of raw/mask.

**E58.7 — Preferences are mechanically foreclosed.** `inverse.py` emits none
and says why: "$v1 never appears as a hard reg in this function's pre-RA RTL,
so global.c set_preference can never record a preference for it." The
function's only call, `func_80077D00()`, takes no arguments, so there is no
hard-register copy anywhere to seed a preference. `inverse.py global --goal
'{"74":3,"72":5}' --depth 2` on model_vA.json = NEGATIVE within bounds
refs +12/-6, live length +/-2,4,8 (tmp/grind/func_80034F88/s58/inverse_vA.txt).

**E58.8 — Tooling note.** `fake_ablate.py` on the current candidate.c is flat
(keep-all 10 / drop-1 10, both 49 insns): the s57 dead re-set buys the
instruction SHAPE (reload instead of a load-delay nop) and the divergence CLASS
(PRE-RA -> RA), not distance. Do not read the s57 CONFIRMED entry as a distance
claim.

- [s58] A 49-instruction body exists that matches the target's ENTIRE instruction stream in shape and ORDER: rejected/s58-twoalias-ORDER-EXACT-3cycle-rotation-49insn-score15.c (= tmp/grind/func_80034F88/s58/vA.c). Its only divergence is a 3-cycle register rotation ($a1->$v1 x7, $v1->$a0 x7, $a2->$a1 x6). It scores 15 only because the engine metric prices register substitutions, not structure -- the score-10 candidate.c is structurally FARTHER (it emits the store before block 1's la and, without its FAKE, no reload at all).

- [s58] The two levers that produce that stream are orthogonal and both are needed: two pointer pseudos buy the ORDER (with one pseudo, block 1's la is a set of the register block 0's store reads, so the dependence forbids hoisting it), and the s57 value-side dead re-set buys the RELOAD (cse2 invalidation).

- [s58] The target register goal as pseudos (goal_from_tgt.py goal --model on model_vA.json): {"74": 3, "72": 5} -- handle A to $v1, p to $a1 -- with the block-0 value chain following to $a0, where it legally shares a hard register with handle B (they do not conflict: `82 conflicts: 72 74 77 78 82 2 29`).

- [s58] GATE 1 is local-alloc: block 0's raw and mask are block-local quantities (2 refs / 2 insns, block_alloc priority 10000) and local-alloc's ascending find_free_reg gives them $v1, which appears as hard reg 3 in the handle-A allocno's conflict set in the greg dump BEFORE global.c's assignment loop runs (f88.vA.lreg.txt `;; Register 75 in 3.`, `;; Register 76 in 3.`; f88.vA.greg.txt `74 conflicts: 72 74 77 78 82 2 3 29`).

- [s58] GATE 2 is global.c's priority order and it survives the removal of gate 1: globalizing the block-0 value pseudos (vC by variable reuse, vD by a single value variable with a `v = 0;` invalidator) deletes hard reg 3 from the pointer's conflict set, and the bytes do not change -- the value chain (priority 17500) still takes $v1 ahead of the pointer (3571).

- [s58] The gates cannot be separated: handle A's live range strictly contains the block-0 value chain's, so any hard-reg-3 blocker that reaches the value chain also reaches the pointer. Only allocation ORDER remains.

- [s58] The order route is priced with a verified formula. GCC 2.7.2 allocation priority = floor_log2(refs)*refs*size*10000/live_length; recomputed by hand it reproduces the greg allocation order exactly on all three s58 variants (vA `73 84 78 83 77 82 74 72`, vC `73 77 74 75 76 82 78 72`, vD `73 82 75 76 81 80 74 72`). Handle A (live_length 28) needs floor_log2(r)*r > 49, i.e. >= 16 references against the 5 the target's stream contains; on the vA local-alloc gate the equivalent bar is a block-0-local quantity with floor_log2(r)*r > 4 (r >= 4) over the mask's 4-insn window.

- [s58] Preferences are mechanically foreclosed as a lever: inverse.py emits no preference atoms and states why -- $v1 never appears as a hard reg in this function's pre-RA RTL, so global.c set_preference can never record one. The function's only call takes no arguments, so there is no hard-register copy anywhere to seed a preference.

- [s58] Tooling: fake_ablate.py on candidate.c is flat (keep-all 10 / drop-1 10, both 49 insns). The s57 dead re-set buys the instruction shape and the divergence class (PRE-RA -> RA), not distance; the s57 CONFIRMED entry must not be read as a distance claim.

## s59 (rederive, 2026-09-05) -- THE ARRAY OBJECT MODEL

Chassis check at dispatch: driver measurement unavailable; re-measured HEAD +
the s58/s57 candidate.c this session = **score 10, 49/49**. Ledger floor 10
confirmed, nothing had drifted.

**E59.1 -- The declaration change, and that it is free.** `include/code6cac.h:472`
was changed from `extern u8 D_80106A70;` to `extern u8 D_80106A70[4];`, with the
two scalar use-sites in the other TU rewritten to element form
(`src/code6cac.c:340,345` -> `D_80106A70[0]`). `src/code6cac_c_mid.c:205`
(`(Quad *)&D_80106A70`) needed no change. Every existing consumer still matches
byte-for-byte with the aggregate declaration in place:
`sandbox func_8001945C --disable all` = **score 0 (11/11)**,
`sandbox func_80019488` = **score 0 (14/14)**,
`sandbox func_80037F40` = **score 0 (51/51)**.
The relocation is byte-identical too: the assembler emits
`R_MIPS_HI16/LO16 D_80106A70` with addend 3, which links to the same
`lui 0x8010 / addiu ...,0x6a73` words the target has at 80034F98/F9C. So the
object model the DATA MODEL signal has been naming since the census
(D_80106A70 is a four-byte array; D_80106A73 is its element [3]) is adoptable at
zero cost to the rest of the project. This retires the "verify the relocation
still resolves" precondition the s58 frontier attached to this axis.

**E59.2 -- vA: the plain array body (no pointer object at all) = 50 insns,
score 25.** `D_80106A70[3]` read/modified/written directly in all three flag
blocks, no locals carried across. cse forwards the store into EVERY following
read: the disassembly (tmp/grind/func_80034F88/s59/vA.dis.txt) has exactly ONE
`lbu` of the flag byte in the whole function, THREE `nop`s in the
`lw $v0,0x20($a1)` load-delay slots that the target fills with reloads, and an
extra `j` from an inverted branch arm in block 1. Banked as
rejected/s59a-array-model-plain-cse-forwards-all-3-blocks-50insn-score25.c.

**E59.3 -- vB: the array body + the s57 value-class invalidator in ALL THREE
blocks = 50 insns, score 24, and BLOCK 0's REGISTER ASSIGNMENT IS THE TARGET'S,
EXACTLY, WITH NO POINTER OBJECT AND NO ALIAS.** Body: three flag blocks written
straight on `D_80106A70[3]`, with `c = raw;` (a dead re-set of the stored-value
local) after the first two stores. Result
(tmp/grind/func_80034F88/s59/vB.dis.txt):

    4f48 lui  v1,%hi(D_80106A70)     | 80034F98 lui  v1,%hi(D_80106A73)
    4f4c addiu v1,v1,3               | 80034F9C addiu v1,v1,%lo
    4f50 lbu  a0,0(v1)               | 80034FA0 lbu  a0,0(v1)
    4f54 move a1,v0                  | 80034FA4 addu a1,v0,zero
    4f58 andi a0,a0,0xf8             | 80034FA8 andi a0,a0,0xF8
    4f5c sb   a0,0(v1)               | 80034FAC sb   a0,0(v1)
    4f60 lw   v0,0x20(a1)            | 80034FB0 lw   v0,0x20(a1)
    4f64 lbu  a0,0(v1)               | 80034FB4 lbu  a0,0(v1)

Eight instructions, register-for-register identical to the target, produced by a
body with ZERO pointer objects. All three reloads are present. The residual is a
SINGLE instruction: vB materialises the flag address at each BLOCK'S STORE (so
one `la` serves store-N and read-(N+1)), while the target materialises it at each
BLOCK'S READ (so one `la` serves read-N and store-N). vB's phase leaves the final
store with no live address and it pays `lui $at; sb $v1,3($at)` (2 insns) where
the target pays `sb $v0,0($a0)` (1). Banked as
rejected/s59b-array-model-no-pointer-object-3-reloads-block0-regs-EXACT-50insn-score24.c.
THIS IS THE CLOSEST ANY FAKE-ALIAS-FREE FORM HAS COME IN 59 SESSIONS and the
first evidence that block 0's target seat ($v1 address / $a0 value) is reachable
without a coercion construct.

**E59.4 -- vC: the proven single-`q` chassis ported onto the array declaration =
49 insns, score 10, block-0 reload PRESENT, zero declaration puns.** `q` is now
`&D_80106A70[3]` (a pointer INTO the declared aggregate, not a second name for a
scalar symbol) and the trailing loop is `D_80106A70[i] = ...` instead of the
`*(&D_80106A70 + i)` pun that the auto-scan flags. Same floor as the s57/s58
candidate, strictly cleaner form. This is the new candidate.c. Disassembly in
tmp/grind/func_80034F88/s59/vC.dis.txt.

**E59.5 -- vD: THE FAKE IS OPTIONAL ON THIS CHASSIS.** vC with the `mv = raw;`
dead re-set deleted ALSO measures **49 insns / score 10**, with ZERO FAKE
constructs of any kind. What the dead store buys on the array chassis is only the
block-0 RELOAD: vD's 4f64 is a `nop` where vC (and the target, at 80034FB4) has
`lbu`. So the honest floor of 10 is now established as reachable with NO coercion
whatsoever -- the coercion buys instruction SHAPE, not distance. Banked as
rejected/s59d-array-model-NO-FAKE-49insn-score10-block0-reload-absent.c.

**E59.6 -- vE: mixing the two spellings is strictly worse.** Array spelling for
block 0, pointer `q` for blocks 1 and 2 = **51 insns, score 17**. The block-0
store re-materialises its own address in the merge block AND `q` materialises a
second one, so the merge block pays two `la` pairs. Banked as
rejected/s59e-array-block0-pointer-blocks12-51insn-score17.c.

**E59.7 -- THE RESIDUAL, RESTATED ON THE NEW CHASSIS.** On vC the RTL shape is
the target's: three address materialisations, the first covering the mask pair
AND flag-block 0, the second covering flag-block 1, the third covering
flag-block 2 -- exactly the target's `la` at 80034F98 / 80034FC8 / 80034FF0.
Every arithmetic instruction matches. Two differences remain and they are one
difference:
  (a) block 0's seat is inverted -- target address $v1 / value $a0, vC address
      $a0 / value $v1. Blocks 1 and 2 already agree with the target.
  (b) consequently block 1's `la` cannot hoist. The target emits
      `lui $a0; addiu $a0` at 80034FC8/FCC ABOVE block 0's store
      `sb $v0,0($v1)` at 80034FD0 because the la writes $a0 and the store reads
      $v1 -- independent, so sched1 moves it up. On vC both are $a0, so the la is
      anti-dependent on the store and stays below it.
So the whole 59-session residual is now ONE inequality: block 0's address pseudo
must be seated in $v1 rather than $a0. The la hoist and the 3-cycle rotation s58
chased with two FAKE aliases (score 15) both fall out of that one assignment.

**E59.8 -- Siblings.** func_80034708 (same file, same D_80106A73 xref) still has
no candidate.c after its s1, so there is nothing to transplant; CD_sync and
CD_datasync were spent at s33/s30 and neither shares a code window with this
function (different file, different shape). No sibling debt outstanding.

**E59.9 -- Tree state at end of session.** src/code6cac_b.c, src/code6cac.c and
include/code6cac.h were all reverted to HEAD before the outcome was written; the
array declaration and its two use-site edits are recorded in candidate.c's header
as the integration handoff they are.

- [s59] Chassis re-check: HEAD + the inherited s57/s58 candidate.c measures score 10, 49 target insns / 49 build insns on HEAD 2026-09-05. The ledger floor of 10 is current; nothing had drifted.

- [s59] include/code6cac.h:472 `extern u8 D_80106A70;` -> `extern u8 D_80106A70[4];` plus src/code6cac.c:340,345 rewritten to `D_80106A70[0]` leaves all three existing consumers byte-identical: func_8001945C 0 (11/11), func_80019488 0 (14/14), func_80037F40 0 (51/51). src/code6cac_c_mid.c:205 `(Quad *)&D_80106A70` needs no change.

- [s59] The array relocation is byte-identical to the target's: the assembler emits R_MIPS_HI16/LO16 D_80106A70 with addend 3 and the linker folds it to the same lui 0x8010 / addiu 0x6a73 words the target has at 80034F98/80034F9C.

- [s59] New candidate.c (vC) = 49 insns, score 10, block-0 reload present, ZERO declaration puns (the `*(&D_80106A70 + i)` pun the auto-scan flagged is now `D_80106A70[i]`), ONE pointer object which is now a pointer INTO a declared aggregate rather than a second name for a scalar symbol.

- [s59] The honest floor of 10 is reachable with ZERO FAKE constructs of any kind (vD, 49/49 score 10). The dead store in candidate.c buys the target's 80034FB4 reload -- instruction shape, not distance -- and can be dropped without moving the floor.

- [s59] vB (no pointer object at all) reproduces target instructions 80034F98-80034FB4 register-for-register, all three reloads present, and is exactly ONE instruction over: its address materialisations are phased at each block's STORE rather than at each block's READ, so the final store has no live address and pays lui $at + sb ...,3($at) instead of sb ...,0($a0).

- [s59] The 59-session residual, restated on the new chassis: block 0's address pseudo is seated in $a0 with its value in $v1, the inverse of the target ($v1 address, $a0 value); blocks 1 and 2 already agree with the target. Because block 1's la writes the same $a0 the block-0 store reads, the la is anti-dependent on the store and sched1 cannot hoist it above the store as it does in the target at 80034FC8/FCC vs 80034FD0. One seat assignment produces both differences.

- [s59] Sibling duty discharged: func_80034708 (same file, same D_80106A73 xref) still has no candidate.c after its s1, so nothing is transplantable; CD_sync and CD_datasync were spent at s33/s30 and share no code window with this function.

- [s59] Tree reverted to HEAD at end of session: src/code6cac_b.c, src/code6cac.c and include/code6cac.h are all clean; the header edit is recorded as an integration handoff in candidate.c's header.

## s60 -- rederive: the value model, and why the single-pointer chassis is
## structurally short of the target

**E60.1 (dump-proven, new).** The 60-session "$v1 seat race" on the address
object is not a priority race at all on the s59 chassis -- it is a HARD-REGISTER
CONFLICT created by LOCAL-ALLOC before global.c ever runs. With the s59
candidate installed (array declaration, single `q`, four block-0 locals),
tmp/grind/func_80034F88/dumps/code6cac_b.lreg reports

    Register 75 used 2 times across 2 insns in block 0;  ;; Register 75 in 3.
    Register 76 used 2 times across 2 insns in block 0;  ;; Register 76 in 3.

(reg 75 = `raw`, reg 76 = `mv`, identified from the .greg RTL: insn 20 sets
reg 75 to the zero_extend of the flag byte, insn 22 sets reg 76 to
`reg 75 & 248`), and code6cac_b.greg then reports

    ;; 74 conflicts: 72 74 77 78 82 83 86 87 2 3 29

i.e. the address allocno 74 conflicts with HARD REG 3 ($v1). $v1 was therefore
never available to `q`, whatever its priority. Every earlier session's priority
arithmetic on this seat was measuring the wrong gate.

**E60.2 (new form, floor-neutral, strictly cleaner).** Reusing ONE ordinary
local for the mask value and for every flag block's value makes each block-0
value pseudo live across the if, so block 0 contains no block-local quantity and
local-alloc has nothing to seat in $v1. Measured on the same chassis:

    ;; 74 conflicts: 72 74 75 76 2 29          (no hard reg 3)

`sandbox func_80034F88 --disable all` = score 10, 49/49 -- the floor is
unchanged, but the body carries no FAKE-annotated construct, no dead store, no
declaration pun and no alias handle. This is the new candidate.c.

**E60.3 (the residual, fully priced).** With the conflict gone the seat is
decided purely by global.c's allocno_compare ordering, priority =
floor_log2(n_refs) * n_refs / live_length. Measured for the new body:

    73  i   11/7  -> 4.71   $v1        76  c   15/18 -> 2.50   $v0
    75  v   14/20 -> 2.10   $v1        74  q   10/31 -> 0.97   $a0
    72  p    6/34 -> 0.35   $a1
    emitted order: ";; 5 regs to allocate: 73 76 75 74 72"

The target needs 74 above 75. Splitting the value variable (rejected/s60l, s60m:
`v` for the mask + block 0, `w` for blocks 1-2) gives 75 = 8/10 -> 2.40 and
76 = w 6/10 -> 1.20; both still outrank q at 0.97 and the order is unchanged.
Raising q instead needs floor_log2(r)*r > 65 at live_length 31, i.e. 17 refs
against the 10 the body has.

**E60.4 (structural, and it reframes the whole function).** The target carries
the flag address in TWO different hard registers that are live in overlapping
regions of the same function: $v1 for block 0 (la at 80034F98, last read by the
store at 80034FD0) and $a0 for blocks 1 and 2 (la at 80034FC8, which sched1
hoists ABOVE that same store precisely because the two registers are
independent, and la at 80034FF0). A single C pointer object is a single allocno;
global.c:1275 writes exactly one hard register per allocno
(`reg_renumber[allocno_reg[allocno]] = best_reg;`) and GCC 2.7.2 performs no
live-range splitting. So no single-pointer body can produce the target's address
geometry, however the priorities are arranged. The two routes that CAN are (a)
the pointer-free array spelling, where cse rematerialises a fresh address pseudo
per extended basic block -- rejected/s59b-... is 50 insns with block 0
register-for-register EXACT -- and (b) the Judge-granted two-object form, which
has never been measured on the aggregate declaration.

**E60.5 (kill re-audit, mandated).** The instance kill re-measured is s59's
"pointer-object-free array body reaches block-0 register-exactness but not 49
instructions" (rejected/s59b-...), the banked form that sits closest to the
target. Re-installed on the CURRENT chassis this session: 50 build insns, score
24 -- unchanged, the kill stands. FAKE ablation of the closest FAKE-carrying
form (`tools/fake_ablate.py --candidate memory/grind/func_80034F88/candidate.c`,
the s59 body) reports keep-all 10/49 and drop-1 10/49: the single dead re-set is
inert for distance, confirming H59.3 on a second measurement.

**E60.6 (negative, banked).** On the pointer-free array chassis the remaining
one-instruction excess is block 2's store folding to `lui $at; sb %lo+3($at)`
because no address pseudo survives into it. Three routes to give it one were
measured and all hold 50 or worse: routing block 2 through `q` turns the la into
a `move` (rejected/s60c, 50/23); routing only block 2's store through `q` is
byte-identical to doing nothing (s60c2, 50/24); duplicating block 2's store into
the arms is NOT re-merged by jump2 and costs a `j` (s60d, 50/28). Duplicating
block 0's store into the arms is likewise not re-merged and additionally pushes
`p` from $a1 to $a2 (s60g, 50/34).

**E60.7 (negative, banked).** Block-scoping `q` so it covers only block 0 gives
the target's block-0 GROUPING (the address survives the branch and carries the
post-if store) at 49 instructions, but blocks 1 and 2 then lose their address
pseudos entirely -- their reads fold to `lui; lbu 3(reg)` and their stores to
`lui $at; sb` -- and `p` is displaced to $a2 (s60a 49/33, s60b 49/23). Moving
the trailing loop above the flag blocks is far worse (s60f, 51/42).

- [s60] Chassis re-measured at session start: the s59 candidate body plus the include/code6cac.h array edit gives score 10, 49 target insns / 49 build insns on HEAD 2026-09-05. The dispatch brief's 'measurement unavailable' is resolved: floor is 10.

- [s60] The hard-register gate, dump-proven: on the s59 chassis .lreg shows 'Register 75 used 2 times across 2 insns in block 0' and 'Register 76 used 2 times across 2 insns in block 0' with ';; Register 75 in 3.' / ';; Register 76 in 3.', and .greg shows ';; 74 conflicts: 72 74 77 78 82 83 86 87 2 3 29'. Reg 75 is the zero_extend of the flag byte (insn 20), reg 76 is reg75 & 248 (insn 22) -- i.e. the mask statement's raw and masked values.

- [s60] The gate is removable by ordinary C: reusing one value local for the mask and all three flag values yields ';; 74 conflicts: 72 74 75 76 2 29' -- no hard reg 3 -- at score 10, 49/49, with zero FAKE-annotated constructs, zero dead stores, zero alias handles and zero declaration puns. New candidate.c.

- [s60] The residual is now one priority inequality, fully priced from the dumps. Measured allocnos of the new candidate (priority = floor_log2(n_refs)*n_refs/live_length): 73 i 11/7 -> 4.71 seated $v1; 76 c 15/18 -> 2.50 seated $v0; 75 v 14/20 -> 2.10 seated $v1; 74 q 10/31 -> 0.97 seated $a0; 72 p 6/34 -> 0.35 seated $a1. Emitted order ';; 5 regs to allocate: 73 76 75 74 72' matches exactly.

- [s60] The target's flag address occupies TWO hard registers with overlapping live ranges: $v1 from 80034F98 to the store at 80034FD0, and $a0 from 80034FC8 (read at 80034FD8, stored through at 80034FEC) plus a third la at 80034FF0. sched1's hoist of the 80034FC8 la above the 80034FD0 store is a CONSEQUENCE of that register independence, not a separate phenomenon.

- [s60] The pointer-object-free array spelling is the only measured chassis that produces multiple address pseudos: the s59b form is register-for-register identical to the target for its first eight instructions and is exactly one instruction long, the excess being block 2's store folding to lui $at + sb %lo+3($at).

- [s60] Fifteen new forms banked to memory/grind/func_80034F88/rejected/ (s60a..s60n), covering block-scoped pointers, per-block pointer routing, duplicated stores into arms on three different block positions, statement reordering, value-variable splits, and the loop-first reordering.

## E61 -- s61 (structural, 2026-09-05): the floor moves, 10 -> 9

E61.1  CHASSIS CHECK.  memory/grind/func_80034F88/candidate.c (the s60
pointer-object-free-value-model body) re-measured on HEAD with the s59
aggregate declaration applied: `sandbox func_80034F88 --disable all` = score
10, 49 target insns / 49 build insns.  The ledger's floor was current.

E61.2  THE 60-SESSION FLOOR OF 10 IS BROKEN.  Spending the Judge's two-object
grant ON TOP OF the s59 aggregate declaration and the s60 value model measures
**score 9, 49/49** (tmp/grind/func_80034F88/s61/v/A.c; the C-variant spelling
of the same construct is the new candidate.c).  s55b, the last two-object
measurement, saw 15/17 -- but it was taken on the scalar-symbol chassis where
the address allocnos additionally carried a local-alloc hard-reg-3 conflict,
exactly as the s60 frontier note predicted.  On the aggregate chassis that
conflict is gone and the same construct is worth one instruction more.

E61.3  THE RESIDUAL IS NINE INSTRUCTIONS, ALL IN BLOCK 0, AND IS EXACTLY TWO
DEFECTS.  Blocks 1 and 2 and the trailing loop are byte-exact in the target's
order, INCLUDING the target's la-before-block-0's-store hoist at .L80034FC8
(`lui $a0 / addiu $a0 / sb $v0,0($v1)`).  Objdump comparison (ours vs target):

    lui   $a2            lui   $v1
    addiu $a2            addiu $v1
    lbu   $v1, 0($a2)    lbu   $a0, 0($v1)
    andi  $v1, $v1,0xF8  andi  $a0, $a0,0xF8
    sb    $v1, 0($a2)    sb    $a0, 0($v1)
    nop                  lbu   $a0, 0($v1)      <- the block-0 reload
    ori   $v0, $v1, 1    ori   $v0, $a0, 1
    addu  $v0, $v1, $0   addu  $v0, $a0, $0
    sb    $v0, 0($a2)    sb    $v0, 0($v1)

Defect 1: the block-0 address object `q` and the block-0 value `u` hold each
other's target hard registers ($a2/$v1 instead of $v1/$a0).  Defect 2: the
block-0 reload is still folded by cse2; the load-delay nop stands in its place,
which is why the instruction count is 49 with or without it.

E61.4  DEFECT 1 IS PURELY AN ALLOCATION ORDER, AND THE CONFLICT GRAPH ALREADY
ADMITS THE TARGET.  tools/ra_solver/extract.py on the candidate body
(tmp/grind/func_80034F88/s61/C.model.json):

    ord pseudo  hardreg nrefs livelen  pri
     0   73 i     $v1     11     7     47142
     1   77 u     $v1      8    10     24000
     2   75 c     $v0     15    19     23684
     3   74 v     $v1      6    10     12000
     4   81 r     $a0      6    19      6315   <- target seat, correct
     5   72 p     $a1      6    34      3529   <- target seat, correct
     6   76 q     $a2      4    28      2857   <- allocated LAST

    ;; 76 conflicts: 72 75 76 77 81 2 29      (73 and 74 absent)
    ;; 77 conflicts: 72 75 76 77 2 29         (81 absent)

76 does not conflict with 73 or 74, and 77 does not conflict with 81.  So
{q, v, i} may all share $v1 and {u, r} may both share $a0 -- which IS the
target's assignment ($v1 = q + blocks-1/2 value + loop index; $a0 = block-0
value + blocks-1/2 address).  Only the descending-priority order stops it: 77
reaches find_reg first and takes $v1.  If 76 were reached before 77, 76 would
take $v1 (73's seat, no conflict) and 77 would be pushed to $a0 (81's seat, no
conflict).

E61.5  THE INVERSE SOLVER PRICES DEFECT 1 AT ONE ATOM.
`inverse.py global ... --goal '{"76": 3}' --depth 2` -> minimal solution size
1 atom, 2 distinct vectors:
  (a) [calls_crossed] pseudo 77: 0 -> 1  -- semantically unreachable, block 0's
      value is computed after the function's only call, and forcing it across
      would move it to a callee-saved register (extra save/restore insns).
  (b) [refs_down] pseudo 77: refs 8 -> 2.
Ten preference atoms are reported FORECLOSED ("$v1 never appears as a hard reg
in this function's pre-RA RTL, so global.c set_preference can never record a
preference for it").  Arithmetic for (b): pri = floor_log2(n)*n*10000/livelen,
so with 76 at 2857 the block-0 value must reach pri < 2857, i.e. 3 refs with
livelen >= 11, or 2 refs.  The minimum honest ref count for the block-0 value
is 3 (one def plus the two arm reads the target's `ori $v0,$a0,1` /
`addu $v0,$a0,$0` pair requires), so the surviving sub-goal is precisely:
**a 3-reference block-0 value pseudo whose live length is >= 11**.  The
alternative, lifting 76 above 77's 7500 (the 3-ref case), needs 76 at 8 refs
with livelen 28; block 0 offers at most 5 (la + four memory accesses).

E61.6  THE MASK-VALUE SPLIT IS FREE ON THIS CHASSIS ONLY IF THE CARRIER IS A
GLOBAL ALLOCNO.  Giving the mask its own block-local `mv`
(tmp/grind/func_80034F88/s61/v/H.c) drops the block-0 value to 3 refs (pri
7500, livelen 4) but local-alloc seats `mv` in $v1 and `;; 76 conflicts` gains
hard reg 3 -- the same hard-reg block s60 removed.  Carrying the mask in the
OUTER value variable instead (v/L.c) keeps 76 free of hard-3 but makes 76
conflict with 74, closing the $v1 seat a different way.  Carrying the mask in
`c` (v/N.c, v/O.c) avoids both but costs 25 points (34 / 33).

E61.7  cse2's BLOCK-0 STORE-FORWARDING SURVIVES EVERY ZERO-COST INVALIDATOR
TRIED ON THE TWO-OBJECT CHASSIS.  Spelling the reload as a different lvalue
(`u = D_80106A70[3];` after `*q = u;`, v/B.c and v/D.c) folds identically (9,
nop retained).  Re-ordering so the reload follows `c = p[8] & 1;` (v/G.c) folds
identically.  The s59 FAKE dead re-set (`mv = raw;`, v/E.c) DOES defeat the
fold on this chassis but costs 6 points (15).  Reusing `c` as the mask carrier
(v/N.c) genuinely restores the lbu -- the store's value pseudo is overwritten
by `c = p[8] & 1;` before the reload, so cse2 has no live equivalent -- but it
costs an instruction (50) and 25 points.  So an ordinary-C invalidator EXISTS;
what is missing is one that does not also move the value out of its seat.

E61.8  CONTROLS THAT DID NOT MOVE THE SCORE (all 49/49, score 9): reload via
the array element (B), block-0 value split from blocks-1/2 value (C), B+C (D),
mask store via the array element (F), reload after the flag test (G), mask in a
block-local (H), q declared after the values (I), value declared before q (K),
mask in the outer v (L).  Score 9 is robust across the whole spelling family --
consistent with E61.4, where the defect is an ordering fact about ONE pseudo's
reference count, not a spelling fact.

E61.9  Hoisting the loop index's initialisation above the flag blocks (`i = 0;`
before block 0, `for (; i < 3; i++)`) -- the attempt to make the block-0 value
conflict with `i` and be pushed off $v1 -- costs three instructions (52) and
measures 31 (v/J.c).

- [s61] Chassis check: the s60 candidate.c re-measured on HEAD this session with the s59 aggregate declaration applied gives score 10, 49/49 -- the ledger floor was current, and the new floor of 9 is a real improvement measured against it in the same session.

- [s61] New floor 9 (49 target insns / 49 build insns), the first movement since s52. Body saved to memory/grind/func_80034F88/candidate.c with both granted /* FAKE */ annotations at the pointer declarations.

- [s61] All nine residual instructions are inside block 0. Blocks 1 and 2 and the trailing loop now match the target byte-for-byte AND in the target's emission order, including the la for blocks 1-2 being emitted at .L80034FC8 above block 0's store.

- [s61] Measured allocation for the candidate body (tools/ra_solver/extract.py, tmp/grind/func_80034F88/s61/C.model.json): ord 0 pseudo 73 i $v1 (11 refs/7, pri 47142); ord 1 pseudo 77 block-0 value $v1 (8/10, 24000); ord 2 pseudo 75 c $v0 (15/19, 23684); ord 3 pseudo 74 blocks-1/2 value $v1 (6/10, 12000); ord 4 pseudo 81 r $a0 (6/19, 6315); ord 5 pseudo 72 p $a1 (6/34, 3529); ord 6 pseudo 76 q $a2 (4/28, 2857).

- [s61] The conflict graph already admits the target seating: ';; 76 conflicts: 72 75 76 77 81 2 29' (73 and 74 absent) and ';; 77 conflicts: 72 75 76 77 2 29' (81 absent). r ($a0) and p ($a1) are already on their target seats.

- [s61] tools/ra_solver/inverse.py global --goal '{"76": 3}' --depth 2: minimal solution size 1 atom, 2 vectors -- [calls_crossed] 77 0->1 (unreachable) and [refs_down] 77 refs 8->2. Ten preference atoms FORECLOSED: '$v1 never appears as a hard reg in this function's pre-RA RTL, so global.c set_preference can never record a preference for it.'

- [s61] Derived sub-goal from the priority formula: the block-0 value pseudo must reach pri < 2857, i.e. 3 references with live length >= 11 (2 references is below its honest minimum, since the target's `ori $v0,$a0,1` / `addu $v0,$a0,$zero` pair needs two arm reads plus one def). The alternative -- lifting the address object above the 3-ref value's 7500 -- would need 8 references on an object that block 0 can give at most 5 (la plus four memory accesses).

- [s61] Score 9 is robust across nine independent spellings (reload via array element, block-0/blocks-1-2 value split, both together, mask store via element, reload after the flag test, block-local mask carrier, q declared after the values, value declared before q, mask in the outer value local) -- consistent with the defect being a reference-count ordering fact about one pseudo rather than a spelling fact.

- [s61] First ordinary-C defeat of cse2's block-0 store-forwarding on record: reusing the flag-test local `c` as the mask-store carrier overwrites the stored value's pseudo before the reload, and the target's `lbu` appears. It costs a re-seat (33-34), so the open question is an invalidator that does not disturb the seats.

- [s61] The s59 FAKE dead re-set is chassis-specific: it measures 15 here versus 9 for the identical body without it, and should not be carried forward onto the two-object chassis.

- [s61] The header edit (include/code6cac.h:472 `extern u8 D_80106A70[4];` plus the two element-form edits at src/code6cac.c:340,345) remains a discharged, byte-neutral integration handoff, now exercised across roughly 35 further builds this session without incident. src/ was reverted to HEAD at session end.


## s62 (structural, 2026-09-05) -- chassis re-measured at 9; object model split; the two defects proven to be one

E62.1  CHASSIS CHECK.  The s61 candidate (two granted pointer objects, `u8
D_80106A70[4]` declaration) re-measures 9 / 49 target insns / 49 build insns on
HEAD this session.  The ledger floor was current.

E62.2  THE SPLIT DECLARATION IS FREE AND STRICTLY MORE HONEST (frontier item 3
CONFIRMED).  `extern u8 D_80106A70[3];` (absorbing the D_80106A71/D_80106A72
scalars, with their two consumers in src/code6cac.c converted to element form)
plus `extern u8 D_80106A73;` left as the separate scalar it already is in
src/code6cac_b.c:128, with the candidate's pointers respelled `= &D_80106A73`:
score 9, 49/49 -- identical to the [4] form, and the extracted RA model is
byte-for-byte the same (order [73,77,75,74,81,72,76]; 77 = 8 refs/livelen
10/pri 24000; 76 = 4/28/2857).  Byte-neutrality of the header change on the
other consumers re-verified this session: func_8001945C, func_80019488 and
func_80037F40 each sandbox at score 0.  The [4] declaration should not be
carried forward; the [3] + separate-scalar pair is the model the target's three
%hi/%lo(D_80106A73) la pairs and the loop's %hi(D_80106A70) base actually name.

E62.3  EVERY REF-REDUCTION SPELLING OF BLOCK 0 BUYS ITS REF CUT WITH A
$v1-SEATED BLOCK-LOCAL.  Measured (score, then extract.py / local_extract.py):
  C  "u = *q & 0xF8; *q = u;" + arms read u, no reload stmt   9  77: 4 refs/8/10000, hard76 [2,3,29]
  D2 "u = *q & 0xF8; *q = u; u = *q;"                         9  77: 6/9/13333,      hard76 [2,3,29]
  D4 as D2 with the reload after the flag read                9  77: 6/9/13333,      hard76 [2,3,29]
  D5 "u = *q; *q = u & 0xF8; u = *q;"                         9  77: 5/7/14285,      hard76 [2,3,29]
  F  "*q = *q & 0xF8; u = *q;"                                9  77: 3/4/7500,       hard76 [2,3,29]
  H1 no value local at all, both arms read "*q"               9  78: 4/8/10000,      hard76 [2,3,29]
In each case local_extract.py shows ONE extra block-0 quantity that local-alloc
seats at hard reg 3 (F: "blk=0 ord=0 qty=0 reg1=78 birth=6 death=8 refs=2
got=3"), which is precisely the hard-reg-3 entry that appears in the address
allocno's hard_conflicts.  A hard conflict with $v1 forecloses the seat outright
-- no priority change can recover it.  The s61 candidate and H2 are the only
spellings measured that keep hard76 = [2, 29].

E62.4  ARMS-READ-THE-POINTER IS THE FIRST REF CUT THAT IS FREE (new best RA
state).  "u = *q; u = u & 0xF8; *q = u;" followed by arms that read "*q"
directly (duplicate-read-into-arms, ordinary C) measures 9 / 49 with
hard76 = [2, 29] AND drops the block-0 value allocno from 8 refs/pri 24000 to
6 refs/livelen 9/pri 13333; the address allocno improves to livelen 26/pri 3076.
The priority gap the seat needs closed is now 4.3x, down from 8.4x.  Banked as
the new candidate.c.  The mixed form (one arm reads "*q", the other reads the
local, H3) costs four instructions: 53 insns, score 13.

E62.5  THE SEAT DEFECT AND THE MISSING RELOAD ARE ONE DEFECT.  The target's
block-0 reload "lbu $a0, 0($v1)" (80034FB4) sits in the LOAD-DELAY SLOT of
"lw $v0, 0x20($a1)" (80034FB0) -- legal only because its destination differs
from the lw's.  K1/K3 (mask carried in "c", so "c = p[8] & 1;" clobbers the
stored value's pseudo and cse2 cannot forward) DO restore the reload; the
disassembly shows "lw v0,32(a2); nop; andi v1,v0,1; lbu v0,0(a0)" -- our reload
lands in $v0, collides with the lw, the scheduler cannot fill the slot, and the
body costs 50 insns / score 34.  Consequence: the ninth point is not a separate
lever.  Put the value in $a0 and the reload becomes free.

E62.6  THE TARGET'S SEATING IS SELF-CONSISTENT AND THE ARITHMETIC IS NOW
CLOSED.  Extracted on K1 (reload present): the address allocno gains a fifth
reference -> 5 refs / livelen 28 / pri 3571; the block-0 value is 3 refs.  A
3-reference value has pri 30000/livelen, so it falls below 3571 at livelen >= 9.
In the target's own emission order the reload stands about five instructions
above the last arm read (lbu, andi, bnez, ori, addu) -> livelen ~10 -> pri
~3000.  global.c's descending-priority loop then reaches the ADDRESS allocno
first, gives it $v1 (it conflicts with neither 73 nor 74), and pushes the value
to $a0 (it conflicts with neither 81 nor 82, which is why blocks 1-2 keep $a0).
K1 misses only because its reload is emitted BELOW the flag read (livelen 4,
pri 7500).  The open sub-goal is therefore exact: a cse2 invalidator that leaves
the reload at a source position ABOVE the flag read.

E62.7  ONE SUCH INVALIDATOR EXISTS BUT COSTS AN la.  Storing the mask through
the SYMBOL while reloading through the pointer ("D_80106A73 = u; u = *q;", L3)
defeats cse2 with the reload in the high position: the reload survives and the
score is 12 at 50 insns -- the closest non-49 form on record.  The cost is that
the symbol store re-materialises the address ("lui at; sb v1,0(at)") instead of
reusing the pointer's register.  The reverse (store through the pointer, reload
through the symbol) folds identically, as s61 measured.  A full symbol-form
read-modify-write (L1/L2) costs two instructions (51, score 35).

E62.8  FRONTIER ITEM 2 AS WRITTEN IS SEMANTICALLY INVALID.  Moving block 1's
"v = *r;" above block 0's reload changes what block 1 reads: block 1 must see
block 0's SECOND store (the flag-0 result), and hoisting the read above it drops
bit 0.  No measurement was spent; the probe is retired rather than killed.

- [s62] [s62] Chassis check: the s61 candidate re-measures 9 (49 target insns / 49 build insns) on HEAD this session; the ledger floor was current.

- [s62] [s62] The split declaration `extern u8 D_80106A70[3];` (absorbing D_80106A71/D_80106A72, their two consumers in src/code6cac.c converted to element form) plus the already-separate `extern u8 D_80106A73;` measures score 9 on func_80034F88 and score 0 on func_8001945C, func_80019488 and func_80037F40 -- byte-neutral, and it removes the last model stretch (an array whose fourth element was an unrelated census-named flag byte).

- [s62] [s62] Under the split declaration the extracted RA model is unchanged from the [4] form: order [73,77,75,74,81,72,76]; block-0 value 8 refs / livelen 10 / pri 24000; address object 4 refs / livelen 28 / pri 2857.

- [s62] [s62] New best RA state at the same score: with both arms reading *q, the block-0 value is 6 refs / livelen 9 / pri 13333 (ord 2) and the address object 4 refs / livelen 26 / pri 3076, with hard conflicts [2, 29] -- no local-alloc $v1 block.

- [s62] [s62] Six independent ref-reduction spellings (C, D2, D4, D5, F, H1) all score 9 and all add hard reg 3 to the address object's hard conflicts; local_extract.py names the culprit each time (e.g. F: blk=0 ord=0 qty=0 reg1=78 birth=6 death=8 refs=2 got=3). A hard conflict with $v1 forecloses the seat outright, so priority work on those chassis is wasted.

- [s62] [s62] The target's block-0 reload `lbu $a0, 0($v1)` at 80034FB4 occupies the load-delay slot of `lw $v0, 0x20($a1)` at 80034FB0; it can only sit there because the reload's destination differs from the lw's. K1 restores the reload into $v0, the slot takes a nop, and the body costs 50 insns / score 34.

- [s62] [s62] Seat arithmetic with the reload present (extracted on K1): the address object gains a fifth reference -> 5 refs / livelen 28 / pri 3571, while a 3-reference block-0 value has pri 30000/livelen and so drops below 3571 at livelen >= 9. The target's own emission order puts the reload about five instructions above the last arm read (livelen ~10, pri ~3000), which makes the target's seating self-consistent: the address object is reached first, takes $v1 (no conflict with 73 or 74), and the value is pushed to $a0 (no conflict with 81/82, which is why blocks 1-2 keep $a0).

- [s62] [s62] K1 misses that state only because its reload is emitted BELOW the flag read (livelen 4, pri 7500). The open sub-goal is exactly: a cse2 invalidator that leaves the reload at a source position ABOVE the flag read at zero instruction cost.

- [s62] [s62] Such an invalidator exists but is not free: storing the mask through the symbol while reloading through the pointer (L3) keeps the reload high and measures 50 insns / score 12 -- the closest non-49 form on record -- because the symbol store emits `lui at; sb v1,0(at)` instead of reusing the pointer register. The reverse direction folds identically (s61).

- [s62] [s62] Frontier item 2 as written (moving block 1's `v = *r;` above block 0's reload) is semantically invalid, not merely unmeasured: block 1 must read block 0's second store, and hoisting the read above it drops bit 0. Retired without spending a measurement.

- [s62] [s62] src/, include/ and the tree were restored to HEAD at session end; the split-declaration header edit remains an integration handoff documented in candidate.c.

## s63 (synthesis, 2026-09-05) -- the residual is a 3-cycle rotation around ONE seat, and the mask value takes that seat three different ways

E63.1  CHASSIS CHECK.  The s62 candidate (split declaration `extern u8
D_80106A70[3];` + `extern u8 D_80106A73;`, two granted pointer objects, both
block-0 arms reading `*q`) re-measures **score 9 / 49 target insns / 49 build
insns** on HEAD this session.  The ledger floor was current; the dispatch
digest's "measurement unavailable" is a driver artefact, not a chassis change.
The apply script (header + src/code6cac.c element form + body) is banked at
tmp/grind/func_80034F88/s63/apply.py so no future session has to re-derive it.

E63.2  KILL RE-AUDIT (mandated).  The two closest-to-target instance kills whose
`measured_on` named a superseded chassis were re-measured on the current
split-declaration chassis:
  * s61's "port the s59 FAKE dead re-set of the stored-value local onto the
    two-object chassis" -> **15 at 49 insns** (s61 recorded 15 on the [4]
    aggregate chassis).  Kill stands, verbatim.
  * s62's L3 (mask stored through the symbol, reload read through the pointer)
    -> **12 at 50 insns**.  Kill stands, verbatim.
Neither verdict was chassis-sensitive.  Banked as
tmp/grind/func_80034F88/s63/body_v1_deadreset_reload_high.c and
body_l3_symbol_store_ptr_reload.c.

E63.3  THE RESIDUAL, READ FROM THE OBJDUMP, IS EXACTLY THREE REGISTER
SUBSTITUTIONS.  The dead-re-set body (E63.2, score 15) builds 49 instructions
that match the target's stream one-for-one -- the block-0 reload in the
load-delay slot of `lw $v0,0x20(p)`, block 1's `la` between the arm's `move`
and block 0's second store, the `move p,$v0` after block 0's `lbu`, the whole
loop.  Side by side (ours | target):

    lui/addiu $a1        | lui/addiu $v1     block-0 address object
    lbu   $v1, 0($a1)    | lbu   $a0, 0($v1) block-0 value (mask)
    move  $a2, $v0       | addu  $a1, $v0, $zero   p
    andi  $v1, $v1, 0xf8 | andi  $a0, $a0, 0xF8
    sb    $v1, 0($a1)    | sb    $a0, 0($v1)
    lw    $v0, 32($a2)   | lw    $v0, 0x20($a1)
    lbu   $v1, 0($a1)    | lbu   $a0, 0($v1) the block-0 reload
    andi  $v0, $v0, 1    | andi  $v0, $v0, 0x1
    bnez  $v0            | bnez  $v0
    ori   $v0, $v1, 1    | ori   $v0, $a0, 0x1
    move  $v0, $v1       | addu  $v0, $a0, $zero
    lui/addiu $a0        | lui/addiu $a0     blocks-1/2 address object  (MATCH)
    sb    $v0, 0($a1)    | sb    $v0, 0($v1)

The divergence is a 3-cycle rotation: block-0 address $a1->$v1, block-0 value
$v1->$a0, p $a2->$a1.  Blocks-1/2's address ($a0), blocks-1/2's value ($v1),
the loop index ($v1) and the $v0 temp already sit in the target's registers.
The engine metric prices this at 15 while the reload-free body below it prices
at 9 -- the 9 is structurally FARTHER from the target, exactly as s58 warned.

E63.4  THE WHOLE ROTATION IS ONE SEAT.  Replaying find_reg by hand on the
extracted model: if block 0's ADDRESS allocno takes $v1, then at its own turn
the block-0 value finds $v0 hard-excluded and $v1 conflict-blocked and lands in
$a0 (it conflicts with neither blocks-1/2 allocno, which is why they keep $a0
and $v1), and `p` -- which conflicts with everything -- finds 2/3/4 taken and
lands in $a1.  Every remaining defect in this function is that ONE seat.

E63.5  THE s63 LAW: BLOCK 0's MASK VALUE TAKES $v1 AHEAD OF THE ADDRESS BY
THREE DIFFERENT MECHANISMS, AND THE C SPELLING ONLY CHOOSES WHICH.  All three
chassis below carry the reload, all three build 49 instructions, all three
score 15; the allocation numbers are from tools/ra_solver/extract.py:
  (a) MASK AND RELOAD IN ONE LOCAL (body_v1_deadreset_reload_high.c):
      one block-0 value allocno, 7 refs / livelen 8 / **pri 17500**, sorted to
      ord 2 and it takes $v1 on PRIORITY.  Address allocno 5 refs / livelen 28
      / pri 3571, hard conflicts [2, 29] (clean).
  (b) MASK IN ITS OWN LOCAL, RELOAD IN A SECOND LOCAL (body_w1_split_mask_
      reload.c, and w2/w3/w4 = flag-read-first, else-arm-reads-*q, merged mask
      expression -- four spellings, all identical):
      the block-0 value allocno drops to **3 refs / livelen 4 / pri 7500** (the
      ref cut works), but the mask is now a BLOCK-LOCAL quantity and
      local-alloc seats it in $v1, which puts hard reg 3 back into the address
      allocno's conflicts ([2, 3, 29]).  LOCAL-ALLOC takes the seat.
  (c) MASK CARRIED IN THE SAME LOCAL BLOCKS 1-2 USE (body_z2_mask_in_v.c):
      no block-local quantity anywhere in block 0 -- the address allocno's hard
      conflicts are back to [2, 29] -- AND the block-0 value allocno stays at
      3 refs / livelen 4 / pri 7500.  Both s62 defects are gone at once, the
      best RA state ever recorded here.  But the mask carrier is now allocno 74
      (10 refs / livelen 13 / **pri 23076**), it CONFLICTS with the address
      allocno (conflicts[76] = [72,74,75,76,77,81]) and it takes $v1 at ord 1.
      A CONFLICT takes the seat.
There is no fourth place to put a mask value: it is either in the reload's
local, in its own local, or in another block's local.

E63.6  THE INVERSE SOLVER PRICES CHASSIS (c) AT TWO ATOMS, BOTH BAD.
`inverse.py global` on the (c) model with goal {77:$a0, 76:$v1, 72:$a1}
(tmp/grind/func_80034F88/s63/inverse_z2.txt): "minimal solution size: 2
atom(s) -- 25 distinct vector(s)", and every emitted vector is
`calls_crossed 74: 0->1` paired with `refs_up 76: 5->8..15`.  Preferences stay
mechanically FORECLOSED (no hard reg for $v0/$a0/$a1 appears in this function's
pre-RA RTL).  A call-crossing restructure is not available (the function's only
call is the first statement) and +3 refs on the address object means three more
memory accesses through `q`.  Chassis (c) is the wrong one to grind despite the
clean numbers.

E63.7  THE TARGET IS CHASSIS (b), AND ITS MASK QUANTITY IS SEATED IN $a0.  The
target's block 0 is `lbu $a0 / andi $a0 / sb $a0` at 80034FA0-80034FAC and its
reload is `lbu $a0` at 80034FB4: the mask value and the reload value share a
register but the mask dies inside the first basic block, i.e. it is a
block-local quantity that local-alloc seated at **$a0**, while on our (b)
chassis the identical quantity is seated at **$v1**.  This retypes the last
open question in this function from a global.c priority question (where s49-s62
lived) to a **local_alloc/block_alloc find_free_reg question**:

    why does find_free_reg pick $a0 over $v1 for block 0's mask quantity in the
    original, when on our (b) chassis it picks $v1?

find_free_reg's `used` = fixed_reg_set | union(regs_live_at[birth..death]).
$v0 is already excluded on our chassis (func_80077D00's return value is still
live across block 0's `lbu`, which is why every extracted address allocno
carries hard conflict 2).  So either something in the original makes $v1 live
across the mask window, or the quantity is taking a SUGGESTED register through
qty_phys_copy_sugg / qty_phys_sugg -- the pass tools/ra_solver/local_alloc.py
reports but does not score, and which `local_extract.py --suggest` dumps.

E63.8  FORMS MEASURED AND KILLED THIS SESSION (all on the split-declaration
two-object chassis): the mask local reused for the flag test (`u = p[8] & 1;`
between the store and the reload) collapses the body to 47 insns / score 33;
block 1's pointer declaration moved into the window between the reload and the
arms (to lengthen the value's live range for free) costs an instruction, 50
insns / score 19; the same plus the loop index's initialisation hoisted into
that window, 52 insns / score 31; the block-0 value used in only ONE arm (the
other arm re-reading `*q`) splits into 51 insns / score 11 in the else-arm form
and folds back to the 3-reference shape in the then-arm form.

- [s63] Chassis check: the s62 split-declaration candidate re-measures 9 (49/49) on HEAD; tmp/grind/func_80034F88/s63/apply.py is the banked one-command chassis installer (header + src/code6cac.c element form + body).
- [s63] KILL RE-AUDIT: s61's dead-re-set port re-measures 15 at 49 insns and s62's L3 re-measures 12 at 50 insns on the current chassis -- both kills stand verbatim, neither was chassis-sensitive.
- [s63] The dead-re-set body reproduces the target's ENTIRE 49-instruction stream and its whole divergence is a 3-cycle register rotation: block-0 address $a1->$v1, block-0 value $v1->$a0, p $a2->$a1. Blocks-1/2's address and value, the loop index and the $v0 temp already match.
- [s63] Hand-replaying find_reg on the extracted model shows the rotation is ONE seat: give block 0's address allocno $v1 and the value falls into $a0 and p falls into $a1 by find_reg's own ascending scan, with no other change.
- [s63] THE s63 LAW: block 0's mask value takes $v1 ahead of the address object by three different mechanisms, and the C spelling only chooses which -- (a) mask+reload in one local: 7 refs / pri 17500, takes it on PRIORITY; (b) mask in its own local: value drops to 3 refs / pri 7500 but the mask becomes block-local and LOCAL-ALLOC seats it in $v1 (hard conflict 3 on the address); (c) mask in the blocks-1/2 local: no block-local quantity and value still 3 refs, but the carrier becomes a 10-ref / pri 23076 allocno that CONFLICTS with the address and takes $v1 at ord 1.
- [s63] Chassis (c) is the best RA state ever recorded here (address hard conflicts [2,29] AND block-0 value at 3 refs simultaneously -- the two s62 defects gone at once) and is still score 15; inverse.py prices its fix at 2 atoms, every vector pairing calls_crossed 74:0->1 with refs_up 76:5->8..15, both unavailable in C.
- [s63] The target is chassis (b): its mask value is a block-local quantity seated at $a0 (lbu/andi/sb $a0 at 80034FA0-FAC) where ours is seated at $v1. The last open question in this function is therefore a local_alloc find_free_reg question, not a global.c priority question.

- [s63] Chassis check: the s62 split-declaration candidate re-measures score 9 / 49 target insns / 49 build insns on HEAD; tmp/grind/func_80034F88/s63/apply.py is the banked one-command chassis installer (include/code6cac.h split declaration + src/code6cac.c element form + body), so no future session re-derives it.

- [s63] The dead-re-set body reproduces the target's ENTIRE 49-instruction stream and its whole divergence is a 3-cycle register rotation: block-0 address $a1->$v1, block-0 value $v1->$a0, p $a2->$a1. The score-9 body without the invalidator is structurally FARTHER (no reload, load-delay nop).

- [s63] THE s63 LAW: block 0's mask value takes $v1 ahead of the address object by three different mechanisms, and the C spelling only chooses which. (a) mask+reload in one local: one value allocno, 7 refs / livelen 8 / pri 17500, takes $v1 on PRIORITY, address hard conflicts clean [2,29]. (b) mask in its own local: value drops to 3 refs / livelen 4 / pri 7500 but the mask becomes block-local and LOCAL-ALLOC seats it in $v1, address hard conflicts [2,3,29]. (c) mask in the blocks-1/2 local: no block-local quantity and value still 3 refs, but the carrier becomes a 10-ref / pri 23076 allocno that CONFLICTS with the address and takes $v1 at ord 1.

- [s63] There is no fourth place for a mask value: it sits in the reload's local, in its own local, or in another block's local -- (a)(b)(c) exhaust the spelling axis, and all three build 49 instructions at score 15.

- [s63] Chassis (c) is the best RA state ever recorded on this function (address hard conflicts [2,29] AND block-0 value at 3 references simultaneously). inverse.py global prices its fix at a minimum of 2 atoms over 25 vectors, every one pairing calls_crossed 74:0->1 with refs_up 76:5->8..15; the function's only call is its first statement so the call-crossing atom is unavailable, and +3 refs on the address object means three more memory accesses.

- [s63] The target is chassis (b): its mask value is a block-local quantity seated at $a0 (lbu/andi/sb $a0 at 80034FA0-FAC, reload lbu $a0 at 80034FB4) where the identical quantity on our (b) chassis is seated at $v1. The last open question in this function is therefore a local_alloc/block_alloc find_free_reg question, not a global.c priority question -- the first time in 63 sessions the residual has moved out of global.c.

- [s63] find_free_reg's `used` = fixed_reg_set | union(regs_live_at[birth..death]); $v0 is already excluded on our chassis because func_80077D00's return value is still live across block 0's lbu (which is why every extracted address allocno carries hard conflict 2). So the target either makes $v1 live across the mask window or the quantity takes a SUGGESTED register through qty_phys_copy_sugg / qty_phys_sugg -- rows local_alloc.py reports but does not score, dumped by local_extract.py --suggest.

- [s63] Priority arithmetic for the seat, measured: pri = floor_log2(refs)*refs/livelen*10000. The address object is 5 refs / livelen 28 / 3571. A 3-reference block-0 value needs livelen >= 9 to fall below it; a 2-reference value needs livelen >= 6; raising the address object instead needs 8 references (the floor_log2 2->3 jump) since 7 refs only reaches 2500.

- [s63] src/, include/ and the tree were restored to HEAD at session end; the split-declaration header edit remains an integration handoff documented in candidate.c and scripted in s63/apply.py.

## s64 (synthesis, 2026-09-05) -- THE FLOOR MOVES 9 -> 2.  The 64-session
## register rotation is SOLVED; the residual is two instructions in the loop.

E64.1  THE HEADLINE.  A body that reuses block 0's value local as the copy
loop's byte temp measures **score 2 at 49 target / 49 build instructions** on
the split-declaration chassis.  Every register seat the grind has been chasing
since s49 -- block 0's address at $v1, block 0's value at $a0, `p` at $a1 --
comes out at the target's value, together with all three la pairs, the block-0
reload in the load-delay slot of the flag lw, and blocks 1/2's $a0/$v1.  The
whole remaining divergence is one register field in each of two loop
instructions: ours `lbu $a0, 0x17($v0)` / `sb $a0, %lo(D_80106A70)($at)`
against the target's `lbu $v0, ...` / `sb $v0, ...`.  Body:
memory/grind/func_80034F88/candidate.c (= tmp/grind/func_80034F88/s64/
body_a1_value_reused_as_loop_temp.c).

E64.2  HOW IT WAS FOUND -- inverse.py on the (a) model, which s63 never ran.
s63 ran `inverse.py global` only on the chassis-(c) model (z2) and got
"minimal solution size: 2 atoms", both unavailable, and concluded (c) was the
wrong chassis to grind.  It also concluded from the target's `lbu $a0 / andi
$a0 / sb $a0` that "the target is chassis (b)" and pointed the next session at
local_alloc.  Running the SAME solver on the chassis-(a) model
(tmp/grind/func_80034F88/s64/a.model.json; goal {76:$v1, 77:$a0, 72:$a1})
returns **"minimal solution size: 1 atom(s) -- 6 distinct vector(s)"**
(tmp/grind/func_80034F88/s64/inverse_a.txt).  The cheapest vector is
`[conflict_add] pseudo 73: conflict +77` -- make the LOOP INDEX conflict with
block 0's VALUE -- and the lever the solver names for it is
"(variable identity) reuse one variable across both regions".  The other four:
`refs_down 77: 7->2` and `7->1` (not spellable: the value's 7 references are
lbu + andi-use + andi-set + sb + reload-lbu + ori + move) and `refs_up 76:
5->16` and `5->17` (eleven extra references on the address object).

E64.3  THE ALLOCATION, MEASURED (tmp/grind/func_80034F88/s64/a1.model.json).
  ord0 p73 loop index          11 refs / livelen  7 / pri 47142 -> $v1
  ord1 p76 value + loop temp   11 refs / livelen 10 / pri 33000 -> $a0  TARGET
  ord2 p75 c                   15 refs / livelen 19 / pri 23684 -> $v0  TARGET
  ord3 p74 v                    6 refs / livelen 10 / pri 12000 -> $v1  TARGET
  ord4 p81 r                    6 refs / livelen 19 / pri  6315 -> $a0  TARGET
  ord5 p77 q (block-0 address)  5 refs / livelen 28 / pri  3571 -> $v1  TARGET
  ord6 p72 p                    6 refs / livelen 34 / pri  3529 -> $a1  TARGET
conflicts[73] gains 76, which is the whole difference from the s63 baseline
(where conflicts[73] = [72, 73] and the value took $v1 at ord 2).  The address
object needed no priority change at all: once $v1 is occupied by the index at
its own turn and the value is pushed to $a0, find_reg's ascending scan hands
the address $v1 unaided, exactly as E63.4 predicted.

E64.4  WHY THIS SPELLING IS CAPPED AT 2.  The conflict is bought by making
block 0's value live across the loop, and the only byte-free way to be live in
the loop is to BE the loop's byte temp.  GCC 2.7.2 assigns one hard register
per allocno (global.c:1275, no live-range splitting), and the target needs that
value in $a0 in block 0 and in $v0 in the loop -- the loop's byte temp shares
$v0 with the loop's address temp (`addu $v0,$a1,$v1` / `lbu $v0,0x17($v0)`).
So no spelling of the loop-index-conflict route reaches 0; 2 is its floor.

E64.5  THE OTHER $v1 ALLOCNO, AND WHY REUSE INTO BLOCKS 1/2 DOES NOT WORK.
The only other allocno seated at $v1 is p74 (blocks 1/2's value).  A conflict
with it would seat block 0's value at $a0 the same way.  Measured this session:
reusing the block-0 value local in block 2 (rejected/s64-u-reused-block2-
score10.c) gives score 10 at 49 insns and the extracted model shows
conflicts[76] = [72, 75, 76, 77, 81] -- NO conflict with 74, because the local
is dead throughout block 1, so global.c never sees them live together; the
merged carrier then takes $v1 itself at ord 2 (10 refs / pri 23076).  Reusing
it in block 1 (score 15) and in both blocks (score 15) MERGE the two values
into one allocno rather than making them conflict.

E64.6  CHASSIS (b) IS FORECLOSED FOR THE SEAT -- s63's frontier item 1 is dead
before it is probed.  `local_extract.py code6cac_b --func func_80034F88
--suggest` on the (b) chassis (mask in its own block-local carrier) dumps
find_free_reg's actual scanned sets (tmp/grind/func_80034F88/s64/
chassis_b.sugg.json): for block 0's mask quantity `used` (hard regs) is
{0, 1, 2, 26..31} -- **register 3 ($v1) is NOT in it** -- and `first_used`
equals `used`, with ncopysugg = 0 and nsugg = 0.  So the ascending scan at
tools/gcc-2.7.2/local-alloc.c:2249 hands the mask $v1 (the first free register
after the hard-excluded $v0), which is what puts hard conflict 3 on block 0's
address allocno and makes the $v1 seat unreachable on that chassis regardless
of priority.  There is no suggested-register pass to appeal to: the quantity
carries no copy suggestion and no arithmetic suggestion at all.  s63's planned
find_free_reg investigation is therefore answered and closed.

E64.7  Index-hoisting re-measured on chassis (a) with the cse2 invalidator in
place: `i = 0;` immediately after the call (rejected/s64-index-init-after-
call-52insn-score31.c) and `i = 0;` immediately after block 0's mask store
(rejected/s64-index-init-after-mask-store-52insn-score31.c) both build 52
instructions at score 31, reproducing s61's and s63's numbers on the newer
chassis.  Hoisting the index is not the way to buy the conflict; reusing the
variable is.

- [s64] FLOOR 9 -> 2.  Reusing block 0's value local as the copy loop's byte temp makes the loop index conflict with block 0's value, and that single conflict delivers every register seat in the function except the loop temp's own: 49/49 instructions, score 2, residual = one register field in `lbu $a0,0x17($v0)` and `sb $a0,%lo(D_80106A70)($at)`.
- [s64] inverse.py on the chassis-(a) model prices the target seating at ONE atom (`conflict_add 73 +77`, lever "variable identity"), where s63's run on the chassis-(c) model had priced it at two unavailable atoms; running the solver on the OTHER banked chassis was the whole session.
- [s64] The loop-index-conflict route is capped at score 2: the conflict requires block 0's value to be live in the loop, the only byte-free way to be live there is to be the loop's byte temp, and one allocno gets one hard register (global.c:1275) while the target needs $a0 in block 0 and $v0 in the loop.
- [s64] Reusing the block-0 value local in block 2 does NOT create a conflict with blocks 1/2's value (the local is dead throughout block 1, so global.c never marks them simultaneously live); reusing it in block 1 or in both blocks MERGES the allocnos instead.  Scores 10, 15, 15 at 49 insns.
- [s64] Chassis (b) is foreclosed for the $v1 seat: find_free_reg's measured `used` set for block 0's mask quantity is {0,1,2,26..31} with no register 3, `first_used` == `used`, and the quantity carries zero copy suggestions and zero arithmetic suggestions, so the ascending scan (local-alloc.c:2249) always hands it $v1 and always puts hard conflict 3 on the address allocno.

- [s64] FLOOR 9 -> 2. memory/grind/func_80034F88/candidate.c builds 49 instructions against the target's 49 and differs from the target in exactly two instructions, in one register field each: ours `lbu $a0,0x17($v0)` / `sb $a0,%lo(D_80106A70)($at)` versus the target's `lbu $v0,0x17($v0)` / `sb $v0,%lo(D_80106A70)($at)`.

- [s64] The 15-session $v1 seat hunt is over: block 0's address allocno now takes $v1, block 0's value takes $a0 and p takes $a1, with no priority change to any of them -- exactly the fall-out E63.4 predicted once the seat is freed.

- [s64] The lever is ordinary variable reuse: the block-0 value local is declared at function scope and reused as the copy loop's byte temp, which makes it conflict with the loop index (conflicts[73] gains 76) so the index takes $v1 at ord 0 and pushes the value to $a0.

- [s64] METHOD FINDING: s63 ran inverse.py only on the chassis-(c) model and read '2 atoms, both unavailable' as a property of the function. Running the same solver on the chassis-(a) model returns 'minimal solution size: 1 atom(s)'. The banked-chassis set must be swept with the solver, not sampled.

- [s64] s63's frontier item 1 (a local_alloc find_free_reg investigation on chassis (b)) is answered and closed without spending a session on it: the measured `used` set for block 0's mask quantity is {0,1,2,26..31}, first_used == used, and the quantity carries zero copy suggestions and zero arithmetic suggestions, so the ascending scan can only return $v1.

- [s64] The remaining two instructions are NOT a priority problem and not a local-alloc problem: they are the price of the conflict's current carrier. Block 0's value must be $a0 in block 0 and the loop's byte temp must be $v0, and one allocno gets one hard register (global.c:1275).

- [s64] INTEGRATION HANDOFF unchanged: the body needs `extern u8 D_80106A70[3];` (absorbing D_80106A71/D_80106A72, their two consumers in src/code6cac.c converted to element form) with `extern u8 D_80106A73;` left as its own scalar -- measured byte-neutral project-wide in s62/s63. Installer: python3 tmp/grind/func_80034F88/s63/apply.py <body.c>.

- [s64] src/, include/ and the tree were restored to HEAD at session end; only memory/grind/func_80034F88/ and tmp/ carry this session's output.

## s65 (synthesis, 2026-09-05) -- floor 2 holds; the residual is now stated as arithmetic, not as a spelling search

CHASSIS RE-MEASURE. `memory/grind/func_80034F88/candidate.c` installed with
`python3 tmp/grind/func_80034F88/s63/apply.py` measures **score 2, 49 target /
49 build instructions** on HEAD 2026-09-05. The dispatch brief's "measurement
unavailable" is resolved: the chassis is unchanged and every s64 verdict is
spendable verbatim.

THE PRIORITY FORMULA IS NOW KNOWN EXACTLY. global.c's allocno priority in this
build is

    pri = floor_log2(nrefs) * nrefs * 10000 / live_length

fitted with no free parameters against all seven ALLOCDBG lines of
`tmp/grind/func_80034F88/s64/a.model.json` and re-checked against the two
models extracted this session (`s65/b3.model.json`, `s65/v4.model.json`).
Baseline allocnos: 73 loop index 11 refs / 7 = 47142; 75 shared flag+result
local `c` 15 / 19 = 23684; 77 block-0 value `u` 7 / 8 = 17500; 74 blocks-1/2
value `v` 6 / 10 = 12000; 81 blocks-1/2 address `r` 6 / 19 = 6315; 76 block-0
address `q` 5 / 28 = 3571; 72 `p` 6 / 34 = 3529.

WHAT THE FORMULA SAYS ABOUT THE RESIDUAL. The target seating requires block 0's
value at $a0 while block 0's address takes $v1. find_reg scans hard registers
ascending, so the value takes $v1 unless one of exactly two things is true:

  (A) the address object is allocated BEFORE the value. That needs
      pri(q) > pri(u) = 17500. With q's live length fixed at 28 this needs
      floor_log2(n)*n > 49, i.e. **n >= 16 references on q** (n = 15 gives 45).
      Alternatively q's live length would have to fall to 5 or below.
  (B) the value CONFLICTS with an already-allocated allocno sitting in $v1.

BOTH BRANCHES ARE NOW BOUNDED BY MEASUREMENT.

Branch (B) -- the containment theorem. `q` is live continuously from its `la`
(target 80034F98) to block 0's store (80034FD0); `u` is live from the first
`lbu` (80034FA0) to the last arm read (80034FC4), strictly inside. So any
pseudo made live simultaneously with `u` is also live simultaneously with `q`,
and global.c records the conflict on BOTH. Since `q` is the second-lowest
priority allocno, the new conflict partner is always allocated first, takes
$v1, and forecloses the very seat we are buying. Measured directly this session
by reusing the blocks-1/2 value local `v` inside block 0: b1 (v carries the
flag test) 47 insns / 33; b2 (v carries the arms and the store source) 47 / 12;
b3 (both) 49 / 16. `s65/b3.model.json` shows the mechanism exactly --
conflicts[76 (u)] gains 74, 74 is allocated at ord 2 into $v1, u is correctly
pushed to $a0 and r to $a0 (both TARGET), but conflicts[77 (q)] also gained 74,
so q scans past register 3 to $a1 and p follows to $a2.

The one escape from the containment theorem is a conflict partner that lives
OUTSIDE q's range, which means `u` must stay live past block 0's store. But
blocks-1/2's address object `r` is born at 80034FC8, one instruction BEFORE
that store, and `r` must share $a0 with `u` in the target -- so extending `u`
past the store necessarily makes it conflict with `r` and costs `r` its seat.
This is why s64's only working conflict source was the loop index, and why that
route caps at 2: the conflict is bought by making `u` the loop's temp, and the
target's loop contains no $a0 reference at all.

Branch (A) -- no byte-neutral reference lift for `q` is known. The obvious
sanctioned one, duplicated-statement-into-arms on block 0's store, does NOT
re-merge here because the two arms are not identical (`c = u | 1;` vs
`c = u;`), so cross-jump never fires: v1 = 50 insns / 12, v2 = 50 / 12,
v3 = 52 / 19. Loop-body references to `q` are excluded on principle: they would
extend `q` into the loop and make it conflict with the index, which holds $v1
at ord 0.

CLOSING s64's FRONTIER ITEM 2. A loop spelling in which block 0's value stays
live across the loop while the byte temp is a separate pseudo does exist -- make
the value the loop's ADDRESS temp -- but the value still carries $a0, so the
loop emits `addu $a0,$a1,$v1; lbu $v0,0x17($a0)` against the target's
`addu $v0,$a1,$v1; lbu $v0,0x17($v0)`: w1 = 49 / 3, w9 = 49 / 3, both WORSE
than the score-2 body. w3 (`c = u` after the load) is folded by
copy-propagation and the conflict vanishes entirely (49 / 15); w7 (`u = i;`
inside the loop) costs an instruction (50 / 19).

OTHER MEASUREMENTS. Merging block 0's mask into one expression cuts `u` to 5
refs / pri 14285 but re-creates the block-local $v1 quantity, so
hard_conflicts[q] becomes [2, 3, 29] and the seat is foreclosed before global.c
runs (49 / 15, model `s65/v4.model.json`) -- s62's D-family kill, re-measured on
the current chassis with a model attached. Splitting the shared flag/result
local `c` into three per-block locals is codegen-neutral on both chassis
(score 2 stays 2, score 15 stays 15): the pseudos coalesce back into one
15-reference allocno, so the shared-`c` shape is not costing anything.

WHERE THIS LEAVES THE NEXT SESSION. Our refs and live lengths for `q` (5 / 28)
and `u` (7 / 8) are exactly what the TARGET's own instruction stream implies --
count them at 80034F98..80034FD0 and 80034FA0..80034FC4 -- yet the target's
allocation differs. One of the three modelling premises must therefore be
false, and finding which one is the highest-value probe left:
  (i) `q` is a global allocno in the target (it crosses the branch at 80034FBC,
      so local-alloc should not touch it) -- verify, because if the original C
      produced the 80034FD0 store through a pseudo that local-alloc seated in
      the join block, the entire global-allocno framing for `q` is wrong;
  (ii) `u`'s live length really is 8 in the target (the model's `live_length`
      is 28 for `q` where the .flow dump reports 14, so the two are not the same
      quantity -- find what doubles it, and whether any C shape shrinks `q`'s to
      5, which alone would give pri 20000 > 17500 and hand over branch (A));
  (iii) `u` and the loop index really do not conflict in the target.

- [s65] Chassis re-measure: memory/grind/func_80034F88/candidate.c installed via tmp/grind/func_80034F88/s63/apply.py measures score 2, 49 target / 49 build instructions on HEAD 2026-09-05. The dispatch brief's 'measurement unavailable' is resolved.

- [s65] global.c allocation priority in this build is exactly floor_log2(nrefs) * nrefs * 10000 / live_length -- fitted with no free parameters against all seven ALLOCDBG lines of s64/a.model.json and confirmed predictive on s65/b3.model.json and s65/v4.model.json.

- [s65] Baseline allocnos: 73 loop index 11/7 = 47142; 75 shared flag+result local 15/19 = 23684; 77 block-0 value 7/8 = 17500; 74 blocks-1/2 value 6/10 = 12000; 81 blocks-1/2 address 6/19 = 6315; 76 block-0 address 5/28 = 3571; 72 p 6/34 = 3529.

- [s65] CONTAINMENT ARGUMENT (measured three ways this session): block 0's address object is live continuously from 80034F98 to 80034FD0 and block 0's value is live strictly inside it (80034FA0 to 80034FC4), so no allocno can conflict with the value without also conflicting with the address; because the address is the second-lowest-priority allocno, that partner is allocated first, takes $v1, and forecloses the seat.

- [s65] The one escape from containment -- extending the block-0 value past block 0's store -- necessarily overlaps blocks-1/2's address object, which is born at 80034FC8, one instruction BEFORE that store, and which must share $a0 with the value in the target. This is why s64's only working conflict source was the loop index, and why that route caps at 2: the target's loop contains no $a0 reference at all.

- [s65] Duplicated-statement-into-arms on block 0's store does not cross-jump re-merge here because the arms are not identical (ori vs move): 50 insns / score 12. No byte-neutral reference lift for the address object is known.

- [s65] Merged-mask spelling re-creates a block-local $v1 quantity giving hard_conflicts[address] = [2, 3, 29] -- s62's D-family kill re-measured on the current chassis, now with the extracted model as evidence.

- [s65] OPEN CONTRADICTION for the next session: our refs and live lengths for the block-0 address (5 refs) and the block-0 value (7 refs) are exactly what the TARGET's own instruction stream implies -- count them at 80034F98..80034FD0 and 80034FA0..80034FC4 -- yet the target's seats differ from ours. One of three modelling premises must be false, and identifying which is now the highest-value probe.

## s66 (solver) — MATCHED, score 0, full build SHA1 == oracle

The 65-session residual closed on s65's own arithmetic. s65 established that
global.c prices an allocno at `floor_log2(nrefs) * nrefs * 10000 / live_length`
and that block 0's address object reaches the target's `$v1` seat only if it is
allocated before block 0's value (pri 17500) — i.e. only with **at least 16
references**. Every reference lift s65 tried cost instructions. The free one is
that the address object and the copy loop's counter are the **same C variable**:
the counter's eleven loop-depth-weighted references land on the address allocno,
and they land *after* the address's last use, so live length rises only 14 → 21.
Measured: 16 refs / len 21 / **pri 30476**, allocated at ord 1 into `$v1`.

With `$v1` already taken when block 0's value is allocated (and `$v0` held by
the flag/result allocno), `find_reg`'s ascending scan hands the value `$a0` —
the target register — with no conflict lever at all. That is the structural
difference from s64's route, which bought the same seat with a conflict against
the loop index and was therefore capped at score 2: there, block 0's value had
to *be* the loop's byte temp, and one allocno gets one hard register
(global.c:1275). Here the byte temp is a plain block-local that local-alloc
seats at `$v0`, exactly as the target has it.

All seven allocnos land on their target registers
(tmp/grind/func_80034F88/s66/z2.model.json):

    ord0 p74 c (flag/result)      19 refs / len 21 / pri 36190 -> $v0
    ord1 p76 q (address + index)  16 refs / len 21 / pri 30476 -> $v1
    ord2 p75 u (block-0 value)     7 refs / len  8 / pri 17500 -> $a0
    ord3 p73 v (blocks-1/2 value)  6 refs / len 10 / pri 12000 -> $v1
    ord4 p80 r (blocks-1/2 addr)   6 refs / len 19 / pri  6315 -> $a0
    ord5 p72 p (record pointer)    6 refs / len 34 / pri  3529 -> $a1

`sandbox func_80034F88 --disable all` = **score 0, 49/49**. A full clean-driver
`build` produced SHA1 `62efab4f73f992798c43e8c730aa43baa10bb4fa` == oracle,
which also settles the s62/s63 integration question: the split declaration
(`extern u8 D_80106A70[3]` + `extern u8 D_80106A73`, with the two consumers in
src/code6cac.c converted to element form) is byte-neutral project-wide.

Spelling is load-bearing in two places: the bound must be the signed `(s32)q < 3`
(a `q < (u8 *)3` bound emits `sltu` where the target has `slti`, score 1), and
the source read must be `*((u8 *)p + (s32)q + 0x17)` rather than
`((u8 *)p)[(s32)q + 0x17]` (score 2).

## s66 (second dispatch) -- the function byte-matches; only the declaration's FILE is unresolved

- `memory/grind/func_80034F88/candidate.c` measures **score 0, 49/49 instructions**
  and a full clean-driver build SHA1 of `62efab4f73f992798c43e8c730aa43baa10bb4fa`
  (== the locked oracle) when installed with
  `python3 tmp/grind/func_80034F88/s63/apply.py`. Re-measured this session; the
  tree was then returned to HEAD and `verify-oracle` re-confirmed `ok: true`.
- The fix is one variable reuse: block 0's address object and the copy loop's
  counter are the same C variable, which lifts that allocno from 5 references /
  priority 3571 to 16 references / priority 30476 (live length only 14 -> 21,
  because the counter references come after the pointer's last use), so
  `global.c` seats it in `$v1` ahead of block 0's value allocno (pri 17500) and
  that value scans on to `$a0`. Seven of seven allocnos land on target seats
  (`tmp/grind/func_80034F88/s66/z2.model.json`).
- The body needs `extern u8 D_80106A70[3];` at its canonical extern
  (`include/code6cac.h:472-474`) with the two `src/code6cac.c` consumers
  converted to element form. Those two paths are out of this function's
  candidate scope, so the disposition is an INTEGRATION HANDOFF recorded at
  `docs/grind/decisions.md:23441`, requesting one `tools/grinder/scope_allow.txt`
  line: `func_80034F88 include/code6cac.h src/code6cac.c`.
- A TU-local spelling of the same declaration (block-scope
  `extern u8 D_80106A70[3];`) also measures 0 with an oracle SHA1 match and
  touches only `src/code6cac_b.c`, but is inadmissible under aggregate-merge
  prongs (c) and (d) (`.claude/rules/no-new-park-categories.md:238`). Banked at
  `rejected/s66-blockscope-array-decl-score0-but-prong-d-tu-local.c` so it is
  not re-derived as an in-scope win.

- [s66] func_80034F88 BYTE-MATCHES. memory/grind/func_80034F88/candidate.c installed via tmp/grind/func_80034F88/s63/apply.py measures sandbox score 0, target_insns 49, build_insns 49, rules_dropped 0, strip_cheat_asm true; full clean-driver build SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == the locked oracle.

- [s66] The declaration change is byte-neutral for every other consumer of D_80106A70/71/72 by the strongest proof the pipeline has -- the whole-project oracle build, not a per-function score.

- [s66] The blocker is a file, not a construct: the body must index a declared three-byte array (the target stores through lui $at,%hi(D_80106A70); addu $at,$at,$v1; sb $v0,%lo(D_80106A70)($at) at asm/funcs/func_80034F88.s:44-46), and .claude/rules/no-new-park-categories.md:238 prong (d) requires that declaration at its canonical extern, include/code6cac.h:472-474, with prong (c) converting the two consumers at src/code6cac.c:340-342 and :345 to element form.

- [s66] The requested grant is one line, self-serve class: `func_80034F88 include/code6cac.h src/code6cac.c` in tools/grinder/scope_allow.txt. Both paths match _SCOPE_GRANT_ALLOWED_RE at tools/grinder/grindlib.py:465 and neither is on _SCOPE_GRANT_DENY. Direct precedents already in that file: `replay_camera_Init include/code6cac.h` and `special_camera_get_rot_dir include/code6cac.h` (same header, same kind of honest array declaration) and `func_80061250 src/text1b.c src/text1b_b.c` (a second src/*.c granted for exactly this merge-the-consumers reason).

- [s66] The TU-local escape route was measured, not assumed: a block-scope extern array declaration reaches score 0 with an oracle SHA1 match touching only src/code6cac_b.c, and is banked as inadmissible under prongs (c) and (d) at rejected/s66-blockscope-array-decl-score0-but-prong-d-tu-local.c.

- [s66] The tree is back at HEAD: include/code6cac.h, src/code6cac.c and src/code6cac_b.c are unmodified, the function is still committed as INCLUDE_ASM("asm/funcs", func_80034F88);, and verify-oracle re-confirms ok true with build == oracle.

- [s66] Constructs the next session's layer-1 and Judge will rule on (all four already in candidate.c, all FAKE-annotated with what + mechanism + lever-exhaustion): two pointer aliases on &D_80106A73 (Judge-granted at docs/grind/decisions.md:23437), the `u = 0;` cse2 invalidator (dead-store family), and the variable reuse staging the loop counter through block 0's pointer (variable-reuse family; SOTN PSX precedent `// fake reuse of i?` at docs/reference/sotn-construct-index.md:92).

## s67 (2026-09-05, forensics dispatch executed as the s66 landing) — CANDIDATE-READY

- The pipeline scope grant s66 was waiting on is committed:
  `tools/grinder/scope_allow.txt` carries
  `func_80034F88 include/code6cac.h src/code6cac.c undefined_syms_auto.txt`.
  The out-of-scope Judge constraint from s66's first dispatch is therefore
  superseded for exactly these paths, which are both scope-checked AND staged
  into the Match commit.
- The banked s66 body was installed unchanged and re-proven THIS session:
  `sandbox func_80034F88 --disable all` = score 0, 49 target / 49 build
  instructions, rules_dropped 0; `verify-oracle` = ok true, build_sha1
  62efab4f73f992798c43e8c730aa43baa10bb4fa == original_sha1_locked.
- Prong (c) of the aggregate merge is now complete in the same candidate: no C
  code anywhere names D_80106A71 or D_80106A72, and both surviving
  `undefined_syms_auto.txt` rows carry
  `/* alias of D_80106A70+N; retire with func_8001BE20 */` for the
  still-INCLUDE_ASM sibling. The comment syntax follows the committed precedent
  at undefined_syms_auto.txt:527-528 (the func_800620B8 aliases).
- `memory/grind/func_80034F88/self_vet.md` is written against this diff: four
  claimed families (pointer-alias-fake-exception, dead-store-fake-exception,
  variable reuse, per-word-splat aggregate merge), each with a quoted scope
  sentence and a resolvable citation, and an ANNOTATION-CONFORMANCE block
  carrying all four `/* FAKE: ... */` lines verbatim. Both mechanical driver
  gates pass locally: `grindlib.validate_self_vet` -> (True, '') and
  `grindlib.check_banned_constructs` -> (True, '').
- TOOLING HAZARD FIXED (see hypotheses.md s67): `s63/apply.py` mojibakes every
  non-ASCII character in the files it rewrites (read_text without an encoding on
  this Windows host, then .encode() to UTF-8). It is invisible to the sandbox and
  the oracle because it only touches comments, but it inflates the header and
  src/code6cac.c diffs from 4 and 8 lines to 6 and 40 lines of corrupted prose.
  Use `tmp/grind/func_80034F88/s67/apply_utf8.py` instead.
- The one construct a reviewer is most likely to press on is the variable reuse:
  the copy loop's counter is staged through `q`, which is itself a FAKE pointer
  alias. The vet argues bound 2 of staged-value-reused-variable is satisfied
  because `q` was not invented in order to be borrowed — it performs three
  byte-materializing memory operations and predates the borrow by ten sessions —
  and bound 3 because the pointer value is dead from `*q = c;` onward. If layer-1
  disagrees, the correct next move is a `ruling-request` on that exact question
  (never a respelling: verdicts are body-keyed).

## s68-dispatch (labelled "session 67", rederive) — the bytes hold; the blocker is now the BAN, not the search

- CHASSIS RE-CONFIRMED THIS SESSION. `tools/grinder/scope_allow.txt` carries
  `func_80034F88 include/code6cac.h src/code6cac.c undefined_syms_auto.txt`.
  With `memory/grind/func_80034F88/candidate.c` installed over the INCLUDE_ASM
  line, `sandbox func_80034F88 --disable all` = **score 0, target_insns 49,
  build_insns 49, rules_dropped 0**. The dispatch brief's "measurement
  unavailable" is resolved: the score-0 chassis is intact.
- THE STANDING BLOCKER IS A CONTRADICTION IN THE RECORD, NOT A RESIDUAL. The
  Judge's 2026-09-05 14:44 ESCALATE ruling (`docs/grind/decisions.md:23549`,
  re-filed at `:23569`) installed the candidate itself, scored it, built the
  whole game, and ruled on every construct in it. On the loop-counter reuse it
  wrote: "The copy loop's counter is the same C variable as block 0's dead
  pointer handle. That is the frozen 'variable reuse for codegen control' entry
  (no-new-park-categories.md:185) ... Both values are real and used, the loop is
  in the target's own bytes, and the form is byte-neutral (49/49)." It then
  stated the only blocker was scope: "That is the whole blocker: a file the
  candidate is not allowed to edit, not a residual and not a disallowed
  construct." ELEVEN MINUTES LATER the layer-1 cheat-reviewer FAILed the same
  body on that same construct (`decisions.md:23598`), and the driver banked
  `banned_constructs` entry 2 from that FAIL. So the pipeline now holds a Judge
  PASS-in-substance and a layer-1-derived BAN on one construct, with the ban
  dated later. Layer-1 does not outrank the Judge, but the ban is what the
  driver enforces mechanically, so the contradiction has to be ruled on before
  the body can be spent. (Mechanically both driver gates currently pass:
  `grindlib.validate_self_vet` -> (True, '') and
  `grindlib.check_banned_constructs` -> (True, '') against the existing
  `self_vet.md`. Submitting anyway would put the ONLY known matching body in
  front of a FINAL CALL that could kill it permanently, which is why this
  session files the ruling request instead.)
- CITATION CORRECTION FOR THE RECORD. The 14:44 ruling says the ledger's SOTN
  precedent line ":92" is wrong and that the correct index lines are ":153" and
  ":360". That is itself a mis-read: 153 and 360 are the line numbers *inside
  the SOTN source files*, not inside the index. In
  `docs/reference/sotn-construct-index.md` the PSX `// fake reuse of i?` rows are
  at index lines **51, 81, 92, 97 and 109** (`src/boss/mar/cutscene.c:172`,
  `src/st/cen/cutscene.c:211`, `src/st/lib/cutscene.c:153`,
  `src/st/no3/cutscene.c:360`, `src/st/top/cutscene.c:143`); index line 93 is the
  PSP twin and does not count, and index lines 153/360 are unrelated rows
  (`EntityFakeSypha`, a return-type comment). The self-vet's existing
  `:92` citation is therefore CORRECT and must not be "fixed" to :153/:360.
- BOTH REMAINING FAKEs ARE LOAD-BEARING (measured this session, three sandbox
  runs). Dropping the `u = 0;` cse2 invalidator: **score 27** at 49 insns.
  Dropping the `r = &D_80106A73;` re-initialisation before block 2: **score 39**
  at 47 insns. Dropping both: **score 38** at 47 insns. Neither is decorative and
  neither can be trimmed to shrink the review surface.
- THE `(s32)` CASTS ARE FORCED BY THE TARGET'S ADDRESSING, NOT CHOSEN TO INFLATE
  `reg_n_refs` — which is the factual premise `banned_constructs` entry 2 rests
  on. The target's loop keeps a plain integer index in `$v1` and uses it against
  TWO different bases (`addu $v0,$a1,$v1` for the source and
  `addu $at,$at,$v1` for the destination store,
  `asm/funcs/func_80034F88.s:38-50`). A cast-free natural pointer walk therefore
  cannot express it: `for (q = D_80106A70; q < D_80106A70 + 3; q++)` with the
  source read as `*((u8 *)p + (q - D_80106A70) + 0x17)` builds **54 instructions
  at score 13** (identically 54/13 whether the byte goes straight to `*q` or
  through the shared `c`). Five extra instructions is what the cast-free spelling
  costs. The casts exist because the value the loop needs is an INDEX and the
  variable that must carry it is a pointer.
- THE CARRIER IS NOT FREE EITHER. Staging the counter through `r` (the blocks-1/2
  address object) instead of `q` measures **score 21** at 49 insns: `r`'s lifted
  allocno takes the `$v1` seat that block 0's address object needs, so the seat
  that s66 bought is lost. `q` is the only carrier that produces the target's
  allocation.
