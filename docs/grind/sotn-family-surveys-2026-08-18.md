# SOTN precedent surveys — the 7 refused technique families

Read-only research. No tracked BB2 file was modified. Survey corpus: shallow clone of
`github.com/Xeeynamo/sotn-decomp` master @ `8bd7c7779269905097c2df4d3559370e5e58e92b`
(2026-08-18, "Decompile `sat/HitDetection` (#3592)") at `tmp/sotn/` — 3,013 `.c` + 307 `.h`
under `src/`.

Sweep scripts (reproducible): `tmp/fam/sweep.py` (F1/F2/F6), `tmp/fam/f3.py` (F3),
`tmp/fam/f7.py` + `tmp/fam/f7b.py` (F7). Raw hit dumps: `tmp/fam/*.json`.

## Methodology (inherited from `memory/reference/sotn-prototype-struct-precedent-2026-08-10.md`)

1. **Matched-code check.** A construct only counts if it sits in a C body that SOTN actually
   builds and byte-checks. Every cited carrier file was checked for `INCLUDE_ASM` count;
   all primary exhibits below are in files with **`INCLUDE_ASM = 0`** (whole file decompiled).
   `src/pc/**` (PC port, `FIX_UB`, not byte-matched) is excluded from all verdicts.
2. **Directionality.** The 2026-08-10 memory's caveat applies: exhibiting construct X in SOTN
   licenses *writing* X, it does not license the inverse operation. Noted per family where relevant.
3. **Annotation weight.** SOTN marks knowingly-unnatural match-driven code with `!FAKE` /
   `FAKE?`. There are exactly **12 `!FAKE` sites** tree-wide; several are load-bearing here,
   so the whole catalog is reproduced in Appendix A.
4. **Port caveat.** `src/saturn/**` is a genuine byte-matched target (`config/check.saturn.sha`
   pins `GAME.PRG`, `T_BAT.PRG`, per-stage `.PRG`) but is SH-2, not MIPS/GCC 2.7.2. Hits there
   are flagged.

---

## F1 — constant-staging-through-a-reused-live-local

### Definition (quoted, `docs/grind/decisions.md:1361`, 2026-07-23 escalation, func_80061658)

> **(a) SANCTION the constant-staging-through-a-reused-live-local family**, contingent on an
> owner-run SOTN-master-branch census finding community precedent for "stage a CONSTANT through
> a reused live local to steer local-alloc register CHOICE on STRAIGHT-LINE code" (annotated
> `/* FAKE */`). The bytes-proven form is `val = 0x10FFFF; mask = val;`
>
> HONEST COST … the staging hop has no semantic purpose — a human writing this from spec would
> never route the constant through `val`; its only effect is the local-alloc copy-preference.

### Signatures searched

- `<local> = <literal const>;` followed within 3 lines by `<other local> = <that local>;`
  (the exact `val = 0x10FFFF; mask = val;` shape) — `tmp/fam/sweep.py`.
- Fabricated staging locals with no semantic purpose in straight-line matched code.
- Redundant re-assignment of an already-live local.

### Hits

**Exact shape: 2 candidates, both NOISE.**

- `src/dra/8D3E8.c:1258` — `angle = 0x800;` … `D_80138430 = angle;`. Rejected: the two statements
  are in different control-flow positions; `angle` is a real conditionally-set variable and the
  store is the semantic use.
- `src/st/no1/unk_36490.c:440` — `x1 = 0x7000;` … `xMin = x1;`. Rejected: `x1 = 0x7000` is a
  sentinel in an `else` arm and `xMin = x1` is inside a subsequent `if (x0 > x1)` — real logic.

**Adjacent-class shape: several REAL hits in matched code.**

- `src/weapon/w_049.c:293` (INCLUDE_ASM 0) — a constant assigned into a live local with no
  semantic purpose, self-annotated:
  ```c
  i = 0; // FAKE? why here?
  green = red >> 1;
  ...
  for (i = 0; i < 16; i++) {
  ```
  `i` is immediately re-initialized by the `for`. A dead constant store into a reused live local,
  kept for the match.
- `src/dra/42398.c:299-312` (INCLUDE_ASM 0) — a **reused live local** (`color_fake`) restaged
  redundantly, twice, with the source unchanged between:
  ```c
  color = *palette;
  color_fake = *palette;
  new_var4 = (color_fake & 0x1F) << 3;
  ...
  color_fake = color;      /* redundant — color_fake already holds this value */
  new_var4 = color_fake >> 2;
  ...
  color_fake = color;      /* redundant again */
  new_var3 = color_fake >> 7;
  ...
  color_fake = (i * 15) + 8;   /* never read; the same expression is stored to x0 below */
  ```
  The `_fake` naming is SOTN's own admission. This is the "route a value through a reused live
  local for codegen effect" mechanism — with a *value*, not a literal constant.
- `src/dra/menu.c:1993, 2009, 2017` (INCLUDE_ASM 0) — `j = menu->unk1D; // FAKE?` where `j` is
  overwritten a few lines later by `j = 16 - menu->unk1D;`. Dead assignment into a reused live local.
- `src/st/e_fire_warg.h:1232-1242` — **fabricated staging locals on straight-line code, permuter-found,
  PS1-only**, the closest structural analogue found:
  ```c
  #if !defined(VERSION_PSP)
      // These are both !FAKE; psp matches without them. ps1 needs
      // them. I dunno. permuter found them.
      s32 twobits;
      s32 leftside;
      ...
      leftside = ((unk5APlus3 & 1) << 7) + 0x21;
      prim->u1 = prim->u0 = leftside;
      ...
      twobits = unk5APlus3 & 2;
      prim->v1 = prim->v3 = ((twobits) << 6) + 0x59;
  #else
      prim->u1 = prim->u0 = ((unk5APlus3 & 1) << 7) + 0x21;
      prim->v1 = prim->v3 = ((unk5APlus3 & 2) << 6) + 0x59;
  #endif
  ```
  The `#else` branch proves the locals have zero semantic content: the same code without them is
  the natural spelling and matches on PSP. On PS1 (MIPS/GCC) they are required and are kept,
  annotated, permuter-derived.

### Verdict — **WEAK (leaning overturnable)**

The *genus* is clearly established in matched MIPS SOTN: fabricated, semantically-purposeless
staging locals and redundant assignments through reused live locals, permuter-found, `!FAKE`-annotated,
shipped. What is **not** exhibited is BB2's exact *species*: a literal hex constant routed through
a live local into a second local. F1's escalation asked the census for "stage a CONSTANT through a
reused live local"; SOTN stages *expressions and loaded values* through such locals, and stages
*constants* only as dead stores (`i = 0; // FAKE? why here?`), never as a copy chain into a second local.

Honest read: the refusal's stated basis — "no census-established SOTN precedent" for
purposeless staging locals as a class — is now factually too strong. The narrower basis — no
precedent for the constant→local→local copy chain specifically — survives.

**Affected parks:** `func_80061658`, `func_80061710` (both explicitly refused on this family).
Secondarily `func_800611A4` (same `text1b.c` cluster, same v0↔v1 local-alloc.c:472 mechanism;
its 2026-07-27 park cites the same wall).

---

## F2 — signedness-split dual read

### Definition (quoted, `docs/grind/decisions.md:1474`, 2026-07-23 escalation, func_8001F938)

> The distance-0 pure-C form is the signedness-split / redundant dual-typed-memory-read CSE-defeat
> family — one field at +0x270 read signed (`lh` → `slti` compare) AND unsigned (`lhu` → `sll` index),
> the two views provably identical in program behavior … Its sole effect is to hand GCC 2.7.2's
> combine pass a second typed memory view so it manufactures the 2nd load. … This family has NO SOTN
> precedent: the F2 SOTN-master-branch census (2026-07-01) returned NOT ESTABLISHED.

### Signatures searched

- Same lvalue cast to both `s16`/`u16` (and `s8`/`u8`, `short`/`unsigned short`) in one file.
- `*(s16*)X` and `*(u16*)X` on the same address expression.
- SOTN's own reinterpret macros: `LOH(x) = (*(s16*)&(x))` vs `LOHU(x) = (*(u16*)&(x))`
  (`include/common.h:81,83`) applied to the **same lvalue**.

### Hits

The macro machinery is first-class and heavily used: **`LOH` 382 uses, `LOHU` 54 uses, `LOW` 1778**.
Nine files use both `LOH` and `LOHU`. Same-lvalue pairs, 6 total:

- `src/st/rare/e_white_dragon.c` (INCLUDE_ASM 0) — `self->ext.whiteDragon.unk98` accessed as
  `LOH(...) = 0;` at :588 and as `LOHU(...) -= 0xA0;` / `LOHU(...) & 0x8000` at :328, :431-432,
  :536-537, :613-615, :698-699. **Genuine mixed-signedness access to one field inside one matched
  MIPS function.** But the signedness carries real meaning (`& 0x8000` needs the unsigned view; the
  `= 0` write is signedness-neutral) — these are not two reads of one value.
- `src/st/e_room_bg.h:17,19` and `src/st/no1/e_room_bg.c:58,60` — `LOHU(objInit->facingLeft)` vs
  `LOH(objInit->facingLeft)`, but selected by `#elif defined(VERSION_PSP)` / `#else`. A per-version
  type fix, not a dual read.
- `src/st/prim_helpers.h` — `LOH(prim->next->r2)` at :55-65 vs `LOHU(prim->next->r2)` at :215;
  different functions in the header.
- `src/main/psxsdk/libc/sprintf.c` — `(s16)num` and `(u16)num` present; imported PsyQ library code,
  and the two casts serve the `%d` vs `%u` conversions.

**Zero instances of the same value read twice under two signednesses where the two views are
provably equivalent.**

### Verdict — **WEAK (does not overturn)**

The 2026-07-01 F2 census result stands. What this sweep adds is that SOTN's *typed-view vocabulary*
is richer than "no precedent" suggests — signed and unsigned narrow views of one field coexist in
matched functions — but every instance is semantically load-bearing. The redundant, behaviorally-null
dual read that BB2 needs (a second typed memory view whose only effect is to make `combine`
manufacture a second load) is not exhibited anywhere.

**Affected park:** `func_8001F938` (13 regfix rules). Refusal confirmed.

---

## F3 — compound-address duplication across independent call arg-lists

### Definition (quoted, `docs/grind/decisions.md:895`, 2026-07-19 escalation, func_80057CC8)

> **(a) Sanction the inline-both-call-sites form** … adapted for the "duplicate compound address
> expression at each of two independent call arg-lists" shape …
> Honest cost: the inline-both-call-sites shape has **NO direct SOTN-master-branch precedent for
> compound-address-expression duplication across independent call arg-lists** (the 2026-07-01
> duplicated-statement-into-arms evidence covers per-branch-arm duplication within one control-flow
> diamond, not per-independent-call duplication)

The rejection reasoning (s8 vetting) was: "character-for-character duplication of a 30+ character
compound address expression at both call arg-lists … coercion signal from non-idiomatic
character-for-character expression duplication."

### Signature searched

Per-function partition; extract every call's top-level arguments; flag any argument that is a
*compound address expression* (contains `&`, `[]`, or `->` **and** arithmetic, length ≥ 15 chars)
appearing verbatim in **two or more independent call argument lists** in the same function
(`tmp/fam/f3.py`, 273 raw hits, filtered to compound-address forms).

### Hits — all in files with `INCLUDE_ASM = 0`

| Site | Expression | Call sites | Note |
|---|---|---|---|
| `src/dra/4DA70.c:273,304,317,352,365,414,427,487,497,549,559,592,606,644` | `&r->ot[prim->priority]` | **14× `addPrim`** in `RenderPrimitives` | no pointer local anywhere |
| `src/dra/4CE2C.c:154,255` | `&r->ot[i + r->order]` | 2× `AddPrim` in `RenderTilemap` | |
| `src/dra/4CE2C.c:139,241` | `r->order + page + r->ot` | 2× `addPrim`, same function | three-term address arithmetic, written out twice |
| `src/st/rno0/e_gorgon.c:1372,1379,1408,1422,1427,1452` | `&self->ext.gorgon.unk80` | 6× `StepTowards` | |
| `src/st/lib/e_spellbook_magic_tome.c:371,404,435` and `:669,702,733` | `&self->ext.spellbookMagicTome.unk82` | 3× `func_801CDC80`, in two separate functions | |
| `src/st/nz1/e_cloaked_knight.c:125,146,169` | `&self->ext.cloakedKnight.unk9E` | 3× `StepTowards` | |
| `src/dra/42398.c:750,753` | `&g_BackBuffer->disp` | 2× `PutDispEnv`, 3 lines apart | |

**Tightest analogue — two calls, same straight-line block, offset-bearing compound address**
(`src/st/st0/e_bg_vortex.c:246-256`, INCLUDE_ASM 0):

```c
gte_ldrgb(&scratchColor[i]);
...
    gte_ldv0(&D_801C1BC8[vectorIndex1] + 0x10);
    ...
        gte_ldrgb(&scratchColor[i] + 1);
        gte_dpcs();
        gte_strgb(&prim->r2);
        gte_ldv0(&D_801C1BC8[vectorIndex2] + 0x10);
        gte_rtps();
        gte_stsxy((long*)&prim->x3);
        gte_ldrgb(&scratchColor[i] + 1);      /* same expression, 6 lines later, same block */
```

`&scratchColor[i] + 1` — an index-plus-raw-offset compound address — is written character-for-character
at two `gte_ldrgb` argument positions in one straight-line block, with no pointer local, where the
natural human spelling hoists it.

### Verdict — **ESTABLISHED**

The refusal's stated factual premise ("NO direct SOTN-master-branch precedent for compound-address-expression
duplication across independent call arg-lists") is **refuted**. Writing a compound address expression out
at every call argument position, rather than binding it to a pointer local, is routine, high-volume,
committed SOTN style in fully-matched files — up to 14 repetitions of one expression in a single function,
and at least one instance of the same offset-bearing address written twice inside one straight-line block.

Caveat to weigh: none of these SOTN sites is annotated as match-motivated, so they are *idiomatic
style*, not *documented coercion*. That cuts in favour of sanction rather than against it — the
construct is what SOTN authors write naturally, which is exactly the "human-writable from spec"
test the s8 vetting said it failed.

**Affected park:** `func_80057CC8` (`text1b`, 7 regfix rules, owner-marked **re-attempt eligible**).

---

## F4 — cross-symbol arithmetic idiom

### Definition (quoted, `docs/grind/decisions.md:848`, 2026-07-19 escalation, cpu_side_move_dir_4 / now `CD_sync`)

> **(a) Sanction the cross-symbol arithmetic idiom** … Honest cost: this creates a NEW coercion family
> with no direct community precedent for the cross-symbol `idx_1495 = &D_800A1494[1]` initializer
> spelling; the mechanism is documented (s97 forensics: `expr.c::expand_expr` …) but the SPELLING is
> unique to csmd4 in this repo.

The construct: derive the address of symbol **B** by offsetting from a **different** symbol **A**.

### Signatures searched

- `&SYM ± <literal>` and `(u8*)&SYM + N` cast-and-offset forms.
- `&SYM_A - &SYM_B` / symbol-minus-symbol arithmetic.
- `&D_xxxxxxxx[N]` with nonzero literal index.
- Comments admitting adjacency/out-of-bounds/"beyond the array" reasoning.

### Hits

Every address-arithmetic instance found stays **inside a single symbol**:

- `src/dra/7879C.c:1949-1952`, `src/boss/bo4/unk_46E7C.c:1721-1724`, `src/boss/rbo5/unk_4648C.c:1696-1699`
  — the one `!FAKE`-annotated address-arithmetic hack in the tree:
  ```c
  // !FAKE, this should probably be &entity_ranges[unk9C] or similar,
  // instead of doing &entity_ranges followed by +=
  data = (u8*)&entity_ranges[0];
  data += self->ext.factory.kind * 2;
  ```
  `entity_ranges` is declared `static u8 entity_ranges[NUM_BLUEPRINT_KIND][2]` at `src/dra/7879C.c:1847`.
  The `+=` walks **within** that array. Base-plus-offset re-spelling of an in-bounds index — not cross-symbol.
- `src/st/mad/D8C8.c:151-153` — `LOHU(*((u8*)&tilemap[x] + 0x140 + rowOffset))`: raw byte offsets into
  the `tilemap` array, in bounds.
- `src/st/st0/e_bg_vortex.c:246,253` — `&D_801C1BC8[vectorIndexN] + 0x10`: within `D_801C1BC8`.
- `src/main/psxsdk/libc/sprintf.c:90` — `(char*)&args - sizeof(printf_info) - 4`: varargs frame walking
  in imported PsyQ library code, not a named-symbol crossing.

**Zero instances of deriving one named symbol's address from a different named symbol.**

Corroborating (from the 2026-08-10 memory, re-confirmed by this clone): SOTN's *resolution* for
adjacent per-word `D_` symbols is the **opposite** operation — merge them into a struct with
address-derived member names (`include/game.h` `Vram`, 25 `D_`-named struct members, merge PRs
#1175 / bd612229 / 88344c03). That is the same resolution BB2's own `gpu.c` `GpuCtx` work took
(`docs/grind/decisions.md:545`), and it forecloses the cross-symbol spelling rather than licensing it.

### Verdict — **ABSENT** — refusal confirmed, and independently reinforced by SOTN's struct-merge norm.

**Affected park:** `CD_sync` (`system`, 5 regfix rules, owner-marked **re-attempt eligible**).
The re-attempt lever indicated by this survey is not the cross-symbol idiom but a struct model over
the adjacent symbols, if the objects are genuinely one aggregate.

---

## F5 — union-constructor CLOBBER

### Definition (quoted, `docs/grind/decisions.md:761`, 2026-07-18 escalation, motion_SetMotion / now `func_80038C70`)

> may the union-constructor CLOBBER family — retyping a local as a single-member union and spelling one
> assignment as a GCC cast-to-union constructor (`union selu { s32 a; } sel; ... sel = (union selu)0xD;`)
> so that `expr.c:2996` emits a byte-free bare `(clobber (reg))` that stops jump2's `find_cross_jump`
> from merging the two `[li s0,13; j sel_dispatch]` blocks — be sanctioned …

### Signatures searched

Cast-to-union in expression position (`= (union X)`), single-member union declarations (brace-balanced
parse of every `union … { … }` in the tree), and the literal string `clobber` (case-insensitive).

### Hits

| Signature | Count on master @ 8bd7c77 |
|---|---|
| Cast-to-union `= (union X)…` in expression position | **0** |
| Single-member union declarations | **0** (25 union declarations total, all multi-member) |
| `clobber` (any case, `src/` + `include/`) | **0** |

This exactly reproduces the 2026-07-18 s7 survey result (which also covered rood-reverse and esa),
now re-verified against a master clone thirteen months of commits newer.

### Verdict — **ABSENT** — refusal confirmed on fresh evidence.

**Affected park:** `func_80038C70` (`code6cac_c_mid`, distance 1, 1 regfix rule — the sole
category-A "family refused" park).

---

## F6 — fabricated cancellation-pair increment split

### Definition (quoted, `docs/grind/decisions.md:1785`, 2026-07-27 17:57 Judge FAIL, func_80044098)

> The request asks me to sanction `v1++; v1--;` (an in-loop cancellation pair after the real `v1++`)
> as a new `/* FAKE */` family. … net +1 spelled as +1,+1,-1 … It has zero semantic purpose, no human
> would write it from spec, and its sole documented function is GCC-steering …
> **No file+function citation of an adjacent same-variable increment/decrement cancellation statement
> pair in matched SOTN/VS/ESA code was produced**, so gate (ii) fails by the request's own admission.

### Signature searched

Adjacent (within 2 lines) `X++;` / `X--;` on the **same** lvalue with opposite operators, both prefix
and postfix (`tmp/fam/sweep.py`). 41 raw hits; all contexts inspected manually.

### Hits

40 of 41 are **noise** — `if (cond) x++; else x--;` arm pairs (e.g. `src/boss/rbo0/e_boss_doors.c:103`,
`src/st/e_armor_lord.h:594`, `src/main/psxsdk/libsnd/tempo.c:13`). One is a real hit:

**`src/saturn/game_3b.c:1442-1455`** — `DecrementStatBuffTimers`, file `INCLUDE_ASM = 0`:

```c
void DecrementStatBuffTimers(void) {
    s32 i;
    for (i = 0; i < 16; i++) {
        if (!g_StatBuffTimers[i]) {
            continue;
        }
        switch ((u32)i) {
        default:
            // !FAKE
            i++;
            i--;
        case 0:
        ...
```

The **exact** construct: an adjacent same-variable increment/decrement cancellation pair, no semantic
purpose, `!FAKE`-annotated, in a decompiled C body of a byte-checked target (`config/check.saturn.sha`).

**Caveat:** `src/saturn/**` is the Saturn build — SH-2, not MIPS/GCC 2.7.2.

**MIPS-side corroboration — the same function on PSX carries an equally fabricated construct:**
`src/dra/5D5BC.c:761-775` (`INCLUDE_ASM = 0`):

```c
        switch (i) {
        default:
        case 0:
            // !FAKE, permuter found it. Whatever I guess. Matches on all
            // versions.
            if (!i) {
            }
        case 1:
```

An empty `if` with an empty body: zero semantic purpose, permuter-derived, kept because it matches.
Third instance of the same genus, also MIPS: `src/st/st0/cutscene.c:203` — `if (prim && prim) { // !FAKE }`
(a fabricated redundant duplicate condition with an empty body).

### Verdict — **ESTABLISHED**

Gate (ii) as the Judge framed it — "a file+function citation of an adjacent same-variable
increment/decrement cancellation statement pair in matched code" — is now satisfiable:
`src/saturn/game_3b.c:1450-1452`, `DecrementStatBuffTimers`. The owner must weigh the SH-2 caveat.
If the citation is required to be MIPS, the exact spelling is not available, but the *class*
(semantically-null fabricated statements inserted solely for codegen, `!FAKE`-annotated,
permuter-found, shipped in matched MIPS code) is established three times over — including in the
literal MIPS sibling of the same function.

**Affected park:** `func_80044098` (`text1a_c`, distance 13, 0 rules, terminal park under the
2026-07-27 standing ruling, marked re-attempt eligible).

---

## F7 — store-scheduling duplication beyond `duplicated-statement-into-arms` scope

### Definition (quoted, `docs/grind/decisions.md:1624`, 2026-07-24 16:38 Judge FAIL, func_80072CD4)

> The construct … duplicates the two UNCONDITIONAL common-tail stores `*(arg1+4)=fc_const` and
> `*(arg1+0xC)=fc_const` into both inner if/else arms; jump2 cross-jump re-merges the copies
> byte-neutrally … the ONLY surviving effect is steering the list-scheduler so @4/@0xC emit before @0xE.
>
> Here @4/@0xC are unconditional common code written on both paths regardless of the inner branch; the
> natural human spelling places them once in the merge block … **Hoisting unconditional common stores UP
> into both arms solely to reorder emission is a materially different construct** with a different GCC
> mechanism (sched2 launch/deferral, not `flow.c` `reg_n_refs`) **and no SOTN-master precedent has been
> presented.**

### Signatures searched

1. `if/else` pairs where a **textually identical store statement** appears in both arms (`tmp/fam/f7.py`).
2. Stricter: `if/else` pairs whose arms are **textually identical in full** — i.e. the branch is
   semantically vacuous and both arms are unconditional common code (`tmp/fam/f7b.py`).

### Hits

Signature 1: **1,275 sites**. Signature 2 (fully identical arms): **25 sites**, all verified by hand
against `#ifdef` false positives. Selected exhibits, all `INCLUDE_ASM = 0`:

- **`src/ric/pl_blueprints.c:1996`** · **`src/maria/pl_blueprints.c:1720`** ·
  **`src/boss/bo6/us_39144.c:914,927`** · `src/st/cen_psp/e_maria_blueprints.c:1718` —
  four independent overlays carry the same shape:
  ```c
  case 6:
      if (self->facingLeft) {
          prim->x0 = prim->x1 = selfX;
          prim->u0 = prim->u1 = xMargin;
      } else {
          prim->x0 = prim->x1 = selfX;
          prim->u0 = prim->u1 = xMargin;
      }
      prim->y0 = selfY + height;
      ...
  ```
  Two unconditional common stores written into both arms of a real branch. The natural human spelling
  is the two stores once, outside the `if`. In the *same* `switch`, `case 0` duplicates 3 of 4 stores
  (`x0`, `u0`, `u1` identical; only `x1` differs) — partial hoisting into arms.
- **`src/st/rnz0/e_lesser_demon.c:728`** (and `src/st/lib/e_lesser_demon.c:720`) — **six** identical
  store statements in both arms:
  ```c
      prim->r0 = RainbowRGBs[prim->v0 - 1][0] + 0x20;
      ... (6 stores) ...
  } else {
      prim->r0 = RainbowRGBs[prim->v0 - 1][0] + 0x20;
      ... (identical 6 stores) ...
  }
  prim->r1 = prim->r0;
  ```
  followed by a merge block that consumes them — the exact "unconditional common code duplicated into
  arms, merge block below" topology.
- **`src/st/e_plate_lord.h:904`** — three identical statements in both arms
  (`self->step_s = 3; part = self + 5; part->step = 0x12;`).
- **`src/weapon/w_020.c:157`** — self-annotated: `// Useless if-statement`, both arms
  `g_api.PlaySfx(SFX_KARMA_COIN_JINGLE);`.
- **`src/st/st0/2DAC8.c:1095`** — self-annotated:
  ```c
  // Bug? Code is the same in both paths
  // Usually the +/- are flipped for else
  if (!self->facingLeft) {
      prim->x0 = prim->x2 = self->posX.i.hi - 0x40;
      prim->x1 = prim->x3 = self->posX.i.hi + 0x40;
  } else {
      prim->x0 = prim->x2 = self->posX.i.hi - 0x40;
      prim->x1 = prim->x3 = self->posX.i.hi + 0x40;
  }
  ```
- Further: `src/boss/bo4/unk_46E7C.c:5489`, `src/st/collision.h:655`, `src/st/mad/collision.c:537`,
  `src/st/cat/e_hellfire_beast.c:564`, `src/st/no0/42A34.c:145`, `src/st/nz1/e_vandal_sword.c:98`,
  `src/main/psxsdk/libc/sprintf.c:278`, `src/st/e_bone_archer.h:293`, `src/st/sel_psp/7D40.c:278`,
  `src/ric/pl_whip.c:272`.

### Verdict — **ESTABLISHED (construct); the mechanism-scope objection survives)**

The refusal's factual premise — "no SOTN-master precedent has been presented" for unconditional common
stores duplicated into both arms — is **refuted**. Matched SOTN ships 25 fully-identical-arm `if/else`
constructs and 1,275 identical-store-in-both-arms sites, several with SOTN's own annotations conceding
the branch is useless.

The part of the ruling that this survey does **not** overturn is the mechanism-scope argument: the
`duplicated-statement-into-arms` rule is evidenced for a `reg_n_refs` RA-priority lift, and BB2's F7
effect is a `sched2` store-emission-order lift. But note that the BB2 rule itself already records
(`.claude/rules/duplicated-statement-into-arms.md:31-34`) that "whether cross-jump merges them is
invisible to the author; both merged and unmerged instances ship" — SOTN authors have no view of which
GCC pass their duplication feeds, so requiring the *mechanism* to be independently SOTN-evidenced is a
bar no SOTN citation can ever clear. The evidenceable question is the construct, and the construct is
established.

Directionality caveat (per the 2026-08-10 memory): SOTN's instances read as *preserving* a redundancy
the authors believe the original code had ("Bug? Code is the same in both paths"), whereas BB2's F7
*hoists* stores that are genuinely common. Both produce a C form whose only justification is the bytes,
but the framing differs and the owner may weigh it.

**Affected park:** `func_80072CD4` (`text1b`, distance 12, 9 regfix rules, clean floor-4 candidate
retained on main).

---

## Appendix A — the complete `!FAKE` catalog on master @ 8bd7c77 (12 sites)

| Site | Construct | Bears on |
|---|---|---|
| `src/boss/bo4/unk_46E7C.c:117` · `src/boss/rbo5/unk_4648C.c:114` | `temp = 0; // TODO: !FAKE` then compared | F1 |
| `src/boss/bo4/unk_46E7C.c:1721` · `src/boss/rbo5/unk_4648C.c:1696` · `src/dra/7879C.c:1949` | `data = (u8*)&entity_ranges[0]; data += kind * 2;` — in-bounds base+offset re-spelling | F4 (negative) |
| `src/dra/5087C.c:559` | "Ugly casts" compound address for `D_801375BC.def` | F3 |
| `src/dra/5D5BC.c:770` | `if (!i) { }` — empty if, "permuter found it" | F6 |
| `src/saturn/game_3b.c:1450` | `i++; i--;` cancellation pair | **F6 (direct)** |
| `src/st/e_fire_warg.h:1232` | two fabricated staging locals, PS1-only, permuter-found | **F1 (closest)** |
| `src/st/no0/e_stone_rose.c:609` | `Entity* fakeEntity = self;` — pointer-alias local | — |
| `src/st/sel/2C048.c:564` | `volatile u32 pad; // !FAKE:` — **phantom frame-slot carrier** | not one of the 7; see below |
| `src/st/st0/cutscene.c:203` | `if (prim && prim) { }` — fabricated redundant condition | F6 |
| `src/dra/menu.c:1993,2009,2017` | `j = menu->unk1D; // FAKE?` ×3 (marked `FAKE?`, not `!FAKE`) | F1 |
| `src/weapon/w_049.c:293` | `i = 0; // FAKE? why here?` | F1 |

### Off-brief but load-bearing: `src/st/sel/2C048.c:564`

```c
    volatile u32 pad; // !FAKE:
```

An unused `volatile` local declared solely to reserve a stack slot, in a file with `INCLUDE_ASM = 0`.
This is a **phantom-frame-slot carrier** — the construct behind six BB2 parks that were refused for
lack of SOTN precedent: `func_80047FBC` (+32B `buf[8]`), `func_80047EE8` (+32B sibling),
`func_80049A2C` (+8B `dummy[2]`), `func_80017FA0` (8-byte zero-store leaf frame), `file_LoadSectors`
(`_pad[2]`), and adjacently `func_80022F34`. It also sits against the 2026-08-10 finding that
"volatile-in-one-TU is essentially absent" in SOTN — this is a third instance, and unlike the other two
it is neither imported PsyQ code nor semantically motivated.

I did not survey that family (out of brief) and make no verdict on it, but it is the single highest-value
follow-up this sweep turned up: one `!FAKE`-annotated exhibit that speaks directly to six parked functions.

---

## Verdict summary

| # | Family | Verdict | Affected parks |
|---|---|---|---|
| F1 | constant-staging-through-reused-live-local | **WEAK** (genus established, species not) | `func_80061658`, `func_80061710`, (`func_800611A4`) |
| F2 | signedness-split dual read | **WEAK** (no redundant dual read) | `func_8001F938` |
| F3 | compound-address duplication at call arg-lists | **ESTABLISHED** | `func_80057CC8` |
| F4 | cross-symbol arithmetic idiom | **ABSENT** | `CD_sync` |
| F5 | union-constructor CLOBBER | **ABSENT** | `func_80038C70` |
| F6 | fabricated cancellation-pair increment split | **ESTABLISHED** (SH-2 exhibit; MIPS class-siblings) | `func_80044098` |
| F7 | store-scheduling duplication into arms | **ESTABLISHED** (construct); mechanism-scope objection survives | `func_80072CD4` |
