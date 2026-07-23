# Evidence bank — ang_hosei_80056FE8

- WIP rejected_form: {'form': 'drop the `var_v0 =` test-assignment (test the load directly)', 'score': 14, 'reason': 'Worse: target reuses the loaded value, so removing the assignment changes the load. Keep `(var_v0 = *(s16*)(arg0+0x5E))`.'}

- WIP rejected_form: {'form': 'compute base = a3*40 AFTER the if/else / inline at use', 'score': 27, 'reason': 'Much worse (build 39): moves the multiply past the load. base must be computed early (as target does).'}

- WIP rejected_form: {'form': 'base += var_v0 in place / partial += 0x12C / group const', 'score': '15-16', 'reason': 'Addition reordering does not force partial before the load; all worse than the named-partial floor 10.'}

- WIP rejected_form: {'form': 'structural variants of accumulator/split-init/declaration-order (2026-06-22 session, 10 forms)', 'score': 10, 'reason': "All produce masked floor 10 (= candidate.c floor) but differ in WHICH physical register hosts var_v0. NONE produce var_v0 -> $v0. GCC 2.7.2's allocator routes var_v0 to $a1 / $v1 / $v0(via accumulator) depending on form; even when var_v0 lands in $v0 (full-accumulator variant), the partial-add (`addu` of base+var_v0) doesn't displace the lh delay slot fill because base lives in a different reg (e.g. $a2), so the scheduler still fills the lh's slot with the partial-add. The wall isn't 'var_v0 in $v0' — it's the FULL coupling: var_v0 in $v0 AND base in $a1 simultaneously, so that `addu $a1, $a1, $v0` is FORCED before the `lw $v0, 0($a0)` (otherwise the lw clobbers live var_v0)."}

- == imported from memory/wip notes.md ==
# ang_hosei_80056FE8 (text1b.c) — WIP, floor 10, BLOCKED (RA tie)

## TL;DR (2026-06-14)
40-insn angle-correction lookup. HEAD matched only via a forbidden
`register s32 partial asm("$5")` pin + `asm volatile("")` scheduling barrier.
Removed both → clean candidate, floor sandbox 10 / build_insns 42 (target 43).
Full build SHA1 mismatch (650213f4...) confirms a real register-allocation
gap, NOT a masking artifact. No pure-C lever closed it this session.

## The gap
Target: `base = a3*40` lives in **$a1**, `var_v0` in **$v0** throughout; then
`addu a1,a1,v0` computes `partial = base+var_v0` BEFORE the final
`lh v0,1034(v0)` (so the lh's load-delay slot is a `nop`).
Mine: `var_v0`'s test `lh a1,94(a0)` claims **$a1** first, so `base` is parked
in **$a3**; the partial-add then fills the lh delay slot → build is 1 insn
SHORT (42 vs 43). It's a `base`<->`var_v0` register swap (masked-invisible in
the sandbox score) plus the delay-slot-fill consequence.

## Resume steps
1. Paste candidate.c; confirm sandbox 10.
2. Push `var_v0` into $v0 and `base` into $a1 in pure C. The masked sandbox
   CANNOT see the a1/a3 choice (scores 10 either way) — verify ONLY via the
   FULL build SHA1.
3. Likely modality: decomp-permuter (tiny fn, pure RA/scheduling residual).

## Ruled out (do not re-derive)
- Dropping the `var_v0 =` test assignment: floor 14 (target reuses the value).
- Computing base late / inline: floor 27 (multiply moves past the load).
- Addition reordering (base+=, partial+=0x12C, group const): floor 15-16.

### 2026-06-22 — 10 more structural variants, all stay at masked 10 / build 42 insns
- No `partial` intermediate (inline `base+var_v0` in return): same alloc.
- `var_v0` declared FIRST (lower LUID): same alloc.
- Remove `a3` intermediate (inline def): same alloc.
- `var_v0 += base; return var_v0 + lookup + 0x12C;`: DIFFERENT alloc (base→$a2, var_v0→$v1), still 42.
- `s32 partial = base + var_v0; s32 lookup = ...; return partial+lookup+0x12C;`: same alloc as canonical candidate.
- Full accumulator (`var_v0 += base; var_v0 += lookup; var_v0 += 0x12C; return var_v0;`): DIFFERENT alloc (base→$a2, var_v0→$v1, lookup→$v0); still 42.
- Same-var split-init (`s32 partial = a3 * 40;` … `partial += var_v0;`): DIFFERENT alloc (partial→$a2, var_v0→$v1); still 42.
- `return (base + lookup + 0x12C) + var_v0;` (var_v0 as LAST addend): addiu 0x12C fills lh slot; still 42.
- `register s32 var_v0;` (storage-class hint, NOT asm-pin): same alloc; NOTE: cheat_asm_stripped counter increments → dropped.

**Pattern:** every variant produces the same TOTAL insn count (42) but with DIFFERENT register choices. Masked sandbox cannot distinguish — all score 10. The wall is the FULL coupling `var_v0→$v0 AND base→$a1 SIMULTANEOUSLY` — only then does the partial-add `addu $a1, $a1, $v0` become forced-before the `lw $v0, 0($a0)` (since the lw would clobber live var_v0 in $v0). When base is in $a2/$a3, the addu doesn't conflict with the lw's $v0 destination, so the scheduler is free to put it in the lh delay slot.

## Pointers
- `.claude/rules/register-alloc-pure-c.md` (Lever A; pins/barriers forbidden)
- Same masked-10 RA-coupling class as this batch's other backlog items.


- == s1 recon (2026-07-23) ==
- FLOOR confirmed live: sandbox 10, build_insns 42 vs target 43, canonical verdict C. Clean candidate (no pin/barrier) applied to src/text1b.c; cheat_asm_stripped 352 with clean form vs 355 with HEAD's pin form.
- NO ANALOG: find_duplicates.py at threshold 0.85 AND 0.6 -> zero near-clone pairs for ang_hosei_80056FE8. Function is a standalone leaf; the "ang_hosei" name family is structurally unrelated (ang_hosei_8003F62C is a non-leaf with jal chains).
- m2c reference generated (tmp/grind/ang_hosei_80056FE8/s1/m2c_reference.c): confirms the candidate's dataflow (base=a3*40, 3-arm var_v0, arg0 RELOAD at 0x40A not a2-reuse). m2c inlines base into return; target/candidate compute it early (base-late measured floor 27).
- The 1-insn gap is a single load-delay NOP: target FORCES partial-add (`addu $a1,$a1,$v0`) before the arg0 reload (`lw $v0,0($a0)`) because the reload clobbers live var_v0 in $v0; our build lets the partial-add fill the lh's delay slot. Requires coupled {base->$a1, var_v0->$v0} simultaneously.
- NOT yet attempted (open avenues): RTL .greg register-disposition dump; clean single-function-target permuter campaign. Prior 2 sessions only did manual structural sweeps + objdump-tail comparison.

- [s1] sandbox --disable all: score 10, target_insns 43, build_insns 42, verdict C (canonical). Floor unchanged from imported wip.

- [s1] Clean candidate (no register-asm pin, no asm-volatile barrier) applied to src/text1b.c; sandbox floor holds at 10 (cheat_asm_stripped 352 vs 355 for HEAD's pin form).

- [s1] find_duplicates.py at threshold 0.85 AND 0.6: zero near-clone pairs for ang_hosei_80056FE8 — no analog to seed from.

- [s1] m2c reference saved (tmp/grind/ang_hosei_80056FE8/s1/m2c_reference.c): base=a3*40, 3-arm var_v0 dispatch, arg0 RELOADED at +0x40A (not a2-reuse). Matches candidate.

- [s1] The single missing insn is the lh(+0x40A) load-delay nop; it appears only when partial-add is forced before the arg0 reload, i.e. only under {base->$a1 AND var_v0->$v0} simultaneously.

- [s1] NOT yet attempted on this function: RTL .greg register-disposition dump, and a clean single-function-target permuter campaign. Prior 2 sessions did manual structural sweeps + objdump-tail comparison only.
