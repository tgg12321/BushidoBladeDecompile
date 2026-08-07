# special_camera_Exec — WIP checkpoint (2026-06-08)

**Status:** INCOMPLETE, blocked on jtbl-rodata-split-infrastructure (multi-file architecture decision).

## TL;DR

I hand-derived a complete pure-C body from the target asm + the audit-confirmed
Kengo:HIGH cross-reference (`nm_special_cam/special_camera_Exec`, 274i). It is
structurally correct (sandbox `--disable all` drops 272 → 69 = 75% reduction)
but **cannot link** with the asmfix bridge removed:

```
ld: 101C.rodata_pre.o:(.rodata+0x110): undefined reference to `.L80036D74'
(+13 more)
```

The external `jtbl_80010978` (in `asm/data/101C.rodata_pre.s`) references
`.L80036xxx` labels that USED to live inside the asmfile body. With pure C, GCC
emits a new jtbl into `code6cac_b2.o(.rodata)` which cannot be at 0x80010978
(rodata link order, `bb2.ld:29-31` < `bb2.ld:50`).

This is the **identical pattern** as `replay_camera_rob_back_loose2` — the only
other current jtbl-infra case in BB2 — which is in the `authorize` bucket as
jtbl-infra-parked.

## Resume from here

1. Read `meta.json` for the full session ledger + remaining_gap structural detail.
2. Apply `candidate.c` body (replacing the `void special_camera_Exec(void) {}` stub
   in `src/code6cac_b2.c`) AND remove `special_camera_Exec: replace_with_asmfile ...`
   from `asmfix.txt`. Add the 4 required externs documented in `candidate.c`'s header.
3. `& tools/eng.ps1 sandbox special_camera_Exec --disable all` should report
   score **69** (matches `scores.candidate_floor`).
4. **Confirm the link failure** (this is the blocker) with `& tools/eng.ps1 verify-oracle --rebuild`
   — expect 14 `undefined reference to .L80036xxx` errors. That's the proof the
   gap is structural-rodata, not register-allocation.
5. Revert (keep oracle green). The blocker is real and only fixable by user policy.

## Why I did not commit jtbl-infra rules

Per `[[no-new-regfix-rules]]` (2026-06-08), net additions to `regfix.txt` /
`asmfix.txt` are forbidden by default. `jtbl-infra` is a sanctioned escape-hatch
category, BUT per `[[jtbl-rodata-split-infrastructure]] § EVIDENCE-BASED
ATTRIBUTION ONLY` (2026-06-01), the resolution path is **evidence-based
source-file re-attribution requiring user policy sign-off + multi-function impact
analysis — out of scope for a single-function completion pass**. Adding the
14-rule jtbl-infra cluster here would expand the multi-owner block by another
function, deepening the debt rather than retiring it.

The brief for this session explicitly authorized this WIP-and-stop outcome:
> If you can't reach SHA1 == oracle without any of these → commit a WIP entry
> at `memory/wip/special_camera_Exec/` documenting your candidate floor + tried
> levers + next hypotheses. Do NOT commit cheats. Do NOT force a match with a cheat.

## The 4-sibling cluster context

`special_camera_Exec` is one of 4 stub functions in the same multi-owner rodata
block (`tmp/4-sibling-audit-2026-06-08.md`):

| Function | Insns | Kengo | Status |
|---|---|---|---|
| `func_80034708` | ~352 | LOW (reverted) | ASM-STRUCTURAL, distance 542 |
| `func_80035828` | ~227 | none | ASM-SUSPECT, distance 358 |
| `special_camera_set_win_cam` | ~318 | MED 502i (-10) | ASM-STRUCTURAL, distance 510 |
| **`special_camera_Exec`** | **~178** | **HIGH 274i** | **ASM-SUSPECT, distance 272** (this WIP: **floor 69**) |

This (HIGH cross-ref + smallest of the 4) is the easiest-tractable. The 4-sibling
audit explicitly endorsed pure-C decomp as the legitimate path (NOT canonical-asm
authorization). My WIP confirms the pure-C body is reachable — the blocker is
purely rodata link order.

## Audit-grade evidence the candidate body is correct

- Sandbox `--disable all` (cheat-free) measures **69** vs HEAD's 272 (75% gap closure).
- Build_insns 284 vs target_insns 274 (10 over, mostly the bounds-check codegen
  difference — my switch emits `sltiu<14; addiu-2; sll/sra; sltiu<12` to skip
  cases 0xE/0xF, vs target's `sltiu<14; sll<<2; jtbl[]` mapping 0xE/0xF to
  fall-through end via duplicate jtbl entries).
- Function-by-function objdump shows the body is structurally close to target:
  same call sequences (`Vu0SetLightColMatrix_800801E8`, `myRobGeneiDraw3`,
  `_DispCharacterName`, `cdrom_*`, `func_80080148`, `sys_VSync`), same state
  transitions, same global writes (`D_80101E62`, `D_80101E66`, `D_80101E5C`,
  `D_80101E80`, `D_80101E84`, `D_80101E8C`, `D_80101E98`, `D_80101EA0`).

## Key call-site disambiguation (Kengo / named_syms.txt)

- `myRobGeneiDraw3` (= `func_80080390` = `tslTm2_command_with_retry_no_arg3_80080390`)
  is the 2-arg variant; declared `extern void myRobGeneiDraw3(s32, void *);`
- `_DispCharacterName` (= `func_80080258` = `tslTm2_command_with_retry_80080258`)
  is the 3-arg variant; declared `extern void _DispCharacterName(s32, s32, s32);`
- The arg-count distinction matters per `[[cross-jump-call-merge]]` — calls with
  different signatures don't merge their FUNCTION_USAGE lists.

## Related

- `[[jtbl-rodata-split-infrastructure]]` — the rule governing this blocker
- `[[no-new-regfix-rules]]` — why jtbl-infra rules are no-go even though
  jtbl-infra is a sanctioned escape-hatch category (this expansion deepens debt)
- `[[no-new-park-categories]] § "Speculative system-wide rodata reorders"` — why
  reordering bb2.ld without evidence is a structural cheat
- `tmp/4-sibling-audit-2026-06-08.md` — the audit that triaged this function as
  pure-C-reachable (LOW verdict against canonical-asm authorization)
- `replay_camera_rob_back_loose2` (src/code6cac_b2.c:191) — the canonical example
  of the same blocker (jtbl-infra parked); use as architectural template if/when
  user authorizes rodata re-attribution
