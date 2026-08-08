# Evidence bank — func_80021A98

## Session 1 (recon, 2026-08-07)

### Baseline
- `canonical func_80021A98` → verdict **C**, 158 insns, distance 20 — pure-C target, no ASM routing question.
- `sandbox func_80021A98 --disable all` → honest floor **20 / 158** (19 regfix rules dropped; rules are all `subst` register renames — regfix.txt lines 653-674).
- `diagnose` → LARGE (d45 raw), but the raw number includes reloc/alias noise; the masked 20 is the true gradient.
- Build insn count matches target exactly (158 = 158): **zero structural diffs — the entire distance is register allocation.**

### The complete diff map (tmp/grind/func_80021A98/s1/{target.txt,ours.txt,norm_diff.py})
After normalizing objdump aliases (li = addiu $N,$0,K; move = addu; jal reloc), exactly **two independent RA permutation clusters** remain, ~20 scored insns:

**Cluster 1 (~16 insns, target lines 26-53): `v1` holds $3 in target, $2 in ours.**
- `v1` = the `u16` read of `*(arg1+4)` (stored to s0+0x5C), then `v1*4` shifted IN PLACE (sll $3,$3,2 target / sll $2,$2,2 ours) and used as table index in BOTH arms of the `if (arg2)`.
- Target allocation: v1→$3, table-pointer loads/result→$2, else-arm `idx = a3*5*4`→$4.
- Our allocation: v1→$2, which cascades: else-arm idx→$3, table loads→$3/$4, addu operand orders flip (`addu $2,$2,$3` target vs `addu $2,$3,$2`/`addu $2,$4,$2` ours).
- Fixing v1→$3 alone should cascade-fix the entire cluster (idx moves to $4 because $3 is taken, table loads get $2).
- The final addu operand order follows register identity, not C operand order — it self-fixes with the swap.

**Cluster 2 (4 insns, target lines 57-69): `a0_58`/`a1_val` hold $4/$5 in target, $5/$4 in ours.**
- `a0_58` = `*(s32*)(s0+0x58)` loaded line 57, dereferenced line 69 (`lbu $2,0($4)` target).
- `a1_val` = `*(u8*)(v0_50+6)` loaded line 60, stored to s0+0x40 line 68.
- Both pseudos have 2 refs; a0_58 has the LONGER live range yet wins $4 in target. Under GCC 2.7.2 global.c priority (refs/live-length), the shorter-lived a1_val should win the first pick — and in our build it does (gets $4). So the target's allocation is NOT explained by plain priority order — something in the original C gave a0_58 an edge (extra ref, different creation order effect, or a1_val was e.g. narrower/differently typed).

### m2c reference shape (structure hints)
- m2c folds the second u16 read inline: `var_v0 = tbl2 + M2C_FIELD(temp_v0, u16*, 2)` — does NOT reuse v1. (Measured byte-neutral for us — see P1.)
- m2c shares ONE `*(s0+0x58) = var_v0` store after the if/else; our per-arm stores produce the identical bytes via cross-jump (measured: forcing the shared store is WORSE — see P2).
- m2c shows `func_800324D0(temp_s0, temp_a1, var_a3)` (3 args) — VERIFIED SPURIOUS: target asm sets only $4 (`jal` + `addu $a0,$s0,$zero` delay); $5/$6 are just live-through. The real call is 1-arg as our C has it.
- No sibling/duplicate analog: D_80102764/D_801027B4 table idiom appears nowhere else in src (only the initializer func_80020DDC); tmp/duplicates_leads.txt has no entry for this function. func_8001EFA0 (comment at src line 1124) is a matched sibling by FAMILY but different shape (void, no table block) — its regfix block is empty (matched clean).

### Probe measurements (session 1)
- **P1** fold second read inline (`v0 = tbl2 + *(u16*)(v0+2)` instead of `v1 = ...; v0 = tbl2 + v1`): **byte-identical** (score 20, same diff). CSE unifies both spellings. Kept in source (matches m2c shape); not a lever.
- **P2** single shared `v0` across arms + one shared 0x58 store after if/else (m2c literal shape): **WORSE — 25, build 157 insns** (breaks the cross-jump tail shape; one insn lost). Rejected → rejected/shared-v0-shared-store.c.
- **P3** declare `a1_val` before `a0_58` (pseudo creation order swap for cluster 2): **byte-identical** (score 20, cluster-2 diff unchanged). Declaration order alone does not flip the $4/$5 tie. Killed as spelled; reverted.

### Artifacts
- tmp/grind/func_80021A98/s1/target.txt, ours.txt — normalized instruction listings (158 each)
- tmp/grind/func_80021A98/s1/norm_diff.py, diff.sh — rerunnable differ (run diff.sh in WSL after any sandbox call)
- tmp/grind/func_80021A98/s1/sbs.txt — side-by-side raw

- [s1] canonical: verdict C, 158 insns, distance 20 — no ASM routing question

- [s1] sandbox --disable all: honest floor 20/158; the 19 regfix rules (regfix.txt 653-674) are ALL subst register renames matching exactly the two diff clusters

- [s1] build emits exactly 158 insns = target: ZERO structural diffs; the whole distance is register identity

- [s1] Cluster 1 (~16 insns): target holds v1 (u16 read of arg1+4, shifted in place) in $3 with table-pointer temps in $2; ours reverses; else-arm idx and all addu operand orders are downstream cascade of this one swap

- [s1] Cluster 2 (4 insns): a0_58 (lw s0+0x58, longer live range) wins $4 in target; plain refs/live-length priority predicts OUR allocation (a1_val gets $4), so the original spelling carried an extra edge for a0_58 — prime suspect: a0_58 unified with the second 0x58 load (v1_58) into one 4-ref variable

- [s1] m2c 3-arg call func_800324D0(s0, a1_val, a3) is SPURIOUS — target asm sets only $a0 before jal; real call is 1-arg as in our C

- [s1] no sibling/duplicate analog: D_80102764/D_801027B4 idiom unique to this function; tmp/duplicates_leads.txt has no entry

## Session 2 (structural, 2026-08-07)

### Floor: 20 -> 2 (158/158 insns). Five stacked pure-C levers, all measured individually.

| # | lever | score |
|---|---|---|
| P4 | s32 v1 + `v1 <<= 2` per arm (in-place shift, removes the sll local temp) | 26 alone (polluted), 12 stacked |
| P9 | `arg0 = a3*5` dead-param reuse for the else-arm index | 20 -> 15 (first lever landed) |
| P9b | split-init chain `arg0=a3<<2; arg0+=a3; arg0<<=2` (kills the a3<<2 temp) | 12 -> 10 |
| P11 | arg1 param retyped s32 + reused for the a1_val byte (`arg1 = *(u8*)(v0_50+6)`) | 10 -> 6. REQUIRES include/code6cac.h:442 -> (s32,s32,s32) |
| P12 | operand flip `*(u16*)(v0+2) + BASE` in both arms' second sum | 6 -> 2 |

### The mechanism map (from cc1 -da greg/lreg dumps + instrumented-cc1 ALLOCDBG/FINDREGDBG; artifacts in tmp/grind/func_80021A98/s2/)
- Pseudo roles (baseline numbering): 94 = v1 (HImode u16 var), 95/106 = v0 if/else, 98 = sll temp,
  99/102 = table base values (if-arm), 75 = s0, 85 = a3, 72/73/74 = args.
- **Cluster-1 root cause:** with `u16 v1` and `(v1*4)`, the shift result is a SEPARATE block-local
  temp; local-alloc (runs BEFORE global) gives it $2 (highest qty priority, len 1), which pushes the
  base loads to $3 and leaves v1 free to take $2 globally.  Writing the shift in place
  (`s32 v1; v1 <<= 2`) deletes that temp; base loads then take $2 and v1 is pushed off $2.
- **local-alloc priority formula** (local-alloc.c qty_compare): pri = floor_log2(n_refs) * n_refs *
  size / (death - birth), computed on POST-sched1 suids, so source statement order barely moves it
  (explains P3's null result).  Copy-suggested qtys allocate first, then priority order; ties break
  by qty birth order.
- **Cluster-2 root cause:** a1_val [60..68] pri 1*2/8 = 0.25 beats a0_58 [57..69] 1*2/12 = 0.167, so
  a1_val picked $4 first and a0_58 fell to $5.  FIXED by removing a1_val from local-alloc entirely:
  retype arg1 to s32 and reuse it for the byte (`arg1 = *(u8*)(v0_50+6)`) - arg1's pseudo is GLOBAL
  (prologue copy from $5) with copy-pref 5, so it takes $5 (target) and a0_58 gets $4 (target).
- **Else-arm index chain** must be the REUSED arg0 param (dead after the prologue, home-copy
  preference $4): `arg0 = a3*5*4` gives the whole sll/addu/sll chain $4 as in target; the split-init
  spelling keeps every intermediate in arg0's pseudo (no $2 temp).  a3 then loses $4 to the conflict
  and correctly falls to $7 (its $2..$6 are all blocked: v0-web, arg1, arg0-idx, arg2).
- **Preference inheritance (the v1=$5 trap):** global.c expand_preferences merges hard-reg
  preferences between an insn's SET_DEST pseudo and any REG_DEAD pseudo on that insn when they do
  NOT conflict.  v1's birth insn (lhu) carries arg1's REG_DEAD, so v1 inherits arg1's home pref $5,
  and find_reg's preference override sends v1 to $5 even though $3 is free.  find_reg tries own
  copy-prefs then own full prefs, scanning ASCENDING - a pref set {3,5} resolves to $3.  P12
  exploits this: spelling the second table sum with the u16 read as the FIRST operand of PLUS makes
  set_preference (which reads only XEXP(src,0)) record the lhu temp's local-allocated $3 as a pref
  on the v0 web, which the REG_DEAD merge at v1's death propagates to v1 -> v1 = $3.
- **The last 2 insns (score 2):** GCC 2.7.2 does NOT canonicalize reg-reg PLUS operand order; the
  emitted addu operand order follows the source.  So the SAME spelling that delivers pref-3 also
  flips `addu $v0,$v0,$v1` (target, base first, lines 38/53) into `addu $2,$3,$2` (ours).
  All registers are correct; only these two operand orders differ.
- **Order-preserving variants measured:** sw-first + no flip = 6 (v1 = $5);
  lhu-first + no flip = 2 with the sw/lhu PAIR transposed instead (target: sw@25 lhu@26; the
  scheduler cannot reorder them - mutually-aliasing mems, emitted order = source order; lhu-first
  source makes arg1 die on the mem-dest sw, which expand_preferences skips, killing the pref without
  P12).  Two distinct score-2 states exist: (a) order-correct/operand-flipped [BANKED as candidate],
  (b) operand-correct/sw-lhu-transposed.
- cc1 stderr on this file ("conflicting types", "parse error before GameObj") is pre-existing and
  non-fatal (parser recovers; the build tolerates it).  A pipe through `head` SIGPIPE-kills cc1 -
  redirect stderr to a file instead.
- Instrumented cc1 = tools/gcc-2.7.2/cc1 (env BB2_ALLOC_DEBUG=1, BB2_FINDREG_DEBUG=<pseudo>,
  stderr to file); build/cc1 has NO hooks (confirmed via strings).

### FINAL: sandbox distance 0 (158/158) — P13 mixed operand order
After banking the score-2 form, flipping ONLY the else-arm's second sum back to base-first
(if-arm stays `*(u16*)(v0+2) + D_80102768`, else-arm `*(s32*)((u8*)&D_801027B8+arg0) + *(u16*)(v0+2)`)
measured **sandbox 0**.  The three arrangements: base-first/base-first = 6 (v1=$5),
offset-first/offset-first = 2 (both addus operand-swapped), MIXED = 0.  Note the if-arm addu EMITS
base-first despite its offset-first spelling (an expand/combine-level swap not fully explained —
the pref-3 delivery theory is validated only for the register outcome, not the emission order).
Session returned RULING-REQUEST, not candidate-ready: the mixed operand order is justified by
set_preference mechanics (GCC-internals, T3 signal) and the arms are spelled inconsistently, so it
may fall under the or-tree-shape-shift forbidden family (operand reordering in
associative+commutative expressions) — or may be ordinary 2-operand-sum spelling (both orders
appear naturally throughout src).  Bytes are PROVEN; only the construct's classification is open.

## Session 3 (structural, 2026-08-07) — candidate-ready

- The s2 ruling-request was ANSWERED: Judge PASS, 2026-08-07 22:21
  (docs/grind/decisions.md:4073-4075, committed 9b326242). The mixed 2-operand-sum
  operand order is ordinary expression spelling; or-tree-shape-shift is scoped to
  3+-operand associative chains; no FAKE annotation required. The ruling also
  independently re-verified sandbox 0 at the time it was made.
- Session 3 found src/code6cac.c ROLLED BACK to the pre-s2 form (the driver
  discards src edits on non-candidate-ready outcomes; only memory/ was committed).
  Re-applied the banked candidate.c body + the include/code6cac.h:442 prototype
  (u8* -> s32) exactly as banked.
- `sandbox func_80021A98 --disable all` THIS session: **score 0, 158/158,
  19 rules dropped, cheat_asm_stripped 139** — distance 0 proven live in src.
- self_vet.md written (6-test vet per construct + 3 sanctioned-family claims with
  verbatim scope quotes and file:line precedents). Outcome: candidate-ready.

## Session 4 (structural, 2026-08-07) — candidate-ready re-affirmed

- Dispatched on a STALE digest (showed only s1 state — the s2/s3 ledger updates
  were uncommitted, per the known grinder-stale-digest-uncommitted-ledger
  failure mode). No re-derivation was needed: the ledger carried everything.
- Found src/code6cac.c + include/code6cac.h rolled back to the pre-s2 form
  again. Re-applied the banked candidate.c body verbatim (arg1 param u8*->s32
  in both the src signature and the header prototype at include/code6cac.h:442,
  arg0 split-init reuse, s32 v1 in-place shifts, byte-offset table casts,
  mixed second-sum operand order per the Judge-PASSed ruling
  docs/grind/decisions.md:4073-4075 / commit 9b326242).
- `sandbox func_80021A98 --disable all` THIS session: **score 0, 158/158,
  19 rules dropped, cheat_asm_stripped 139** — distance 0 proven live in src.
- self_vet.md re-verified against the live diff (identical constructs) and
  annotated. Outcome: candidate-ready.

## Session 5 (structural, 2026-08-07) — candidate-ready re-affirmed again

- Dispatched on the SAME stale s1-only digest (s2–s4 ledger updates still
  uncommitted — grinder-stale-digest-uncommitted-ledger failure mode). The
  on-disk ledger carried everything; zero re-derivation performed.
- Found src/code6cac.c + include/code6cac.h rolled back to the pre-s2 form a
  third time. Re-applied the banked candidate.c body verbatim: arg1 param
  u8* -> s32 (src signature + include/code6cac.h:442 prototype) reused for the
  a1_val byte, arg0 split-init reuse for the else-arm index, s32 v1 with
  in-place `v1 <<= 2` per arm, byte-offset table casts, mixed second-sum
  operand order per the Judge-PASSed ruling (docs/grind/decisions.md:4073-4075,
  commit 9b326242).
- `sandbox func_80021A98 --disable all` THIS session: **score 0, 158/158,
  19 rules dropped, cheat_asm_stripped 139** — distance 0 proven live in src.
- self_vet.md re-verified against the live diff (identical constructs) and its
  header updated. Outcome: candidate-ready.
- OPERATOR NOTE: this is the THIRD consecutive candidate-ready on an identical
  banked form. The stale-digest loop persists because memory/grind/func_80021A98/
  is dirty-uncommitted; committing the ledger (or accepting the candidate)
  breaks the loop.

## Session 6 (structural, 2026-08-07) — candidate-ready re-affirmed (4th)

- Dispatched on the SAME stale s1-only digest (grinder-stale-digest-uncommitted-
  ledger failure mode; s2–s5 ledger updates still uncommitted). Zero
  re-derivation — the on-disk ledger carried everything.
- Found src/code6cac.c + include/code6cac.h rolled back to the pre-s2 form a
  FOURTH time. Re-applied the banked candidate.c body verbatim via 5 targeted
  edits: arg1 param u8* -> s32 (src signature + include/code6cac.h:442
  prototype) reused for the a1_val byte, arg0 split-init reuse for the else-arm
  index, s32 v1 with in-place `v1 <<= 2` per arm, byte-offset table casts,
  mixed second-sum operand order per the Judge-PASSed ruling
  (docs/grind/decisions.md:4073-4075, commit 9b326242).
- `sandbox func_80021A98 --disable all` THIS session: **score 0, 158/158,
  19 rules dropped, cheat_asm_stripped 139** — distance 0 proven live in src.
- self_vet.md FIXED per the 22:31 OPERATOR NOTE: the split-init family
  PRECEDENT line was still the prose reference to the user-memory file (the
  exact citation-format discard cause — no file.ext:LINE / hex-hash match for
  grindlib.py:54's regex). Replaced with the prescribed anchor `ad11a8c8` +
  docs/grind/decisions.md:4075. All three family PRECEDENT lines now carry
  regex-passing citations. Outcome: candidate-ready.

## Session 7 (structural, 2026-08-07) — candidate-ready re-affirmed (5th)

- Dispatched on the SAME stale s1-only digest (grinder-stale-digest-uncommitted-
  ledger failure mode; s2–s6 ledger updates still uncommitted per git status).
  Zero re-derivation — the on-disk ledger carried everything.
- Found src/code6cac.c + include/code6cac.h rolled back to the pre-s2 form a
  FIFTH time. Re-applied the banked candidate.c body verbatim via 5 targeted
  edits: arg1 param u8* -> s32 (src signature + include/code6cac.h:442
  prototype) reused for the a1_val byte, arg0 split-init reuse for the else-arm
  index, s32 v1 with in-place `v1 <<= 2` per arm, byte-offset table casts,
  mixed second-sum operand order per the Judge-PASSed ruling
  (docs/grind/decisions.md:4073-4075, commit 9b326242).
- `sandbox func_80021A98 --disable all` THIS session: **score 0, 158/158,
  19 rules dropped, cheat_asm_stripped 139** — distance 0 proven live in src.
- self_vet.md re-verified against the live diff (identical constructs; all
  three PRECEDENT lines already carry the regex-passing citations from the
  s6 fix — ad11a8c8, .claude/rules/no-new-park-categories.md:170,
  docs/grind/decisions.md:4073-4075). Header updated to note the 5th
  re-affirmation. Outcome: candidate-ready.
- OPERATOR NOTE: FIFTH consecutive candidate-ready on the identical banked
  form, and the first since the s6 citation-format fix landed in self_vet.md.
  If THIS session is also discarded, the discard reason fed back into the next
  brief (per commit bb6ac932) is the thing to read first — the ledger and vet
  are internally complete.

## Session 8 (structural, 2026-08-07) — SRC-ONLY sandbox-0 form (header constraint resolved)

- NEW BINDING CONSTRAINT in this session's brief: the driver ruled the
  include/code6cac.h edit OUT OF SCOPE — candidates for func_80021A98 may only
  edit src/code6cac.c; the u8* -> s32 prototype change at include/code6cac.h:442
  can never be accepted. This invalidated the s2–s7 banked form AS SPELLED
  (it required the header edit for the P11 arg1-reuse lever).
- RESOLUTION (measured this session): arg1 KEEPS its header type `u8 *` in the
  src signature (header untouched), and the P11 reuse is spelled with the two
  semantically-required 32-bit int<->pointer conversion casts:
  `arg1 = (u8 *) *((u8 *) (v0_50 + 6));` and
  `*((s16 *)(s0 + 0x40)) = (s32) arg1;`. Rationale: RA operates on RTL modes,
  not C types — the arg1 pseudo is SImode either way, its GLOBAL status and $5
  prologue copy-preference (the actual P11 mechanism) are type-independent, and
  the casts emit zero instructions (no width change). All other levers
  unchanged from the banked form (arg0 split-init reuse, s32 v1 in-place
  shifts, byte-offset table casts, Judge-PASSed mixed second-sum operand
  order).
- `sandbox func_80021A98 --disable all` THIS session with ONLY src/code6cac.c
  modified: **score 0, 158/158, 19 rules dropped, cheat_asm_stripped 139** —
  the src-only form is byte-proven; the out-of-scope constraint is fully
  satisfied (no OWNER-ESCALATION for scope_allow.txt needed).
- candidate.c re-banked with the src-only body (supersedes the s2 form —
  header note updated to say NO header edit needed or allowed). self_vet.md
  rewritten for the new construct list (casts vetted: not F2 — no width
  change, mandatory for the assignment to compile; not volatile-coercion — no
  volatile anywhere). All PRECEDENT lines keep the regex-passing citations
  from the s6 fix. Outcome: candidate-ready.

### Artifacts (tmp/grind/func_80021A98/s2/)
- greg_dump.sh - regenerates .i + cc1 -da + extracts func.greg/func.lreg (rerunnable)
- func.greg, func.lreg - RTL dumps (score-2 form)
- allocdbg.txt (ALLOCDBG priority table), findreg94.txt (FINDREGDBG for v1)
- show_diff.py - prints only true-mismatch lines from the s1 differ

## OPERATOR NOTE — 2026-08-07 22:31 circuit-break resolution (citation format)

Three sessions were discarded for one mechanical reason: self_vet.md
PRECEDENT lines must match the validator regex (grindlib.py:54) —
`file.ext:LINE` or a 7-40 hex commit hash. A prose reference to
memory/feedback/split-init-accumulation-sanctioned.md (a user-memory
file outside the repo) can never satisfy it.

Regex-passing anchors for the split-init-accumulation sanction:
- ad11a8c8 — "Match: func_80049C24 (text1b.c) — COMPLETED-C, retires 10
  rules (USER-SANCTIONED split-init)" — the sanction's landing commit.
- The 22:21 Judge PASS ruling on this very function cites split-init as
  a sanctioned class: docs/grind/decisions.md:4075

Use this exact form in future self_vet.md PRECEDENT lines. (The driver
now also feeds the discard reason into the respawned session's brief.)
