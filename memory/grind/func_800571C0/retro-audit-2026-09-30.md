# func_800571C0 — retro-audit CONCERN (owner Q49), analysis 2026-09-30 (laneG) — open

Archived landing ledger: memory/grind/_completed/func_800571C0/. Concern (retro-audit 2026-09-29 batch_01,
ecfdc52a6): `s8 nr` holds the right-side clear-step count, then is reused as the chosen-side flag
(`nr = 0` / `nr = 1`, read by `if (nr != 0)` in the waypoint loop); the landing said "no FAKE constructs" and
claimed no ruling. Undisclosed reuse = FAIL class (lane brief lesson 3).

Measured (sandbox --disable all, probes-2026-09-30/, landed body 0/287): fresh flag replacing the reuse
`u8 toL` 68 (295/287), `s32 toL` 72 (294/287), `s8 toL` declared before `nl` 47 (295/287) — the auditor's 47.
The reuse saves 8 instructions, so it is load-bearing.

Routes (none executed yet): (1) an ordinary spelling that needs no reuse (e.g. derive the side from the counts
without a separate flag write); (2) a Q51 SOTN citation of a count reused as a direction flag; (3) Ruling 11
package (dumps naming the pass, honest generic name — `nr` fails (E) — annotation, layer-2). If none closes,
the Q37 fallback (revert to INCLUDE_ASM + reopen) goes to the orchestrator first.

First dump look (2026-09-30, `cc1 -dl -dg -df` on the landed body and the `s8 toL` probe; scratch in
tmp/func_800571C0/rtl_*): same frame (168, 10 saved regs); the fresh flag perturbs global allocation in the
probe loop — the target keeps `&hit` in a stack slot (sp+120) and 0x1F8002B8 in $s7, the probe spills the flag
instead and reseats those call arguments ($s6/$s7 swap, extra moves). Mechanism not yet named from .greg.

Mechanism (named 2026-09-30 from the .lreg/.greg of the same two builds; cmdline in tmp/func_800571C0/cmdline.txt,
to be re-banked under dumps/ when this item is worked): global.c allocno priority
(floor_log2(n_refs) * n_refs / live_length). Reuse: `nr` = pseudo 75, 13 refs over 161 insns -> 3*13/161 = 0.242,
above the three call-argument pseudos competing for the last callee-saved registers (125: 9 refs/121 = 0.223,
126: 9/120 = 0.225, 127: 9/118 = 0.229) -> nr gets $s6 ($22), pseudo 125 (`&hit`) is spilled to sp+120 as in the
target. Fresh flag: `nr` = pseudo 76, 10 refs over 149 insns -> 0.201, below all three -> nr is spilled
(`sb $0,120($sp)`), the constant takes $s6, and the flag (pseudo 74, 3 refs/12 insns) sits in $a0. This is the
Ruling 11 (D)(2) mechanism; (D)(3)/(D)(4) (search, permuter, ablations), (E) name, (F) annotation still owed.
