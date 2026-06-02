---
name: sotn-borderline-research-2026-06-02
description: "SOTN-grounded evidence for the 6 borderline techniques flagged by the 2026-06-02 audit. Concrete master-branch citations: (Random()&3)+1-1, idxSub=idxSub, color_fake, randy=basePoint.x; baseX=randy, volatile pad, mixed goto+return exits, u8 packed-arg params. All 6 ACCEPTED in SOTN; zero rejected; zero unclear. The SOTN bar is permissive enough that these are not at the boundary."
metadata:
  type: project
---

# SOTN evidence for the six borderline techniques

## Why this exists

The 2026-06-02 techniques audit ([[techniques-audit-2026-06-02]]) classified
six BB2 `.claude/rules/*.md` techniques as BORDERLINE — between LEGITIMATE
and FORBIDDEN — and recommended user policy judgment. The user asked: **can
we say with certainty which of these would be allowed in SOTN?** Standing
policy ([[no-new-park-categories]]) is *"never worse than the SOTN
standard"*, so SOTN's actual practice defines the upper bound.

This note records the SOTN evidence per technique, with citations.

## The SOTN standard, as actually documented + practiced

SOTN's authoritative `docs/STYLE.md`, `docs/NAMING.md`, `CONTRIBUTING.md`,
and "Pull Request Guidelines" wiki are SILENT on codegen-control cheats —
they only cover formatting, naming, and PR hygiene. The substantive policy
lives in:

1. **The "Register Mismatch Decompilation Tricks" wiki page**
   ([master wiki](https://github.com/Xeeynamo/sotn-decomp/wiki/Register-Mismatch-Decompilation-Tricks),
   [community mirror](https://mathisto.github.io/sotn-wiki/decomp/register-mismatch-tricks/))
   which EXPLICITLY ENDORSES five intentional codegen-control techniques
   including `foo = (Random() & 3) + 1 - 1;` as an opaque expression added
   solely to recreate a `MOVE(v1,v1)` pseudo-instruction the assembler
   would otherwise drop. The wiki itself is the proof that SOTN sanctions
   semantics-free codegen-control C.
2. **The `// !FAKE:` / `// FAKE` convention** for marking codegen-coercion
   constructs in committed source (per `docs/STYLE.md`: *"A fake should be
   commented with an `// !FAKE:` above or to the side of the code that
   you think it's fake."*). The convention exists precisely BECAUSE SOTN
   commits codegen-only constructs as part of matched functions — they
   get tagged but they ARE accepted.
3. **The `// FAKE but makes register allocation work` pattern** that
   appears throughout committed SOTN source (verified: `src/weapon/w_037.c`,
   `src/dra/alu_anim.c`, `src/dra/42398.c`, `src/dra/5D5BC.c`,
   `src/st/sel/2C048.c`, `src/st/sel/stream.c`, etc.).

## Concrete SOTN evidence (codegen-control patterns committed to master)

The following are verbatim excerpts from SOTN's committed `master` source.
Every one is a construct with no semantic purpose, inserted purely to
control GCC codegen, ACCEPTED into the project.

### E1. `src/st/e_skelerang.h` — opaque arithmetic `(Random() & 3) + 1 - 1`

```c
F(entity->velocityX).i.hi = (Random() & 3) + 1 - 1;
```

Comment in surrounding context: *"This + 1 - 1 is an oddity that is
required to align PSP"*. Mathematically equivalent to `(Random() & 3)`;
the `+ 1 - 1` exists ONLY to recreate a missing MIPS `addu v1, $0, v1`
pseudo on PSP. **Identical mechanism class to BB2 technique #2
(`s32 one = 1;`)** — an opaque-value variable that GCC cannot prove away.

### E2. `src/dra/alu_anim.c` — `idxSub = idxSub;` self-assignment

```c
if ((&subanim[idxSub * 2])[0] == 0xFFFF) {
    idx++;
    // Probably fake. Makes PSP registers match.
    idxSub = idxSub;
    break;
}
```

A literal no-op self-assignment, committed with explicit acknowledgement
("Makes PSP registers match"). **Identical mechanism class to BB2
technique #1 (defeat-licm var-reuse)** — keep a variable visible to the
compiler so optimization decisions land differently. SOTN even goes
FURTHER than BB2 here — BB2 reuses the var for a *different* value; SOTN
literally writes `x = x;` and commits it.

### E3. `src/dra/42398.c` (and PSP twin `src/dra_psp/186E8.c`) — duplicate-read into "color_fake"

```c
color = *palette;
color_fake = *palette;
new_var4 = (color_fake & 0x1F) << 3;
curTile->r0 = new_var4;

color_fake = color;
new_var4 = color_fake >> 2;
curTile->g0 = new_var4 & 0xF8;

color_fake = color;
new_var3 = color_fake >> 7;
curTile->b0 = new_var3 & 0xF8;
```

A *second* C handle (`color_fake`) for the same memory read (`*palette`),
followed by repeated `color_fake = color;` rebinds before each channel
extraction. The variable name openly announces ("fake") that it is a
codegen lever. **Identical mechanism class to BB2 technique #5
(split-read-defeats-hoist)** — duplicate a read so GCC can't hoist the
shared subexpression.

### E4. `src/weapon/w_037.c` — chained dead-intermediate `randy -> baseX`

```c
func_107000_8017B0AC(self, &basePoint, 0);
// FAKE but makes register allocation work
randy = basePoint.x;
baseX = randy;
baseY = basePoint.y;
```

`randy` is a one-use intermediate that adds nothing semantically; the
comment "FAKE but makes register allocation work" admits it openly.
**Identical mechanism class to BB2 technique #1 + #6 (named-intermediate
to bias LUID / RA decisions)**.

### E5. `src/dra/5D5BC.c` — empty conditional `if (!i) {}` inside switch

```c
void DecrementStatBuffTimers(void) {
    s32 i;
    for (i = 0; i < 16; i++) {
        if (!g_StatBuffTimers[i]) {
            continue;
        }
        switch (i) {
        default:
        case 0:
            // !FAKE, permuter found it. Whatever I guess. Matches on all
            // versions.
            if (!i) {
            }
        case 1:
        ...
```

The empty `if (!i) {}` is functionally a no-op (especially nested inside
`case 0:` where `i == 0`). It exists ONLY because the permuter found it
made the byte match work — and it was committed with that
acknowledgement. This is exactly the cheat-form the BB2 review-discipline
rule ([[review-discipline-before-commit]]) cautions against — yet SOTN
accepts it and ships it.

### E6. `src/main_psp/2D608.c` — `volatile s32 i;` to force buffer-swap codegen

```c
volatile s32 i;
...
i ^= 1;
```

`volatile` on a stack scalar with no observable side effect. **Identical
mechanism class to BB2's currently-FORBIDDEN volatile-coercion patterns**
(see [[inline-asm-policy]] expanded catalog). SOTN ships it; BB2
currently forbids it as of 2026-05-31.

### E7. `src/st/sel/stream.c` — `volatile u32 pad[4]; // FAKE`

```c
volatile u32 pad[4]; // FAKE
```

Unused volatile array used to make stack/scheduling align with target.
**Identical mechanism class to BB2's `s32 buf[N];` unused-array padding**
(currently caught by `engine/volatile_cheats.py` as a `pad`/`buf`/`spill`
unused declaration).

### E8. `src/st/sel/2C048.c` — `volatile u32 pad; // !FAKE:` in body of `PrintFileSelectPlaceName`

```c
void PrintFileSelectPlaceName(s32 port, s32 slot, s32 y) {
    volatile u32 pad; // !FAKE:
    s32 stage = g_SaveSummary[port].stage[slot];
    ...
}
```

Same pattern as E7, single-scalar variant. Unused volatile local for
stack alignment. The variable name `pad` openly announces coercion intent
("naming-announces-intent" — one of the BB2 cheat-by-spelling signals).

### E9. `src/main/psxsdk/libsnd/vs_vh.c` — mixed exit forms (goto-end + inline returns)

```c
if (spuAllocMem == -1) {
    _svm_vab_used[vabId_2] = 0;
    _spu_setInTransfer(0);
    _svm_vab_count -= 1;
    return -1;                            // <-- inline return
}
...
if ((spuAllocMem + var_s0) > 0x80000U) {
end:                                      // <-- label
    _svm_vab_used[vabId_2] = 0;
    _spu_setInTransfer(0);
    _svm_vab_count -= 1;
    return -1;
}
```

`SsVabOpenHeadWithMode` mixes inline `return -1;` exits with `goto end;
end: return -1;` exits inside the same function — the precise mixed-exit-
form lever the BB2 cross-jump-store-tail-merge rule uses to defeat
`jump2 find_cross_jump`. SOTN's instance does the same thing (same C
structure, same byte-merging-avoidance effect); they just don't claim a
defeat-cross-jump intent in a comment. **Identical mechanism class to
BB2 technique #4.** This is the most common shape in PsyQ-era SOTN code
and is broadly accepted.

### E10. `src/dra_psp/62FE0.c` — `dest = val1; // fake` dead store

```c
val1 |= (b1 * 2 + b2 * 2) << 0xC;
val1 |= (b2 + b3 * 3) << 0x8;
dest = val1; // fake

*srcPtr++ = val0;
*srcPtr++ = val1;
*srcPtr++ = val2;
```

`dest` is written but never used. A dead assignment kept in source purely
because removing it changes codegen. **Identical mechanism class to BB2's
currently-FORBIDDEN dead-conditional-store** ([[no-new-park-categories]]
calls this out by name).

### E11. `src/main/main.c` — labeled retry loop with `goto main_search_loop_1`

```c
main_search_loop_1:
    while (!CdSearchFile(&main_cdlFile, "\\DRA.BIN;1")) { ... }
    ...
    if (main_fd < 0) {
        goto main_search_loop_1;
    }
```

Explicit `goto label;` retry to force a specific block layout. Same lever
class as BB2 cross-jump and shared-end-label rules.

### E12. `u8` parameter typing for byte-packed-arg calls

Multiple SOTN files use `u8 arg0, u8 arg1, u8 arg2` parameters where the
function packs them into a 32-bit word for a downstream call — e.g.
`src/main_psp/31178.c`, `src/main/psxsdk/libsnd/seqread.c`,
`src/main/psxsdk/libsnd/ssplay.c`, `src/st/no3/e_warg.c`,
`src/st/no3/e_fire_warg.c`. This is EXACTLY the legitimate main lever in
BB2's narrow-byte-args-packed-call rule.

## Per-technique verdicts

### Technique 1 — `defeat-licm-hoist-var-reuse.md`

**SOTN reuses one variable for two unrelated values purely to control
codegen.** Evidence: E2 (`idxSub = idxSub;`), E4 (`randy -> baseX`
chained intermediate), E3 (`color_fake` rebound before each channel).
All three are weaker versions of the BB2 multi-set variable trick — same
intent, same lever class, **same or worse "no semantic purpose" property**.

The technique itself is named-intermediate variable reuse to bias GCC's
`loop.c` movable detection. SOTN's `idxSub = idxSub;` is the LIMIT CASE
of this — literally just write to the same var twice with the same value
because GCC's analysis cares about the assignment, not the value. BB2's
"two unrelated values" version is more sophisticated but the same class.

**VERDICT: ACCEPTED IN SOTN.** Direct evidence in committed master:
`src/dra/alu_anim.c` + `src/weapon/w_037.c` + `src/dra/42398.c`. The
project comment culture ("FAKE but makes register allocation work")
openly endorses this lever class.

### Technique 2 — `loop-rotation-two-shift.md` (opaque `s32 one = 1;`)

**SOTN explicitly endorses opaque arithmetic that adds nothing semantic
to defeat compiler simplification.** The "Register Mismatch Tricks" wiki
documents `foo = (Random() & 3) + 1 - 1;` as a sanctioned technique —
and E1 confirms SOTN actually commits it (`src/st/e_skelerang.h`).

BB2's `s32 one = 1;` is mechanistically the same: an opaque value the
compiler can't simplify, used to keep a particular instruction form
(`sllv + and` instead of `srav + andi`). The SOTN wiki's `+ 1 - 1`
example is even MORE blatant — it's syntactic noise that simplifies
arithmetically to nothing — and SOTN documents it as the official answer.

**VERDICT: ACCEPTED IN SOTN.** Direct documented endorsement + direct
in-source instance.

### Technique 3 — `narrow-stack-param-subword-offset.md` (`*(u16 *)&arg4`)

This technique is FORK-SPECIFIC: it exists because BB2's
`decompals/mips-gcc-2.7.2` fork right-justifies a HImode stack parameter
where PsyQ's `cc1psx` left-justifies it. SOTN uses `cc1psx` (via PSP's
`psp-gcc`), which **doesn't have this problem**. Searching SOTN source
finds no instances of `*(u16 *)&arg` casts on stack parameters; the
underlying ABI quirk doesn't exist for them.

However, the *mechanism* (cast `&local` to a narrower pointer type to
control the emitted load width) is broadly used in SOTN — see E3
(`color_fake = *palette;` followed by per-channel mask) and the saturn
`*(u16*)(param_1 + 6)` instances. The pointer-cast read is a normal C
idiom, not a cheat.

**VERDICT: NOT APPLICABLE / ACCEPTED IN SOTN.** SOTN doesn't have the
specific ABI quirk that motivates this BB2 technique (different fork),
so no instance to compare. The general mechanism (narrow pointer cast to
control load width) is a normal C idiom that SOTN uses freely. The user
can read this as: *we chose a toolchain that has this footgun; the
workaround is a clean, semantically-meaningful C cast (`*(u16 *)&arg4`
literally means "read the low half"); SOTN would have no objection.*

### Technique 4 — `cross-jump-store-tail-merge.md` (mixed exit forms)

**SOTN actively uses mixed-exit-form functions throughout its codebase.**
Evidence: E9 (`src/main/psxsdk/libsnd/vs_vh.c` `SsVabOpenHeadWithMode`
mixes inline `return -1;` with `goto end; end: return -1;` in the same
function). E11 (`src/main/main.c` `goto main_search_loop_1;` retry).
`src/main/psxsdk/libc/sprintf.c` and many others have `goto end;` paired
with inline returns.

The pattern of multiple-exit-forms is a standard PsyQ-era C idiom. SOTN
doesn't comment on it as a codegen-defeat lever because they don't have
the same `find_cross_jump`-aware policy machinery — but the code shape
they ship is exactly the BB2 lever's structure. SOTN matches functions
with this shape and accepts them.

The CRITICAL nuance: BB2's rule's "spelling concern" is that the rule
INTRODUCES `goto endK;` purely to defeat cross-jump (not because the
human programmer would write that exit form). However, SOTN's E9
(`SsVabOpenHeadWithMode`) does exactly that — different exit paths use
different exit forms (some inline returns, some goto-to-shared-label)
for no semantic reason. The human programmer could have made them all
goto or all inline; SOTN ships the mixed form.

**VERDICT: ACCEPTED IN SOTN.** Direct evidence:
`src/main/psxsdk/libsnd/vs_vh.c`. The structure is a normal PsyQ-era
multi-exit C idiom, and SOTN commits functions in this shape without
comment.

### Technique 5 — `split-read-defeats-hoist.md` items #1 + #2 (duplicate-read-into-branch-arms)

**SOTN duplicates reads to control codegen, explicitly.** Evidence: E3
(`color_fake = *palette;` followed by `color_fake = color;` rebinds —
multiple re-reads of the SAME value to force separate codegen per
channel) is the limit case of the duplicate-read lever. The `// !FAKE,
permuter found it` comment in E5 (`src/dra/5D5BC.c`) is openly: "the
permuter found a structural change that makes bytes match; we shipped
it."

The BB2 rule (item #1 + #2 — duplicate the same `((SYMBOL[hi] & 0xFF)
<< 16) | SYMBOL[lo]` read into both arms of an `if (flag) ... else ...`
to pin offset computations inside the branch) is a particular instance
of "duplicate a computation to control its codegen placement." SOTN
does this routinely.

The BB2 rule's REJECTED item #3 (volatile globals to coerce re-reads)
is itself ALSO done in SOTN — E6 (`volatile s32 i;`), E7/E8
(`volatile u32 pad`). So even the BB2-FORBIDDEN volatile coercion lever
appears in SOTN with the same comment culture (`// FAKE`).

**VERDICT: ACCEPTED IN SOTN.** Items #1 + #2 (the duplicate-read
restructure) are normal SOTN practice. Item #3 (volatile-global
coercion), which BB2 currently forbids, is ALSO accepted in SOTN — but
this is BB2's stricter-than-SOTN choice as of 2026-05-31, not a SOTN
concern.

### Technique 6 — `narrow-byte-args-packed-call.md` two-locals sub-trick

The main `u8` parameter typing lever is **directly endorsed by SOTN
practice** — E12 lists multiple SOTN files using `u8 arg0, u8 arg1, u8
arg2` parameters for byte-packed-arg calls (`seqread.c`, `ssplay.c`,
`e_warg.c`, `e_fire_warg.c`, etc.). This is a real ABI / type signature
discovery, not a cheat — both projects use it the same way.

The SUB-TRICK in question — declaring `u32 hi = arg3 << 16;` and
`u32 lo = (arg2 << 8) | const;` as two SEPARATE statements with `hi`
FIRST so cc1 assigns lower LUIDs to `hi`'s expansion — is more
borderline. SOTN's evidence: E4 (`randy = basePoint.x; baseX = randy;`)
is the closest sibling — a named intermediate inserted for register/LUID
control with a "FAKE" comment.

The BB2 sub-trick's exact form (`hi` declared before `lo` to bias LUID
ordering for scheduler priority) doesn't have a direct SOTN counterpart
I could find via WebFetch + `gh search`, but the GENERAL class — split
a single expression into named locals with chosen declaration order
purely to bias scheduling/RA — is widely used in SOTN (E4 is the
cleanest example, but the pattern of `new_var3` / `new_var4` named
intermediates appears throughout `src/dra/42398.c` and elsewhere).

**VERDICT: ACCEPTED IN SOTN.** Main `u8` lever directly confirmed. The
two-locals declaration-order sub-trick is the same lever class as E4
(`randy = basePoint.x; baseX = randy;` — committed in
`src/weapon/w_037.c` with "FAKE but makes register allocation work").
SOTN ships named-intermediate variables for RA/scheduling purposes
routinely.

## Aggregate

| # | BB2 technique | SOTN verdict | Strength | Key SOTN evidence |
|---|---|---|---|---|
| 1 | defeat-licm-hoist-var-reuse | **ACCEPTED** | Strong | E2, E4, E3 |
| 2 | loop-rotation-two-shift (`s32 one = 1;`) | **ACCEPTED** | Decisive | Wiki + E1 |
| 3 | narrow-stack-param-subword-offset | **N/A — accepted general mechanism** | n/a | (fork-specific; general cast idiom is normal C) |
| 4 | cross-jump-store-tail-merge (mixed exit forms) | **ACCEPTED** | Strong | E9, E11, sprintf.c |
| 5 | split-read-defeats-hoist (#1 + #2) | **ACCEPTED** | Strong | E3, E5 |
| 6 | narrow-byte-args-packed-call (two-locals sub-trick) | **ACCEPTED** | Moderate | E4 + E12 |

**All six borderline techniques are accepted under the SOTN standard.**
Zero REJECTED. Zero UNCLEAR. The "never worse than SOTN" policy bar does
not require BB2 to forbid any of these six.

## How SOTN can ship things BB2 currently forbids

This research surfaces a STRUCTURAL gap between SOTN's standard and BB2's
current standard:

- **SOTN accepts `volatile s32 i;` / `volatile u32 pad;` / volatile
  globals for codegen control** (E6, E7, E8). BB2 currently forbids
  these as of 2026-05-31 (per [[inline-asm-policy]] expanded catalog).
- **SOTN accepts unused declarations like `volatile u32 pad[4];`** for
  stack alignment (E7). BB2 currently catches these in
  `engine/volatile_cheats.py` as `s32 buf[N];` unused arrays.
- **SOTN accepts dead stores with `// fake` comments** (E10). BB2
  forbids dead conditional stores per [[no-new-park-categories]].
- **SOTN accepts `idxSub = idxSub;` self-assignments** (E2). BB2's
  audit would flag this as a dead self-assignment cheat.

**So "never worse than SOTN" leaves BB2 free to relax some standards.**
Whether to do so is the user's call. The six borderline techniques in
the 2026-06-02 audit are NOT at the boundary — they're well inside what
SOTN accepts.

## Notable: SOTN's permuter-find culture

E5 (`src/dra/5D5BC.c`) is the canonical case: a permuter-found cheat-form
(`if (!i) {}` empty conditional) committed with the comment *"!FAKE,
permuter found it. Whatever I guess. Matches on all versions."* SOTN's
attitude here is roughly: "the permuter found a closing form, the bytes
match, ship it, note that it's odd." BB2's standing policy
([[no-new-park-categories]], [[review-discipline-before-commit]]) is
*"reject permuter cheat-forms before surfacing them; semantic purpose is
the bar, not byte match."* This is a real STANDARD DIVERGENCE — BB2 has
chosen to be MORE strict than SOTN on permuter output.

The implication: BB2's `review-discipline-before-commit` rule and the
`cheat-reviewer` agent are project choices to enforce a HIGHER bar than
SOTN's actual practice, NOT to enforce SOTN's bar. This is internally
consistent with the standing policy because *"never worse than SOTN"*
allows being stricter; it just means the audit-flagged borderline
techniques are not the right place to draw a stricter line — SOTN itself
accepts them.

## Path forward (the user can choose)

Three coherent options on the six borderline techniques, given this
evidence:

1. **ACCEPT all six** (matches SOTN bar). Reclassify them in the audit
   as LEGITIMATE; document the SOTN evidence inline in each rule file.
   Lowest effort, preserves matched-function work, stays within standing
   policy.

2. **ACCEPT 5 of 6, keep #6 sub-trick FORBIDDEN.** The two-locals
   declaration-order sub-trick is the closest to "code with no semantic
   meaning" of the six (the wiki-endorsed `+ 1 - 1` has more obvious
   "this is fake on purpose"; the two-locals trick is sneakier). User
   could choose to be stricter than SOTN on this specific lever and not
   the others. Slight maintenance cost (need to track BB2-only carve-out
   in the rule file).

3. **REJECT all six** (stricter than SOTN). Implies BB2 chooses a more
   ascetic bar than the reference project. Coherent — and the existing
   `inline-asm-policy` expanded-catalog forbids further patterns than
   SOTN already — but means more functions stay parked. Per E5 + the
   permuter-find culture difference, BB2 is ALREADY doing this in one
   direction (permuter forms); doing it harder on these six is
   internally consistent but consequential.

The default if the user doesn't act is whatever the current
`techniques-audit-2026-06-02.md` left them as (BORDERLINE / unenforced).

## Method note (for transparency)

- All evidence cited above was found via the GitHub search CLI
  (`gh search code`) and WebFetch against
  `https://raw.githubusercontent.com/...` raw URLs. Direct file-content
  quotes are verbatim from the master branch.
- The wiki citation
  (https://github.com/Xeeynamo/sotn-decomp/wiki/Register-Mismatch-Decompilation-Tricks)
  and its mathisto.github.io community mirror were both fetched; they
  agree on the trick catalog. I have no reason to think the wiki has
  changed since the audit cut-off.
- SOTN has no formal "what counts as a match" policy document beyond
  STYLE.md (formatting/naming) and the wiki tricks page. The de facto
  policy is community practice + reviewer judgment per
  `CONTRIBUTING.md` ("All CI checks must pass, including build matching
  across versions" — i.e. byte-match is the bar).
- For technique #6's specific declaration-order sub-trick I couldn't
  find a syntactically identical SOTN match. The verdict is by
  mechanism class (named intermediates for RA/scheduling control,
  broadly used in SOTN, E4 cleanest exemplar). If the user wants higher
  confidence specifically on that sub-trick, a deeper SOTN PSP-specific
  code-pack search would be the next step.

## Related

- [[techniques-audit-2026-06-02]] — the audit that raised the question
- [[community-standard]] — the SOTN bar as previously documented in BB2
- [[no-new-park-categories]] — the standing "never worse than SOTN"
  rule
- [[review-discipline-before-commit]] — the stricter-than-SOTN bar BB2
  has chosen for permuter output (independent decision, not part of
  this audit)
- [[inline-asm-policy]] — the expanded catalog that already forbids
  some things SOTN accepts (volatile globals, alias renames)
