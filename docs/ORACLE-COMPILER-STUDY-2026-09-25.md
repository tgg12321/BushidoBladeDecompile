# Oracle-compiler study — what separates the kept `addu` sites from the `ori` sites (2026-09-25)

> **Superseded as policy (owner ruling 2026-09-26, Q17):** a compiler patch is a cheat.
> This study and the narrow-patch adoption it led to (9bc64b751) are history; its findings
> no longer justify patching the build compiler. See `.claude/rules/no-compiler-divergence.md`
> § "Owner ruling 2026-09-26 (ninth batch, Q17)".

**Scope.** This is the scratch-only study authorized by the 2026-09-25 owner ruling
([ORACLE-COMPILER.md](ORACLE-COMPILER.md) § "Owner ruling 2026-09-25", item (C)).
Every compiler variant was built in the WSL home directory (`~/cc1study/`). Every
full build ran on a scratch copy of the tree, with `CC1=` given on the `make`
command line. Nothing in the repository changed except this report and a
one-line pointer in ORACLE-COMPILER.md. The build compiler, the diagnostic
compiler, the live `tools/gcc-2.7.2/` tree, the Makefile, `CC_FLAGS`,
`engine/buildconfig.py` and `build/` are all untouched. **Adoption is not
authorized.** Nothing here is grounds for a completion or a construct ruling
(item (C)).

## Answer

**Partial.** One simple condition explains every site that can be compiled today:

> Rewrite `a + b` as `a | b` when the bits are disjoint, **except** when the
> addition is a plain register plus a constant, `(plus REG CONST_INT)`.

A compiler built with that condition in place of the project patch does all of
the following:

- It builds the **unchanged current tree** to the oracle SHA1
  `62efab4f73f992798c43e8c730aa43baa10bb4fa`. That was a full scratch build.
- It builds the tree to the **same SHA1** with func_80073C78's honest `+`
  candidate (`memory/grind/func_80073C78/candidate.c`) spliced in for its
  `INCLUDE_ASM`. The function becomes byte-complete, and the target's
  `ori a1,v1,0x0` / `ori a0,v0,0x0` come out of the natural `u + du0`.
- It compiles **site C** (`main`) to the target bytes from its natural
  fresh-expression spelling `((D_800A36F1 - 1) << 8) + 0x80`. The patched
  compiler needs the granted FAKE same-pseudo chain there.
- It keeps **site A**'s `andi 3 ; addiu 4`.

It is only a partial answer, for three reasons:

1. **Only three sites discriminate.** In the current tree plus the queue's compilable
   candidates, the rewrite changes the final bytes at exactly three places:
   - site A (the original kept `addu`);
   - site C (the original matches the rewrite-performing compilers);
   - func_80073C78 (the original has `ori`).

   Every other inventoried site is neutral: stock GCC, Sony's cc1psx and the
   patched compiler all produce the same bytes there from the current C.
2. **Other conditions also fit.** A second narrowing tested here, `exprop`, fits
   all three sites too. The two disagree on isolated register+register shapes
   such as `(p[0] << 8) + p[1]`, and no compiled site in the game decides
   between them yet.
3. **No upstream precedent.** GCC 2.5.8, 2.7.2 and 2.8.1 all do the rewrite
   unconditionally. GCC 3.x's own later narrowing ("only if the IOR simplifies
   further") was tested here and fails at func_80073C78 and site C. The
   recommended condition is therefore an inference from three data points, not a
   recovered historical compiler.

## 1. Site inventory

### 1.1 The rewrite at every known site

Columns:
- *stock*: pristine upstream 43d1cdb6.
- *oracle*: pristine plus `tools/cc1-no-plus-to-ior.patch`. It is behaviourally
  identical to the operative `build/cc1` on all 34 TUs.
- *cc1psx*: Sony's `tools/cc1psx.exe`, calibration only.
- *narrow*: the candidate in §4.

"✓" means the output equals the shipped bytes.

| site | shipped bytes | current C | stock | oracle | cc1psx | narrow | deciding RTL (from the instrumented log) |
|---|---|---|---|---|---|---|---|
| **A** func_800174F4 0x800175C4 (`ings`, formerly "gnd_disp_loop_ctrl") | `andi v0,v0,3 ; addiu s1,v0,4` | `v0 = rand(); v0 &= 3; h = v0 + 4;` (`h` is `unsigned short`) | `ori` ✗ | ✓ | `ori` ✗ | ✓ | 2-insn combine, i3 = the HI store to `h` (uid 121), i2 = the SI add (uid 119): `(plus:SI (reg/v:SI 86) (const_int 4))`, nonzero bits 0x3 / 0x4. The 0x3 comes from `reg_last_set_value` of a pseudo set twice (`= rand()`, `&= 3`). **REG + CONST_INT.** |
| **C** `main` 0x80017360–0x8001736C (`ings`) | `addiu -1 ; sll 8 ; addiu 0x80` | granted FAKE chain `lim = lim - 1; lim <<= 8; lim += 0x80` | ✓ | ✓ | (not run on the chain) | ✓ | chain: neutral, all compilers agree |
| C, natural spelling `((D_800A36F1 - 1) << 8) + 0x80` (4 variants: fresh expr, fresh var, `*256+128` ×2) | same | (scratch only) | ✓ | ✗ `sll 8 ; addiu -128` | ✓ | ✓ | 3-insn combine i3 = 181, i2 = 179, i1 = 178: `(plus:SI (ashift:SI (reg 106) (const_int 8)) (const_int 128))`, nonzero bits 0xffffff00 / 0x80. The rewrite turns the PLUS into an IOR, which blocks combine from distributing `(x-1)<<8` into `(x<<8)-256`. The combination then fails and all three insns survive. **Expression + CONST_INT.** |
| **func_80073C78** 0x800740E0 / 0x800740F4 (`text1b`, INCOMPLETE) | `ori a1,v1,0x0` / `ori a0,v0,0x0` | honest `+` candidate (floor 2/362) | ✓ | ✗ `addu a1,zero,v1` | ✓ | ✓ | 2-insn combine i3 = 684 / 693: `(plus:SI (subreg:SI (reg/v:HI du0) 0) (reg/v:SI u))`, nonzero bits 0x0 / 0x1ffff. `du0` is an always-0 local, and reload later substitutes its REG_EQUIV 0. **REG + REG.** |
| sprintf 0x80079F0C (`text1b_b`, the octal digit, formerly "func_80079A30") | `andi v0,a0,7 ; addiu v0,v0,0x30` | `*--bufPtr = (num % 8U) + '0';` | ✓ | ✓ | ✓ | ✓ | `(plus (and (reg 77) 7) 48)` is rewritten, but the combination is rejected and undone: a 2-insn combine that fails recognition is not split. **Neutral.** See §6, correction 1. |
| **B** func_80033DF4 0x80033F74 (`code6cac_b`, the multiply idiom) | `sll a0,v0,2 ; addu a0,a0,v0` | current C | ✓ | ✓ | ✓ | ✓ | `(plus (reg 136) (reg 133))`, nonzero bits 0x4 / 0x1. The rewrite is attempted, then rejected. **Neutral.** |

Site A under stock was probed with 19 respellings in its real TU. They
covered the statement shape, `v0` as s32/u32/s16/u16/u8, `h` as
u16/s16/s32/u32/int, and `% 4` forms. Every spelling that keeps the rest of
the function matching gets `ori` under stock. The rewrite at A depends on `h`
being 16-bit: the add feeds a HI store, and that store is what gives combine
its 2-insn combination. The respellings that change `h`'s type break the
function elsewhere (17–38 lines) under every compiler, so none of them is an
honest alternative. This agrees with the 2026-08-07 Phase-0 finding of 21
spellings. Site A is real evidence against unconditional rewriting.

### 1.2 The forensics' isolated probes (`r1`–`r5`, recreated in `tmp/pior/probes.c`)

The 2026-08-07 files `tmp/fx_*` no longer exist, so the probes were rebuilt
from the table in `docs/grind/cc1-forensics-2026-08-07.md`. "IOR" means the
output contains `or`/`ori`.

| probe | C | oracle | cc1psx | stock | narrow | exprop (study) |
|---|---|---|---|---|---|---|
| r1 | `(x & 3) + 4` | add | IOR | IOR | add | add |
| r2 | `(t<<2) + t`, t = a<b | add | IOR | IOR | **IOR** | add |
| r3 | `(a&1) + (a&2)` | `andi 3` | `andi 3` | `andi 3` | `andi 3` | `andi 3` |
| r4 | `((v-1)<<8) + 128` | add | IOR | IOR | add | add |
| r5 | `(hi<<8) + lo`, lo u8 | add | IOR | IOR | **IOR** | add |
| rA | site A's shape in isolation | add | IOR | IOR | add | add |
| rC | `(p[0]<<8) + p[1]` | add | IOR | IOR | **IOR** | add |

These probes are synthetic. The original game has no bytes for them, so they
constrain nothing about the original compiler. They show where the candidate
conditions differ, and they confirm the forensics' finding that cc1psx rewrites
every reachable shape.

### 1.3 Scan of the shipped binary

`tmp/pior/scan_target.py` walks every `asm/funcs/*.s` file (the original
EXE). It tracks per-block nonzero-bits by local dataflow: `andi`, `lbu`, `lhu`,
`sll`, `srl`, `slt*`, constants, and moves. It records every
`addu`/`addiu`/`or`/`ori` whose two operands are provably bit-disjoint.

| class | in COMPLETED-C functions | in INCLUDE_ASM functions |
|---|---|---|
| ADD kept (`addu`/`addiu` with disjoint operands) | 67 | 28 |
| IOR (`or`/`ori` with disjoint operands) | 120 | 175 |

**Zero-operand signature.** The whole binary has exactly two `ori rX,rY,0x0`,
both in func_80073C78. It has **no** `addu rX,$zero,rY` (zero as the first
source operand). A kept PLUS with a reload-substituted zero would print in that
form; `move` assembles as `addu rX,rY,$zero`. So no site in the game
contradicts func_80073C78's class.

**How to read the counts:**
- Every ADD site in a C function compiles to the same bytes under stock, cc1psx
  (where run) and every narrowing that passes (a). In those contexts the rewrite
  never survives to the final code, so the sites are neutral.
- The IOR sites in C functions come from source `|`: the oracle compiler never
  rewrites, and the current C matches.
- The 28 ADD sites and 175 IOR sites in INCLUDE_ASM functions are **untested**.
  Their C does not exist yet.

Two ADD-site families in INCLUDE_ASM functions will test the candidates once
their C exists:
- register+register byte packing: func_80067D14 `lbu` plus `lbu << 8`, and
  func_80063084 `(x & 0xff) + (y << 8)`;
- shift-plus-mask: func_800198D0, func_8003993C, func_80048FFC, func_8005E54C,
  func_800693CC.

The scanner is a local heuristic. Its handful of "mask 0 + const" ADD entries
are reorg artifacts: a delay-slot `li 0` seen on the fall-through path. They are
not combine-time zero operands.

## 2. Instrumentation method

**Study compiler.** It lives in `~/cc1study/gcc` and is built by
`tmp/pior/setup.sh`:
- pristine upstream `43d1cdb67e…` (`git checkout -- .` on a copy);
- plus the operative compiler's 2026-08-24 `negate_rtx` declaration in `reorg.c`
  (see §7);
- plus `tmp/pior/patch_combine.py`, which instruments the PLUS→IOR site in
  `simplify_rtx`;
- built with the documented recipe: all objects `-O0 -g`, `combine.o` alone at
  `-O`, `-fgnu89-inline`.

**What the instrumentation records:**
- Every disjoint-bits opportunity (an `ATTEMPT` record): function, i3/i2/i1
  insn uids, mode, both operands' `nonzero_bits`, operand RTX codes, the full
  PLUS, and whether the active policy allowed the rewrite.
- An `ACCEPT` record whenever a `try_combine` that performed a rewrite commits.
  It includes the committed pattern(s).

**Policy selection.** The environment variable `BB2_PIOR` selects the policy at
run time, so one binary covers every policy:
- `never` reproduces the oracle compiler: 0 differing lines over all 34 TUs.
- `always` reproduces stock: it differs from the oracle only at site A.

**Validation.** The recommended condition was also built without
instrumentation, as a fixed binary from a real patch file
(`tmp/pior/build_cand.sh`). Two controls used the same pipeline:
- pristine plus the tracked patch: `simplify_rtx` 0x39ab, identical to the oracle;
- pristine stock: `simplify_rtx` 0x3a11.

These control sizes match ORACLE-COMPILER.md, so the scratch recipe is the
documented recipe.

**Headline log numbers, current tree, stock policy.** Over the 34 TUs there
were 281 disjoint-bits rewrites. Only **one** reached the final code:
func_800174F4, site A. Every other rewrite happened inside a combination that
was then rejected and undone. That is why the rewrite is so rarely visible, and
why sites like sprintf and B are neutral.

## 3. Narrowings tried

Each narrowing was scored on three criteria:
- (a) the current tree compiles identically to the oracle compiler on all 34 TUs;
- (b) func_80073C78's `+` candidate yields the target `ori` pair;
- (c) site C's natural spelling yields the target.

Site A is part of (a). Tools: `tmp/pior/sweep.sh`, `siteC.py`, `probes.sh`.

| policy | rewrite allowed when… | (a) tree | (b) f73c78 | (c) site C natural | notes |
|---|---|---|---|---|---|
| `always` | always (stock) | ✗ site A `ori` | ✓ | ✓ | = stock / cc1psx behaviour |
| `never` | never (current patch) | ✓ | ✗ | ✗ | the current oracle compiler |
| `zero` | one operand's nonzero bits are 0 | ✓ | ✓ | ✗ | |
| `zeroreg` | a REG operand is known zero | ✓ | ✓ | ✗ | |
| `noconst` | neither operand is a CONST_INT | ✓ | ✓ | ✗ | |
| `const` | an operand is a CONST_INT | ✗ | ✗ | — | |
| `regreg` | both operands are REG/SUBREG | ✓ | ✓ | ✗ | |
| `further` | the IOR simplifies further (GCC 3.x's rule) | ✓ | ✗ | ✗ | the real later-upstream narrowing does not fit |
| `sset` | disjoint using only whole-function nonzero bits for multiply-set pseudos | ✗ site A | ✓ | — | combine's own `reg_n_sets--` after merging makes pseudo 86 single-set before the deciding attempt |
| **`noregconst`** | **not `(plus REG/SUBREG CONST_INT)`** | ✓ | ✓ | ✓ | the recommended condition's family |
| `exprop` | some operand is neither a REG nor a CONST_INT, or a REG operand is zero | ✓ | ✓ | ✓ | also fits; differs from noregconst on r2/r5/rC |

The fixed-binary form of `noregconst` checks REG only, not SUBREG (§4). It
gives the same results on (a), (b) and (c). The only difference found is one
scratch respelling of site A, which is not the current C: `v0` declared `u8`.
That spelling gets `ori` under the fixed binary and `addiu` under the SUBREG
form. The current C matches under both.

### Full scratch builds (`tmp/pior/fullbuild.sh`)

Each build ran on a fresh copy of the tree, with `make build/bb2.exe CC1=<scratch cc1>`.

| compiler (scratch binary SHA1) | tree | EXE SHA1 | |
|---|---|---|---|
| pristine + tracked patch (`574beea7…`, control) | current | `62efab4f73f992798c43e8c730aa43baa10bb4fa` | **MATCH** |
| **narrow candidate** (`939035d4…`, simplify_rtx 0x3a6a) | current | `62efab4f73f992798c43e8c730aa43baa10bb4fa` | **MATCH** |
| **narrow candidate** | current + func_80073C78 `+` candidate spliced | `62efab4f73f992798c43e8c730aa43baa10bb4fa` | **MATCH** |
| pristine stock (`9d569de5…`, control) | current | `b868111dc5cb6debabe779ffe79c80afeec78490` | mismatch (site A) |
| pristine + tracked patch (control) | current + func_80073C78 `+` candidate | `0422ed345884dcb267013c2ec9a30c790041d6dc` | mismatch (the 2-insn floor) |

Every queue item with a compilable `memory/grind/*/candidate.c` was also
compiled. There were ten, the others failing on TU declaration conflicts
unrelated to this study. Under stock, `noregconst` and `exprop`, none of them
differs from the oracle compiler except func_80073C78.

## 4. Recommended candidate (NOT adopted): `tmp/pior/cc1-plus-to-ior-narrow.patch`

The patch applies to pristine upstream `combine.c` **in place of**
`tools/cc1-no-plus-to-ior.patch`, not on top of it:

```diff
@@ -3618,10 +3618,15 @@ simplify_rtx (x, op0_mode, last, in_dest)
       /* If we are adding two things that have no bits in common, convert
 	 the addition into an IOR.  This will often be further simplified,
 	 for example in cases like ((a & 1) + (a & 2)), which can
-	 become a & 3.  */
+	 become a & 3.
+
+	 A register plus a constant is left alone: it is the add-immediate
+	 and address form, and an IOR of an opaque register cannot simplify.  */
 
       if (GET_MODE_BITSIZE (mode) <= HOST_BITS_PER_WIDE_INT
+	  && ! (GET_CODE (XEXP (x, 0)) == REG
+		&& GET_CODE (XEXP (x, 1)) == CONST_INT)
 	  && (nonzero_bits (XEXP (x, 0), mode)
 	      & nonzero_bits (XEXP (x, 1), mode)) == 0)
 	return gen_binary (IOR, mode, XEXP (x, 0), XEXP (x, 1));
       break;
```

Line by line:

- **The comment lines** record the exception and its rationale. The rewrite
  exists so that masks and shifts can merge, as in `(a&1)+(a&2)` → `a&3`. An
  IOR whose operand is a bare register can never merge with anything, so
  rewriting `reg + const` gains nothing. That same form is the machine's
  `addiu` and the canonical address shape. GCC 3.x later added a guard on
  exactly this ground ("PLUS appears in many special purpose address
  arithmetic instructions"), though with a different test that does not fit
  the evidence.
- **`GET_MODE_BITSIZE (mode) <= HOST_BITS_PER_WIDE_INT`** is unchanged.
- **`! (GET_CODE (XEXP (x, 0)) == REG && GET_CODE (XEXP (x, 1)) == CONST_INT)`**
  is the new condition. Canonical RTL keeps a constant as the second operand
  of a PLUS, so testing operand 1 is enough. Only a plain `REG` is excluded;
  a `SUBREG` still converts. This line is what keeps site A's
  `(plus (reg 86) (const_int 4))` as `addiu`.
- **The nonzero-bits test and the `return`** are unchanged from stock. Site C's
  `(plus (ashift …) (const_int 128))` is an expression plus a constant, so it
  still converts, and so does func_80073C78's `(plus (subreg du0) (reg u))`,
  a register plus a register.

The condition does not key on any function name, address, or project-specific
fact.

## 5. What remains unexplained or untested

- **Which narrowing, if any, the original compiler had.** `noregconst` (the
  candidate) and `exprop` both fit every compiled site. They disagree on
  register+register disjoint adds: probes r2 (`(t<<2)+t`), r5 and rC (byte
  packing). The candidate rewrites those and `exprop` does not. The game's
  INCLUDE_ASM functions contain kept `addu` sites of exactly those shapes:
  func_80067D14 byte packing, and func_8003993C / func_80048FFC / func_800693CC
  `(x<<n) + (y&1)`. If their eventual honest C gets `or` under the candidate,
  the candidate is falsified for that shape. Their C-function siblings (e.g.
  func_8006B898, func_8006E2A8) compile to `addu` under every compiler, so
  context may again make them neutral. That cannot be known in advance.
- **The multiply idiom (571 `sll;addu` vs 0 `sll;or`).** It is consistent with
  every policy for the C functions checked. For the rest it is not evidence
  either way, because most of those operands are not provably disjoint.
- **cc1psx.** It rewrites at site A, which the original did not, and it agrees
  with the original at site C and func_80073C78. So the game was not built by
  this `cc1psx.exe`'s behaviour at site A — or site A's C is still wrong in a
  way no tested spelling reveals. The PsyQ build that shipped the game
  (PsyQ 3.5, 1997) is not necessarily the same build as `tools/cc1psx.exe`. A
  narrower rewrite in the shipped compiler is one explanation. It is not the
  only one.
- **No upstream precedent.** GCC 2.5.8 and 2.8.1 (downloaded to
  `tmp/pior/gccsrc/`) have the same unconditional rewrite and the same
  `nonzero_bits` REG logic as 2.7.2.

## 6. Corrections to the existing record (evidence, not rulings)

1. **sprintf (formerly func_80079A30) is neutral evidence.** ORACLE-COMPILER.md
   lists its `andi 7 ; addiu 0x30` as evidence that the original did NOT
   rewrite. From the current C, **stock GCC and cc1psx both emit
   `andi ; addu 48` there as well**, the same as the oracle compiler. The
   rewrite fires in a 2-insn combination that fails recognition and is undone.
   The site says nothing about the rewrite. Site A is the only standing `addu`
   evidence.
2. **Site C cuts the other way, and cleanly.** The natural fresh-expression
   spelling matches the shipped bytes under stock, cc1psx and the candidate. It
   fails only under the current patch, where it folds to `sll 8 ; addiu -128`.
   The FAKE same-pseudo chain granted on 2026-08-11 is needed **only because of
   the patch**. The grant's premise, "fresh-variable spellings fold", is
   compiler-dependent. This report takes no action on that; it is recorded for
   the owner.
3. **Sites B and C are no longer oracle-vs-stock divergences.** The 2026-08-07
   table listed five lines at three sites. The current tree differs between
   stock and the oracle compiler at **site A only**, 2 lines in `ings`.
   `tools/build_oracle_cc1.sh --stock` still expects `code6cac_b ings`, so its
   stock self-check would now report FAIL on a correct build.

## 7. Manifest drift (investigated read-only; not fixed)

`tools/gcc-2.7.2/reorg.c` hashes to `6e1cf6a9…`, while the manifest records
`73a15a52…`. Removing exactly the 2026-08-24 "BB2 crash fix" block (the
`negate_rtx` PROTO declaration and its comment, 9 lines plus one blank)
reproduces `73a15a5245d2e7ad55fa9e3c42d724488b1892d6` byte for byte. The drift
is therefore entirely the owner-approved crash fix (ruling `262930f1b`,
adoption `fea9fa2ac`), whose commit did not re-record ORACLE-COMPILER.md's
manifest.

The same adoption also moved the build compiler:

- `build/cc1` is now `0f438e42…`, where the manifest says `ea11be50…`.
- `ea11be50` survives as `build/cc1.PRE-CRASHFIX-045c9543`, whose suffix names
  the wrong hash.
- `oracle/manifest.json` `notes.cc1_build` records the rebuild.

This has two consequences:
- `tools/build_oracle_cc1.sh` refuses to run, as the borderline note says.
- If it did run, it would not reproduce the operative compiler. Its recipe
  applies only `cc1-no-plus-to-ior.patch`, so the output would lack the
  crash-fix declaration.

This study used pristine upstream plus that same declaration as its base. The
control (pristine + tracked patch + declaration) is behaviourally identical to
`build/cc1` on all 34 TUs and full-builds to the oracle SHA1. Re-recording the
manifest and folding the declaration into the recipe is a separate change.
The ruling does not allow this study to make it.

## 8. Reproduction (scratch; `tmp/` is gitignored)

| script | purpose |
|---|---|
| `tmp/pior/setup.sh` + `patch_combine.py` | build the instrumented, policy-selectable study cc1 in `~/cc1study/gcc` |
| `tmp/pior/tus.sh`, `cmp.sh`, `logsum.py` | compile all 34 TUs, compare with the oracle compiler, summarize the rewrite log |
| `tmp/pior/sweep.sh`, `f73.sh`, `splice.py`, `dump.sh` | per-policy sweep, the func_80073C78 candidate, RTL dumps |
| `tmp/pior/siteA.py`, `siteC.py`, `siteC_log.sh` | site A respellings, site C natural spellings, site C combine trace |
| `tmp/pior/probes.c`, `probes.sh`, `extra.sh`, `psx.sh`, `psx2.sh` | isolated probes; cc1psx cross-checks |
| `tmp/pior/cands.py` | every compilable grind candidate under each compiler |
| `tmp/pior/scan_target.py`, `sites_report.py` | shipped-binary inventory (`target_sites.tsv`) |
| `tmp/pior/build_cand.sh`, `fixed_eval.sh`, `fullbuild.sh` | fixed binaries from patch files; full scratch builds and SHA1 |
| `tmp/pior/cc1-plus-to-ior-narrow.patch` | the candidate (§4) |

## 9. Question for the owner (plain language)

> Our compiler has one change from standard GCC: it never turns `a + b` into
> `a | b`. This study found a narrower version of that change. The compiler
> *does* turn `a + b` into `a | b`, except when the sum is just "a variable plus
> a fixed number". With that version, the whole game still builds byte-for-byte
> identical. func_80073C78 then matches from its natural code. `main` would no
> longer need its special three-step spelling. The one place where the original
> kept a plain add still keeps it.
>
> The catch: only three places in the game currently tell the versions apart,
> and at least one other narrower version also fits all three. So this is the
> best fit to the evidence so far, not proof of what the original compiler did.
> Some functions not yet decompiled could later contradict it.
>
> Should we (1) switch the project's compiler to this narrower version, with
> the risk that a future function contradicts it and we revisit; or (2) keep
> the current compiler, leave func_80073C78 unfinished, and re-test the
> narrower version as more functions become C?
