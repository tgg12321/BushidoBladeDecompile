# mk_leaf_newpos — WIP (round 2, 2026-08-05)

`src/code6cac_b.c:3374`, stem **`code6cac_b`** (note: `code6cac.c` only declares it
`extern`; callers are in `code6cac.c:1839/1944` and `code6cac_c2.c:873`).
Measure: `& tools/wteng.ps1 main sandbox mk_leaf_newpos --disable all`;
frame `wsl bash tmp/csz/gn_frame.sh code6cac_b mk_leaf_newpos`.

## Baseline — this is a PURE register-allocation residual

| quantity | ours | target |
|---|---|---|
| honest `sandbox --disable all` | **65** | 0 |
| `build_insns` | **176** | **176** |
| frame | `.frame $sp,48 vars=0 regs=7/0 args=16` | `addiu $sp,$sp,-0x30` = 48 |

**Instruction count and frame already match exactly**, and FRAMEDBG shows `vars=0` — no
stack temps, no spill slots, no phantom slots. So none of the frame/phantom machinery
applies here; every point of the 65 is register choice or ordering. That is consistent
with the heavy-12 triage row (43 rules, RENAME 39 = 60%, STRUCT 9, class MIXED RA 60% /
LICM 15%) and makes this the cleanest `ra_solver` case in the queue.

## ra_solver is now EXACT here (18/18) — and the fix was a real solver bug

Round 1 measured 17/18 with pseudo 89 missing (sim `$2`, dump `$4`). Diagnosed:
`BB2_FINDREG_DEBUG=89` shows `retry=0` and **zero `retry=1` anywhere in this function**,
so it was NOT the known reload-spill-retry gap. The dump gives
`own_copy_prefs: 4`, `own_full_prefs: 2 4` — GCC took the COPY preference (`$4`) while
`simulate.py` took the lowest of the merged set (`$2`).

Cause: `simulate.py`'s preference upgrade was an explicit approximation
(`# single dumped pref set; copy==full approx`). GCC's `find_reg`
(`global.c:1077-1115`) upgrades in **two stages** — scan
`hard_reg_copy_preferences` ascending and on a hit `goto no_prefs`, **skipping the
plain-preference stage entirely**; only if no copy pref fits does `hard_reg_preferences`
get its turn. `prune_preferences` masks the copy set the same way (`global.c:897`).
Fixed in `tools/ra_solver/simulate.py` (the model already extracted `copy_prefs`
separately, so no extractor change was needed).

**Regression check: `validate.py` still 9/10**, identical to the documented baseline —
the only miss remains `saTan4FireDisp` (the known reload-retry case, 14/17). No banked
function changed. `mk_leaf_newpos` goes 17/18 -> **18/18 exact**.

Per-function instrumented-cc1 parity for `mk_leaf_newpos` in `code6cac_b` is **OK**
(`wsl bash tmp/csz/mk_parity.sh code6cac_b mk_leaf_newpos`) — the TU-level `parity=False`
noted in the sched_solver README is confined to `func_80030900` (`or` vs `addu`, line
9204) and does not touch this function.

## The primary rename has an exact spec (10 single-atom solutions)

`regfix.txt` carries 41 rules; the clean rename leads are `$16 <-> $17 @ 1-71`,
`$16 <-> $20 @ 77-78`, `$5 <-> $3 @ 81-97`, `$5 <-> $3 @ 136-142`.

For the first: ALLOCDBG has **84 -> `$16` (nrefs 12, livelen 22, pri 16363)** and
**72 -> `$17` (nrefs 17, livelen 42, pri 16190)**. 84 outranks 72 by only **173 points**,
so 84 takes `$16` first; target wants the opposite order. `perturb.py` with spec
`{"84": 17, "72": 16}` returns **ten** single-atom solutions:

| allocno | atoms that flip it |
|---|---|
| 72 | `refs+1`, `refs+2`, `live-2`, `live-4`, `live-8` |
| 84 | `refs-1`, `live+2`, `live+4`, `pref+r17`, `pref=r17` (reroute) |

i.e. lift 72's priority or lower 84's. Per the catalog laws: refs = real uses,
livelen = statement span, pref = a copy relationship. **Identify what 72 and 84 ARE in
the C before spelling anything** — the atoms are cheap to satisfy in several ways and
only the semantically-motivated one is acceptable.

NB the rule set is NOT purely RA: it also has `delete`, `reorder`, `insert`, and
`subst "nop" "<insn>"` rules (the last are lost-codegen inserts). Instruction count
already matches, so those are scheduling/delay-slot shape, not missing work.

## Next

1. **Identify pseudos 72 and 84 in the C** (`.flow`/`.lreg` dump for `code6cac_b`,
   `tmp/csz/hw_rtl.py` or `gn_pseudo.py`), then pick the atom whose C spelling is
   semantically motivated — NOT whichever is easiest to force. Two reviewers have already
   rejected a codegen-motivated construct in this queue; the same bar applies.
2. Then the remaining rename leads (`$16 <-> $20`, the two `$5 <-> $3` ranges): build
   their specs the same way and re-run `perturb.py`.
3. `defeat-licm-hoist-var-reuse` (LICM 15% in triage) is the secondary axis; treat it as
   downstream of the RA fix.
4. **Do NOT commit src.** Owner runs the gate; layer-2 before any completion claim.

Tools: `tmp/csz/mk_parity.sh`, `mk_findreg.sh`, `mk_perturb.sh`.
