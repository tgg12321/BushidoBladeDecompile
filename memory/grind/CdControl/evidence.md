# Evidence bank — CdControl

## Session 1 (recon, 2026-08-14)

### Baseline, routing, shape
- `canonical CdControl` → verdict **C**, `asm_insns 0`, `total 78`. Pure-C target;
  there is no canonical-asm question for this function.
- Honest floor at session start: **25** (`sandbox CdControl --disable all`),
  `target_insns 78 / build_insns 78`. Instruction COUNT and control-flow shape
  were already exact at session start — the entire distance was (and still is)
  register assignment plus the prologue emission order that follows from it.
- Rules dropped by the sandbox: 3 (`regfix.txt:58-61` — `$18 <-> $20 @ 11-59`,
  `$19 <-> $20 @ 11-59`, and a `reorder 3,18,4,17,... @ 3-19` prologue rule).
  Confirms the same reading: two register renames + a prologue-order shuffle.
- The HEAD form carried a **register pin** (`register s32 result asm("s7");`)
  plus the `int new_var = 3; unsigned long long new_var2 = new_var; count =
  new_var2;` DImode chain. Both are gone from the session-1 candidate; neither
  was buying anything the sandbox could see.

### Target register seating (from `asm/funcs/CdControl.s`)

| value | target |
|---|---|
| `count` (=3, retry counter) | s0 |
| `a1` (param copy) | s1 |
| `a2` (param copy) | s2 |
| `idx` (`a0 & 0xFF`) | s3 |
| `a0` (raw param copy) | s4 |
| `saved` (old `g_cd_callback_a`) | s5 |
| `elem` (`&g_cd_sector_buf[idx]`) | s6 |
| `result` (0 / -1, returned as `result + 1`) | s7 |

Note this is the SAME relative ordering as the already-matched sibling
`CdControlF` (count, a1, [a2], idx, a0, saved, elem, result), shifted by the
one extra param. `CdControlB` — also already matched, also 3 params, same body —
seats s1=a1, s2=a2, s3=idx, s4=a0, s5=saved, s6=elem (`asm/funcs/CdControlB.s`),
i.e. identical. The three siblings are one family and the solved ones are the
primary evidence source for this function.

### The floor walk this session: 25 → 21 → 20 → 4

1. **Port the matched sibling's solved shape (25 → 21).** Delete the `s7` pin and
   the `new_var`/`new_var2` DImode chain; replace the one-step
   `elem = &g_cd_sector_buf[idx];` with `CdControlF`/`CdControlB`'s two-step
   `base = g_cd_sector_buf; elem = base + idx;`. This is the spelling both
   matched siblings use; it makes the symbol materialise into `v1` and the
   scaled index into `v0`, matching target.
2. **Init-statement order sweep, no wrap (21 → 20).** All **240** legal
   permutations of the six initialiser statements (constraints: `idx` before
   `elem`, `base` before `elem`) scored over the range **20 … 31**. Best:
   `idx | base | result | saved | count | elem` → 20. Artifact:
   `tmp/grind/CdControl/s1/sweep_results.json`.
   Residual seating at 20: s2=idx (want s3), s3=saved (want s5), s5=a2 (want s2),
   s6=result (want s7), s7=elem (want s6) — a 3-cycle plus a swap.
   Artifact: `tmp/grind/CdControl/s1/build20_CdControl.txt`.
3. **`do { ... } while (0);` loop-note wrap around the goto-loop body (20 → 6).**
   The single biggest lever, and it lands the ENTIRE register assignment exactly
   on target (s0..s7 all correct) in one step. Artifact:
   `tmp/grind/CdControl/s1/build6_CdControl.txt`.
4. **Re-sweep init order with the wrap in place (6 → 4).** All 240 permutations
   again; **six** orders reach 4, and every one of them ends with `elem` then
   `result` as the last two initialisers. Best (and the form the candidate
   uses): `idx | saved | count | base | elem | result` — byte-for-byte the same
   init order the matched `CdControlF` uses. `CdControlB`'s own order
   (`saved | count | idx | base | elem` + `result` last) also scores 4.
   Artifact: `tmp/grind/CdControl/s1/sweep2_results.json`.

### Where the remaining 4 sits (exact)
Candidate form (`memory/grind/CdControl/candidate.c`, applied to src/system.c):
s0=count ✓, s1=a1 ✓, s2=a2 ✓, s3=idx ✓, s6=elem ✓, s7=result ✓, but
**s4=saved (target: a0)** and **s5=a0 (target: saved)** — one pairwise priority
inversion between the raw-`a0` copy and `saved`, plus the prologue
store/define emission order that follows from it. Artifact:
`tmp/grind/CdControl/s1/build4_CdControl.txt`.

Value-definition order actually emitted at 4:
`a0, a1, a2, result, count, idx, [lui/addiu %hi %lo], saved, elem`
Target's: `a1, a2, a0, count, idx, [lui/addiu], saved, elem, result`.

### The mechanism, and why the do-while(0) wrap moves it
GCC 2.7.2 `flow.c` life analysis accumulates `REG_N_REFS (regno) += loop_depth`
per reference; `global.c allocno_compare` then orders allocnos by roughly
`floor_log2(n_refs) * n_refs * size / live_length`, and `global_alloc` hands out
hard registers in that order (so a higher-priority allocno gets a lower-numbered
callee-save). A backward `goto` does NOT create `NOTE_INSN_LOOP_BEG`, so without
a wrap every reference in the retry loop is weighted at depth 0. The
`do { ... } while (0);` wrap emits the loop notes, which re-weights exactly the
references inside the retry body: `a2` (+2), `a1` (+2), `count` (+2), `idx` (+1),
`elem` (+1), `saved` (+1 of its 3), `a0` (+1 of its 2), and `result` (+0 — its
only two writes are outside the body). That is precisely the direction the
target seating needs, which is why one construct moved 20 → 6.

### Killed: the honest real-loop restructure (two spellings, both regress)
Writing the retry loop as a genuine `do { ... } while (count != -1);` — no FAKE
construct at all, plain honest C, loop notes for free — scores **13**, not 4.
Cause (verified in the disassembly): a loop.c-visible loop lets LICM hoist the
loop-invariant `-1` into a NINTH callee-saved register (`s8`), so the function
grows to 82 instructions vs target's 78 and gains an s8 save/restore pair.
Target materialises `-1` inside the loop every iteration
(`addiu $v0,$zero,-1`, `asm/funcs/CdControl.s:65`).
Follow-up spelling `} while (count >= 0);` denies loop.c an invariant to hoist
and does remove `s8` — but scores **13** at 79 instructions, because target
spends TWO instructions on the exit test and `bgez` spends one.
Artifacts: `tmp/grind/CdControl/s1/build13_realloop.txt`,
`tmp/grind/CdControl/s1/build13_bgez.txt`.
Banked forms: `memory/grind/CdControl/rejected/real-loop-licm-hoists-minus1-regress-13.c`,
`memory/grind/CdControl/rejected/real-loop-bgez-exit-regress-13.c`.

**But the real-loop form is the single most informative negative of the session:**
under it, `a0` seats in **s4** and `saved` in **s5** — i.e. the exact pairwise
ordering the score-4 candidate is missing. A loop.c-visible loop fixes the
a0/saved inversion; it just costs an instruction elsewhere. Any form that
reaches 0 must obtain that a0/saved ordering while keeping 78 instructions and
the two-instruction `!= -1` exit test.

### Notes on the do-while(0) construct's status
The wrap is a SANCTIONED family ([[do-while-zero-exception]] / the frozen SOTN
list), used with an identical mechanism and an identical FAKE annotation in the
already-committed matched sibling `CdControlF` (`src/system.c:192-197`, commit
`589bf161`). It is annotated in the candidate. It is NOT being submitted this
session: the floor is 4, not 0, and the exhaustion prerequisite is explicitly
not yet met — this is session 1 and the frontier below is un-run.

## Session 2 (structural, 2026-08-14) — floor 4 → 0

### The closing lever: the parameter's declared TYPE
`CdControl`'s first parameter is a CD command **byte**. HEAD and the session-1
candidate both declared it `s32`. Declaring it `u8` — the real PsyQ libcd
signature (`int CdControl(u_char com, u_char *param, u_char *result)`) and the
spelling BOTH already-matched siblings in the same file use (`CdControlF`
`src/system.c:184`, `CdControlB` `src/system.c:236`) — moves the honest sandbox
distance from **4 to 0** with nothing else changed. It is a one-token change and
it is a typing CORRECTION, not a coercion: the target masks the value at each
use (`andi $s3,$s4,0xFF` at entry, `andi $a0,$s4,0xFF` in the retry loop,
`asm/funcs/CdControl.s:12,56`), which is precisely what GCC 2.7.2 does for a
`u8` parameter whose incoming register is not known extended.

That single change dissolves the entire session-1 residual: the a0-copy/`saved`
priority inversion at s4/s5, and the prologue emission order that followed from
it, both land on target simultaneously.

### The measured surface (all at 78 == 78 instructions)

| axis | result | artifact |
|---|---|---|
| param `s32`/`u8` × named `raw = a0;` intermediate | s32/no-raw **4**, s32/raw **4**, **u8/no-raw 0**, u8/raw **5** | `phaseA.json` |
| `u8`, wrap-free, all 240 legal init orders | floor **17**, range 17…30 | `phaseD_nowrap_u8.json` |
| `u8`, wrap-free, all 720 declaration orders | **every order 17** — declaration order is completely inert | `phaseF_nowrap_declorder.json` |
| `u8`, with wrap, all 240 legal init orders | **6 orders reach 0**, range 0…30 | `phaseE_wrap_u8.json` |
| `u8`, mask-`idx` × mask-call-arg × wrap | masks entirely inert: **0** with wrap in all 4, **17** without in all 4 | `phaseG_masks.json` |

(Artifacts live in `tmp/grind/CdControl/s2/`; harness `probe.py`.)

Consequences banked:
- **The `& 0xFF` masks are dropped.** At `u8` they are no-ops that buy nothing,
  so the candidate reads `idx = a0;` and `CD_cw(a0, a1, a2, 0)` — byte-for-byte
  CdControlF's spelling, and one fewer redundant-width-cast question for review.
- **The do-while(0) wrap is load-bearing and worth 17 points** at the correct
  parameter type. Removing it cannot be rescued by any initialiser order (240
  swept) or any declaration order (720 swept) or by the masks. That is the
  lever-exhaustion evidence the FAKE annotation cites.
- **Declaration order is a dead axis for this function** — GCC 2.7.2 creates
  pseudos at first USE, so the initialiser order (already exhausted in s1 and
  re-swept here) is the only order lever. Do not re-sweep declarations.
- **The six zero-scoring init orders are exactly the six that tied at 4 in
  session 1.** The `u8` type shifted the whole score surface down by 4; it did
  not select a different order. Session 1's order choice was already optimal.

### Status
Honest sandbox distance **0** (`sandbox CdControl --disable all`, 78/78) with the
edits in place in `src/system.c`. The three regfix rules (regfix.txt:58-61) are
untouched by this session and are inert under the cheat-invisible sandbox; the
operator's `retire` + full-build SHA1 verification is the remaining step.
Self-vet written to `memory/grind/CdControl/self_vet.md`.

- [s2] The closing lever was the DECLARED TYPE of the command parameter: s32 -> u8 (the real PsyQ CdControl(u_char com, ...) signature, and the type both matched siblings CdControlF/CdControlB already use) moves the honest floor from 4 to 0 with nothing else changed, at 78 == 78 instructions.

- [s2] Mechanism: for a u8 parameter GCC 2.7.2 does not treat the incoming register as known-extended, so it keeps the raw copy live and masks at each use — exactly what target does (andi $s3,$s4,0xFF at entry, andi $a0,$s4,0xFF in the retry loop, asm/funcs/CdControl.s:12,56). That restores the a0->s4 / saved->s5 seating the whole session-1 residual consisted of.

- [s2] KILLED: a named 'raw = a0;' intermediate for the raw command word. Inert at s32 (4 -> 4) and a REGRESSION at u8 (0 -> 5) — it splits the parameter's own allocno from the copy. Banked at memory/grind/CdControl/rejected/named-raw-intermediate-regress-5.c.

- [s2] KILLED: declaration order as a lever. All 720 permutations of the six local declarations score 17 (wrap-free, u8, best init order) — GCC 2.7.2 creates pseudos at first USE, so initialiser order is the only order lever. Do not re-sweep declarations for this function.

- [s2] KILLED: the hope that the correct u8 parameter type makes the do-while(0) wrap unnecessary. Wrap-free floor is 17 across all 240 legal init orders (range 17..30) and unmoved by all 720 declaration orders and by every mask combination. The wrap is load-bearing and worth 17 points; this is the lever-exhaustion evidence backing its FAKE annotation.

- [s2] The '& 0xFF' masks are inert once a0 is u8 (all four mask/no-mask combinations score 0 with the wrap, 17 without), so the candidate drops them: idx = a0; and CD_cw(a0, a1, a2, 0) — byte-for-byte CdControlF's spelling.

- [s2] The six init orders that reach 0 with the wrap are exactly the six that tied at 4 in session 1 — the u8 type shifted the entire score surface down by 4 rather than selecting a new order, so session 1's order choice was already optimal.

- [s1] canonical CdControl -> verdict C, asm_insns 0, total 78: pure-C target, no canonical-asm question exists for this function.

- [s1] Honest floor at session start was 25 with target_insns 78 == build_insns 78 — instruction count and control flow were already exact, so the whole distance was and still is register assignment plus the prologue emission order that follows from it.

- [s1] The 3 rules the sandbox drops are regfix.txt:58-61 — two register renames ($18<->$20, $19<->$20 @ 11-59) and one prologue 'reorder ... @ 3-19'. That is the same reading as the disassembly.

- [s1] HEAD carried a register pin (register s32 result asm("s7")) and an int/unsigned-long-long DImode chain for count = 3. Both are removed in the session-1 candidate and neither was buying anything the cheat-invisible sandbox could see.

- [s1] Target seating (asm/funcs/CdControl.s): s0=count, s1=a1, s2=a2, s3=idx, s4=a0(raw), s5=saved, s6=elem, s7=result.

- [s1] CdControlB (already matched, 3 params, same body, NO do-while(0) wrap in its C) seats s1=a1, s2=a2, s3=idx, s4=a0, s5=saved, s6=elem — identical to CdControl's target. It is a working counter-example proving this seating is reachable from a plain goto-loop.

- [s1] Score-4 candidate seating: s0,s1,s2,s3,s6,s7 all correct; s4=saved and s5=a0 are inverted versus target. That single pairwise inversion plus its knock-on prologue emission order is the entire remaining 4.

- [s1] Value-definition order emitted at score 4: a0, a1, a2, result, count, idx, [lui/addiu], saved, elem. Target's: a1, a2, a0, count, idx, [lui/addiu], saved, elem, result.

- [s1] MOST INFORMATIVE NEGATIVE: the KILLED real-loop form is the only form measured this session that seats a0 in s4 and saved in s5 — exactly the inversion the score-4 candidate carries. Loop.c visibility fixes the a0/saved order; it just costs an instruction elsewhere. Any zero form must obtain that ordering while keeping 78 instructions and the two-instruction != -1 exit test.

- [s1] The do-while(0) wrap in the candidate is a sanctioned family with an identical mechanism and identical FAKE annotation already committed in the matched sibling CdControlF (src/system.c:192-197, commit 589bf161). It is annotated in the candidate but is NOT being submitted this session: the floor is 4, not 0, and the lever-exhaustion prerequisite is explicitly not yet met.
