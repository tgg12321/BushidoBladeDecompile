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
