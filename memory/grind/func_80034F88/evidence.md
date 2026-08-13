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
