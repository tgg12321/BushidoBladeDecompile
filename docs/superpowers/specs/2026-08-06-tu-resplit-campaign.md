# Campaign 4 — drive `asmfix.txt` to zero

**Status:** scoping + design complete, no build files touched. Execution not started.
**Authority:** owner ruling 2026-08-06 (`docs/grind/decisions.md`, last entry) — all 237
asmfix lines are debt; the project's end state carries no `regfix.txt` and no `asmfix.txt`.
**Machine-readable map:** `tmp/resplit_map.json` (regenerate with
`python3 tmp/resplit_scope.py && python3 tmp/build_resplit_map.py`).
**Supporting scans:** `tmp/stub_risk.json`, probes in `tmp/probe/`.

---

## 1. Headline finding — the campaign is much smaller than the ruling assumed

The ruling's remediation shape was "TU re-split so each function's `asm/funcs/*.s` is
assembled and linked as its own object via `bb2.ld`", with the stated root cause:

> The wiring papers over a fork divergence (under our cc1's `-G8`, file-scope `__asm__`
> floats to `.text 0`; cc1psx evidently kept mid-TU asm in place).

**That diagnosis is correct but its scope was over-generalised. The float is `-G8`-only.**
Measured directly against the project's own cc1 (`tmp/probe/run.sh`, source
`tmp/probe/p.c` = `int a(){} __asm__("MARKER"); int b(){} int c(){}`):

```
########## -G0 ##########          ########## -G8 ##########
18:  .ent  a                       22:  MARKER_ASM_BLOCK     <-- floated to the top
36:  MARKER_ASM_BLOCK   <-- in     36:  .ent  a
42:  .ent  b                place  50:  .ent  b
60:  .ent  c                       64:  .ent  c
```

Under `-G0`, GCC 2.7.2's MIPS back end emits function bodies straight to `asm_out_file`
in source order, so a file-scope `__asm__` lands exactly where it is written. Under `-G8`,
`TARGET_FILE_SWITCHING` buffers function bodies to a temp file and dumps them at end of
TU, so file-scope asm — which bypasses the buffer — floats ahead of every function.

**`GP_FILES := text1a` is the only `-G8` file in the project** (Makefile:104). So:

| Route | TUs | Functions | `bb2.ld` change | New objects | TU splits |
|---|---:|---:|---:|---:|---:|
| `include-asm-in-place` (`-G0`) | 14 | **205** | none | none | none |
| `tu-resplit` (`-G8`, text1a) | 1 | **1** (`save_vc_ctrl`) | 4 lines | 1 asm `.o` | 1 |

The TU-re-split topology the brief asked for was computed anyway and is retained in
`tmp/resplit_map.json` per TU (`asm_runs_if_resplit` / `c_fragments_if_resplit` /
`new_objects_if_resplit`) as the fallback shape. **If the whole campaign were run on the
re-split route it would need 109 split points, 118 C fragments, and 227 objects
replacing today's 15.** The in-place route reduces that to 1 split, 1 fragment pair and
1 new object.

### In-tree proof, not just a probe

`src/ings2.c` already ships two mid-file file-scope `__asm__ glabel` blocks in the
byte-matching build — `func_80083698` at line 577 and `md_gview_init` at line 595, sitting
immediately before `s32 ang_hosei(...)` at line 608:

```c
s32 sys_GetVideoMode(void) { return g_sys_video_mode; }

__asm__(
    ".section .text\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "glabel func_80083698\n"
    ...
    "endlabel func_80083698\n"
    "    .set reorder\n"
    "    .set at\n"
);
```

`src/ings.c:91` and `:117` carry the same form. The mechanism is already load-bearing for
the oracle; this campaign generalises it.

### The pipeline tolerates the `.include` form too

`tmp/probe/run2.sh` pushed `INCLUDE_ASM("asm/funcs", ang_hosei)` through the **full**
per-file pipeline (`cpp | cc1 -G0 | prologue_fix | maspsx | multu_pad | regfix |
regfix_stage2 | asmfix | as`) and disassembled the object:

```
00000000 <qa>:      jr ra / addiu v0,a0,1
00000008 <ang_hosei>:  move a3,a2 / move a2,a1 / ...    <-- in place, correct bytes
```

`as` resolved `.include "asm/funcs/ang_hosei.s"` from the repo root (make's cwd), `glabel`
resolved from the `#include "include_asm.h"`-emitted `.include "include/macro.inc"`, and
the block assembled clean with zero stderr. A standalone assembly of the same `.s` (probe
C) produced byte-identical text, confirming `AS_FLAGS` alone is sufficient for these files
— relevant to the one TU that does need a separate object.

---

## 2. What is actually in scope

`asmfix.txt` is 319 lines / 237 rule lines / 218 functions:

- **206 `replace_with_asmfile` rules — this campaign.** 66 are canonical-asm authorized
  (`inline_asm_canonical.txt`), 140 are queue-active undecompiled functions.
- **29 lines across 12 functions — tier 1, NOT in scope.** `delete_between` /
  `insert_before` body injections and label-retarget compensations. These resolve via real
  decomp, not representation work: `pad_main_control`, `saSeInit_2`, `func_80033DF4`,
  `saTan4FireDisp`, `mk_leaf_newpos`, `saTan5TakeAnim2`, `func_80017848`, `CalcHiraNormal`,
  `func_8002CA8C`, `func_80070C70`, `func_80056CB8`, `func_80060A68`. After this campaign
  they are all that remains in `asmfix.txt`.
- `SetPacketData` is in both buckets (a `replace_with_asmfile` plus a label-retarget rule);
  only its asmfile line is in scope.

**Zero of the 206 carry any `regfix.txt` rule.** Deleting them touches `asmfix.txt` only.

### Stub-body census (`tmp/stub_risk.json`)

| Class | Count | Meaning |
|---|---:|---|
| empty stub (≤3 body lines) | 194 | `void f(A a){ (void)a; }` — emits no `.L` labels, no rodata |
| substantial body | 12 | a real WIP decomp attempt that the build silently discards |
| emits non-`.text` | 3 | `cpu_check_run_attack`, `SetPacketData` (string literals), `func_80089F3C` (switch) |
| declared `static` | 0 | no visibility changes needed |

The 194 empty stubs are the mechanically safe bulk: removing a body with no control flow
and no literals shifts nothing — not cc1's `.L` counter, not `.rodata`, not `.data`/`.bss`.

---

## 3. Risks, measured

**R1 — cc1 `.L`-label counter drift (the only real one).** cc1's label counter is
file-global and monotonic, so deleting a function body renumbers every later sibling's
labels. `asmfix.txt:22` documents a past instance verbatim: *"removing `save_vc_ctrl`'s
pure-C body shifted text1a.c's cc1 label counter down by 7."* Rules written with
`{lbl#N}` slot references are drift-robust; rules with a hardcoded `.L<N>` are not.

> **ERRATUM (2026-08-06, Wave-0 execution audit).** The table below is a scan
> artifact and Wave 0 had NOTHING to convert. `tmp/resplit_scope.py`'s
> `HARD_L = re.compile(r'(?<!\{lbl)\.L\d+')` lacks a trailing boundary, so it
> matches the digit PREFIX of self-defined ADDRESS-form labels
> (`.L80070D7C` → matches `.L80070`). Per-line audit
> (`tmp/verify_wave0_drift.py`): all four "drifted rule owners" have ZERO
> cc1-numbered label references — func_80056CB8 / func_80070C70 are already
> `{lbl#N}` slot-form; CalcHiraNormal / func_8002CA8C anchor on `^\.frame` /
> `^\.end` assembler directives with payload-self-defined address labels.
> The genuinely hardcoded cc1-label references in the tree are elsewhere:
> asmfix SetPacketData (`.L761/.L762/.L764`), regfix tslPrintScreen (`.L26`),
> marionation_Exec (`.L999` ×3), mk_leaf_newpos (`.L631`). tslPrintScreen and
> marionation_Exec sit before every substantial stub in their TU (safe);
> **SetPacketData and mk_leaf_newpos were NOT ordering-assessed by this plan
> and must be checked before their TUs' waves (5 and text1b's 6).** Fix the
> regex (`\.L\d+(?![0-9A-Za-z_])`) and regenerate `label_drift_exposure` in
> `tmp/resplit_map.json` before consulting it for Waves 2–7.

Measured exposure — a substantial stub with a hardcoded-`.L` rule **after** it in the same
TU — is **four rules in three TUs**:

| TU | drifted rule owner | driven by |
|---|---|---|
| `code6cac_b` | `func_8002CA8C` | `saTan2KabutoWareMove` (137 lines), `PutRobShadow` (85) |
| `code6cac_c2` | `CalcHiraNormal` | `replay_camera_get_attack_number` (4) |
| `text1b` | `func_80056CB8`, `func_80070C70` | `func_80048FFC` (87) |

All four owners are tier-1 functions. `config`/`tslPrintScreen`, `system`/`marionation_Exec`
and `ings`/`func_80017848` also hold hardcoded-`.L` rules but sit *before* every substantial
stub in their TU, so they cannot drift. Mitigation is Wave 0 below.

**R2 — discarding a WIP decomp attempt.** The 12 substantial stubs represent real prior
work (see `docs/history/asmfix_attempt_notes.md` for the plateau notes). Do not delete them
into the void; move each body to the function's grind ledger (`memory/grind/<func>/`) before
replacing it with `INCLUDE_ASM`. This is not optional — the body is the resume point.

**R3 — non-`.text` emission from a removed body.** Three functions only. Verify against the
build after each: if a discarded switch table or string literal was contributing bytes to
the object's `.rodata`, removing it shifts every later rodata symbol in that object and
every subsequent object's rodata. Treat these three as individual commits, never batched.

**R4 — concurrent edits to `src/text1a.c`.** Another agent is working that file now, and it
is the one TU needing the true re-split. It is scheduled last (Wave 7) for exactly this
reason. Do not start Wave 7 until that work has landed and `verify-oracle` is green.

**R5 — `-G8` is not the only special flag.** `code6cac_b` is in `EXPAND_LB_FILES`;
`RODATA_ALIGN2_FILES` covers 11 of the 15 TUs. Neither interacts with the in-place route
(both are stream post-processing, blind to whether a function's text came from cc1 or an
`.include`), but a TU split would need both lists extended for the new stems — one more
reason to prefer the in-place route.

**R6 — engine/tooling awareness.** *(CORRECTED 2026-08-06 after the Wave 1 pilot. The
original text is preserved at the end of this section; all three of its claims were
measurably FALSE, and acting on them would have silently deleted ~140 undecompiled
functions from the queue.)*

`engine/cheats.py` counts asmfix rules per function and `engine/canonical.py` gates on
them. The intended end state is unchanged:

- the 66 canonical-authorized functions stay listed in `inline_asm_canonical.txt` — they
  then reach **COMPLETED-INLINE-ASM-CANONICAL with zero rules**, which is the whole point
  of the ruling;
- the 140 unauthorized ones stay **INCOMPLETE** in `engine/queue.json` — they are
  undecompiled. What changes is that their INCOMPLETE state is now honestly represented in
  source (SOTN's `INCLUDE_ASM` / `NON_MATCHING` state) instead of a build-time text
  substitution.

**But none of that was true of the engine as it stood.** Measured on the pilot:

| Claim in the original R6 | Measured before the fix |
|---|---|
| "a whole-body `INCLUDE_ASM` is cheat-asm by the detectors' definition" | FALSE — `engine/inlineasm.py` runs on UNEXPANDED source and `cia.ASM_KEYWORD_RE` matches only `__asm__`/`__asm`. `INCLUDE_ASM(...)` is a macro INVOCATION: zero matches, never a candidate. It only becomes `__asm__` after cpp, which the stripper never runs. |
| "the 140 unauthorized ones stay INCOMPLETE, correctly" | FALSE — with no C body, `file_func_cheat_asm_count` returned -1, which `generate()` (`cheat_count <= 0`) read as CLEAN and dropped, and which `mark_done()` (`cheat_count > 0`) did not refuse. `queue done` would have recorded an undecompiled function as COMPLETED-C. |
| "`sandbox --disable all` strips cheat-asm, so the honest distance is unaffected" | FALSE — nothing was stripped, so the sandbox assembled the target bytes straight from `asm/funcs/<name>.s`: `sandbox ang_hosei --disable all` returned **0** for a function with zero lines of C (recorded distance 50). A direct break of CLAUDE.md non-negotiable #3. |

`engine test` passing did NOT contradict any of this — no test pinned `INCLUDE_ASM`
behaviour. Layer-2 review FAILed the pilot on these grounds and the source change was
reverted until the engine was fixed.

**Resolved by commit `9e68966c`** (`engine: make whole-body asm visible to the cheat
detectors + completion gate`), which must land before any further wave:

- whole-body asm is recognised and ATTRIBUTED to the named function across all three
  spellings (`INCLUDE_ASM`, the hand-expanded `.include`, and `glabel` bodies), so a
  function with no C body counts > 0 instead of UNKNOWN;
- the `INCLUDE_ASM` forms are STRIPPED for scoring, so the honest distance reflects "no C
  exists" (pilot: 0 → 51). Attribution is not stripping — `glabel` bodies keep their
  never-strip `canonical_body` treatment;
- `mark_done()` refuses a non-canonical function on an UNKNOWN count; `generate()` drops
  only on a MEASURED-ZERO count, with an explicit `_not_a_c_function()` exception for
  symbols that are not C-level functions at all;
- asm-supplied functions route on the opcode verdict alone, so the "whole function is
  missing" distance cannot push large conversions to `ASM-STRUCTURAL`/`authorize` and out
  of the active lane.

**Standing check for waves 2-7.** After each wave, confirm on a sample of the converted
functions: `sandbox <func> --disable all` is NON-ZERO with `no_c_body: true`; a read-only
regen simulation retains them as active; `queue done <func>` is REFUSED. A conversion that
makes the sandbox read 0 is the defect recurring, not a success.

<details><summary>Original R6 text (superseded — retained so the failure mode stays legible)</summary>

> A whole-body `INCLUDE_ASM` is cheat-asm by the detectors' definition, so: […] the 140
> unauthorized ones stay **INCOMPLETE** in `engine/queue.json`, correctly […]
> `sandbox --disable all` strips cheat-asm before scoring, so the honest pure-C distance
> metric is unaffected by the conversion. Confirm with `engine test` after Wave 1.

The lesson: the plan asserted what the detectors *ought* to do from reading their intent,
and never measured what they *did*. "Confirm with `engine test`" was circular — the suite
had no assertion covering the new construct.
</details>

---

## 4. Pilot — `ings2` / `ang_hosei`

The lowest-risk item in the project: one asm function, an empty stub, no hardcoded-`.L`
siblings, not `-G8`, not `EXPAND_LB`, not `RODATA_ALIGN2`, and **the target form already
exists twice in the same file, thirty lines above the edit site**.

Current state — `src/ings2.c:608`, and `asmfix.txt:` the line
`ang_hosei: replace_with_asmfile "asm/funcs/ang_hosei.s"`:

```c
s32 ang_hosei(s32 a0, s32 a1, s32 a2) {
    (void)a0;
    (void)a1;
    (void)a2;
    return 0;
}
```

### Recipe

1. **Baseline.** `& tools/wteng.ps1 main verify-oracle --rebuild` → must be
   `62efab4f73f992798c43e8c730aa43baa10bb4fa` before touching anything.
2. **Header.** `src/ings2.c` already has `#define INCLUDE_ASM_USE_MACRO_INC 1` +
   `#include "include_asm.h"`. Confirm; add if absent (all 15 in-scope TUs already have it
   except where noted in `tmp/resplit_map.json`).
3. **Prototype.** Add `extern s32 ang_hosei(s32, s32, s32);` to the file's extern block so
   in-file callers keep their declaration. (Callers outside the TU already use externs.)
4. **Replace the definition in place** — same source position, so the emitted text address
   is unchanged:
   ```c
   INCLUDE_ASM("asm/funcs", ang_hosei);
   ```
5. **Delete** the `ang_hosei: replace_with_asmfile ...` line from `asmfix.txt`.
6. **Verify.** `& tools/wteng.ps1 main verify-oracle --rebuild`. SHA1 must be unchanged.
   This is the whole gate — the output is byte-identical or it is not.
7. **Audit.** `python3 tools/check_completion_integrity.py` and `& tools/wteng.ps1 main test`.
8. **Commit** `cheat-cleanup: ang_hosei — asmfix replace_with_asmfile retired via in-place
   INCLUDE_ASM`, after a fresh layer-2 `cheat-reviewer` per
   `.claude/rules/review-discipline-before-commit.md`. The reviewer brief should ask
   specifically whether converting a build-rule substitution into a source-level
   `INCLUDE_ASM` is representation work or a relocation of the same cheat — the answer
   should be the former (the bytes' provenance is unchanged and verbatim; what changes is
   that the source now says so), but that judgment belongs to the reviewer, not the author.

LF line endings on `src/ings2.c` and `asmfix.txt`. Do not run `make setup`.

### After the pilot

Wave 1 establishes the pattern and the reviewer verdict. Everything after it is the same
five edits repeated, so build `tools/resplit_convert.py` (read `tmp/resplit_map.json`,
apply steps 3–5 for a named function or a whole TU, emit the diff) rather than hand-editing
205 sites. One `verify-oracle` per commit; batch by TU, not by function, except for the 12
substantial stubs and the 3 non-`.text` emitters.

---

## 5. Wave plan

| Wave | TUs | asm fns | Notes |
|---:|---|---:|---|
| 0 | prereq | — | Convert the 4 drifted hardcoded-`.L` rules (`func_8002CA8C`, `CalcHiraNormal`, `func_80056CB8`, `func_80070C70`) to `{lbl#N}` slot form. Own commit, own `verify-oracle`. Independently useful — it de-risks every future edit to those TUs. |
| 1 | `ings2` | 1 | **The pilot.** Establishes form + reviewer precedent + `tools/resplit_convert.py`. |
| 2 | `ings`, `config`, `code6cac_c_mid` | 3 | One asm fn each, all empty stubs, zero drift. |
| 3 | `code6cac_b2_post`, `text1b_b`, `system`, `code6cac_c2` | 11 | Small multi-function TUs. `system`/`tslTm2LoadImage` and `code6cac_c2`/`replay_camera_get_attack_number` are marginal-substantial (4–5 lines, 0 control flow) — ledger the body anyway. |
| 4 | `text1a_c`, `display` | 6 | Straightforward; `text1a_c` is `-G0` despite the name (only `text1a` is `-G8`). |
| 5 | `main`, `code6cac`, `code6cac_b` | 48 | Holds 6 of the 12 substantial stubs and all 3 non-`.text` emitters — individual commits for those 9, batched for the other 39. |
| 6 | `text1b` | 136 | The bulk. 65 of the 66 canonical-authorized functions live here, so this wave is what converts the canonical set to zero-rule finished form. Split into ~5 commits by address range. |
| 7 | `text1a` | 1 | The only true TU re-split. Blocked on the concurrent `src/text1a.c` work. |
| 8 | pipeline | — | Blocked on tier-1 (12 functions of real decomp). When `asmfix.txt` is empty: drop the `ASMFIX` stage from the Makefile pipeline, retire `tools/asmfix.py`, drop it from `PIPELINE_DEPS`, and update `engine/cheats.py` + `CLAUDE.md`. |

### Wave 7 recipe — `text1a` / `save_vc_ctrl` (0x80041434)

The only place the jtbl-precedent shape genuinely applies. `save_vc_ctrl` sits between
`func_80041430` (16 C functions before it) and `func_8004153C` (21 after).

1. Split `src/text1a.c` into `src/text1a_pre.c` (through `func_80041430`) and
   `src/text1a_post.c` (from `func_8004153C`), duplicating the entire preamble —
   `#include`s, macros and every `extern` — verbatim into both, exactly as `code6cac_b2_pre.c`
   / `_post.c` did in commit `672768f6`. Do not try to partition the declarations.
2. Delete `save_vc_ctrl`'s stub; it becomes a linked object, not a source construct.
3. Makefile: add an explicit opt-in list rather than widening the `asm/*.s` wildcard (which
   would build all 1437 `asm/funcs` objects to be discarded):
   ```make
   LINKED_ASM_FUNCS := save_vc_ctrl
   ASM_FUNC_O_FILES := $(patsubst %,$(BUILD_DIR)/$(ASM_DIR)/funcs/%.o,$(LINKED_ASM_FUNCS))
   ```
   and add `$(ASM_FUNC_O_FILES)` to `ALL_O_FILES`. The existing
   `$(BUILD_DIR)/$(ASM_DIR)/%.o: $(ASM_DIR)/%.s` pattern rule already covers
   `asm/funcs/save_vc_ctrl.s` with the correct `AS_FLAGS`.
4. `asm/funcs/save_vc_ctrl.s` needs a standalone preamble — it currently has
   `.set noat` / `.set noreorder` but no `.include "macro.inc"` and no `.section .text`.
   Prepend them (probe C in `tmp/probe/run2.sh` is the reference form). Note it is one of
   the 4 `asm/funcs` files with no `endlabel`; add one.
5. `bb2.ld` — 4 sections, mirroring `672768f6`:
   ```
   .rodata:  text1a.o -> text1a_pre.o, text1a_post.o
   .text:    text1a.o -> text1a_pre.o, build/asm/funcs/save_vc_ctrl.o, text1a_post.o
   .data:    text1a.o -> text1a_pre.o, text1a_post.o
   .bss:     text1a.o -> text1a_pre.o, text1a_post.o
   ```
   This is the project's **first** raw asm object linked between two C fragments — every
   prior split placed only C-derived objects. `bb2.ld` is hand-maintained; edit it directly
   and never run `make setup`.
6. Makefile lists: `GP_FILES := text1a_pre text1a_post` and both stems added to
   `RODATA_ALIGN2_FILES` (replacing `text1a` in each).
7. Delete the `save_vc_ctrl` asmfix line. `verify-oracle --rebuild`.

Expect the `.L`-counter shift to land here: `asmfix.txt:22-23` records that this exact
removal moved text1a's counter by 7 last time. `saTan4FireDisp`'s `{lbl#6}` rules are
already in drift-robust slot form, which is why they survived it.

---

## 6. Effort estimate

The unit of work is a TU, not a function. Waves 0–6 are 15 commits' worth of highly
repetitive edits behind one script, each gated by a single `verify-oracle --rebuild`:

| Bucket | Units | Character |
|---|---:|---|
| Wave 0 label-rule conversion | 1 commit | Mechanical, 4 rules, independently valuable |
| Waves 1–6, empty stubs (194 fns) | ~13 commits | Scripted; the risk is entirely in the verify, not the edit |
| Substantial stubs (12) + non-`.text` (3) | ~9 commits | Individual; each needs its body preserved to the grind ledger first |
| Wave 7 `text1a` re-split | 1 commit | The only structural change; the only `bb2.ld`/Makefile edit |
| Wave 8 pipeline retirement | 1 commit | **Blocked on tier-1 decomp**, not on this campaign |

Waves 0–7 are a small number of sessions of low-novelty work, dominated by full-rebuild
verify time rather than by thinking. The campaign does **not** finish `asmfix.txt`: it
takes it from 237 lines to **29 lines across 12 functions**, all of which are genuine
decomp debt that only the Grinder can retire. Wave 8 — deleting `tools/asmfix.py` from the
pipeline — is gated on those 12.

## 7. Open question for the owner

The 140 non-canonical functions convert from a build-rule substitution to a source-level
`INCLUDE_ASM`. That is SOTN's exact representation for an undecompiled function and it
removes the dishonesty the ruling named (a C body compiled and silently discarded). It does
**not** decompile anything: those 140 stay INCOMPLETE and queue-active. Worth confirming
the owner reads that as progress on representation rather than as a change in completion
status — the queue count does not move.
