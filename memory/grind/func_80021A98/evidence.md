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
