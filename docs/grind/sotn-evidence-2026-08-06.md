# SOTN evidence passes — 2026-08-06

Two owner-directed evidence passes run to the 2026-06-02 borderline-research standard:
direct master-branch SOTN evidence, file:line citations, quoted code, negative results
reported as valid outcomes.

## Corpus and method

- **Snapshot:** `tmp/sotn-decomp`, a **shallow (depth-1) sparse clone** of
  `github.com/Xeeynamo/sotn-decomp` at master commit **`aa53500`**
  ("Decompile and codeshare `rcen` `cutscene` (#3319)").
  The working tree was sparse (`src/main/psxsdk` + `include/psxsdk` only, 175 `.c`),
  but the object store holds the full tree, so the complete `src/` + `include/` was
  extracted read-only via `git archive HEAD src include` into a scratchpad
  (**1699 `.c` + 401 `.h`**). The sparse checkout in `tmp/` was left as found.
- **Coverage after excluding `src/pc/3rd/` (vendored cJSON, not decompiled PSX code):
  1936 `.c`/`.h` files.**
- **History is NOT available** (depth-1, `.git/shallow` present) — so PR-message
  corroboration was done over the network instead of by `git log`.
- Version-provenance discipline: hits are labelled PSX (`src/dra`, `src/st`, `src/boss`,
  `src/main/psxsdk`, `src/weapon`, `src/ric`, `src/maria`, `src/servant`) vs
  PSP-only (`src/*_psp`, `src/main_psp`, `#ifdef VERSION_PSP` regions) vs Saturn
  (`src/saturn`, different compiler — excluded from load-bearing claims).

### Project-level convention (sets the bar for both questions)

`docs/STYLE.md:152` (fetched from master):

> - A fake should be commented with an `// !FAKE:` above or to the side of the code that you think it's fake.

This is the ONLY match-hack convention in SOTN's style guide; it contains no guidance
about duplication, statement ordering, or store placement. Because SOTN's own rule is
that match-motivated code carries a `!FAKE` annotation, **an exhaustive census of the
annotation is the primary instrument for both questions.** That census found **117
FAKE/match-flavoured comments** across `src/` + `include/`; every one was read or
classified.

---

# Q1 — Does the duplicated-statement-into-arms family base include CONTROL-TRANSFER duplication?

## Q1.a — The two cited base files

**`src/boss/bo4/unk_365FC.c` (`EntityUnkId1B`, 186 lines, PSX):** all cross-arm
duplication is assignments/stores/calls. The duplicated multi-statement blocks are
`g_Player.padSim = …; g_Player.demo_timer = …;` (case 2 at :104-105 vs case 3 at
:121-122) and the tilemap-index computation (case 3 at :139-146 vs case 5 at :158-165).
The only control transfers are the ordinary per-case `break;` (5 sites). **No duplicated
goto/return/continue.**

**`src/boss/bo4/doors.c` (794 lines, PSX): the premise "all assignments/stores/calls"
is NOT accurate for this file.** It duplicates a two-statement block ending in a
`return` into **three** arms:

`src/boss/bo4/doors.c:222-224` (`EntityUnkId17` case 0), `:437-439`
(`EntityUnkId19` case 2), `:678-680` (`EntityUnkId1A` case 2) — each identical:

```c
        } else {
            DestroyEntity(self);
            return;
        }
```

plus `DestroyEntity(self); break;` duplicated at `:328` and `:790`. These are three
separate functions in one file, so they are cross-function repetition of an idiom rather
than sibling arms of one switch; the `break;` pair at `:328`/`:790` likewise. Within a
single function the duplicated blocks are all assignment/call blocks (e.g. `EntityUnkId17`
cases 4 and 5 duplicate the whole `magnitude`/`xOffset`/`yOffset`/`prim`/`posX`/`posY`
recomputation at `:271-276` vs `:309-314`, and the `AnimateEntity` if/else at `:277-281`
vs `:315-319`).

**The rule doc's other cited counts check out:** `src/boss/bo4/doppleganger.c` has
`DOPPLEGANGER.animSet = ANIMSET_OVL(1);` **8×** and `D_us_801D3D24 = THINK_STEP_0;`
**12×** (doc says 7 and 11 arms; the extra occurrence in each is a non-arm site).

## Q1.b — Wider-tree sweep: control-transfer duplication DOES exist

A structural scan of all 1936 files for **repeated (≥2 sites) multi-statement blocks
whose last statement is a control transfer** returned **3906 blocks**, of which **37 are
`goto`-terminated**. Classifying every `goto`-terminated one by target direction:

**Backward gotos (loop continuation / state-machine re-dispatch) duplicated into arms — 5 distinct sites:**

1. **`src/main/psxsdk/libc/sprintf.c:99-111` (PSX, matched libc).** The two-statement
   block `ch = *++f; goto loop_30;` is written into **three sibling switch arms**:

```c
    loop_30:
        switch (ch) {
        case 'h':
            info.isHalf = true;
            ch = *++f;
            goto loop_30;

        case 'l':
            info.isLong = true;
            ch = *++f;
            goto loop_30;

        case 'L':
            info.isLongLong = true;
            ch = *++f;
            goto loop_30;
```

   This is the closest thing in the tree to the BB2 construct: an assignment plus a
   **backward** control transfer, duplicated across arms. Note it is UNCONDITIONAL and
   targets a **shared** label. Unannotated. (The same file carries an unrelated sanctioned
   construct at `:129`, `} while (0); // FAKE`.)

2. **`src/st/lib/e_shop.c:986-1009` (`func_us_801B0FBC`, PSX).** `goto loop;` duplicated
   into two arms, each preceded by an arm-specific counter bump:

```c
loop:
    ch = *str++;
    if (ch) {
        if (ch == ' ') {
            x++;
            goto loop;
        }
        rect.x = ((ch & 0x0F) << 1) + 0x380;
        ...
        MoveImage(&rect, x, y);
        x += 2;
        goto loop;
    }
```

   (PSP twin at `src/st/lib_psp/e_shop.c:5113-5130`, plus a third site at `:6570`.)

3. **`src/dra/menu.c:3609` + `:3680` + `:3701`** — `g_MenuStep = MENU_STEP_EXIT_BEGIN;
   goto block_4;` ×3, where `block_4:` (`:3377`) is the head of the state-machine switch
   (backward re-dispatch). **Only `:3609` is compiled on PSX**; `:3680`/`:3701` are inside
   `#if defined(VERSION_PSP)` — so on PSX this is a single site, not duplication.
   (`src/dra_psp/menu.c:3745/3819/3840` is the PSP twin.)

4. **`src/dra/menu.c:4138` + `:4167`** — `goto block_191;` in two sibling `else if (i == 1)`
   arms, targeting a shared backward label at `:4091`.

5. **`src/main/main.c:64` + `:75`** — two `goto main_search_loop_2;` sites, but they are two
   DIFFERENT retry conditions (`!CdSearchFile(...)` and `main_fd < 0`), not a duplicated block.

**Forward gotos (shared error/exit label) duplicated into arms — routine and abundant**,
e.g. `src/dra/6E42C.c:614-682` (`g_CurrentEntity->velocityX = 0; PlaySfx(SFX_VO_ALU_ATTACK_B);
goto block_98;` at **7 sites**, and a 5-statement version ×3), `src/dra/692E8.c:884/899`
and `:1035/1054`, `src/dra/7E4BC.c:1745/1757`, `src/boss/bo4/unk_46E7C.c:4451/4463`,
`src/main/psxsdk/libcd/iso9660.c:54/59`, `src/ric/pl_main.c:567/570`,
`src/maria/pl_main.c:608/611`.

**`return`-terminated tails duplicated into arms, coexisting with a shared-label `goto`
in the same function — `src/main/psxsdk/libsnd/vs_vh.c` (PSX, matched libsnd,
`SsVabOpenHeadWithMode`).** The four-statement error tail is written out verbatim at
`:69-72`, `:116-121` and `:124-128`, while a fourth arm jumps to a label placed inside
one of those inline copies:

```c
        if ((spuAllocMem + var_s0) > 0x80000U) {
        end:
            _svm_vab_used[vabId_2] = 0;

            _spu_setInTransfer(0);
            _svm_vab_count -= 1;
            return -1;
        }
        ...
    } else {
        goto end;
    }
```

This is the single strongest base instance for "an inline tail copy and a jump-to-shared-tail
legitimately coexist in one matched function" — and it is also the already-sanctioned
mixed-exit-forms precedent this project cites.

## Q1.c — The specific BB2 construct: NOT present

The BB2 `func_80021280` construct is: replace `goto next_21280;` (a jump to a shared tail)
with an inline COPY of the loop tail `a1++; if (a1 < 3) goto loop2_21280; return;` inside
the `if (a0 == 0)` arm.

Searches for that shape returned **zero** hits:

- **Zero** single-line `if (…) goto L;` anywhere in the tree.
- **Zero** conditional backward gotos of any spelling. A scan for every backward goto
  preceded within 5 lines by a counter bump or guarded by an `if` returned **5 hits, all
  in `e_shop.c`/`e_shop_psp.c`, all UNCONDITIONAL** (`guarded=False` in every case).
- **Zero** instances of an inline loop-tail copy replacing a jump to a shared tail. In
  **every** SOTN instance found, the duplicated control transfer is either (a) a jump to a
  **shared label** — i.e. the label-sharing spelling, the very thing BB2 proposes to
  replace — or (b) a `return` ending an error tail.
- **Zero** FAKE/match annotations attached to any duplicated control transfer anywhere in
  the 117-instance census. The nearest FAKE-annotated control constructs are an empty
  `if (!i) { }` inside a switch arm (`src/dra/5D5BC.c:766-771`,
  `// !FAKE, permuter found it. Whatever I guess. Matches on all versions.`) and an empty
  `if (prim && prim) { // !FAKE }` (`src/st/st0/cutscene.c:203`) — both are INSERTED dead
  conditionals, not duplicated transfers.

## Q1 VERDICT

The question has two scopes and they resolve differently. Stating both rather than
blurring them:

- **Scope 1 — "are the base instances all assignments/stores/calls?" → EVIDENCE-FOUND
  (premise corrected).** Matched SOTN routinely duplicates multi-statement blocks that
  TERMINATE in a control transfer into control-flow arms, including one of the two cited
  base files (`src/boss/bo4/doors.c:222-224`, `:437-439`, `:678-680`). The strongest
  citations are `src/main/psxsdk/libc/sprintf.c:99-111` (assignment + **backward** goto
  across 3 sibling switch arms), `src/st/lib/e_shop.c:986-1009` (counter bump + backward
  goto across 2 arms), and `src/main/psxsdk/libsnd/vs_vh.c:69-128` (4-statement
  `return`-terminated tail written out 3× while a 4th arm uses `goto end`).

- **Scope 2 — the exact BB2 construct (inline copy of a CONDITIONAL loop tail replacing a
  jump to a shared tail) → NO-EVIDENCE.** Search space covered: all 1936 non-vendored
  files; all 218 `goto` statements; all 14 backward-goto labels with ≥2 sites; all 37
  duplicated goto-terminated blocks; all 3906 duplicated control-transfer-terminated
  blocks; all 117 FAKE annotations. Every SOTN duplicated transfer jumps to a shared label
  or returns; none is a conditional backward branch, and none is annotated as
  match-motivated.

The gap between the two scopes is precisely what the layer-2 reviewer flagged: SOTN
duplicates the *jump*, BB2 proposes to duplicate the *tail body including its conditional
branch*. Whether that gap is a difference in kind or in degree is the owner's call, not
a fact the corpus settles.

---

# Q2 — Does matched SOTN RELOCATE a real, semantically-required store for codegen effect?

Target signature (from `memory/wip/hirahira_w_ctrl_2/notes.md`): a real store
(`a1[2] = sinB;`) moved from its natural tail position to early in the function so that it
acts as a memory barrier, stopping `combine.c` folding `lhu + sll 16 + sra 16` into `lh`.

## Q2.a — Annotation census: no store-position hits

All 117 FAKE/match comments were enumerated and every position-plausible one read. None
attaches to the POSITION of a real store. Classification of the position-adjacent ones:

| Site | Construct | Class |
|---|---|---|
| `src/weapon/w_049.c:293` `i = 0; // FAKE? why here?` | **dead** store to a local, immediately re-assigned by the following `for (i = 0; …)` | dead-store family, not relocation |
| `src/st/st_common.h:248` `y = i = …; // FAKE, no reason to set i.` | dead store fused into a chained assign | dead-store family |
| `src/dra/66590.c:390` `dest = val1; // fake` (PSP twin `src/dra_psp/62FE0.c:137`) | dead store | dead-store family |
| `src/boss/rbo0/e_coffin.c:255` `MATRIX* spDummyMatrix; // used only for dead store` | declaration for a dead store | dead-store family |
| `src/main/psxsdk/libcd/cdread.c:84` `mode = mode2; // FAKE` | value split across two locals (`&mode` is then passed to `CdControl`) | variable-reuse family |
| `src/dra/menu.c:1993`, `:2009`, `:2017` `j = menu->unk1D; // FAKE?` | redundant duplicated reads into `j` | split-read family (duplication, not position) |
| `src/dra/42398.c:299-312` `color_fake = *palette; … color_fake = color;` | repeated rebinds of one local | split-read / variable-reuse family |
| `src/ric/2F8E8.c:876` `// Weird fake stuff to load unk7E a second time` | duplicated **load** | split-read family |
| `src/st/e_fire_warg.h:1231-1243` `// These are both !FAKE; psp matches without them. ps1 needs them. I dunno. permuter found them.` | two named intermediate locals (`twobits`, `leftside`) | named-intermediate family |
| `src/st/nz1/e_boss_doors.c:234` `offsetY = 208 - offsetY; // critical, all in one line breaks regalloc` | expression split across statements | expression-shape family |
| `src/dra/71830.c:171-174` `// … the compiler comes out wrong if we don't split it into two` | `(x + 16) + y - 8` instead of `+8` | expression-shape family |
| `src/weapon/w_037.c:300` `// FAKE but makes register allocation work` | variable reuse | variable-reuse family |

**Every match-motivated memory construct in the corpus is duplication, insertion, naming,
typing, or expression shape. None is repositioning.**

## Q2.b — Structural scans: no discriminating hit

- **Indexed-array scan** (a store to `base[k]` whose index falls inside a sibling
  cluster's index range but is textually separated): **12 hits tree-wide**, all benign —
  separate program paths (`src/dra/menu.c:2711` vs `:3250` are 539 lines and several
  functions apart) or genuine algorithmic nibble-swap ordering
  (`src/st/st0/cutscene.c:762` etc., under the comment `// Swap upper and lower`).
  **Zero** hits where a store is moved among its own siblings.
- **Field-store scan** (`base->f =` isolated ≥4 statements from a ≥4-store sibling
  cluster): **3186 hits**, i.e. the signature is not discriminating — ordinary
  initialize-then-compute-then-finish code produces it. No hit is annotated, and spot
  checks (`src/dra/42398.c:309`, `src/dra/5F60C.c:469`, `src/dra/84B88.c:625`) are all
  ordinary program structure. Reported for auditability, not as evidence either way.
- The one genuine store-ORDER oddity found, `src/dra/42398.c:280-288`, is
  `sprite->x0 = 112; sprite->u0 = …; sprite->y0 = 80; sprite->v0 = …;` — x0/y0 split by
  the u0 store where a natural writer batches x/y then u/v. But these are **independent
  stores to distinct fields of one struct**, so this is source-order choice among
  siblings, not relocation of a store past unrelated computation. Unannotated.

## Q2.c — What SOTN does INSTEAD for the exact `lh`-vs-`lhu` problem

This is the substantive positive finding of the pass. SOTN faces BB2's precise
sign-extension-fold problem and solves it at the **type level**, never by statement
position:

**1. Cast macros at the use site** — `include/common.h:69-74`:

```c
#define LOH(x) (*(s16*)&(x))
#define HIH(x) (((s16*)&(x))[1])
#define LOHU(x) (*(u16*)&(x))
#define LOW(x) (*(s32*)&(x))
#define LOWU(x) (*(u32*)&(x))
```

Usage counts across `src/` + `include/`: **`LOH` 330, `LOHU` 40, `LOW` 1002, `LOWU` 18.**
`LOH` forces `lh`, `LOHU` forces `lhu` — selected per use site by type punning.

**2. Header type correction driven by the observed load form** — `include/stage.h:86-95`:

```c
// Used in e_blade, e_gurkha, no2/hammer
// note on member types: length and zOffset definitely look signed
// in the data, but psp uses lhu instead of lh.
typedef struct {
    s16 eArrayOffset;
    s16 eArrayParentOffset;
    u16 length;
    u16 params;
    u16 zOffset;
} giantBroBodyPartsInit;
```

SOTN declared members `u16` *against* the apparent data semantics because the emitted
load form said so. This is exactly the untried axis #3 in
`memory/wip/hirahira_w_ctrl_2/notes.md` (`extern s16 Judge[]` → `u16`).

**3. When an extra memory ACCESS is genuinely needed at a position, SOTN inserts a bare
dead read — it does not move a real store.** `src/st/no1/e_room_bg.c:55-61`:

```c
#ifdef VERSION_PSP
        objInit->facingLeft; // For match on PSP
        self->unk5A = LOHU(objInit->facingLeft);
#else
        self->unk5A = LOH(objInit->facingLeft);
#endif
```

Caveats the owner should weigh: this is **PSP-only** (`#ifdef VERSION_PSP`, a different
compiler from the PSX GCC 2.7.2 BB2 uses), it is an **inserted dead read** rather than a
relocated real store, and it is annotated. It is the nearest analogue in the corpus to
"an extra memory operation placed at a chosen point to shape a load form" — and it still
differs from the BB2 construct on the axis the reviewers objected to.

## Q2 VERDICT

**NO-EVIDENCE.**

Search space covered: all 1936 non-vendored `.c`/`.h` files; the complete 117-instance
FAKE/match-annotation census (every position-plausible instance read in full); dedicated
greps for ordering/placement/movement vocabulary and for compiler-mechanism vocabulary
(`barrier`, `CSE`, `hoist`, `sign-ext`, `lh`/`lhu`/`sll`/`sra`, `combine`, `optimi[sz]`,
`codegen`, `register alloc`); two structural scans for out-of-place stores (indexed-array
and struct-field); and the project style guide, which documents no ordering or placement
convention beyond the `// !FAKE:` annotation.

**Matched SOTN does not relocate a real, semantically-required store for codegen effect,
and nowhere states that a store's position is load-bearing.** Where SOTN needs the exact
codegen effect BB2 is chasing, it uses a **type-level** lever (`LOH`/`LOHU`, 370 uses; the
`stage.h` member-type correction), and where it needs an extra memory access at a point it
**inserts a dead read** rather than moving a live store.

**What would settle it the other way:** a matched PSX function in which a store whose
value is genuinely required later appears textually before code that does not depend on
it, with a comment or PR discussion identifying the position as load-bearing. Nothing of
that shape exists at commit `aa53500`. The one avenue this pass could NOT check is PR
review discussion: the local clone is depth-1, so per-commit messages and PR threads were
unavailable, and only the style guide could be corroborated over the network. If the owner
wants that avenue closed, a full-history clone plus a `git log -S`/PR-thread search over
`LOHU`, `!FAKE`, and the `e_room_bg.c` change would do it.

---

## Appendix — every search run (auditable coverage list)

Corpus prep:
1. `git log --oneline -3`, `git remote -v`, `git config core.sparseCheckout`, `git rev-list --count HEAD`, `ls .git/shallow` — established commit `aa53500`, shallow depth-1, sparse tree.
2. `git ls-tree -d HEAD src/`, `git ls-tree HEAD src/boss/bo4/`, `git cat-file -s` — confirmed full object store.
3. `git archive HEAD src include | tar -x -C <scratchpad>` — 1699 `.c`, 401 `.h`.

Q1:
4. `grep -rn '\bgoto\b' --include='*.c' src/` — 218 statements in 38 files.
5. `grep -rn 'goto +\w+' -o` piped to a per-file/per-label histogram — label frequency table.
6. `scan.py` — every label with ≥2 **backward** goto sites (14 results; 8 in vendored cJSON).
7. `scan2.py` — every repeated (≥2-site) 2-to-5-statement block ending in `goto`/`return`/`break`/`continue`, whole tree minus `src/pc/3rd/` (3906 blocks; 37 goto-terminated, all listed and classified by target direction).
8. `scan3.py` — every backward goto preceded within 5 lines by a counter bump or guarded by an `if` (5 hits, all unconditional, all `e_shop.c`).
9. `grep -rnE 'if\s*\(.*\)\s*goto\s+\w+;'` — zero single-line conditional gotos.
10. Full reads: `src/boss/bo4/unk_365FC.c` (all 186 lines), `src/boss/bo4/doors.c` (all 794 lines), `src/main/psxsdk/libsnd/vs_vh.c` (`SsVabOpenHeadWithMode`, :19-145), plus targeted reads of `sprintf.c:80-125`, `main.c:45-90`, `e_shop.c:960-1030`, `dra/menu.c:3360-3395`, `:3595-3715`, `:4085-4175`, `dra/5D5BC.c:735-800`, `st0/cutscene.c:195-212`.
11. `grep -c` verification of the rule doc's doppleganger.c arm counts (8 and 12 raw occurrences).

Q2:
12. `grep -rniE '(//|/\*).*\b(fake|for match|to match|no match|match hack|matching hack|moved for|store here)\b'` over `src/` + `include/` — the full 117-instance census (all listed in-session, all position-plausible ones read).
13. `grep -rniE '…(order|ordering|reorder|position|placement|placed|moved|earlier|out of order|swapped|swap|assigned here|set here|store)…'` with false-positive filtering (SPDX, z-order, ordering tables, draw order, screen position).
14. `grep -rniE '…(barrier|CSE|hoist|sign-ext|lhu|lh|sll|sra|combine|optimi[sz]|codegen|compiler|regalloc|register alloc|allocation)…'`.
15. `scan4.py` — indexed-array out-of-place store scan (12 hits, all classified).
16. `scan5.py` — struct-field isolated-store scan (3186 hits; signature non-discriminating, spot-checked).
17. `grep -rn '#define (LOH|LOHU|LOW|LOWU|HIH)'` + per-macro usage counts.
18. Targeted reads: `w_049.c:280-300`, `ric/2F8E8.c:866-890`, `libcd/cdread.c:74-95`, `dra/menu.c:1975-2025`, `no1/e_room_bg.c:45-65`, `e_fire_warg.h:1222-1248`, `st_common.h:240-275`, `e_stage_name_jp.h:60-80`, `stage.h:78-95`, `no3/e_warg.c:420-450`, `dra/71830.c:160-190`, `rbo0/e_coffin.c:245-275`, `dra/42398.c:258-315`, `ric/pl_setstep.c:35-60`.

Network:
19. `curl` of `docs/STYLE.md` at master — the `// !FAKE:` convention (`:152`); no ordering/duplication/placement guidance.
20. WebFetch of the wiki matching-tips page — **page does not exist** (returned the "create new page" form); WebSearch over `github.com` surfaced only the repo README/STYLE.md, no technique discussion. PR-thread search was NOT possible (shallow clone, no history).
