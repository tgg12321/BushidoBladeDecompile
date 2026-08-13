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
